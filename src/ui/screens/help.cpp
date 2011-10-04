// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2011 Ares Central
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
// License along with this program.  If not, see
// <http://www.gnu.org/licenses/>.

#include "ui/screens/help.hpp"

#include "data/resource.hpp"
#include "drawing/color.hpp"
#include "drawing/interface.hpp"
#include "drawing/pix-map.hpp"
#include "drawing/retro-text.hpp"
#include "drawing/text.hpp"
#include "game/globals.hpp"
#include "ui/card.hpp"
#include "ui/interface-handling.hpp"
#include "video/driver.hpp"

using sfz::Bytes;
using sfz::Exception;
using sfz::String;
using sfz::format;

namespace macroman = sfz::macroman;

namespace antares {

HelpScreen::HelpScreen()
        : InterfaceScreen(5012, play_screen, false) {
    // TODO(sfiera): top and bottom buffer of 1, not just top buffer of 2.
    offset((world.width() / 2) - (viewport.width() / 2), 2);

    _bounds = item(BOX).bounds;
    _bounds.offset(viewport.left, 0);

    Resource rsrc("text", "txt", 6002);
    String text(macroman::decode(rsrc.data()));
    Replace_KeyCode_Strings_With_Actual_Key_Names(&text, 1000, 4);

    RgbColor fore = GetRGBTranslateColorShade(RED, VERY_LIGHT);
    RgbColor back = GetRGBTranslateColorShade(RED, VERY_DARK);
    RetroText retro_text(text, kComputerFontNum, fore, back);
    retro_text.wrap_to(_bounds.width(), 0);

    ArrayPixMap pix(_bounds.width(), _bounds.height());
    pix.fill(RgbColor::kClear);
    retro_text.draw(&pix, pix.size().as_rect());
    _sprite.reset(VideoDriver::driver()->new_sprite("/x/help_screen", pix));
}

HelpScreen::~HelpScreen() { }

void HelpScreen::handle_button(int button) {
    switch (button) {
      case DONE:
        stack()->pop(this);
        break;

      default:
        throw Exception(format("Got unknown button {0}.", button));
    }
}

void HelpScreen::draw() const {
    InterfaceScreen::draw();
    _sprite->draw(_bounds.left, _bounds.top);
}

}  // namespace antares
