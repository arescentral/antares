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

// Ares Guide Maker.c

//#define pGenerateImages

#include "AresGuideMaker.hpp"

#include <QDOffscreen.h>
#include <Quickdraw.h>

#include "AnyChar.hpp"
#include "AresGlobalType.hpp"
#include "ConditionalMacros.h"
#include "Debug.hpp"
#include "Error.hpp"
#include "LoadClip2Gif.h"
#include "NatePixTable.hpp"
#include "OffscreenGWorld.hpp"
#include "Races.hpp"
#include "Resources.h"
#include "SpaceObjectHandling.hpp"
#include "SpriteHandling.hpp"
#include "StringHandling.hpp"
#include "StringNumerics.hpp"

#define kSpriteBoundsWidth  128
#define kSpriteBoundsHeight 128

#define kGraphWidth         50

struct fieldRangeType {
    long                        minmass;
    long                        maxmass;
    long                        minturn;
    long                        maxturn;
    long                        minaccel;
    long                        maxaccel;
    long                        minvel;
    long                        maxvel;
    long                        minwarp;
    long                        maxwarp;
    long                        minshields;
    long                        maxshields;
    long                        mincost;
    long                        maxcost;
    long                        minbuild;
    long                        maxbuild;
    long                        minenergy;
    long                        maxenergy;
    long                        minwammo;
    long                        maxwammo;
    smallFixedType              minwfire;
    smallFixedType              maxwfire;
    long                        minwvel;
    long                        maxwvel;
    long                        minwrange;
    long                        maxwrange;
    long                        minwdamage;
    long                        maxwdamage;
    long                        minwenergy;
    long                        maxwenergy;
};

struct weaponDataType {
    long                        ammo;
    smallFixedType              fireTime;
    long                        velocity;
    long                        range;
    long                        damage;
    Boolean                     guided;
    Boolean                     autoTarget;
    long                        energyCost;
};

extern long             WORLD_WIDTH, WORLD_HEIGHT;
extern GWorldPtr        gOffWorld;
extern TypedHandle<baseObjectType>  gBaseObjectData;
extern objectActionType**       gObjectActionData;
extern aresGlobalType*          gAresGlobal;

void ConvertPortraitIntoGIF(long, unsigned char*);
void InsertWeaponText( short, short, Handle, weaponDataType *, fieldRangeType *);
void AppendStringToHandle( const unsigned char*, Handle);
void AdjustRangeFromObject( baseObjectType *, fieldRangeType *);
void AdjustRangeFromWeaponData( weaponDataType *, fieldRangeType *);
void GetWeaponData( long, weaponDataType *);
void ReplaceIndStringWithIndStringInHandle( long, long, long, long, Handle);
void ReplaceIndStringWithStringInHandle(long, long, unsigned char*, Handle);
void ReplaceIndStringWithHandleInHandle( long, long, Handle, Handle);
void GetFileNameFromObject(long, unsigned char*);
void GetFileNameFromSprite(long, unsigned char*);
void GetFileNameFromPortrait(long, unsigned char*);
void NumToNDigitString(long, unsigned char*, long);
void InsertGraphText( long, long, long, short, short, Handle);
Handle GetGraphText( long);
void InsertGraphicText( long, long, Handle);
void InsertIndexText( long, long, Handle);
long GetPreviousIndexBaseObject( long);
Handle NewHandleFromString(unsigned char*);
void CheckStringForNil( const unsigned char*);
void CheckHandleForNil( Handle);

void InitAresGuide( void)
{
    OSErr   err;

    err = LoadClip2Gif();
    if ( err != noErr)
    {
        MyDebugString("\pCouldn't Load Clip2GIF!");
//      ExitToShell();
        return;
    }
}

