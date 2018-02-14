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

#include "game/globals.hpp"

#include "config/gamepad.hpp"
#include "drawing/color.hpp"
#include "drawing/sprite-handling.hpp"
#include "game/admiral.hpp"
#include "game/input-source.hpp"
#include "game/labels.hpp"
#include "game/minicomputer.hpp"
#include "game/motion.hpp"
#include "game/space-object.hpp"
#include "game/starfield.hpp"
#include "game/vector.hpp"
#include "lang/defines.hpp"
#include "sound/driver.hpp"

namespace antares {

static ANTARES_GLOBAL aresGlobalType* gAresGlobal;

ANTARES_GLOBAL GlobalState& g = head;
ANTARES_GLOBAL GlobalState head;
ANTARES_GLOBAL GlobalState tail;

aresGlobalType* globals() { return gAresGlobal; }

void init_globals() {
    gAresGlobal = new aresGlobalType;

    g.time     = game_ticks();
    g.ship     = Handle<SpaceObject>(0);
    g.closest  = Handle<SpaceObject>(0);
    g.farthest = Handle<SpaceObject>(0);
}

aresGlobalType::aresGlobalType() {}

aresGlobalType::~aresGlobalType() {}

}  // namespace antares
