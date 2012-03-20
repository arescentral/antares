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

#ifndef ANTARES_GAME_MINICOMPUTER_HPP_
#define ANTARES_GAME_MINICOMPUTER_HPP_

#include "data/space-object.hpp"

namespace antares {

enum lineKindType {
    plainLineKind = 0,
    buttonOffLineKind = 1,
    buttonOnLineKind = 2
};

enum lineSelectType {
    cannotSelect = 0,
    selectDim = 1,
    selectable = 2
};

struct miniScreenLineType {
    sfz::String     string;
    sfz::String     statusFalse;
    sfz::String     statusTrue;
    sfz::String     statusString;
    sfz::String     postString;
    long            hiliteLeft;
    long            hiliteRight;
    long            whichButton;
    lineSelectType  selectable;
    bool         underline;
    lineKindType    lineKind;
    long            value;      // for keeping track of changing values
    long            statusType;
    long            whichStatus;
    long            statusPlayer;
    long            negativeValue;
    baseObjectType* sourceData;
};

void MiniScreenInit( void);
void MiniScreenCleanup( void);
void SetMiniScreenStatusStrList( short);
void DisposeMiniScreenStatusStrList( void);
void ClearMiniScreenLines( void);
void ClearMiniObjectData( void);
void DrawMiniScreen( void);
void DrawAndShowMiniScreenLine( long);
void ShowWholeMiniScreen( void);
void MakeMiniScreenFromIndString( short);
void MiniComputerHandleKeys( unsigned long, unsigned long);
void MiniComputerHandleNull( long);
long MiniComputerGetPriceOfCurrentSelection( void);
void UpdateMiniScreenLines( void);
void UpdatePlayerAmmo(int32_t ammo_one, int32_t ammo_two, int32_t ammo_special);
void UpdateMiniShipData( spaceObjectType *, spaceObjectType *, unsigned char, short, short);
void MiniComputerDoAccept( void);
void MiniComputerExecute( long, long, long);
void MiniComputerDoCancel( void);
void MiniComputerSetBuildStrings( void);
void MiniComputerHandleClick( Point);
void MiniComputerHandleDoubleClick( Point);
void MiniComputerHandleMouseUp( Point);
void MiniComputerHandleMouseStillDown( Point);
void MiniComputer_SetScreenAndLineHack( long whichScreen, long whichLine);

}  // namespace antares

#endif // ANTARES_GAME_MINICOMPUTER_HPP_
