// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2017 The Antares Authors
//
// This file is part of Antares, a tactical space combat game.
//
// Antares is free software: you can redistribute it and/or modify it
// under the terms of the Lesser GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Antares is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with Antares.  If not, see http://www.gnu.org/licenses/

#include "data/replay.hpp"

#include <fcntl.h>
#include <glob.h>
#include <time.h>
#include <unistd.h>
#include <pn/file>
#include <sfz/sfz.hpp>

#include "config/dirs.hpp"
#include "config/keys.hpp"
#include "config/preferences.hpp"
#include "game/sys.hpp"

using sfz::range;
using std::map;

namespace path = sfz::path;

namespace antares {

ReplayData::ReplayData() {}

ReplayData::ReplayData(pn::data_view in) {
    if (!read_from(in.open(), this)) {
        throw std::runtime_error("error while reading replay data");
    }
}

void ReplayData::key_down(uint64_t at, uint32_t key) {
    if (actions.empty() || (actions.back().at != at)) {
        actions.emplace_back();
        actions.back().at = at;
    }
    Action& action = actions.back();
    action.keys_down.push_back(key);
}

void ReplayData::key_up(uint64_t at, uint32_t key) {
    if (actions.empty() || (actions.back().at != at)) {
        actions.emplace_back();
        actions.back().at = at;
    }
    Action& action = actions.back();
    action.keys_up.push_back(key);
}

enum {
    VARINT           = 0,
    FIXED64          = 1,
    LENGTH_DELIMITED = 2,
    FIXED32          = 5,
};

enum {
    SCENARIO    = (0x01 << 3) | LENGTH_DELIMITED,
    CHAPTER     = (0x02 << 3) | VARINT,
    GLOBAL_SEED = (0x03 << 3) | VARINT,
    DURATION    = (0x04 << 3) | VARINT,
    ACTION      = (0x05 << 3) | LENGTH_DELIMITED,

    SCENARIO_IDENTIFIER = (0x01 << 3) | LENGTH_DELIMITED,
    SCENARIO_VERSION    = (0x02 << 3) | LENGTH_DELIMITED,

