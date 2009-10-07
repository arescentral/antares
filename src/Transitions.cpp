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

namespace antares {

#define kStartAnimation     -255
#define kEndAnimation       255

#define kAnimationSteps     255

#define kNoColorGoal        -1

void InitTransitions() {
    globals()->gColorAnimationTable.reset(gRealWorld->colors->clone());
    globals()->gSaveColorTable.reset(gRealWorld->colors->clone());
}

void ResetTransitions() {
    InitTransitions();
}

void CleanupTransitions() {
    globals()->gColorAnimationTable.reset();
    globals()->gSaveColorTable.reset();
}

void StartColorAnimation( long inSpeed, long outSpeed, unsigned char goalColor) {
    globals()->gColorAnimationStep = kStartAnimation;
    globals()->gColorAnimationInSpeed = inSpeed;
    globals()->gColorAnimationOutSpeed = outSpeed;
    GetRGBTranslateColor( &globals()->gColorAnimationGoal,  GetRetroIndex( goalColor));
}

void UpdateColorAnimation(long timePassed) {
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

void StartBooleanColorAnimation(long inSpeed, long outSpeed, unsigned char goalColor) {
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

void UpdateBooleanColorAnimation(long timePassed) {
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

void RestoreOriginalColors() {
    if ( globals()->gColorAnimationInSpeed != kNoColorGoal)
    {
        RestoreEntries(*globals()->gSaveColorTable);
        globals()->gColorAnimationInSpeed = kNoColorGoal;
    }
}

void InstantGoalTransition() {  // instantly goes to total goal color
    for (size_t i = 0; i < globals()->gColorAnimationTable->size(); ++i) {
        globals()->gColorAnimationTable->set_color(i, globals()->gColorAnimationGoal);
    }
    RestoreEntries(*globals()->gColorAnimationTable);
}

bool AutoFadeTo(long tickTime, RGBColor *goalColor, bool eventSkip) {
    long        startTime, thisTime = 0, lastStep = 0, thisStep = 0;
    bool     anyEventHappened = false;

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

bool AutoFadeFrom(long tickTime, bool eventSkip) { // assumes you've set up with AutoFadeTo
    long        startTime, thisTime = 0, lastStep = 0, thisStep = 0;
    bool         anyEventHappened = false;

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

bool AutoMusicFadeTo(long tickTime, RGBColor *goalColor, bool eventSkip) {
    long        startTime, thisTime = 0, lastStep = 0, thisStep = 0, musicVol, musicStep;
    bool     anyEventHappened = false;

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

bool CustomPictFade(short pictID, short clutID) {
    ColorTable colors(clutID);
    Picture pict(pictID);
    RGBColor fadeColor = {0, 0, 0};

    ClearScreen();
    Rect pictRect = pict.frame();
    pictRect.center_in(gRealWorld->bounds);

    ResetTransitions();
    AutoFadeTo(1, &fadeColor, true);
    pict.draw(pictRect);

    bool gotAnyEvent = AutoFadeFrom(100, true);
    if (!gotAnyEvent) {
        gotAnyEvent = TimedWaitForAnyEvent(80);
    }
    if (!gotAnyEvent) {
        gotAnyEvent = AutoFadeTo(100, &fadeColor, true);
    } else {
        AutoFadeTo(1, &fadeColor, true);
    }

    ClearScreen();
    AutoFadeFrom(1, true);
    ResetTransitions();
    return gotAnyEvent;
}

bool StartCustomPictFade(short pictID, short clutID, bool fast) {
    ColorTable colors(clutID);
    Picture pict(pictID);
    RGBColor fadeColor = {0, 0, 0};

    ClearScreen();
    Rect pictRect = pict.frame();
    pictRect.center_in(gRealWorld->bounds);

    ResetTransitions();
    AutoFadeTo(1, &fadeColor, true);
    pict.draw(pictRect);

    return AutoFadeFrom(fast ? 20 : 100, true) || fast;
}

bool EndCustomPictFade(bool fast) {
    RGBColor fadeColor = {0, 0, 0};

    bool gotAnyEvent = TimedWaitForAnyEvent(fast ? 60 : 300);
    if (!gotAnyEvent) {
        gotAnyEvent = AutoFadeTo(fast ? 20 : 100, &fadeColor, true);
    } else {
        AutoFadeTo(1, &fadeColor, true);
    }

    ClearScreen();
    AutoFadeFrom(1, true);
    ResetTransitions();
    return fast || gotAnyEvent;
}

namespace {

double now() {
    uint64_t usec;
    Microseconds(&usec);
    return usec / 1000000.0;
}

}  // namespace

PictFade::PictFade(int pict_id, int clut_id, bool* skipped)
        : _state(NEW),
          _pict_id(pict_id),
          _transition_colors(clut_id),
          _current_colors(clut_id),
          _skipped(skipped) { }


void PictFade::become_front() {
    *_skipped = false;

    RGBColor black = {0, 0, 0};
    _current_colors.transition_between(_transition_colors, black, 1.0);
    RestoreEntries(_current_colors);

    ClearScreen();
    Picture pict(_pict_id);
    Rect pictRect = pict.frame();
    pictRect.center_in(gRealWorld->bounds);
    pict.draw(pictRect);

    _state = WAXING;
    _wax_start = now();
    _wax_duration = 5.0 / 3.0;
    _wane_start = _wax_start + 3.0;
    _wane_duration = 5.0 / 3.0;
}

void PictFade::resign_front() {
    ClearScreen();
    RestoreEntries(*globals()->gSaveColorTable);
}

bool PictFade::mouse_down(int button, const Point& loc) {
    (void)button;
    (void)loc;
    *_skipped = true;
    VideoDriver::driver()->pop_listener(this);
    return true;
}

double PictFade::delay() {
    double tick = 1.0 / 60.0;
    switch (_state) {
      case WAXING:
      case WANING:
        return tick;
      case FULL:
        return std::max(_wane_start - now(), tick);
      default:
        return 0.0;
    }
}

void PictFade::fire_timer() {
    RGBColor black = {0, 0, 0};
    switch (_state) {
      case WAXING:
        {
            double fraction = 1.0 - ((now() - _wax_start) / _wax_duration);
            if (fraction > 0.0) {
                _current_colors.transition_between(_transition_colors, black, fraction);
                RestoreEntries(_current_colors);
            } else {
                _state = FULL;
                RestoreEntries(_transition_colors);
            }
        }
        break;

      case FULL:
        _state = WANING;
        break;

      case WANING:
        {
            double fraction = (now() - _wane_start) / _wane_duration;
            if (fraction < 1.0) {
                _current_colors.transition_between(_transition_colors, black, fraction);
                RestoreEntries(_current_colors);
            } else {
                VideoDriver::driver()->pop_listener(this);
            }
        }
        break;

      default:
        break;
    }
}

}  // namespace antares
