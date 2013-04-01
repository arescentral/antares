// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2012 The Antares Authors
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
// License along with Antares.  If not, see http://www.gnu.org/licenses/

#include "game/messages.hpp"

#include "config/keys.hpp"
#include "data/resource.hpp"
#include "data/string-list.hpp"
#include "drawing/color.hpp"
#include "drawing/interface.hpp"
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
using std::unique_ptr;

namespace utf8 = sfz::utf8;

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
const uint8_t kReturnChar               = '\n';
const uint8_t kCodeChar                 = '\\';
const uint8_t kCodeTabChar              = 't';
const uint8_t kCodeInvertChar           = 'i';
const uint8_t kCodeColorChar            = 'c';
const uint8_t kCodeRevertChar           = 'r';
const uint8_t kCodeForeColorChar        = 'f';
const uint8_t kCodeBackColorChar        = 'b';

const int16_t kStringMessageID          = 1;

const int32_t kHBuffer                  = 4;

template <typename T>
void clear(T& t) {
    using std::swap;
    T u = T();
    swap(t, u);
}

enum longMessageStageType {
    kNoStage = 0,
    kStartStage = 1,
    kClipStage = 2,
    kShowStage = 3,
    kEndStage = 4
};

}  // namespace

struct Messages::longMessageType {
    longMessageStageType    stage;
    int32_t                 charDelayCount;
    Rect                    pictBounds;
    int32_t                 pictDelayCount;
    int32_t                 pictCurrentLeft;
    int32_t                 pictCurrentTop;
    int32_t                 time;
    int32_t                 textHeight;
    int16_t                 startResID;
    int16_t                 endResID;
    int16_t                 currentResID;
    int16_t                 lastResID;
    int16_t                 previousStartResID;
    int16_t                 previousEndResID;
    int16_t                 pictID;
    uint8_t                 backColor;
    sfz::String             stringMessage;
    sfz::String             lastStringMessage;
    bool                 newStringMessage;
    sfz::String             text;
    std::unique_ptr<StyledText> retro_text;
    Point                   retro_origin;
    int32_t                 at_char;
    bool                 labelMessage;
    bool                 lastLabelMessage;
    int16_t                 labelMessageID;
};

std::queue<sfz::String> Messages::message_data;
Messages::longMessageType* Messages::long_message_data;
int32_t Messages::time_count;
int32_t Messages::message_label_num;
int32_t Messages::status_label_num;

void MessageLabel_Set_Special(int16_t id, const StringSlice& text);

