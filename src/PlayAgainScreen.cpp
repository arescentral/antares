// Ares, a tactical space combat game.
// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#include "PlayAgainScreen.hpp"

#include "AresGlobalType.hpp"
#include "CardStack.hpp"
#include "Error.hpp"
#include "OffscreenGWorld.hpp"
#include "Options.hpp"
#include "PixMap.hpp"
#include "PlayerInterfaceDrawing.hpp"

namespace antares {

extern PixMap* gRealWorld;
extern PixMap* gSaveWorld;

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
            fail("allow_skip specified without allow_resume");
        } else {
            return 5008;
        }
    }
}

}  // namespace

PlayAgainScreen::PlayAgainScreen(bool allow_resume, bool allow_skip, Item* button_pressed)
        : InterfaceScreen(interface_id(allow_resume, allow_skip)),
          _button_pressed(button_pressed) { }

PlayAgainScreen::~PlayAgainScreen() { }

void PlayAgainScreen::become_front() {
    gSaveWorld->copy(*gRealWorld);
    InterfaceScreen::become_front();
}

void PlayAgainScreen::resign_front() {
    InterfaceScreen::resign_front();
    gRealWorld->copy(*gSaveWorld);
}

void PlayAgainScreen::adjust_interface() {
    if (globals()->gOptions & kOptionNetworkOn) {
        mutable_item(RESTART)->set_status(kDimmed);
    }
}

void PlayAgainScreen::draw() const {
    Rect bounds;
    GetAnyInterfaceItemGraphicBounds(item(BOX), &bounds);
    gRealWorld->view(bounds).fill(RgbColor::kBlack);
    InterfaceScreen::draw();
}

void PlayAgainScreen::handle_button(int button) {
    switch (button) {
      case RESTART:
      case QUIT:
      case RESUME:
      case SKIP:
        *_button_pressed = static_cast<Item>(button);
        stack()->pop(this);
        break;

      default:
        fail("Got unknown button %d.", button);
    }
}

}  // namespace antares
