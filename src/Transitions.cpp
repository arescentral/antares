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

#include "Quickdraw.h"

#include "AresGlobalType.hpp"
#include "AresMain.hpp"
#include "CardStack.hpp"
#include "ColorTable.hpp"
#include "ColorTranslation.hpp"
#include "Debug.hpp"
#include "Error.hpp"
#include "FakeDrawing.hpp"
#include "KeyMapTranslation.hpp"
#include "Music.hpp"
#include "Picture.hpp"
#include "Time.hpp"

namespace antares {

#define kStartAnimation     -255
#define kEndAnimation       255

#define kAnimationSteps     255

#define kNoColorGoal        -1

extern PixMap* gActiveWorld;

void InitTransitions() {
    globals()->gColorAnimationTable.reset(gRealWorld->colors().clone());
    globals()->gSaveColorTable.reset(gRealWorld->colors().clone());
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
                RgbColor color(
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
                        -globals()->gColorAnimationStep);
                globals()->gColorAnimationTable->set_color(i, color);
            }
            RestoreEntries(*globals()->gColorAnimationTable);
            globals()->gColorAnimationStep += globals()->gColorAnimationInSpeed * timePassed;
        } else if (( globals()->gColorAnimationStep + globals()->gColorAnimationOutSpeed * timePassed) < kAnimationSteps)
        {
            for (size_t i = 0; i < globals()->gColorAnimationTable->size(); ++i) {
                RgbColor color(
                    globals()->gColorAnimationGoal.red - (( globals()->gColorAnimationGoal.red -
                    globals()->gSaveColorTable->color(i).red) / kAnimationSteps) *
                    globals()->gColorAnimationStep,

                    globals()->gColorAnimationGoal.green - (( globals()->gColorAnimationGoal.green -
                    globals()->gSaveColorTable->color(i).green) / kAnimationSteps) *
                    globals()->gColorAnimationStep,

                    globals()->gColorAnimationGoal.blue - (( globals()->gColorAnimationGoal.blue -
                    globals()->gSaveColorTable->color(i).blue) / kAnimationSteps) *
                    globals()->gColorAnimationStep);

                globals()->gColorAnimationTable->set_color(i, color);
            }
            RestoreEntries(*globals()->gColorAnimationTable);
            globals()->gColorAnimationStep += globals()->gColorAnimationOutSpeed * timePassed;
        } else
        {
            gRealWorld->set_transition_fraction(0.0);
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
        gRealWorld->set_transition_fraction(0.5);
        gRealWorld->set_transition_to(globals()->gColorAnimationGoal);
    } else
    {
        globals()->gColorAnimationStep = kStartAnimation;
        globals()->gColorAnimationInSpeed = inSpeed;
        globals()->gColorAnimationOutSpeed = outSpeed;
        GetRGBTranslateColor( &globals()->gColorAnimationGoal,  GetRetroIndex( goalColor));
        gRealWorld->set_transition_to(globals()->gColorAnimationGoal);
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
            gRealWorld->set_transition_fraction(0.0);
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

bool AutoFadeTo(long tickTime, const RgbColor& goalColor, bool eventSkip) {
    long        startTime, thisTime = 0, lastStep = 0, thisStep = 0;
    bool     anyEventHappened = false;

    globals()->gColorAnimationStep = kStartAnimation;
    globals()->gColorAnimationInSpeed = 1;
    globals()->gColorAnimationOutSpeed = globals()->gColorAnimationInSpeed;
    globals()->gColorAnimationGoal = goalColor;
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

bool AutoMusicFadeTo(long tickTime, const RgbColor& goalColor, bool eventSkip) {
    long        startTime, thisTime = 0, lastStep = 0, thisStep = 0, musicVol, musicStep;
    bool     anyEventHappened = false;

    globals()->gColorAnimationStep = kStartAnimation;
    globals()->gColorAnimationInSpeed = 1;
    globals()->gColorAnimationOutSpeed = globals()->gColorAnimationInSpeed;
    globals()->gColorAnimationGoal = goalColor;
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

ColorFade::ColorFade(
        int clut_id, Direction direction, const RgbColor& color, double duration, bool allow_skip,
        bool* skipped)
        : _direction(direction),
          _transition_colors(clut_id),
          _current_colors(clut_id),
          _color(color),
          _allow_skip(allow_skip),
          _skipped(skipped),
          _next_event(now_secs() + (1.0 / 60.0)),
          _duration(duration) { }

void ColorFade::become_front() {
    _start = now_secs();
    gActiveWorld->set_transition_to(_color);
    gActiveWorld->set_transition_fraction(_direction);
}

void ColorFade::resign_front() {
    gActiveWorld->set_transition_to(_color);
    gActiveWorld->set_transition_fraction(0.0);
}

void ColorFade::mouse_down(int button, const Point& loc) {
    static_cast<void>(button);
    static_cast<void>(loc);
    if (_allow_skip) {
        *_skipped = true;
        stack()->pop(this);
    }
}

double ColorFade::next_timer() {
    return _next_event;
}

void ColorFade::fire_timer() {
    double now = now_secs();
    while (_next_event < now) {
        _next_event += (1.0 / 60.0);
    }
    double fraction = (now - _start) / _duration;
    if (fraction < 1.0) {
        if (_direction == TO_COLOR) {
            gActiveWorld->set_transition_fraction(fraction);
        } else {
            gActiveWorld->set_transition_fraction(1.0 - fraction);
        }
    } else {
        stack()->pop(this);
    }
}

PictFade::PictFade(int pict_id, int clut_id, bool* skipped)
        : _state(NEW),
          _pict_id(pict_id),
          _clut_id(clut_id),
          _skipped(skipped) { }

void PictFade::become_front() {
    switch (_state) {
      case NEW:
        wax();
        break;

      case WAXING:
        if (!this->skip()) {
            _state = FULL;
            _wane_start = now_secs() + this->display_time();
            break;
        }
        // fall through.

      case WANING:
        _state = NEW;
        stack()->pop(this);
        break;

      default:
        break;
    }
}

void PictFade::resign_front() {
    if (_state == NEW) {
        gActiveWorld->fill(RgbColor::kBlack);
        gActiveWorld->set_transition_fraction(0);
    }
}

void PictFade::mouse_down(int button, const Point& loc) {
    static_cast<void>(button);
    static_cast<void>(loc);
    *_skipped = true;
    if (this->skip()) {
        stack()->pop(this);
    } else {
        wane();
    }
}

double PictFade::next_timer() {
    if (_state == FULL) {
        return _wane_start;
    } else {
        return 0.0;
    }
}

void PictFade::fire_timer() {
    // Timer only fires when _state == FULL.
    wane();
}

void PictFade::wax() {
    _state = WAXING;

    gActiveWorld->fill(RgbColor::kBlack);
    Picture pict(_pict_id);
    Rect pictRect = pict.bounds();
    pictRect.center_in(gRealWorld->bounds());
    CopyBits(&pict, gActiveWorld, pict.bounds(), pictRect);

    stack()->push(new ColorFade(
                _clut_id, ColorFade::FROM_COLOR, RgbColor::kBlack, this->fade_time(), true,
                _skipped));
}

void PictFade::wane() {
    _state = WANING;
    stack()->push(new ColorFade(
                _clut_id, ColorFade::TO_COLOR, RgbColor::kBlack, this->fade_time(), true,
                _skipped));
}

double PictFade::fade_time() const {
    return 5.0 / 3.0;
}

double PictFade::display_time() const {
    return 4.0 / 3.0;
}

bool PictFade::skip() const {
    return *_skipped;
}

}  // namespace antares
