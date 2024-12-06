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

#include "config/keys.hpp"

#include <string.h>

#include "config/preferences.hpp"
#include "game/sys.hpp"

namespace antares {

Key Key::named(pn::string_view name) {
    for (int i = 0; i < sys.key_names.size(); ++i) {
        if (sys.key_names.at(i) == name) {
            return Key(i);
        }
    }
    return Key::NONE;
}

int Key::digit() const {
    switch (_value) {
        case Key::K0:
        case Key::N0: return 0;
        case Key::K1:
        case Key::N1: return 1;
        case Key::K2:
        case Key::N2: return 2;
        case Key::K3:
        case Key::N3: return 3;
        case Key::K4:
        case Key::N4: return 4;
        case Key::K5:
        case Key::N5: return 5;
        case Key::K6:
        case Key::N6: return 6;
        case Key::K7:
        case Key::N7: return 7;
        case Key::K8:
        case Key::N8: return 8;
        case Key::K9:
        case Key::N9: return 9;
        default: return -1;
    }
}

pn::string_view Key::name() const { return sys.key_names.at(_value); }

pn::string_view Key::long_name() const { return sys.key_long_names.at(_value); }

}  // namespace antares
