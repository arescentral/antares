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

#include <pn/output>
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

namespace antares {

namespace {

constexpr char kShipDataText[]        = "object";
constexpr char kShipDataNameStrings[] = "object";
constexpr char kWeaponDataText[]      = "weapon";

enum {
    kShipDataDashStringNum    = 2,
    kShipDataYesStringNum     = 3,
    kShipDataNoStringNum      = 4,
    kShipDataPulseStringNum   = 5,
    kShipDataBeamStringNum    = 6,
    kShipDataSpecialStringNum = 7,
};

constexpr char kHelpScreenKeyStrings[] = "controls/short";

pn::string_view::size_type find_replace(
        pn::string_ref data, int pos, pn::string_view search, pn::string_view replace) {
    size_t at = data.find(search, pos);
    if (at != data.npos) {
        data.replace(at, search.size(), replace);
    }
    return at;
}

}  // namespace

pn::string CreateWeaponDataText(
        const sfz::optional<BaseObject::Weapon>& weapon, pn::string_view weaponName);

//
// BothCommandAndQ:
//  returns true if both the command and q keys are set by player. If this is
//  true, then command-q for quit during a game should be disabled.
//

bool BothCommandAndQ() {
    bool command = false;
    bool q       = false;

    for (int i = 0; i < kKeyExtendedControlNum; i++) {
        Key key = sys.prefs->key(i);
        q |= (key == Key::Q);
        command |= (key == Key::COMMAND);
    }

    return command && q;
}

pn::string CreateObjectDataText(const BaseObject& object) {
    pn::string tpl = Resource::text(kShipDataText);

    auto values = Resource::strings(kShipDataNameStrings);

    bool can_think = (object.attributes & kCanThink);

    pn::string result = pn::format(
            tpl.c_str(), can_think ? values.at(0) : values.at(1), object.long_name,
            stringify(Fixed(object.mass)), object.health, object.warpSpeed.val(),
            stringify(Fixed(object.maxVelocity)), stringify(Fixed(object.thrust)),
            stringify(Fixed(object.turn_rate)));
    result += CreateWeaponDataText(object.weapons.pulse, values.at(kShipDataPulseStringNum));
    result += CreateWeaponDataText(object.weapons.beam, values.at(kShipDataBeamStringNum));
    result += CreateWeaponDataText(object.weapons.special, values.at(kShipDataSpecialStringNum));

    return result;
}

pn::string CreateWeaponDataText(
        const sfz::optional<BaseObject::Weapon>& weapon, pn::string_view weaponName) {
    if (!weapon.has_value()) {
        return pn::string{};
    }
    const auto& weaponObject = weapon->base;

    // TODO(sfiera): catch exception.
    pn::string tpl = Resource::text(kWeaponDataText);

    // damage; this is tricky--we have to guess by walking through activate actions,
    //  and for all the createObject actions, see which creates the most damaging
    //  object.  We calc this first so we can use is_guided
    int32_t mostDamage = 0;
    bool    is_guided  = false;
    if (weaponObject->activate.action.size() > 0) {
        for (const auto& action : weaponObject->activate.action) {
            const NamedHandle<const BaseObject>* created_base;
            switch (action.type()) {
                case Action::Type::CREATE: created_base = &action.create.base; break;
                case Action::Type::MORPH: created_base = &action.morph.base; break;
                case Action::Type::EQUIP: created_base = &action.equip.base; break;
                default: continue;
            }
            if ((*created_base)->attributes & kIsGuided) {
                is_guided = true;
            }
            if ((*created_base)->collide.damage > mostDamage) {
                mostDamage = (*created_base)->collide.damage;
            }
        }
    }

    auto values = Resource::strings(kShipDataNameStrings);

    pn::string_view yes         = values.at(kShipDataYesStringNum);
    pn::string_view no          = values.at(kShipDataNoStringNum);
    pn::string_view dash        = values.at(kShipDataDashStringNum);
    bool            auto_target = weaponObject->attributes & kAutoTarget;

    return pn::format(
            tpl.c_str(), weaponName, weaponObject->long_name, is_guided ? yes : no,
            lsqrt(weaponObject->device->range.squared),
            (mostDamage > 0) ? pn::dump(mostDamage, pn::dump_short) : dash,
            auto_target ? yes : no);
}

void Replace_KeyCode_Strings_With_Actual_Key_Names(
        pn::string& text, pn::string_view string_rsrc, size_t padTo) {
    auto keys   = Resource::strings(kHelpScreenKeyStrings);
    auto values = Resource::strings(string_rsrc);

    for (int i = 0; i < kKeyExtendedControlNum; ++i) {
        pn::string_view search  = keys.at(i);
        pn::string      replace = values.at(sys.prefs->key(i).value()).copy();
        // First, pad to the desired width.
        while (replace.size() < padTo) {
            replace += pn::rune{' '};
        }

        // Double backslashes.  The text produced here will be fed into
        // StyledText::retro(), which interprets backslashes specially.
        // Don't do this until after padding, though.
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