    ACTION_AT       = (0x01 << 3) | VARINT,
    ACTION_KEY_DOWN = (0x02 << 3) | VARINT,
    ACTION_KEY_UP   = (0x03 << 3) | VARINT,
};

static void write_varint(pn::file_view out, uint64_t value) {
    if (value == 0) {
        constexpr uint8_t zero = '\0';
        out.write(pn::data_view{&zero, 1});
    }
    while (value != 0) {
        uint8_t byte = value & 0x7f;
        value >>= 7;
        if (value) {
            byte |= 0x80;
        }
        out.write(pn::data_view{&byte, 1});
    }
}

static void tag_varint(pn::file_view out, uint64_t tag, uint64_t value) {
    write_varint(out, tag);
    write_varint(out, value);
}

// On mac, size_t is a distinct type.  On linux, it's the same as one
// of the built-in integer types.  is_size_t lets us define methods for
// both without getting a linker error for having a duplicate symbol.
// It doesn't matter which version we use if they're the same.
template <typename T, bool is_size_t = std::is_same<T, size_t>::value>
static bool read_varint(pn::file_view in, T* out);

template <>
bool read_varint<uint64_t, false>(pn::file_view in, uint64_t* out) {
    uint64_t byte;
    *out      = 0;
    int shift = 0;
    do {
        uint8_t c;
        if (fread(&c, 1, 1, in.c_obj()) != 1) {
            return false;
        }
        byte = c;
        *out |= (byte & 0x7f) << shift;
        shift += 7;
    } while (byte & 0x80);
    return true;
}

template <>
bool read_varint<int64_t, false>(pn::file_view in, int64_t* out) {
    uint64_t u64;
    if (!read_varint<uint64_t, false>(in, &u64)) {
        return false;
    }
    *out = u64 & 0x7fffffffffffffffULL;
    if (u64 & 0x8000000000000000ULL) {
        *out += -0x8000000000000000ULL;
    }
    return true;
}

template <>
bool read_varint<size_t, true>(pn::file_view in, size_t* out) {
    uint64_t u64;
    if (!read_varint<uint64_t, false>(in, &u64)) {
        return false;
    }
    *out = u64;
    return true;
}

template <>
bool read_varint<int32_t, false>(pn::file_view in, int32_t* out) {
    int64_t i64;
    if (!read_varint<int64_t, false>(in, &i64)) {
        return false;
    }
    *out = i64;
    return true;
}

template <>
bool read_varint<uint8_t, false>(pn::file_view in, uint8_t* out) {
    uint64_t u64;
    if (!read_varint<uint64_t, false>(in, &u64)) {
        return false;
    }
    *out = u64;
    return true;
}

static void tag_string(pn::file_view out, uint64_t tag, pn::string_view s) {
    write_varint(out, tag);
    write_varint(out, s.size());
    out.write(s);
}

static bool read_string(pn::file_view in, pn::string* out) {
    size_t size;
    if (!read_varint(in, &size)) {
        return false;
    }
    pn::data data;
    data.resize(size);
    if (fread(data.data(), 1, data.size(), in.c_obj()) != data.size()) {
        return false;
    }
    *out = data.as_string().copy();
    return true;
}

template <typename T>
static void tag_message(pn::file_view out, uint64_t tag, const T& message) {
    pn::data bytes;
    message.write_to(bytes.open("a"));
    write_varint(out, tag);
    write_varint(out, bytes.size());
    out.write(bytes);
}

template <typename T>
static bool read_message(pn::file_view in, T* out) {
    size_t size;
    if (!read_varint(in, &size)) {
        return false;
    }
    pn::data d;
    d.resize(size);
    if (fread(d.data(), 1, d.size(), in.c_obj()) != d.size()) {
        return false;
    }
    return read_from(d.open(), out);
}

bool read_from(pn::file_view in, ReplayData* replay) {
    while (true) {
        uint64_t tag;
        if (!read_varint(in, &tag)) {
            if (in.eof()) {
                return true;
            }
            throw std::runtime_error("error while reading replay");
        }

        switch (tag) {
            case SCENARIO:
                if (!read_message(in, &replay->scenario)) {
                    return false;
                }
                break;
            case CHAPTER:
                if (!read_varint(in, &replay->chapter_id)) {
                    return false;
                }
                break;
            case GLOBAL_SEED:
                if (!read_varint(in, &replay->global_seed)) {
                    return false;
                }
                break;
            case DURATION:
                if (!read_varint(in, &replay->duration)) {
                    return false;
                }
                break;
            case ACTION:
                replay->actions.emplace_back();
                if (!read_message(in, &replay->actions.back())) {
                    return false;
                }
                break;
        }
    }
}

bool read_from(pn::file_view in, ReplayData::Scenario* scenario) {
    while (true) {
        uint64_t tag;
        if (!read_varint(in, &tag)) {
            if (in.eof()) {
                return true;
            }
            throw std::runtime_error("error while reading replay scenario");
        }

        switch (tag) {
            case SCENARIO_IDENTIFIER:
                if (!read_string(in, &scenario->identifier)) {
                    return false;
                }
                break;
            case SCENARIO_VERSION:
                if (!read_string(in, &scenario->version)) {
                    return false;
                }
                break;
        }
    }
}

bool read_from(pn::file_view in, ReplayData::Action* action) {
    while (true) {
        uint64_t tag;
        if (!read_varint(in, &tag)) {
            if (in.eof()) {
                return true;
            }
            throw std::runtime_error("error while reading replay scenario");
        }

        switch (tag) {
            case ACTION_AT:
                if (!read_varint(in, &action->at)) {
                    return false;
                }
                break;

            case ACTION_KEY_DOWN:
                action->keys_down.emplace_back();
                if (!read_varint(in, &action->keys_down.back())) {
                    return false;
                }
                break;

            case ACTION_KEY_UP:
                action->keys_up.emplace_back();
                if (!read_varint(in, &action->keys_up.back())) {
                    return false;
                }
                break;
        }
    }
}

void ReplayData::write_to(pn::file_view out) const {
    tag_message(out, SCENARIO, scenario);
    tag_varint(out, CHAPTER, chapter_id);
    tag_varint(out, GLOBAL_SEED, global_seed);
    tag_varint(out, DURATION, duration);
    for (const ReplayData::Action& action : actions) {
        tag_message(out, ACTION, action);
    }
}

void ReplayData::Scenario::write_to(pn::file_view out) const {
    tag_string(out, SCENARIO_IDENTIFIER, identifier);
    tag_string(out, SCENARIO_VERSION, version);
}

void ReplayData::Action::write_to(pn::file_view out) const {
    tag_varint(out, ACTION_AT, at);
    for (uint8_t key : keys_down) {
        tag_varint(out, ACTION_KEY_DOWN, key);
    }
    for (uint8_t key : keys_up) {
        tag_varint(out, ACTION_KEY_UP, key);
    }
}

ReplayBuilder::ReplayBuilder() {}

namespace {

// TODO(sfiera): put globbing in a central location.
struct ScopedGlob {
    glob_t data;
    ScopedGlob() { memset(&data, 0, sizeof(data)); }
    ~ScopedGlob() { globfree(&data); }
};

}  // namespace

// Deletes the oldest replay until there are fewer than `count` in the replays folder.
static void cull_replays(size_t count) {
    if (path::isdir(dirs().replays)) {
        ScopedGlob g;
        pn::string str = pn::format("{0}/*.nlrp", dirs().replays);
        glob(str.c_str(), 0, NULL, &g.data);

        map<int64_t, const char*> files;
        for (int i = 0; i < g.data.gl_pathc; ++i) {
            struct stat st;
            if (stat(g.data.gl_pathv[i], &st) < 0) {
                continue;
            }
            files[st.st_mtime] = g.data.gl_pathv[i];
        }
        while (files.size() >= count) {
            if (unlink(files.begin()->second) < 0) {
                break;
            }
        }
    } else {
        sfz::makedirs(dirs().replays, 0755);
    }
}

void ReplayBuilder::init(
        pn::string_view scenario_identifier, pn::string_view scenario_version, int32_t chapter_id,
        int32_t global_seed) {
    _scenario.identifier = scenario_identifier.copy();
    _scenario.version    = scenario_version.copy();
    _chapter_id          = chapter_id;
    _global_seed         = global_seed;
}

void ReplayBuilder::start() {
    _at = 1;
    cull_replays(10);
    time_t    t;
    struct tm tm;
    char      buffer[1024];
    if ((time(&t) < 0) || !localtime_r(&t, &tm) || (strftime(buffer, 1024, "%c", &tm) <= 0)) {
        return;
    }
    pn::string path = pn::format("{0}/Replay {1}.nlrp", dirs().replays, buffer);
    pn::file   f    = pn::open(path, "w");
    if (f.c_obj()) {
        _file = std::move(f);
        tag_message(_file, SCENARIO, _scenario);
        tag_varint(_file, CHAPTER, _chapter_id);
        tag_varint(_file, GLOBAL_SEED, _global_seed);
    }
}

void ReplayBuilder::key_down(const KeyDownEvent& event) {
    if (!_file.c_obj()) {
        return;
    }
    for (auto i : range<int>(KEY_COUNT)) {
        if (event.key() == sys.prefs->key(i)) {
            ReplayData::Action action = {};
            action.at                 = _at;
            action.keys_down.push_back(i);
            tag_message(_file, ACTION, action);
        }
    }
}

void ReplayBuilder::key_up(const KeyUpEvent& event) {
    if (_file.c_obj()) {
        return;
    }
    for (auto i : range<int>(KEY_COUNT)) {
        if (event.key() == sys.prefs->key(i)) {
            ReplayData::Action action = {};
            action.at                 = _at;
            action.keys_up.push_back(i);
            tag_message(_file, ACTION, action);
            break;
        }
    }
}

void ReplayBuilder::next() { ++_at; }

void ReplayBuilder::finish() {
    if (!_file.c_obj()) {
        return;
    }
    tag_varint(_file, DURATION, _at);
}

}  // namespace antares
