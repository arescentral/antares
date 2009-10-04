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

// Beam.c

#include "Beam.hpp"

#include "AresGlobalType.hpp"
#include "ColorTranslation.hpp"
#include "Debug.hpp"
#include "Error.hpp"
#include "MathMacros.hpp"
#include "NateDraw.hpp"
#include "Options.hpp"
#include "Randomize.hpp"
#include "Rotation.hpp"
#include "UniverseUnit.hpp"

#define kBeamError      "\pBEAM"

#define kBeamNum        256
#define kBoltChangeTime 0

extern PixMap*          gActiveWorld;
extern PixMap*          gOffWorld;
extern long             gNatePortLeft, gNatePortTop, gAbsoluteScale,
                        CLIP_LEFT, CLIP_TOP, CLIP_RIGHT, CLIP_BOTTOM;
extern coordPointType   gGlobalCorner;
extern TypedHandle<spaceObjectType> gSpaceObjectData;

void DetermineBeamRelativeCoordFromAngle( spaceObjectType *, short);

short InitBeams() {
    beamType    *beam;
    short           i;

    globals()->gBeamData.create(kBeamNum);
    beam = *globals()->gBeamData;
    for (i = 0; i < kBeamNum; i++) {
        beam->active = false;
        beam++;
    }
    return kNoError;
}

void CleanupBeams( void)
{
    if (globals()->gBeamData.get() != nil) globals()->gBeamData.destroy();
}

void ResetBeams( void)
{
    beamType    *aBeam = *globals()->gBeamData;
    short       i;

    for ( i = 0; i < kBeamNum; i++)
    {
        aBeam->active = false;
        aBeam->thisLocation.left = aBeam->thisLocation.right =
            aBeam->thisLocation.top = aBeam->thisLocation.bottom = 0;
        aBeam->lastLocation.left = aBeam->lastLocation.right =
            aBeam->lastLocation.top = aBeam->lastLocation.bottom = 0;
        aBeam->lastGlobalLocation.h = aBeam->lastGlobalLocation.v = 0;
        aBeam->objectLocation.h = aBeam->objectLocation.v = 0;
        aBeam->lastApparentLocation.h = aBeam->lastApparentLocation.v = 0;
        aBeam->killMe = false;

        aBeam++;
    }
}

beamType *AddBeam(coordPointType *location, unsigned char color,
    beamKindType kind, long accuracy, long range, long *whichBeam)
{
    beamType    *aBeam = *globals()->gBeamData;
    long        h;

    *whichBeam = 0;
    while (( aBeam->active) && ( *whichBeam < kBeamNum))
    {
        aBeam++;
        (*whichBeam)++;
    }
    if ( *whichBeam == kBeamNum)
    {
        *whichBeam = -1;
        return( nil);
    } else
    {
        aBeam->lastGlobalLocation = aBeam->objectLocation =
            aBeam->lastApparentLocation = *location;
        aBeam->killMe = false;
        aBeam->active = true;
        aBeam->color = color;

        h = (location->h - gGlobalCorner.h) * gAbsoluteScale;
        h >>= SHIFT_SCALE;
        aBeam->thisLocation.left = aBeam->thisLocation.right = h + CLIP_LEFT;
        h = (location->v - gGlobalCorner.v) * gAbsoluteScale;
        h >>= SHIFT_SCALE; //+ CLIP_TOP
        aBeam->thisLocation.top = aBeam->thisLocation.bottom = h;

        aBeam->lastLocation.left = aBeam->lastLocation.right =
                aBeam->thisLocation.left;
        aBeam->lastLocation.top = aBeam->lastLocation.bottom =
                aBeam->thisLocation.top;

        aBeam->beamKind = kind;
        aBeam->accuracy = accuracy;
        aBeam->range = range;
        aBeam->fromObjectNumber = aBeam->fromObjectID = -1;
        aBeam->fromObject = nil;
        aBeam->toObjectNumber = aBeam->toObjectID = -1;
        aBeam->toObject = nil;
        aBeam->toRelativeCoord.h = aBeam->toRelativeCoord.v = 0;
        aBeam->boltRandomSeed = 0;
        aBeam->boltCycleTime = 0;
        aBeam->boltState = 0;
        return( aBeam);
    }
}

