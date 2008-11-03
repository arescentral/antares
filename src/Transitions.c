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

/* Transitions Color Animations.c */
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

#include "AresGlobalType.h"
#include "Quickdraw.h"
#ifndef __PALETTES__
#include <Palettes.h>
#endif
#include "Resources.h"

#include "Debug.h"
#include "AresMain.h"
#include "HandleHandling.h"

#include "ColorTranslation.h"
#include "KeyMapTranslation.h"

#include "Music.h"

#include "Transitions.h"
#include "Error.h"

//#define   kDontMessWithColors

#define kStartAnimation     -255
#define kEndAnimation       255

#define kAnimationSteps     255

#define kNoColorGoal        -1

#define kTransitionError    "\pTRAN"

extern GDHandle                 theDevice;
extern long                     gInterfaceFileRefID;

typedef struct
{
    short   reqLSize;
    short   reqLData[256];
} bigReqListRec;

extern aresGlobalType *gAresGlobal;
extern short gSpriteFileRefID;

//long  gAresGlobal->gColorAnimationStep = 0, gAresGlobal->gColorAnimationInSpeed = -1,
//      gAresGlobal->gColorAnimationOutSpeed = -1;

//CTabHandle    gAresGlobal->gColorAnimationTable = nil, gAresGlobal->gSaveColorTable = nil;
//RGBColor  gAresGlobal->gColorAnimationGoal;

void InitTransitions( void)

{
    PixMapHandle        onScreenPixMap;

    onScreenPixMap = (**theDevice).gdPMap;
    gAresGlobal->gColorAnimationTable = (**onScreenPixMap).pmTable;
    HandToHand ((Handle*) &gAresGlobal->gColorAnimationTable);
    MoveHHi( (Handle)gAresGlobal->gColorAnimationTable);
    gAresGlobal->gSaveColorTable = gAresGlobal->gColorAnimationTable;
    HandToHand ((Handle*) &gAresGlobal->gSaveColorTable);
    HLock( (Handle)gAresGlobal->gColorAnimationTable);
    MoveHHi( (Handle)gAresGlobal->gSaveColorTable);
    HLock( (Handle)gAresGlobal->gSaveColorTable);

}

void ResetTransitions( void) // for resetting the color map

{
    CleanupTransitions();
    InitTransitions();
}

void CleanupTransitions( void)

{
    if ( gAresGlobal->gColorAnimationTable != nil)
        DisposeHandle( (Handle)gAresGlobal->gColorAnimationTable);
    if ( gAresGlobal->gSaveColorTable != nil)
        DisposeHandle( (Handle)gAresGlobal->gSaveColorTable);
}

/* DitherFadePixMapToScreenPixMap

    Takes in two sources, one of which is a COPY of the screen map, and fades from the first
    to the second using B&W patterns (PAT# 500)
*/

/*
PROCEDURE GetIndPattern (VAR thePattern: Pattern;
                                 patListID: Integer; index: Integer);

void DitherFadePixMapToScreenPixMap( PixMap *sourcePix, Rect *sourceRect, PixMap *destMap)

{
    int     x, y, width, height;
    long    *sword, *dword, srowplus, drowplus, srowbytes, drowbytes, sright;
    Rect    fixRect;

    fixRect = *sourceRect;
    fixRect.left /= 4;
    if (( fixRect.right % 4) == 0)
        fixRect.right /= 4;
    else fixRect.right = fixRect.right / 4 + 1;
    srowbytes = sourcePix->rowBytes & 0x3fff;
    srowbytes /= 4;
    drowbytes = destMap->rowBytes & 0x3fff;
    drowbytes /= 4;
    sright = sourcePix->bounds.right / 4;
    if ( fixRect.left < 0)
        fixRect.left = 0;
    if ( fixRect.right > sright)
        fixRect.right = sright;
    if ( fixRect.right > drowbytes)
        fixRect.right = drowbytes;
    if ( fixRect.top < 0)
        fixRect.top = 0;
    if ( fixRect.bottom > sourcePix->bounds.bottom)
        fixRect.bottom = sourcePix->bounds.bottom;
    if ( fixRect.bottom > destMap->bounds.bottom)
        fixRect.bottom = destMap->bounds.bottom;
    srowplus = srowbytes - (fixRect.right - fixRect.left);
    drowplus = drowbytes - (fixRect.right - fixRect.left);
    sword = (long *)sourcePix->baseAddr + (long)fixRect.top * srowbytes +
            (long)fixRect.left;
    dword = (long *)destMap->baseAddr + (long)(fixRect.top + gNatePortTop) * drowbytes +
            (long)(fixRect.left + gNatePortLeft);
    for ( y = fixRect.top; y < fixRect.bottom; y++)
    {
        for ( x = fixRect.left; x < fixRect.right; x++)
        {
            *dword++ = *sword++;
        }
        dword += drowplus;
        sword += srowplus;
    }
}
*/

