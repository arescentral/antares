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

#include "Processor.h"

#pragma options align=mac68k

// #define WORLD_WIDTH     800//640
// #define WORLD_HEIGHT    600//480

//
// #define CLIP_LEFT               128
// #define CLIP_TOP                0
// #define CLIP_RIGHT              768//608
// #define CLIP_BOTTOM             WORLD_HEIGHT
//
// #define kPlayScreenSize         WORLD_HEIGHT
// #define kPlayScreenWidth        kPlayScreenSize
// #define kPlayScreenHeight       kPlayScreenSize
//

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

int CreateOffscreenWorld ( Rect *, CTabHandle);
void CleanUpOffscreenWorld( void);
void DrawInRealWorld( void);
void DrawInOffWorld ( void);
void DrawInSaveWorld ( void);
void EraseOffWorld( void);
void EraseSaveWorld( void);
void CopyOffWorldToRealWorld ( WindowPtr, Rect *);
void CopyRealWorldToSaveWorld( WindowPtr, Rect *);
void CopyRealWorldToOffWorld( WindowPtr port, Rect *bounds);
void CopySaveWorldToOffWorld( Rect *);
void CopyOffWorldToSaveWorld( Rect *);
void NormalizeColors( void);
void SetBlackBack( void);
void GWorldExperiment ( void);

#ifdef kDontDoLong68KAssem
void ChunkCopyPixMapToScreenPixMap( PixMap *, Rect *, PixMap *);
#else
void asm ChunkCopyPixMapToScreenPixMap( PixMap *, Rect *, PixMap *);
#endif

void ChunkCopyPixMapToPixMap( PixMap *, Rect *, PixMap *);
void AsmChunkCopyPixMapToPixMap( PixMap *, Rect *, PixMap *);
void SetWindowPaletteFromClut( CWindowPtr, CTabHandle);
void ColorTest( void);
void ChunkErasePixMap( PixMap *, Rect *);

#pragma options align=reset

#endif // ANTARES_OFFSCREEN_GWORLD_HPP_
