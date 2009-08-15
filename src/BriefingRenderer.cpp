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

// Briefing_Renderer.c

#include "BriefingRenderer.hpp"

#include "Admiral.hpp"
#include "AresGlobalType.hpp"
#include "ColorTranslation.hpp"
#include "Debug.hpp"
#include "Error.hpp"
#include "NateDraw.hpp"
#include "NatePixTable.hpp"
#include "Scenario.hpp"
#include "ScenarioMaker.hpp"
#include "SpriteHandling.hpp"

#define kBriefing_Grid_Size             16

struct briefingSpriteBoundsType {
    Rect    bounds;
    long    objectIndex;
};

extern aresGlobalType*      gAresGlobal;
extern spaceObjectType**    gSpaceObjectData;
extern scenarioType*        gThisScenario;

briefingSpriteBoundsType    *gBriefingSpriteBounds = nil;

Point BriefingSprite_GetBestLocation( spritePix *sprite, long scale,
    Point fromWhere, Boolean *grid, long gridWidth, long gridHeight,
    Rect *bounds);

void GetInitialObjectSpriteData( long whichScenario, long whichObject,
        long maxSize, Rect *bounds, coordPointType *corner,
        long scale, long *thisScale, spritePix *aSpritePix, Point *where,
        longRect *spriteRect);

void GetRealObjectSpriteData( coordPointType *realCoord, baseObjectType *baseObject,
        long owner, long spriteOverride, long maxSize, Rect *bounds, coordPointType *corner,
        long scale, long *thisScale, spritePix *aSpritePix, Point *where,
        longRect *spriteRect);

void SpriteBounds_Get( spritePix *sprite, Point where, long scale,
    Rect *bounds);

Boolean BriefingSprite_IsLocationLegal( spritePix *sprite, long scale,
    Point where, Boolean *grid, long gridWidth, long gridHeight,
    Rect *bounds);

void BriefingSprite_UseLocation( spritePix *sprite, long scale,
    Point where, Boolean *grid, long gridWidth, long gridHeight,
    Rect *bounds);

Boolean Briefing_Grid_Get( Boolean *grid, long x, long y, long gridWidth,
    long gridHeight);

void Briefing_Grid_Set( Boolean *grid, long x, long y, long gridWidth,
    long gridHeight, Boolean value);


Point BriefingSprite_GetBestLocation( spritePix *sprite, long scale,
    Point fromWhere, Boolean *grid, long gridWidth, long gridHeight,
    Rect *bounds)
{
    long            offsetSize = 1, i;
    Point           result = fromWhere;

    if ( BriefingSprite_IsLocationLegal( sprite, scale, result, grid,
            gridWidth, gridHeight, bounds)) return result;

    while ( offsetSize < gridWidth)
    {
        for ( i = -offsetSize; i <= offsetSize; i++)
        {
            // try left
            result.h = fromWhere.h - (offsetSize * kBriefing_Grid_Size);
            result.v = fromWhere.v + (i * kBriefing_Grid_Size);
            if ( BriefingSprite_IsLocationLegal( sprite, scale, result, grid,
                    gridWidth, gridHeight, bounds)) return result;

            // try right
            result.h = fromWhere.h + (offsetSize * kBriefing_Grid_Size);
            result.v = fromWhere.v + (i * kBriefing_Grid_Size);
            if ( BriefingSprite_IsLocationLegal( sprite, scale, result, grid,
                    gridWidth, gridHeight, bounds)) return result;

            // try top
            result.h = fromWhere.h + (i * kBriefing_Grid_Size);
            result.v = fromWhere.v - (offsetSize * kBriefing_Grid_Size);
            if ( BriefingSprite_IsLocationLegal( sprite, scale, result, grid,
                    gridWidth, gridHeight, bounds)) return result;

            // try bottom
            result.h = fromWhere.h + (i * kBriefing_Grid_Size);
            result.v = fromWhere.v + (offsetSize * kBriefing_Grid_Size);
            if ( BriefingSprite_IsLocationLegal( sprite, scale, result, grid,
                    gridWidth, gridHeight, bounds)) return result;
        }
        offsetSize++;
    }
    result = fromWhere;
    return result;
}