void Messages::init() {
    longMessageType *tmessage = NULL;

    antares::clear(message_data);
    long_message_data = new longMessageType;

    message_label_num = Labels::add(
            kMessageScreenLeft, kMessageScreenTop, 0, 0, NULL, false, kMessageColor);

    if (message_label_num < 0) {
        throw Exception("Couldn't add a screen label.");
    }
    status_label_num = Labels::add(
            kStatusLabelLeft, kStatusLabelTop, 0, 0, NULL, false, kStatusLabelColor);
    if (status_label_num < 0) {
        throw Exception("Couldn't add a screen label.");
    }

    tmessage = long_message_data;
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

void Messages::clear() {
    longMessageType *tmessage;

    time_count = 0;
    message_label_num = Labels::add(
            kMessageScreenLeft, kMessageScreenTop, 0, 0, NULL, false, kMessageColor);
    status_label_num = Labels::add(
            kStatusLabelLeft, kStatusLabelTop, 0, 0, NULL, false, kStatusLabelColor);

    tmessage = long_message_data;
    tmessage->startResID = -1;
    tmessage->endResID = -1;
    tmessage->currentResID = -1;
    tmessage->lastResID = -1;
    tmessage->textHeight = 0;
    tmessage->previousStartResID = tmessage->previousEndResID = -1;
    tmessage = long_message_data;
    tmessage->stringMessage.clear();
    tmessage->lastStringMessage.clear();
    tmessage->newStringMessage = false;
    tmessage->labelMessage = false;
    tmessage->lastLabelMessage = false;
    tmessage->retro_text.reset();
    viewport.bottom = play_screen.bottom;
    tmessage->labelMessageID = Labels::add(0, 0, 0, 0, NULL, false, SKY_BLUE);
    Labels::set_keep_on_screen_anyway( tmessage->labelMessageID, true);
}

void Messages::add(const sfz::PrintItem& message) {
    message_data.emplace(message);
}

void Messages::start(int16_t startResID, int16_t endResID) {
    longMessageType *tmessage;

    tmessage = long_message_data;

    if ( tmessage->currentResID != -1)
    {
        tmessage->startResID = startResID;
        tmessage->endResID = endResID;
        tmessage->currentResID = startResID - 1;
        advance();
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

void Messages::clip( void)

{
    longMessageType *tmessage;
    unique_ptr<String> textData;

    tmessage = long_message_data;
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
                textData.reset(new String(utf8::decode(rsrc.data())));
                Replace_KeyCode_Strings_With_Actual_Key_Names(textData.get(), KEY_LONG_NAMES, 0);
                if (textData->at(0) == '#') {
                    tmessage->labelMessage = true;
                }
                else tmessage->labelMessage = false;

            }
            if (textData.get() != NULL) {
                const RgbColor& light_blue = GetRGBTranslateColorShade(SKY_BLUE, VERY_LIGHT);
                const RgbColor& dark_blue = GetRGBTranslateColorShade(SKY_BLUE, DARKEST);
                tmessage->text.assign(*textData);
                tmessage->retro_text.reset(new StyledText(tactical_font));
                tmessage->retro_text->set_fore_color(light_blue);
                tmessage->retro_text->set_back_color(dark_blue);
                tmessage->retro_text->set_retro_text(*textData);
                tmessage->retro_text->set_tab_width(60);
                tmessage->retro_text->wrap_to(
                        viewport.width() - kHBuffer - tactical_font->logicalWidth + 1, 0, 0);
                tmessage->textHeight = tmessage->retro_text->height();
                tmessage->textHeight += kLongMessageVPadDouble;
                tmessage->retro_origin = Point(
                        viewport.left + kHBuffer,
                        viewport.bottom + tactical_font->ascent + kLongMessageVPad);
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

void Messages::draw_long_message(int32_t time_pass) {
    Rect            tRect, uRect;
    Rect            lRect, cRect;
    longMessageType *tmessage;
    RgbColor        color;

    tmessage = long_message_data;
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
            Labels::set_age(tmessage->labelMessageID, 1);
        }

        // draw in offscreen world
        if ((tmessage->currentResID >= 0) && ( tmessage->stage == kShowStage)) {
            if (tmessage->retro_text.get() != NULL) {
                if (tmessage->labelMessage) {
                    Labels::set_age(tmessage->labelMessageID, 0);

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
            PlayVolumeSound(kTeletype, kMediumLowVolume, kShortPersistence, kLowPrioritySound);
            while (tmessage->charDelayCount > 0) {
                tmessage->charDelayCount -= 3;
            }
        }
        tmessage->at_char += time_pass;
    }
}

void Messages::end() {
    longMessageType *tmessage;

    tmessage = long_message_data;
    tmessage->previousStartResID = tmessage->startResID;
    tmessage->previousEndResID = tmessage->endResID;
    tmessage->startResID = -1;
    tmessage->endResID = -1;
    tmessage->currentResID = -1;
    tmessage->stage = kStartStage;
    tmessage->retro_text.reset();
    tmessage->lastStringMessage.assign(tmessage->stringMessage);
}

void Messages::advance() {
    longMessageType *tmessage;

    tmessage = long_message_data;
    if ( tmessage->currentResID != -1)
    {
        if ( tmessage->currentResID < tmessage->endResID)
        {
            tmessage->currentResID++;
            tmessage->stage = kStartStage;
        }
        else
        {
            end();
        }
    }
}

void Messages::previous() {
    longMessageType *tmessage;

    tmessage = long_message_data;
    if ( tmessage->currentResID != -1)
    {
        if ( tmessage->currentResID > tmessage->startResID)
        {
            tmessage->currentResID--;
            tmessage->stage = kStartStage;
        }
    }
}

void Messages::replay() {
    longMessageType *tmessage;

    tmessage = long_message_data;
    if (( tmessage->previousStartResID >= 0) && ( tmessage->currentResID < 0))
    {
        tmessage->stringMessage.assign(tmessage->lastStringMessage);
        start( tmessage->previousStartResID, tmessage->previousEndResID);
    }
}

// WARNING: RELIES ON kMessageNullCharacter (SPACE CHARACTER #32) >> NOT WORLD-READY <<

void Messages::draw_message_screen(int32_t by_units) {
    // increase the amount of time current message has been shown
    time_count += by_units;

    // if it's been shown for too long, then get the next message
    if (time_count > kMessageDisplayTime) {
        time_count = 0;
        message_data.pop();
    }

    if (!message_data.empty()) {
        const String& message = message_data.front();

        if (time_count < kRaiseTime) {
            Labels::set_position(
                    message_label_num, kMessageScreenLeft,
                    viewport.bottom - time_count);
        } else if (time_count > kLowerTime) {
            Labels::set_position(
                    message_label_num, kMessageScreenLeft,
                    viewport.bottom - (kMessageDisplayTime - time_count));
        }

        Labels::set_string(message_label_num, message);
    } else {
        Labels::clear_string(message_label_num);
        time_count = 0;
    }
}

void Messages::set_status(const StringSlice& status, uint8_t color) {
    Labels::set_color(status_label_num, color);
    Labels::set_string(status_label_num, status);
    Labels::set_age(status_label_num, kStatusLabelAge);
}

int16_t Messages::current() {
    return long_message_data->currentResID;
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
void MessageLabel_Set_Special(int16_t id, const StringSlice& text) {
    char whichType;
    int32_t value = 0;
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

    Labels::set_string(id, message);
    Labels::set_keep_on_screen_anyway(id, true);

    switch (whichType) {
      case 'R':
        Labels::set_offset(id, 0, 0);
        Labels::set_position(
                id, play_screen.right - (Labels::get_width(id)+10),
                globals()->gInstrumentTop + value);
        break;

      case 'L':
        Labels::set_offset(id, 0, 0);
        Labels::set_position(id, 138, globals()->gInstrumentTop + value);
        break;

      case 'O':
        {
            spaceObjectType* o = GetObjectFromInitialNumber(value);
            Labels::set_offset(id, -(Labels::get_width(id)/2), 64);
            Labels::set_object(id, o);

            hintLine = true;
        }
        break;
    }
    attachPoint.v -= 2;
    Labels::set_attached_hint_line(id, hintLine, attachPoint);
}

void Messages::draw_message() {
    if (viewport.bottom == play_screen.bottom) {
        return;
    }

    const RgbColor& dark_blue = GetRGBTranslateColorShade(SKY_BLUE, DARKEST);
    const RgbColor& light_blue = GetRGBTranslateColorShade(SKY_BLUE, VERY_LIGHT);
    Rect message_bounds(play_screen.left, viewport.bottom, play_screen.right, play_screen.bottom);
    VideoDriver::driver()->fill_rect(message_bounds, light_blue);
    message_bounds.inset(0, 1);
    VideoDriver::driver()->fill_rect(message_bounds, dark_blue);

    longMessageType *tmessage = long_message_data;
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
