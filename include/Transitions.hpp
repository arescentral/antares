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

#ifndef ANTARES_TRANSITIONS_HPP_
#define ANTARES_TRANSITIONS_HPP_

/* Transitions.h */
#include <Palettes.h>

#pragma options align=mac68k

void InitTransitions( void);
void ResetTransitions( void);
void CleanupTransitions( void);
void StartColorAnimation( long, long, unsigned char);
void UpdateColorAnimation( long);
void StartBooleanColorAnimation( long, long, unsigned char);
void UpdateBooleanColorAnimation( long);
void RestoreOriginalColors( void);
void InstantGoalTransition( void);
Boolean AutoFadeTo( long, RGBColor *, Boolean);
Boolean AutoFadeFrom( long, Boolean);
Boolean AutoMusicFadeTo( long, RGBColor *, Boolean);
Boolean CustomPictFade( long, long, short, short, WindowPtr);
Boolean StartCustomPictFade( long, long, short, short, WindowPtr, PaletteHandle *,
    PaletteHandle *, CTabHandle *, Boolean);
Boolean EndCustomPictFade( WindowPtr, PaletteHandle *, PaletteHandle *, CTabHandle *,
    Boolean);
#pragma options align=reset

#endif // ANTARES_TRANSITIONS_HPP_
