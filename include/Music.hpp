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

// Music.h

namespace antares {

#define kTitleSongID        4001
#define kPlaySongID         4000
#define kMusicVolume        54
#define kMaxMusicVolume     64

STUB0(MusicInit, int(), 0);
STUB0(MusicCleanup, void());
STUB0(PlaySong, void());
STUB0(ToggleSong, void());
STUB0(SongIsPlaying, bool(), false);
STUB0(StopAndUnloadSong, void());
STUB1(LoadSong, void( short));
STUB0(GetSongVolume, long(), 0);
STUB1(SetSongVolume, void( long));

}  // namespace antares

#endif // ANTARES_MUSIC_HPP_
