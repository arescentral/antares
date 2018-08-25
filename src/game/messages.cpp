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

using std::unique_ptr;

namespace antares {

static const int32_t kMessageScreenLeft = 200;
static const int32_t kMessageScreenTop  = 454;

static const Hue   kMessageColor       = Hue::RED;
static const ticks kMessageMoveTime    = ticks(30);
static const ticks kMessageDisplayTime = (kMessageMoveTime * 2 + secs(2));
static const ticks kLowerTime          = (kMessageDisplayTime - kMessageMoveTime);
static const ticks kRaiseTime          = kMessageMoveTime;

static const int32_t kStatusLabelLeft = 200;
static const int32_t kStatusLabelTop  = 50;
static const ticks   kStatusLabelAge  = secs(2);

static const int32_t kLongMessageVPad       = 5;
static const int32_t kLongMessageVPadDouble = 10;

static const int32_t kHBuffer = 4;

static const int16_t kAutoPilotOnString  = 8;
static const int16_t kAutoPilotOffString = 9;
static const Hue     kStatusLabelColor   = Hue::AQUA;
static const Hue     kStatusWarnColor    = Hue::PINK;

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
    longMessageStageType           stage          = kNoStage;
    ticks                          charDelayCount = ticks(0);
    sfz::optional<int64_t>         start_id;
    const std::vector<pn::string>* pages              = nullptr;
    int16_t                        current_page_index = -1;
    int16_t                        last_page_index    = -1;
    uint8_t                        backColor          = 0;
    pn::string                     text               = "";
    std::unique_ptr<StyledText>    retro_text         = nullptr;
    Point                          retro_origin       = {0, 0};
    int32_t                        at_char            = 0;
    bool                           labelMessage       = false;
    bool                           lastLabelMessage   = false;
    Handle<Label>                  labelMessageID     = Label::none();

    bool have_pages() const { return !pages->empty(); }
    bool have_current() const { return current_page_index >= 0; }
    bool had_current() const { return last_page_index >= 0; }
    bool have_next() const { return (current_page_index + 1) < pages->size(); }
    bool have_previous() const { return current_page_index > 0; }
    bool was_updated() const { return current_page_index != last_page_index; }
};

ANTARES_GLOBAL std::queue<pn::string> Messages::message_data;
ANTARES_GLOBAL Messages::longMessageType* Messages::long_message_data;
ANTARES_GLOBAL ticks Messages::time_count;

void MessageLabel_Set_Special(Handle<Label> id, pn::string_view text);

void Messages::init() {
    antares::clear(message_data);
    long_message_data = new longMessageType;

    g.message_label = Label::add(
            kMessageScreenLeft, kMessageScreenTop, 0, 0, SpaceObject::none(), false,
            kMessageColor);

    if (!g.message_label.get()) {
        throw std::runtime_error("Couldn't add a screen label.");
    }
    g.status_label = Label::add(
            kStatusLabelLeft, kStatusLabelTop, 0, 0, SpaceObject::none(), false,
            kStatusLabelColor);
    if (!g.status_label.get()) {
        throw std::runtime_error("Couldn't add a screen label.");
    }

    *long_message_data = longMessageType();
}

void Messages::clear() {
    time_count = ticks(0);
    std::queue<pn::string> empty;
    swap(message_data, empty);
    g.message_label = Label::add(
            kMessageScreenLeft, kMessageScreenTop, 0, 0, SpaceObject::none(), false,
            kMessageColor);
    g.status_label = Label::add(
            kStatusLabelLeft, kStatusLabelTop, 0, 0, SpaceObject::none(), false,
            kStatusLabelColor);

    *long_message_data = longMessageType();
    g.bottom_border    = 0;
    long_message_data->labelMessageID =
            Label::add(0, 0, 0, 0, SpaceObject::none(), false, Hue::SKY_BLUE);
    long_message_data->labelMessageID->set_keep_on_screen_anyway(true);
}

void Messages::add(pn::string_view message) { message_data.emplace(message.copy()); }

void Messages::start(sfz::optional<int64_t> start_id, const std::vector<pn::string>* pages) {
    longMessageType* m = long_message_data;
    if (!m->have_current()) {
        m->retro_text.reset();
        m->charDelayCount = ticks(0);
    }
    m->start_id           = start_id;
    m->pages              = pages;
    m->current_page_index = 0;
    m->last_page_index    = -1;  // Force clip() to be run.
    m->stage              = kStartStage;
}

void Messages::clip() {
    longMessageType* m = long_message_data;
    if (!m->was_updated()) {
        return;
    }
    g.bottom_border = 0;

    if (!m->have_current() || (m->stage != kClipStage)) {
        m->stage = kClipStage;
        return;
    }

    pn::string text = (*m->pages)[m->current_page_index].copy();
    Replace_KeyCode_Strings_With_Actual_Key_Names(text, KEY_LONG_NAMES, 0);
    if (*text.begin() == pn::rune{'#'}) {
        m->labelMessage = true;
    } else {
        m->labelMessage = false;
    }

    const RgbColor& light_blue = GetRGBTranslateColorShade(Hue::SKY_BLUE, LIGHTEST);
    const RgbColor& dark_blue  = GetRGBTranslateColorShade(Hue::SKY_BLUE, DARKEST);
    m->retro_text.reset(new StyledText(sys.fonts.tactical));
    m->retro_text->set_fore_color(light_blue);
    m->retro_text->set_back_color(dark_blue);
    m->retro_text->set_retro_text(text);
    m->retro_text->set_tab_width(60);
    m->retro_text->wrap_to(
            viewport().width() - kHBuffer - sys.fonts.tactical.logicalWidth + 1, 0, 0);
    m->retro_origin =
            Point(viewport().left + kHBuffer,
                  viewport().bottom + sys.fonts.tactical.ascent + kLongMessageVPad);
    m->text    = std::move(text);
    m->at_char = 0;

    if (!m->labelMessage) {
        g.bottom_border = m->retro_text->height() + kLongMessageVPadDouble;
    }
    m->stage = kShowStage;
}