void StartColorAnimation( long inSpeed, long outSpeed, unsigned char goalColor)

{

    gAresGlobal->gColorAnimationStep = kStartAnimation;
    gAresGlobal->gColorAnimationInSpeed = inSpeed;
    gAresGlobal->gColorAnimationOutSpeed = outSpeed;
    GetRGBTranslateColor( &gAresGlobal->gColorAnimationGoal,  GetRetroIndex( goalColor));

}

void UpdateColorAnimation( long timePassed)

{

    int                 entryCount;
    bigReqListRec       recList;
    GDHandle            originalDevice = GetGDevice();

    #ifndef kDontMessWithColors
    SetGDevice( theDevice);
    #endif
    if ( gAresGlobal->gColorAnimationInSpeed != kNoColorGoal)
    {

        if ( gAresGlobal->gColorAnimationStep < 0)
        {
            recList.reqLSize = (**gAresGlobal->gColorAnimationTable).ctSize - 1;

            for (entryCount = 0; entryCount <= (**gAresGlobal->gColorAnimationTable).ctSize - 1; entryCount++)
            {
                (**gAresGlobal->gColorAnimationTable).ctTable[entryCount].rgb.red =
                    gAresGlobal->gColorAnimationGoal.red - (( gAresGlobal->gColorAnimationGoal.red -
                    (**gAresGlobal->gSaveColorTable).ctTable[entryCount].rgb.red) / kAnimationSteps) *
                    -gAresGlobal->gColorAnimationStep;

                (**gAresGlobal->gColorAnimationTable).ctTable[entryCount].rgb.green =
                    gAresGlobal->gColorAnimationGoal.green - (( gAresGlobal->gColorAnimationGoal.green -
                    (**gAresGlobal->gSaveColorTable).ctTable[entryCount].rgb.green) / kAnimationSteps) *
                    -gAresGlobal->gColorAnimationStep;

                (**gAresGlobal->gColorAnimationTable).ctTable[entryCount].rgb.blue =
                    gAresGlobal->gColorAnimationGoal.blue - (( gAresGlobal->gColorAnimationGoal.blue -
                    (**gAresGlobal->gSaveColorTable).ctTable[entryCount].rgb.blue) / kAnimationSteps) *
                    -gAresGlobal->gColorAnimationStep;

                recList.reqLData[entryCount] = entryCount;

            }
            #ifndef kDontMessWithColors
            RestoreEntries( gAresGlobal->gColorAnimationTable, nil, (ReqListRec *)&recList);
            #endif
            gAresGlobal->gColorAnimationStep += gAresGlobal->gColorAnimationInSpeed * timePassed;
        } else if (( gAresGlobal->gColorAnimationStep + gAresGlobal->gColorAnimationOutSpeed * timePassed) < kAnimationSteps)
        {
            recList.reqLSize = (**gAresGlobal->gColorAnimationTable).ctSize - 1;

            for (entryCount = 0; entryCount <= (**gAresGlobal->gColorAnimationTable).ctSize - 1; entryCount++)
            {
                (**gAresGlobal->gColorAnimationTable).ctTable[entryCount].rgb.red =
                    gAresGlobal->gColorAnimationGoal.red - (( gAresGlobal->gColorAnimationGoal.red -
                    (**gAresGlobal->gSaveColorTable).ctTable[entryCount].rgb.red) / kAnimationSteps) *
                    gAresGlobal->gColorAnimationStep;

                (**gAresGlobal->gColorAnimationTable).ctTable[entryCount].rgb.green =
                    gAresGlobal->gColorAnimationGoal.green - (( gAresGlobal->gColorAnimationGoal.green -
                    (**gAresGlobal->gSaveColorTable).ctTable[entryCount].rgb.green) / kAnimationSteps) *
                    gAresGlobal->gColorAnimationStep;

                (**gAresGlobal->gColorAnimationTable).ctTable[entryCount].rgb.blue =
                    gAresGlobal->gColorAnimationGoal.blue - (( gAresGlobal->gColorAnimationGoal.blue -
                    (**gAresGlobal->gSaveColorTable).ctTable[entryCount].rgb.blue) / kAnimationSteps) *
                    gAresGlobal->gColorAnimationStep;

                recList.reqLData[entryCount] = entryCount;

            }
            #ifndef kDontMessWithColors
            RestoreEntries( gAresGlobal->gColorAnimationTable, nil, (ReqListRec *)&recList);
            #endif
            gAresGlobal->gColorAnimationStep += gAresGlobal->gColorAnimationOutSpeed * timePassed;
        } else
        {
            recList.reqLSize = (**gAresGlobal->gSaveColorTable).ctSize - 1;

            for (entryCount = 0; entryCount <= (**gAresGlobal->gSaveColorTable).ctSize - 1; entryCount++)
            {
                recList.reqLData[entryCount] = entryCount;

            }
            #ifndef kDontMessWithColors
            RestoreEntries( gAresGlobal->gSaveColorTable, nil, (ReqListRec *)&recList);
            #endif
            gAresGlobal->gColorAnimationInSpeed = kNoColorGoal;
        }
    }
    #ifndef kDontMessWithColors
    SetGDevice( originalDevice);
    #endif
}

