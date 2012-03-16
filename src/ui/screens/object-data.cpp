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

#include "ui/screens/object-data.hpp"

#include "drawing/retro-text.hpp"
#include "drawing/text.hpp"
#include "game/globals.hpp"
#include "ui/interface-handling.hpp"
#include "video/driver.hpp"

using sfz::Bytes;
using sfz::Exception;
using sfz::String;
using sfz::format;

namespace macroman = sfz::macroman;

namespace antares {

namespace {

const int32_t kShipDataWidth = 240;

Rect object_data_bounds(Point origin, Size size) {
    Rect bounds(Point(0, 0), size);
    bounds.center_in(Rect(origin, Size(0, 0)));
    Rect inside = world;
    inside.inset(9, 5);
    if (bounds.left < inside.left) {
        bounds.offset(inside.left - bounds.left, 0);
    }
    if (bounds.right > inside.right) {
        bounds.offset(inside.right - bounds.right, 0);
    }
    if (bounds.top < inside.top) {
        bounds.offset(0, inside.top - bounds.top);
    }
    if (bounds.bottom > inside.bottom) {
        bounds.offset(0, inside.bottom - bounds.bottom);
    }
    return bounds;
}

}  // namespace

ObjectDataScreen::ObjectDataScreen(Point origin, int32_t object_id, Trigger trigger, int which):
        _trigger(trigger),
        _which(which) {
    String text;
    CreateObjectDataText(&text, object_id);
    _text.reset(new RetroText(
                text, kButtonFontNum,
                GetRGBTranslateColorShade(GREEN, VERY_LIGHT),
                GetRGBTranslateColorShade(GREEN, DARKEST)));
    _text->wrap_to(kShipDataWidth, 0);
    _bounds = object_data_bounds(origin, Size(_text->auto_width(), _text->height()));
}

ObjectDataScreen::~ObjectDataScreen() { }

void ObjectDataScreen::mouse_up(const MouseUpEvent& event) {
    if ((_trigger == MOUSE) && (event.button() == _which)) {
        stack()->pop(this);
    }
}

void ObjectDataScreen::key_up(const KeyUpEvent& event) {
    if ((_trigger == KEY) && (event.key() == _which)) {
        stack()->pop(this);
    }
}

void ObjectDataScreen::draw() const {
    next()->draw();
    Rect outside = _bounds;
    outside.inset(-8, -4);
    const RgbColor light_green = GetRGBTranslateColorShade(GREEN, VERY_LIGHT);
    VideoDriver::driver()->fill_rect(outside, light_green);
    outside.inset(1, 1);
    VideoDriver::driver()->fill_rect(outside, RgbColor::kBlack);
    _text->draw(_bounds);
}

}  // namespace antares
