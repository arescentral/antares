/*
Ares, a tactical space combat game.
Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

// Music.h

#pragma options align=mac68k

#define	kTitleSongID		4001
#define	kPlaySongID			4000
#define	kMusicVolume		54
#define	kMaxMusicVolume		64

int MusicInit( void);
void MusicCleanup( void);
void PlaySong( void);
void ToggleSong( void);
Boolean SongIsPlaying( void);
void StopAndUnloadSong( void);
void LoadSong( short);
long GetSongVolume( void);
void SetSongVolume( long);

void DoConversionS3M( Str255, short);

unsigned char *pStrcat(unsigned char *, unsigned char *);
Handle DoExp1to3( Handle, unsigned long);
Handle DoExp1to6( Handle, unsigned long);
void ConvertInstrument( register	Byte	*,	register long);
Handle SndToHandle( Handle, short	*);

#pragma options align=reset