void SetSpecialBeamAttributes( spaceObjectType *beamObject, spaceObjectType *sourceObject)
{
    spaceObjectType *target;
    long            h, v;

    beamObject->frame.beam.beam->fromObjectNumber = sourceObject->entryNumber;
    beamObject->frame.beam.beam->fromObjectID = sourceObject->id;
    beamObject->frame.beam.beam->fromObject = sourceObject;

    if ( sourceObject->targetObjectNumber >= 0)
    {
        target = *gSpaceObjectData + sourceObject->targetObjectNumber;

        if ( ( target->active) && ( target->id == sourceObject->targetObjectID))
        {
//          beamObject->frame.beam.beam->fromObjectNumber = sourceObject->entryNumber;
//          beamObject->frame.beam.beam->fromObjectID = sourceObject->id;
//          beamObject->frame.beam.beam->fromObject = ( spaceObjectTypePtr) sourceObject;

            if ( target->location.h > beamObject->location.h)
                h = target->location.h - beamObject->location.h;
            else h = beamObject->location.h - target->location.h;
            if ( target->location.v > beamObject->location.v)
                v = target->location.v - beamObject->location.v;
            else v = beamObject->location.v - target->location.v;

            if (((( h * h) + ( v * v)) >
                ( beamObject->frame.beam.beam->range *
                    beamObject->frame.beam.beam->range)) ||
                ( h > kMaximumRelevantDistance) ||
                ( v > kMaximumRelevantDistance))
            {
                if ( beamObject->frame.beam.beam->beamKind ==
                    eStaticObjectToObjectKind)
                {
                    beamObject->frame.beam.beam->beamKind =
                        eStaticObjectToRelativeCoordKind;
                } else if ( beamObject->frame.beam.beam->beamKind ==
                    eBoltObjectToObjectKind)
                {
                    beamObject->frame.beam.beam->beamKind =
                        eBoltObjectToRelativeCoordKind;
                }
                DetermineBeamRelativeCoordFromAngle( beamObject,
                    sourceObject->targetAngle);
            } else
            {
                if ( ( beamObject->frame.beam.beam->beamKind ==
                        eStaticObjectToRelativeCoordKind) ||
                    ( beamObject->frame.beam.beam->beamKind ==
                        eBoltObjectToRelativeCoordKind))
                {
                    beamObject->frame.beam.beam->toRelativeCoord.h =
                        target->location.h - sourceObject->location.h;
                    beamObject->frame.beam.beam->toRelativeCoord.v =
                        target->location.v - sourceObject->location.v;
                    beamObject->frame.beam.beam->toRelativeCoord.h +=
                        - beamObject->frame.beam.beam->accuracy +
                        RandomSeeded(beamObject->frame.beam.beam->accuracy << 1,
                                    &(beamObject->randomSeed), 'beam', 1);
                    beamObject->frame.beam.beam->toRelativeCoord.v +=
                        - beamObject->frame.beam.beam->accuracy +
                        RandomSeeded(beamObject->frame.beam.beam->accuracy << 1,
                                    &(beamObject->randomSeed), 'beam', 1);

                } else
                {
                    beamObject->frame.beam.beam->toObjectNumber =
                        target->entryNumber;
                    beamObject->frame.beam.beam->toObjectID = target->id;
                    beamObject->frame.beam.beam->toObject = target;
                }
            }
        } else // target not valid
        {

                if ( beamObject->frame.beam.beam->beamKind ==
                    eStaticObjectToObjectKind)
                {
                    beamObject->frame.beam.beam->beamKind =
                        eStaticObjectToRelativeCoordKind;
                } else if ( beamObject->frame.beam.beam->beamKind ==
                    eBoltObjectToObjectKind)
                {
                    beamObject->frame.beam.beam->beamKind =
                        eBoltObjectToRelativeCoordKind;
                }
            DetermineBeamRelativeCoordFromAngle( beamObject,
                sourceObject->direction);
//          beamObject->frame.beam.beam->killMe = true;
        }
    } else // target not valid
    {
                if ( beamObject->frame.beam.beam->beamKind ==
                    eStaticObjectToObjectKind)
                {
                    beamObject->frame.beam.beam->beamKind =
                        eStaticObjectToRelativeCoordKind;
                } else if ( beamObject->frame.beam.beam->beamKind ==
                    eBoltObjectToObjectKind)
                {
                    beamObject->frame.beam.beam->beamKind =
                        eBoltObjectToRelativeCoordKind;
                }
        DetermineBeamRelativeCoordFromAngle( beamObject,
            sourceObject->direction);
//      beamObject->frame.beam.beam->killMe = true;
    }
}

