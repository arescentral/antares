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

#include <pn/file>
#include <sfz/sfz.hpp>
#include <vector>

#include "data/resource.hpp"
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

using sfz::dec;
using std::chrono::duration_cast;
using std::unique_ptr;
using std::vector;

namespace antares {

namespace {

const usecs kTypingDelay      = kMajorTick;
const int   kScoreTableHeight = 120;
const int   kTextWidth        = 300;

void string_replace(pn::string_ref s, pn::string_view in, pn::string_view out) {
    size_t index = s.find(in);
    while (index != s.npos) {
        s.replace(index, in.size(), out);
        index = s.find(in, index + 1);
    }
}

BoxRectData interface_item(const Rect& text_bounds) {
    BoxRectData r;
    r.bounds = text_bounds;
    r.hue    = Hue::GOLD;
    r.style  = InterfaceStyle::LARGE;
    r.label.emplace("Results");
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

unique_ptr<StyledText> style_score_text(pn::string text) {
    unique_ptr<StyledText> result(new StyledText(sys.fonts.button));
    result->set_fore_color(GetRGBTranslateColorShade(Hue::GOLD, LIGHTEST));
    result->set_back_color(GetRGBTranslateColorShade(Hue::GOLD, DARKEST));
    result->set_retro_text(text);
    return result;
}

}  // namespace

DebriefingScreen::DebriefingScreen(pn::string_view message)
        : _state(DONE), _typed_chars(0), _data_item(initialize(message, false)) {}

DebriefingScreen::DebriefingScreen(
        pn::string_view message, game_ticks your_time, game_ticks par_time, int your_loss,
        int par_loss, int your_kill, int par_kill)
        : _state(TYPING), _typed_chars(0), _data_item(initialize(message, true)) {
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
    _data_item.draw({0, 0}, KEYBOARD_MOUSE);

    draw_text_in_rect(interface_bounds, _message, InterfaceStyle::LARGE, Hue::GOLD);

    RgbColor bracket_color  = GetRGBTranslateColorShade(Hue::GOLD, LIGHTEST);
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
        throw std::runtime_error(pn::format(
                                         "DebriefingScreen::fire_timer() called but _state is {0}",
                                         stringify(_state))
                                         .c_str());
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

BoxRect DebriefingScreen::initialize(pn::string_view message, bool do_score) {
    _message = message.copy();

    int text_height = GetInterfaceTextHeightFromWidth(_message, InterfaceStyle::LARGE, kTextWidth);
    Rect text_bounds(0, 0, kTextWidth, text_height);
    if (do_score) {
        text_bounds.bottom += kScoreTableHeight;
    }
    text_bounds.center_in(viewport());

    BoxRect data_item = interface_item(text_bounds);
    _pix_bounds       = data_item.outer_bounds();
    _message_bounds   = text_bounds;
    _message_bounds.offset(-_pix_bounds.left, -_pix_bounds.top);
    return data_item;
}

pn::string DebriefingScreen::build_score_text(
        game_ticks your_time, game_ticks par_time, int your_loss, int par_loss, int your_kill,
        int par_kill) {
    pn::string text = Resource::text(6000);

    auto strings = Resource::strings(6000);

    const int your_mins  = duration_cast<secs>(your_time.time_since_epoch()).count() / 60;
    const int your_secs  = duration_cast<secs>(your_time.time_since_epoch()).count() % 60;
    const int par_mins   = duration_cast<secs>(par_time.time_since_epoch()).count() / 60;
    const int par_secs   = duration_cast<secs>(par_time.time_since_epoch()).count() % 60;
    const int your_score = score(your_time, par_time, your_loss, par_loss, your_kill, par_kill);
    const int par_score  = 100;

    string_replace(text, strings.at(0), pn::dump(your_mins, pn::dump_short));
    string_replace(text, strings.at(1), dec(your_secs, 2));
    if (par_time > game_ticks()) {
        string_replace(text, strings.at(2), pn::dump(par_mins, pn::dump_short));
        pn::string secs_string;
        secs_string += ":";
        secs_string += dec(par_secs, 2);
        string_replace(text, strings.at(3), secs_string);
    } else {
        auto data_strings = Resource::strings(6002);
        string_replace(text, strings.at(2), data_strings.at(8));  // = "N/A"
        string_replace(text, strings.at(3), "");
    }
    string_replace(text, strings.at(4), pn::dump(your_loss, pn::dump_short));
    string_replace(text, strings.at(5), pn::dump(par_loss, pn::dump_short));
    string_replace(text, strings.at(6), pn::dump(your_kill, pn::dump_short));
    string_replace(text, strings.at(7), pn::dump(par_kill, pn::dump_short));
    string_replace(text, strings.at(8), pn::dump(your_score, pn::dump_short));
    string_replace(text, strings.at(9), pn::dump(par_score, pn::dump_short));
    return text;
}

const char* stringify(DebriefingScreen::State state) {
    switch (state) {
        case DebriefingScreen::TYPING: return "TYPING";
        case DebriefingScreen::DONE: return "DONE";
    }
    return "?";
}

}  // namespace antares
