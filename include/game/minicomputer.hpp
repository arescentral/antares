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
    int32_t         hiliteLeft;
    int32_t         hiliteRight;
    int32_t         whichButton;
    lineSelectType  selectable;
    bool         underline;
    lineKindType    lineKind;
    int32_t         value;      // for keeping track of changing values
    int32_t         statusType;
    int32_t         whichStatus;
    int32_t         statusPlayer;
    int32_t         negativeValue;
    baseObjectType* sourceData;
};

void MiniScreenInit( void);
void MiniScreenCleanup( void);
void SetMiniScreenStatusStrList( int16_t);
void DisposeMiniScreenStatusStrList( void);
void ClearMiniScreenLines( void);
void ClearMiniObjectData( void);
void draw_mini_screen();
void MakeMiniScreenFromIndString( int16_t);
void minicomputer_handle_keys(uint32_t new_keys, uint32_t old_keys, bool cancel);
void minicomputer_cancel();
void MiniComputerHandleNull(int32_t);
int32_t MiniComputerGetPriceOfCurrentSelection( void);
void UpdateMiniScreenLines( void);
void draw_player_ammo(int32_t ammo_one, int32_t ammo_two, int32_t ammo_special);
void draw_mini_ship_data(
        const spaceObjectType& newObject, uint8_t headerColor,
        int16_t screenTop, int16_t whichString);
void MiniComputerDoAccept( void);
void MiniComputerExecute(int32_t, int32_t, int32_t);
void MiniComputerDoCancel( void);
void MiniComputerSetBuildStrings( void);
void MiniComputerHandleClick( Point);
void MiniComputerHandleDoubleClick( Point);
void MiniComputerHandleMouseUp( Point);
void MiniComputerHandleMouseStillDown( Point);
void MiniComputer_SetScreenAndLineHack(int32_t whichScreen, int32_t whichLine);

}  // namespace antares

#endif // ANTARES_GAME_MINICOMPUTER_HPP_
