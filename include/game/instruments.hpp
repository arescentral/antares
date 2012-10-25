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

#ifndef ANTARES_GAME_INSTRUMENTS_HPP_
#define ANTARES_GAME_INSTRUMENTS_HPP_

#include "drawing/shapes.hpp"
#include "drawing/sprite-handling.hpp"

namespace antares {

const int32_t kMiniBuildTimeHeight = 25;

void InstrumentInit();
void UpdateRadar(int32_t);
void InstrumentCleanup();
void ResetInstruments();
void DrawInstrumentPanel();
void draw_instruments();
void EraseSite();
void update_site(bool replay);
void draw_site();
void update_sector_lines();
void draw_sector_lines();
void InstrumentsHandleClick();
void InstrumentsHandleDoubleClick();
void InstrumentsHandleMouseUp();
void InstrumentsHandleMouseStillDown();
void DrawArbitrarySectorLines(coordPointType *, int32_t, int32_t, Rect *, PixMap*);
void GetArbitrarySingleSectorBounds(coordPointType*, coordPointType*, int32_t, int32_t, Rect*,
        Rect*);

}  // namespace antares

#endif // ANTARES_GAME_INSTRUMENTS_HPP_
