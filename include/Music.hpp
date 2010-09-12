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

#ifndef ANTARES_MUSIC_HPP_
#define ANTARES_MUSIC_HPP_

namespace antares {

const int kTitleSongID = 4001;  // Doomtroopers, Unite!

const double kMusicVolume = 0.84375;  // In-game music volume.
const double kMaxMusicVolume = 1.0;   // Idle music volume.

void MusicInit();
void MusicCleanup();
void PlaySong();
void ToggleSong();
bool SongIsPlaying();
void StopAndUnloadSong();
void LoadSong(int id);
void SetSongVolume(double volume);

}  // namespace antares

#endif // ANTARES_MUSIC_HPP_
