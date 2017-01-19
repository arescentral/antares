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

#include "sound/music.hpp"

#include "config/preferences.hpp"
#include "game/sys.hpp"
#include "lang/defines.hpp"
#include "sound/driver.hpp"

using sfz::format;
using std::unique_ptr;

namespace antares {

void Music::init() {
    _playing   = false;
    _song_type = IDLE;
    _song_id   = kTitleSongID;
    _song.reset();
    _channel = sys.audio->open_channel();
}

void Music::play(Type type, int16_t id) {
    bool   play   = false;
    double volume = 1.0;
    if (type == IDLE) {
        play = sys.prefs->play_idle_music();
    } else if (type == IN_GAME) {
        play   = sys.prefs->play_music_in_game();
        volume = 0.84375;
    }

    if (_playing && play && (_song_id == id)) {
        return;
    }
    stop();
    _song_type = type;
    _song_id   = id;

    if (play) {
        _song = sys.audio->open_sound(format("/music/{0}", id));
        _channel->activate();
        _channel->amp(255 * volume);
        _song->loop();
        _playing = true;
    }
}

void Music::stop() {
    _channel->quiet();
    _playing = false;
}

void Music::toggle() {
    if (_playing) {
        stop();
    } else {
        play(_song_type, _song_id);
    }
}

void Music::sync() {
    if (_song_type == IDLE) {
        if (sys.prefs->play_idle_music()) {
            play(_song_type, _song_id);
        } else {
            stop();
        }
    } else if (_song_type == IN_GAME) {
        if (sys.prefs->play_music_in_game()) {
            play(_song_type, _song_id);
        } else {
            stop();
        }
    }
}

}  // namespace antares
