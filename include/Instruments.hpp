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

#ifndef ANTARES_RADAR_HPP_
#define ANTARES_RADAR_HPP_

// Radar.h

#include "Base.h"

#include "NateDraw.hpp"

namespace antares {

#define kMiniBuildTimeHeight    ( 25)

int InstrumentInit();
void UpdateRadar(int32_t);
void InstrumentCleanup();
void ResetInstruments();
void ResetSectorLines();
void DrawInstrumentPanel();
//void EraseDrawMainSectorLines();
//void ShowMainSectorLines();
void EraseSite();
void EraseSectorLines();
void DrawSite();
void DrawSectorLines();
void ShowSite();
void ShowSectorLines();
void InstrumentsHandleClick();
void InstrumentsHandleDoubleClick();
void InstrumentsHandleMouseUp();
void InstrumentsHandleMouseStillDown();
void DrawArbitrarySectorLines(coordPointType *, int32_t, int32_t, Rect *, PixMap*, int32_t,
        int32_t);
void GetArbitrarySingleSectorBounds(coordPointType*, coordPointType*, int32_t, int32_t, Rect*,
        Rect*);
void UpdateBarIndicator(int16_t, int32_t, int32_t, PixMap*);
void DrawBuildTimeBar(int32_t);

}  // namespace antares

#endif // ANTARES_RADAR_HPP_
