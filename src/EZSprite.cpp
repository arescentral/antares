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

#ifndef __CONDITIONALMACROS__
#include "ConditionalMacros.h"
#endif // __CONDITIONALMACROS__

#if TARGET_OS_WIN32

    #ifndef __QUICKTIMEVR__
    #include <QuickTimeVR.h>
    #endif

    #ifndef __QTUtilities__
    #include "QTUtilities.h"
    #endif

    #ifndef __QTVRUtilities__
    #include "QTVRUtilities.h"
    #endif

    #include <TextUtils.h>
    #include <Script.h>
    #include <string.h>
#endif // TARGET_OS_WIN32

#include "Resources.h"

#include "EZSprite.h"

#ifndef kQDOffscreen
#include <QDOffscreen.h>
#define kQDOffscreen
#endif

#include "Error.h"
#include "OffscreenGWorld.h"
#include "Debug.h"
#ifndef kSpriteHandling
#include "SpriteHandling.h"
#endif
#include "NatePixTable.h"
#ifndef kNateDraw
#include "NateDraw.h"
#endif
#include "HandleHandling.h"

extern  WindowPtr       gTheWindow;
extern  PixMapHandle    thePixMapHandle;
extern  GDHandle        theDevice;
extern  GWorldPtr       gOffWorld, gRealWorld, gSaveWorld;
extern  long            gNatePortLeft, gNatePortTop;

void EZDrawSpriteOffByID( short resID, long whichShape, long scale, unsigned char color,
    Rect *bounds)
{
    GrafPtr             oldPort;
    Handle              spriteTable = nil;
    spritePix           aSpritePix;
    PixMapHandle        offPixBase = GetGWorldPixMap( gOffWorld);
    char                *pixData;

    GetPort( &oldPort);
    EZMakeSpriteFromID( resID, &spriteTable, &aSpritePix, &pixData, whichShape);
    if ( spriteTable == nil) return;

    if ( color != 0) ColorizeNatePixTableColor( spriteTable, color);
    else RemapNatePixTableColor( spriteTable);

    DrawInOffWorld();
    NormalizeColors();
    EZDrawSpriteCenteredInRectBySprite( &aSpritePix, offPixBase, scale, bounds);

    ReleaseResource( spriteTable);

    MacSetPort( oldPort);
}

/* EZDrawSpriteOffToOnByID
    Draws sprite from offworld to on, given res id, shape, scale, color, and
    bounding box. Centers in and clips to bounding box.

    The sprite data is released before this returns. Not for animation.
*/

void EZDrawSpriteOffToOnByID( short resID, long whichShape, long scale,
    unsigned char color, Rect *bounds)
{
    GrafPtr             oldPort;
    Handle              spriteTable = nil;
    spritePix           aSpritePix;
    PixMapHandle        offPixBase = GetGWorldPixMap( gOffWorld);
    char                *pixData;

    GetPort( &oldPort);
    EZMakeSpriteFromID( resID, &spriteTable, &aSpritePix, &pixData, whichShape);
    if ( spriteTable == nil) return;

    if ( color != 0) ColorizeNatePixTableColor( spriteTable, color);
    else RemapNatePixTableColor( spriteTable);

    DrawInOffWorld();
    NormalizeColors();
    EZDrawSpriteCenteredInRectBySprite( &aSpritePix, offPixBase, scale, bounds);

    ReleaseResource( spriteTable);

    DrawInRealWorld();
    CopyOffWorldToRealWorld( gTheWindow, bounds);

    MacSetPort( oldPort);
}

void EZDrawSpriteCenteredInRectBySprite( spritePix *aSpritePix,
    PixMapHandle pixBase, long thisScale, Rect *bounds)
{
    coordPointType      coord;
    long                tlong;
    Point               where;
    longRect            dRect, spriteRect;

    dRect.left = bounds->left;
    dRect.right = bounds->right;
    dRect.top = bounds->top;
    dRect.bottom = bounds->bottom;

    coord.h = aSpritePix->center.h;
    coord.h *= thisScale;
    coord.h >>= SHIFT_SCALE;
    tlong = aSpritePix->width;
    tlong *= thisScale;
    tlong >>= SHIFT_SCALE;
    where.h = ( (bounds->right - bounds->left) / 2) - ( tlong / 2);
    where.h += dRect.left + coord.h;

    coord.v = aSpritePix->center.v;
    coord.v *= thisScale;
    coord.v >>= SHIFT_SCALE;
    tlong = aSpritePix->height;
    tlong *= thisScale;
    tlong >>= SHIFT_SCALE;
    where.v = ( (bounds->bottom - bounds->top) / 2) - ( tlong / 2);
    where.v += dRect.top + coord.v;


    // draw the sprite

    OptScaleSpritePixInPixMap( aSpritePix, where, thisScale,
            &spriteRect, &dRect, pixBase);
}

