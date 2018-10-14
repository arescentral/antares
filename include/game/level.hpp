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

#ifndef ANTARES_GAME_LEVEL_HPP_
#define ANTARES_GAME_LEVEL_HPP_

#include <bitset>
#include <map>
#include <pn/fwd>

#include "data/handle.hpp"
#include "math/geometry.hpp"

namespace antares {

union Level;

struct LoadState {
    bool    done = false;
    int32_t step = 0;
    int32_t max  = 1;  // So that (step / max) is 0 before construct_level() starts.
};

LoadState start_construct_level(const Level& level);
void      construct_level(LoadState* state);
void      DeclareWinner(Handle<Admiral> whichPlayer, const Level* nextLevel, pn::string_view text);
void      GetLevelFullScaleAndCorner(
             int32_t rotation, coordPointType* corner, int32_t* scale, Rect* bounds);
coordPointType Translate_Coord_To_Level_Rotation(int32_t h, int32_t v);

}  // namespace antares

#endif  // ANTARES_GAME_LEVEL_HPP_