void DetermineBeamRelativeCoordFromAngle( spaceObjectType *beamObject,
    short angle)
{
    long            h = angle;
    smallFixedType  fcos, fsin, x = 0,
        y = mLongToFixed( beamObject->frame.beam.beam->range);

    mAddAngle( h, -90);
    mGetRotPoint( fcos, fsin, h);
    fcos = -fcos;
    fsin = -fsin;

    beamObject->frame.beam.beam->toRelativeCoord.h = mMultiplyFixed( x, fcos);
    beamObject->frame.beam.beam->toRelativeCoord.h -= mMultiplyFixed( y, fsin);
    beamObject->frame.beam.beam->toRelativeCoord.v = mMultiplyFixed( x, fsin);
    beamObject->frame.beam.beam->toRelativeCoord.v += mMultiplyFixed( y, fcos);
    beamObject->frame.beam.beam->toRelativeCoord.h =
        mFixedToLong( beamObject->frame.beam.beam->toRelativeCoord.h);
    beamObject->frame.beam.beam->toRelativeCoord.v =
        mFixedToLong( beamObject->frame.beam.beam->toRelativeCoord.v);
}

/*
void DrawAllBeams( void)

{
    beamType        *aBeam = ( beamType *)*globals()->gBeamData;
    baseObjectType  *baseObject;
    short           i;
    Rect        bounds;
    long            h;
    PixMapHandle    whatWorld = thePixMapHandle;

    if (( globals()->gOptions & kOptionQDOnly)) whatWorld = GetGWorldPixMap( gOffWorld);

    bounds.left = CLIP_LEFT;
    bounds.right = CLIP_RIGHT;
    bounds.top = CLIP_TOP;
    bounds.bottom = CLIP_BOTTOM;

    for ( i = 0; i < kBeamNum; i++)
    {
        if ( aBeam->active)
        {
            if (( aBeam->lastApparentLocation.h != aBeam->objectLocation.h) ||
                ( aBeam->lastApparentLocation.v != aBeam->objectLocation.v))
            {
                h = ( aBeam->lastApparentLocation.h - gGlobalCorner.h) * gAbsoluteScale;
                h >>= SHIFT_SCALE;
                aBeam->thisLocation.right = h + CLIP_LEFT;
                h = (aBeam->lastApparentLocation.v - gGlobalCorner.v) * gAbsoluteScale;
                h >>= SHIFT_SCALE; //+ CLIP_TOP;
                aBeam->thisLocation.bottom = h;

                h = ( aBeam->objectLocation.h - gGlobalCorner.h) * gAbsoluteScale;
                h >>= SHIFT_SCALE;
                aBeam->thisLocation.left = h + CLIP_LEFT;
                h = (aBeam->objectLocation.v - gGlobalCorner.v) * gAbsoluteScale;
                h >>= SHIFT_SCALE; //+ CLIP_TOP;
                aBeam->thisLocation.top = h;

                aBeam->lastApparentLocation.h = aBeam->objectLocation.h;
                aBeam->lastApparentLocation.v = aBeam->objectLocation.v;
            }
            if (( !aBeam->killMe) && ( aBeam->active != kObjectToBeFreed))
            {
                if ( aBeam->color)
                {
                    DrawNateLine( *whatWorld, &bounds, (long)aBeam->thisLocation.left,
                            (long)aBeam->thisLocation.top,
                            (long)aBeam->thisLocation.right,
                            (long)aBeam->thisLocation.bottom,
                            gNatePortLeft << 2, gNatePortTop, aBeam->color);
                }
            } else
            {
                aBeam->active = false;
            }
            if (( aBeam->color) &&
                (( aBeam->lastLocation.top != aBeam->thisLocation.top) ||
                ( aBeam->lastLocation.right != aBeam->thisLocation.right) ||
                ( aBeam->lastLocation.bottom != aBeam->thisLocation.bottom) ||
                ( aBeam->lastLocation.left != aBeam->thisLocation.left)))
            {
                DrawNateLine( *whatWorld, &bounds, (long)aBeam->lastLocation.left,
                            (long)aBeam->lastLocation.top,
                            (long)aBeam->lastLocation.right,
                            (long)aBeam->lastLocation.bottom,
                            gNatePortLeft << 2, gNatePortTop, BLACK);
            }

            aBeam->lastLocation = aBeam->thisLocation;
        }
        aBeam++;
    }
}
*/

void DrawAllBeams( void)

