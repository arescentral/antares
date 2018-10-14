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

#include "config/gamepad.hpp"

#include <sfz/sfz.hpp>

#include "game/sys.hpp"

using sfz::range;

namespace antares {

Gamepad::Button Gamepad::num(pn::string_view name) {
    for (auto i : range<int>(BEGIN, END)) {
        if (sys.gamepad_names.at(i) == name) {
            return static_cast<Gamepad::Button>(i);
        }
    }
    return Gamepad::Button::NONE;
}

bool Gamepad::name(Gamepad::Button button, pn::string& out) {
    int index = static_cast<int>(button);
    if ((0 <= index) && (index < sys.gamepad_names.size())) {
        out = sys.gamepad_names.at(index).copy();
        return true;
    }
    return false;
}

}  // namespace antares
