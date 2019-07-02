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

#include <pn/output>

#include "config/preferences.hpp"
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

namespace antares {

static pn::string help_text() {
    pn::string text = Resource::text(6002);
    Replace_KeyCode_Strings_With_Actual_Key_Names(text, 1000, 4);
    return text;
}

HelpScreen::HelpScreen()
        : InterfaceScreen("help", {128, 0, 608, 480}),
          _text{StyledText::retro(
                  help_text(), {sys.fonts.computer, widget(BOX)->inner_bounds().width()},
                  GetRGBTranslateColorShade(Hue::RED, LIGHTEST),
                  GetRGBTranslateColorShade(Hue::RED, VERY_DARK))} {
    button(DONE)->bind({[this] { stack()->pop(this); }});
}

HelpScreen::~HelpScreen() {}

void HelpScreen::key_down(const KeyDownEvent& event) {
    if (event.key() == sys.prefs->key(kHelpKeyNum)) {
        stack()->pop(this);
    } else {
        InterfaceScreen::key_down(event);
    }
}

void HelpScreen::overlay() const {
    Rect  bounds = widget(BOX)->inner_bounds();
    Point off    = offset();
    bounds.offset(off.h, off.v);
    _text.draw(bounds);
}

}  // namespace antares
