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

#ifndef ANTARES_GAME_CHEAT_HPP_
#define ANTARES_GAME_CHEAT_HPP_

#include "game/globals.hpp"

namespace antares {

enum class Cheat {
    NONE           = 0,
    ACTIVATE_CHEAT = 1,
    AUTO_PLAY      = 2,
    PAY_MONEY      = 3,
    NAME_OBJECT    = 4,
    OBSERVER       = 5,  // makes your ship appear to not be engageable
    BUILD_FAST     = 6,
    RAISE_PAY_RATE = 7,  // determines your payscale
    LOWER_PAY_RATE = 8,
};

enum {
    kCheatActiveBit  = 1 << (static_cast<int>(Cheat::ACTIVATE_CHEAT) - 1),
    kAutoPlayBit     = 1 << (static_cast<int>(Cheat::AUTO_PLAY) - 1),
    kNameObjectBit   = 1 << (static_cast<int>(Cheat::NAME_OBJECT) - 1),
    kObserverBit     = 1 << (static_cast<int>(Cheat::OBSERVER) - 1),
    kBuildFastBit    = 1 << (static_cast<int>(Cheat::BUILD_FAST) - 1),
    kRaisePayRateBit = 1 << (static_cast<int>(Cheat::RAISE_PAY_RATE) - 1),
    kLowerPayRateBit = 1 << (static_cast<int>(Cheat::LOWER_PAY_RATE) - 1),
};

Cheat GetCheatFromString(pn::string_view string);
void  ExecuteCheat(Cheat c, Handle<Admiral> a);

}  // namespace antares

#endif  // ANTARES_GAME_CHEAT_HPP_