void StartBooleanColorAnimation( long inSpeed, long outSpeed, unsigned char goalColor)

{
    bigReqListRec       recList;
    int                 entryCount;
    GDHandle            originalDevice = GetGDevice();

    if ( gAresGlobal->gColorAnimationInSpeed == kNoColorGoal)
    {
        gAresGlobal->gColorAnimationStep = kStartAnimation;
        gAresGlobal->gColorAnimationInSpeed = inSpeed;
        gAresGlobal->gColorAnimationOutSpeed = outSpeed;
        GetRGBTranslateColor( &gAresGlobal->gColorAnimationGoal,  GetRetroIndex( goalColor));

        #ifndef kDontMessWithColors
        SetGDevice( theDevice);
        #endif

        recList.reqLSize = (**gAresGlobal->gColorAnimationTable).ctSize;
        for (entryCount = 0; entryCount <= (**gAresGlobal->gColorAnimationTable).ctSize; entryCount++)
        {
            (**gAresGlobal->gColorAnimationTable).ctTable[entryCount].rgb.red = (gAresGlobal->gColorAnimationGoal.red >> 1L) +
                    ((**gAresGlobal->gSaveColorTable).ctTable[entryCount].rgb.red >> 1L);

            (**gAresGlobal->gColorAnimationTable).ctTable[entryCount].rgb.green = (gAresGlobal->gColorAnimationGoal.green >> 1L) +
                    ((**gAresGlobal->gSaveColorTable).ctTable[entryCount].rgb.green >> 1L);

            (**gAresGlobal->gColorAnimationTable).ctTable[entryCount].rgb.blue = (gAresGlobal->gColorAnimationGoal.blue >> 1L) +
                    ((**gAresGlobal->gSaveColorTable).ctTable[entryCount].rgb.blue >> 1L);

            recList.reqLData[entryCount] = entryCount;

        }
        #ifndef kDontMessWithColors
        RestoreEntries( gAresGlobal->gColorAnimationTable, nil, (ReqListRec *)&recList);
        SetGDevice( originalDevice);
        #endif
    } else
    {
        gAresGlobal->gColorAnimationStep = kStartAnimation;
        gAresGlobal->gColorAnimationInSpeed = inSpeed;
        gAresGlobal->gColorAnimationOutSpeed = outSpeed;
        GetRGBTranslateColor( &gAresGlobal->gColorAnimationGoal,  GetRetroIndex( goalColor));
    }
}