void MakeAresGuide( void)
{
    baseObjectType          *baseObject, *weaponObject;
    Str255                  scrapString, ss2;
    fieldRangeType          range;
    Handle                  sourceText, newText;
    long                    count, whichShape;
    short                   raceNum;
    weaponDataType          weaponData;

    range.minmass=0x7fffffff;
    range.maxmass=-1;
    range.minturn=0x7fffffff;
    range.maxturn=-1;
    range.minaccel=0x7fffffff;
    range.maxaccel=-1;
    range.minvel=0x7fffffff;
    range.maxvel=-1;
    range.minwarp=0x7fffffff;
    range.maxwarp=-1;
    range.minshields=0x7fffffff;
    range.maxshields=-1;
    range.mincost=0x7fffffff;
    range.maxcost=-1;
    range.minbuild=0x7fffffff;
    range.maxbuild=0;
    range.minenergy=0x7fffffff;
    range.maxenergy=-1;
    range.minwammo=0x7fffffff;
    range.maxwammo=-1;
    range.minwfire=0x7fffffff;
    range.maxwfire=-1;
    range.minwvel=0x7fffffff;
    range.maxwvel=-1;
    range.minwrange=0x7fffffff;
    range.maxwrange=-1;
    range.minwdamage=0x7fffffff;
    range.maxwdamage=-1;
    range.minwenergy=0x7fffffff;
    range.maxwenergy=-1;

    InitAresGuide();

    // 1st pass guage mins & maxs
    baseObject = *gBaseObjectData;

    for ( count = 0; count < kMaxBaseObject; count++)
    {
        if (baseObject->internalFlags & 0x40000000)
        {
            AdjustRangeFromObject( baseObject, &range);
            if ( baseObject->pulse != kNoWeapon)
            {
//              weaponObject = (baseObjectType *)*gBaseObjectData + baseObject->pulse;
//              AdjustRangeFromObject( weaponObject, &range);
                GetWeaponData( baseObject->pulse, &weaponData);
                AdjustRangeFromWeaponData( &weaponData, &range);
            }
            if ( baseObject->beam != kNoWeapon)
            {
//              weaponObject = (baseObjectType *)*gBaseObjectData + baseObject->beam;
//              AdjustRangeFromObject( weaponObject, &range);
                GetWeaponData( baseObject->beam, &weaponData);
                AdjustRangeFromWeaponData( &weaponData, &range);
            }
            if ( baseObject->special != kNoWeapon)
            {
//              weaponObject = (baseObjectType *)*gBaseObjectData + baseObject->special;
//              AdjustRangeFromObject( weaponObject, &range);
                GetWeaponData( baseObject->special, &weaponData);
                AdjustRangeFromWeaponData( &weaponData, &range);
            }

        }
        baseObject++;
    }


    baseObject = *gBaseObjectData;

#ifdef pGenerateImages
    for ( count = 0; count < kMaxBaseObject; count++) // kMaxBaseObject
    {
        baseObject = (baseObjectType *)*gBaseObjectData + count;
        mWriteDebugString("\pLooking at:");
        WriteDebugLong( count);
        WriteDebugLong( baseObject->pixResID);
        if (baseObject->internalFlags & 0x40000000)
        {
            if (baseObject->pixResID > 0 )
            {
                GetFileNameFromSprite( baseObject->pixResID, fileName);
                whichShape = 0;
                if ( baseObject->attributes & kShapeFromDirection)
                {
                    whichShape = 225 / baseObject->frame.rotation.rotRes;
                }
                ConvertSpriteIntoGIF( baseObject->pixResID, whichShape, fileName);
            }

            GetFileNameFromPortrait( count, fileName);
            ConvertPortraitIntoGIF( count, fileName);
        }
    }
#endif

    sourceText = GetResource( 'TEXT', 6551);
    if ( sourceText == nil) MyDebugString("\pcouldn't get source text.");
    DetachResource( sourceText);

    baseObject = *gBaseObjectData;

    for ( count = 0; count < kMaxBaseObject; count++) // kMaxBaseObject
    {
        baseObject = *gBaseObjectData + count;
        if (baseObject->internalFlags & 0x40000000)
        {
            newText = sourceText;
            HandToHand( &newText);
            CheckHandleForNil( newText);

            raceNum = GetRaceNumFromID( baseObject->baseRace);
            if ( raceNum >= 0)
            {
                GetRaceString( ss2, kRaceAdjective, raceNum);
            } else
            {
                GetIndString(  ss2, 6454, 23);
            }

            // title
            GetIndString( scrapString, 5000, count + 1);
            ConcatenatePString( scrapString, "\p, ");
            ConcatenatePString( scrapString, ss2);
            ReplaceIndStringWithStringInHandle( 6453, 78, scrapString, newText);

            // race name
            ReplaceIndStringWithStringInHandle( 6453, 1, ss2, newText);

            // chapter name ( = race name, only when object is 1st of race)
            whichShape = GetPreviousIndexBaseObject( count);
            if ( whichShape >= 0)
            {
                weaponObject = *gBaseObjectData + whichShape;
                if ( weaponObject->baseRace != baseObject->baseRace)
                {
                    ReplaceIndStringWithStringInHandle( 6453, 79, ss2, newText);
                }
            }

            // ship name
            ReplaceIndStringWithIndStringInHandle( 6453, 2, 5000, count + 1, newText);
            ReplaceIndStringWithIndStringInHandle( 6453, 3, 5001, count + 1, newText);
            ReplaceIndStringWithIndStringInHandle( 6453, 80, 5000, count + 1, newText);

            // mass
            SmallFixedToString( baseObject->mass, scrapString);
            ReplaceIndStringWithStringInHandle( 6453, 7, scrapString, newText);
            InsertGraphText( baseObject->mass, range.minmass, range.maxmass, 6453, 8, newText);

            // turn
            if ( baseObject->attributes & kCanTurn)
            {
                SmallFixedToString( baseObject->frame.rotation.maxTurnRate, scrapString);
                ReplaceIndStringWithStringInHandle( 6453, 9, scrapString, newText);
                InsertGraphText( baseObject->frame.rotation.maxTurnRate, range.minturn, range.maxturn, 6453, 10, newText);
            }

            // accel
            SmallFixedToString( baseObject->maxThrust, scrapString);
            ReplaceIndStringWithStringInHandle( 6453, 11, scrapString, newText);
            InsertGraphText( baseObject->maxThrust, range.minaccel, range.maxaccel, 6453, 12, newText);

            // velocity
            SmallFixedToString( baseObject->maxVelocity, scrapString);
            ReplaceIndStringWithStringInHandle( 6453, 13, scrapString, newText);
            InsertGraphText( baseObject->maxVelocity, range.minvel, range.maxvel, 6453, 14, newText);

            // warp
            SmallFixedToString( baseObject->warpSpeed, scrapString);
            ReplaceIndStringWithStringInHandle( 6453, 15, scrapString, newText);
            InsertGraphText( baseObject->warpSpeed, range.minwarp, range.maxwarp, 6453, 16, newText);

            // shields
            NumToString( baseObject->health, scrapString);
            ReplaceIndStringWithStringInHandle( 6453, 17, scrapString, newText);
            InsertGraphText( baseObject->health, range.minshields, range.maxshields, 6453, 18, newText);

            // cost
            NumToString( baseObject->price, scrapString);
            ReplaceIndStringWithStringInHandle( 6453, 19, scrapString, newText);
            InsertGraphText( baseObject->price, range.mincost, range.maxcost, 6453, 20, newText);

            // build time
            NumToString( baseObject->buildTime, scrapString);
            ReplaceIndStringWithStringInHandle( 6453, 21, scrapString, newText);
            InsertGraphText( baseObject->buildTime, range.minbuild, range.maxbuild, 6453, 22, newText);

            // energy
            SmallFixedToString( baseObject->energy, scrapString);
            ReplaceIndStringWithStringInHandle( 6453, 23, scrapString, newText);
            InsertGraphText( baseObject->energy, range.minenergy, range.maxenergy, 6453, 24, newText);

            // weapon 1
            if ( baseObject->pulse != kNoWeapon)
            {
                GetIndString( scrapString, 5000, baseObject->pulse + 1);
                ReplaceIndStringWithStringInHandle( 6453, 4, scrapString, newText);

                GetWeaponData( baseObject->pulse, &weaponData);
                InsertWeaponText( 6453, 25, newText, &weaponData, &range);
            } else
            {
                ReplaceIndStringWithIndStringInHandle( 6453, 4, 6454, 24, newText);
            }

            // weapon 2
            if ( baseObject->beam != kNoWeapon)
            {
                GetIndString( scrapString, 5000, baseObject->beam + 1);
                ReplaceIndStringWithStringInHandle( 6453, 5, scrapString, newText);

                GetWeaponData( baseObject->beam, &weaponData);
                InsertWeaponText( 6453, 41, newText, &weaponData, &range);
            } else
            {
                ReplaceIndStringWithIndStringInHandle( 6453, 5, 6454, 24, newText);
            }

            // weapon 3
            if ( baseObject->special != kNoWeapon)
            {
                GetIndString( scrapString, 5000, baseObject->special + 1);
                ReplaceIndStringWithStringInHandle( 6453, 6, scrapString, newText);

                GetWeaponData( baseObject->special, &weaponData);
                InsertWeaponText( 6453, 57, newText, &weaponData, &range);
            } else
            {
                ReplaceIndStringWithIndStringInHandle( 6453, 6, 6454, 24, newText);
            }

            InsertGraphicText( count, baseObject->pixResID, newText);
            InsertIndexText( count, baseObject->baseRace, newText);

            GetFileNameFromObject( count, scrapString);
            MoveHHi( newText);
            HLock( newText);
            CheckHandleForNil( newText);
            SaveBlockToFile( *newText, GetHandleSize( newText), scrapString);
            HUnlock( newText);
            DisposeHandle( newText);
        }
    }
    DisposeHandle( sourceText);
}

