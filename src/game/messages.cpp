// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2017 The Antares Authors
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
#include "game/condition.hpp"
#include "game/globals.hpp"
#include "game/initial.hpp"
#include "game/instruments.hpp"
#include "game/labels.hpp"
#include "game/level.hpp"
#include "game/space-object.hpp"
#include "game/sys.hpp"
#include "lang/defines.hpp"
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

static const int32_t kMessageScreenLeft = 200;
static const int32_t kMessageScreenTop  = 454;

static const uint8_t kMessageColor       = RED;
static const ticks   kMessageMoveTime    = ticks(30);
static const ticks   kMessageDisplayTime = (kMessageMoveTime * 2 + secs(2));
static const ticks   kLowerTime          = (kMessageDisplayTime - kMessageMoveTime);
static const ticks   kRaiseTime          = kMessageMoveTime;

static const int32_t kStatusLabelLeft = 200;
static const int32_t kStatusLabelTop  = 50;
static const ticks   kStatusLabelAge  = secs(2);

static const int32_t kLongMessageVPad       = 5;
static const int32_t kLongMessageVPadDouble = 10;

static const int16_t kStringMessageID = 1;

static const int32_t kHBuffer = 4;

namespace {

template <typename T>
void clear(T& t) {
    using std::swap;
    T u = T();
    swap(t, u);
}

enum longMessageStageType {
    kNoStage    = 0,
    kStartStage = 1,
    kClipStage  = 2,
    kShowStage  = 3,
    kEndStage   = 4
};

}  // namespace

struct Messages::longMessageType {
    longMessageStageType        stage;
    ticks                       charDelayCount;
    Rect                        pictBounds;
    int32_t                     pictDelayCount;
    int32_t                     pictCurrentLeft;
    int32_t                     pictCurrentTop;
    int32_t                     time;
    int32_t                     textHeight;
    int16_t                     startResID;
    int16_t                     endResID;
    int16_t                     currentResID;
    int16_t                     lastResID;
    int16_t                     previousStartResID;
    int16_t                     previousEndResID;
    int16_t                     pictID;
    uint8_t                     backColor;
    sfz::String                 stringMessage;
    sfz::String                 lastStringMessage;
    bool                        newStringMessage;
    sfz::String                 text;
    std::unique_ptr<StyledText> retro_text;
    Point                       retro_origin;
    int32_t                     at_char;
    bool                        labelMessage;
    bool                        lastLabelMessage;
    Handle<Label>               labelMessageID;
};

ANTARES_GLOBAL std::queue<sfz::String> Messages::message_data;
ANTARES_GLOBAL Messages::longMessageType* Messages::long_message_data;
ANTARES_GLOBAL ticks Messages::time_count;

void MessageLabel_Set_Special(Handle<Label> id, const StringSlice& text);

void Messages::init() {
    longMessageType* tmessage = NULL;

    antares::clear(message_data);
    long_message_data = new longMessageType;

    g.message_label = Label::add(
            kMessageScreenLeft, kMessageScreenTop, 0, 0, SpaceObject::none(), false,
            kMessageColor);

    if (!g.message_label.get()) {
        throw Exception("Couldn't add a screen label.");
    }
    g.status_label = Label::add(
            kStatusLabelLeft, kStatusLabelTop, 0, 0, SpaceObject::none(), false,
            kStatusLabelColor);
    if (!g.status_label.get()) {
        throw Exception("Couldn't add a screen label.");
    }

    tmessage             = long_message_data;
    tmessage->startResID = tmessage->endResID = tmessage->lastResID = tmessage->currentResID = -1;
    tmessage->time                                                                           = 0;
    tmessage->stage      = kNoStage;
    tmessage->textHeight = 0;
    tmessage->retro_text.reset();
    tmessage->charDelayCount  = ticks(0);
    tmessage->pictBounds.left = tmessage->pictBounds.right = 0;
    tmessage->pictCurrentLeft                              = 0;
    tmessage->pictCurrentTop                               = 0;
    tmessage->pictID                                       = -1;
    tmessage->stringMessage.clear();
    tmessage->lastStringMessage.clear();
    tmessage->newStringMessage = false;
    tmessage->labelMessage     = false;
    tmessage->lastLabelMessage = false;
    tmessage->labelMessageID   = Label::none();
}

