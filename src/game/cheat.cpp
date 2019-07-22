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

#include "game/cheat.hpp"

#include <algorithm>
#include <pn/output>

#include "game/admiral.hpp"
#include "game/globals.hpp"
#include "game/messages.hpp"
#include "game/player-ship.hpp"
#include "game/space-object.hpp"
#include "game/sys.hpp"
#include "lang/defines.hpp"
#include "math/fixed.hpp"

using std::unique_ptr;

namespace antares {

const int32_t kCheatCodeValue = 5;

void CheatFeedback(Cheat c, bool activate, Handle<Admiral> whichPlayer);
void CheatFeedbackPlus(Cheat c, bool activate, Handle<Admiral> whichPlayer, pn::string_view extra);

Cheat GetCheatFromString(pn::string_view s) {
    pn::string code_string;
    for (pn::rune r : s) {
        code_string += pn::rune{r.value() + kCheatCodeValue};
    }
    auto it = std::find(sys.cheat.codes.begin(), sys.cheat.codes.end(), code_string);
    if (it == sys.cheat.codes.end()) {
        return Cheat::NONE;
    }
    return static_cast<Cheat>(it - sys.cheat.codes.begin() + 1);
}

void ExecuteCheat(Cheat c, Handle<Admiral> a) {
    switch (c) {
        case Cheat::NONE: break;

        case Cheat::NAME_OBJECT:
            a->cheats() |= kNameObjectBit;
            CheatFeedback(c, true, a);
            break;

        case Cheat::ACTIVATE_CHEAT:
            for (auto adm : Admiral::all()) {
                adm->cheats() = 0;
            }
            CheatFeedback(c, false, a);
            break;

        case Cheat::PAY_MONEY:
            a->pay_absolute(Cash{Fixed::from_long(15000)});
            CheatFeedback(c, true, a);
            break;

        case Cheat::AUTO_PLAY:
            a->cheats() ^= kAutoPlayBit;
            CheatFeedback(c, a->cheats() & kAutoPlayBit, a);
            break;

        case Cheat::BUILD_FAST:
            a->cheats() ^= kBuildFastBit;
            CheatFeedback(c, a->cheats() & kBuildFastBit, a);
            break;

        case Cheat::OBSERVER:
            if (a->flagship().get()) {
                a->flagship()->attributes &= ~(kCanBeEngaged | kHated);
                CheatFeedback(c, true, a);
            }
            break;

        case Cheat::RAISE_PAY_RATE:
            a->set_earning_power(a->earning_power() + Fixed::from_float(0.125));
            CheatFeedbackPlus(c, true, a, stringify(Fixed(a->earning_power())));
            break;

        case Cheat::LOWER_PAY_RATE:
            a->set_earning_power(a->earning_power() - Fixed::from_float(0.125));
            CheatFeedbackPlus(c, true, a, stringify(Fixed(a->earning_power())));
            break;
    }
}

void CheatFeedback(Cheat c, bool activate, Handle<Admiral> whichPlayer) {
    pn::string_view admiral_name = GetAdmiralName(whichPlayer);
    pn::string_view feedback;
    if (activate) {
        feedback = sys.cheat.on.at(static_cast<int>(c) - 1);
    } else {
        feedback = sys.cheat.off.at(static_cast<int>(c) - 1);
    }
    Messages::add(pn::format("{0}{1}", admiral_name, feedback));
}

void CheatFeedbackPlus(
        Cheat c, bool activate, Handle<Admiral> whichPlayer, pn::string_view extra) {
    pn::string_view admiral_name = GetAdmiralName(whichPlayer);
    pn::string_view feedback;
    if (activate) {
        feedback = sys.cheat.on.at(static_cast<int>(c) - 1);
    } else {
        feedback = sys.cheat.off.at(static_cast<int>(c) - 1);
    }
    Messages::add(pn::format("{0}{1}{2}", admiral_name, feedback, extra));
}

}  // namespace antares
