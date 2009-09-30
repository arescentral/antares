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

// Transitions Color Animations.c

#include "Transitions.hpp"

#include <ConditionalMacros.h>
#include <Quickdraw.h>
#include <Resources.h>

#include "AresGlobalType.hpp"
#include "AresMain.hpp"
#include "ColorTable.hpp"
#include "ColorTranslation.hpp"
#include "Debug.hpp"
#include "Error.hpp"
#include "FakeDrawing.hpp"
#include "KeyMapTranslation.hpp"
#include "Music.hpp"
#include "Picture.hpp"

//#define   kDontMessWithColors

#define kStartAnimation     -255
#define kEndAnimation       255

#define kAnimationSteps     255

#define kNoColorGoal        -1

#define kTransitionError    "\pTRAN"

extern long gInterfaceFileRefID;
extern short gSpriteFileRefID;
extern Window fakeWindow;

struct bigReqListRec {
    short   reqLSize;
    short   reqLData[256];
};

void InitTransitions() {
    PixMap* onScreenPixMap = &fakeWindow.portBits;
    globals()->gColorAnimationTable.reset(onScreenPixMap->colors->clone());
    globals()->gSaveColorTable.reset(onScreenPixMap->colors->clone());
}

void ResetTransitions( void) // for resetting the color map

{
    CleanupTransitions();
    InitTransitions();
}

void CleanupTransitions() {
    globals()->gColorAnimationTable.reset();
    globals()->gSaveColorTable.reset();
}

// DitherFadePixMapToScreenPixMap
//
//  Takes in two sources, one of which is a COPY of the screen map, and fades from the first
//  to the second using B&W patterns (PAT# 500)

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

    globals()->gColorAnimationStep = kStartAnimation;
    globals()->gColorAnimationInSpeed = inSpeed;
    globals()->gColorAnimationOutSpeed = outSpeed;
    GetRGBTranslateColor( &globals()->gColorAnimationGoal,  GetRetroIndex( goalColor));

}

void UpdateColorAnimation( long timePassed)

{
    if ( globals()->gColorAnimationInSpeed != kNoColorGoal)
    {

        if ( globals()->gColorAnimationStep < 0)
        {
            for (size_t i = 0; i < globals()->gColorAnimationTable->size(); ++i) {
                RGBColor color = {
                    globals()->gColorAnimationGoal.red
                        - ((globals()->gColorAnimationGoal.red
                                    - globals()->gSaveColorTable->color(i).red)
                                / kAnimationSteps) *
                        -globals()->gColorAnimationStep,

                    globals()->gColorAnimationGoal.green
                        - ((globals()->gColorAnimationGoal.green
                                    - globals()->gSaveColorTable->color(i).green)
                            / kAnimationSteps) *
                        -globals()->gColorAnimationStep,

                    globals()->gColorAnimationGoal.blue
                        - ((globals()->gColorAnimationGoal.blue
                                    - globals()->gSaveColorTable->color(i).blue)
                            / kAnimationSteps) *
                        -globals()->gColorAnimationStep,
                };
                globals()->gColorAnimationTable->set_color(i, color);
            }
            RestoreEntries(*globals()->gColorAnimationTable);
            globals()->gColorAnimationStep += globals()->gColorAnimationInSpeed * timePassed;
        } else if (( globals()->gColorAnimationStep + globals()->gColorAnimationOutSpeed * timePassed) < kAnimationSteps)
        {
            for (size_t i = 0; i < globals()->gColorAnimationTable->size(); ++i) {
                RGBColor color = {
                    globals()->gColorAnimationGoal.red - (( globals()->gColorAnimationGoal.red -
                    globals()->gSaveColorTable->color(i).red) / kAnimationSteps) *
                    globals()->gColorAnimationStep,

                    globals()->gColorAnimationGoal.green - (( globals()->gColorAnimationGoal.green -
                    globals()->gSaveColorTable->color(i).green) / kAnimationSteps) *
                    globals()->gColorAnimationStep,

                    globals()->gColorAnimationGoal.blue - (( globals()->gColorAnimationGoal.blue -
                    globals()->gSaveColorTable->color(i).blue) / kAnimationSteps) *
                    globals()->gColorAnimationStep,
                };

                globals()->gColorAnimationTable->set_color(i, color);
            }
            RestoreEntries(*globals()->gColorAnimationTable);
            globals()->gColorAnimationStep += globals()->gColorAnimationOutSpeed * timePassed;
        } else
        {
            RestoreEntries(*globals()->gSaveColorTable);
            globals()->gColorAnimationInSpeed = kNoColorGoal;
        }
    }
}

void StartBooleanColorAnimation( long inSpeed, long outSpeed, unsigned char goalColor)

