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

#include <sfz/sfz.hpp>
#include <vector>

#include "drawing/sprite-handling.hpp"
#include "sound/fx.hpp"
#include "sound/music.hpp"

namespace antares {

class SoundDriver;
class Font;
class PrefsDriver;
class VideoDriver;

struct SystemGlobals {
    struct {
        const Font* tactical;
        const Font* computer;
        const Font* button;
        const Font* title;
        const Font* small_button;
    } fonts;

    std::vector<sfz::String> key_names;
    std::vector<sfz::String> key_long_names;
    std::vector<sfz::String> gamepad_names;
    std::vector<sfz::String> gamepad_long_names;

    enum { ROT_TABLE_SIZE = 720 };
    int32_t rot_table[ROT_TABLE_SIZE];

    SoundDriver* audio = nullptr;
    VideoDriver* video = nullptr;
    PrefsDriver* prefs = nullptr;

    struct {
        std::vector<sfz::String> codes;
        std::vector<sfz::String> on;
        std::vector<sfz::String> off;
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
