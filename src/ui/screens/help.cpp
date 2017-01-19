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

#include "ui/screens/help.hpp"

#include "data/resource.hpp"
#include "drawing/color.hpp"
#include "drawing/interface.hpp"
#include "drawing/pix-map.hpp"
#include "drawing/styled-text.hpp"
#include "drawing/text.hpp"
#include "game/globals.hpp"
#include "game/sys.hpp"
#include "ui/card.hpp"
#include "ui/interface-handling.hpp"
#include "video/driver.hpp"

using sfz::Bytes;
using sfz::Exception;
using sfz::String;
using sfz::format;

namespace utf8 = sfz::utf8;

namespace antares {

HelpScreen::HelpScreen()
        : InterfaceScreen("help", {128, 0, 608, 480}, false), _text(sys.fonts.computer) {
    Resource rsrc("text", "txt", 6002);
    String   text(utf8::decode(rsrc.data()));
    Replace_KeyCode_Strings_With_Actual_Key_Names(&text, 1000, 4);

    RgbColor fore = GetRGBTranslateColorShade(RED, VERY_LIGHT);
    RgbColor back = GetRGBTranslateColorShade(RED, VERY_DARK);
    _text.set_fore_color(fore);
    _text.set_back_color(back);
    _text.set_retro_text(text);
    _text.wrap_to(item(BOX).bounds().width(), 0, 0);
}

HelpScreen::~HelpScreen() {}

void HelpScreen::key_down(const KeyDownEvent& event) {
    if (event.key() == sys.prefs->key(kHelpKeyNum) - 1) {
        stack()->pop(this);
    } else {
        InterfaceScreen::key_down(event);
    }
}

void HelpScreen::handle_button(Button& button) {
    switch (button.id) {
        case DONE: stack()->pop(this); break;

        default: throw Exception(format("Got unknown button {0}.", button.id));
    }
}

void HelpScreen::overlay() const {
    Rect  bounds = item(BOX).bounds();
    Point off    = offset();
    bounds.offset(off.h, off.v);
    _text.draw(bounds);
}

}  // namespace antares