{
    beamType        *aBeam = *globals()->gBeamData;
    short           i, j;
    Rect        bounds;
    long            h, v;
    unsigned char   currentColor;

    bounds.left = CLIP_LEFT;
    bounds.right = CLIP_RIGHT;
    bounds.top = CLIP_TOP;
    bounds.bottom = CLIP_BOTTOM;

    for ( i = 0; i < kBeamNum; i++)
    {
        if ( aBeam->active)
        {
            if (( aBeam->lastApparentLocation.h != aBeam->objectLocation.h) ||
                ( aBeam->lastApparentLocation.v != aBeam->objectLocation.v))
            {
                h = ( aBeam->lastApparentLocation.h - gGlobalCorner.h) * gAbsoluteScale;
                h >>= SHIFT_SCALE;
                aBeam->thisLocation.right = h + CLIP_LEFT;
                h = (aBeam->lastApparentLocation.v - gGlobalCorner.v) * gAbsoluteScale;
                h >>= SHIFT_SCALE; //+ CLIP_TOP;
                aBeam->thisLocation.bottom = h;

                h = ( aBeam->objectLocation.h - gGlobalCorner.h) * gAbsoluteScale;
                h >>= SHIFT_SCALE;
                aBeam->thisLocation.left = h + CLIP_LEFT;
                h = (aBeam->objectLocation.v - gGlobalCorner.v) * gAbsoluteScale;
                h >>= SHIFT_SCALE; //+ CLIP_TOP;
                aBeam->thisLocation.top = h;

                aBeam->lastApparentLocation.h = aBeam->objectLocation.h;
                aBeam->lastApparentLocation.v = aBeam->objectLocation.v;
            }

            if ( aBeam->color)
            {
                if (( aBeam->beamKind == eBoltObjectToObjectKind) ||
                    ( aBeam->beamKind == eBoltObjectToRelativeCoordKind))
                {
                    for ( j = 1; j < kBoltPointNum; j++)
                    {
                        DrawNateLine(gOffWorld, &bounds,
                                    aBeam->lastBoltPoint[j-1].h,
                                    aBeam->lastBoltPoint[j-1].v,
                                    aBeam->lastBoltPoint[j].h,
                                    aBeam->lastBoltPoint[j].v,
                                    0, 0, BLACK);
                    }
                } else
                {
                    DrawNateLine(gOffWorld, &bounds, aBeam->lastLocation.left,
                                aBeam->lastLocation.top,
                                aBeam->lastLocation.right,
                                aBeam->lastLocation.bottom,
                                0, 0, BLACK);
                }
            }

            if (( !aBeam->killMe) && ( aBeam->active != kObjectToBeFreed))
            {
                if ( aBeam->color)
                {
                    if ( aBeam->beamKind != eKineticBeamKind)
                    {
                        aBeam->boltState++;
                        if ( aBeam->boltState > 24) aBeam->boltState = -24;
                        currentColor = GetRetroIndex( aBeam->color);
                        currentColor &= 0xf0;
                        if ( aBeam->boltState < 0)
                            currentColor += (-aBeam->boltState) >> 1;
                        else
                            currentColor += aBeam->boltState >> 1;
                        aBeam->color = GetTranslateIndex( currentColor);
                    }
                    if (( aBeam->beamKind == eBoltObjectToObjectKind) ||
                        ( aBeam->beamKind == eBoltObjectToRelativeCoordKind))
                    {
                        aBeam->boltCycleTime++;
                        if ( aBeam->boltCycleTime > kBoltChangeTime)
                        {
                            aBeam->boltCycleTime = 0;
                            aBeam->thisBoltPoint[0].h = aBeam->thisLocation.left;
                            aBeam->thisBoltPoint[0].v = aBeam->thisLocation.top;
                            aBeam->thisBoltPoint[kBoltPointNum - 1].h =
                                aBeam->thisLocation.right;
                            aBeam->thisBoltPoint[kBoltPointNum - 1].v =
                                aBeam->thisLocation.bottom;
                            h = ABS(( aBeam->thisLocation.right -
                                aBeam->thisLocation.left) / kBoltPointNum);
                            v = ABS(( aBeam->thisLocation.bottom -
                                aBeam->thisLocation.top) / kBoltPointNum);
                            h >>= 1;
                            v >>= 1;
                            if ( h > v) v = h;
                            else h = v;
                            for ( j = 1; j < (kBoltPointNum - 1); j++)
                            {
                                aBeam->thisBoltPoint[j].h =
                                    aBeam->thisLocation.left +
                                    (
                                        (
                                            (
                                                aBeam->thisLocation.right -
                                                aBeam->thisLocation.left
                                            )
                                            * j
                                        )
                                        / kBoltPointNum
                                    ) - h + Randomize( h << 1);
                                aBeam->thisBoltPoint[j].v =
                                    aBeam->thisLocation.top +
                                    (
                                        (
                                            (
                                                aBeam->thisLocation.bottom -
                                                aBeam->thisLocation.top
                                            )
                                            * j
                                        )
                                        / kBoltPointNum
                                    ) - v + Randomize( v << 1);
                            }
                        }
                        for ( j = 1; j < kBoltPointNum; j++)
                        {
                            DrawNateLine(gOffWorld, &bounds,
                                        aBeam->thisBoltPoint[j-1].h,
                                        aBeam->thisBoltPoint[j-1].v,
                                        aBeam->thisBoltPoint[j].h,
                                        aBeam->thisBoltPoint[j].v,
                                        0, 0, aBeam->color);
                        }
                    } else
                    {
                        DrawNateLine(gOffWorld, &bounds, aBeam->thisLocation.left,
                                aBeam->thisLocation.top,
                                aBeam->thisLocation.right,
                                aBeam->thisLocation.bottom,
                                0, 0, aBeam->color);
                    }
                }
            } else
            {
//              aBeam->active = false;
            }
        }
        aBeam++;
    }
}