/* EZMakeSpriteFromID
    Given resID, loads resource into spriteTable and fills out aSpritePix.
    spriteTable is locked and unlocking it invalidates aSpritePix->pixData.
    Note that, unfortunately, you have to keep the pixData ptr alive.
*/
void EZMakeSpriteFromID( short resID, Handle *spriteTable, spritePix *aSpritePix,
    char **pixData, long whichShape)
{
    *spriteTable = GetResource( kPixResType, resID);
    if ( *spriteTable == nil) return;
    HLock( *spriteTable);

    *pixData = GetNatePixTableNatePixData( *spriteTable, whichShape);

    aSpritePix->data = pixData;
    aSpritePix->center.h = GetNatePixTableNatePixHRef( *spriteTable, whichShape);
    aSpritePix->center.v = GetNatePixTableNatePixVRef( *spriteTable, whichShape);
    aSpritePix->width = GetNatePixTableNatePixWidth( *spriteTable, whichShape);
    aSpritePix->height = GetNatePixTableNatePixHeight( *spriteTable, whichShape);
}

void DrawAnySpriteOffToOn( short resID, long whichShape, long scale, unsigned char color,
    Rect *bounds)
{
    Handle              spriteTable = nil;
    PixMapHandle        offPixBase = GetGWorldPixMap( gOffWorld);
    spritePix           aSpritePix;
    char                *pixData;
    Point               where;
    long                tlong, thisScale;
    longRect            dRect, spriteRect;
    coordPointType      coord;
    GrafPtr             oldPort;

    GetPort( &oldPort);
    mWriteDebugString("\pOpening:");
    WriteDebugLong( resID);
    spriteTable = GetResource( kPixResType, resID);
    if ( spriteTable == nil) return;
    HLock( spriteTable);

    if ( color != 0) ColorizeNatePixTableColor( spriteTable, color);
    else RemapNatePixTableColor( spriteTable);

    DrawInOffWorld();
    NormalizeColors();

    // set up the sprite

    dRect.left = bounds->left;
    dRect.right = bounds->right;
    dRect.top = bounds->top;
    dRect.bottom = bounds->bottom;

    pixData = GetNatePixTableNatePixData( spriteTable, whichShape);

    aSpritePix.data = &pixData;
    aSpritePix.center.h = GetNatePixTableNatePixHRef( spriteTable, whichShape);
    aSpritePix.center.v = GetNatePixTableNatePixVRef( spriteTable, whichShape);
    aSpritePix.width = GetNatePixTableNatePixWidth( spriteTable, whichShape);
    aSpritePix.height = GetNatePixTableNatePixHeight( spriteTable, whichShape);

    thisScale = scale;//SCALE_SCALE;
    // calculate the correct position

    coord.h = aSpritePix.center.h;
    coord.h *= thisScale;
    coord.h >>= SHIFT_SCALE;
    tlong = aSpritePix.width;
    tlong *= thisScale;
    tlong >>= SHIFT_SCALE;
    where.h = ( (bounds->right - bounds->left) / 2) - ( tlong / 2);
    where.h += dRect.left + coord.h;

    coord.v = aSpritePix.center.v;
    coord.v *= thisScale;
    coord.v >>= SHIFT_SCALE;
    tlong = aSpritePix.height;
    tlong *= thisScale;
    tlong >>= SHIFT_SCALE;
    where.v = ( (bounds->bottom - bounds->top) / 2) - ( tlong / 2);
    where.v += dRect.top + coord.v;


    // draw the sprite

    OptScaleSpritePixInPixMap( &aSpritePix, where, thisScale,
            &spriteRect, &dRect, offPixBase);

    // clean up the sprite

    ReleaseResource( spriteTable);

    DrawInRealWorld();
    CopyOffWorldToRealWorld( gTheWindow, bounds);

    MacSetPort( oldPort);
}
