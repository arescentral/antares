// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2012 The Antares Authors
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
    playing = false;
    song.reset();
    channel = sys.audio->open_channel();
}

void Music::PlaySong() {
    channel->activate();
    song->loop();
    playing = true;
}

void Music::StopSong() {
    channel->quiet();
    playing = false;
}

void Music::ToggleSong() {
    if (playing) {
        StopSong();
    } else {
        PlaySong();
    }
}

bool Music::SongIsPlaying() {
    return playing;
}

void Music::StopAndUnloadSong() {
    StopSong();
    song.reset();
}

void Music::LoadSong(int id) {
    StopSong();
    song = sys.audio->open_sound(format("/music/{0}", id));
}

void Music::SetSongVolume(double volume) {
    channel->amp(255 * volume);
}

}  // namespace antares
