// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2011 Ares Central
//
// This file is part of Antares, a tactical space combat game.
//
// Antares is free software: you can redistribute it and/or modify it
// under the terms of the Lesser GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Antares is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program.  If not, see
// <http://www.gnu.org/licenses/>.

#include "game/messages.hpp"

#include "config/keys.hpp"
#include "data/resource.hpp"
#include "data/string-list.hpp"
#include "drawing/color.hpp"
#include "drawing/interface.hpp"
#include "drawing/offscreen-gworld.hpp"
#include "drawing/sprite-handling.hpp"
#include "drawing/text.hpp"
#include "game/globals.hpp"
#include "game/labels.hpp"
#include "game/scenario-maker.hpp"
#include "ui/interface-handling.hpp"
#include "video/driver.hpp"

using sfz::Bytes;
using sfz::BytesSlice;
using sfz::Exception;
using sfz::String;
using sfz::StringSlice;
using sfz::make_linked_ptr;
using sfz::scoped_ptr;

namespace macroman = sfz::macroman;

namespace antares {

namespace {

const int32_t kMessageScreenLeft        = 200;
const int32_t kMessageScreenTop         = 454;
const int32_t kMessageScreenRight       = 420;
const int32_t kMessageScreenBottom      = 475;

const uint8_t kMessageColor             = RED;
const int32_t kMessageMoveTime          = 30;
const int32_t kMessageDisplayTime       = (kMessageMoveTime * 2 + 120);
const int32_t kLowerTime                = (kMessageDisplayTime - kMessageMoveTime);
const int32_t kRaiseTime                = kMessageMoveTime;

const int32_t kDestinationLength        = 127;

const int32_t kStatusLabelLeft          = 200;
const int32_t kStatusLabelTop           = 50;
const int32_t kStatusLabelAge           = 120;

const int32_t kMaxRetroSize             = 10;
const int32_t kLongMessageVPad          = 5;
const int32_t kLongMessageVPadDouble    = 10;

const int32_t kMessageCharTopBuffer     = 0;
const int32_t kMessageCharBottomBuffer  = 0;

// These should be Rune constants, but we operate over bytes for now.
const uint8_t kReturnChar               = '\r';
const uint8_t kCodeChar                 = '\\';
const uint8_t kCodeTabChar              = 't';
const uint8_t kCodeInvertChar           = 'i';
const uint8_t kCodeColorChar            = 'c';
const uint8_t kCodeRevertChar           = 'r';
const uint8_t kCodeForeColorChar        = 'f';
const uint8_t kCodeBackColorChar        = 'b';

const int16_t kStringMessageID          = 1;

const int16_t kLongMessageFontNum       = kTacticalFontNum;

const int32_t kHBuffer                  = 4;

inline int mHexDigitValue(char c) {
    if ('0' <= c && c <= '9') {
        return c - '0';
    } else {
        return c - 'a' + 10;
    }
}

}  // namespace

namespace {

int mac_roman_char_width(uint8_t ch) {
    BytesSlice bytes(&ch, 1);
    String str(macroman::decode(bytes));
    uint8_t width;
    mDirectCharWidth(width, str.at(0));
    return width;
}

template <typename T>
void clear(T& t) {
    using std::swap;
    T u = T();
    swap(t, u);
}

}  // namespace

void MessageLabel_Set_Special(short id, const StringSlice& text);

void InitMessageScreen() {
    longMessageType *tmessage = NULL;

    clear(globals()->gMessageData);
    globals()->gStatusString.reset(new unsigned char[kDestinationLength]);
    globals()->gLongMessageData.reset(new longMessageType);

    globals()->gMessageLabelNum = AddScreenLabel(
            kMessageScreenLeft, kMessageScreenTop, 0, 0, NULL, false, kMessageColor);

    if (globals()->gMessageLabelNum < 0) {
        throw Exception("Couldn't add a screen label.");
    }
    globals()->gStatusLabelNum = AddScreenLabel(
            kStatusLabelLeft, kStatusLabelTop, 0, 0, NULL, false, kStatusLabelColor);
    if (globals()->gStatusLabelNum < 0) {
        throw Exception("Couldn't add a screen label.");
    }

    tmessage = globals()->gLongMessageData.get();
    tmessage->startResID =  tmessage->endResID = tmessage->lastResID = tmessage->currentResID =
        -1;
    tmessage->time = 0;
    tmessage->stage = kNoStage;
    tmessage->textHeight = 0;
    tmessage->retro_text.reset();
    tmessage->charDelayCount = 0;
    tmessage->pictBounds.left = tmessage->pictBounds.right= 0;
    tmessage->pictCurrentLeft = 0;
    tmessage->pictCurrentTop = 0;
    tmessage->pictID = -1;
    tmessage->stringMessage.clear();
    tmessage->lastStringMessage.clear();
    tmessage->newStringMessage = false;
    tmessage->labelMessage = false;
    tmessage->lastLabelMessage = false;
    tmessage->labelMessageID = -1;
}

void MessageScreenCleanup() {
    clear(globals()->gMessageData);
    globals()->gStatusString.reset();
}

void ClearMessage( void) {
    longMessageType *tmessage;

    globals()->gMessageTimeCount = 0;
    globals()->gMessageLabelNum = AddScreenLabel(
            kMessageScreenLeft, kMessageScreenTop, 0, 0, NULL, false, kMessageColor);
    globals()->gStatusLabelNum = AddScreenLabel(
            kStatusLabelLeft, kStatusLabelTop, 0, 0, NULL, false, kStatusLabelColor);

    tmessage = globals()->gLongMessageData.get();
    tmessage->startResID = -1;
    tmessage->endResID = -1;
    tmessage->currentResID = -1;
    tmessage->lastResID = -1;
    tmessage->textHeight = 0;
    tmessage->previousStartResID = tmessage->previousEndResID = -1;
    tmessage = globals()->gLongMessageData.get();
    tmessage->stringMessage.clear();
    tmessage->lastStringMessage.clear();
    tmessage->newStringMessage = false;
    tmessage->labelMessage = false;
    tmessage->lastLabelMessage = false;
    tmessage->retro_text.reset();
    viewport.bottom = play_screen.bottom;
    tmessage->labelMessageID = AddScreenLabel(0, 0, 0, 0, NULL, false, SKY_BLUE);
    SetScreenLabelKeepOnScreenAnyway( tmessage->labelMessageID, true);
}

void AddMessage(const sfz::PrintItem& message) {
    globals()->gMessageData.push(make_linked_ptr(new String(message)));
}

void StartLongMessage( short startResID, short endResID)

{
    longMessageType *tmessage;

    tmessage = globals()->gLongMessageData.get();

    if ( tmessage->currentResID != -1)
    {
        tmessage->startResID = startResID;
        tmessage->endResID = endResID;
        tmessage->currentResID = startResID - 1;
        AdvanceCurrentLongMessage();
    } else
    {
        tmessage->previousStartResID = tmessage->startResID;
        tmessage->previousEndResID = tmessage->endResID;
        tmessage->startResID = startResID;
        tmessage->endResID = endResID;
        tmessage->currentResID = startResID;
        tmessage->time = 0;
        tmessage->stage = kStartStage;
        tmessage->textHeight = 0;
        tmessage->retro_text.reset();
        tmessage->charDelayCount = 0;
        tmessage->pictBounds.left = tmessage->pictBounds.right= 0;
        // tmessage->pictDelayCount;
        tmessage->pictCurrentLeft = 0;
        tmessage->pictCurrentTop = 0;
        tmessage->pictID = -1;
    }
}

void ClipToCurrentLongMessage( void)

{
    longMessageType *tmessage;
    scoped_ptr<String> textData;

    tmessage = globals()->gLongMessageData.get();
    if (( tmessage->currentResID != tmessage->lastResID) || ( tmessage->newStringMessage))
    {

        if ( tmessage->lastResID >= 0)
        {
            viewport.bottom = play_screen.bottom;
        }

        // draw in offscreen world
        if (( tmessage->currentResID >= 0) && ( tmessage->stage == kClipStage))
        {
            if ( tmessage->currentResID == kStringMessageID)
            {
                textData.reset(new String);
                if (textData.get() != NULL) {
                    print(*textData, tmessage->stringMessage);
                }
                tmessage->labelMessage = false;
            } else
            {
                Resource rsrc("text", "txt", tmessage->currentResID);
                textData.reset(new String(macroman::decode(rsrc.data())));
                Replace_KeyCode_Strings_With_Actual_Key_Names(textData.get(), KEY_LONG_NAMES, 0);
                if (textData->at(0) == '#') {
                    tmessage->labelMessage = true;
                }
                else tmessage->labelMessage = false;

            }
            if (textData.get() != NULL) {
                const RgbColor& light_blue = GetRGBTranslateColorShade(SKY_BLUE, VERY_LIGHT);
                const RgbColor& dark_blue = GetRGBTranslateColorShade(SKY_BLUE, DARKEST);
                mSetDirectFont(kLongMessageFontNum);
                tmessage->text.assign(*textData);
                tmessage->retro_text.reset(
                        new RetroText(*textData, kLongMessageFontNum, light_blue, dark_blue));
                tmessage->retro_text->set_tab_width(60);
                tmessage->retro_text->wrap_to(
                        viewport.width() - kHBuffer - gDirectText->logicalWidth, 0);
                tmessage->textHeight = tmessage->retro_text->height();
                tmessage->textHeight += kLongMessageVPadDouble;
                tmessage->retro_origin = Point(
                        viewport.left + kHBuffer,
                        viewport.bottom + mDirectFontAscent() + kLongMessageVPad);
                tmessage->at_char = 0;

                if (tmessage->labelMessage == false) {
                    viewport.bottom = play_screen.bottom - tmessage->textHeight;
                } else {
                    viewport.bottom = play_screen.bottom;
                }
                tmessage->stage = kShowStage;

                /*
                tmessage->retroTextSpec.topBuffer = kMessageCharTopBuffer;
                tmessage->retroTextSpec.bottomBuffer = kMessageCharBottomBuffer;
                tmessage->retroTextSpec.thisPosition = 0;
                tmessage->retroTextSpec.lineCount = 0;
                tmessage->retroTextSpec.linePosition = 0;
                tmessage->retroTextSpec.xpos = viewport.left + kHBuffer;
                tmessage->retroTextSpec.ypos = viewport.bottom + mDirectFontAscent() + kLongMessageVPad + tmessage->retroTextSpec.topBuffer;
                tmessage->retroTextSpec.tabSize = 60;
                tmessage->retroTextSpec.color = GetRGBTranslateColorShade(SKY_BLUE, VERY_LIGHT);
                tmessage->retroTextSpec.backColor = GetRGBTranslateColorShade(SKY_BLUE, DARKEST);
                tmessage->retroTextSpec.nextColor = tmessage->retroTextSpec.color;
                tmessage->retroTextSpec.nextBackColor = tmessage->retroTextSpec.backColor;
                tmessage->retroTextSpec.originalColor = tmessage->retroTextSpec.color;
                tmessage->retroTextSpec.originalBackColor = tmessage->retroTextSpec.backColor;
                */
            }
        } else {
            viewport.bottom = play_screen.bottom;
            tmessage->stage = kClipStage;
        }
    }
}

void DrawCurrentLongMessage(int32_t time_pass) {
    Rect            tRect, uRect;
    Rect            lRect, cRect;
    longMessageType *tmessage;
    RgbColor        color;

    tmessage = globals()->gLongMessageData.get();
    if ((tmessage->currentResID != tmessage->lastResID)
            || (tmessage->newStringMessage)) {
        // TODO(sfiera): figure out what this meant.
        //
        // we check scenario conditions here for ambrosia tutorial
        // but not during net game -- other players wouldn't care what message
        // we were looking at
        //
        // if ( !(globals()->gOptions & kOptionNetworkOn))
        // {
            CheckScenarioConditions( 0);
        // }

        if ((tmessage->lastResID >= 0) && (tmessage->lastLabelMessage)) {
            SetScreenLabelAge(tmessage->labelMessageID, 1);
        }

        // draw in offscreen world
        if ((tmessage->currentResID >= 0) && ( tmessage->stage == kShowStage)) {
            if (tmessage->retro_text.get() != NULL) {
                if (tmessage->labelMessage) {
                    SetScreenLabelAge(tmessage->labelMessageID, 0);

                    if (tmessage->retro_text.get() != NULL) {
                        MessageLabel_Set_Special(tmessage->labelMessageID, tmessage->text);
                    }
                }
            }
        }
        if ((tmessage->stage == kShowStage) || (tmessage->currentResID < 0)) {
            tmessage->lastResID = tmessage->currentResID;
            tmessage->lastLabelMessage = tmessage->labelMessage;
            tmessage->newStringMessage = false;
        }
    } else if ((tmessage->currentResID >= 0)
            && (tmessage->retro_text.get() != NULL)
            && (tmessage->at_char < tmessage->retro_text->size())
            && (tmessage->stage == kShowStage)
            && !tmessage->labelMessage) {
        time_pass = std::min(time_pass, tmessage->retro_text->size() - tmessage->at_char);
        // Play teletype sound at least once every 3 ticks.
        tmessage->charDelayCount += time_pass;
        if (tmessage->charDelayCount > 0) {
            mSetDirectFont( kLongMessageFontNum);
            PlayVolumeSound(kTeletype, kMediumLowVolume, kShortPersistence, kLowPrioritySound);
            while (tmessage->charDelayCount > 0) {
                tmessage->charDelayCount -= 3;
            }
        }
        tmessage->at_char += time_pass;
    }
}

void EndLongMessage( void)

{
    longMessageType *tmessage;

    tmessage = globals()->gLongMessageData.get();
    tmessage->previousStartResID = tmessage->startResID;
    tmessage->previousEndResID = tmessage->endResID;
    tmessage->startResID = -1;
    tmessage->endResID = -1;
    tmessage->currentResID = -1;
    tmessage->stage = kStartStage;
    tmessage->retro_text.reset();
    tmessage->lastStringMessage.assign(tmessage->stringMessage);
}

void AdvanceCurrentLongMessage( void)
{
    longMessageType *tmessage;

    tmessage = globals()->gLongMessageData.get();
    if ( tmessage->currentResID != -1)
    {
        if ( tmessage->currentResID < tmessage->endResID)
        {
            tmessage->currentResID++;
            tmessage->stage = kStartStage;
        }
        else
        {
            EndLongMessage();
        }
    }
}

void PreviousCurrentLongMessage( void)
{
    longMessageType *tmessage;

    tmessage = globals()->gLongMessageData.get();
    if ( tmessage->currentResID != -1)
    {
        if ( tmessage->currentResID > tmessage->startResID)
        {
            tmessage->currentResID--;
            tmessage->stage = kStartStage;
        }
        else
        {
        }
    }
}

void ReplayLastLongMessage( void)
{
    longMessageType *tmessage;

    tmessage = globals()->gLongMessageData.get();
    if (( tmessage->previousStartResID >= 0) && ( tmessage->currentResID < 0))
    {
        tmessage->stringMessage.assign(tmessage->lastStringMessage);
        StartLongMessage( tmessage->previousStartResID, tmessage->previousEndResID);
    }
}

// WARNING: RELIES ON kMessageNullCharacter (SPACE CHARACTER #32) >> NOT WORLD-READY <<

void DrawMessageScreen(int32_t by_units) {
    // increase the amount of time current message has been shown
    globals()->gMessageTimeCount += by_units;

    // if it's been shown for too long, then get the next message
    if (globals()->gMessageTimeCount > kMessageDisplayTime) {
        globals()->gMessageTimeCount = 0;
        globals()->gMessageData.pop();
    }

    mSetDirectFont( kTacticalFontNum);

    if (!globals()->gMessageData.empty()) {
        const String& message = *globals()->gMessageData.front();

        if (globals()->gMessageTimeCount < kRaiseTime) {
            SetScreenLabelPosition(
                    globals()->gMessageLabelNum, kMessageScreenLeft,
                    viewport.bottom - globals()->gMessageTimeCount);
        } else if (globals()->gMessageTimeCount > kLowerTime) {
            SetScreenLabelPosition(
                    globals()->gMessageLabelNum, kMessageScreenLeft,
                    viewport.bottom - (kMessageDisplayTime - globals()->gMessageTimeCount));
        }

        SetScreenLabelString(globals()->gMessageLabelNum, message);
    } else {
        ClearScreenLabelString(globals()->gMessageLabelNum);
        globals()->gMessageTimeCount = 0;
    }
}

void SetStatusString(const StringSlice& status, unsigned char color) {
    SetScreenLabelColor(globals()->gStatusLabelNum, color);
    SetScreenLabelString(globals()->gStatusLabelNum, status);
    SetScreenLabelAge(globals()->gStatusLabelNum, kStatusLabelAge);
}

long DetermineDirectTextHeightInWidth( retroTextSpecType *retroTextSpec, long inWidth)

{
    long            charNum = 0, height = mDirectFontHeight(), x = 0, oldx = 0, oldCharNum, wordLen,
                    *lineLengthList = retroTextSpec->lineLength;
    unsigned char   wrapState; // 0 = none, 1 = once, 2 = more than once
    Bytes bytes(macroman::encode(*retroTextSpec->text));
    const uint8_t*  thisChar = bytes.data();

    *lineLengthList = 0;
    retroTextSpec->autoWidth = 0;
    retroTextSpec->lineNumber = 1;
    while ( charNum < retroTextSpec->textLength)
    {
        if ( *thisChar == kReturnChar)
        {
            if ( x > retroTextSpec->autoWidth) retroTextSpec->autoWidth = x;
            height += mDirectFontHeight() + retroTextSpec->topBuffer + retroTextSpec->bottomBuffer;
            x = 0;
            thisChar++;
            charNum++;
            (*lineLengthList)++;
            lineLengthList++;
            *lineLengthList = 0;
            retroTextSpec->lineNumber++;
        } else if ( *thisChar == ' ')
        {
            do
            {
                x += mac_roman_char_width(' ');
                thisChar++;
                charNum++;
                (*lineLengthList)++;
            } while (( *thisChar == ' ')  && ( charNum < retroTextSpec->textLength));
        } else if ( *thisChar == kCodeChar)
        {
            thisChar++;
            charNum++;
            (*lineLengthList)++;
            switch( *thisChar)
            {
                case kCodeTabChar:
                    wordLen = 0;
                    oldx = 0;
                    while ( oldx <= x)
                    {
                        oldx += retroTextSpec->tabSize;
                        wordLen++;
                    }
                    x = 0 + retroTextSpec->tabSize * wordLen;
                    break;

                case kCodeForeColorChar:
                case kCodeBackColorChar:
                    thisChar++;
                    charNum++;
                    (*lineLengthList)++;
                    thisChar++;
                    charNum++;
                    (*lineLengthList)++;
                    break;

                case kCodeChar:
                    x += mac_roman_char_width(*thisChar);
                    wordLen++;
                    break;
            }
            thisChar++;
            charNum++;
            (*lineLengthList)++;
        } else
        {
            oldx = x;
            oldCharNum = charNum;
            wordLen = 0;
            wrapState = 0;
            do
            {
                x += mac_roman_char_width(*thisChar);
                wordLen++;
                if ( x >= (inWidth - gDirectText->logicalWidth))
                {
                    if ( !wrapState)
                    {
                        wrapState = 1;
                        if ( oldx > retroTextSpec->autoWidth) retroTextSpec->autoWidth = oldx;
                        x = x - oldx;
                        oldx = 0;
                        height += mDirectFontHeight() + retroTextSpec->topBuffer + retroTextSpec->bottomBuffer;
                    } else
                    {
                        wrapState = 2;
                        wordLen--;
                    }
                }
                thisChar++;
                charNum++;
            } while (( *thisChar != ' ') && ( *thisChar != kReturnChar) && ( wrapState < 2) &&
                ( *thisChar != kCodeChar) && ( charNum < retroTextSpec->textLength));
            if ( wrapState)
            {
                if ( x > retroTextSpec->autoWidth) retroTextSpec->autoWidth = x;
                lineLengthList++;
                *lineLengthList = 0;
                retroTextSpec->lineNumber++;
            }
            *lineLengthList += wordLen;
        }
    }
    if ( x > retroTextSpec->autoWidth) retroTextSpec->autoWidth = x;
    retroTextSpec->autoWidth += 1;
    retroTextSpec->autoHeight = height;
    return ( height);
}

void DrawRetroTextCharInRect(
        retroTextSpecType *retroTextSpec, long charsToDo, const Rect& bounds, const Rect& clipRect,
        PixMap *destMap) {
    Bytes bytes(macroman::encode(*retroTextSpec->text));
    const uint8_t* thisChar = bytes.data();
    Rect        cursorRect, lineRect, tlRect;
    long            oldx, wordLen, *lineLength = &(retroTextSpec->lineLength[retroTextSpec->lineCount]);
    unsigned char   calcColor, calcShade;
    RgbColor tempColor;
    bool         drawCursor = ( charsToDo > 0);

    cursorRect.left = retroTextSpec->xpos;
    cursorRect.top = retroTextSpec->ypos -
        (mDirectFontAscent()  + retroTextSpec->topBuffer);
    cursorRect.right = cursorRect.left + gDirectText->logicalWidth;
    cursorRect.bottom = cursorRect.top + mDirectFontHeight() +
        retroTextSpec->topBuffer + retroTextSpec->bottomBuffer;
    tlRect = cursorRect;
    tlRect.clip_to(bounds);
    if ( retroTextSpec->originalBackColor != RgbColor::kWhite)
        DrawNateRectClipped(destMap, &tlRect, clipRect, retroTextSpec->originalBackColor);

    if ( charsToDo <= 0) charsToDo = retroTextSpec->lineLength[retroTextSpec->lineCount];

    while (( charsToDo > 0) && ( retroTextSpec->thisPosition <
        retroTextSpec->textLength))
    {
        thisChar = bytes.data() + retroTextSpec->thisPosition;
        if ( *thisChar == kCodeChar)
        {
            thisChar++;
            charsToDo--;
            (retroTextSpec->thisPosition)++;
            (retroTextSpec->linePosition)++;
            switch( *thisChar)
            {
                case kCodeTabChar:
                    wordLen = 0;
                    oldx = bounds.left;
                    cursorRect.left = retroTextSpec->xpos;
                    while ( oldx <= retroTextSpec->xpos)
                    {
                        oldx += retroTextSpec->tabSize;
                        wordLen++;
                    }
                    retroTextSpec->xpos = bounds.left + retroTextSpec->tabSize *
                        wordLen;
                    cursorRect.right = retroTextSpec->xpos;
                    tlRect = cursorRect;
                    tlRect.clip_to(bounds);
                    if ( retroTextSpec->backColor != RgbColor::kWhite)
                        DrawNateRectClipped(destMap, &tlRect, clipRect, retroTextSpec->backColor);
                    break;

                case kCodeChar:
                    oldx = cursorRect.left = retroTextSpec->xpos;
                    cursorRect.top = retroTextSpec->ypos - (mDirectFontAscent() + retroTextSpec->topBuffer);
                    cursorRect.bottom = cursorRect.top + mDirectFontHeight() + retroTextSpec->topBuffer + retroTextSpec->bottomBuffer;
                    retroTextSpec->xpos += mac_roman_char_width(*thisChar);
                    tlRect = cursorRect;
                    tlRect.clip_to(bounds);
                    if ( retroTextSpec->backColor != RgbColor::kWhite)
                        DrawNateRectClipped(destMap, &tlRect, clipRect, retroTextSpec->backColor);
                    cursorRect.right = retroTextSpec->xpos;
                    {
                        const char text[] = {'\\', '\0'};
                        DrawDirectTextStringClipped(
                                Point(oldx, retroTextSpec->ypos), text,
                                ((retroTextSpec->color == RgbColor::kWhite)
                                    ? (RgbColor::kBlack) : (retroTextSpec->color)),
                                destMap, clipRect);
                    }
                    break;

                case kCodeInvertChar:
                    tempColor = retroTextSpec->color;
                    retroTextSpec->nextColor = retroTextSpec->backColor;
                    retroTextSpec->nextBackColor = tempColor;
                    break;

                case kCodeForeColorChar:
                    thisChar++;
                    charsToDo--;
                    (retroTextSpec->thisPosition)++;
                    (retroTextSpec->linePosition)++;
                    calcColor = mHexDigitValue(*thisChar);
                    thisChar++;
                    charsToDo--;
                    (retroTextSpec->thisPosition)++;
                    (retroTextSpec->linePosition)++;
                    calcShade = mHexDigitValue(*thisChar);
                    retroTextSpec->nextColor = GetRGBTranslateColorShade(calcColor, calcShade);
                    break;

                case kCodeBackColorChar:
                    thisChar++;
                    charsToDo--;
                    (retroTextSpec->thisPosition)++;
                    (retroTextSpec->linePosition)++;
                    calcColor = mHexDigitValue(*thisChar);
                    thisChar++;
                    charsToDo--;
                    (retroTextSpec->thisPosition)++;
                    (retroTextSpec->linePosition)++;
                    calcShade = mHexDigitValue(*thisChar);
                    if (( calcColor) && (calcShade == 0))
                    {
                        retroTextSpec->nextBackColor = RgbColor::kBlack;
                    } else
                    {
                        retroTextSpec->nextBackColor = GetRGBTranslateColorShade(calcColor, calcShade);
                    }
                    break;

                case kCodeRevertChar:
                    retroTextSpec->nextColor = retroTextSpec->originalColor;
                    retroTextSpec->nextBackColor = retroTextSpec->originalBackColor;
                    break;
            }
            thisChar++;
            charsToDo--;
            (retroTextSpec->thisPosition)++;
            (retroTextSpec->linePosition)++;
        } else if ( *thisChar == kReturnChar)
        {
            thisChar++;
            (retroTextSpec->thisPosition)++;
            (retroTextSpec->linePosition)++;
            charsToDo--;
        } else
        {
            char bytes[] = {*thisChar, '\0'};
            if (bytes[0] == '_') {
                bytes[0] = ' ';
            }

            retroTextSpec->color = retroTextSpec->nextColor;
            retroTextSpec->backColor = retroTextSpec->nextBackColor;
            cursorRect.left = retroTextSpec->xpos;
            int32_t oldx = retroTextSpec->xpos;
            int32_t y = retroTextSpec->ypos;
            retroTextSpec->xpos += mac_roman_char_width(*thisChar);
            cursorRect.right = retroTextSpec->xpos;
            tlRect = cursorRect;
            tlRect.clip_to(bounds);
            if ( retroTextSpec->backColor != RgbColor::kWhite)
                DrawNateRectClipped(destMap, &tlRect, clipRect, retroTextSpec->backColor);
            String text(macroman::decode(bytes));
            DrawDirectTextStringClipped(
                    Point(oldx, y), text,
                    ((retroTextSpec->color == RgbColor::kWhite)
                        ? (RgbColor::kBlack) : (retroTextSpec->color)),
                    destMap, clipRect);
            (retroTextSpec->thisPosition)++;
            (retroTextSpec->linePosition)++;
            charsToDo--;
        }
        if ( retroTextSpec->linePosition >= *lineLength)
        {
            lineRect.left = retroTextSpec->xpos;
            lineRect.right = bounds.right;
            lineRect.top = cursorRect.top;
            lineRect.bottom = cursorRect.bottom;
            tlRect = lineRect;
            tlRect.clip_to(bounds);
            if ( retroTextSpec->backColor != RgbColor::kWhite)
                DrawNateRectClipped(destMap, &tlRect, clipRect, retroTextSpec->backColor);

            retroTextSpec->linePosition = 0;
            retroTextSpec->ypos += mDirectFontHeight() + retroTextSpec->topBuffer + retroTextSpec->bottomBuffer;
            retroTextSpec->xpos = bounds.left;
            (retroTextSpec->lineCount)++;
            lineLength++;
            cursorRect.top = retroTextSpec->ypos - (mDirectFontAscent() + retroTextSpec->topBuffer);
            cursorRect.bottom = cursorRect.top + mDirectFontHeight() + retroTextSpec->topBuffer + retroTextSpec->bottomBuffer;
        } else
        {
        }

    }
    if ( retroTextSpec->thisPosition < retroTextSpec->textLength)
    {
        cursorRect.left = retroTextSpec->xpos;
        cursorRect.right = cursorRect.left + gDirectText->logicalWidth;
        if ( drawCursor)
        {
            tlRect = cursorRect;
            tlRect.clip_to(bounds);
            if ( retroTextSpec->backColor != RgbColor::kWhite)
                DrawNateRectClipped(destMap, &tlRect, clipRect, retroTextSpec->originalColor);
        }
    }
}

//
// MessageLabel_Set_Special
//  for ambrosia emergency tutorial; Sets screen label given specially formatted
//  text. Text must have its own line breaks so label fits on screen.
//
//  First few chars of text must be in this format:
//
//  #tnnn...#
//
//  Where '#' is literal;
//  t = one of three characters: 'L' for left, 'R' for right, and 'O' for object
//  nnn... are digits specifying value (distance from top, or initial object #)
//
void MessageLabel_Set_Special(short id, const StringSlice& text) {
    char whichType;
    long value = 0;
    Point attachPoint;
    bool hintLine = false;

    StringSlice::const_iterator it = text.begin();

    // if not legal, bail
    if (*it != '#') {
        return;
    }

    ++it;

    whichType = *it;
    ++it;
    while ((it != text.end()) && (*it != '#')) {
        value *= 10;
        value += *it - '0';
        ++it;
    }

    ++it;
    if (*it == '#') {  // also a hint line attached
        hintLine = true;
        ++it;
        // h coord
        while ((it != text.end()) && (*it != ',')) {
            attachPoint.h *= 10;
            attachPoint.h += *it - '0';
            ++it;
        }

        ++it;

        while ((it != text.end()) && (*it != '#')) {
            attachPoint.v *= 10;
            attachPoint.v += *it - '0';
            ++it;
        }
        attachPoint.v += globals()->gInstrumentTop;
        if (attachPoint.h >= (kSmallScreenWidth - kRightPanelWidth)) {
            attachPoint.h = (attachPoint.h - (kSmallScreenWidth - kRightPanelWidth)) +
                play_screen.right;
        }
        ++it;
    }

    String message;
    while (it != text.end()) {
        message.push(1, *it);
        ++it;
    }

    SetScreenLabelString(id, message);
    SetScreenLabelKeepOnScreenAnyway(id, true);

    switch (whichType) {
      case 'R':
        SetScreenLabelOffset(id, 0, 0);
        SetScreenLabelPosition(
                id, play_screen.right - (GetScreenLabelWidth(id)+10),
                globals()->gInstrumentTop + value);
        break;

      case 'L':
        SetScreenLabelOffset(id, 0, 0);
        SetScreenLabelPosition(id, 138, globals()->gInstrumentTop + value);
        break;

      case 'O':
        {
            spaceObjectType* o = GetObjectFromInitialNumber(value);
            SetScreenLabelOffset(id, -(GetScreenLabelWidth(id)/2), 64);
            SetScreenLabelObject(id, o);

            hintLine = true;
        }
        break;
    }
    attachPoint.v -= 2;
    SetScreenLabelAttachedHintLine(id, hintLine, attachPoint);
}

void draw_message() {
    if (viewport.bottom == play_screen.bottom) {
        return;
    }

    const RgbColor& dark_blue = GetRGBTranslateColorShade(SKY_BLUE, DARKEST);
    const RgbColor& light_blue = GetRGBTranslateColorShade(SKY_BLUE, VERY_LIGHT);
    Rect message_bounds(play_screen.left, viewport.bottom, play_screen.right, play_screen.bottom);
    VideoDriver::driver()->fill_rect(message_bounds, light_blue);
    message_bounds.inset(0, 1);
    VideoDriver::driver()->fill_rect(message_bounds, dark_blue);

    longMessageType *tmessage = globals()->gLongMessageData.get();

    Rect bounds(viewport.left, viewport.bottom, viewport.right, play_screen.bottom);
    bounds.inset(kHBuffer, 0);
    bounds.top += kLongMessageVPad;
    for (int i = 0; i < tmessage->at_char; ++i) {
        tmessage->retro_text->draw_char(bounds, i);
    }
    // The final char is a newline; don't display a cursor rect for it.
    if ((0 < tmessage->at_char) && (tmessage->at_char < (tmessage->retro_text->size() - 1))) {
        tmessage->retro_text->draw_cursor(bounds, tmessage->at_char);
    }
}

}  // namespace antares