void InsertWeaponText( short resID, short startNum, Handle newText, weaponDataType *data, fieldRangeType *range)
{
    Str255  scrapString;

    // ammo
    if ( data->ammo < 0)
    {
        ReplaceIndStringWithIndStringInHandle( resID, startNum + 0, 5454, 1, newText);
    } else
    {
        NumToString( data->ammo, scrapString);
        ReplaceIndStringWithStringInHandle( resID, startNum + 0, scrapString, newText);
        InsertGraphText( data->ammo, range->minwammo, range->maxwammo, resID, startNum + 1, newText);
    }

    // fireTime
    SmallFixedToString( data->fireTime, scrapString);
    ReplaceIndStringWithStringInHandle( resID, startNum + 2, scrapString, newText);
    InsertGraphText( data->fireTime, range->minwfire, range->maxwfire, resID, startNum + 3, newText);

    // velocity
    SmallFixedToString( data->velocity, scrapString);
    ReplaceIndStringWithStringInHandle( resID, startNum + 4, scrapString, newText);
    InsertGraphText( data->velocity, range->minwvel, range->maxwvel, resID, startNum + 5, newText);

    // range
    NumToString( data->range, scrapString);
    ReplaceIndStringWithStringInHandle( resID, startNum + 6, scrapString, newText);
    InsertGraphText( data->range, range->minwrange, range->maxwrange, resID, startNum + 7, newText);

    // damage
    NumToString( data->damage, scrapString);
    ReplaceIndStringWithStringInHandle( resID, startNum + 8, scrapString, newText);
    InsertGraphText( data->damage, range->minwdamage, range->maxwdamage, resID, startNum + 9, newText);

    // guided
    if ( data->guided)
    {
        ReplaceIndStringWithIndStringInHandle( resID, startNum + 10, 6454, 5, newText);
        InsertGraphText( kGraphWidth, 0, kGraphWidth, resID, startNum + 11, newText);
    } else
    {
        ReplaceIndStringWithIndStringInHandle( resID, startNum + 10, 6454, 6, newText);
        InsertGraphText( 0, 0, kGraphWidth, resID, startNum + 11, newText);
    }

    // autoTarget
    if ( data->autoTarget)
    {
        ReplaceIndStringWithIndStringInHandle( resID, startNum + 12, 6454, 5, newText);
        InsertGraphText( kGraphWidth, 0, kGraphWidth, resID, startNum + 13, newText);
    } else
    {
        ReplaceIndStringWithIndStringInHandle( resID, startNum + 12, 6454, 6, newText);
        InsertGraphText( 0, 0, kGraphWidth, resID, startNum + 13, newText);
    }

    // energy
    NumToString( data->energyCost, scrapString);
    ReplaceIndStringWithStringInHandle( resID, startNum + 14, scrapString, newText);
    InsertGraphText( data->energyCost, range->minwenergy, range->maxwenergy, resID, startNum + 15, newText);
}

