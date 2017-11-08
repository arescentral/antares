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

#include <sfz/sfz.hpp>
#include <vector>

#include "config/keys.hpp"
#include "config/preferences.hpp"
#include "data/interface.hpp"
#include "data/picture.hpp"
#include "data/races.hpp"
#include "data/resource.hpp"
#include "data/string-list.hpp"
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

using sfz::BytesSlice;
using sfz::Exception;
using sfz::PrintItem;
using sfz::ReadSource;
using sfz::String;
using sfz::StringSlice;
using sfz::read;
using std::make_pair;
using std::max;
using std::min;
using std::pair;
using std::unique_ptr;
using std::vector;

namespace utf8 = sfz::utf8;

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

int find_replace(String& data, int pos, const StringSlice& search, const PrintItem& replace) {
    size_t at = data.find(search, pos);
    if (at != String::npos) {
        String replace_string;
        replace.print_to(replace_string);
        data.replace(at, search.size(), replace_string);
    }
    return at;
}

}  // namespace

void CreateWeaponDataText(
        String* text, Handle<BaseObject> weaponObject, const StringSlice& weaponName);

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

void CreateObjectDataText(String* text, Handle<BaseObject> object) {
    Resource rsrc("text", "txt", kShipDataTextID);
    String   data(utf8::decode(rsrc.data()));

    StringList keys(kShipDataKeyStringID);
    StringList values(kShipDataNameID);

    // *** Replace place-holders in text with real data, using the fabulous find_replace routine
    // an object or a ship?
    if (object->attributes & kCanThink) {
        const StringSlice& name = values.at(0);
        find_replace(data, 0, keys.at(kShipOrObjectStringNum), name);
    } else {
        const StringSlice& name = values.at(1);
        find_replace(data, 0, keys.at(kShipOrObjectStringNum), name);
    }

    // ship name
    {
        const StringSlice& name = get_object_name(object);
        find_replace(data, 0, keys.at(kShipTypeStringNum), name);
    }

    // ship mass
    find_replace(data, 0, keys.at(kMassStringNum), Fixed(object->mass));

    // ship shields
    find_replace(data, 0, keys.at(kShieldStringNum), object->health);

    // light speed
    find_replace(data, 0, keys.at(kHasLightStringNum), object->warpSpeed.val());

    // max velocity
    find_replace(data, 0, keys.at(kMaxSpeedStringNum), Fixed(object->maxVelocity));

    // thrust
    find_replace(data, 0, keys.at(kThrustStringNum), Fixed(object->maxThrust));

    // par turn
    find_replace(data, 0, keys.at(kTurnStringNum), Fixed(object->frame.rotation.turnAcceleration));

    // now, check for weapons!
    CreateWeaponDataText(&data, object->pulse.base, values.at(kShipDataPulseStringNum));
    CreateWeaponDataText(&data, object->beam.base, values.at(kShipDataBeamStringNum));
    CreateWeaponDataText(&data, object->special.base, values.at(kShipDataSpecialStringNum));

    print(*text, data);
}

void CreateWeaponDataText(
        String* text, Handle<BaseObject> weaponObject, const StringSlice& weaponName) {
    int32_t mostDamage;
    bool    isGuided = false;

    if (!weaponObject.get()) {
        return;
    }

    // TODO(sfiera): catch exception.
    Resource rsrc("text", "txt", kWeaponDataTextID);
    String   data(utf8::decode(rsrc.data()));
    // damage; this is tricky--we have to guess by walking through activate actions,
    //  and for all the createObject actions, see which creates the most damaging
    //  object.  We calc this first so we can use isGuided

    mostDamage = 0;
    isGuided   = false;
    if (weaponObject->activate.size() > 0) {
        for (auto action : weaponObject->activate) {
            if ((action->verb == kCreateObject) || (action->verb == kCreateObjectSetDest)) {
                Handle<BaseObject> missileObject = action->argument.createObject.whichBaseType;
                if (missileObject->attributes & kIsGuided)
                    isGuided = true;
                if (missileObject->damage > mostDamage)
                    mostDamage = missileObject->damage;
            }
        }
    }

    StringList keys(kShipDataKeyStringID);
    StringList values(kShipDataNameID);

    // weapon name #
    find_replace(data, 0, keys.at(kWeaponNumberStringNum), weaponName);

    // weapon name
    {
        const StringSlice& name = get_object_name(weaponObject);
        find_replace(data, 0, keys.at(kWeaponNameStringNum), name);
    }

    const StringSlice& yes  = values.at(kShipDataYesStringNum);
    const StringSlice& no   = values.at(kShipDataNoStringNum);
    const StringSlice& dash = values.at(kShipDataDashStringNum);

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
    find_replace(data, 0, keys.at(kWeaponRangeStringNum), lsqrt(weaponObject->frame.weapon.range));

    if (mostDamage > 0) {
        find_replace(data, 0, keys.at(kWeaponDamageStringNum), mostDamage);
    } else {
        find_replace(data, 0, keys.at(kWeaponDamageStringNum), dash);
    }
    print(*text, data);
}

void Replace_KeyCode_Strings_With_Actual_Key_Names(String* text, int16_t resID, size_t padTo) {
    StringList keys(kHelpScreenKeyStringID);
    StringList values(resID);

    for (int i = 0; i < kKeyExtendedControlNum; ++i) {
        const StringSlice& search = keys.at(i);
        String             replace(values.at(sys.prefs->key(i) - 1));
        // First, pad to the desired width.
        if (replace.size() < padTo) {
            replace.resize(padTo, ' ');
        }

        // Double backslashes.  The text produced here will be fed into
        // StyledText.set_retro_text(), which interprets backslashes
        // specially.  Don't do this until after padding, though.
        size_t pos = 0;
        while ((pos = find_replace(replace, pos, "\\", "\\\\")) != String::npos) {
            pos += 2;  // Don't find the just-inserted backslashes again.
        }

        // Replace search string with value string in resulting text.
        while (find_replace(*text, 0, search, replace) != String::npos) {
            pos += 1;
        };
    }
}

}  // namespace antares
