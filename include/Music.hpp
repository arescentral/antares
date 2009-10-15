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

STUB2(DoConversionS3M, void( Str255, short));

STUB2(pStrcat, unsigned char *(unsigned char *, unsigned char *), NULL);
STUB2(DoExp1to3, Handle( Handle, unsigned long), NULL);
STUB2(DoExp1to6, Handle( Handle, unsigned long), NULL);
STUB2(SndToHandle, Handle( Handle, short   *), NULL);

#if 0
int MusicInit( void);
void MusicCleanup( void);
void PlaySong( void);
void ToggleSong( void);
bool SongIsPlaying( void);
void StopAndUnloadSong( void);
void LoadSong( short);
long GetSongVolume( void);
void SetSongVolume( long);

void DoConversionS3M( Str255, short);

unsigned char *pStrcat(unsigned char *, unsigned char *);
Handle DoExp1to3( Handle, unsigned long);
Handle DoExp1to6( Handle, unsigned long);
Handle SndToHandle( Handle, short   *);
#endif

}  // namespace antares

#endif // ANTARES_MUSIC_HPP_