void Messages::draw_long_message(ticks time_pass) {
    longMessageType* m = long_message_data;

    if (m->was_updated()) {
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

        if (m->had_current() && m->lastLabelMessage) {
            m->labelMessageID->set_age(kMinorTick);
        }

        // draw in offscreen world
        if (m->have_current() && (m->stage == kShowStage)) {
            if (m->retro_text.get() != NULL) {
                if (m->labelMessage) {
                    m->labelMessageID->set_age(ticks(0));

                    if (m->retro_text.get() != NULL) {
                        MessageLabel_Set_Special(m->labelMessageID, m->text);
                    }
                }
            }
        }
        if ((m->stage == kShowStage) || !m->have_current()) {
            m->last_page_index  = m->current_page_index;
            m->lastLabelMessage = m->labelMessage;
        }
    } else if (
            m->have_current() && (m->retro_text.get() != NULL) &&
            (m->at_char < m->retro_text->size()) && (m->stage == kShowStage) && !m->labelMessage) {
        time_pass = std::min(time_pass, ticks(m->retro_text->size() - m->at_char));
        // Play teletype sound at least once every 3 ticks.
        m->charDelayCount += time_pass;
        if (m->charDelayCount > ticks(0)) {
            sys.sound.teletype();
            while (m->charDelayCount > ticks(0)) {
                m->charDelayCount -= kMajorTick;
            }
        }
        m->at_char += time_pass.count();
    }
}

void Messages::end() {
    longMessageType* m    = long_message_data;
    m->current_page_index = -1;
    m->stage              = kStartStage;
    m->retro_text.reset();
}

void Messages::advance() {
    longMessageType* m = long_message_data;
    if (!m->have_current()) {
        return;
    }
    if (m->have_next()) {
        m->current_page_index++;
        m->stage = kStartStage;
    } else {
        end();
    }
}

void Messages::previous() {
    longMessageType* m = long_message_data;
    if (m->have_current() && m->have_previous()) {
        m->current_page_index--;
        m->stage = kStartStage;
    }
}

void Messages::replay() {
    longMessageType* m = long_message_data;
    if (m->have_pages() && !m->have_current()) {
        start(m->start_id, m->pages);
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
        pn::string_view message = message_data.front();

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

void Messages::set_status(pn::string_view status, Hue hue) {
    g.status_label->set_hue(hue);
    g.status_label->set_string(status);
    g.status_label->set_age(kStatusLabelAge);
}

void Messages::zoom(Zoom zoom) {
    set_status(sys.messages.at(static_cast<int>(zoom)), kStatusLabelColor);
}
void Messages::autopilot(bool on) {
    set_status(sys.messages.at(on ? kAutoPilotOnString : kAutoPilotOffString), kStatusLabelColor);
}
void Messages::shields_low() { set_status("WARNING: Shields Low", kStatusWarnColor); }
void Messages::max_ships_built() { set_status("Maximum number of ships built", Hue::ORANGE); }

std::pair<sfz::optional<int64_t>, int> Messages::current() {
    return {long_message_data->start_id, long_message_data->current_page_index};
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
void MessageLabel_Set_Special(Handle<Label> label, pn::string_view text) {
    pn::rune whichType;
    int32_t  value = 0;
    Point    attachPoint;
    bool     hintLine = false;

    pn::string_view::iterator it = text.begin();

    // if not legal, bail
    if (*it != pn::rune{'#'}) {
        return;
    }

    ++it;

    whichType = *it;
    ++it;
    while ((it != text.end()) && (*it != pn::rune{'#'})) {
        value *= 10;
        value += (*it).value() - '0';
        ++it;
    }

    ++it;
    if (*it == pn::rune{'#'}) {  // also a hint line attached
        hintLine = true;
        ++it;
        // h coord
        while ((it != text.end()) && (*it != pn::rune{','})) {
            attachPoint.h *= 10;
            attachPoint.h += (*it).value() - '0';
            ++it;
        }

        ++it;

        while ((it != text.end()) && (*it != pn::rune{'#'})) {
            attachPoint.v *= 10;
            attachPoint.v += (*it).value() - '0';
            ++it;
        }
        attachPoint.v += instrument_top();
        if (attachPoint.h >= (kSmallScreenWidth - kRightPanelWidth)) {
            attachPoint.h =
                    (attachPoint.h - (kSmallScreenWidth - kRightPanelWidth)) + play_screen().right;
        }
        ++it;
    }

    pn::string message;
    while (it != text.end()) {
        message += *it;
        ++it;
    }

    label->set_string(message);
    label->set_keep_on_screen_anyway(true);

    switch (whichType.value()) {
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
            auto o = GetObjectFromInitialNumber(Handle<const Initial>(value));
            label->set_offset(-(label->get_width() / 2), 64);
            label->set_object(o);

            hintLine = true;
        } break;
    }
    attachPoint.v -= 2;
    label->set_attached_hint_line(hintLine, attachPoint);
}

void Messages::draw_message() {
    if ((g.bottom_border == 0) || !long_message_data->have_current()) {
        return;
    }

    const RgbColor& dark_blue  = GetRGBTranslateColorShade(Hue::SKY_BLUE, DARKEST);
    const RgbColor& light_blue = GetRGBTranslateColorShade(Hue::SKY_BLUE, LIGHTEST);
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

pn::string_view Messages::pause_string() { return sys.messages.at(10); }

}  // namespace antares
