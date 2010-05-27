// Ares, a tactical space combat game.
// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

// Ares Cheat

#include "AresCheat.hpp"

#include "sfz/sfz.hpp"
#include "Admiral.hpp"
#include "AresGlobalType.hpp"
#include "Debug.hpp"
#include "MessageScreen.hpp"
#include "PlayerShip.hpp"
#include "StringHandling.hpp"
#include "StringList.hpp"
#include "StringNumerics.hpp"

using sfz::BytesPiece;
using sfz::String;

namespace macroman = sfz::macroman;

namespace antares {

#define kCheatStringListID      750
#define kCheatFeedbackOnID      751
#define kCheatFeedbackOffID     752

#define kCheatCodeValue         5

#define kActivateCheatCheat     1
#define kAutoPlayCheat          2
#define kPayMoneyCheat          3
#define kNameObjectCheat        4
#define kObserverCheat          5   // makes your ship appear to not be engageable
#define kBuildFastCheat         6
#define kRaisePayRateCheat      7   // determines your payscale
#define kLowerPayRateCheat      8

void CheatFeedback( short, bool, long);
void CheatFeedbackPlus(short, bool, long, unsigned char*);

void AresCheatInit() {
    globals()->gAresCheatStrings.reset(new StringList(kCheatStringListID));
}

void CleanupAresCheat() {
    globals()->gAresCheatStrings.reset();
}

short GetCheatNumFromString(unsigned char* s)
{
//  return( -1);
    Str255  codeString;
    short       strLen;

    strLen = s[0];
    codeString[0] = strLen;
    while ( strLen > 0)
    {
        codeString[strLen] = s[strLen] + kCheatCodeValue;
        strLen--;
    }
    String string(macroman::decode(PStringBytes(codeString)));
    return globals()->gAresCheatStrings.get()->index_of(string) + 1;
}

void ExecuteCheat( short whichCheat, long whichPlayer)
{
    long                    i;
    spaceObjectType *anObject = nil;
    Str255              s;

    mWriteDebugString("\pWhich Cheat:");
    WriteDebugLong( whichCheat);
    mWriteDebugString("\pWhich Player:");
    WriteDebugLong( whichPlayer);
    WriteDebugHex( globals()->gActiveCheats[whichPlayer], 8);

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
#endif NETSPROCKET_AVAILABLE
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
                if ( anObject != nil)
                {
                    anObject->attributes &= ~(kCanBeEngaged | kHated);
                    CheatFeedback( whichCheat, true, whichPlayer);
                }
                break;

            case kRaisePayRateCheat:
                SetAdmiralEarningPower( whichPlayer, GetAdmiralEarningPower( whichPlayer) +
                    0x00000020);
                SmallFixedToString( GetAdmiralEarningPower( whichPlayer), s);
                CheatFeedbackPlus( whichCheat, true, whichPlayer, s);
                break;

            case kLowerPayRateCheat:
                SetAdmiralEarningPower( whichPlayer, GetAdmiralEarningPower( whichPlayer) -
                    0x00000020);
                SmallFixedToString( GetAdmiralEarningPower( whichPlayer), s);
                CheatFeedbackPlus( whichCheat, true, whichPlayer, s);
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

void CheatFeedback( short whichCheat, bool activate, long whichPlayer)
{
    Str255          s, feedback;
    unsigned char* name;

    if ( activate) GetIndString( feedback, kCheatFeedbackOnID, whichCheat);
    else GetIndString( feedback, kCheatFeedbackOffID, whichCheat);
    name = GetAdmiralName( whichPlayer);
    CopyPString( s, name);
    ConcatenatePString( s, feedback);
    StartMessage();
    AppendStringToMessage( s);
    EndMessage();
}

void CheatFeedbackPlus( short whichCheat, bool activate, long whichPlayer,
    unsigned char* infoString)
{
    Str255          s, feedback;
    unsigned char* name;

    if ( activate) GetIndString( feedback, kCheatFeedbackOnID, whichCheat);
    else GetIndString( feedback, kCheatFeedbackOffID, whichCheat);
    name = GetAdmiralName( whichPlayer);
    CopyPString( s, name);
    ConcatenatePString( s, feedback);
    StartMessage();
    AppendStringToMessage( s);
    AppendStringToMessage( infoString);
    EndMessage();
}

}  // namespace antares