void UpdateBooleanColorAnimation( long timePassed)

{

    int                 entryCount;
    bigReqListRec       recList;
    GDHandle            originalDevice = GetGDevice();

    #ifndef kDontMessWithColors
    SetGDevice( theDevice);
    #endif
    if ( gAresGlobal->gColorAnimationInSpeed != kNoColorGoal)
    {
        if ( gAresGlobal->gColorAnimationStep < 0)
        {
            gAresGlobal->gColorAnimationStep += gAresGlobal->gColorAnimationInSpeed * timePassed;
        } else if (( gAresGlobal->gColorAnimationStep + gAresGlobal->gColorAnimationOutSpeed * timePassed) < kAnimationSteps)
        {
            gAresGlobal->gColorAnimationStep += gAresGlobal->gColorAnimationOutSpeed * timePassed;
        } else
        {
            recList.reqLSize = (**gAresGlobal->gSaveColorTable).ctSize;

            for (entryCount = 0; entryCount <= (**gAresGlobal->gSaveColorTable).ctSize; entryCount++)
            {
                recList.reqLData[entryCount] = entryCount;

            }
            #ifndef kDontMessWithColors
            RestoreEntries( gAresGlobal->gSaveColorTable, nil, (ReqListRec *)&recList);
            #endif
            gAresGlobal->gColorAnimationInSpeed = kNoColorGoal;
        }
    }
    #ifndef kDontMessWithColors
    SetGDevice( originalDevice);
    #endif
}

void RestoreOriginalColors( void)
{
    GDHandle            originalDevice = GetGDevice();
    bigReqListRec       recList;
    int                 entryCount;

    #ifndef kDontMessWithColors
    SetGDevice( theDevice);
    #endif
    if ( gAresGlobal->gColorAnimationInSpeed != kNoColorGoal)
    {
        #ifndef kDontMessWithColors
        recList.reqLSize = (**gAresGlobal->gSaveColorTable).ctSize;

        for (entryCount = 0; entryCount <= (**gAresGlobal->gSaveColorTable).ctSize; entryCount++)
        {
            recList.reqLData[entryCount] = entryCount;

        }
        RestoreEntries( gAresGlobal->gSaveColorTable, nil, (ReqListRec *)&recList);
        #endif
        gAresGlobal->gColorAnimationInSpeed = kNoColorGoal;
    }
    #ifndef kDontMessWithColors
    SetGDevice( originalDevice);
    #endif
}

void InstantGoalTransition( void)   // instantly goes to total goal color

{
    bigReqListRec       recList;
    int                 entryCount;
    GDHandle            originalDevice = GetGDevice();

    #ifndef kDontMessWithColors
    SetGDevice( theDevice);
    #endif

    recList.reqLSize = (**gAresGlobal->gColorAnimationTable).ctSize;
    for (entryCount = 0; entryCount <= (**gAresGlobal->gColorAnimationTable).ctSize; entryCount++)
    {
        (**gAresGlobal->gColorAnimationTable).ctTable[entryCount].rgb.red = gAresGlobal->gColorAnimationGoal.red;

        (**gAresGlobal->gColorAnimationTable).ctTable[entryCount].rgb.green = gAresGlobal->gColorAnimationGoal.green;

        (**gAresGlobal->gColorAnimationTable).ctTable[entryCount].rgb.blue = gAresGlobal->gColorAnimationGoal.blue;

        recList.reqLData[entryCount] = entryCount;

    }
    #ifndef kDontMessWithColors
    RestoreEntries( gAresGlobal->gColorAnimationTable, nil, (ReqListRec *)&recList);
    SetGDevice( originalDevice);
    #endif
}

