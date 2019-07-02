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

#include "drawing/interface.hpp"

#include "config/gamepad.hpp"
#include "config/keys.hpp"
#include "data/interface.hpp"
#include "drawing/color.hpp"
#include "drawing/styled-text.hpp"
#include "drawing/text.hpp"
#include "game/sys.hpp"
#include "video/driver.hpp"

using std::unique_ptr;
using std::vector;

namespace antares {

const Font& interface_font(InterfaceStyle style) {
    if (style == InterfaceStyle::SMALL) {
        return sys.fonts.small_button;
    } else {
        return sys.fonts.button;
    }
}

void draw_text_in_rect(Rect tRect, pn::string_view text, InterfaceStyle style, Hue hue) {
    const StyledText interface_text =
            StyledText::interface(text, GetRGBTranslateColorShade(hue, LIGHTEST))
                    .wrap_to(
                            interface_font(style), tRect.width(), kInterfaceTextHBuffer,
                            kInterfaceTextVBuffer);
    tRect.offset(0, -kInterfaceTextVBuffer);
    interface_text.draw(tRect);
}

int16_t GetInterfaceTextHeightFromWidth(
        pn::string_view text, InterfaceStyle style, int16_t boundsWidth) {
    const StyledText interface_text = StyledText::interface(text).wrap_to(
            interface_font(style), boundsWidth, kInterfaceTextHBuffer, kInterfaceTextVBuffer);
    return interface_text.height();
}

}  // namespace antares
