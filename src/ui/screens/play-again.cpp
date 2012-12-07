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

#include "ui/screens/play-again.hpp"

#include "drawing/interface.hpp"
#include "drawing/pix-map.hpp"
#include "game/globals.hpp"
#include "ui/card.hpp"
#include "video/transitions.hpp"

using sfz::Exception;
using sfz::format;

namespace antares {

namespace {

int interface_id(bool allow_resume, bool allow_skip) {
    if (allow_resume) {
        if (allow_skip) {
            return 5017;
        } else {
            return 5009;
        }
    } else {
        if (allow_skip) {
            throw Exception("allow_skip specified without allow_resume");
        } else {
            return 5008;
        }
    }
}

}  // namespace

PlayAgainScreen::PlayAgainScreen(bool allow_resume, bool allow_skip, Item* button_pressed)
        : InterfaceScreen(interface_id(allow_resume, allow_skip), play_screen, false),
          _state(ASKING),
          _button_pressed(button_pressed) { }

PlayAgainScreen::~PlayAgainScreen() { }

void PlayAgainScreen::become_front() {
    switch (_state) {
      case ASKING:
        InterfaceScreen::become_front();
        break;

      case FADING_OUT:
        stack()->pop(this);
        break;
    }
}

void PlayAgainScreen::adjust_interface() {
    // TODO(sfiera): disable if networked.
    mutable_item(RESTART)->set_status(kActive);
}

void PlayAgainScreen::handle_button(int button) {
    switch (button) {
      case RESTART:
        _state = FADING_OUT;
        *_button_pressed = static_cast<Item>(button);
        stack()->push(new ColorFade(ColorFade::TO_COLOR, RgbColor::kBlack, 1e6, false, NULL));
        break;

      case QUIT:
      case RESUME:
      case SKIP:
        *_button_pressed = static_cast<Item>(button);
        stack()->pop(this);
        break;

      default:
        throw Exception(format("Got unknown button {0}.", button));
    }
}

void print_to(sfz::PrintTarget out, PlayAgainScreen::Item item) {
    switch (item) {
      case PlayAgainScreen::RESTART:
        print(out, "RESTART");
        return;
      case PlayAgainScreen::QUIT:
        print(out, "QUIT");
        return;
      case PlayAgainScreen::RESUME:
        print(out, "RESUME");
        return;
      case PlayAgainScreen::SKIP:
        print(out, "SKIP");
        return;
      case PlayAgainScreen::BOX:
        print(out, "BOX");
        return;
    }
    print(out, static_cast<int64_t>(item));
}

}  // namespace antares