Boolean AutoFadeTo( long tickTime, RGBColor *goalColor, Boolean eventSkip)

{
    long        startTime, thisTime = 0, lastStep = 0, thisStep = 0;
    Boolean     anyEventHappened = gAresGlobal->returnToMain;

    gAresGlobal->gColorAnimationStep = kStartAnimation;
    gAresGlobal->gColorAnimationInSpeed = 1;
    gAresGlobal->gColorAnimationOutSpeed = gAresGlobal->gColorAnimationInSpeed;
    gAresGlobal->gColorAnimationGoal = *goalColor;
    startTime = TickCount();
    while (( gAresGlobal->gColorAnimationStep < 0) && ( !anyEventHappened))
    {
        thisTime = TickCount() - startTime;
        thisStep = kAnimationSteps * thisTime;
        thisStep /= tickTime;
        UpdateColorAnimation( thisStep - lastStep);

        lastStep = thisStep;

        if ( eventSkip)
            anyEventHappened = AnyEvent();
    }
    InstantGoalTransition();
    gAresGlobal->gColorAnimationStep = 0;
    return( anyEventHappened);
}

Boolean AutoFadeFrom( long tickTime, Boolean eventSkip) // assumes you've set up with AutoFadeTo

{
    long        startTime, thisTime = 0, lastStep = 0, thisStep = 0;
    Boolean         anyEventHappened = gAresGlobal->returnToMain;

    gAresGlobal->gColorAnimationOutSpeed = 1;
    startTime = TickCount();

    while ( gAresGlobal->gColorAnimationInSpeed != kNoColorGoal && ( !anyEventHappened))
    {
        thisTime = TickCount() - startTime;
        thisStep = kAnimationSteps * thisTime;
        thisStep /= tickTime;
        UpdateColorAnimation( thisStep - lastStep);

        lastStep = thisStep;

        if ( eventSkip)
            anyEventHappened = AnyEvent();
    }
    gAresGlobal->gColorAnimationStep = kEndAnimation;
    UpdateColorAnimation( 1);

    return( anyEventHappened);
}

Boolean AutoMusicFadeTo( long tickTime, RGBColor *goalColor, Boolean eventSkip)

{
    long        startTime, thisTime = 0, lastStep = 0, thisStep = 0, musicVol, musicStep;
    Boolean     anyEventHappened = gAresGlobal->returnToMain;

    gAresGlobal->gColorAnimationStep = kStartAnimation;
    gAresGlobal->gColorAnimationInSpeed = 1;
    gAresGlobal->gColorAnimationOutSpeed = gAresGlobal->gColorAnimationInSpeed;
    gAresGlobal->gColorAnimationGoal = *goalColor;
    musicVol = GetSongVolume();
    if ( musicVol > 0)
        musicStep = kAnimationSteps / musicVol + 1;
    else musicStep = 1;

    startTime = TickCount();

    while (( gAresGlobal->gColorAnimationStep < 0) && ( !anyEventHappened))
    {
        thisTime = TickCount() - startTime;
        thisStep = kAnimationSteps * thisTime;
        thisStep /= tickTime;
        UpdateColorAnimation( thisStep - lastStep);
        musicVol = (-gAresGlobal->gColorAnimationStep) / musicStep;
        if ( musicVol > kMaxMusicVolume) musicVol = kMaxMusicVolume;
        else if ( musicVol < 0) musicVol = 0;
        SetSongVolume( musicVol);

        lastStep = thisStep;

        if ( eventSkip)
            anyEventHappened = AnyEvent();
    }
    InstantGoalTransition();
    gAresGlobal->gColorAnimationStep = 0;
    StopAndUnloadSong();
    return( anyEventHappened);
}

