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

#include "ui/screens/debriefing.hpp"

#include <sfz/sfz.hpp>
#include <vector>

#include "data/resource.hpp"
#include "data/string-list.hpp"
#include "drawing/color.hpp"
#include "drawing/interface.hpp"
#include "drawing/shapes.hpp"
#include "drawing/styled-text.hpp"
#include "drawing/text.hpp"
#include "game/globals.hpp"
#include "game/sys.hpp"
#include "game/time.hpp"
#include "sound/fx.hpp"
#include "ui/card.hpp"
#include "video/driver.hpp"

using sfz::Bytes;
using sfz::Exception;
using sfz::PrintItem;
using sfz::String;
using sfz::dec;
using sfz::format;
using std::chrono::duration_cast;
using std::unique_ptr;
using std::vector;

namespace utf8 = sfz::utf8;

namespace antares {

namespace {

const usecs kTypingDelay      = kMajorTick;
const int   kScoreTableHeight = 120;
const int   kTextWidth        = 300;

void string_replace(String* s, const String& in, const PrintItem& out) {
    size_t index = s->find(in);
    while (index != String::npos) {
        String out_string;
        out.print_to(out_string);
        s->replace(index, in.size(), out_string);
        index = s->find(in, index + 1);
    }
}

LabeledRect interface_item(const Rect& text_bounds) {
    return LabeledRect(0, text_bounds, {2001, 29}, GOLD, kLarge);
}

Rect pix_bounds(const InterfaceItem& item) {
    Rect r;
    GetAnyInterfaceItemGraphicBounds(item, &r);
    return r;
}

int score_low_target(double yours, double par, double value) {
    if (par == 0) {
        return (yours == 0) ? value : 0;
    }
    const double ratio = yours / par;
    if (ratio <= 1.0) {
        return value * std::min(2.0, (3 - (2 * ratio)));
    } else {
        return value * std::max(0.0, 2 - ratio);
    }
}

int score_high_target(double yours, double par, double value) {
    if (par == 0) {
        return (yours == 0) ? value : (value * 2);
    }
    const double ratio = yours / par;
    if (ratio >= 1.0) {
        return value * std::min(2.0, ratio);
    } else {
        return value * std::max(0.0, ratio * 2 - 1);
    }
}

int score(
        game_ticks your_length, game_ticks par_length, int your_loss, int par_loss, int your_kill,
        int par_kill) {
    int score = 0;
    score += score_low_target(
            duration_cast<secs>(your_length.time_since_epoch()).count(),
            duration_cast<secs>(par_length.time_since_epoch()).count(), 50);
    score += score_low_target(your_loss, par_loss, 30);
    score += score_high_target(your_kill, par_kill, 20);
    return score;
}

unique_ptr<StyledText> style_score_text(String text) {
    unique_ptr<StyledText> result(new StyledText(sys.fonts.button));
    result->set_fore_color(GetRGBTranslateColorShade(GOLD, VERY_LIGHT));
    result->set_back_color(GetRGBTranslateColorShade(GOLD, DARKEST));
    result->set_retro_text(text);
    return result;
}

}  // namespace

DebriefingScreen::DebriefingScreen(int text_id)
        : _state(DONE), _typed_chars(0), _data_item(initialize(text_id, false)) {}

DebriefingScreen::DebriefingScreen(
        int text_id, game_ticks your_time, game_ticks par_time, int your_loss, int par_loss,
        int your_kill, int par_kill)
        : _state(TYPING), _typed_chars(0), _data_item(initialize(text_id, true)) {
    Rect score_area = _message_bounds;
    score_area.top  = score_area.bottom - kScoreTableHeight;

    _score = style_score_text(
            build_score_text(your_time, par_time, your_loss, par_loss, your_kill, par_kill));
    _score->set_tab_width(60);
    _score->wrap_to(_message_bounds.width(), 0, 2);
    _score_bounds = Rect(0, 0, _score->auto_width(), _score->height());
    _score_bounds.center_in(score_area);

    _score_bounds.offset(_pix_bounds.left, _pix_bounds.top);
}

void DebriefingScreen::become_front() {
    _typed_chars = 0;
    if (_state == TYPING) {
        _next_update = now() + kTypingDelay;
    } else {
        _next_update = wall_time();
    }
}

void DebriefingScreen::resign_front() {}

void DebriefingScreen::draw() const {
    next()->draw();
    Rects().fill(_pix_bounds, RgbColor::black());
    if (_score) {
        _score->draw_range(_score_bounds, 0, _typed_chars);
    }
    Rect interface_bounds = _message_bounds;
    interface_bounds.offset(_pix_bounds.left, _pix_bounds.top);
    draw_interface_item(_data_item, KEYBOARD_MOUSE);

    draw_text_in_rect(interface_bounds, _message, kLarge, GOLD);

    RgbColor bracket_color  = GetRGBTranslateColorShade(GOLD, VERY_LIGHT);
    Rect     bracket_bounds = _score_bounds;
    bracket_bounds.inset(-2, -2);
    draw_vbracket(Rects(), bracket_bounds, bracket_color);
}

void DebriefingScreen::mouse_down(const MouseDownEvent& event) {
    static_cast<void>(event);
    if (_state == DONE) {
        stack()->pop(this);
    }
}

void DebriefingScreen::key_down(const KeyDownEvent& event) {
    static_cast<void>(event);
    if (_state == DONE) {
        stack()->pop(this);
    }
}

void DebriefingScreen::gamepad_button_down(const GamepadButtonDownEvent& event) {
    static_cast<void>(event);
    if (_state == DONE) {
        stack()->pop(this);
    }
}

bool DebriefingScreen::next_timer(wall_time& time) {
    if (_state == TYPING) {
        time = _next_update;
        return true;
    }
    return false;
}

void DebriefingScreen::fire_timer() {
    if (_state != TYPING) {
        throw Exception(format("DebriefingScreen::fire_timer() called but _state is {0}", _state));
    }
    sys.sound.teletype();
    wall_time now = antares::now();
    while (_next_update <= now) {
        if (_typed_chars < _score->size()) {
            _next_update += kTypingDelay;
            ++_typed_chars;
        } else {
            _next_update = wall_time();
            _state       = DONE;
            break;
        }
    }
}

LabeledRect DebriefingScreen::initialize(int text_id, bool do_score) {
    Resource rsrc("text", "txt", text_id);
    _message.assign(utf8::decode(rsrc.data()));

    int  text_height = GetInterfaceTextHeightFromWidth(_message, kLarge, kTextWidth);
    Rect text_bounds(0, 0, kTextWidth, text_height);
    if (do_score) {
        text_bounds.bottom += kScoreTableHeight;
    }
    text_bounds.center_in(viewport());

    LabeledRect data_item = interface_item(text_bounds);
    _pix_bounds           = pix_bounds(data_item);
    _message_bounds       = text_bounds;
    _message_bounds.offset(-_pix_bounds.left, -_pix_bounds.top);
    return data_item;
}

String DebriefingScreen::build_score_text(
        game_ticks your_time, game_ticks par_time, int your_loss, int par_loss, int your_kill,
        int par_kill) {
    Resource rsrc("text", "txt", 6000);
    String   text(utf8::decode(rsrc.data()));

    StringList strings(6000);

    const int your_mins  = duration_cast<secs>(your_time.time_since_epoch()).count() / 60;
    const int your_secs  = duration_cast<secs>(your_time.time_since_epoch()).count() % 60;
    const int par_mins   = duration_cast<secs>(par_time.time_since_epoch()).count() / 60;
    const int par_secs   = duration_cast<secs>(par_time.time_since_epoch()).count() % 60;
    const int your_score = score(your_time, par_time, your_loss, par_loss, your_kill, par_kill);
    const int par_score  = 100;

    string_replace(&text, strings.at(0), your_mins);
    string_replace(&text, strings.at(1), dec(your_secs, 2));
    if (par_time > game_ticks()) {
        string_replace(&text, strings.at(2), par_mins);
        String secs_string;
        print(secs_string, format(":{0}", dec(par_secs, 2)));
        string_replace(&text, strings.at(3), secs_string);
    } else {
        StringList data_strings(6002);
        string_replace(&text, strings.at(2), data_strings.at(8));  // = "N/A"
        string_replace(&text, strings.at(3), "");
    }
    string_replace(&text, strings.at(4), your_loss);
    string_replace(&text, strings.at(5), par_loss);
    string_replace(&text, strings.at(6), your_kill);
    string_replace(&text, strings.at(7), par_kill);
    string_replace(&text, strings.at(8), your_score);
    string_replace(&text, strings.at(9), par_score);
    return text;
}

void print_to(sfz::PrintTarget out, DebriefingScreen::State state) {
    switch (state) {
        case DebriefingScreen::TYPING: print(out, "TYPING"); return;
        case DebriefingScreen::DONE: print(out, "DONE"); return;
    }
    print(out, static_cast<int64_t>(state));
}

}  // namespace antares