OSErr ConvertSpriteIntoGIF(short resID, long whichShape, unsigned char* forceName)
{
    Boolean             tableExisted = true;
    natePixType**       spriteTable = nil;
    Rect                r;
    PixMapHandle        offPixBase = GetGWorldPixMap( gOffWorld);
    spritePix           aSpritePix;
    unsigned char       *pixData;
    Point               where;
    long                tlong, thisScale;
    longRect            dRect, spriteRect;
    coordPointType      coord;
    PicHandle           newPic;
    OSErr               err = noErr;
    FSSpec              newFile;
    GrafPtr             oldPort;

    GetPort( &oldPort);
    mWriteDebugString("\pOpening:");
    WriteDebugLong( resID);
    spriteTable = reinterpret_cast<natePixType**>(GetPixTable( resID));
    if ( spriteTable == nil)
    {
        mWriteDebugString("\pNot Exist--adding.");
        tableExisted = false;
        spriteTable = AddPixTable( resID);
        if ( spriteTable == nil) return ( -1);
        mWriteDebugString("\padded.");
    }

    DrawInOffWorld();
    NormalizeColors();
    MacSetRect( &r, 0, 0, WORLD_WIDTH, WORLD_HEIGHT);
    PaintRect( &r);

    // set up the sprite

    dRect.left = 0;
    dRect.right = kSpriteBoundsWidth;
    dRect.top = 0;
    dRect.bottom = kSpriteBoundsHeight;

    pixData = GetNatePixTableNatePixData( spriteTable, whichShape);

    aSpritePix.data = &pixData;
    aSpritePix.center.h = GetNatePixTableNatePixHRef( spriteTable, whichShape);
    aSpritePix.center.v = GetNatePixTableNatePixVRef( spriteTable, whichShape);
    aSpritePix.width = GetNatePixTableNatePixWidth( spriteTable, whichShape);
    aSpritePix.height = GetNatePixTableNatePixHeight( spriteTable, whichShape);

    // calculate the correct size

/*  tlong = (long)(kSpriteBoundsHeight - 2) * SCALE_SCALE;
    tlong /= aSpritePix.height;
    thisScale = (long)(kSpriteBoundsWidth - 2) * SCALE_SCALE;
    thisScale /= aSpritePix.width;

    if ( tlong < thisScale) thisScale = tlong;
    if ( thisScale > SCALE_SCALE) thisScale = SCALE_SCALE;
*/
    thisScale = SCALE_SCALE;
    // calculate the correct position

    coord.h = aSpritePix.center.h;
    coord.h *= thisScale;
    coord.h >>= SHIFT_SCALE;
    tlong = aSpritePix.width;
    tlong *= thisScale;
    tlong >>= SHIFT_SCALE;
    where.h = ( kSpriteBoundsWidth / 2) - ( tlong / 2);
    where.h += dRect.left + coord.h;

    coord.v = aSpritePix.center.v;
    coord.v *= thisScale;
    coord.v >>= SHIFT_SCALE;
    tlong = aSpritePix.height;
    tlong *= thisScale;
    tlong >>= SHIFT_SCALE;
    where.v = ( kSpriteBoundsHeight / 2) - ( tlong / 2);
    where.v += dRect.top + coord.v;


    // draw the sprite

    OptScaleSpritePixInPixMap( &aSpritePix, where, thisScale,
            &spriteRect, &dRect, offPixBase);

    // clean up the sprite

    SetAllPixTablesNoKeep();
    RemoveAllUnusedPixTables();

    MacSetRect( &r, 0, 0, kSpriteBoundsWidth, kSpriteBoundsHeight);

    err = FSMakeFSSpec( 0, 0, forceName, &newFile);
    if ( err != -43)
    {
        MacSetPort( oldPort);
        return( err);
    }

    // make the gif

    newPic = MakePicHandleFromScreen( offPixBase, &r);
    if ( newPic == nil) Debugger();
    err = ConvertPictToGIFFile( newPic,
        &newFile,
        0,                  //interlaced,
        transparencyNo,     //transparency,
        8,                  //depth,
        colorPaletteSystem);//colors);

    // clean up the picture
    KillPicture( newPic);
    MacSetPort( oldPort);
    return( err);
}

