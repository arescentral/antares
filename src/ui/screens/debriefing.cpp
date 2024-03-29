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

#include <pn/output>
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

namespace antares {

namespace {

const usecs    kTypingDelay      = kMajorTick;
const int      kScoreTableHeight = 120;
const int      kTextWidth        = 300;
constexpr char kParDataStrings[] = "object";

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

}  // namespace

DebriefingScreen::DebriefingScreen(pn::string_view message)
        : _state(DONE), _data_item(initialize(message, false)) {}

DebriefingScreen::DebriefingScreen(
        pn::string_view message, game_ticks your_time, game_ticks par_time, int your_loss,
        int par_loss, int your_kill, int par_kill)
        : _state(TYPING),
          _data_item(initialize(message, true)),
          _score{StyledText::retro(
                  build_score_text(your_time, par_time, your_loss, par_loss, your_kill, par_kill),
                  {sys.fonts.button, _message_bounds.width(), 0, 2, 60},
                  GetRGBTranslateColorShade(Hue::GOLD, LIGHTEST),
                  GetRGBTranslateColorShade(Hue::GOLD, DARKEST))} {
    Rect score_area = _message_bounds;
    score_area.top  = score_area.bottom - kScoreTableHeight;

    _score_bounds = Rect(0, 0, _score.auto_width(), _score.height());
    _score_bounds.center_in(score_area);

    _score_bounds.offset(_pix_bounds.left, _pix_bounds.top);
}

void DebriefingScreen::become_front() {
    _score.hide();
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
    if (!_score.empty()) {
        _score.draw(_score_bounds);
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
        if (!_score.done()) {
            _next_update += kTypingDelay;
            _score.advance();
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

    BoxRect data_item{interface_item(text_bounds)};
    _pix_bounds     = data_item.outer_bounds();
    _message_bounds = text_bounds;
    _message_bounds.offset(-_pix_bounds.left, -_pix_bounds.top);
    return data_item;
}

pn::string DebriefingScreen::build_score_text(
        game_ticks your_time, game_ticks par_time, int your_loss, int par_loss, int your_kill,
        int par_kill) {
    pn::string tpl = Resource::text("par");

    auto data_strings = Resource::strings(kParDataStrings);

    const int your_mins  = duration_cast<secs>(your_time.time_since_epoch()).count() / 60;
    const int your_secs  = duration_cast<secs>(your_time.time_since_epoch()).count() % 60;
    const int par_mins   = duration_cast<secs>(par_time.time_since_epoch()).count() / 60;
    const int par_secs   = duration_cast<secs>(par_time.time_since_epoch()).count() % 60;
    const int your_score = score(your_time, par_time, your_loss, par_loss, your_kill, par_kill);
    const int par_score  = 100;

    pn::string your_minsecs, par_minsecs;
    your_minsecs = pn::format("{0}:{1}", your_mins, dec(your_secs, 2));
    if (par_time > game_ticks()) {
        par_minsecs = pn::format("{0}:{1}", par_mins, dec(par_secs, 2));
    } else {
        par_minsecs = data_strings.at(8).copy();
    }

    return pn::format(
            tpl.c_str(), your_minsecs, par_minsecs, your_loss, par_loss, your_kill, par_kill,
            your_score, par_score);
}

const char* stringify(DebriefingScreen::State state) {
    switch (state) {
        case DebriefingScreen::TYPING: return "TYPING";
        case DebriefingScreen::DONE: return "DONE";
    }
    return "?";
}

}  // namespace antares
