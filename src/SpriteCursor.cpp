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

#include "SpriteCursor.hpp"

#include <QDOffscreen.h>

#include "ColorTranslation.hpp"
#include "ConditionalMacros.h"
#include "Error.hpp"
#include "NatePixTable.hpp"
#include "OffscreenGWorld.hpp"
#include "Resources.h"

extern  GWorldPtr       gOffWorld, gRealWorld, gSaveWorld;
extern  PixMapHandle    thePixMapHandle;
extern  long            WORLD_WIDTH, WORLD_HEIGHT, gNatePortLeft, gNatePortTop;

spriteCursorType    *gSpriteCursor = nil;

long    gFlashingLineColor = 3;

short InitSpriteCursor( void)

{
    gSpriteCursor = new spriteCursorType;
    if ( gSpriteCursor == nil)
    {
        return( MEMORY_ERROR);
    }

    ResetSpriteCursor();
    return( kNoError);
}

void CleanupSpriteCursor( void)

{
    if (gSpriteCursor != nil) {
        delete gSpriteCursor;
    }
}

void ResetSpriteCursor( void)

{
    gSpriteCursor->where.h = gSpriteCursor->where.v = 0;
    gSpriteCursor->lastWhere.h = gSpriteCursor->lastWhere.v = 0;
    gSpriteCursor->sprite.thisRect.left = gSpriteCursor->sprite.thisRect.top = 0;
    gSpriteCursor->sprite.thisRect.right = gSpriteCursor->sprite.thisRect.bottom = -1;
    gSpriteCursor->sprite.lastRect.left = gSpriteCursor->sprite.lastRect.top = 0;
    gSpriteCursor->sprite.lastRect.right = gSpriteCursor->sprite.lastRect.bottom = -1;
    gSpriteCursor->sprite.where = gSpriteCursor->where;
    gSpriteCursor->sprite.whichShape = 0;
    gSpriteCursor->sprite.scale = SCALE_SCALE;
    gSpriteCursor->sprite.style = spriteNormal;
    gSpriteCursor->sprite.styleColor = 0;
    gSpriteCursor->sprite.styleData = 0;
    gSpriteCursor->sprite.tinySize = 0;
    gSpriteCursor->sprite.whichLayer = 0;
    gSpriteCursor->sprite.tinyColor = 0;
    gSpriteCursor->sprite.killMe = false;

    gSpriteCursor->showLevel = kSpriteCursorHidden;

    gSpriteCursor->thisShowLine = gSpriteCursor->lastShowLine = false;
    gSpriteCursor->thisLineStart.h = gSpriteCursor->thisLineStart.v =
        gSpriteCursor->thisLineEnd.h = gSpriteCursor->thisLineEnd.v =
        gSpriteCursor->lastLineStart.h = gSpriteCursor->lastLineStart.v =
        gSpriteCursor->lastLineEnd.h = gSpriteCursor->lastLineEnd.v = -1;
    gSpriteCursor->thisLineColor = gSpriteCursor->thisLineColorDark = 0;
}

void ShowSpriteCursor( Boolean force)
{
    if ( !force)
        gSpriteCursor->showLevel++;
    else gSpriteCursor->showLevel = kSpriteCursorVisible;
}

void HideSpriteCursor( Boolean force)
{
    if ( !force)
        gSpriteCursor->showLevel--;
    else gSpriteCursor->showLevel = kSpriteCursorHidden;
}

void ShowHideSpriteCursor( Boolean showOrHide)  // true = show
{
    if ( showOrHide) ShowSpriteCursor( true);
    else HideSpriteCursor( true);
}

Boolean SpriteCursorVisible( void)
{
    if ( gSpriteCursor->showLevel >= kSpriteCursorVisible) return( true);
    else return ( false);
}

Boolean SetSpriteCursorTable( short resID)

{
    gSpriteCursor->sprite.table = GetPixTable( resID);
    if (gSpriteCursor->sprite.table.get() == nil) {
        gSpriteCursor->sprite.table = AddPixTable( resID);

        if (gSpriteCursor->sprite.table.get() == nil) {
            return false;
        }
        else return( true);
    } else return( true);
}

void SetSpriteCursorShape( short whichShape)
{
    gSpriteCursor->sprite.whichShape = whichShape;
}

void EraseSpriteCursorSprite( void)