Boolean BriefingSprite_IsLocationLegal( spritePix *sprite, long scale,
    Point where, Boolean *grid, long gridWidth, long gridHeight,
    Rect *bounds)
{
    Rect    spriteBounds;
    long    i, j;

    SpriteBounds_Get( sprite, where, scale, &spriteBounds);
    OffsetRect( &spriteBounds, -bounds->left, -bounds->top);
    spriteBounds.left /= kBriefing_Grid_Size;
    spriteBounds.right /= kBriefing_Grid_Size;
    spriteBounds.top /= kBriefing_Grid_Size;
    spriteBounds.bottom /= kBriefing_Grid_Size;
    if ( spriteBounds.left < 1) return false;
    if ( spriteBounds.right >= gridWidth) return false;
    if ( spriteBounds.top < 1) return false;
    if ( spriteBounds.bottom >= gridHeight) return false;

    for ( j = spriteBounds.top; j <= spriteBounds.bottom; j++)
    {
        for ( i = spriteBounds.left; i <= spriteBounds.right; i++)
        {
            if ( Briefing_Grid_Get( grid, i, j, gridWidth, gridHeight))
                return false;
        }
    }
    return true;
}

void BriefingSprite_UseLocation( spritePix *sprite, long scale,
    Point where, Boolean *grid, long gridWidth, long gridHeight,
    Rect *bounds)
{
    Rect    spriteBounds;
    long    i, j;

    SpriteBounds_Get( sprite, where, scale, &spriteBounds);
    OffsetRect( &spriteBounds, -bounds->left, -bounds->top);
    spriteBounds.left /= kBriefing_Grid_Size;
    spriteBounds.right /= kBriefing_Grid_Size;
    spriteBounds.top /= kBriefing_Grid_Size;
    spriteBounds.bottom /= kBriefing_Grid_Size;
    if ( spriteBounds.left < 1) return;
    if ( spriteBounds.right >= gridWidth) return;
    if ( spriteBounds.top < 1) return;
    if ( spriteBounds.bottom >= gridHeight) return;

    for ( j = spriteBounds.top; j <= spriteBounds.bottom; j++)
    {
        for ( i = spriteBounds.left; i <= spriteBounds.right; i++)
        {
            Briefing_Grid_Set( grid, i, j, gridWidth, gridHeight, true);
        }
    }
}

Boolean Briefing_Grid_Get( Boolean *grid, long x, long y, long gridWidth,
    long gridHeight)
{
    if ( grid == nil) return true;
    if ( x < 1) return true;
    if ( x >= gridWidth) return true;
    if ( y < 1) return true;
    if ( y >= gridHeight) return true;

    grid = grid + (y * gridWidth) + x;
    return *grid;
}

void Briefing_Grid_Set( Boolean *grid, long x, long y, long gridWidth,
    long gridHeight, Boolean value)
{
    if ( grid == nil) return;
    if ( x < 1) return;
    if ( x >= gridWidth) return;
    if ( y < 1) return;
    if ( y >= gridHeight) return;

    grid = grid + (y * gridWidth) + x;
    *grid = value;
}

void GetInitialObjectSpriteData( long whichScenario, long whichObject,
        long maxSize, Rect *bounds, coordPointType *corner,
        long scale, long *thisScale, spritePix *aSpritePix, Point *where,
        longRect *spriteRect)

{
    spaceObjectType         *sObject = nil;
    scenarioInitialType     *initial;
    scenarioType            *scenario = reinterpret_cast<scenarioType *>(*gAresGlobal->gScenarioData) + whichScenario;
    briefingSpriteBoundsType    *sBounds = gBriefingSpriteBounds;

    initial = mGetScenarioInitial( scenario, whichObject);

/*  if (initial->attributes & kInitiallyHidden)
    {
        aSpritePix->data = nil;
        return;
    }
*/
/*  mGetRealObjectFromInitial( sObject, initial, whichObject)
    if ( sObject != nil)
    {
        GetRealObjectSpriteData( &(sObject->location), sObject->baseType, sObject->owner,
            sObject->pixResID, maxSize, bounds, corner, scale, thisScale,
            aSpritePix, where, spriteRect);
    } else
    {
        aSpritePix->data = nil;
        return;
    }
*/
    spriteRect->right = spriteRect->left = -1;
    aSpritePix->data = nil;

    mGetRealObjectFromInitial( sObject, initial, whichObject);

    if ( sObject != nil)
    {
        // this old thing just fills in the aSpritePix correctly
        GetRealObjectSpriteData( &(sObject->location), sObject->baseType, sObject->owner,
            sObject->pixResID, maxSize, bounds, corner, scale, thisScale,
            aSpritePix, where, spriteRect);

        if ( sBounds == nil) return;
        while ( (sBounds->objectIndex >= 0) &&
            ( sBounds->objectIndex != sObject->entryNumber)) sBounds++;

        if ( sBounds->objectIndex < 0)
        {
            SysBeep( 20);
            return;
        }
        RectToLongRect( &sBounds->bounds, spriteRect);
    } else
    {
        aSpritePix->data = nil;
    }
}