{
    if ( globals()->gColorAnimationInSpeed == kNoColorGoal)
    {
        globals()->gColorAnimationStep = kStartAnimation;
        globals()->gColorAnimationInSpeed = inSpeed;
        globals()->gColorAnimationOutSpeed = outSpeed;
        GetRGBTranslateColor( &globals()->gColorAnimationGoal,  GetRetroIndex( goalColor));

        for (size_t i = 0; i < globals()->gColorAnimationTable->size(); ++i) {
            RGBColor color = {
                (globals()->gColorAnimationGoal.red >> 1L) +
                    (globals()->gSaveColorTable->color(i).red >> 1L),
                (globals()->gColorAnimationGoal.green >> 1L) +
                    (globals()->gSaveColorTable->color(i).green >> 1L),
                (globals()->gColorAnimationGoal.blue >> 1L) +
                    (globals()->gSaveColorTable->color(i).blue >> 1L),
            };
            globals()->gColorAnimationTable->set_color(i, color);
        }
        RestoreEntries(*globals()->gColorAnimationTable);
    } else
    {
        globals()->gColorAnimationStep = kStartAnimation;
        globals()->gColorAnimationInSpeed = inSpeed;
        globals()->gColorAnimationOutSpeed = outSpeed;
        GetRGBTranslateColor( &globals()->gColorAnimationGoal,  GetRetroIndex( goalColor));
    }
}

void UpdateBooleanColorAnimation( long timePassed)

{
    if ( globals()->gColorAnimationInSpeed != kNoColorGoal)
    {
        if ( globals()->gColorAnimationStep < 0)
        {
            globals()->gColorAnimationStep += globals()->gColorAnimationInSpeed * timePassed;
        } else if (( globals()->gColorAnimationStep + globals()->gColorAnimationOutSpeed * timePassed) < kAnimationSteps)
        {
            globals()->gColorAnimationStep += globals()->gColorAnimationOutSpeed * timePassed;
        } else
        {
            RestoreEntries(*globals()->gSaveColorTable);
            globals()->gColorAnimationInSpeed = kNoColorGoal;
        }
    }
}

void RestoreOriginalColors( void)
{
    if ( globals()->gColorAnimationInSpeed != kNoColorGoal)
    {
        RestoreEntries(*globals()->gSaveColorTable);
        globals()->gColorAnimationInSpeed = kNoColorGoal;
    }
}

void InstantGoalTransition( void)   // instantly goes to total goal color

{
    for (size_t i = 0; i < globals()->gColorAnimationTable->size(); ++i) {
        globals()->gColorAnimationTable->set_color(i, globals()->gColorAnimationGoal);
    }
    RestoreEntries(*globals()->gColorAnimationTable);
}

Boolean AutoFadeTo( long tickTime, RGBColor *goalColor, Boolean eventSkip)

{
    long        startTime, thisTime = 0, lastStep = 0, thisStep = 0;
    Boolean     anyEventHappened = globals()->returnToMain;

    globals()->gColorAnimationStep = kStartAnimation;
    globals()->gColorAnimationInSpeed = 1;
    globals()->gColorAnimationOutSpeed = globals()->gColorAnimationInSpeed;
    globals()->gColorAnimationGoal = *goalColor;
    startTime = TickCount();
    while (( globals()->gColorAnimationStep < 0) && ( !anyEventHappened))
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
    globals()->gColorAnimationStep = 0;
    return( anyEventHappened);
}

Boolean AutoFadeFrom( long tickTime, Boolean eventSkip) // assumes you've set up with AutoFadeTo

{
    long        startTime, thisTime = 0, lastStep = 0, thisStep = 0;
    Boolean         anyEventHappened = globals()->returnToMain;

    globals()->gColorAnimationOutSpeed = 1;
    startTime = TickCount();

    while ( globals()->gColorAnimationInSpeed != kNoColorGoal && ( !anyEventHappened))
    {
        thisTime = TickCount() - startTime;
        thisStep = kAnimationSteps * thisTime;
        thisStep /= tickTime;
        UpdateColorAnimation( thisStep - lastStep);

        lastStep = thisStep;

        if ( eventSkip)
            anyEventHappened = AnyEvent();
    }
    globals()->gColorAnimationStep = kEndAnimation;
    UpdateColorAnimation( 1);

    return( anyEventHappened);
}

Boolean AutoMusicFadeTo( long tickTime, RGBColor *goalColor, Boolean eventSkip)

{
    long        startTime, thisTime = 0, lastStep = 0, thisStep = 0, musicVol, musicStep;
    Boolean     anyEventHappened = globals()->returnToMain;

    globals()->gColorAnimationStep = kStartAnimation;
    globals()->gColorAnimationInSpeed = 1;
    globals()->gColorAnimationOutSpeed = globals()->gColorAnimationInSpeed;
    globals()->gColorAnimationGoal = *goalColor;
    musicVol = GetSongVolume();
    if ( musicVol > 0)
        musicStep = kAnimationSteps / musicVol + 1;
    else musicStep = 1;

    startTime = TickCount();

    while (( globals()->gColorAnimationStep < 0) && ( !anyEventHappened))
    {
        thisTime = TickCount() - startTime;
        thisStep = kAnimationSteps * thisTime;
        thisStep /= tickTime;
        UpdateColorAnimation( thisStep - lastStep);
        musicVol = (-globals()->gColorAnimationStep) / musicStep;
        if ( musicVol > kMaxMusicVolume) musicVol = kMaxMusicVolume;
        else if ( musicVol < 0) musicVol = 0;
        SetSongVolume( musicVol);

        lastStep = thisStep;

        if ( eventSkip)
            anyEventHappened = AnyEvent();
    }
    InstantGoalTransition();
    globals()->gColorAnimationStep = 0;
    StopAndUnloadSong();
    return( anyEventHappened);
}