void ConvertPortraitIntoGIF(long whichObject, unsigned char* fileName) {
    PicHandle           newPic;
    OSErr               err = noErr;
    FSSpec              newFile;

    err = FSMakeFSSpec( 0, 0, fileName, &newFile);
    if ( err != -43) // file does not exist err (FSSpec is OK, just no file)
    {
        return;
    }

    // make the gif

    newPic = GetPicture( whichObject + 1001);
    if ( newPic == nil) return;
    err = ConvertPictToGIFFile( newPic,
        &newFile,
        0,                  //interlaced,
        transparencyNo,     //transparency,
        8,                  //depth,
        colorPaletteSystem);//colors);
    ReleaseResource( reinterpret_cast<Handle>(newPic));
}

PicHandle MakePicHandleFromScreen( PixMapHandle sourceMap, Rect *sourceRect)
{
    PicHandle   newPic = nil;

    newPic = OpenPicture( sourceRect);
    if ( newPic != nil)
    {
        CopyBits( reinterpret_cast<BitMap *>(*sourceMap), reinterpret_cast<BitMap *>(*sourceMap),
                sourceRect, sourceRect, srcCopy, nil);
        ClosePicture();
    }
    return( newPic);
}

void AdjustRangeFromObject( baseObjectType *o, fieldRangeType *range)
{
    if ( o->attributes & ( kCanTurn | kIsSelfAnimated | kIsBeam))
    {
        if ( o->mass < range->minmass) range->minmass = o->mass;
        if ( o->mass > range->maxmass) range->maxmass = o->mass;

        if ( o->attributes & kCanTurn)
        {
            if ( o->frame.rotation.maxTurnRate < range->minturn)
                range->minturn = o->frame.rotation.maxTurnRate;
            if ( o->frame.rotation.maxTurnRate > range->maxturn)
                range->maxturn = o->frame.rotation.maxTurnRate;
        }

        if ( o->maxThrust < range->minaccel) range->minaccel = o->maxThrust;
        if ( o->maxThrust > range->maxaccel) range->maxaccel = o->maxThrust;

        if ( o->maxVelocity < range->minvel) range->minvel = o->maxVelocity;
        if ( o->maxVelocity > range->maxvel) range->maxvel = o->maxVelocity;

        if ( o->warpSpeed < range->minwarp) range->minwarp = o->warpSpeed;
        if ( o->warpSpeed > range->maxwarp) range->maxwarp = o->warpSpeed;

        if ( o->health < range->minshields) range->minshields = o->health;
        if ( o->health > range->maxshields) range->maxshields = o->health;

        if ( o->price < range->mincost) range->mincost = o->price;
        if ( o->price > range->maxcost) range->maxcost = o->price;

        if ( o->buildTime < static_cast<uint32_t>(range->minbuild)) range->minbuild = o->buildTime;
        if ( o->buildTime > static_cast<uint32_t>(range->maxbuild)) range->maxbuild = o->buildTime;

        if ( o->energy < range->minenergy) range->minenergy = o->energy;
        if ( o->energy > range->maxenergy) range->maxenergy = o->energy;
    }/* else
    {
        if ( o->frame.weapon.ammo < range->minwammo) range->minwammo = o->frame.weapon.ammo;
        if ( o->frame.weapon.ammo > range->maxwammo) range->maxwammo = o->frame.weapon.ammo;

        if ( o->frame.weapon.fireTime < range->minwfire) range->minwfire = o->frame.weapon.fireTime;
        if ( o->frame.weapon.fireTime > range->maxwfire) range->maxwfire = o->frame.weapon.fireTime;

        if ( o->frame.weapon.range < range->minwrange) range->minwrange = o->frame.weapon.range;
        if ( o->frame.weapon.range > range->maxwrange) range->minwrange = o->frame.weapon.range;

        if ( o->frame.weapon.energyCost < range->minwenergy) range->minwenergy = o->frame.weapon.energyCost;
        if ( o->frame.weapon.energyCost > range->maxwenergy) range->maxwenergy = o->frame.weapon.energyCost;
    }*/


}

void AdjustRangeFromWeaponData( weaponDataType *w, fieldRangeType *range)
{
    if ( w->ammo < range->minwammo) range->minwammo = w->ammo;
    if ( w->ammo > range->maxwammo) range->maxwammo = w->ammo;

    if ( w->fireTime < range->minwfire) range->minwfire = w->fireTime;
    if ( w->fireTime > range->maxwfire) range->maxwfire = w->fireTime;

    if ( w->velocity < range->minwvel) range->minwvel = w->velocity;
    if ( w->velocity > range->maxwvel) range->maxwvel = w->velocity;

    if ( w->range < range->minwrange) range->minwrange = w->range;
    if ( w->range > range->maxwrange) range->minwrange = w->range;

    if ( w->damage < range->minwdamage) range->minwdamage = w->damage;
    if ( w->damage > range->maxwdamage) range->maxwdamage = w->damage;

    if ( w->energyCost < range->minwenergy) range->minwenergy = w->energyCost;
    if ( w->energyCost > range->maxwenergy) range->maxwenergy = w->energyCost;
}

