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

#include "ui/screens/debriefing.hpp"

#include <vector>
#include <sfz/sfz.hpp>

#include "data/resource.hpp"
#include "data/string-list.hpp"
#include "drawing/color.hpp"
#include "drawing/interface.hpp"
#include "drawing/retro-text.hpp"
#include "drawing/text.hpp"
#include "game/globals.hpp"
#include "game/time.hpp"
#include "sound/fx.hpp"
#include "ui/card.hpp"
#include "video/driver.hpp"

using sfz::Bytes;
using sfz::Exception;
using sfz::String;
using sfz::PrintItem;
using sfz::dec;
using sfz::format;
using std::vector;

namespace macroman = sfz::macroman;

namespace antares {

namespace {

const int64_t kTypingDelay = 1e6 / 20;
const int kScoreTableHeight = 120;
const int kTextWidth = 300;

void string_replace(String* s, const String& in, const PrintItem& out) {
    size_t index = s->find(in);
    while (index != String::npos) {
        String out_string;
        out.print_to(out_string);
        s->replace(index, in.size(), out_string);
        index = s->find(in, index + 1);
    }
}

interfaceItemType interface_item(const Rect& text_bounds) {
    interfaceItemType result;
    result.bounds = text_bounds;
    result.color = GOLD;
    result.kind = kLabeledRect;
    result.style = kLarge;
    result.item.labeledRect.label.stringID = 2001;
    result.item.labeledRect.label.stringNumber = 29;
    return result;
}

Rect pix_bounds(const Rect& text_bounds) {
    Rect r;
    GetAnyInterfaceItemGraphicBounds(interface_item(text_bounds), &r);
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
        int your_length, int par_length, int your_loss, int par_loss, int your_kill,
        int par_kill) {
    int score = 0;
    score += score_low_target(your_length, par_length, 50);
    score += score_low_target(your_loss, par_loss, 30);
    score += score_high_target(your_kill, par_kill, 20);
    return score;
}

RetroText* score_text(
        int your_length, int par_length, int your_loss, int par_loss, int your_kill,
        int par_kill) {
    Resource rsrc("text", "txt", 6000);
    String text(macroman::decode(rsrc.data()));

    StringList strings(6000);

    const int your_mins = your_length / 60;
    const int your_secs = your_length % 60;
    const int par_mins = par_length / 60;
    const int par_secs = par_length % 60;
    const int your_score = score(your_length, par_length, your_loss, par_loss, your_kill, par_kill);
    const int par_score = 100;

    string_replace(&text, strings.at(0), your_mins);
    string_replace(&text, strings.at(1), dec(your_secs, 2));
    if (par_length > 0) {
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

    RgbColor fore_color;
    fore_color = GetRGBTranslateColorShade(GOLD, VERY_LIGHT);
    RgbColor back_color;
    back_color = GetRGBTranslateColorShade(GOLD, DARKEST);
    return new RetroText(text, button_font, fore_color, back_color);
}

}  // namespace

DebriefingScreen::DebriefingScreen(int text_id)
        : _state(DONE),
          _next_update(0),
          _typed_chars(0) {
    initialize(text_id, false);
    _sprite.reset(VideoDriver::driver()->new_sprite("/x/debriefing_screen", *_pix));
}

DebriefingScreen::DebriefingScreen(
        int text_id, int your_length, int par_length, int your_loss, int par_loss,
        int your_kill, int par_kill)
        : _state(TYPING),
          _next_update(0),
          _typed_chars(0) {
    initialize(text_id, true);

    Rect score_area = _message_bounds;
    score_area.top = score_area.bottom - kScoreTableHeight;

    _score.reset(score_text(your_length, par_length, your_loss, par_loss, your_kill, par_kill)),
    _score->set_tab_width(60);
    _score->wrap_to(_message_bounds.width(), 2);
    _score_bounds = Rect(0, 0, _score->auto_width(), _score->height());
    _score_bounds.center_in(score_area);

    RgbColor bracket_color;
    bracket_color = GetRGBTranslateColorShade(GOLD, VERY_LIGHT);
    Rect bracket_bounds = _score_bounds;
    bracket_bounds.inset(-2, -2);
    DrawNateVBracket(_pix.get(), bracket_bounds, _pix->size().as_rect(), bracket_color);
    _sprite.reset(VideoDriver::driver()->new_sprite("/x/debriefing_screen", *_pix));

    _score_bounds.offset(_pix_bounds.left, _pix_bounds.top);
}

void DebriefingScreen::become_front() {
    _typed_chars = 0;
    if (_state == TYPING) {
        _next_update = now_usecs() + kTypingDelay;
    } else {
        _next_update = 0;
    }
}

void DebriefingScreen::resign_front() {
}

void DebriefingScreen::draw() const {
    next()->draw();
    _sprite->draw(_pix_bounds.left, _pix_bounds.top);
    for (int i = 0; i < _typed_chars; ++i) {
        _score->draw_char(_score_bounds, i);
    }
    Rect interface_bounds = _message_bounds;
    interface_bounds.offset(_pix_bounds.left, _pix_bounds.top);
    draw_interface_item(interface_item(interface_bounds));

    vector<inlinePictType> inline_pict;
    draw_text_in_rect(interface_bounds, _message, kLarge, GOLD, inline_pict);
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

bool DebriefingScreen::next_timer(int64_t& time) {
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
    PlayVolumeSound(kTeletype, kMediumLowVolume, kShortPersistence, kLowPrioritySound);
    int64_t now = now_usecs();
    while (_next_update <= now) {
        if (_typed_chars < _score->size()) {
            _next_update += kTypingDelay;
            ++_typed_chars;
        } else {
            _next_update = 0;
            _state = DONE;
            break;
        }
    }
}

void DebriefingScreen::initialize(int text_id, bool do_score) {
    Resource rsrc("text", "txt", text_id);
    _message.assign(macroman::decode(rsrc.data()));

    int text_height = GetInterfaceTextHeightFromWidth(_message, kLarge, kTextWidth);
    Rect text_bounds(0, 0, kTextWidth, text_height);
    if (do_score) {
        text_bounds.bottom += kScoreTableHeight;
    }
    text_bounds.center_in(viewport);

    _pix_bounds = pix_bounds(text_bounds);
    _message_bounds = text_bounds;
    _message_bounds.offset(-_pix_bounds.left, -_pix_bounds.top);
    _pix.reset(new ArrayPixMap(_pix_bounds.width(), _pix_bounds.height()));
}

void print_to(sfz::PrintTarget out, DebriefingScreen::State state) {
    switch (state) {
      case DebriefingScreen::TYPING:
        print(out, "TYPING");
        return;
      case DebriefingScreen::DONE:
        print(out, "DONE");
        return;
    }
    print(out, static_cast<int64_t>(state));
}

}  // namespace antares