void GetRealObjectSpriteData( coordPointType *realCoord,
        baseObjectType *baseObject,
        long owner, long spriteOverride, long maxSize, Rect *bounds,
        coordPointType *corner,
        long scale, long *thisScale, spritePix *aSpritePix, Point *where,
        longRect *spriteRect)
{
    Handle          pixTable;
    int             whichShape;
    long            tlong;
    coordPointType  coord = *realCoord;


    if ( spriteOverride == -1)
    {
        tlong = baseObject->pixResID;
        if ( baseObject->attributes & kCanThink)
        {
            pixTable = GetPixTable( tlong +
                (implicit_cast<short>(GetAdmiralColor( owner)) << kSpriteTableColorShift));
        } else
        {
            pixTable = GetPixTable( tlong);
        }
    } else
    {
        tlong = spriteOverride;
        pixTable = GetPixTable( tlong);
    }


    if ( pixTable == nil)
    {
        ShowErrorAny( eContinueErr, kErrorStrID, nil, nil, nil, nil, kLoadSpriteError, -1, -1, -1, __FILE__, tlong);
    }

    if ( baseObject->attributes & kIsSelfAnimated)
        whichShape = baseObject->frame.animation.firstShape >> kFixedBitShiftNumber;
    else
        whichShape = 0;

    //  for archaic reasons, aSpritePix->data wants a ptr to a ptr (a Handle)
    //  but we only have a ptr.  Thus we pass back a ptr in the ->data field.  It must be altered to be a ptr to a ptr
    //  before it is used in any sprite drawing routines.

    aSpritePix->data = reinterpret_cast<char **>(GetNatePixTableNatePixData( pixTable, whichShape));
    aSpritePix->center.h = GetNatePixTableNatePixHRef( pixTable, whichShape);
    aSpritePix->center.v = GetNatePixTableNatePixVRef( pixTable, whichShape);
    aSpritePix->width = GetNatePixTableNatePixWidth( pixTable, whichShape);
    aSpritePix->height = GetNatePixTableNatePixHeight( pixTable, whichShape);

    tlong = *thisScale = implicit_cast<long>(maxSize) * SCALE_SCALE;
    *thisScale /= aSpritePix->width;
    tlong /= aSpritePix->height;

    if ( tlong < *thisScale) *thisScale = tlong;

    coord.h = coord.h - corner->h;
    coord.h *= scale;
    coord.h >>= SHIFT_SCALE;
    coord.h += bounds->left;

    coord.v = coord.v - corner->v;
    coord.v *= scale;
    coord.v >>= SHIFT_SCALE;
    coord.v += bounds->top;

    where->h = coord.h;
    where->v = coord.v;

    spriteRect->left = aSpritePix->center.h;
    spriteRect->left *= *thisScale;
    spriteRect->left >>= SHIFT_SCALE;
    spriteRect->left = where->h - spriteRect->left;

    spriteRect->right = aSpritePix->width;
    spriteRect->right *= *thisScale;
    spriteRect->right >>= SHIFT_SCALE;
    spriteRect->right = spriteRect->left + spriteRect->right;

    spriteRect->top = aSpritePix->center.v;
    spriteRect->top *= *thisScale;
    spriteRect->top >>= SHIFT_SCALE;
    spriteRect->top = where->v - spriteRect->top;

    spriteRect->bottom = aSpritePix->height;
    spriteRect->bottom *= *thisScale;
    spriteRect->bottom >>= SHIFT_SCALE;
    spriteRect->bottom = spriteRect->top + spriteRect->bottom;

}

