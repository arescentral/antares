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

#ifndef ANTARES_CONFIG_GAMEPAD_HPP_
#define ANTARES_CONFIG_GAMEPAD_HPP_

#include <stdint.h>
#include <pn/string>

namespace antares {

struct Gamepad {
    enum class Button {
        NONE = 0x00,

        A      = 0x01,
        B      = 0x02,
        X      = 0x03,
        Y      = 0x04,
        LB     = 0x05,
        RB     = 0x06,
        LSB    = 0x07,
        RSB    = 0x08,
        START  = 0x09,
        BACK   = 0x0a,
        VENDOR = 0x0b,
        UP     = 0x0c,
        DOWN   = 0x0d,
        LEFT   = 0x0e,
        RIGHT  = 0x0f,

        LT = 0x32,
        RT = 0x35,
    };

    enum class Stick {
        LS = 0x30,
        RS = 0x33,
    };

    enum {
        BEGIN      = static_cast<int>(Button::A),
        END        = static_cast<int>(Button::RT) + 1,
        NAMES      = 1001,
        LONG_NAMES = 1003,
    };

    static Gamepad::Button num(pn::string_view name);
    static bool            name(Gamepad::Button button, pn::string& out);
};

}  // namespace antares

#endif  // ANTARES_CONFIG_GAMEPAD_HPP_
