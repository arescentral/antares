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

#include "ui/screens/play-again.hpp"

#include <pn/file>

#include "drawing/interface.hpp"
#include "drawing/pix-map.hpp"
#include "game/globals.hpp"
#include "ui/card.hpp"
#include "video/transitions.hpp"

namespace antares {

namespace {

const char* interface_id(bool allow_resume, bool allow_skip) {
    if (allow_resume) {
        if (allow_skip) {
            return "play-again/tutorial";
        } else {
            return "play-again/normal";
        }
    } else {
        if (allow_skip) {
            throw std::runtime_error("allow_skip specified without allow_resume");
        } else {
            return "play-again/lose";
        }
    }
}

}  // namespace

PlayAgainScreen::PlayAgainScreen(bool allow_resume, bool allow_skip, Item* button_pressed)
        : InterfaceScreen(interface_id(allow_resume, allow_skip), {48, 0, 688, 480}, false),
          _state(ASKING),
          _button_pressed(button_pressed) {}

PlayAgainScreen::~PlayAgainScreen() {}

void PlayAgainScreen::become_front() {
    switch (_state) {
        case ASKING: InterfaceScreen::become_front(); break;

        case FADING_OUT: stack()->pop(this); break;
    }
}

void PlayAgainScreen::adjust_interface() {
    // TODO(sfiera): disable if networked.
    dynamic_cast<ButtonData&>(mutable_item(RESTART)).status = kActive;
}

void PlayAgainScreen::handle_button(ButtonData& button) {
    switch (button.id) {
        case RESTART:
            _state           = FADING_OUT;
            *_button_pressed = static_cast<Item>(button.id);
            stack()->push(
                    new ColorFade(ColorFade::TO_COLOR, RgbColor::black(), secs(1), false, NULL));
            break;

        case QUIT:
        case RESUME:
        case SKIP:
            *_button_pressed = static_cast<Item>(button.id);
            stack()->pop(this);
            break;

        default:
            throw std::runtime_error(pn::format("Got unknown button {0}.", button.id).c_str());
    }
}

const char* stringify(PlayAgainScreen::Item item) {
    switch (item) {
        case PlayAgainScreen::RESTART: return "RESTART";
        case PlayAgainScreen::QUIT: return "QUIT";
        case PlayAgainScreen::RESUME: return "RESUME";
        case PlayAgainScreen::SKIP: return "SKIP";
        case PlayAgainScreen::BOX: return "BOX";
    }
    return "?";
}

}  // namespace antares
