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

//
// Liaison between the application and Interface Drawing.  Takes in events ( key events, mouse
// down events), hilights and scrolls as needed, and returns results.  Also handles editable text.
//

#include "ui/interface-handling.hpp"

#include <vector>
#include <sfz/sfz.hpp>

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
#include "game/main.hpp"
#include "game/messages.hpp"
#include "game/scenario-maker.hpp"
#include "game/space-object.hpp"
#include "game/starfield.hpp"
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
using sfz::scoped_array;
using sfz::scoped_ptr;
using std::make_pair;
using std::max;
using std::min;
using std::pair;
using std::vector;

namespace utf8 = sfz::utf8;

namespace antares {

namespace {

const int32_t kTargetScreenWidth = 640;
const int32_t kTargetScreenHeight = 480;

const int32_t kMissionDataWidth         = 200;
const int32_t kMissionDataVBuffer       = 40;
const int32_t kMissionDataTopBuffer     = 30;
const int32_t kMissionDataBottomBuffer  = 15;
const int32_t kMissionDataHBuffer       = 41;
const int32_t kMissionLineHJog          = 10;
const int32_t kMissionBriefPointOffset  = 2;

const int16_t kShipDataTextID       = 6001;
const int16_t kShipDataKeyStringID  = 6001;
const int16_t kShipDataNameID       = 6002;
const int16_t kWeaponDataTextID     = 6003;

enum {
    kShipOrObjectStringNum      = 0,
    kShipTypeStringNum          = 1,
    kMassStringNum              = 2,
    kShieldStringNum            = 3,
    kHasLightStringNum          = 4,
    kMaxSpeedStringNum          = 5,
    kThrustStringNum            = 6,
    kTurnStringNum              = 7,
    kWeaponNumberStringNum      = 8,
    kWeaponNameStringNum        = 9,
    kWeaponGuidedStringNum      = 10,
    kWeaponRangeStringNum       = 11,
    kWeaponDamageStringNum      = 12,
    kWeaponAutoTargetStringNum  = 13,
};

enum {
    kShipDataDashStringNum      = 2,
    kShipDataYesStringNum       = 3,
    kShipDataNoStringNum        = 4,
    kShipDataPulseStringNum     = 5,
    kShipDataBeamStringNum      = 6,
    kShipDataSpecialStringNum   = 7,
};

const int32_t kShipDataWidth = 240;

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

void CreateWeaponDataText(sfz::String* text, long whichWeapon, const sfz::StringSlice& weaponName);

//
// BothCommandAndQ:
//  returns true if both the command and q keys are set by player. If this is
//  true, then command-q for quit during a game should be disabled.
//

bool BothCommandAndQ() {
    bool command = false;
    bool q = false;

    for (int i = 0; i < kKeyExtendedControlNum; i++) {
        uint32_t key = Preferences::preferences()->key(i);
        q |= (key == Keys::Q);
        command |= (key == Keys::COMMAND);
    }

    return command && q;
}

void update_mission_brief_point(
        interfaceItemType *dataItem, long whichBriefPoint, const Scenario* scenario,
        coordPointType *corner, long scale, Rect *bounds, vector<inlinePictType>& inlinePict,
        Rect& highlight_rect, vector<pair<Point, Point> >& lines, String& text) {
    if (whichBriefPoint < kMissionBriefPointOffset) {
        // No longer handled here.
        return;
    }

    whichBriefPoint -= kMissionBriefPointOffset;

    Rect hiliteBounds;
    long            headerID, headerNumber, contentID;
    BriefPoint_Data_Get(whichBriefPoint, scenario, &headerID, &headerNumber, &contentID,
            &hiliteBounds, corner, scale, 16, 32, bounds);
    hiliteBounds.offset(bounds->left, bounds->top);

    // TODO(sfiera): catch exception.
    Resource rsrc("text", "txt", contentID);
    text.assign(utf8::decode(rsrc.data()));
    short textHeight = GetInterfaceTextHeightFromWidth(text, dataItem->style, kMissionDataWidth);
    if (hiliteBounds.left == hiliteBounds.right) {
        dataItem->bounds.left = (bounds->right - bounds->left) / 2 - (kMissionDataWidth / 2) + bounds->left;
        dataItem->bounds.right = dataItem->bounds.left + kMissionDataWidth;
        dataItem->bounds.top = (bounds->bottom - bounds->top) / 2 - (textHeight / 2) + bounds->top;
        dataItem->bounds.bottom = dataItem->bounds.top + textHeight;
        highlight_rect = Rect();
    } else {
        if ((hiliteBounds.left + (hiliteBounds.right - hiliteBounds.left) / 2) >
                (bounds->left + (bounds->right - bounds->left) / 2)) {
            dataItem->bounds.right = hiliteBounds.left - kMissionDataHBuffer;
            dataItem->bounds.left = dataItem->bounds.right - kMissionDataWidth;
        } else {
            dataItem->bounds.left = hiliteBounds.right + kMissionDataHBuffer;
            dataItem->bounds.right = dataItem->bounds.left + kMissionDataWidth;
        }

        dataItem->bounds.top = hiliteBounds.top + (hiliteBounds.bottom - hiliteBounds.top) / 2 -
                                textHeight / 2;
        dataItem->bounds.bottom = dataItem->bounds.top + textHeight;
        if (dataItem->bounds.top < (bounds->top + kMissionDataTopBuffer)) {
            dataItem->bounds.top = bounds->top + kMissionDataTopBuffer;
            dataItem->bounds.bottom = dataItem->bounds.top + textHeight;
        }
        if (dataItem->bounds.bottom > (bounds->bottom - kMissionDataBottomBuffer)) {
            dataItem->bounds.bottom = bounds->bottom - kMissionDataBottomBuffer;
            dataItem->bounds.top = dataItem->bounds.bottom - textHeight;
        }

        if (dataItem->bounds.left < (bounds->left + kMissionDataVBuffer)) {
            dataItem->bounds.left = bounds->left + kMissionDataVBuffer;
            dataItem->bounds.right = dataItem->bounds.left + kMissionDataWidth;
        }
        if (dataItem->bounds.right > (bounds->right - kMissionDataVBuffer)) {
            dataItem->bounds.right = bounds->right - kMissionDataVBuffer;
            dataItem->bounds.left = dataItem->bounds.right - kMissionDataWidth;
        }

        hiliteBounds.right++;
        hiliteBounds.bottom++;
        highlight_rect = hiliteBounds;
        Rect newRect;
        GetAnyInterfaceItemGraphicBounds(*dataItem, &newRect);
        lines.clear();
        if (dataItem->bounds.right < hiliteBounds.left) {
            Point p1(hiliteBounds.left, hiliteBounds.top);
            Point p2(newRect.right + kMissionLineHJog, hiliteBounds.top);
            Point p3(newRect.right + kMissionLineHJog, newRect.top);
            Point p4(newRect.right + 2, newRect.top);
            lines.push_back(make_pair(p1, p2));
            lines.push_back(make_pair(p2, p3));
            lines.push_back(make_pair(p3, p4));

            Point p5(hiliteBounds.left, hiliteBounds.bottom - 1);
            Point p6(newRect.right + kMissionLineHJog, hiliteBounds.bottom - 1);
            Point p7(newRect.right + kMissionLineHJog, newRect.bottom - 1);
            Point p8(newRect.right + 2, newRect.bottom - 1);
            lines.push_back(make_pair(p5, p6));
            lines.push_back(make_pair(p6, p7));
            lines.push_back(make_pair(p7, p8));
        } else {
            Point p1(hiliteBounds.right, hiliteBounds.top);
            Point p2(newRect.left - kMissionLineHJog, hiliteBounds.top);
            Point p3(newRect.left - kMissionLineHJog, newRect.top);
            Point p4(newRect.left - 3, newRect.top);
            lines.push_back(make_pair(p1, p2));
            lines.push_back(make_pair(p2, p3));
            lines.push_back(make_pair(p3, p4));

            Point p5(hiliteBounds.right, hiliteBounds.bottom - 1);
            Point p6(newRect.left - kMissionLineHJog, hiliteBounds.bottom - 1);
            Point p7(newRect.left - kMissionLineHJog, newRect.bottom - 1);
            Point p8(newRect.left - 3, newRect.bottom - 1);
            lines.push_back(make_pair(p5, p6));
            lines.push_back(make_pair(p6, p7));
            lines.push_back(make_pair(p7, p8));
        }
    }
    dataItem->item.labeledRect.label.stringID = headerID;
    dataItem->item.labeledRect.label.stringNumber = headerNumber;
    Rect newRect;
    GetAnyInterfaceItemGraphicBounds(*dataItem, &newRect);
    populate_inline_picts(dataItem->bounds, text, dataItem->style, inlinePict);
}

void CreateObjectDataText(String* text, short id) {
    Resource rsrc("text", "txt", kShipDataTextID);
    String data(utf8::decode(rsrc.data()));

    const baseObjectType& baseObject = gBaseObjectData.get()[id];

    StringList keys(kShipDataKeyStringID);
    StringList values(kShipDataNameID);

    // *** Replace place-holders in text with real data, using the fabulous find_replace routine
    // an object or a ship?
    if ( baseObject.attributes & kCanThink) {
        const StringSlice& name = values.at(0);
        find_replace(data, 0, keys.at(kShipOrObjectStringNum), name);
    } else {
        const StringSlice& name = values.at(1);
        find_replace(data, 0, keys.at(kShipOrObjectStringNum), name);
    }

    // ship name
    {
        StringList names(5000);
        const StringSlice& name = names.at(id);
        find_replace(data, 0, keys.at(kShipTypeStringNum), name);
    }

    // ship mass
    find_replace(data, 0, keys.at(kMassStringNum), fixed(baseObject.mass));

    // ship shields
    find_replace(data, 0, keys.at(kShieldStringNum), baseObject.health);

    // light speed
    find_replace(data, 0, keys.at(kHasLightStringNum), baseObject.warpSpeed);

    // max velocity
    find_replace(data, 0, keys.at(kMaxSpeedStringNum), fixed(baseObject.maxVelocity));

    // thrust
    find_replace(data, 0, keys.at(kThrustStringNum), fixed(baseObject.maxThrust));

    // par turn
    find_replace(data, 0, keys.at(kTurnStringNum),
            fixed(baseObject.frame.rotation.turnAcceleration));

    // now, check for weapons!
    CreateWeaponDataText(&data, baseObject.pulse, values.at(kShipDataPulseStringNum));
    CreateWeaponDataText(&data, baseObject.beam, values.at(kShipDataBeamStringNum));
    CreateWeaponDataText(&data, baseObject.special, values.at(kShipDataSpecialStringNum));

    print(*text, data);
}

void CreateWeaponDataText(String* text, long whichWeapon, const StringSlice& weaponName) {
    baseObjectType      *weaponObject, *missileObject;
    long                mostDamage, actionNum;
    objectActionType    *action;
    bool             isGuided = false;

    if (whichWeapon == kNoShip) {
        return;
    }

    weaponObject = gBaseObjectData.get() + whichWeapon;

    // TODO(sfiera): catch exception.
    Resource rsrc("text", "txt", kWeaponDataTextID);
    String data(utf8::decode(rsrc.data()));
    // damage; this is tricky--we have to guess by walking through activate actions,
    //  and for all the createObject actions, see which creates the most damaging
    //  object.  We calc this first so we can use isGuided

    mostDamage = 0;
    isGuided = false;
    if ( weaponObject->activateActionNum > 0)
    {
        action = gObjectActionData.get() + weaponObject->activateAction;
        for ( actionNum = 0; actionNum < weaponObject->activateActionNum; actionNum++)
        {
            if (( action->verb == kCreateObject) || ( action->verb == kCreateObjectSetDest))
            {
                missileObject = gBaseObjectData.get() +
                    action->argument.createObject.whichBaseType;
                if ( missileObject->attributes & kIsGuided) isGuided = true;
                if ( missileObject->damage > mostDamage) mostDamage = missileObject->damage;
            }
            action++;
        }
    }

    StringList keys(kShipDataKeyStringID);
    StringList values(kShipDataNameID);

    // weapon name #
    find_replace(data, 0, keys.at(kWeaponNumberStringNum), weaponName);

    // weapon name
    {
        StringList names(5000);
        const StringSlice& name = names.at(whichWeapon);
        find_replace(data, 0, keys.at(kWeaponNameStringNum), name);
    }

    const StringSlice& yes = values.at(kShipDataYesStringNum);
    const StringSlice& no = values.at(kShipDataNoStringNum);
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
    find_replace(data, 0, keys.at(kWeaponRangeStringNum),
            lsqrt(weaponObject->frame.weapon.range));

    if (mostDamage > 0) {
        find_replace(data, 0, keys.at(kWeaponDamageStringNum), mostDamage);
    } else {
        find_replace(data, 0, keys.at(kWeaponDamageStringNum), dash);
    }
    print(*text, data);
}

void Replace_KeyCode_Strings_With_Actual_Key_Names(String* text, short resID, size_t padTo) {
    StringList keys(kHelpScreenKeyStringID);
    StringList values(resID);

    for (int i = 0; i < kKeyExtendedControlNum; ++i) {
        const StringSlice& search = keys.at(i);
        String replace(values.at(Preferences::preferences()->key(i) - 1));
        if (replace.size() < padTo) {
            replace.resize(padTo, ' ');
        }
        while (find_replace(*text, 0, search, replace) > 0) {
            // DO NOTHING
        };
    }
}

}  // namespace antares