void EraseAllBeams( void)
{
}

void ShowAllBeams( void)
{
    beamType        *aBeam = *globals()->gBeamData;
    short           i, j;
    Rect        bounds;

    bounds.left = CLIP_LEFT;
    bounds.right = CLIP_RIGHT;
    bounds.top = CLIP_TOP;
    bounds.bottom = CLIP_BOTTOM;

    for ( i = 0; i < kBeamNum; i++)
    {
        if ( aBeam->active)
        {
            if (( !aBeam->killMe) && ( aBeam->active != kObjectToBeFreed))
            {
                if ( aBeam->color)
                {
                    if (( aBeam->beamKind == eBoltObjectToObjectKind) ||
                        ( aBeam->beamKind == eBoltObjectToRelativeCoordKind))
                    {
                        for ( j = 1; j < kBoltPointNum; j++)
                        {
                            CopyNateLine(gOffWorld, gActiveWorld, &bounds,
                                        aBeam->thisBoltPoint[j-1].h,
                                        aBeam->thisBoltPoint[j-1].v,
                                        aBeam->thisBoltPoint[j].h,
                                        aBeam->thisBoltPoint[j].v,
                                        gNatePortLeft << 2, gNatePortTop);
                        }
                    } else
                    {
                        CopyNateLine(gOffWorld, gActiveWorld, &bounds,
                                aBeam->thisLocation.left,
                                aBeam->thisLocation.top,
                                aBeam->thisLocation.right,
                                aBeam->thisLocation.bottom,
                                gNatePortLeft << 2, gNatePortTop);
                    }
                }
            } else
            {
                aBeam->active = false;
            }
            if ( aBeam->color)
            {
                if (( aBeam->beamKind == eBoltObjectToObjectKind) ||
                    ( aBeam->beamKind == eBoltObjectToRelativeCoordKind))
                {
                    for ( j = 1; j < kBoltPointNum; j++)
                    {
                        CopyNateLine(gOffWorld, gActiveWorld, &bounds,
                                    aBeam->lastBoltPoint[j-1].h,
                                    aBeam->lastBoltPoint[j-1].v,
                                    aBeam->lastBoltPoint[j].h,
                                    aBeam->lastBoltPoint[j].v,
                                    gNatePortLeft << 2, gNatePortTop);
                    }
                    for ( j = 0; j < kBoltPointNum; j++)
                    {
                        aBeam->lastBoltPoint[j].h =
                            aBeam->thisBoltPoint[j].h;
                        aBeam->lastBoltPoint[j].v =
                            aBeam->thisBoltPoint[j].v;
                    }

                } else
                {
                    CopyNateLine(gOffWorld, gActiveWorld, &bounds,
                            aBeam->lastLocation.left,
                            aBeam->lastLocation.top,
                            aBeam->lastLocation.right,
                            aBeam->lastLocation.bottom,
                            gNatePortLeft << 2, gNatePortTop);
                }
            }

            aBeam->lastLocation = aBeam->thisLocation;
        }
        aBeam++;
    }
}


void CullBeams( void)
{
    beamType        *aBeam = *globals()->gBeamData;
    short           i;

    for ( i = 0; i < kBeamNum; i++)
    {
        if ( aBeam->active)
        {
                if (( aBeam->killMe) || ( aBeam->active == kObjectToBeFreed))
                {
                    aBeam->active = false;
                }
                aBeam->lastLocation = aBeam->thisLocation;
        }
        aBeam++;
    }
}
