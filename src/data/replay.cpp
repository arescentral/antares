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
#include <time.h>
#include <pn/input>
#include <pn/output>
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

ReplayData::ReplayData(pn::input_view in) {
    if (!read_from(in, this)) {
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

static void write_varint(pn::output_view out, uint64_t value) {
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

static void tag_varint(pn::output_view out, uint64_t tag, uint64_t value) {
    write_varint(out, tag);
    write_varint(out, value);
}

// On mac, size_t is a distinct type.  On linux, it's the same as one
// of the built-in integer types.  is_size_t lets us define methods for
// both without getting a linker error for having a duplicate symbol.
// It doesn't matter which version we use if they're the same.
template <typename T, bool is_size_t = std::is_same<T, size_t>::value>
static bool read_varint(pn::input_view in, T* out);

template <>
bool read_varint<uint64_t, false>(pn::input_view in, uint64_t* out) {
    uint64_t byte;
    *out      = 0;
    int shift = 0;
    do {
        uint8_t c;
        if (!in.read(&c)) {
            return false;
        }
        byte = c;
        *out |= (byte & 0x7f) << shift;
        shift += 7;
    } while (byte & 0x80);
    return true;
}

template <>
bool read_varint<int64_t, false>(pn::input_view in, int64_t* out) {
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
bool read_varint<size_t, true>(pn::input_view in, size_t* out) {
    uint64_t u64;
    if (!read_varint<uint64_t, false>(in, &u64)) {
        return false;
    }
    *out = u64;
    return true;
}

template <>
bool read_varint<int32_t, false>(pn::input_view in, int32_t* out) {
    int64_t i64;
    if (!read_varint<int64_t, false>(in, &i64)) {
        return false;
    }
    *out = i64;
    return true;
}

template <>
bool read_varint<uint8_t, false>(pn::input_view in, uint8_t* out) {
    uint64_t u64;
    if (!read_varint<uint64_t, false>(in, &u64)) {
        return false;
    }
    *out = u64;
    return true;
}

static void tag_string(pn::output_view out, uint64_t tag, pn::string_view s) {
    write_varint(out, tag);
    write_varint(out, s.size());
    out.write(s);
}

static bool read_string(pn::input_view in, pn::string* out) {
    size_t size;
    if (!read_varint(in, &size)) {
        return false;
    }
    pn::data data;
    data.resize(size);
    if (!in.read(&data)) {
        return false;
    }
    *out = data.as_string().copy();
    return true;
}

template <typename T>
static void tag_message(pn::output_view out, uint64_t tag, const T& message) {
    pn::data bytes;
    message.write_to(bytes.output());
    write_varint(out, tag);
    write_varint(out, bytes.size());
    out.write(bytes);
}

template <typename T>
static bool read_message(pn::input_view in, T* out) {
    size_t size;
    if (!read_varint(in, &size)) {
        return false;
    }
    pn::data d;
    d.resize(size);
    if (!in.read(&d)) {
        return false;
    }
    return read_from(d.input(), out);
}

bool read_from(pn::input_view in, ReplayData* replay) {
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

bool read_from(pn::input_view in, ReplayData::Scenario* scenario) {
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

bool read_from(pn::input_view in, ReplayData::Action* action) {
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

void ReplayData::write_to(pn::output_view out) const {
    tag_message(out, SCENARIO, scenario);
    tag_varint(out, CHAPTER, chapter_id);
    tag_varint(out, GLOBAL_SEED, global_seed);
    tag_varint(out, DURATION, duration);
    for (const ReplayData::Action& action : actions) {
        tag_message(out, ACTION, action);
    }
}

void ReplayData::Scenario::write_to(pn::output_view out) const {
    tag_string(out, SCENARIO_IDENTIFIER, identifier);
    tag_string(out, SCENARIO_VERSION, version);
}

void ReplayData::Action::write_to(pn::output_view out) const {
    tag_varint(out, ACTION_AT, at);
    for (uint8_t key : keys_down) {
        tag_varint(out, ACTION_KEY_DOWN, key);
    }
    for (uint8_t key : keys_up) {
        tag_varint(out, ACTION_KEY_UP, key);
    }
}

ReplayBuilder::ReplayBuilder() {}

static bool is_replay(pn::string_view s) { return s.rfind(".nlrp") == (s.size() - 5); }

// Deletes the oldest replay until there are fewer than `count` in the replays folder.
static void cull_replays(size_t count) {
    if (path::isdir(dirs().replays)) {
        map<int64_t, pn::string> files;
        for (const auto& ent : sfz::scandir(dirs().replays)) {
            if (is_replay(ent.name)) {
                // TODO(sfiera): make the pn::string_view{} constructor unnecessary.
                files[ent.st.st_mtime] =
                        sfz::path::join(dirs().replays, pn::string_view{ent.name});
            }
        }
        while (files.size() >= count) {
            sfz::unlink(files.begin()->second);
            files.erase(files.begin());
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

static bool safe_localtime(time_t* t, struct tm* tm) {
#ifdef _WIN32
    return localtime_s(tm, t) == 0;
#else
    return localtime_r(t, tm) != nullptr;
#endif
}

void ReplayBuilder::start() {
    _at = 1;
    cull_replays(10);
    time_t    t;
    struct tm tm;
    char      buffer[1024];
    if ((time(&t) < 0) || !safe_localtime(&t, &tm) || (strftime(buffer, 1024, "%c", &tm) <= 0)) {
        return;
    }
    pn::string path = pn::format("{0}/Replay {1}.nlrp", dirs().replays, buffer);
    pn::output o    = pn::output{path, pn::binary};
    if (o) {
        _out = std::move(o);
        tag_message(_out, SCENARIO, _scenario);
        tag_varint(_out, CHAPTER, _chapter_id);
        tag_varint(_out, GLOBAL_SEED, _global_seed);
    }
}

void ReplayBuilder::key_down(const KeyDownEvent& event) {
    if (!_out.c_obj()) {
        return;
    }
    for (auto i : range<int>(KEY_COUNT)) {
        if (event.key() == sys.prefs->key(i)) {
            ReplayData::Action action = {};
            action.at                 = _at;
            action.keys_down.push_back(i);
            tag_message(_out, ACTION, action);
        }
    }
}

void ReplayBuilder::key_up(const KeyUpEvent& event) {
    if (_out.c_obj()) {
        return;
    }
    for (auto i : range<int>(KEY_COUNT)) {
        if (event.key() == sys.prefs->key(i)) {
            ReplayData::Action action = {};
            action.at                 = _at;
            action.keys_up.push_back(i);
            tag_message(_out, ACTION, action);
            break;
        }
    }
}

void ReplayBuilder::next() { ++_at; }

void ReplayBuilder::finish() {
    if (!_out.c_obj()) {
        return;
    }
    tag_varint(_out, DURATION, _at);
}

}  // namespace antares
