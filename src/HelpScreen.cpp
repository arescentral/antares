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

#include "HelpScreen.hpp"

#include "AresGlobalType.hpp"
#include "CardStack.hpp"
#include "ColorTranslation.hpp"
#include "DirectText.hpp"
#include "Error.hpp"
#include "OffscreenGWorld.hpp"
#include "Options.hpp"
#include "PixMap.hpp"
#include "PlayerInterface.hpp"
#include "PlayerInterfaceDrawing.hpp"
#include "RetroText.hpp"
#include "Resource.hpp"

using sfz::Bytes;
using sfz::String;

namespace macroman = sfz::macroman;

namespace antares {

extern long CLIP_LEFT, CLIP_TOP, CLIP_RIGHT;
extern PixMap* gRealWorld;
extern PixMap* gSaveWorld;

namespace {

int interface_id() {
    return 5012;
}

int text_id() {
    return 6002;
}

}  // namespace

HelpScreen::HelpScreen()
        : InterfaceScreen(
                interface_id(),
                Rect(CLIP_LEFT, CLIP_TOP, CLIP_RIGHT, globals()->gTrueClipBottom),
                false) {
    // TODO(sfiera): top and bottom buffer of 1, not just top buffer of 2.
    offset((gRealWorld->bounds().width() / 2) - ((CLIP_RIGHT - CLIP_LEFT) / 2), 2);

    Resource rsrc("text", "txt", text_id());
    String text(macroman::decode(rsrc.data()));
    RgbColor fore;
    RgbColor back;
    GetRGBTranslateColorShade(&fore, RED, VERY_LIGHT);
    GetRGBTranslateColorShade(&back, RED, VERY_DARK);
    Replace_KeyCode_Strings_With_Actual_Key_Names(&text, 1000, 4);

    _text.reset(new RetroText(text, kComputerFontNum, fore, back));
    _text->wrap_to(item(BOX).bounds.width(), 0);

    _text_bounds = item(BOX).bounds;
    _text_bounds.offset(CLIP_LEFT, 0);
}

HelpScreen::~HelpScreen() { }

void HelpScreen::become_front() {
    gSaveWorld->copy(*gRealWorld);
    InterfaceScreen::become_front();
}

void HelpScreen::resign_front() {
    InterfaceScreen::resign_front();
    gRealWorld->copy(*gSaveWorld);
}

void HelpScreen::handle_button(int button) {
    switch (button) {
      case DONE:
        stack()->pop(this);
        break;

      default:
        fail("Got unknown button %d.", button);
    }
}

void HelpScreen::draw() const {
    InterfaceScreen::draw();
    _text->draw(gRealWorld, _text_bounds);
}

}  // namespace antares