{
    PixMapHandle    savePixBase, offPixBase;

    savePixBase = GetGWorldPixMap( gSaveWorld);
    offPixBase = GetGWorldPixMap( gOffWorld);
    if (gSpriteCursor->sprite.table.get() != nil) {
        if ( gSpriteCursor->sprite.thisRect.left < gSpriteCursor->sprite.thisRect.right)
        {
            ChunkCopyPixMapToPixMap( *savePixBase,
                &(gSpriteCursor->sprite.thisRect), *offPixBase);
            if ( gSpriteCursor->showLevel <= kSpriteCursorHidden)
                gSpriteCursor->sprite.lastRect = gSpriteCursor->sprite.thisRect;
        }
    }
    if ( gSpriteCursor->thisShowLine)
    {
        longRect    tc;

        SetLongRect( &tc, 0, 0, WORLD_WIDTH, WORLD_HEIGHT);

        CopyNateLine( *savePixBase, *offPixBase,
            &tc, gSpriteCursor->thisLineStart.h, gSpriteCursor->thisLineStart.v,
            gSpriteCursor->thisLineEnd.h, gSpriteCursor->thisLineEnd.v,
            0, 0);
        CopyNateLine( *savePixBase, *offPixBase,
            &tc, gSpriteCursor->thisLineStart.h, gSpriteCursor->thisLineStart.v+1,
            gSpriteCursor->thisLineEnd.h, gSpriteCursor->thisLineEnd.v+1,
            0, 0);
        CopyNateLine( *savePixBase, *offPixBase,
            &tc, gSpriteCursor->thisLineStart.h, gSpriteCursor->thisLineStart.v+2,
            gSpriteCursor->thisLineEnd.h, gSpriteCursor->thisLineEnd.v+2,
            0, 0);
    } else if ( gSpriteCursor->lastShowLine)
    {
        longRect    tc;

        SetLongRect( &tc, 0, 0, WORLD_WIDTH, WORLD_HEIGHT);

        CopyNateLine( *savePixBase, *offPixBase,
            &tc, gSpriteCursor->lastLineStart.h, gSpriteCursor->lastLineStart.v,
            gSpriteCursor->lastLineEnd.h, gSpriteCursor->lastLineEnd.v,
            0, 0);
        CopyNateLine( *savePixBase, *offPixBase,
            &tc, gSpriteCursor->lastLineStart.h, gSpriteCursor->lastLineStart.v+1,
            gSpriteCursor->lastLineEnd.h, gSpriteCursor->lastLineEnd.v+1,
            0, 0);
        CopyNateLine( *savePixBase, *offPixBase,
            &tc, gSpriteCursor->lastLineStart.h, gSpriteCursor->lastLineStart.v+2,
            gSpriteCursor->lastLineEnd.h, gSpriteCursor->lastLineEnd.v+2,
            0, 0);
    }
}

void DrawSpriteCursorSprite( longRect *clipRect)

