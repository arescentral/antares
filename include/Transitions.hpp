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

#ifndef ANTARES_TRANSITIONS_HPP_
#define ANTARES_TRANSITIONS_HPP_

// Transitions.h
#include <Palettes.h>

#pragma options align=mac68k

STUB0(InitTransitions, void( void));
STUB0(ResetTransitions, void( void));
STUB0(CleanupTransitions, void( void));
STUB3(StartColorAnimation, void( long, long, unsigned char));
STUB1(UpdateColorAnimation, void( long));
STUB3(StartBooleanColorAnimation, void( long, long, unsigned char));
STUB1(UpdateBooleanColorAnimation, void( long));
STUB0(RestoreOriginalColors, void( void));
STUB0(InstantGoalTransition, void( void));
STUB3(AutoFadeTo, Boolean( long, RGBColor *, Boolean), true);
STUB2(AutoFadeFrom, Boolean( long, Boolean), true);
STUB3(AutoMusicFadeTo, Boolean( long, RGBColor *, Boolean), true);
STUB5(CustomPictFade, Boolean( long, long, short, short, WindowPtr), true);
inline bool StartCustomPictFade( long, long, short, short, WindowPtr, PaletteHandle *,
    PaletteHandle *, CTabHandle *, Boolean) { return true; }
STUB5(EndCustomPictFade, Boolean( WindowPtr, PaletteHandle *, PaletteHandle *, CTabHandle *,
    Boolean), true);
#pragma options align=reset

#endif // ANTARES_TRANSITIONS_HPP_