void GetWeaponData( long whichWeapon, weaponDataType *data)
{
    baseObjectType      *weaponObject, *missileObject;
    long                mostDamage, actionNum, mostSpeed;
    objectActionType    *action;
    Boolean             isGuided = false;

    if ( whichWeapon != kNoShip)
    {
        weaponObject = *gBaseObjectData + whichWeapon;

        // damage; this is tricky--we have to guess by walking through activate actions,
        //  and for all the createObject actions, see which creates the most damaging
        //  object.  We calc this first so we can use isGuided

        mostDamage = mostSpeed = 0;
        isGuided = false;
        if ( weaponObject->activateActionNum > 0)
        {
            action = *gObjectActionData + weaponObject->activateAction;
            for ( actionNum = 0; actionNum < weaponObject->activateActionNum; actionNum++)
            {
                if (( action->verb == kCreateObject) || ( action->verb == kCreateObjectSetDest))
                {
                    missileObject = *gBaseObjectData +
                        action->argument.createObject.whichBaseType;
                    if ( missileObject->attributes & kIsGuided) isGuided = true;
                    if ( missileObject->damage > mostDamage) mostDamage = missileObject->damage;
                    if ( missileObject->maxVelocity > mostSpeed) mostSpeed = missileObject->maxVelocity;
                }
                action++;
            }
        }

        data->guided = isGuided;
        // is autotarget
        if ( weaponObject->attributes & kAutoTarget)
            data->autoTarget = true;
        else data->autoTarget = false;

        // range
        data->range = lsqrt(weaponObject->frame.weapon.range);

        if ( mostDamage > 0)
        {
            data->damage = mostDamage;
        } else
        {
            data->damage = 0;
        }
        data->velocity = mostSpeed;
        data->energyCost = weaponObject->frame.weapon.energyCost;
        data->ammo = weaponObject->frame.weapon.ammo;
/*      if ( data->ammo > 500)
        {
            mWriteDebugString("\pTOO MUCH AMMO?");
            WriteDebugLong( whichWeapon + 1);
            DebugStr("\pLOOK AT WHICH WEAPON.");
        }
*/      data->fireTime = mLongToFixed(weaponObject->frame.weapon.fireTime);
        data->fireTime = mDivideFixed(mLongToFixed(60), data->fireTime);
    }
}

OSErr SaveBlockToFile(Ptr data, long len, unsigned char* fileName) {
    OSErr   err = noErr;
    FSSpec  newFile;
    short   newRefNum;
    long    count;

    if ( data != nil)
    {

        err = FSMakeFSSpec( 0, 0, fileName, &newFile);
        if (( err != noErr) && ( err != fnfErr))
            ShowSimpleStringAlert( "\pCouldn't make FSSpec out of", fileName, nil, nil);
        err = FSpCreate( &newFile, 'MSIE', 'TEXT', smSystemScript);
        if ( err == dupFNErr)
        {
            err = FSpDelete( &newFile);
            if ( err == noErr) err = FSpCreate( &newFile, 'CWIE', 'TEXT', smSystemScript);
            else SysBeep(20);
        }
        if ( err == noErr)
        {
            err = FSpOpenDF( &newFile, fsCurPerm, &newRefNum);
            if ( err == noErr)
            {
                count = len;
                err = FSWrite( newRefNum, &count, data);
                if ( err == noErr)
                {
                    err = FSClose( newRefNum);
                } else ShowErrorRecover ( RESOURCE_ERROR, "\pClose", err);
            } else ShowErrorRecover ( RESOURCE_ERROR, "\pOpen", err);
        } else ShowErrorRecover ( RESOURCE_ERROR, "\pCreate", err);
    }
    return( err);
}

void AppendStringToHandle(const unsigned char* s, Handle data)
{
    char lenCount;
    unsigned char* c;
    const unsigned char* sc;
    long    fileLen;

    CheckStringForNil( s);
    if ( data != nil)
    {
        fileLen = GetHandleSize( data);
        sc = s;
        if ( *sc == 0) return;
        SetHandleSize( data, fileLen + implicit_cast<long>(*sc));
        if ( MemError() == noErr)
        {
            HLock( data);
            c = *data + (fileLen);
            lenCount = *sc;
            sc++;
            while( lenCount > 0)
            {
                *c = *sc;
                c++;
                sc++;
                lenCount--;
            }
            HUnlock( data);
        } else MyDebugString("\pMem Error");
    }
}

void ReplaceIndStringWithIndStringInHandle( long destID, long destNum, long sourceID, long sourceNum, Handle data)
{
    Str255      sourceString, destString;

    GetIndString( destString, destID, destNum);
    GetIndString( sourceString, sourceID, sourceNum);
    CheckStringForNil( sourceString);
    Munger( data, 0, (destString + 1), *destString, sourceString + 1, *sourceString);
}

void ReplaceIndStringWithStringInHandle(long destID, long destNum, unsigned char* sourceString,
        Handle data) {
    Str255      destString;

    GetIndString( destString, destID, destNum);
    CheckStringForNil( sourceString);
    Munger( data, 0, (destString + 1), *destString, sourceString + 1, *sourceString);
}

void ReplaceIndStringWithHandleInHandle( long destID, long destNum, Handle insertData, Handle data)
{
    Str255      destString;

    CheckHandleForNil( insertData);
    HLock( insertData);
    GetIndString( destString, destID, destNum);
    Munger( data, 0, (destString + 1), *destString, *insertData, GetHandleSize( insertData));
    HUnlock( insertData);
}

