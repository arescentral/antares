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
#include "sound/driver.hpp"

using sfz::format;
using std::unique_ptr;

namespace antares {

namespace {

bool playing = false;
unique_ptr<Sound> song;
unique_ptr<SoundChannel> channel;

}  // namespace

void MusicInit() {
    playing = false;
    song.reset();
    channel = SoundDriver::driver()->open_channel();
}

void MusicCleanup() {
    channel->quiet();
    channel.reset();
    song.reset();
    playing = false;
}

void PlaySong() {
    channel->activate();
    song->loop();
    playing = true;
}

void StopSong() {
    channel->quiet();
    playing = false;
}

void ToggleSong() {
    if (playing) {
        StopSong();
    } else {
        PlaySong();
    }
}

bool SongIsPlaying() {
    return playing;
}

void StopAndUnloadSong() {
    StopSong();
    song.reset();
}

void LoadSong(int id) {
    StopSong();
    song = SoundDriver::driver()->open_sound(format("/music/{0}", id));
}

void SetSongVolume(double volume) {
    channel->amp(255 * volume);
}

}  // namespace antares