{
    PixMapHandle    pixMap, savePixBase;
    longRect        sRect, tc;
    spritePix       aSpritePix;
    TypedHandle<natePixType> pixTable;
    int             whichShape;
    Rect            tRect;

    pixMap = GetGWorldPixMap( gOffWorld);
    savePixBase = GetGWorldPixMap( gSaveWorld);
    if ((gSpriteCursor->sprite.table.get() != nil) &&
        ( gSpriteCursor->showLevel >= kSpriteCursorVisible))
    {
        MacSetRect( &tRect, gSpriteCursor->where.h - 16, gSpriteCursor->where.v - 16,
            gSpriteCursor->where.h + 16, gSpriteCursor->where.v + 16);
        ChunkCopyPixMapToPixMap( *pixMap, &tRect, *savePixBase);
    }

    if ( gSpriteCursor->thisShowLine)
    {

        SetLongRect( &tc, 0, 0, WORLD_WIDTH, WORLD_HEIGHT);

        CopyNateLine( *pixMap, *savePixBase,
            &tc, gSpriteCursor->thisLineStart.h, gSpriteCursor->thisLineStart.v,
            gSpriteCursor->thisLineEnd.h, gSpriteCursor->thisLineEnd.v,
            0, 0);
        CopyNateLine( *pixMap, *savePixBase,
            &tc, gSpriteCursor->thisLineStart.h, gSpriteCursor->thisLineStart.v+1,
            gSpriteCursor->thisLineEnd.h, gSpriteCursor->thisLineEnd.v+1,
            0, 0);
        CopyNateLine( *pixMap, *savePixBase,
            &tc, gSpriteCursor->thisLineStart.h, gSpriteCursor->thisLineStart.v+2,
            gSpriteCursor->thisLineEnd.h, gSpriteCursor->thisLineEnd.v+2,
            0, 0);
    }

    if ((gSpriteCursor->sprite.table.get() != nil) &&
        ( gSpriteCursor->showLevel >= kSpriteCursorVisible))
    {
        pixTable = gSpriteCursor->sprite.table;
        whichShape = gSpriteCursor->sprite.whichShape;
        aSpritePix.data = GetNatePixTableNatePixData( pixTable, whichShape);
        aSpritePix.center.h = GetNatePixTableNatePixHRef( pixTable, whichShape);
        aSpritePix.center.v = GetNatePixTableNatePixVRef( pixTable, whichShape);
        aSpritePix.width = GetNatePixTableNatePixWidth( pixTable, whichShape);
        aSpritePix.height = GetNatePixTableNatePixHeight( pixTable, whichShape);

        OptScaleSpritePixInPixMap( &aSpritePix, gSpriteCursor->where, SCALE_SCALE,
                &sRect, clipRect, pixMap);
        mCopyAnyRect( gSpriteCursor->sprite.thisRect, sRect);
    } else
    {
        gSpriteCursor->sprite.thisRect.left = gSpriteCursor->sprite.thisRect.top = 0;
        gSpriteCursor->sprite.thisRect.right = gSpriteCursor->sprite.thisRect.bottom = -1;
    }


    if ( gSpriteCursor->thisShowLine)
    {
//      ShowHintLine( gSpriteCursor->thisLineStart, gSpriteCursor->thisLineEnd,
//              SKY_BLUE, gFlashingLineColor);

//      if ( gFlashingLineColor == GREEN) gFlashingLineColor = RED;
//      else if ( gFlashingLineColor == RED) gFlashingLineColor = BLUE;
//      else gFlashingLineColor = GREEN;
        gFlashingLineColor--;
        if ( gFlashingLineColor < 2) gFlashingLineColor = 14;

        DrawNateLine( *pixMap, &tc,
            gSpriteCursor->thisLineStart.h, gSpriteCursor->thisLineStart.v+2,
            gSpriteCursor->thisLineEnd.h, gSpriteCursor->thisLineEnd.v+2,
            0, 0, gSpriteCursor->thisLineColorDark);
        DrawNateLine( *pixMap, &tc,
            gSpriteCursor->thisLineStart.h, gSpriteCursor->thisLineStart.v+1,
            gSpriteCursor->thisLineEnd.h, gSpriteCursor->thisLineEnd.v+1,
            0, 0, gSpriteCursor->thisLineColor);
        DrawNateLine( *pixMap, &tc,
            gSpriteCursor->thisLineStart.h, gSpriteCursor->thisLineStart.v,
            gSpriteCursor->thisLineEnd.h, gSpriteCursor->thisLineEnd.v,
            0, 0, gSpriteCursor->thisLineColor);
    }
}

void ShowSpriteCursorSprite( void)

