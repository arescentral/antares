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

#include <pn/input>
#include <pn/map>
#include <pn/output>
#include <sfz/sfz.hpp>

#include "config/dirs.hpp"
#include "config/keys.hpp"

using sfz::makedirs;
using sfz::range;
using sfz::path::dirname;

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

static void set(bool& v, pn::value_cref x) {
    if (x.is_bool()) {
        v = x.as_bool();
    }
}

static void set(int& v, pn::value_cref x) {
    if (x.is_int()) {
        v = x.as_int();
    }
}

static void set(int16_t& v, pn::value_cref x) {
    if (x.is_int()) {
        v = x.as_int();
    }
}

static void set(Key& v, pn::value_cref x) {
    if (x.is_int()) {
        v = Key(x.as_int());
    }
}

static void set(Size& s, pn::value_cref x) {
    if (x.is_map()) {
        set(s.width, x.as_map().get("width"));
        set(s.height, x.as_map().get("height"));
    }
}

FilePrefsDriver::FilePrefsDriver(pn::string_view path) : _path(path.copy()) {
    using ::antares::set;

    pn::value x;
    pn::input in = pn::input{_path, pn::text};
    if (!in || !pn::parse(in, &x, nullptr)) {
        return;
    }
    pn::map_cref m = x.as_map();

    pn::map_cref sound = m.get("sound").as_map();
    set(_current.volume, sound.get("volume"));
    set(_current.speech_on, sound.get("speech"));
    set(_current.play_idle_music, sound.get("idle music"));
    set(_current.play_music_in_game, sound.get("game music"));

    pn::map_cref keys = m.get("keys").as_map();
    for (auto i : range<size_t>(KEY_COUNT)) {
        set(_current.keys[i], keys.get(kKeyNames[i]));
    }

    pn::map_cref video = m.get("video").as_map();
    set(_current.fullscreen, video.get("fullscreen"));
    set(_current.window_size, video.get("window"));
}

void FilePrefsDriver::set(const Preferences& p) {
    _current = p.copy();

    pn::map keys;
    for (auto i : range<size_t>(KEY_COUNT)) {
        keys[kKeyNames[i]] = p.keys[i].value();
    }

    makedirs(dirname(_path), 0755);
    pn::output output = pn::output{_path, pn::text};
    output.dump(pn::map{
            {"sound",
             pn::map{
                     {"volume", p.volume},
                     {"speech", p.speech_on},
                     {"idle music", p.play_idle_music},
                     {"game music", p.play_music_in_game},
             }},
            {"keys", std::move(keys)},
            {"video",
             pn::map{
                     {"fullscreen", p.fullscreen},
                     {"window",
                      pn::map{
                              {"width", p.window_size.width},
                              {"height", p.window_size.height},
                      }},
             }},
    });
}

}  // namespace antares
