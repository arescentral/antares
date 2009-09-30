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

#include "EZSprite.hpp"

#include "ConditionalMacros.h"
#include "Debug.hpp"
#include "Error.hpp"
#include "NateDraw.hpp"
#include "NatePixTable.hpp"
#include "OffscreenGWorld.hpp"
#include "Resources.h"
#include "SpriteHandling.hpp"

extern  WindowPtr       gTheWindow;
extern  PixMap*         gActiveWorld;
extern  PixMap*         gOffWorld;
extern  PixMap*         gRealWorld;
extern  PixMap*         gSaveWorld;
extern  long            gNatePortLeft, gNatePortTop;

void EZDrawSpriteOffByID( short resID, long whichShape, long scale, unsigned char color,
    Rect *bounds)
{
    GrafPtr             oldPort;
    TypedHandle<natePixType> spriteTable;
    spritePix           aSpritePix;

    GetPort( &oldPort);
    EZMakeSpriteFromID( resID, &spriteTable, &aSpritePix, whichShape);
    if (spriteTable.get() == nil) {
        return;
    }

    if ( color != 0) ColorizeNatePixTableColor( spriteTable, color);
    else RemapNatePixTableColor( spriteTable);

    DrawInOffWorld();
    NormalizeColors();
    EZDrawSpriteCenteredInRectBySprite( &aSpritePix, gOffWorld, scale, bounds);

    spriteTable.destroy();

    MacSetPort( oldPort);
}

// EZDrawSpriteOffToOnByID
//  Draws sprite from offworld to on, given res id, shape, scale, color, and
//  bounding box. Centers in and clips to bounding box.
//
//  The sprite data is released before this returns. Not for animation.

void EZDrawSpriteOffToOnByID( short resID, long whichShape, long scale,
    unsigned char color, Rect *bounds)
{
    GrafPtr             oldPort;
    TypedHandle<natePixType> spriteTable;
    spritePix           aSpritePix;

    GetPort( &oldPort);
    EZMakeSpriteFromID( resID, &spriteTable, &aSpritePix, whichShape);
    if (spriteTable.get() == nil) {
        return;
    }

    if ( color != 0) ColorizeNatePixTableColor( spriteTable, color);
    else RemapNatePixTableColor( spriteTable);

    DrawInOffWorld();
    NormalizeColors();
    EZDrawSpriteCenteredInRectBySprite( &aSpritePix, gOffWorld, scale, bounds);

    spriteTable.destroy();

    DrawInRealWorld();
    CopyOffWorldToRealWorld(bounds);

    MacSetPort( oldPort);
}

void EZDrawSpriteCenteredInRectBySprite( spritePix *aSpritePix,
    PixMap* pixBase, long thisScale, Rect *bounds)
{
    coordPointType      coord;
    long                tlong;
    Point               where;
    Rect            dRect, spriteRect;

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

// EZMakeSpriteFromID
//  Given resID, loads resource into spriteTable and fills out aSpritePix.
//  spriteTable is locked and unlocking it invalidates aSpritePix->pixData.

void EZMakeSpriteFromID( short resID, TypedHandle<natePixType>* spriteTable, spritePix *aSpritePix,
    long whichShape)
{
    spriteTable->load_resource(kPixResType, resID);
    if (spriteTable->get() == nil) {
        return;
    }

    aSpritePix->data = GetNatePixTableNatePixData( *spriteTable, whichShape);
    aSpritePix->center.h = GetNatePixTableNatePixHRef( *spriteTable, whichShape);
    aSpritePix->center.v = GetNatePixTableNatePixVRef( *spriteTable, whichShape);
    aSpritePix->width = GetNatePixTableNatePixWidth( *spriteTable, whichShape);
    aSpritePix->height = GetNatePixTableNatePixHeight( *spriteTable, whichShape);
}

void DrawAnySpriteOffToOn( short resID, long whichShape, long scale, unsigned char color,
    Rect *bounds)
{
    TypedHandle<natePixType> spriteTable;
    spritePix           aSpritePix;
    Point               where;
    long                tlong, thisScale;
    Rect            dRect, spriteRect;
    coordPointType      coord;
    GrafPtr             oldPort;

    GetPort( &oldPort);
    mWriteDebugString("\pOpening:");
    WriteDebugLong( resID);
    spriteTable.load_resource(kPixResType, resID);
    if (spriteTable.get() == nil) {
        return;
    }

    if ( color != 0) ColorizeNatePixTableColor( spriteTable, color);
    else RemapNatePixTableColor( spriteTable);

    DrawInOffWorld();
    NormalizeColors();

    // set up the sprite

    dRect.left = bounds->left;
    dRect.right = bounds->right;
    dRect.top = bounds->top;
    dRect.bottom = bounds->bottom;

    aSpritePix.data = GetNatePixTableNatePixData( spriteTable, whichShape);
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
            &spriteRect, &dRect, gOffWorld);

    // clean up the sprite

    spriteTable.destroy();

    DrawInRealWorld();
    CopyOffWorldToRealWorld(bounds);

    MacSetPort( oldPort);
}
