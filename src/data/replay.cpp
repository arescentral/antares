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
#include <sfz/sfz.hpp>

#include "config/dirs.hpp"
#include "config/keys.hpp"
#include "config/preferences.hpp"
#include "game/sys.hpp"

using sfz::Bytes;
using sfz::BytesSlice;
using sfz::CString;
using sfz::Exception;
using sfz::ReadSource;
using sfz::ScopedFd;
using sfz::String;
using sfz::StringSlice;
using sfz::WriteTarget;
using sfz::format;
using sfz::range;
using sfz::read;
using sfz::write;
using std::map;

namespace path = sfz::path;
namespace utf8 = sfz::utf8;

namespace antares {

ReplayData::ReplayData() {}

ReplayData::ReplayData(sfz::BytesSlice in) {
    read(in, *this);
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

static void write_varint(WriteTarget out, uint64_t value) {
    if (value == 0) {
        out.push(1, '\0');
    }
    while (value != 0) {
        uint8_t byte = value & 0x7f;
        value >>= 7;
        if (value) {
            byte |= 0x80;
        }
        out.push(1, byte);
    }
}

static void tag_varint(WriteTarget out, uint64_t tag, uint64_t value) {
    write_varint(out, tag);
    write_varint(out, value);
}

// On mac, size_t is a distinct type.  On linux, it's the same as one
// of the built-in integer types.  is_size_t lets us define methods for
// both without getting a linker error for having a duplicate symbol.
// It doesn't matter which version we use if they're the same.
template <typename T, bool is_size_t = std::is_same<T, size_t>::value>
static T read_varint(ReadSource in);

template <>
uint64_t read_varint<uint64_t, false>(ReadSource in) {
    uint64_t byte;
    uint64_t value = 0;
    int      shift = 0;
    do {
        byte = read<uint8_t>(in);
        value |= (byte & 0x7f) << shift;
        shift += 7;
    } while (byte & 0x80);
    return value;
}

template <>
int64_t read_varint<int64_t, false>(ReadSource in) {
    uint64_t u64 = read_varint<uint64_t, false>(in);
    int64_t  s64 = u64 & 0x7fffffffffffffffULL;
    if (u64 & 0x8000000000000000ULL) {
        s64 += -0x8000000000000000ULL;
    }
    return s64;
}

template <>
size_t read_varint<size_t, true>(ReadSource in) {
    return read_varint<uint64_t, false>(in);
}

template <>
int32_t read_varint<int32_t, false>(ReadSource in) {
    return read_varint<int64_t, false>(in);
}

template <>
uint8_t read_varint<uint8_t, false>(ReadSource in) {
    return read_varint<int64_t, false>(in);
}

static void tag_string(WriteTarget out, uint64_t tag, const String& s) {
    write_varint(out, tag);
    write_varint(out, s.size());
    write(out, utf8::encode(s));
}

static String read_string(ReadSource in) {
    Bytes bytes(read_varint<size_t>(in), '\0');
    in.shift(bytes.data(), bytes.size());
    return String(utf8::decode(bytes));
}

template <typename T>
static void tag_message(WriteTarget out, uint64_t tag, const T& message) {
    Bytes bytes;
    write(bytes, message);
    write_varint(out, tag);
    write_varint(out, bytes.size());
    write(out, bytes);
}

template <typename T>
static T read_message(ReadSource in) {
    Bytes bytes(read_varint<size_t>(in), '\0');
    in.shift(bytes.data(), bytes.size());
    T          message;
    BytesSlice slice = bytes;
    read(slice, message);
    return message;
}

void read_from(ReadSource in, ReplayData& replay) {
    while (!in.empty()) {
        switch (read_varint<uint64_t>(in)) {
            case SCENARIO: replay.scenario       = read_message<ReplayData::Scenario>(in); break;
            case CHAPTER: replay.chapter_id      = read_varint<int32_t>(in); break;
            case GLOBAL_SEED: replay.global_seed = read_varint<int32_t>(in); break;
            case DURATION: replay.duration       = read_varint<uint64_t>(in); break;
            case ACTION: replay.actions.push_back(read_message<ReplayData::Action>(in)); break;
        }
    }
}

void read_from(ReadSource in, ReplayData::Scenario& scenario) {
    while (!in.empty()) {
        switch (read_varint<uint64_t>(in)) {
            case SCENARIO_IDENTIFIER: scenario.identifier = read_string(in); break;
            case SCENARIO_VERSION: scenario.version       = read_string(in); break;
        }
    }
}

void read_from(ReadSource in, ReplayData::Action& action) {
    while (!in.empty()) {
        switch (read_varint<uint64_t>(in)) {
            case ACTION_AT: action.at = read_varint<uint64_t>(in); break;
            case ACTION_KEY_DOWN: action.keys_down.push_back(read_varint<uint8_t>(in)); break;
            case ACTION_KEY_UP: action.keys_up.push_back(read_varint<uint8_t>(in)); break;
        }
    }
}

void write_to(WriteTarget out, const ReplayData& replay) {
    tag_message(out, SCENARIO, replay.scenario);
    tag_varint(out, CHAPTER, replay.chapter_id);
    tag_varint(out, GLOBAL_SEED, replay.global_seed);
    tag_varint(out, DURATION, replay.duration);
    for (const ReplayData::Action& action : replay.actions) {
        tag_message(out, ACTION, action);
    }
}

void write_to(WriteTarget out, const ReplayData::Scenario& scenario) {
    tag_string(out, SCENARIO_IDENTIFIER, scenario.identifier);
    tag_string(out, SCENARIO_VERSION, scenario.version);
}

void write_to(WriteTarget out, const ReplayData::Action& action) {
    tag_varint(out, ACTION_AT, action.at);
    for (uint8_t key : action.keys_down) {
        tag_varint(out, ACTION_KEY_DOWN, key);
    }
    for (uint8_t key : action.keys_up) {
        tag_varint(out, ACTION_KEY_UP, key);
    }
}

ReplayBuilder::ReplayBuilder() {}

namespace {

// TODO(sfiera): put globbing in a central location.
struct ScopedGlob {
    glob_t data;
    ScopedGlob() { memset(&data, sizeof(data), 0); }
    ~ScopedGlob() { globfree(&data); }
};

}  // namespace

// Deletes the oldest replay until there are fewer than `count` in the replays folder.
static void cull_replays(size_t count) {
    if (path::isdir(dirs().replays)) {
        ScopedGlob g;
        String     str(format("{0}/*.nlrp", dirs().replays));
        CString    c_str(str);
        glob(c_str.data(), 0, NULL, &g.data);

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
        makedirs(dirs().replays, 0755);
    }
}

void ReplayBuilder::init(
        StringSlice scenario_identifier, StringSlice scenario_version, int32_t chapter_id,
        int32_t global_seed) {
    _scenario.identifier.assign(scenario_identifier);
    _scenario.version.assign(scenario_version);
    _chapter_id  = chapter_id;
    _global_seed = global_seed;
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
    sfz::String path(format("{0}/Replay {1}.nlrp", dirs().replays, utf8::decode(buffer)));
    int         fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0644);
    if (fd >= 0) {
        _file.reset(new ScopedFd(fd));
        tag_message(*_file, SCENARIO, _scenario);
        tag_varint(*_file, CHAPTER, _chapter_id);
        tag_varint(*_file, GLOBAL_SEED, _global_seed);
    }
}

void ReplayBuilder::key_down(const KeyDownEvent& event) {
    if (!_file) {
        return;
    }
    for (auto i : range<int>(KEY_COUNT)) {
        if (event.key() == sys.prefs->key(i) - 1) {
            ReplayData::Action action = {};
            action.at                 = _at;
            action.keys_down.push_back(i);
            tag_message(*_file, ACTION, action);
        }
    }
}

void ReplayBuilder::key_up(const KeyUpEvent& event) {
    if (!_file) {
        return;
    }
    for (auto i : range<int>(KEY_COUNT)) {
        if (event.key() == sys.prefs->key(i) - 1) {
            ReplayData::Action action = {};
            action.at                 = _at;
            action.keys_up.push_back(i);
            tag_message(*_file, ACTION, action);
            break;
        }
    }
}

void ReplayBuilder::next() {
    ++_at;
}

void ReplayBuilder::finish() {
    if (!_file) {
        return;
    }
    tag_varint(*_file, DURATION, _at);
}

}  // namespace antares