void SpriteBounds_Get( spritePix *sprite, Point where, long scale,
    Rect *bounds)
{
    long    tlong;

    tlong = sprite->center.h;
    tlong *= scale;
    tlong >>= SHIFT_SCALE;
    tlong = where.h - tlong;
    bounds->left = tlong;

    tlong = sprite->width;
    tlong *= scale;
    tlong >>= SHIFT_SCALE;
    tlong = bounds->left + tlong;
    bounds->right = tlong;

    tlong = sprite->center.v;
    tlong *= scale;
    tlong >>= SHIFT_SCALE;
    tlong = where.v - tlong;
    bounds->top = tlong;

    tlong = sprite->height;
    tlong *= scale;
    tlong >>= SHIFT_SCALE;
    tlong = bounds->top + tlong;
    bounds->bottom = tlong;
}

void Briefing_Objects_Render( long whichScenario, PixMapHandle destmap,
            long maxSize, Rect *bounds, long portleft, long portright,
            coordPointType *corner, long scale)

{
    long        count, thisScale, gridWidth, gridHeight, i, j, color,
                objectNum;
    Point       where;
    spritePix   aSpritePix;
    longRect    spriteRect, clipRect;
    scenarioType    *scenario = reinterpret_cast<scenarioType *>(*gAresGlobal->gScenarioData) + whichScenario;
    baseObjectType  *baseObject = nil;
    char            *pixData;
    spaceObjectType *anObject = *gSpaceObjectData;
    Boolean         *gridCells = nil;
    briefingSpriteBoundsType    *sBounds = nil;

#pragma unused( portleft, portright)

    gridWidth = (bounds->right - bounds->left) / kBriefing_Grid_Size;
    gridHeight = (bounds->bottom - bounds->top) / kBriefing_Grid_Size;

    gridCells = reinterpret_cast<Boolean *>(NewPtr( sizeof( Boolean) * gridWidth * gridHeight));

    if ( gridCells == nil) return;
    for ( j = 0; j < gridHeight; j++)
    {
        for ( i = 0; i < gridWidth; i++)
        {
            Briefing_Grid_Set( gridCells, i, j, gridWidth, gridHeight, false);
        }
    }

    if ( gBriefingSpriteBounds != nil)
    {
        DisposePtr( reinterpret_cast<Ptr>(gBriefingSpriteBounds));
    }

    objectNum = 1;  // extra 1 for last null briefingSpriteBounds

    for ( count = 0; count < kMaxSpaceObject; count++)
    {
        if (( anObject->active == kObjectInUse) && ( anObject->sprite != nil))
        {
            objectNum++;
        }
    }

    gBriefingSpriteBounds = reinterpret_cast<briefingSpriteBoundsType *>(NewPtr(
        sizeof( briefingSpriteBoundsType) * objectNum));

    if ( gBriefingSpriteBounds == nil) return;
    sBounds = gBriefingSpriteBounds;

    for ( count = 0; count < kMaxSpaceObject; count++)
    {
        if (( anObject->active == kObjectInUse) && ( anObject->sprite != nil))
        {
            baseObject = anObject->baseType;
            if ( baseObject->maxVelocity == 0)
            {
                GetRealObjectSpriteData( &(anObject->location),
                    anObject->baseType, anObject->owner,
                    anObject->pixResID, maxSize, bounds, corner, scale,
                    &thisScale, &aSpritePix, &where, &spriteRect);
                if ( aSpritePix.data != nil)
                {
//                  thisScale = kOneHalfScale;
                    thisScale = (kOneQuarterScale * baseObject->naturalScale)
                                >> SHIFT_SCALE,
                    RectToLongRect( bounds, &clipRect);

                    pixData = reinterpret_cast<char *>(aSpritePix.data);
                    aSpritePix.data = &pixData;
                    clipRect.left = clipRect.top = 0;
                    clipRect.right -= 1;
                    clipRect.bottom -= 1;

                    where = BriefingSprite_GetBestLocation( &aSpritePix,
                                thisScale,
                                where, gridCells, gridWidth, gridHeight,
                                bounds);

                    BriefingSprite_UseLocation(  &aSpritePix, thisScale, where,
                        gridCells, gridWidth, gridHeight, bounds);

                    if ( anObject->owner == 0) color = GREEN;
                    else if ( anObject->owner < 0) color = BLUE;
                    else color = RED;

                    OptScaleSpritePixInPixMap( &aSpritePix, where,
//                          (kOneQuarterScale * baseObject->naturalScale) >> SHIFT_SCALE,
                            thisScale,
                            &spriteRect, &clipRect, destmap);

                    LongRectToRect( &spriteRect, &sBounds->bounds);
                    sBounds->objectIndex = count;
                    sBounds++;
                }

            } else
            {

                GetRealObjectSpriteData( &(anObject->location), anObject->baseType, anObject->owner,
                    anObject->pixResID, maxSize / 2, bounds, corner, scale, &thisScale,
                    &aSpritePix, &where, &spriteRect);
                if ( aSpritePix.data != nil)
                {
//                  thisScale = kOneQuarterScale;
//                  if ( anObject->attributes & kCanThink)
//                      thisScale = kOneHalfScale;
                    thisScale = (kOneQuarterScale * baseObject->naturalScale)
                        >> SHIFT_SCALE,

                    RectToLongRect( bounds, &clipRect);

                    pixData = reinterpret_cast<char *>(aSpritePix.data);
                    aSpritePix.data = &pixData;
                    clipRect.left = clipRect.top = 0;
                    clipRect.right -= 1;
                    clipRect.bottom -= 1;
                    where = BriefingSprite_GetBestLocation( &aSpritePix,
                                thisScale,
                                where, gridCells, gridWidth, gridHeight,
                                bounds);
                    BriefingSprite_UseLocation(  &aSpritePix, thisScale, where,
                        gridCells, gridWidth, gridHeight, bounds);

                    if ( anObject->owner == 0) color = GREEN;
                    else if ( anObject->owner < 0) color = BLUE;
                    else color = RED;

                    OutlineScaleSpritePixInPixMap( &aSpritePix, where,
//                          (kOneQuarterScale * baseObject->naturalScale) >> SHIFT_SCALE,
                            //(kOneHalfScale * baseObject->naturalScale) >> SHIFT_SCALE,
                            thisScale,
                            &spriteRect, &clipRect, destmap
                            ,
                            GetTranslateColorShade( color, LIGHT),
                            GetTranslateColorShade( color, DARK)
                            );

                    LongRectToRect( &spriteRect, &sBounds->bounds);
                    sBounds->objectIndex = count;
                    sBounds++;
                }
            }
        }
        anObject++;
    }
    sBounds->objectIndex = -1;
    if ( gridCells != nil) DisposePtr( reinterpret_cast<Ptr>(gridCells));
}