void GetFileNameFromObject(long whichObject, unsigned char* name) {
    baseObjectType  *o;
    short           raceNum;
    Str255          shipName;

    o = mGetBaseObjectPtr( whichObject);
    raceNum = GetRaceNumFromID( o->baseRace);
    if ( raceNum >= 0)
        GetRaceString( name, kRaceAdjective, raceNum);
    else
        CopyPString( name, "\pmisc");
    NumToNDigitString( o->baseClass, shipName, 5);
    ConcatenatePString( name, shipName);
    GetIndString( shipName, 5001, whichObject + 1);
    ConcatenatePString( name, shipName);
    NumToNDigitString( whichObject, shipName, 3);
    ConcatenatePString( name, shipName);
    ConcatenatePString( name, "\p.html");
    CheckStringForNil( name);
}

void GetFileNameFromSprite(long whichSprite, unsigned char* name) {
    Str255  tString;

    CopyPString(name, "\psprite");
    NumToNDigitString( whichSprite, tString, 5);
    ConcatenatePString( name, tString);
    ConcatenatePString( name, "\p.gif");
}

void GetFileNameFromPortrait(long whichObject, unsigned char* name) {
    Str255  tString;

    whichObject += 1001;
    CopyPString(name, "\pportrait");
    NumToNDigitString( whichObject, tString, 5);
    ConcatenatePString( name, tString);
    ConcatenatePString( name, "\p.gif");
}

void NumToNDigitString(long num, unsigned char* s, long digits) {
/*  *c = 3;
    c++;
    *c = '0' + ( num / 100);
    c++;
    *c = '0' + (( num % 100) / 10);
    c++;
    *c = '0' + ( num % 10);
*/
    NumToString( num, s);
    if ( num >= 0)
    {
        while ( mGetAnyCharPStringLength( s) < digits)
        {
            InsertAnyCharPStringInPString(s, "\p0", 0);
        }
    }
    CheckStringForNil( s);
}

void InsertGraphText( long val, long minval, long maxval, short resID, short strNum, Handle newText)
{
    Handle  graphText;

    graphText = GetGraphText( (( val - minval) *
        kGraphWidth) / ( maxval - minval));
    if ( graphText != nil)
    {
        ReplaceIndStringWithHandleInHandle(  resID, strNum, graphText, newText);
        DisposeHandle( graphText);
    }
}

Handle GetGraphText( long val)
{
    Handle  newData = nil;
    Str255  s;

    GetIndString( s, 6454, 8);
    newData = NewHandleFromString( s);
    if ( newData == nil) return nil;
    if ( val > 0)
    {
        GetIndString( s, 6454, 9);
        AppendStringToHandle( s, newData);
        NumToString( val, s);
        AppendStringToHandle( s, newData);
        GetIndString( s, 6454, 10);
        AppendStringToHandle( s, newData);
    }

    if ( val < kGraphWidth)
    {
        GetIndString( s, 6454, 11);
        AppendStringToHandle( s, newData);
        NumToString( kGraphWidth - val, s);
        AppendStringToHandle( s, newData);
        GetIndString( s, 6454, 12);
        AppendStringToHandle( s, newData);
    }
    GetIndString( s, 6454, 13);
    AppendStringToHandle( s, newData);
    return( newData);
}

void InsertGraphicText( long whichObject, long whichSprite, Handle text)
{
    Handle      newData = nil;
    Str255      s;
    PicHandle   pic;

    if ( whichSprite >= 0)
    {
        GetIndString( s, 6454, 17);
        newData = NewHandleFromString( s);
        if ( newData == nil) return;

        GetFileNameFromSprite( whichSprite, s);
        AppendStringToHandle( s, newData);
        GetIndString( s, 6454, 18);
        AppendStringToHandle( s, newData);

        ReplaceIndStringWithHandleInHandle( 6453, 73, newData, text);
        DisposeHandle( newData);
    }
    pic = GetPicture( whichObject + 1001);
    if ( pic != nil)
    {
        GetIndString( s, 6454, 19);
        newData = NewHandleFromString( s);
        if ( newData == nil) return;

        GetFileNameFromPortrait( whichObject, s);
        AppendStringToHandle( s, newData);
        GetIndString( s, 6454, 20);
        AppendStringToHandle( s, newData);
        NumToString( (**pic).picFrame.right - (**pic).picFrame.left, s);
        AppendStringToHandle( s, newData);

        GetIndString( s, 6454, 21);
        AppendStringToHandle( s, newData);
        NumToString( (**pic).picFrame.bottom - (**pic).picFrame.top, s);
        AppendStringToHandle( s, newData);
        GetIndString( s, 6454, 22);
        AppendStringToHandle( s, newData);

        ReplaceIndStringWithHandleInHandle( 6453, 74, newData, text);
        DisposeHandle( newData);
        ReleaseResource( reinterpret_cast<Handle>(pic));
    }
}

