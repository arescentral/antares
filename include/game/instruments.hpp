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

#ifndef ANTARES_GAME_INSTRUMENTS_HPP_
#define ANTARES_GAME_INSTRUMENTS_HPP_

#include "drawing/sprite-handling.hpp"
#include "math/units.hpp"

namespace antares {

class GameCursor;
class PlayerShip;

const int32_t kMiniBuildTimeHeight = 25;

void    InstrumentInit();
int32_t instrument_top();
void UpdateRadar(ticks units);
void InstrumentCleanup();
void ResetInstruments();
void set_up_instruments();
void draw_instruments();
void EraseSite();
void update_site(bool replay);
void draw_site(const PlayerShip& player);
void update_sector_lines();
void draw_sector_lines();
void InstrumentsHandleClick(const GameCursor& cursor);
void InstrumentsHandleDoubleClick(const GameCursor& cursor);
void InstrumentsHandleMouseUp(const GameCursor& cursor);
void InstrumentsHandleMouseStillDown(const GameCursor& cursor);
void draw_arbitrary_sector_lines(
        const coordPointType& corner, int32_t scale, int32_t minSectorSize, const Rect& bounds);
void GetArbitrarySingleSectorBounds(
        coordPointType*, coordPointType*, int32_t, int32_t, Rect*, Rect*);

}  // namespace antares

#endif  // ANTARES_GAME_INSTRUMENTS_HPP_
