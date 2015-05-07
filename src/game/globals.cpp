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

#include "game/globals.hpp"

#include "config/gamepad.hpp"
#include "data/string-list.hpp"
#include "drawing/color.hpp"
#include "drawing/sprite-handling.hpp"
#include "game/admiral.hpp"
#include "game/input-source.hpp"
#include "game/minicomputer.hpp"
#include "game/motion.hpp"
#include "game/space-object.hpp"
#include "game/starfield.hpp"
#include "sound/driver.hpp"

namespace antares {

static aresGlobalType* gAresGlobal;
GlobalState g;

aresGlobalType* globals() {
    return gAresGlobal;
}

void init_globals() {
    gAresGlobal = new aresGlobalType;
}

aresGlobalType::aresGlobalType() {
    gKeyMapBuffer = new KeyMap[kKeyMapBufferNum];
    gKeyMapBufferTop = 0;
    gKeyMapBufferBottom = 0;
    gGameOver = 1;
    g.time = 0;
    gClosestObject = Handle<SpaceObject>(0);
    gFarthestObject = Handle<SpaceObject>(0);
    gCenterScaleH = 0;
    gCenterScaleV = 0;
    g.ship = Handle<SpaceObject>(0);
    gZoomMode = kTimesTwoZoom;
    gPreviousZoomMode = kNearestFoeZoom;
    gRadarCount = 0;
    gRadarSpeed = 30;
    gRadarRange = kRadarSize * 50;
    radar_is_functioning = false;
    gWhichScaleNum = 0;
    gLastScale = SCALE_SCALE;
    gInstrumentTop = 0;
    gLastSoundTime = 0;
    key_names.reset(new StringList(KEY_NAMES));
    key_long_names.reset(new StringList(KEY_LONG_NAMES));
    gamepad_names.reset(new StringList(Gamepad::NAMES));
    gamepad_long_names.reset(new StringList(Gamepad::LONG_NAMES));
    gAutoPilotOff = true;
    levelNum = 31;
    keyMask = 0;
    gSerialNumerator = 0;
    gSerialDenominator = 0;

    hotKeyDownTime = -1;
}

aresGlobalType::~aresGlobalType() {
}

}  // namespace antares
