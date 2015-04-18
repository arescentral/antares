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

#include "game/player-ship.hpp"

#include <sfz/sfz.hpp>

#include "config/gamepad.hpp"
#include "config/keys.hpp"
#include "config/preferences.hpp"
#include "data/space-object.hpp"
#include "data/string-list.hpp"
#include "drawing/color.hpp"
#include "drawing/text.hpp"
#include "game/admiral.hpp"
#include "game/cheat.hpp"
#include "game/cursor.hpp"
#include "game/globals.hpp"
#include "game/input-source.hpp"
#include "game/instruments.hpp"
#include "game/labels.hpp"
#include "game/messages.hpp"
#include "game/minicomputer.hpp"
#include "game/non-player-ship.hpp"
#include "game/scenario-maker.hpp"
#include "game/space-object.hpp"
#include "game/starfield.hpp"
#include "game/time.hpp"
#include "math/fixed.hpp"
#include "math/macros.hpp"
#include "math/rotation.hpp"
#include "math/special.hpp"
#include "math/units.hpp"
#include "sound/fx.hpp"

using sfz::Exception;
using sfz::BytesSlice;
using sfz::PrintTarget;
using sfz::String;
using sfz::StringSlice;
using sfz::format;

namespace macroman = sfz::macroman;

namespace antares {

namespace {

const int32_t kSendMessageVOffset = 20;
const int32_t kCursorBoundsSize = 16;  // should be same in instruments.c

}  // namespace

int32_t HotKey_GetFromObject( spaceObjectType *object);
void Update_LabelStrings_ForHotKeyChange( void);

namespace {

static KeyMap gLastKeyMap;
static int32_t gDestKeyTime = 0;
static int32_t gDestinationLabel = -1;
static int32_t gAlarmCount = -1;
static int32_t gSendMessageLabel = -1;

struct HotKeySuffix {
    spaceObjectType* space_object;
};

void print_to(PrintTarget out, const HotKeySuffix& suffix) {
    int h = HotKey_GetFromObject(suffix.space_object);
    if (h < 0) {
        return;
    }

    int keyNum = Preferences::preferences()->key(h + kFirstHotKeyNum);
    if (keyNum < 0) {
        return;
    }

    StringList strings(KEY_LONG_NAMES);
    StringSlice key_name = strings.at(keyNum - 1);
    print(out, format(" < {0} >", key_name));
};

HotKeySuffix hot_key_suffix(spaceObjectType* space_object) {
    HotKeySuffix result = {space_object};
    return result;
};

}  // namespace

void ResetPlayerShip(int32_t which) {
    globals()->gPlayerShipNumber = which;
    globals()->gSelectionLabel = Labels::add(0, 0, 0, 10, NULL, true, YELLOW);
    gDestinationLabel = Labels::add(0, 0, 0, -20, NULL, true, SKY_BLUE);
    gSendMessageLabel = Labels::add(200, 200, 0, 30, NULL, false, GREEN);
    globals()->starfield.reset(globals()->gPlayerShipNumber);
    gAlarmCount = -1;
    globals()->gAutoPilotOff = true;
    globals()->keyMask = 0;
    gLastKeyMap.clear();
    globals()->gLastMessageKeyMap.clear();
    globals()->gZoomMode = kNearestFoeZoom;
    globals()->gKeyMapBufferTop = globals()->gKeyMapBufferBottom = 0;

    for (int h = 0; h < kHotKeyNum; h++) {
        globals()->hotKey[h].objectNum = -1;
        globals()->hotKey[h].objectID = -1;
    }
    globals()->hotKeyDownTime = 0;
    globals()->lastHotKey = -1;
    globals()->destKeyUsedForSelection = false;
    globals()->hotKey_target = false;
}

PlayerShip::PlayerShip():
    gTheseKeys(0),
    _gamepad_keys(0),
    gLastKeys(0),
    _gamepad_state(NO_BUMPER),
    _control_active(false),
    _control_direction(0) { }

void PlayerShip::update_keys(const KeyMap& keys) {
    for (int i = 0; i < 256; ++i) {
        if (keys.get(i) && ! _keys.get(i)) {
            key_down(KeyDownEvent(now_usecs(), i));
        } else if (_keys.get(i) && ! keys.get(i)) {
            key_up(KeyUpEvent(now_usecs(), i));
        }
    }
}

static int key_num(uint32_t key) {
    for (int i = 0; i < kKeyExtendedControlNum; ++i) {
        if (key == (Preferences::preferences()->key(i) - 1)) {
            return i;
        }
    }
    return -1;
}

static void zoom_to(ZoomType zoom) {
    if (globals()->gZoomMode != zoom) {
        globals()->gZoomMode = zoom;
        PlayVolumeSound(kComputerBeep3, kMediumVolume, kMediumPersistence, kLowPrioritySound);
        StringList strings(kMessageStringID);
        StringSlice string = strings.at(globals()->gZoomMode + kZoomStringOffset - 1);
        Messages::set_status(string, kStatusLabelColor);
    }
}

static void zoom_shortcut(ZoomType zoom) {
    if (globals()->keyMask & kShortcutZoomMask) {
        return;
    }
    ZoomType previous = globals()->gPreviousZoomMode;
    globals()->gPreviousZoomMode = globals()->gZoomMode;
    if (globals()->gZoomMode == zoom) {
        zoom_to(previous);
    } else {
        zoom_to(zoom);
    }
}

static void zoom_in() {
    if (globals()->keyMask & kZoomInKey) {
        return;
    }
    if (globals()->gZoomMode > kTimesTwoZoom) {
        zoom_to(static_cast<ZoomType>(globals()->gZoomMode - 1));
    }
}

static void zoom_out() {
    if (globals()->keyMask & kZoomOutKey) {
        return;
    }
    if (globals()->gZoomMode < kSmallestZoom) {
        zoom_to(static_cast<ZoomType>(globals()->gZoomMode + 1));
    }
}

static void engage_autopilot() {
    spaceObjectType* player = mGetSpaceObjectPtr(globals()->gPlayerShipNumber);
    if (!(player->attributes & kOnAutoPilot)) {
        player->keysDown |= kAutoPilotKey;
    }
    player->keysDown |= kAdoptTargetKey;
}

static void pick_object(
        spaceObjectType* origin_ship, int32_t direction, bool destination, int32_t attributes,
        int32_t nonattributes, int32_t select_ship_num, Allegiance allegiance) {
    uint64_t huge_distance;
    if (select_ship_num >= 0) {
        spaceObjectType* select_ship = mGetSpaceObjectPtr(select_ship_num);
        uint32_t difference = ABS<int>(origin_ship->location.h - select_ship->location.h);
        uint32_t dcalc = difference;
        difference =  ABS<int>(origin_ship->location.v - select_ship->location.v);
        uint32_t distance = difference;

        if ((dcalc > kMaximumRelevantDistance)
                || (distance > kMaximumRelevantDistance)) {
            huge_distance = dcalc;  // must be positive
            MyWideMul(huge_distance, huge_distance, &huge_distance);
            select_ship->distanceFromPlayer = distance;
            MyWideMul(select_ship->distanceFromPlayer, select_ship->distanceFromPlayer, &select_ship->distanceFromPlayer);
            select_ship->distanceFromPlayer += huge_distance;
        } else {
            select_ship->distanceFromPlayer = distance * distance + dcalc * dcalc;
        }
        huge_distance = select_ship->distanceFromPlayer;
    } else {
        huge_distance = 0;
    }

    select_ship_num = GetManualSelectObject(
            origin_ship, direction, attributes, nonattributes, &huge_distance, select_ship_num,
            allegiance);

    if (select_ship_num >= 0) {
        if (destination) {
            SetPlayerSelectShip(select_ship_num, true, globals()->gPlayerAdmiralNumber);
        } else {
            SetPlayerSelectShip(select_ship_num, false, globals()->gPlayerAdmiralNumber);
        }
    }
}

static void select_friendly(spaceObjectType* origin_ship, int32_t direction) {
    pick_object(
            origin_ship, direction, false, kCanBeDestination, kIsDestination,
            GetAdmiralConsiderObject(globals()->gPlayerAdmiralNumber), FRIENDLY);
}

static void target_friendly(spaceObjectType* origin_ship, int32_t direction) {
    pick_object(
            origin_ship, direction, true, kCanBeDestination, kIsDestination,
            GetAdmiralDestinationObject(globals()->gPlayerAdmiralNumber), FRIENDLY);
}

static void target_hostile(spaceObjectType* origin_ship, int32_t direction) {
    pick_object(
            origin_ship, direction, true, kCanBeDestination, kIsDestination,
            GetAdmiralDestinationObject(globals()->gPlayerAdmiralNumber), HOSTILE);
}

static void select_base(spaceObjectType* origin_ship, int32_t direction) {
    pick_object(
            origin_ship, direction, false, kCanAcceptBuild, 0,
            GetAdmiralConsiderObject(globals()->gPlayerAdmiralNumber), FRIENDLY);
}

static void target_base(spaceObjectType* origin_ship, int32_t direction) {
    pick_object(
            origin_ship, direction, true, kIsDestination, 0,
            GetAdmiralDestinationObject(globals()->gPlayerAdmiralNumber), FRIENDLY_OR_HOSTILE);
}

static void target_self() {
    SetPlayerSelectShip(globals()->gPlayerShipNumber, true, globals()->gPlayerAdmiralNumber);
}

void PlayerShip::key_down(const KeyDownEvent& event) {
    _keys.set(event.key(), true);

    if (!active()) {
        return;
    }

    int key = key_num(event.key());
    switch (key) {
      case kZoomOutKeyNum:
        zoom_out();
        break;
      case kZoomInKeyNum:
        zoom_in();
        break;
      case kScale121KeyNum:
        zoom_shortcut(kActualSizeZoom);
        break;
      case kScale122KeyNum:
        zoom_shortcut(kHalfSizeZoom);
        break;
      case kScale124KeyNum:
        zoom_shortcut(kQuarterSizeZoom);
        break;
      case kScale1216KeyNum:
        zoom_shortcut(kEighthSizeZoom);
        break;
      case kScaleHostileKeyNum:
        zoom_shortcut(kNearestFoeZoom);
        break;
      case kScaleObjectKeyNum:
        zoom_shortcut(kNearestAnythingZoom);
        break;
      case kScaleAllKeyNum:
        zoom_shortcut(kSmallestZoom);
        break;
      case kTransferKeyNum:
        MiniComputerExecute(3, 1, globals()->gPlayerAdmiralNumber);
        break;
      default:
        if (key < kKeyControlNum) {
            if (!(gTheseKeys & (0x01 << key) & ~globals()->keyMask)) {
                gTheseKeys ^= (0x01 << key) & ~globals()->keyMask;
            }
        }
        break;
    }
}

void PlayerShip::key_up(const KeyUpEvent& event) {
    _keys.set(event.key(), false);

    if (!active()) {
        return;
    }

    int key = key_num(event.key());
    switch (key) {
      default:
        if (key < kKeyControlNum) {
            if (gTheseKeys & (0x01 << key) & ~globals()->keyMask) {
                gTheseKeys ^= (0x01 << key) & ~globals()->keyMask;
            }
        }
        break;
    }
}

void PlayerShip::gamepad_button_down(const GamepadButtonDownEvent& event) {
    switch (event.button) {
      case Gamepad::LB:
        if (_gamepad_state & SELECT_BUMPER) {
            _gamepad_state = TARGET_BUMPER_OVERRIDE;
        } else if (!(_gamepad_state & TARGET_BUMPER)) {
            _gamepad_state = TARGET_BUMPER;
        }
        return;
      case Gamepad::RB:
        if (_gamepad_state & TARGET_BUMPER) {
            _gamepad_state = SELECT_BUMPER_OVERRIDE;
        } else if (!(_gamepad_state & SELECT_BUMPER)) {
            _gamepad_state = SELECT_BUMPER;
        }
        return;
    }

    if (!active()) {
        return;
    }

    spaceObjectType* player = mGetSpaceObjectPtr(globals()->gPlayerShipNumber);
    if (_gamepad_state) {
        switch (event.button) {
          case Gamepad::A:
            if (_control_active) {
                if (_gamepad_state & SELECT_BUMPER) {
                    select_friendly(player, _control_direction);
                } else {
                    target_friendly(player, _control_direction);
                }
            }
            return;
          case Gamepad::B:
            if (_control_active) {
                if (_gamepad_state & TARGET_BUMPER) {
                    target_hostile(player, _control_direction);
                }
            }
            return;
          case Gamepad::X:
            if (_control_active) {
                if (_gamepad_state & SELECT_BUMPER) {
                    select_base(player, _control_direction);
                } else {
                    target_base(player, _control_direction);
                }
            }
            return;
          case Gamepad::Y:
            if (_gamepad_state & SELECT_BUMPER) {
                player->keysDown |= kGiveCommandKey;
            } else {
                engage_autopilot();
            }
            return;
          case Gamepad::LSB:
            if (_gamepad_state & TARGET_BUMPER) {
                target_self();
            } else {
                MiniComputerExecute(3, 1, globals()->gPlayerAdmiralNumber);
            }
            return;
        }
    }

    switch (event.button) {
      case Gamepad::A:
        _gamepad_keys |= kUpKey;
        break;
      case Gamepad::B:
        _gamepad_keys |= kDownKey;
        break;
      case Gamepad::X:
        zoom_out();
        break;
      case Gamepad::Y:
        zoom_in();
        break;
      case Gamepad::BACK:
        Messages::advance();
        break;
      case Gamepad::LT:
        _gamepad_keys |= kEnterKey;
        break;
      case Gamepad::RT:
        _gamepad_keys |= kOneKey;
        _gamepad_keys |= kTwoKey;
        break;
      case Gamepad::LSB:
        if (player->presenceState == kWarpingPresence) {
            _gamepad_keys &= !kWarpKey;
        } else {
            _gamepad_keys |= kWarpKey;
        }
        break;
      case Gamepad::UP:
        minicomputer_handle_keys(kCompUpKey, 0, false);
        break;
      case Gamepad::DOWN:
        minicomputer_handle_keys(kCompDownKey, 0, false);
        break;
      case Gamepad::RIGHT:
        minicomputer_handle_keys(kCompAcceptKey, 0, false);
        break;
      case Gamepad::LEFT:
        minicomputer_handle_keys(kCompCancelKey, 0, false);
        break;
    }
}

void PlayerShip::gamepad_button_up(const GamepadButtonUpEvent& event) {
    switch (event.button) {
      case Gamepad::LB:
        if (_gamepad_state & OVERRIDE) {
            _gamepad_state = SELECT_BUMPER;
        } else {
            _gamepad_state = NO_BUMPER;
        }
        return;
      case Gamepad::RB:
        if (_gamepad_state & OVERRIDE) {
            _gamepad_state = TARGET_BUMPER;
        } else {
            _gamepad_state = NO_BUMPER;
        }
        return;
    }

    if (!active()) {
        return;
    }

    if (_gamepad_state) {
        switch (event.button) {
          case Gamepad::A:
          case Gamepad::B:
          case Gamepad::X:
          case Gamepad::Y:
          case Gamepad::LSB:
            return;
        }
    }

    spaceObjectType* player = mGetSpaceObjectPtr(globals()->gPlayerShipNumber);
    switch (event.button) {
      case Gamepad::A:
        _gamepad_keys &= ~kUpKey;
        break;
      case Gamepad::B:
        _gamepad_keys &= ~kDownKey;
        break;
      case Gamepad::LT:
        _gamepad_keys &= ~kEnterKey;
        break;
      case Gamepad::RT:
        _gamepad_keys &= ~kOneKey;
        _gamepad_keys &= ~kTwoKey;
        break;
      case Gamepad::LSB:
        if (player->presenceState != kWarpingPresence) {
            _gamepad_keys &= !kWarpKey;
        }
        break;
      case Gamepad::RIGHT:
        minicomputer_handle_keys(0, kCompAcceptKey, false);
        break;
      case Gamepad::LEFT:
        minicomputer_handle_keys(0, kCompCancelKey, false);
        break;
    }
}

void PlayerShip::gamepad_stick(const GamepadStickEvent& event) {
    bool active;
    int direction = 0;
    if ((event.x * event.x + event.y * event.y) < 0.90) {
        active = false;
    } else {
        active = true;
        direction = GetAngleFromVector(event.x * 32768, event.y * 32768);
        mAddAngle(direction, ROT_180);
    }
    switch (event.stick) {
      case Gamepad::LS:
        _control_active = active;
        _control_direction = direction;
        break;
    }
}

bool PlayerShip::active() const {
    if (globals()->gPlayerShipNumber < 0) {
        return false;
    }
    spaceObjectType* player = mGetSpaceObjectPtr(globals()->gPlayerShipNumber);
    if (!(player->active && (player->attributes & kIsHumanControlled))) {
        return false;
    }
    return true;
}

void PlayerShip::update(int64_t timePass, const GameCursor& cursor, bool enter_message) {
    spaceObjectType *theShip = NULL, *selectShip = NULL;
    baseObjectType  *baseObject = NULL;
    uint32_t        attributes;

    if (globals()->gPlayerShipNumber < 0) {
        return;
    }

    if (enter_message) {
        gTheseKeys = 0;
    }

    /*
    while ((globals()->gKeyMapBufferBottom != globals()->gKeyMapBufferTop)) {
        bufMap = globals()->gKeyMapBuffer + globals()->gKeyMapBufferBottom;
        globals()->gKeyMapBufferBottom++;
        if (globals()->gKeyMapBufferBottom >= kKeyMapBufferNum) {
            globals()->gKeyMapBufferBottom = 0;
        }
        if (*enterMessage) {
            String* message = Labels::get_string(gSendMessageLabel);
            if (message->empty()) {
                message->assign("<>");
            }
            if ((mReturnKey(*bufMap)) && (!AnyKeyButThisOne(*bufMap, Keys::RETURN))) {
                *enterMessage = false;
                StringSlice sliced = message->slice(1, message->size() - 2);
                int cheat = GetCheatNumFromString(sliced);
                if (cheat > 0) {
                    ExecuteCheat(cheat, globals()->gPlayerAdmiralNumber);
                } else if (!sliced.empty()) {
                    if (globals()->gActiveCheats[globals()->gPlayerAdmiralNumber] & kNameObjectBit)
                    {
                        SetAdmiralBuildAtName(globals()->gPlayerAdmiralNumber, sliced);
                        globals()->gActiveCheats[globals()->gPlayerAdmiralNumber] &= ~kNameObjectBit;
                    }
                }
                Labels::set_position(
                        gSendMessageLabel,
                        viewport.left + ((viewport.width() / 2)),
                        viewport.top + ((play_screen.height() / 2)) +
                        kSendMessageVOffset);
                Labels::recalc_size(gSendMessageLabel);
            } else {
                if ((mDeleteKey(*bufMap)) || (mLeftArrowKey(*bufMap))) {
                    if (message->size() > 2) {
                        if (mCommandKey(*bufMap)) {
                            // delete whole message
                            message->assign("<>");
                        } else {
                            message->resize(message->size() - 2);
                            message->append(1, '>');
                        }
                    }
                } else {
                    if (message->size() < 120) {
                        uint8_t ch = GetAsciiFromKeyMap(*bufMap, globals()->gLastMessageKeyMap);
                        if (ch) {
                            message->resize(message->size() - 1);
                            String s(macroman::decode(BytesSlice(&ch, 1)));
                            message->append(s);
                            message->append(1, '>');
                        }
                    }
                }
                width = tactical_font->string_width(*message);
                strlen = viewport.left + ((viewport.width() / 2) - (width / 2));
                if ((strlen + width) > (viewport.right))
                {
                    strlen -= (strlen + width) - (viewport.right);
                }
                Labels::recalc_size(gSendMessageLabel);
                Labels::set_position(gSendMessageLabel, strlen, viewport.top +
                    ((play_screen.height() / 2) + kSendMessageVOffset));
            }
        } else {
            if ((mReturnKey(*bufMap)) && (!(globals()->keyMask & kReturnKeyMask))) {
                *enterMessage = true;
            }
        }
        globals()->gLastMessageKeyMap.copy(*bufMap);
    }
    */

    // TERRIBLE HACK:
    //  this implements the often requested feature of having a shortcut for
    //  transfering control.

    theShip = mGetSpaceObjectPtr(globals()->gPlayerShipNumber);

    if (!theShip->active) {
        return;
    }

    if (theShip->health < (theShip->baseType->health >> 2L)) {
         if (gAlarmCount < 0) {
            PlayVolumeSound(kKlaxon, kMaxSoundVolume, kLongPersistence, kMustPlaySound);
            gAlarmCount = 0;
            Messages::set_status("WARNING: Shields Low", kStatusWarnColor);
         } else {
            gAlarmCount += timePass;
            if (gAlarmCount > 125) {
                PlayVolumeSound(kKlaxon, kMediumVolume, kMediumLongPersistence, kPrioritySound);
                gAlarmCount = 0;
                Messages::set_status("WARNING: Shields Low", kStatusWarnColor);
            }
        }
    } else {
        gAlarmCount = -1;
    }

    if (!(theShip->attributes & kIsHumanControlled)) {
        return;
    }

    baseObject = theShip->baseType;

    minicomputer_handle_keys(gTheseKeys, gLastKeys, false);

    if ((mMessageNextKey(_keys))
            && (!(mMessageNextKey(gLastKeyMap)))
            && (!enter_message)) {
        Messages::advance();
    }

    uint32_t dcalc = kSelectFriendKey | kSelectFoeKey | kSelectBaseKey;
    attributes = gTheseKeys & dcalc;

    if (gTheseKeys & kDestinationKey) {
        if (gDestKeyTime >= 0) {
            gDestKeyTime += timePass;
        }
    } else {
        if (gDestKeyTime > 45) {
            if ((theShip->attributes & kCanBeDestination)
                    && (!globals()->destKeyUsedForSelection)) {
                target_self();
            }
        }
        gDestKeyTime = 0;
        globals()->destKeyUsedForSelection = false;
    }

// NEW -- do hot key selection
    int hot_key = -1;
    for (int i = 0; i < kHotKeyNum; i++) {
        if (mCheckKeyMap(_keys, kFirstHotKeyNum + i)) {
            hot_key = i;
        }
    }

    if (hot_key >= 0) {
        if (hot_key != globals()->lastHotKey) {
            globals()->lastHotKey = hot_key;
            globals()->hotKeyDownTime = 0;
            globals()->hotKey_target = false;
            if (gTheseKeys & kDestinationKey) {
                globals()->hotKey_target = true;
            }
        } else {
            globals()->hotKeyDownTime += timePass;
        }
    } else if (globals()->lastHotKey >= 0) {
        hot_key = globals()->lastHotKey;
        globals()->lastHotKey = -1;

        if (globals()->hotKeyDownTime > 45) {
            if (globals()->lastSelectedObject >= 0) {
                selectShip = mGetSpaceObjectPtr(globals()->lastSelectedObject);

                if (selectShip->active) {
                    globals()->hotKey[hot_key].objectNum = globals()->lastSelectedObject;

                    globals()->hotKey[hot_key].objectID = globals()->lastSelectedObjectID;
                    Update_LabelStrings_ForHotKeyChange();
                    PlayVolumeSound(
                            kComputerBeep1, kMediumLoudVolume, kMediumPersistence,
                            kLowPrioritySound);
                }
            }
        } else {
            globals()->destKeyUsedForSelection = true;
            if (globals()->hotKey[hot_key].objectNum >= 0) {
                selectShip = mGetSpaceObjectPtr(globals()->hotKey[hot_key].objectNum);
                if ((selectShip->active)
                        && (selectShip->id == globals()->hotKey[hot_key].objectID)) {
                    bool is_target = (gTheseKeys & kDestinationKey)
                        || (selectShip->owner != globals()->gPlayerAdmiralNumber)
                        || (globals()->hotKey_target);
                    SetPlayerSelectShip(
                            globals()->hotKey[hot_key].objectNum,
                            is_target,
                            globals()->gPlayerAdmiralNumber);
                } else {
                    globals()->hotKey[hot_key].objectNum = -1;
                }
            }
            globals()->hotKeyDownTime = 0;
        }
    }
// end new hotkey selection

    // for this we check lastKeys against theseKeys & relevent keys now being pressed
    if ((attributes) && (!(gLastKeys & attributes)) && (!cursor.active())) {
        gDestKeyTime = -1;
        if (gTheseKeys & kSelectFriendKey) {
            if (!(gTheseKeys & kDestinationKey)) {
                select_friendly(theShip, theShip->direction);
            } else {
                target_friendly(theShip, theShip->direction);
            }
        } else if (gTheseKeys & kSelectFoeKey) {
            target_hostile(theShip, theShip->direction);
        } else {
            if (!(gTheseKeys & kDestinationKey)) {
                select_base(theShip, theShip->direction);
            } else {
                target_base(theShip, theShip->direction);
            }
        }
    }

    if (theShip->attributes & kOnAutoPilot) {
        if ((globals()->gAutoPilotOff) && // no off request pending
                ((gTheseKeys | _gamepad_keys) & (kUpKey | kDownKey | kLeftKey | kRightKey))) {
            theShip->keysDown = gTheseKeys | kAutoPilotKey;
            globals()->gAutoPilotOff = false;
        } else {
            theShip->keysDown
                = (theShip->keysDown & (~kMiscKeyMask))
                | (gTheseKeys & (kMiscKeyMask));
        }
    } else {
        theShip->keysDown = gTheseKeys | _gamepad_keys;
        globals()->gAutoPilotOff = true;

        if ((_gamepad_state == NO_BUMPER) && _control_active) {
            int difference = mAngleDifference(_control_direction, theShip->direction);
            if (abs(difference) < 15) {
                // pass
            } else if (difference < 0) {
                theShip->keysDown |= kRightKey;
            } else {
                theShip->keysDown |= kLeftKey;
            }
        }
    }

    if ((gTheseKeys & kOrderKey) && (!(gLastKeys & kOrderKey))) {
        theShip->keysDown |= kGiveCommandKey;
    }

    if ((gTheseKeys & kWarpKey)
            && (gTheseKeys & kDestinationKey)) {
        gDestKeyTime = -1;
        if (!(gLastKeys & kWarpKey)) {
            engage_autopilot();
        }
        theShip->keysDown &= ~kWarpKey;
    }

    gLastKeyMap.copy(_keys);
    gLastKeys = gTheseKeys;
}

bool PlayerShip::show_select() const {
    return _control_active && (_gamepad_state & SELECT_BUMPER);
}

bool PlayerShip::show_target() const {
    return _control_active && (_gamepad_state & TARGET_BUMPER);
}

int32_t PlayerShip::control_direction() const {
    return _control_direction;
}

bool PlayerShip::show_right_stick() const {
    return false;
}

int32_t PlayerShip::goal_direction() const {
    return 0;
}

void PlayerShipHandleClick(Point where, int button) {
    spaceObjectType *theShip = NULL;
    int32_t         selectShipNum;
    Rect            bounds;

    if (globals()->keyMask & kMouseMask) {
        return;
    }

    gDestKeyTime = -1;
    if (globals()->gPlayerShipNumber >= 0) {
        theShip = mGetSpaceObjectPtr(globals()->gPlayerShipNumber);
        if ((theShip->active) && (theShip->attributes & kIsHumanControlled)) {
            bounds.left = where.h - kCursorBoundsSize;
            bounds.top = where.v - kCursorBoundsSize;
            bounds.right = where.h + kCursorBoundsSize;
            bounds.bottom = where.v + kCursorBoundsSize;

            if ((theShip->keysDown & kDestinationKey) || (button == 1)) {
                selectShipNum = GetAdmiralDestinationObject(globals()->gPlayerAdmiralNumber);

                selectShipNum = GetSpritePointSelectObject(
                        &bounds, theShip, kCanBeDestination | kIsDestination,
                        selectShipNum, FRIENDLY_OR_HOSTILE);
                if (selectShipNum >= 0) {
                    SetPlayerSelectShip(selectShipNum, true, globals()->gPlayerAdmiralNumber);
                }
            } else {
                selectShipNum = GetAdmiralConsiderObject(globals()->gPlayerAdmiralNumber);
                selectShipNum = GetSpritePointSelectObject(
                        &bounds, theShip, kCanBeDestination | kCanAcceptBuild,
                        selectShipNum, FRIENDLY);
                if (selectShipNum >= 0) {
                    SetPlayerSelectShip(selectShipNum, false, globals()->gPlayerAdmiralNumber);
                }
            }
        }
    }
}

void SetPlayerSelectShip( int32_t whichShip, bool target, int32_t admiralNumber)
{
    spaceObjectType *selectShip = mGetSpaceObjectPtr(whichShip),
                    *theShip = GetAdmiralFlagship( admiralNumber);

    if ( admiralNumber == globals()->gPlayerAdmiralNumber)
    {
        globals()->lastSelectedObject = whichShip;
        globals()->lastSelectedObjectID = selectShip->id;
        globals()->destKeyUsedForSelection = true;
    }
    if (target) {
        SetAdmiralDestinationObject( admiralNumber, whichShip, kObjectDestinationType);
        if (admiralNumber == globals()->gPlayerAdmiralNumber) {
            Labels::set_object( gDestinationLabel, selectShip);
            if (whichShip == globals()->gPlayerShipNumber) {
                Labels::set_age(gDestinationLabel, Labels::kVisibleTime);
            }
            PlayVolumeSound(
                    kComputerBeep1, kMediumLoudVolume, kMediumPersistence, kLowPrioritySound);
            if (selectShip->attributes & kIsDestination) {
                String string(GetDestBalanceName(selectShip->destinationObject));
                print(string, hot_key_suffix(selectShip));
                Labels::set_string(gDestinationLabel, string);
            } else {
                String string(get_object_name(selectShip->whichBaseObject));
                print(string, hot_key_suffix(selectShip));
                Labels::set_string(gDestinationLabel, string);
            }
        }

        if (!(theShip->attributes & kOnAutoPilot)) {
            SetObjectDestination(theShip, NULL);
        }
    } else {
        SetAdmiralConsiderObject(admiralNumber, whichShip);
        if (admiralNumber == globals()->gPlayerAdmiralNumber) {
            Labels::set_object(globals()->gSelectionLabel, selectShip);
            if (whichShip == globals()->gPlayerShipNumber) {
                Labels::set_age(globals()->gSelectionLabel, Labels::kVisibleTime);
            }
            PlayVolumeSound(
                    kComputerBeep1, kMediumLoudVolume, kMediumPersistence, kLowPrioritySound);
            if (selectShip->attributes & kIsDestination) {
                String string(GetDestBalanceName(selectShip->destinationObject));
                print(string, hot_key_suffix(selectShip));
                Labels::set_string(globals()->gSelectionLabel, string);
            } else {
                String string(get_object_name(selectShip->whichBaseObject));
                print(string, hot_key_suffix(selectShip));
                Labels::set_string(globals()->gSelectionLabel, string);
            }
        }
    }
}

// ChangePlayerShipNumber()
// assumes that newShipNumber is the number of a valid (legal, living) ship and that
// gPlayerShip already points to the current, legal living ship

void ChangePlayerShipNumber( int32_t whichAdmiral, int32_t newShipNumber)
{
    spaceObjectType *anObject = GetAdmiralFlagship( whichAdmiral);
    if (anObject == NULL) {
        throw Exception(format(
                    "whichAdmiral: {0}, newShipNumber: {1}",
                    whichAdmiral, newShipNumber));
    }

    if ( whichAdmiral == globals()->gPlayerAdmiralNumber)
    {
        anObject->attributes &= (~kIsHumanControlled) & (~kIsPlayerShip);
        if ( newShipNumber != globals()->gPlayerShipNumber)
        {
            globals()->gPlayerShipNumber = newShipNumber;
            globals()->starfield.reset(globals()->gPlayerShipNumber);
        }


        anObject = mGetSpaceObjectPtr(globals()->gPlayerShipNumber);
        if (anObject == NULL) {
            throw Exception(format(
                        "whichAdmiral: {0}, newShipNumber: {1}, gPlayerShipNumber: {2}",
                        whichAdmiral, newShipNumber, globals()->gPlayerShipNumber));
        }

//      if ( !(globals()->gActiveCheats[whichAdmiral] & kAutoPlayBit))
            anObject->attributes |= (kIsHumanControlled) | (kIsPlayerShip);
//      else
//          anObject->attributes |= kIsPlayerShip;

        if ( newShipNumber == GetAdmiralConsiderObject( globals()->gPlayerAdmiralNumber))
        {
            Labels::set_age( globals()->gSelectionLabel, Labels::kVisibleTime);
        }
        if ( newShipNumber == GetAdmiralDestinationObject( globals()->gPlayerAdmiralNumber))
        {
            Labels::set_age( gDestinationLabel, Labels::kVisibleTime);
        }
    } else
    {
        anObject->attributes &= ~(kIsRemote | kIsPlayerShip);
        anObject = mGetSpaceObjectPtr(newShipNumber);
        anObject->attributes |= (kIsRemote | kIsPlayerShip);
    }
    SetAdmiralFlagship( whichAdmiral, newShipNumber);
}

void TogglePlayerAutoPilot(spaceObjectType *theShip) {
    if ( theShip->attributes & kOnAutoPilot)
    {
        theShip->attributes &= ~kOnAutoPilot;
        if ((theShip->owner == globals()->gPlayerAdmiralNumber) &&
            ( theShip->attributes & kIsHumanControlled))
        {
            StringList strings(kMessageStringID);
            StringSlice string = strings.at(kAutoPilotOffString - 1);
            Messages::set_status(string, kStatusLabelColor);
        }
    } else
    {
        SetObjectDestination( theShip, NULL);
        theShip->attributes |= kOnAutoPilot;
        if ((theShip->owner == globals()->gPlayerAdmiralNumber) &&
            ( theShip->attributes & kIsHumanControlled))
        {
            StringList strings(kMessageStringID);
            StringSlice string = strings.at(kAutoPilotOnString - 1);
            Messages::set_status(string, kStatusLabelColor);
        }
    }
}

bool IsPlayerShipOnAutoPilot( void)
{
    spaceObjectType *theShip;

    if ( globals()->gPlayerShipNumber < 0) return false;
    theShip = mGetSpaceObjectPtr(globals()->gPlayerShipNumber);
    if ( theShip->attributes & kOnAutoPilot) return true;
    else return false;
}

void PlayerShipGiveCommand( int32_t whichAdmiral)
{
    spaceObjectType *selectShip;
    int32_t selectShipNum = GetAdmiralConsiderObject( whichAdmiral);

    if ( selectShipNum >= 0)
    {
        selectShip = mGetSpaceObjectPtr(selectShipNum);
        SetObjectDestination( selectShip, NULL);
        if ( whichAdmiral == globals()->gPlayerAdmiralNumber)
            PlayVolumeSound(  kMorseBeepSound, kMediumVolume, kMediumPersistence, kLowPrioritySound);
    }
}

// bool sourceIsBody was hacked in to use this for xferring control
void PlayerShipBodyExpire( spaceObjectType *theShip, bool sourceIsBody)
{
    spaceObjectType *selectShip = NULL;
    int32_t         selectShipNum;

    selectShipNum = GetAdmiralConsiderObject( theShip->owner);

    if ( selectShipNum >= 0)
    {
        selectShip = mGetSpaceObjectPtr(selectShipNum);
        if (( selectShip->active != kObjectInUse) ||
            ( !(selectShip->attributes & kCanThink)) ||
            ( selectShip->attributes & kStaticDestination)
            || ( selectShip->owner != theShip->owner) ||
            (!(selectShip->attributes & kCanAcceptDestination))
            )
            selectShip = NULL;
    }
    if ( selectShip == NULL)
    {
//      selectShip = gSpaceObjectData.get();
//      selectShipNum = 0;
        selectShip = gRootObject;
        selectShipNum = gRootObjectNumber;
        while ( ( selectShip != NULL) &&
                (
                    ( selectShip->active != kObjectInUse)
                    ||
                    ( selectShip->attributes & kStaticDestination)
                    ||
                    (
                        !(
                            (selectShip->attributes & kCanThink) &&
                            (selectShip->attributes & kCanAcceptDestination)
                        )
                    )
                    ||
                    ( selectShip->owner != theShip->owner)
                )
            )
        {
            selectShipNum = selectShip->nextObjectNumber;
            selectShip = selectShip->nextObject;
        }
    }
    if (( selectShip == NULL) && ( sourceIsBody))
    {
        if ( globals()->gGameOver >= 0)
        {
            globals()->gGameOver = -180;
        }
        if (theShip->owner == globals()->gPlayerAdmiralNumber) {
            globals()->gScenarioWinner.text = kScenarioNoShipTextID + gThisScenario->levelNameStrNum;
        } else {
            globals()->gScenarioWinner.text = 10050 + gThisScenario->levelNameStrNum;
        }
        SetAdmiralFlagship( theShip->owner, -1);
    } else if ( selectShip != NULL)
    {
        ChangePlayerShipNumber( theShip->owner, selectShipNum);
    }
}

void HandleTextMessageKeys(const KeyMap& keyMap, const KeyMap& lastKeyMap, bool *enterMessage) {
    bool         newKeys = false, anyKeys = false;
    KeyMap          *bufferMap;

    newKeys = (lastKeyMap != keyMap);
    anyKeys = keyMap.any();

    if ( newKeys)
    {
        if (( *enterMessage) && anyKeys)
            PlayVolumeSound(  kTeletype, kMediumLowVolume, kShortPersistence, kLowPrioritySound);
        bufferMap = globals()->gKeyMapBuffer + globals()->gKeyMapBufferTop;
        bufferMap->copy(keyMap);
        if ( mReturnKey( keyMap))
        {
            if ( *enterMessage) *enterMessage = false;
            else *enterMessage = true;
        }
        globals()->gKeyMapBufferTop++;
        if ( globals()->gKeyMapBufferTop >= kKeyMapBufferNum)
            globals()->gKeyMapBufferTop = 0;
    }
}

int32_t HotKey_GetFromObject( spaceObjectType *object)
{
    int32_t i = 0;

    if ( object == NULL) return -1;
    if ( !object->active) return -1;
    while ( i < kHotKeyNum)
    {
        if ( globals()->hotKey[i].objectNum == object->number())
        {
            if ( globals()->hotKey[i].objectID == object->id)
            {
                return i;
            }
        }
        i++;
    }
    return -1;
}

void Update_LabelStrings_ForHotKeyChange( void)
{
    spaceObjectType *selectShip;
    int32_t         whichShip;

    whichShip = GetAdmiralDestinationObject( globals()->gPlayerAdmiralNumber);
    if (whichShip >= 0) {
        selectShip = mGetSpaceObjectPtr(whichShip);

//      if ( admiralNumber == globals()->gPlayerAdmiralNumber)
        {
            Labels::set_object( gDestinationLabel, selectShip);
            if (whichShip == globals()->gPlayerShipNumber) {
                Labels::set_age(gDestinationLabel, Labels::kVisibleTime);
            }
            if (selectShip->attributes & kIsDestination) {
                String string(GetDestBalanceName(selectShip->destinationObject));
                print(string, hot_key_suffix(selectShip));
                Labels::set_string(gDestinationLabel, string);
            } else {
                String string(get_object_name(selectShip->whichBaseObject));
                print(string, hot_key_suffix(selectShip));
                Labels::set_string(gDestinationLabel, string);
            }
        }
    }

    whichShip = GetAdmiralConsiderObject( globals()->gPlayerAdmiralNumber);
    if (whichShip >= 0) {
        selectShip = mGetSpaceObjectPtr(whichShip);
//      if ( admiralNumber == globals()->gPlayerAdmiralNumber)
        {
            Labels::set_object( globals()->gSelectionLabel, selectShip);
            if (whichShip == globals()->gPlayerShipNumber) {
                Labels::set_age(globals()->gSelectionLabel, Labels::kVisibleTime);
            }
            PlayVolumeSound(
                    kComputerBeep1, kMediumLoudVolume, kMediumPersistence, kLowPrioritySound);
            if (selectShip->attributes & kIsDestination) {
                String string(GetDestBalanceName(selectShip->destinationObject));
                print(string, hot_key_suffix(selectShip));
                Labels::set_string(globals()->gSelectionLabel, string);
            } else {
                String string(get_object_name(selectShip->whichBaseObject));
                print(string, hot_key_suffix(selectShip));
                Labels::set_string(globals()->gSelectionLabel, string);
            }
        }
    }
}

}  // namespace antares
