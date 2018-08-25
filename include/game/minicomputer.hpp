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

#ifndef ANTARES_GAME_MINICOMPUTER_HPP_
#define ANTARES_GAME_MINICOMPUTER_HPP_

#include "data/base-object.hpp"
#include "data/level.hpp"

namespace antares {

enum MiniScreenLineKind {
    MINI_NONE       = 0,
    MINI_DIM        = 1,
    MINI_SELECTABLE = 2,
    MINI_BUTTON_ON  = 3,
    MINI_BUTTON_OFF = 4,
};

struct miniScreenLineType {
    MiniScreenLineKind      kind = MINI_NONE;
    pn::string              string;
    pn::string              statusFalse;
    pn::string              statusTrue;
    pn::string              statusString;
    pn::string              postString;
    int32_t                 whichButton = -1;
    bool                    underline   = false;
    int32_t                 value;  // for keeping track of changing values
    int32_t                 statusType;
    Handle<const Condition> condition;
    Counter                 counter;
    int32_t                 negativeValue;
    const BaseObject*       sourceData;
    void (*callback)(Handle<Admiral> adm, int32_t line) = nullptr;
};

void MiniScreenInit(void);
void MiniScreenCleanup(void);
void DisposeMiniScreenStatusStrList(void);
void ClearMiniScreenLines(void);
void draw_mini_screen();
void minicomputer_handle_keys(uint32_t key_presses, uint32_t key_releases);
void minicomputer_cancel();
Cash MiniComputerGetPriceOfCurrentSelection(void);
void UpdateMiniScreenLines(void);
void draw_player_ammo(int32_t ammo_one, int32_t ammo_two, int32_t ammo_special);
void MiniComputerDoAccept(void);
void transfer_control(Handle<Admiral> adm, int line);
void MiniComputerDoCancel(void);
void MiniComputerSetBuildStrings(void);
void MiniComputerHandleClick(Point);
void MiniComputerHandleDoubleClick(Point);
void MiniComputerHandleMouseUp(Point);
void MiniComputerHandleMouseStillDown(Point);
void MiniComputer_SetScreenAndLineHack(Screen whichScreen, int32_t whichLine);

}  // namespace antares

#endif  // ANTARES_GAME_MINICOMPUTER_HPP_
