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

#include "game/sys.hpp"

using sfz::String;
using sfz::StringSlice;
using sfz::range;

namespace antares {

int16_t Gamepad::num(StringSlice name) {
    for (auto i : range<int>(BEGIN, END)) {
        if (sys.gamepad_names.at(i) == name) {
            return i;
        }
    }
    return 0;
}

bool Gamepad::name(int16_t button, String& out) {
    if ((0 <= button) && (button < sys.gamepad_names.size())) {
        out.assign(sys.gamepad_names.at(button));
        return true;
    }
    return false;
}

}  // namespace antares