void BriefPoint_Data_Get( long whichPoint, long whichScenario, long *headerID,
                    long *headerNumber, long *contentID, Rect *hiliteBounds,
                    coordPointType *corner, long scale, long minSectorSize, long maxSize,
                    Rect *bounds)

{
    scenarioType    *scenario = reinterpret_cast<scenarioType *>(*gAresGlobal->gScenarioData) + whichScenario;
    Point           where;
    spritePix       aSpritePix;
    longRect        spriteRect;
    long            thisScale;
    briefPointType  *brief = mGetScenarioBrief( scenario, whichPoint);

#pragma unused( minSectorSize)
    hiliteBounds->right = hiliteBounds->left = 0;
    if ( brief->briefPointKind == kBriefFreestandingKind)
    {
/*      initial = mGetScenarioInitial( scenario, brief->briefPointData.objectBriefType.objectNum);
        coord.h = kUniversalCenter;// + initial->location.h;
        coord.v = kUniversalCenter;// + initial->location.v;

        GetArbitrarySingleSectorBounds( corner, &coord, scale, minSectorSize, bounds,
                    hiliteBounds);
*/
        hiliteBounds->left = hiliteBounds->right = -1;
    } else if ( brief->briefPointKind == kBriefObjectKind)
    {
        GetInitialObjectSpriteData( whichScenario, brief->briefPointData.objectBriefType.objectNum,
                maxSize, bounds, corner, scale, &thisScale, &aSpritePix, &where, &spriteRect);
        hiliteBounds->left = spriteRect.left - 2;
        hiliteBounds->right = spriteRect.right + 2;
        hiliteBounds->top = spriteRect.top - 2;
        hiliteBounds->bottom = spriteRect.bottom + 2;

    }
    *headerID = brief->titleResID;
    *headerNumber = brief->titleNum;
    *contentID = brief->contentResID;
}