/* CustomPictFade:
blackens the window; sets color table to clutD; draws pict resource pictID; fades from
black to pict; holds pict; fades to black; restores orignal palette; stops & returns true
if any key pressed.
>>> YOU SHOULD PROBABLY CALL RESETTRANSITIONS AFTER CALLING THIS since it could screw up
the color translation table.
*/

Boolean CustomPictFade( long fadeSpeed, long holdTime, short pictID, short clutID,
        WindowPtr aWindow)

{
    Boolean                     done = false;
    CTabHandle                  theClut = nil;
    PaletteHandle               thePalette = nil, originalPalette = nil;
    PicHandle                   thePict = nil;
    Rect                        pictRect;
    RGBColor                    fadeColor = {0, 0, 0};
    Boolean                     gotAnyEvent = false;
    short                       oldResFile = CurResFile();

#pragma unused( fadeSpeed, holdTime)

    UseResFile( gSpriteFileRefID);

    MacFillRect( &(aWindow->portRect), &(qd.black));
    theClut = GetCTable( clutID);
    if ( theClut == nil)
    {
        ShowErrorAny( eContinueOnlyErr, kErrorStrID, nil, nil, nil, nil, kLoadColorTableError, -1, -1, -1, __FILE__, 1);
        UseResFile( oldResFile);
        return( true);
    }

    thePalette = NewPalette( (**theClut).ctSize, theClut, pmExplicit + pmTolerant, 0);
    if ( thePalette == nil)
    {
        ShowErrorAny( eContinueOnlyErr, kErrorStrID, nil, nil, nil, nil, kCreatePaletteError, -1, -1, -1, __FILE__, 2);
        UseResFile( oldResFile);
        return( true);
    }

    originalPalette = GetPalette( aWindow);
    if ( originalPalette != nil)
    {
        SetPalette( (WindowPtr)aWindow, thePalette, false);
        ActivatePalette( (WindowPtr)aWindow);
    } else
    {
        ShowErrorAny( eContinueOnlyErr, kErrorStrID, nil, nil, nil, nil, kGetPaletteError, -1, -1, -1, __FILE__, 3);
        UseResFile( oldResFile);
        return( true);
    }

    thePict = GetPicture( pictID);
    if ( thePict == nil)
    {
        ShowErrorAny( eContinueOnlyErr, kErrorStrID, nil, nil, nil, nil, kLoadPictError, -1, -1, -1, __FILE__, 4);
        UseResFile( oldResFile);
        return( true);
    }
    UseResFile( oldResFile);

    pictRect = (**thePict).picFrame;

    MacOffsetRect (&pictRect, ((aWindow->portRect.right - aWindow->portRect.left) / 2) -
        ((pictRect.right - pictRect.left) / 2),
        ((aWindow->portRect.bottom - aWindow->portRect.top) / 2) -
        ((pictRect.bottom - pictRect.top) / 2));

    HideCursor();
    ResetTransitions();
    AutoFadeTo( 1, &fadeColor, TRUE);
    DrawPicture( thePict, &pictRect);
    if ( thePict != nil) ReleaseResource( (Handle)thePict);
    gotAnyEvent = AutoFadeFrom( 100, TRUE);
    if ( !gotAnyEvent) gotAnyEvent = TimedWaitForAnyEvent(80);
    if ( !gotAnyEvent) gotAnyEvent = AutoFadeTo( 100, &fadeColor, true);
    else AutoFadeTo( 1, &fadeColor, true);
    MacFillRect( &(aWindow->portRect), &(qd.black));
    AutoFadeFrom( 1, TRUE);

    MacShowCursor();
    if ( theClut != nil) DisposeCTable( theClut);
    if ( originalPalette != nil)
    {
        SetPalette( (WindowPtr)aWindow, originalPalette, false);
        ActivatePalette( (WindowPtr)aWindow);
    }
    if ( thePalette != nil) DisposePalette( thePalette);
    ResetTransitions();

    return (gotAnyEvent);
}