void Messages::clear() {
    longMessageType* tmessage;

    time_count = ticks(0);
    std::queue<sfz::String> empty;
    swap(message_data, empty);
    g.message_label = Label::add(
            kMessageScreenLeft, kMessageScreenTop, 0, 0, SpaceObject::none(), false,
            kMessageColor);
    g.status_label = Label::add(
            kStatusLabelLeft, kStatusLabelTop, 0, 0, SpaceObject::none(), false,
            kStatusLabelColor);

    tmessage                     = long_message_data;
    tmessage->startResID         = -1;
    tmessage->endResID           = -1;
    tmessage->currentResID       = -1;
    tmessage->lastResID          = -1;
    tmessage->textHeight         = 0;
    tmessage->previousStartResID = tmessage->previousEndResID = -1;
    tmessage                                                  = long_message_data;
    tmessage->stringMessage.clear();
    tmessage->lastStringMessage.clear();
    tmessage->newStringMessage = false;
    tmessage->labelMessage     = false;
    tmessage->lastLabelMessage = false;
    tmessage->retro_text.reset();
    g.bottom_border          = 0;
    tmessage->labelMessageID = Label::add(0, 0, 0, 0, SpaceObject::none(), false, SKY_BLUE);
    tmessage->labelMessageID->set_keep_on_screen_anyway(true);
}

void Messages::add(const sfz::PrintItem& message) {
    message_data.emplace(message);
}

void Messages::start(int16_t startResID, int16_t endResID) {
    longMessageType* tmessage;

    tmessage = long_message_data;

    if (tmessage->currentResID != -1) {
        tmessage->startResID   = startResID;
        tmessage->endResID     = endResID;
        tmessage->currentResID = startResID - 1;
        advance();
    } else {
        tmessage->previousStartResID = tmessage->startResID;
        tmessage->previousEndResID   = tmessage->endResID;
        tmessage->startResID         = startResID;
        tmessage->endResID           = endResID;
        tmessage->currentResID       = startResID;
        tmessage->time               = 0;
        tmessage->stage              = kStartStage;
        tmessage->textHeight         = 0;
        tmessage->retro_text.reset();
        tmessage->charDelayCount  = ticks(0);
        tmessage->pictBounds.left = tmessage->pictBounds.right = 0;
        // tmessage->pictDelayCount;
        tmessage->pictCurrentLeft = 0;
        tmessage->pictCurrentTop  = 0;
        tmessage->pictID          = -1;
    }
}

void Messages::clip(void)

