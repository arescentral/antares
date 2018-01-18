// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2015-2017 The Antares Authors
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

#include "config/file-prefs-driver.hpp"

#include <fcntl.h>
#include <pn/file>
#include <pn/map>
#include <sfz/sfz.hpp>

#include "config/dirs.hpp"
#include "config/keys.hpp"
#include "data/pn.hpp"

using sfz::Exception;
using sfz::MappedFile;
using sfz::ScopedFd;
using sfz::String;
using sfz::StringMap;
using sfz::format;
using sfz::makedirs;
using sfz::path::dirname;
using sfz::range;
using std::vector;

namespace utf8 = sfz::utf8;

namespace antares {

static const char* kKeyNames[KEY_COUNT] = {
        "ship accel",      "ship decel",     "ship ccw",    "ship cw",      "fire 1",
        "fire 2",          "fire s",         "warp",

        "select friendly", "select hostile", "select base", "target",       "order",
        "zoom in",         "zoom out",

        "comp up",         "comp down",      "comp accept", "comp cancel",

        "transfer",        "zoom 1:1",       "zoom 1:2",    "zoom 1:4",     "zoom 1:16",
        "zoom hostile",    "zoom object",    "zoom all",    "next message", "help",
        "volume down",     "volume up",      "game music",  "net settings", "fast motion",

        "hotkey 1",        "hotkey 2",       "hotkey 3",    "hotkey 4",     "hotkey 5",
        "hotkey 6",        "hotkey 7",       "hotkey 8",    "hotkey 9",     "hotkey 10",
};

static bool get(pn::value_cref x, bool& v) {
    if (x.is_bool()) {
        v = x.as_bool();
        return true;
    }
    return false;
}

static bool get(pn::value_cref x, int& v) {
    if (x.is_int()) {
        v = x.as_int();
        return true;
    }
    return false;
}

template <typename ValueType, typename ValueKey, typename PrefsMethod>
static void set_from(
        pn::map_cref x, const char* section_key, ValueKey value_key, Preferences& prefs,
        PrefsMethod pmeth) {
    auto      section = x.get(section_key).as_map();
    auto      value   = section.get(value_key);
    ValueType typed;
    if (get(value, typed)) {
        (prefs.*pmeth) = typed;
    }
}

template <typename ValueType, typename ValueKey, typename PrefsMethod>
static void set_from(
        pn::map_cref x, const char* section_key, ValueKey value_key, Preferences& prefs,
        PrefsMethod pmeth, int index) {
    auto      section = x.get(section_key).as_map();
    auto      value   = section.get(value_key);
    ValueType typed;
    if (get(value, typed)) {
        (prefs.*pmeth)[index] = typed;
    }
}

FilePrefsDriver::FilePrefsDriver() {
    sfz::Json();  // TODO(sfiera): remove me. Just here as a workaround for a weird linker bug.

    try {
        String     path(format("{0}/config.pn", pn2sfz(dirs().root)));
        MappedFile file(path);
        pn::value  x;
        String     data(utf8::decode(file.data()));
        if (!pn::parse(sfz2pn(data).open(), x, nullptr)) {
            return;
        }
        pn::map_cref m = x.as_map();

        set_from<int>(m, "sound", "volume", _current, &Preferences::volume);
        set_from<bool>(m, "sound", "speech", _current, &Preferences::speech_on);
        set_from<bool>(m, "sound", "idle music", _current, &Preferences::play_idle_music);
        set_from<bool>(m, "sound", "game music", _current, &Preferences::play_music_in_game);

        for (auto i : range<size_t>(KEY_COUNT)) {
            set_from<int>(m, "keys", kKeyNames[i], _current, &Preferences::keys, i);
        }
    } catch (Exception& e) {
        // pass
    }
}

void FilePrefsDriver::set(const Preferences& p) {
    _current = p.copy();

    pn::map keys;
    for (auto i : range<size_t>(KEY_COUNT)) {
        keys[kKeyNames[i]] = p.keys[i];
    }

    String path(format("{0}/config.pn", pn2sfz(dirs().root)));
    makedirs(dirname(path), 0755);
    ScopedFd fd(open(path, O_CREAT | O_TRUNC | O_WRONLY, 0644));
    String   pretty =
            pn2sfz(pn::dump(
                    pn::map{{"sound", pn::map{{"volume", p.volume},
                                              {"speech", p.speech_on},
                                              {"idle music", p.play_idle_music},
                                              {"game music", p.play_music_in_game}}},
                            {"keys", std::move(keys)}}));
    write(fd, utf8::encode(pretty));
}

}  // namespace antares
