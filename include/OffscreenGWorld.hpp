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

#ifndef ANTARES_OFFSCREEN_GWORLD_HPP_
#define ANTARES_OFFSCREEN_GWORLD_HPP_

#include "Base.h"
#include "Quickdraw.h"

namespace antares {

// these defs are here for historic reason:
#define kLeftPanelWidth         128
#define kRightPanelWidth        32
#define kPanelHeight            480

#define kSmallScreenWidth       640
#define kSmallScreenHeight      480
#define kMediumScreenWidth      800
#define kMediumScreenHeight     600
#define kLargeScreenWidth       1024
#define kLargeScreenHeight      768

int CreateOffscreenWorld(const Rect& bounds, const ColorTable& colors);
void CleanUpOffscreenWorld();
void DrawInRealWorld();
void DrawInOffWorld();
void DrawInSaveWorld();
void EraseOffWorld();
void EraseSaveWorld();
void CopyOffWorldToRealWorld(const Rect& bounds);
void CopyRealWorldToSaveWorld(const Rect& bounds);
void CopyRealWorldToOffWorld(const Rect& bounds);
void CopySaveWorldToOffWorld(const Rect& bounds);
void CopyOffWorldToSaveWorld(const Rect& bounds);
void NormalizeColors();

}  // namespace antares

#endif // ANTARES_OFFSCREEN_GWORLD_HPP_
