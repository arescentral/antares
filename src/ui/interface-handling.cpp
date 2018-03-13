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

//
// Liaison between the application and Interface Drawing.  Takes in events ( key events, mouse
// down events), hilights and scrolls as needed, and returns results.  Also handles editable text.
//

#include "ui/interface-handling.hpp"

#include <pn/file>
#include <vector>

#include "config/keys.hpp"
#include "config/preferences.hpp"
#include "data/interface.hpp"
#include "data/races.hpp"
#include "data/resource.hpp"
#include "drawing/briefing.hpp"
#include "drawing/color.hpp"
#include "drawing/interface.hpp"
#include "drawing/text.hpp"
#include "game/globals.hpp"
#include "game/instruments.hpp"
#include "game/level.hpp"
#include "game/main.hpp"
#include "game/messages.hpp"
#include "game/space-object.hpp"
#include "game/starfield.hpp"
#include "game/sys.hpp"
#include "math/fixed.hpp"
#include "math/special.hpp"
#include "sound/fx.hpp"
#include "sound/music.hpp"
#include "ui/interface-handling.hpp"
#include "video/driver.hpp"
#include "video/transitions.hpp"

using std::make_pair;
using std::max;
using std::min;
using std::pair;
using std::unique_ptr;
using std::vector;

namespace antares {

namespace {

const int16_t kShipDataTextID      = 6001;
const int16_t kShipDataKeyStringID = 6001;
const int16_t kShipDataNameID      = 6002;
const int16_t kWeaponDataTextID    = 6003;

enum {
    kShipOrObjectStringNum     = 0,
    kShipTypeStringNum         = 1,
    kMassStringNum             = 2,
    kShieldStringNum           = 3,
    kHasLightStringNum         = 4,
    kMaxSpeedStringNum         = 5,
    kThrustStringNum           = 6,
    kTurnStringNum             = 7,
    kWeaponNumberStringNum     = 8,
    kWeaponNameStringNum       = 9,
    kWeaponGuidedStringNum     = 10,
    kWeaponRangeStringNum      = 11,
    kWeaponDamageStringNum     = 12,
    kWeaponAutoTargetStringNum = 13,
};

enum {
    kShipDataDashStringNum    = 2,
    kShipDataYesStringNum     = 3,
    kShipDataNoStringNum      = 4,
    kShipDataPulseStringNum   = 5,
    kShipDataBeamStringNum    = 6,
    kShipDataSpecialStringNum = 7,
};

const int16_t kHelpScreenKeyStringID = 6003;

pn::string_view::size_type find_replace(
        pn::string_ref data, int pos, pn::string_view search, pn::string_view replace) {
    size_t at = data.find(search, pos);
    if (at != data.npos) {
        data.replace(at, search.size(), replace);
    }
    return at;
}

}  // namespace

void CreateWeaponDataText(
        pn::string* text, Handle<BaseObject> weaponObject, pn::string_view weaponName);

//
// BothCommandAndQ:
//  returns true if both the command and q keys are set by player. If this is
//  true, then command-q for quit during a game should be disabled.
//

bool BothCommandAndQ() {
    bool command = false;
    bool q       = false;

    for (int i = 0; i < kKeyExtendedControlNum; i++) {
        uint32_t key = sys.prefs->key(i);
        q |= (key == Keys::Q);
        command |= (key == Keys::L_COMMAND);
    }

    return command && q;
}

void CreateObjectDataText(pn::string& text, Handle<BaseObject> object) {
    pn::string data = Resource::text(kShipDataTextID);

    auto keys   = Resource::strings(kShipDataKeyStringID);
    auto values = Resource::strings(kShipDataNameID);

    // *** Replace place-holders in text with real data, using the fabulous find_replace routine
    // an object or a ship?
    if (object->attributes & kCanThink) {
        pn::string_view name = values.at(0);
        find_replace(data, 0, keys.at(kShipOrObjectStringNum), name);
    } else {
        pn::string_view name = values.at(1);
        find_replace(data, 0, keys.at(kShipOrObjectStringNum), name);
    }

    // ship name
    find_replace(data, 0, keys.at(kShipTypeStringNum), object->name);

    // ship mass
    find_replace(data, 0, keys.at(kMassStringNum), stringify(Fixed(object->mass)));

    // ship shields
    find_replace(data, 0, keys.at(kShieldStringNum), pn::dump(object->health, pn::dump_short));

    // light speed
    find_replace(
            data, 0, keys.at(kHasLightStringNum),
            pn::dump(object->warpSpeed.val(), pn::dump_short));

    // max velocity
    find_replace(data, 0, keys.at(kMaxSpeedStringNum), stringify(Fixed(object->maxVelocity)));

    // thrust
    find_replace(data, 0, keys.at(kThrustStringNum), stringify(Fixed(object->maxThrust)));

    // par turn
    if (object->attributes & kCanTurn) {
        if (object->attributes & kShapeFromDirection) {
            find_replace(
                    data, 0, keys.at(kTurnStringNum),
                    stringify(Fixed(object->frame.rotation.turn_rate)));
        } else {
            find_replace(data, 0, keys.at(kTurnStringNum), stringify(Fixed::from_long(1)));
        }
    } else {
        find_replace(data, 0, keys.at(kTurnStringNum), stringify(Fixed::zero()));
    }

    // now, check for weapons!
    CreateWeaponDataText(&data, object->pulse.base, values.at(kShipDataPulseStringNum));
    CreateWeaponDataText(&data, object->beam.base, values.at(kShipDataBeamStringNum));
    CreateWeaponDataText(&data, object->special.base, values.at(kShipDataSpecialStringNum));

    text = std::move(data);
}

void CreateWeaponDataText(
        pn::string* text, Handle<BaseObject> weaponObject, pn::string_view weaponName) {
    int32_t mostDamage;
    bool    isGuided = false;

    if (!weaponObject.get()) {
        return;
    }

    // TODO(sfiera): catch exception.
    pn::string data = Resource::text(kWeaponDataTextID);
    // damage; this is tricky--we have to guess by walking through activate actions,
    //  and for all the createObject actions, see which creates the most damaging
    //  object.  We calc this first so we can use isGuided

    mostDamage = 0;
    isGuided   = false;
    if (weaponObject->activate.size() > 0) {
        for (const auto& action : weaponObject->activate) {
            Handle<BaseObject> created_base = action->created_base();
            if (created_base.get()) {
                if (created_base->attributes & kIsGuided) {
                    isGuided = true;
                }
                if (created_base->damage > mostDamage) {
                    mostDamage = created_base->damage;
                }
            }
        }
    }

    auto keys   = Resource::strings(kShipDataKeyStringID);
    auto values = Resource::strings(kShipDataNameID);

    // weapon name #
    find_replace(data, 0, keys.at(kWeaponNumberStringNum), weaponName);

    // weapon name
    find_replace(data, 0, keys.at(kWeaponNameStringNum), weaponObject->name);

    pn::string_view yes  = values.at(kShipDataYesStringNum);
    pn::string_view no   = values.at(kShipDataNoStringNum);
    pn::string_view dash = values.at(kShipDataDashStringNum);

    // is guided
    if (isGuided) {
        find_replace(data, 0, keys.at(kWeaponGuidedStringNum), yes);
    } else {
        find_replace(data, 0, keys.at(kWeaponGuidedStringNum), no);
    }

    // is autotarget
    if (weaponObject->attributes & kAutoTarget) {
        find_replace(data, 0, keys.at(kWeaponAutoTargetStringNum), yes);
    } else {
        find_replace(data, 0, keys.at(kWeaponAutoTargetStringNum), no);
    }

    // range
    find_replace(
            data, 0, keys.at(kWeaponRangeStringNum),
            pn::dump((int64_t)lsqrt(weaponObject->frame.weapon.range), pn::dump_short));

    if (mostDamage > 0) {
        find_replace(
                data, 0, keys.at(kWeaponDamageStringNum), pn::dump(mostDamage, pn::dump_short));
    } else {
        find_replace(data, 0, keys.at(kWeaponDamageStringNum), dash);
    }
    *text += data;
}

void Replace_KeyCode_Strings_With_Actual_Key_Names(pn::string& text, int16_t resID, size_t padTo) {
    auto keys   = Resource::strings(kHelpScreenKeyStringID);
    auto values = Resource::strings(resID);

    for (int i = 0; i < kKeyExtendedControlNum; ++i) {
        pn::string_view search  = keys.at(i);
        pn::string      replace = values.at(sys.prefs->key(i) - 1).copy();
        // First, pad to the desired width.
        while (replace.size() < padTo) {
            replace += pn::rune{' '};
        }

        // Double backslashes.  The text produced here will be fed into
        // StyledText.set_retro_text(), which interprets backslashes
        // specially.  Don't do this until after padding, though.
        size_t pos = 0;
        while ((pos = find_replace(replace, pos, pn::string{"\\"}, "\\\\")) != replace.npos) {
            pos += 2;  // Don't find the just-inserted backslashes again.
        }

        // Replace search string with value string in resulting text.
        while (find_replace(text, 0, search, replace) != text.npos) {
            pos += 1;
        };
    }
}

}  // namespace antares
