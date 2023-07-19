// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2016-2017 The Antares Authors
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

#include "game/sys.hpp"

#include <sfz/sfz.hpp>

#include "config/gamepad.hpp"
#include "config/keys.hpp"
#include "data/resource.hpp"
#include "drawing/text.hpp"
#include "lang/defines.hpp"
#include "sound/driver.hpp"
#include "sound/fx.hpp"

namespace antares {

static constexpr char kMessageStrings[]          = "messages";
static constexpr char kMinicomputerStrings[]     = "minicomputer";
static constexpr char kCheatStrings[]            = "cheats/secret";
static constexpr char kCheatFeedbackOnStrings[]  = "cheats/enable";
static constexpr char kCheatFeedbackOffStrings[] = "cheats/disable";

constexpr char kInstLeftPictID[]  = "gui/instruments/left";
constexpr char kInstRightPictID[] = "gui/instruments/right";

ANTARES_GLOBAL SystemGlobals sys;

void sys_init() {
    sys.fonts.tactical     = font("tactical");
    sys.fonts.computer     = font("computer");
    sys.fonts.button       = font("button");
    sys.fonts.title        = font("title");
    sys.fonts.small_button = font("button-small");

    sys.key_names          = Resource::strings(kKeyNameStrings);
    sys.key_long_names     = Resource::strings(kKeyLongNameStrings);
    sys.gamepad_names      = Resource::strings(Gamepad::kNameStrings);
    sys.gamepad_long_names = Resource::strings(Gamepad::kLongNameStrings);

    sys.rot_table = Resource::rotation_table();

    sys.messages     = Resource::strings(kMessageStrings);
    sys.minicomputer = Resource::strings(kMinicomputerStrings);
    sys.cheat.codes  = Resource::strings(kCheatStrings);
    sys.cheat.on     = Resource::strings(kCheatFeedbackOnStrings);
    sys.cheat.off    = Resource::strings(kCheatFeedbackOffStrings);

    if (sys.audio) {
        sys.sound.init();
        sys.music.init();
    }

    sys.pix.reset();

    sys.left_instrument_texture  = Resource::texture(kInstLeftPictID);
    sys.right_instrument_texture = Resource::texture(kInstRightPictID);
}

void sys_shutdown() {
    if (sys.audio) {
        sys.music.shutdown();
        sys.sound.shutdown();
    }
}

}  // namespace antares