// CustomPictFade:
// blackens the window; sets color table to clutD; draws pict resource pictID; fades from
// black to pict; holds pict; fades to black; restores orignal palette; stops & returns true
// if any key pressed.
// >>> YOU SHOULD PROBABLY CALL RESETTRANSITIONS AFTER CALLING THIS since it could screw up
// the color translation table.

Boolean CustomPictFade( long fadeSpeed, long holdTime, short pictID, short clutID,
        WindowPtr aWindow)

{
    scoped_ptr<ColorTable> theClut;
    scoped_ptr<Picture> thePict;
    Rect                        pictRect;
    RGBColor                    fadeColor = {0, 0, 0};
    Boolean                     gotAnyEvent = false;
    short                       oldResFile = CurResFile();

#pragma unused( fadeSpeed, holdTime)

    UseResFile( gSpriteFileRefID);

    ClearScreen();
    theClut.reset(new ColorTable(clutID));
    if (theClut.get() == nil) {
        ShowErrorAny( eContinueOnlyErr, kErrorStrID, nil, nil, nil, nil, kLoadColorTableError, -1, -1, -1, __FILE__, 1);
        UseResFile( oldResFile);
        return( true);
    }

    thePict.reset(new Picture(pictID));
    if (thePict.get() == nil) {
        ShowErrorAny( eContinueOnlyErr, kErrorStrID, nil, nil, nil, nil, kLoadPictError, -1, -1, -1, __FILE__, 4);
        UseResFile( oldResFile);
        return( true);
    }
    UseResFile( oldResFile);

    pictRect = thePict->frame();

    MacOffsetRect (&pictRect, ((aWindow->portRect.right - aWindow->portRect.left) / 2) -
        ((pictRect.right - pictRect.left) / 2),
        ((aWindow->portRect.bottom - aWindow->portRect.top) / 2) -
        ((pictRect.bottom - pictRect.top) / 2));

    HideCursor();
    ResetTransitions();
    AutoFadeTo( 1, &fadeColor, TRUE);
    thePict->draw(pictRect);
    thePict.reset();

    gotAnyEvent = AutoFadeFrom( 100, TRUE);
    if ( !gotAnyEvent) gotAnyEvent = TimedWaitForAnyEvent(80);
    if ( !gotAnyEvent) gotAnyEvent = AutoFadeTo( 100, &fadeColor, true);
    else AutoFadeTo( 1, &fadeColor, true);
    ClearScreen();
    AutoFadeFrom( 1, TRUE);

    MacShowCursor();
    ResetTransitions();

    return (gotAnyEvent);
}

bool StartCustomPictFade(long fadeSpeed, long holdTime, short pictID, short clutID,
        Window* aWindow, bool fast) {
    scoped_ptr<Picture> thePict;
    scoped_ptr<ColorTable> theClut;
    Rect                        pictRect;
    RGBColor                    fadeColor = {0, 0, 0};
    Boolean                     gotAnyEvent = false;

#pragma unused( fadeSpeed, holdTime)

    ClearScreen();
    theClut.reset(new ColorTable(clutID));

    thePict.reset(new Picture(pictID));
    if (thePict.get() == nil) {
        ShowErrorAny( eContinueOnlyErr, kErrorStrID, nil, nil, nil, nil, kLoadPictError, -1, -1, -1, __FILE__, 4);
        return( true);
    }

    pictRect = thePict->frame();

    MacOffsetRect (&pictRect, ((aWindow->portRect.right - aWindow->portRect.left) / 2) -
        ((pictRect.right - pictRect.left) / 2),
        ((aWindow->portRect.bottom - aWindow->portRect.top) / 2) -
        ((pictRect.bottom - pictRect.top) / 2));

    HideCursor();
    ResetTransitions();
    AutoFadeTo( 1, &fadeColor, TRUE);
    thePict->draw(pictRect);
    thePict.reset();

    gotAnyEvent = AutoFadeFrom( fast?20:100, TRUE);
    if ( fast) return true;
    return( gotAnyEvent);
}

bool EndCustomPictFade(Window* aWindow, bool fast) {
    Boolean     gotAnyEvent;
    RGBColor    fadeColor = {0, 0, 0};

    gotAnyEvent = TimedWaitForAnyEvent(fast?60:60);
    if ( !gotAnyEvent) gotAnyEvent = AutoFadeTo( fast?20:100, &fadeColor, true);
    else AutoFadeTo( 1, &fadeColor, true);
    RGBForeColor( &fadeColor);
    PaintRect( &(aWindow->portRect));
    AutoFadeFrom( 1, TRUE);

    ResetTransitions();
    if ( fast) return true;
    return (gotAnyEvent);
}