Boolean StartCustomPictFade( long fadeSpeed, long holdTime, short pictID, short clutID,
        WindowPtr aWindow, PaletteHandle *thePalette,
        PaletteHandle *originalPalette,
        CTabHandle *theClut,
        Boolean fast)

{
    Boolean                     done = false;
    PicHandle                   thePict = nil;
    Rect                        pictRect;
    RGBColor                    fadeColor = {0, 0, 0};
    Boolean                     gotAnyEvent = false;
    short                       oldResFile = CurResFile();

#pragma unused( fadeSpeed, holdTime)

    MacFillRect( &(aWindow->portRect), &(qd.black));
    *theClut = GetCTable( clutID);

    if ( *theClut == nil)
    {
        ShowErrorAny( eContinueOnlyErr, kErrorStrID, nil, nil, nil, nil, kLoadColorTableError, -1, -1, -1, __FILE__, 1);
        return( true);
    }

    *thePalette = NewPalette( (***theClut).ctSize, *theClut,
        pmExplicit + pmTolerant, 0);
    if ( thePalette == nil)
    {
        ShowErrorAny( eContinueOnlyErr, kErrorStrID, nil, nil, nil, nil, kCreatePaletteError, -1, -1, -1, __FILE__, 2);
        return( true);
    }

    *originalPalette = GetPalette( aWindow);
    if ( *originalPalette != nil)
    {
        SetPalette( (WindowPtr)aWindow, *thePalette, false);
        ActivatePalette( (WindowPtr)aWindow);
    } else
    {
        ShowErrorAny( eContinueOnlyErr, kErrorStrID, nil, nil, nil, nil,
            kGetPaletteError, -1, -1, -1, __FILE__, 3);
        return( true);
    }

    thePict = GetPicture( pictID);
    if ( thePict == nil)
    {
        ShowErrorAny( eContinueOnlyErr, kErrorStrID, nil, nil, nil, nil, kLoadPictError, -1, -1, -1, __FILE__, 4);
        return( true);
    }

    pictRect = (**thePict).picFrame;

    MacOffsetRect (&pictRect, ((aWindow->portRect.right - aWindow->portRect.left) / 2) -
        ((pictRect.right - pictRect.left) / 2),
        ((aWindow->portRect.bottom - aWindow->portRect.top) / 2) -
        ((pictRect.bottom - pictRect.top) / 2));

    HideCursor();
    ResetTransitions();
    AutoFadeTo( 1, &fadeColor, TRUE);
    SetPalette( (WindowPtr)aWindow, *thePalette, false);
    ActivatePalette( (WindowPtr)aWindow);
    DrawPicture( thePict, &pictRect);
    if ( thePict != nil) ReleaseResource( (Handle)thePict);
    gotAnyEvent = AutoFadeFrom( fast?20:100, TRUE);
    if ( fast) return true;
    return( gotAnyEvent);
}

Boolean EndCustomPictFade( WindowPtr aWindow, PaletteHandle *thePalette,
    PaletteHandle *originalPalette, CTabHandle *theClut, Boolean fast)

{
    Boolean     gotAnyEvent;
    RGBColor    fadeColor = {0, 0, 0};

    gotAnyEvent = TimedWaitForAnyEvent(fast?60:60);
    if ( !gotAnyEvent) gotAnyEvent = AutoFadeTo( fast?20:100, &fadeColor, true);
    else AutoFadeTo( 1, &fadeColor, true);
    RGBForeColor( &fadeColor);
    PaintRect( &(aWindow->portRect));
    AutoFadeFrom( 1, TRUE);

    if ( *theClut != nil) DisposeCTable( *theClut);
    if ( *originalPalette != nil)
    {
        SetPalette( (WindowPtr)aWindow, *originalPalette, false);
        ActivatePalette( (WindowPtr)aWindow);
    }
    if ( *thePalette != nil) DisposePalette( *thePalette);
    ResetTransitions();
    if ( fast) return true;
    return (gotAnyEvent);
}