{
    Rect            tRect;
    PixMapHandle    pixMap;
    longRect        tc;

    SetLongRect( &tc, 0, 0, WORLD_WIDTH, WORLD_HEIGHT);

    pixMap = GetGWorldPixMap( gOffWorld);
    if (gSpriteCursor->sprite.table.get() != nil) {
        // if thisRect is null
        if (( gSpriteCursor->sprite.thisRect.right <=
                gSpriteCursor->sprite.thisRect.left) ||
            ( gSpriteCursor->sprite.thisRect.bottom <=
                gSpriteCursor->sprite.thisRect.top))
        {
            // and lastRect isn't then
            if ( gSpriteCursor->sprite.lastRect.right > gSpriteCursor->sprite.lastRect.left)
            {
                // show lastRect

                ChunkCopyPixMapToScreenPixMap( *pixMap, &(gSpriteCursor->sprite.lastRect),
                        *thePixMapHandle);


            }
        // else if lastRect is null (we now know this rect isn't)
        } else if (( gSpriteCursor->sprite.lastRect.right <= gSpriteCursor->sprite.lastRect.left) ||
            ( gSpriteCursor->sprite.lastRect.bottom <= gSpriteCursor->sprite.lastRect.top))
        {
            // then show thisRect

            ChunkCopyPixMapToScreenPixMap( *pixMap,
                &(gSpriteCursor->sprite.thisRect), *thePixMapHandle);

        // else if the rects don't intersect
        } else if ( ( gSpriteCursor->sprite.lastRect.right <
                        ( gSpriteCursor->sprite.thisRect.left - 32)) ||
                    ( gSpriteCursor->sprite.lastRect.left >
                        ( gSpriteCursor->sprite.thisRect.right + 32)) ||
                    ( gSpriteCursor->sprite.lastRect.bottom <
                        ( gSpriteCursor->sprite.thisRect.top - 32)) ||
                    ( gSpriteCursor->sprite.lastRect.top >
                        ( gSpriteCursor->sprite.thisRect.bottom + 32)))
        {
            // then draw them individually


            ChunkCopyPixMapToScreenPixMap( *pixMap, &(gSpriteCursor->sprite.lastRect),
                    *thePixMapHandle);
            ChunkCopyPixMapToScreenPixMap( *pixMap, &(gSpriteCursor->sprite.thisRect),
                    *thePixMapHandle);

        // else the rects do intersect (and we know are both non-null)
        } else
        {
            tRect = gSpriteCursor->sprite.thisRect;
            mBiggestRect( tRect, gSpriteCursor->sprite.lastRect);

            ChunkCopyPixMapToScreenPixMap( *pixMap, &tRect, *thePixMapHandle);

        }
        gSpriteCursor->sprite.lastRect = gSpriteCursor->sprite.thisRect;
    }
    if ( gSpriteCursor->lastShowLine)
    {
        CopyNateLine( *pixMap, *thePixMapHandle,
            &tc, gSpriteCursor->lastLineStart.h, gSpriteCursor->lastLineStart.v,
            gSpriteCursor->lastLineEnd.h, gSpriteCursor->lastLineEnd.v,
            gNatePortLeft << 2, gNatePortTop);
        CopyNateLine( *pixMap, *thePixMapHandle,
            &tc, gSpriteCursor->lastLineStart.h, gSpriteCursor->lastLineStart.v+1,
            gSpriteCursor->lastLineEnd.h, gSpriteCursor->lastLineEnd.v+1,
            gNatePortLeft << 2, gNatePortTop);
        CopyNateLine( *pixMap, *thePixMapHandle,
            &tc, gSpriteCursor->lastLineStart.h, gSpriteCursor->lastLineStart.v+2,
            gSpriteCursor->lastLineEnd.h, gSpriteCursor->lastLineEnd.v+2,
            gNatePortLeft << 2, gNatePortTop);

    }
    if ( gSpriteCursor->thisShowLine)
    {
        CopyNateLine( *pixMap, *thePixMapHandle,
            &tc, gSpriteCursor->thisLineStart.h, gSpriteCursor->thisLineStart.v,
            gSpriteCursor->thisLineEnd.h, gSpriteCursor->thisLineEnd.v,
            gNatePortLeft << 2, gNatePortTop);
        CopyNateLine( *pixMap, *thePixMapHandle,
            &tc, gSpriteCursor->thisLineStart.h, gSpriteCursor->thisLineStart.v+1,
            gSpriteCursor->thisLineEnd.h, gSpriteCursor->thisLineEnd.v+1,
            gNatePortLeft << 2, gNatePortTop);
        CopyNateLine( *pixMap, *thePixMapHandle,
            &tc, gSpriteCursor->thisLineStart.h, gSpriteCursor->thisLineStart.v+2,
            gSpriteCursor->thisLineEnd.h, gSpriteCursor->thisLineEnd.v+2,
            gNatePortLeft << 2, gNatePortTop);
    }
    gSpriteCursor->lastShowLine = gSpriteCursor->thisShowLine;
    gSpriteCursor->lastLineStart = gSpriteCursor->thisLineStart;
    gSpriteCursor->lastLineEnd = gSpriteCursor->thisLineEnd;
}

void MoveSpriteCursor( Point where)
{
    gSpriteCursor->where = where;
}

void ShowHintLine( Point fromWhere, Point toWhere,
    unsigned char color, unsigned char brightness)
{
    transColorType  *transColor;

    gSpriteCursor->thisLineStart = fromWhere;
    gSpriteCursor->thisLineEnd = toWhere;
    gSpriteCursor->thisShowLine = true;

    mGetTranslateColorShade( color, brightness, gSpriteCursor->thisLineColor, transColor);
    mGetTranslateColorShade( color, VERY_DARK, gSpriteCursor->thisLineColorDark, transColor);

}

void HideHintLine( void)
{
    gSpriteCursor->thisShowLine = false;
}

void ResetHintLine( void)
{
    gSpriteCursor->thisShowLine = gSpriteCursor->lastShowLine = false;
    gSpriteCursor->thisLineStart.h = gSpriteCursor->thisLineStart.v =
        gSpriteCursor->thisLineEnd.h = gSpriteCursor->thisLineEnd.v =
        gSpriteCursor->lastLineStart.h = gSpriteCursor->lastLineStart.v =
        gSpriteCursor->lastLineEnd.h = gSpriteCursor->lastLineEnd.v = -1;
    gSpriteCursor->thisLineColor = gSpriteCursor->thisLineColorDark = 0;
}