{
    longMessageType*   tmessage;
    unique_ptr<String> textData;

    tmessage = long_message_data;
    if ((tmessage->currentResID != tmessage->lastResID) || (tmessage->newStringMessage)) {
        if (tmessage->lastResID >= 0) {
            g.bottom_border = 0;
        }

        // draw in offscreen world
        if ((tmessage->currentResID >= 0) && (tmessage->stage == kClipStage)) {
            if (tmessage->currentResID == kStringMessageID) {
                textData.reset(new String);
                if (textData.get() != NULL) {
                    print(*textData, tmessage->stringMessage);
                }
                tmessage->labelMessage = false;
            } else {
                Resource rsrc("text", "txt", tmessage->currentResID);
                textData.reset(new String(utf8::decode(rsrc.data())));
                Replace_KeyCode_Strings_With_Actual_Key_Names(textData.get(), KEY_LONG_NAMES, 0);
                if (textData->at(0) == '#') {
                    tmessage->labelMessage = true;
                } else
                    tmessage->labelMessage = false;
            }
            if (textData.get() != NULL) {
                const RgbColor& light_blue = GetRGBTranslateColorShade(SKY_BLUE, VERY_LIGHT);
                const RgbColor& dark_blue  = GetRGBTranslateColorShade(SKY_BLUE, DARKEST);
                tmessage->text.assign(*textData);
                tmessage->retro_text.reset(new StyledText(sys.fonts.tactical));
                tmessage->retro_text->set_fore_color(light_blue);
                tmessage->retro_text->set_back_color(dark_blue);
                tmessage->retro_text->set_retro_text(*textData);
                tmessage->retro_text->set_tab_width(60);
                tmessage->retro_text->wrap_to(
                        viewport().width() - kHBuffer - sys.fonts.tactical->logicalWidth + 1, 0,
                        0);
                tmessage->textHeight = tmessage->retro_text->height();
                tmessage->textHeight += kLongMessageVPadDouble;
                tmessage->retro_origin =
                        Point(viewport().left + kHBuffer,
                              viewport().bottom + sys.fonts.tactical->ascent + kLongMessageVPad);
                tmessage->at_char = 0;

                if (tmessage->labelMessage == false) {
                    g.bottom_border = tmessage->textHeight;
                } else {
                    g.bottom_border = 0;
                }
                tmessage->stage = kShowStage;

                /*
                tmessage->retroTextSpec.topBuffer = kMessageCharTopBuffer;
                tmessage->retroTextSpec.bottomBuffer = kMessageCharBottomBuffer;
                tmessage->retroTextSpec.thisPosition = 0;
                tmessage->retroTextSpec.lineCount = 0;
                tmessage->retroTextSpec.linePosition = 0;
                tmessage->retroTextSpec.xpos = viewport.left + kHBuffer;
                tmessage->retroTextSpec.ypos = viewport.bottom + mDirectFontAscent() +
                kLongMessageVPad + tmessage->retroTextSpec.topBuffer;
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
            g.bottom_border = 0;
            tmessage->stage = kClipStage;
        }
    }
}

void Messages::draw_long_message(ticks time_pass) {
    Rect             tRect, uRect;
    Rect             lRect, cRect;
    longMessageType* tmessage;
    RgbColor         color;

    tmessage = long_message_data;
    if ((tmessage->currentResID != tmessage->lastResID) || (tmessage->newStringMessage)) {
        // TODO(sfiera): figure out what this meant.
        //
        // we check scenario conditions here for ambrosia tutorial
        // but not during net game -- other players wouldn't care what message
        // we were looking at
        //
        // if ( !(globals()->gOptions & kOptionNetworkOn))
        // {
        CheckLevelConditions();
        // }

        if ((tmessage->lastResID >= 0) && (tmessage->lastLabelMessage)) {
            tmessage->labelMessageID->set_age(kMinorTick);
        }

        // draw in offscreen world
        if ((tmessage->currentResID >= 0) && (tmessage->stage == kShowStage)) {
            if (tmessage->retro_text.get() != NULL) {
                if (tmessage->labelMessage) {
                    tmessage->labelMessageID->set_age(ticks(0));

                    if (tmessage->retro_text.get() != NULL) {
                        MessageLabel_Set_Special(tmessage->labelMessageID, tmessage->text);
                    }
                }
            }
        }
        if ((tmessage->stage == kShowStage) || (tmessage->currentResID < 0)) {
            tmessage->lastResID        = tmessage->currentResID;
            tmessage->lastLabelMessage = tmessage->labelMessage;
            tmessage->newStringMessage = false;
        }
    } else if (
            (tmessage->currentResID >= 0) && (tmessage->retro_text.get() != NULL) &&
            (tmessage->at_char < tmessage->retro_text->size()) &&
            (tmessage->stage == kShowStage) && !tmessage->labelMessage) {
        time_pass = std::min(time_pass, ticks(tmessage->retro_text->size() - tmessage->at_char));
        // Play teletype sound at least once every 3 ticks.
        tmessage->charDelayCount += time_pass;
        if (tmessage->charDelayCount > ticks(0)) {
            sys.sound.teletype();
            while (tmessage->charDelayCount > ticks(0)) {
                tmessage->charDelayCount -= kMajorTick;
            }
        }
        tmessage->at_char += time_pass.count();
    }
}

void Messages::end() {
    longMessageType* tmessage;

    tmessage                     = long_message_data;
    tmessage->previousStartResID = tmessage->startResID;
    tmessage->previousEndResID   = tmessage->endResID;
    tmessage->startResID         = -1;
    tmessage->endResID           = -1;
    tmessage->currentResID       = -1;
    tmessage->stage              = kStartStage;
    tmessage->retro_text.reset();
    tmessage->lastStringMessage.assign(tmessage->stringMessage);
}

void Messages::advance() {
    longMessageType* tmessage;

    tmessage = long_message_data;
    if (tmessage->currentResID != -1) {
        if (tmessage->currentResID < tmessage->endResID) {
            tmessage->currentResID++;
            tmessage->stage = kStartStage;
        } else {
            end();
        }
    }
}

void Messages::previous() {
    longMessageType* tmessage;

    tmessage = long_message_data;
    if (tmessage->currentResID != -1) {
        if (tmessage->currentResID > tmessage->startResID) {
            tmessage->currentResID--;
            tmessage->stage = kStartStage;
        }
    }
}

void Messages::replay() {
    longMessageType* tmessage;

    tmessage = long_message_data;
    if ((tmessage->previousStartResID >= 0) && (tmessage->currentResID < 0)) {
        tmessage->stringMessage.assign(tmessage->lastStringMessage);
        start(tmessage->previousStartResID, tmessage->previousEndResID);
    }
}

// WARNING: RELIES ON kMessageNullCharacter (SPACE CHARACTER #32) >> NOT WORLD-READY <<

void Messages::draw_message_screen(ticks by_units) {
    // increase the amount of time current message has been shown
    time_count += by_units;

    // if it's been shown for too long, then get the next message
    if (time_count > kMessageDisplayTime) {
        time_count = ticks(0);
        if (!message_data.empty()) {
            message_data.pop();
        }
    }

    if (!message_data.empty()) {
        const String& message = message_data.front();

        if (time_count < kRaiseTime) {
            g.message_label->set_position(
                    kMessageScreenLeft, viewport().bottom - time_count.count());
        } else if (time_count > kLowerTime) {
            g.message_label->set_position(
                    kMessageScreenLeft,
                    viewport().bottom - (kMessageDisplayTime - time_count).count());
        }

        g.message_label->set_string(message);
    } else {
        g.message_label->clear_string();
        time_count = ticks(0);
    }
}

void Messages::set_status(const StringSlice& status, uint8_t color) {
    g.status_label->set_color(color);
    g.status_label->set_string(status);
    g.status_label->set_age(kStatusLabelAge);
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
void MessageLabel_Set_Special(Handle<Label> label, const StringSlice& text) {
    char    whichType;
    int32_t value = 0;
    Point   attachPoint;
    bool    hintLine = false;

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
        attachPoint.v += instrument_top();
        if (attachPoint.h >= (kSmallScreenWidth - kRightPanelWidth)) {
            attachPoint.h =
                    (attachPoint.h - (kSmallScreenWidth - kRightPanelWidth)) + play_screen().right;
        }
        ++it;
    }

    String message;
    while (it != text.end()) {
        message.push(1, *it);
        ++it;
    }

    label->set_string(message);
    label->set_keep_on_screen_anyway(true);

    switch (whichType) {
        case 'R':
            label->set_offset(0, 0);
            label->set_position(
                    play_screen().right - (label->get_width() + 10), instrument_top() + value);
            break;

        case 'L':
            label->set_offset(0, 0);
            label->set_position(138, instrument_top() + value);
            break;

        case 'O': {
            auto o = GetObjectFromInitialNumber(value);
            label->set_offset(-(label->get_width() / 2), 64);
            label->set_object(o);

            hintLine = true;
        } break;
    }
    attachPoint.v -= 2;
    label->set_attached_hint_line(hintLine, attachPoint);
}

void Messages::draw_message() {
    if ((g.bottom_border == 0) || (long_message_data->currentResID < 0)) {
        return;
    }

    const RgbColor& dark_blue  = GetRGBTranslateColorShade(SKY_BLUE, DARKEST);
    const RgbColor& light_blue = GetRGBTranslateColorShade(SKY_BLUE, VERY_LIGHT);
    Rect            message_bounds(
            play_screen().left, viewport().bottom, play_screen().right, play_screen().bottom);
    {
        Rects rects;
        rects.fill(message_bounds, light_blue);
        message_bounds.inset(0, 1);
        rects.fill(message_bounds, dark_blue);
    }

    Rect bounds(viewport().left, viewport().bottom, viewport().right, play_screen().bottom);
    bounds.inset(kHBuffer, 0);
    bounds.top += kLongMessageVPad;
    long_message_data->retro_text->draw_range(bounds, 0, long_message_data->at_char);
    // The final char is a newline; don't display a cursor rect for it.
    if ((0 < long_message_data->at_char) &&
        (long_message_data->at_char < (long_message_data->retro_text->size() - 1))) {
        long_message_data->retro_text->draw_cursor(bounds, long_message_data->at_char);
    }
}

}  // namespace antares
