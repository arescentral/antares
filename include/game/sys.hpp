// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2016-2017 The Antares Authors
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

#ifndef ANTARES_GAME_SYS_HPP_
#define ANTARES_GAME_SYS_HPP_

#include <pn/string>
#include <vector>

#include "drawing/sprite-handling.hpp"
#include "drawing/text.hpp"
#include "sound/fx.hpp"
#include "sound/music.hpp"

namespace antares {

class SoundDriver;
class PrefsDriver;
class VideoDriver;

struct SystemGlobals {
    struct {
        Font tactical;
        Font computer;
        Font button;
        Font title;
        Font small_button;
    } fonts;

    std::vector<pn::string> key_names;
    std::vector<pn::string> key_long_names;
    std::vector<pn::string> gamepad_names;
    std::vector<pn::string> gamepad_long_names;

    enum { ROT_TABLE_SIZE = 720 };
    std::vector<int32_t> rot_table;

    SoundDriver* audio = nullptr;
    VideoDriver* video = nullptr;
    PrefsDriver* prefs = nullptr;

    std::vector<pn::string> messages;

    struct {
        std::vector<pn::string> codes;
        std::vector<pn::string> on;
        std::vector<pn::string> off;
    } cheat;

    SoundFX sound;
    Music   music;
    Pix     pix;

    Texture left_instrument_texture;
    Texture right_instrument_texture;
};

extern SystemGlobals sys;

void sys_init();

}  // namespace antares

#endif  // ANTARES_GAME_SYS_HPP_
