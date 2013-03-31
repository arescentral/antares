// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2012 The Antares Authors
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

#include <sfz/sfz.hpp>

#include "data/string-list.hpp"
#include "game/admiral.hpp"
#include "game/globals.hpp"
#include "game/messages.hpp"
#include "game/player-ship.hpp"
#include "math/fixed.hpp"

using sfz::BytesSlice;
using sfz::PrintItem;
using sfz::Rune;
using sfz::String;
using sfz::StringSlice;

namespace antares {

const int16_t kCheatStringListID    = 750;
const int16_t kCheatFeedbackOnID    = 751;
const int16_t kCheatFeedbackOffID   = 752;

const int32_t kCheatCodeValue       = 5;

const int16_t kActivateCheatCheat   = 1;
const int16_t kAutoPlayCheat        = 2;
const int16_t kPayMoneyCheat        = 3;
const int16_t kNameObjectCheat      = 4;
const int16_t kObserverCheat        = 5;  // makes your ship appear to not be engageable
const int16_t kBuildFastCheat       = 6;
const int16_t kRaisePayRateCheat    = 7;  // determines your payscale
const int16_t kLowerPayRateCheat    = 8;

void CheatFeedback(short whichCheat, bool activate, long whichPlayer);
void CheatFeedbackPlus(short whichCheat, bool activate, long whichPlayer, PrintItem extra);

void AresCheatInit() {
    globals()->gAresCheatStrings.reset(new StringList(kCheatStringListID));
}

void CleanupAresCheat() {
    globals()->gAresCheatStrings.reset();
}

short GetCheatNumFromString(const StringSlice& s) {
    String code_string;
    for (Rune r: s) {
        code_string.append(1, r + kCheatCodeValue);
    }
    return globals()->gAresCheatStrings.get()->index_of(code_string) + 1;
}

void ExecuteCheat( short whichCheat, long whichPlayer)
{
    long                    i;
    spaceObjectType *anObject = NULL;

    if ( whichCheat == kNameObjectCheat)
    {
        globals()->gActiveCheats[whichPlayer] |= kNameObjectBit;
        CheatFeedback( whichCheat, true, whichPlayer);
        return;
    }

#ifdef NETSPROCKET_AVAILABLE
    if ( GetAllNetPlayersCheating())
#else
    if ( true)
#endif  // NETSPROCKET_AVAILABLE
    {
        switch( whichCheat)
        {
            case kActivateCheatCheat:
                for ( i = 0; i < kMaxPlayerNum; i++)
                {
                    globals()->gActiveCheats[i] = 0;
                }
                CheatFeedback( whichCheat, false, whichPlayer);
                break;

            case kPayMoneyCheat:
                PayAdmiralAbsolute( whichPlayer, mLongToFixed( 5000));
                PayAdmiralAbsolute( whichPlayer, mLongToFixed( 5000));
                PayAdmiralAbsolute( whichPlayer, mLongToFixed( 5000));
                CheatFeedback( whichCheat, true, whichPlayer);
                break;

            case kAutoPlayCheat:
                if ( globals()->gActiveCheats[whichPlayer] & kAutoPlayBit)
                {
                    globals()->gActiveCheats[whichPlayer] &= ~kAutoPlayBit;
                    CheatFeedback( whichCheat, false, whichPlayer);
                    if ( whichPlayer == globals()->gPlayerAdmiralNumber)
                    {
//                      ChangePlayerShipNumber( whichPlayer, globals()->gPlayerShipNumber);
                    }
//                  SetAdmiralAttributes( whichPlayer, kAIsHuman);
                } else
                {
                    globals()->gActiveCheats[whichPlayer] |= kAutoPlayBit;
                    CheatFeedback( whichCheat, true, whichPlayer);
                    if ( whichPlayer == globals()->gPlayerAdmiralNumber)
                    {
//                      ChangePlayerShipNumber( whichPlayer, globals()->gPlayerShipNumber);
                    }
//                  SetAdmiralAttributes( whichPlayer, kAIsComputer);
                }
                break;

            case kBuildFastCheat:
                if ( globals()->gActiveCheats[whichPlayer] & kBuildFastBit)
                {
                    globals()->gActiveCheats[whichPlayer] &= ~kBuildFastBit;
                    CheatFeedback( whichCheat, false, whichPlayer);
                } else
                {
                    globals()->gActiveCheats[whichPlayer] |= kBuildFastBit;
                    CheatFeedback( whichCheat, true, whichPlayer);
                }
                break;

            case kObserverCheat:
                anObject = GetAdmiralFlagship( whichPlayer);
                if ( anObject != NULL)
                {
                    anObject->attributes &= ~(kCanBeEngaged | kHated);
                    CheatFeedback( whichCheat, true, whichPlayer);
                }
                break;

            case kRaisePayRateCheat:
                SetAdmiralEarningPower(
                        whichPlayer, GetAdmiralEarningPower(whichPlayer) + 0x00000020);
                CheatFeedbackPlus(
                        whichCheat, true, whichPlayer, fixed(GetAdmiralEarningPower(whichPlayer)));
                break;

            case kLowerPayRateCheat:
                SetAdmiralEarningPower(
                        whichPlayer, GetAdmiralEarningPower(whichPlayer) - 0x00000020);
                CheatFeedbackPlus(
                        whichCheat, true, whichPlayer, fixed(GetAdmiralEarningPower(whichPlayer)));
                break;
        }
    } else
    {
        if ( whichCheat == kActivateCheatCheat)
        {
            if ( globals()->gActiveCheats[whichPlayer] & kCheatActiveBit)
            {
                for ( i = 0; i < kMaxPlayerNum; i++)
                {
                    globals()->gActiveCheats[i] = 0;
                }

                CheatFeedback( whichCheat, false, whichPlayer);
            } else
            {
                globals()->gActiveCheats[whichPlayer] |= kCheatActiveBit;
                CheatFeedback( whichCheat, true, whichPlayer);
            }
        }
    }
}

void CheatFeedback(short whichCheat, bool activate, long whichPlayer) {
    String admiral_name(GetAdmiralName(whichPlayer));
    String feedback;
    if (activate) {
        feedback.assign(StringList(kCheatFeedbackOnID).at(whichCheat - 1));
    } else {
        feedback.assign(StringList(kCheatFeedbackOffID).at(whichCheat - 1));
    }
    Messages::add(format("{0}{1}", admiral_name, feedback));
}

void CheatFeedbackPlus(
        short whichCheat, bool activate, long whichPlayer, PrintItem extra) {
    String admiral_name(GetAdmiralName(whichPlayer));
    String feedback;
    if (activate) {
        feedback.assign(StringList(kCheatFeedbackOnID).at(whichCheat - 1));
    } else {
        feedback.assign(StringList(kCheatFeedbackOffID).at(whichCheat - 1));
    }
    Messages::add(format("{0}{1}{2}", admiral_name, feedback, extra));
}

}  // namespace antares