void InsertIndexText( long whichObject, long myRace, Handle text)
{
    Handle          newData = nil;
    Str255          s;
    short           raceNum = 0, raceID;
    Boolean         raceHasName = true;
    baseObjectType  *o;
    long            lowestClass, highestClass, thisClass, nextClass, count;

    while ( raceHasName)
    {
        raceID = GetRaceIDFromNum( raceNum);
        GetIndString( s, 6454, 26); // <DT>
        if ( newData == nil)
        {
            newData = NewHandleFromString( s);
            if ( newData == nil) return;
        } else
        {
            AppendStringToHandle( s, newData);
        }
        GetIndString( s, 6454, 14); // <A HREF="
        AppendStringToHandle( s, newData);
        GetRaceString( s, kRaceAdjective, raceNum);
        AppendStringToHandle( s, newData);
        AppendStringToHandle("\p.html", newData);
        GetIndString( s, 6454, 15); // ">
        AppendStringToHandle( s, newData);
        GetRaceString( s, kRaceAdjective, raceNum);
        AppendStringToHandle( s, newData);
        GetIndString( s, 6454, 16); // </A>
        AppendStringToHandle( s, newData);

        if ( raceID == myRace)
        {
            o = *gBaseObjectData;

            lowestClass = 0x7fffffff;
            highestClass = 0;

            for ( count = 0; count < kMaxBaseObject; count++)
            {
                if (( o->internalFlags & 0x40000000) && ( o->baseRace == myRace))
                {
                    if ( o->baseClass < lowestClass) lowestClass = o->baseClass;
                    if ( o->baseClass > highestClass) highestClass = o->baseClass;
                }
                o++;
            }

            nextClass = lowestClass;
            do
            {
                thisClass = nextClass;
                nextClass = highestClass;
                o = *gBaseObjectData;
                for ( count = 0; count < kMaxBaseObject; count++)
                {
                    if (( o->internalFlags & 0x40000000) && ( o->baseRace == myRace))
                    {
                        if ( o->baseClass == thisClass)
                        {
                            if ( count != whichObject)
                            {
                                GetIndString( s, 6454, 27); // <DD>
                                AppendStringToHandle( s, newData);
                                GetIndString( s, 6454, 14); // <A HREF="
                                AppendStringToHandle( s, newData);
                                GetFileNameFromObject( count, s);
                                AppendStringToHandle( s, newData);
                                GetIndString( s, 6454, 15); // ">
                                AppendStringToHandle( s, newData);
                                GetIndString( s, 5000, count + 1);
                                AppendStringToHandle( s, newData);
                                GetIndString( s, 6454, 16); // </A>
                                AppendStringToHandle( s, newData);
                            } else
                            {
                                GetIndString( s, 6454, 27); // <DD>
                                AppendStringToHandle( s, newData);
                                GetIndString( s, 5000, count + 1);
                                AppendStringToHandle( s, newData);
                            }
                        } else
                        {
                            if (( o->baseClass > thisClass) && ( o->baseClass < nextClass))
                            {
                                nextClass = o->baseClass;
                            }
                        }
                    }
                    o++;
                }
            } while ( nextClass > thisClass);
        }

        raceNum++;
        s[0] = 0;
        GetRaceString( s, kRaceAdjective, raceNum);
        if ( s[0] == 0) raceHasName = false;
    }

    ReplaceIndStringWithHandleInHandle( 6453, 75, newData, text);
}

long GetPreviousIndexBaseObject( long whichObject)
{
    long            highestClass = -1, count, resultObject = -1;
    baseObjectType  *source, *o;
    short           previousRace;

    // 1st, look for the object of same race with the largest class _below_ our class
    source = mGetBaseObjectPtr( whichObject);

    o = *gBaseObjectData;
    for ( count = 0; count < kMaxBaseObject; count++)
    {
        if (( count != whichObject) && ( o->baseRace == source->baseRace))
        {
            if (( o->baseClass == source->baseClass) && ( count < whichObject))
            {
                highestClass = o->baseClass;
                resultObject = count;
            } else if (( o->baseClass >= highestClass) && ( o->baseClass < source->baseClass))
            {
                highestClass = o->baseClass;
                resultObject = count;
            }
        }
        o++;
    }

    // if nothing found, find LAST, HIGHEST class object of previous race
    if ( resultObject < 0)
    {
        previousRace = GetRaceNumFromID( source->baseRace);
        if ( previousRace <= 0) return ( -1);
        previousRace--;
        previousRace = GetRaceIDFromNum( previousRace);
        highestClass = -1;

        o = *gBaseObjectData;
        for ( count = 0; count < kMaxBaseObject; count++)
        {
            if (( o->baseRace == previousRace) && ( o->baseClass >= highestClass))
            {
                highestClass = o->baseClass;
                resultObject = count;
            }
            o++;
        }
    }
    return( resultObject);
}

Handle NewHandleFromString(unsigned char* s) {
    unsigned char *c, *sc, lenCount;
    Handle  data = nil;

    CheckStringForNil( s);
    sc = s;
    data = NewHandle( implicit_cast<long>(*sc));
    if ( data != nil)
    {
        HLock( data);
        c = *data;
        lenCount = *sc;
        sc++;
        while( lenCount > 0)
        {
            *c = *sc;
            c++;
            sc++;
            lenCount--;
        }
        HUnlock( data);
    } else MyDebugString("\pNil Handle");
    return( data);
}

void CheckStringForNil( const unsigned char* s)
{
    const unsigned char* c = s;
    short len = *c;

    c++;
    while ( len > 0)
    {
        if ( *c == 0) MyDebugString("\pNULL char");
        c++;
        len--;
    }
}

void CheckHandleForNil( Handle data)
{
    unsigned char* c;
    short   len;

    if ( data == nil)
    {
        MyDebugString("\pNil Data!");
        return;
    }
    HLock( data);
    c = *data;
    len = 0;
    while ( len < GetHandleSize( data))
    {
        if ( *c == 0) MyDebugString("\pNULL data");
        c++;
        len++;
    }
    HUnlock( data);
}

// the end
