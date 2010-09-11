// Ares, a tactical space combat game.
// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#include "Music.hpp"

#include "SoundDriver.hpp"

using sfz::scoped_ptr;

namespace antares {

namespace {

bool playing = false;
scoped_ptr<Sound> song;
scoped_ptr<SndChannel> channel;

}  // namespace

void MusicInit() {
    playing = false;
    song.reset();
    channel.reset(SoundDriver::driver()->new_channel());
}

void MusicCleanup() {
    channel->quiet();
    channel.reset();
    song.reset();
    playing = false;
}

void PlaySong() {
    channel->loop(song.get());
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
    song.reset(SoundDriver::driver()->new_song(id));
}

int GetSongVolume() {
    // TODO(sfiera): implement.
    return kMaxMusicVolume;
}

void SetSongVolume(int volume) {
    // TODO(sfiera): implement.
}

}  // namespace antares
