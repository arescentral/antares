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

#include "game/player-ship.hpp"

#include <sfz/sfz.hpp>

#include "config/gamepad.hpp"
#include "config/keys.hpp"
#include "config/preferences.hpp"
#include "data/base-object.hpp"
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
#include "game/level.hpp"
#include "game/messages.hpp"
#include "game/minicomputer.hpp"
#include "game/non-player-ship.hpp"
#include "game/space-object.hpp"
#include "game/starfield.hpp"
#include "game/sys.hpp"
#include "game/time.hpp"
#include "lang/defines.hpp"
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

// const int32_t kSendMessageVOffset = 20;
const int32_t kCursorBoundsSize = 16;  // should be same in instruments.c

}  // namespace

const static ticks kKlaxonInterval      = ticks(125);
const static ticks kDestKeyHoldDuration = ticks(45);
const static ticks kHotKeyHoldDuration  = ticks(51);  // Compatibility

int32_t HotKey_GetFromObject(Handle<SpaceObject> object);
void Update_LabelStrings_ForHotKeyChange(void);

namespace {

enum DestKeyState {
    DEST_KEY_UP,       // up
    DEST_KEY_DOWN,     // down, and possibly usable for self-selection
    DEST_KEY_BLOCKED,  // down, but used for something else
};

enum HotKeyState {
    HOT_KEY_UP,
    HOT_KEY_SELECT,
    HOT_KEY_TARGET,
};

static ANTARES_GLOBAL DestKeyState gDestKeyState = DEST_KEY_UP;
static ANTARES_GLOBAL wall_time gDestKeyTime;

static ANTARES_GLOBAL HotKeyState gHotKeyState = HOT_KEY_UP;
static ANTARES_GLOBAL wall_time gHotKeyTime;
static ANTARES_GLOBAL int       gHotKeyNum;

static ANTARES_GLOBAL ZoomType gPreviousZoomMode;

struct HotKeySuffix {
    Handle<SpaceObject> space_object;
};

void print_to(PrintTarget out, const HotKeySuffix& suffix) {
    int h = HotKey_GetFromObject(suffix.space_object);
    if (h < 0) {
        return;
    }

    int keyNum = sys.prefs->key(h + kFirstHotKeyNum);
    if (keyNum < 0) {
        return;
    }

    StringList  strings(KEY_LONG_NAMES);
    StringSlice key_name = strings.at(keyNum - 1);
    print(out, format(" < {0} >", key_name));
};

HotKeySuffix hot_key_suffix(Handle<SpaceObject> space_object) {
    HotKeySuffix result = {space_object};
    return result;
};

}  // namespace

void ResetPlayerShip(Handle<SpaceObject> which) {
    g.ship          = which;
    g.control_label = Label::add(0, 0, 0, 10, SpaceObject::none(), true, YELLOW);
    g.target_label  = Label::add(0, 0, 0, -20, SpaceObject::none(), true, SKY_BLUE);
    g.send_label    = Label::add(200, 200, 0, 30, SpaceObject::none(), false, GREEN);
    globals()->starfield.reset(g.ship);
    globals()->next_klaxon = game_ticks();
    g.key_mask             = 0;
    g.zoom                 = kNearestFoeZoom;
    gPreviousZoomMode      = kNearestFoeZoom;

    for (int h = 0; h < kHotKeyNum; h++) {
        globals()->hotKey[h].object   = SpaceObject::none();
        globals()->hotKey[h].objectID = -1;
    }
    gHotKeyState  = HOT_KEY_UP;
    gDestKeyState = DEST_KEY_UP;
}

PlayerShip::PlayerShip()
        : gTheseKeys(0),
          _gamepad_keys(0),
          _key_presses(0),
          _key_releases(0),
          _gamepad_state(NO_BUMPER),
          _control_active(false),
          _control_direction(0) {}

void PlayerShip::update_keys(const KeyMap& keys) {
    for (int i = 0; i < 256; ++i) {
        if (keys.get(i) && !_keys.get(i)) {
            key_down(KeyDownEvent(now(), i));
        } else if (_keys.get(i) && !keys.get(i)) {
            key_up(KeyUpEvent(now(), i));
        }
    }
}

static int key_num(uint32_t key) {
    for (int i = 0; i < kKeyExtendedControlNum; ++i) {
        if (key == (sys.prefs->key(i) - 1)) {
            return i;
        }
    }
    return -1;
}

static void zoom_to(ZoomType zoom) {
    if (g.zoom != zoom) {
        g.zoom = zoom;
        sys.sound.click();
        StringList  strings(kMessageStringID);
        StringSlice string = strings.at(g.zoom + kZoomStringOffset - 1);
        Messages::set_status(string, kStatusLabelColor);
    }
}

static void zoom_shortcut(ZoomType zoom) {
    if (g.key_mask & kShortcutZoomMask) {
        return;
    }
    ZoomType previous = gPreviousZoomMode;
    gPreviousZoomMode = g.zoom;
    if (g.zoom == zoom) {
        zoom_to(previous);
    } else {
        zoom_to(zoom);
    }
}

static void zoom_in() {
    if (g.key_mask & kZoomInKey) {
        return;
    }
    if (g.zoom > kTimesTwoZoom) {
        zoom_to(static_cast<ZoomType>(g.zoom - 1));
    }
}

static void zoom_out() {
    if (g.key_mask & kZoomOutKey) {
        return;
    }
    if (g.zoom < kSmallestZoom) {
        zoom_to(static_cast<ZoomType>(g.zoom + 1));
    }
}

static void engage_autopilot() {
    auto player = g.ship;
    if (!(player->attributes & kOnAutoPilot)) {
        player->keysDown |= kAutoPilotKey;
    }
    player->keysDown |= kAdoptTargetKey;
}

static void pick_object(
        Handle<SpaceObject> origin_ship, int32_t direction, bool destination, int32_t attributes,
        int32_t nonattributes, Handle<SpaceObject> select_ship, Allegiance allegiance) {
    uint64_t huge_distance;
    if (select_ship.get()) {
        uint32_t difference = ABS<int>(origin_ship->location.h - select_ship->location.h);
        uint32_t dcalc      = difference;
        difference          = ABS<int>(origin_ship->location.v - select_ship->location.v);
        uint32_t distance   = difference;

        if ((dcalc > kMaximumRelevantDistance) || (distance > kMaximumRelevantDistance)) {
            huge_distance = dcalc;  // must be positive
            MyWideMul(huge_distance, huge_distance, &huge_distance);
            select_ship->distanceFromPlayer = distance;
            MyWideMul(
                    select_ship->distanceFromPlayer, select_ship->distanceFromPlayer,
                    &select_ship->distanceFromPlayer);
            select_ship->distanceFromPlayer += huge_distance;
        } else {
            select_ship->distanceFromPlayer = distance * distance + dcalc * dcalc;
        }
        huge_distance = select_ship->distanceFromPlayer;
    } else {
        huge_distance = 0;
    }

    select_ship = GetManualSelectObject(
            origin_ship, direction, attributes, nonattributes, &huge_distance, select_ship,
            allegiance);

    if (select_ship.get()) {
        if (destination) {
            SetPlayerSelectShip(select_ship, true, g.admiral);
        } else {
            SetPlayerSelectShip(select_ship, false, g.admiral);
        }
    }
}

static void select_friendly(Handle<SpaceObject> origin_ship, int32_t direction) {
    pick_object(
            origin_ship, direction, false, kCanBeDestination, kIsDestination, g.admiral->control(),
            FRIENDLY);
}

static void target_friendly(Handle<SpaceObject> origin_ship, int32_t direction) {
    pick_object(
            origin_ship, direction, true, kCanBeDestination, kIsDestination, g.admiral->target(),
            FRIENDLY);
}

static void target_hostile(Handle<SpaceObject> origin_ship, int32_t direction) {
    pick_object(
            origin_ship, direction, true, kCanBeDestination, kIsDestination, g.admiral->target(),
            HOSTILE);
}

static void select_base(Handle<SpaceObject> origin_ship, int32_t direction) {
    pick_object(origin_ship, direction, false, kCanAcceptBuild, 0, g.admiral->control(), FRIENDLY);
}

static void target_base(Handle<SpaceObject> origin_ship, int32_t direction) {
    pick_object(
            origin_ship, direction, true, kIsDestination, 0, g.admiral->target(),
            FRIENDLY_OR_HOSTILE);
}

static void target_self() {
    SetPlayerSelectShip(g.ship, true, g.admiral);
}

void PlayerShip::key_down(const KeyDownEvent& event) {
    _keys.set(event.key(), true);

    if (!active()) {
        return;
    }

    int key = key_num(event.key());
    switch (key) {
        case kZoomOutKeyNum: zoom_out(); break;
        case kZoomInKeyNum: zoom_in(); break;
        case kScale121KeyNum: zoom_shortcut(kActualSizeZoom); break;
        case kScale122KeyNum: zoom_shortcut(kHalfSizeZoom); break;
        case kScale124KeyNum: zoom_shortcut(kQuarterSizeZoom); break;
        case kScale1216KeyNum: zoom_shortcut(kEighthSizeZoom); break;
        case kScaleHostileKeyNum: zoom_shortcut(kNearestFoeZoom); break;
        case kScaleObjectKeyNum: zoom_shortcut(kNearestAnythingZoom); break;
        case kScaleAllKeyNum: zoom_shortcut(kSmallestZoom); break;
        case kTransferKeyNum: transfer_control(g.admiral, 0); break;
        case kMessageNextKeyNum: Messages::advance(); break;
        default:
            if (key < kKeyControlNum) {
                _key_presses |= ((1 << key) & ~g.key_mask);
                _key_releases &= ~((1 << key) & ~g.key_mask);
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
                _key_releases |= ((1 << key) & ~g.key_mask);
                _key_presses &= ~((1 << key) & ~g.key_mask);
            }
            break;
    }
}

void PlayerShip::mouse_down(const MouseDownEvent& event) {
    _cursor.mouse_down(event);

    switch (event.button()) {
        case 0:
            if (event.count() == 2) {
                InstrumentsHandleDoubleClick(_cursor);
            } else if (event.count() == 1) {
                InstrumentsHandleClick(_cursor);
            }
            break;
        case 1:
            if (event.count() == 1) {
                PlayerShipHandleClick(event.where(), 1);
            }
            break;
    }
}

void PlayerShip::mouse_up(const MouseUpEvent& event) {
    _cursor.mouse_up(event);

    if (event.button() == 0) {
        InstrumentsHandleMouseStillDown(_cursor);
        InstrumentsHandleMouseUp(_cursor);
    }
}

void PlayerShip::mouse_move(const MouseMoveEvent& event) {
    _cursor.mouse_move(event);
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

    auto player = g.ship;
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
                    transfer_control(g.admiral, 0);
                }
                return;
        }
    }

    switch (event.button) {
        case Gamepad::A: _gamepad_keys |= kUpKey; break;
        case Gamepad::B: _gamepad_keys |= kDownKey; break;
        case Gamepad::X: zoom_out(); break;
        case Gamepad::Y: zoom_in(); break;
        case Gamepad::BACK: Messages::advance(); break;
        case Gamepad::LT: _gamepad_keys |= kEnterKey; break;
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
        case Gamepad::UP: minicomputer_handle_keys(kCompUpKey, 0); break;
        case Gamepad::DOWN: minicomputer_handle_keys(kCompDownKey, 0); break;
        case Gamepad::RIGHT: minicomputer_handle_keys(kCompAcceptKey, 0); break;
        case Gamepad::LEFT: minicomputer_handle_keys(kCompCancelKey, 0); break;
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
            case Gamepad::LSB: return;
        }
    }

    auto player = g.ship;
    switch (event.button) {
        case Gamepad::A: _gamepad_keys &= ~kUpKey; break;
        case Gamepad::B: _gamepad_keys &= ~kDownKey; break;
        case Gamepad::LT: _gamepad_keys &= ~kEnterKey; break;
        case Gamepad::RT:
            _gamepad_keys &= ~kOneKey;
            _gamepad_keys &= ~kTwoKey;
            break;
        case Gamepad::LSB:
            if (player->presenceState != kWarpingPresence) {
                _gamepad_keys &= !kWarpKey;
            }
            break;
        case Gamepad::RIGHT: minicomputer_handle_keys(0, kCompAcceptKey); break;
        case Gamepad::LEFT: minicomputer_handle_keys(0, kCompCancelKey); break;
    }
}

void PlayerShip::gamepad_stick(const GamepadStickEvent& event) {
    bool active;
    int  direction = 0;
    if ((event.x * event.x + event.y * event.y) < 0.90) {
        active = false;
    } else {
        active    = true;
        direction = GetAngleFromVector(event.x * 32768, event.y * 32768);
        mAddAngle(direction, ROT_180);
    }
    switch (event.stick) {
        case Gamepad::LS:
            _control_active    = active;
            _control_direction = direction;
            break;
    }
}

bool PlayerShip::active() const {
    auto player = g.ship;
    return player.get() && player->active && (player->attributes & kIsHumanControlled);
}

void PlayerShip::update(bool enter_message) {
    if (!g.ship.get()) {
        return;
    }

    if (enter_message) {
        _key_presses  = 0;
        _key_releases = gTheseKeys;
    }
    gTheseKeys |= _key_presses;
    gTheseKeys &= ~_key_releases;

    /*
    while ((globals()->gKeyMapBufferBottom != globals()->gKeyMapBufferTop)) {
        bufMap = globals()->gKeyMapBuffer + globals()->gKeyMapBufferBottom;
        globals()->gKeyMapBufferBottom++;
        if (globals()->gKeyMapBufferBottom >= kKeyMapBufferNum) {
            globals()->gKeyMapBufferBottom = 0;
        }
        if (*enterMessage) {
            String* message = Label::get_string(g.send_label);
            if (message->empty()) {
                message->assign("<>");
            }
            if ((mReturnKey(*bufMap)) && (!AnyKeyButThisOne(*bufMap, Keys::RETURN))) {
                *enterMessage = false;
                StringSlice sliced = message->slice(1, message->size() - 2);
                int cheat = GetCheatNumFromString(sliced);
                if (cheat > 0) {
                    ExecuteCheat(cheat, g.admiral);
                } else if (!sliced.empty()) {
                    if (globals()->gActiveCheats[g.admiral] & kNameObjectBit)
                    {
                        SetAdmiralBuildAtName(g.admiral, sliced);
                        globals()->gActiveCheats[g.admiral] &= ~kNameObjectBit;
                    }
                }
                Label::set_position(
                        g.send_label,
                        viewport.left + ((viewport.width() / 2)),
                        viewport.top + ((play_screen.height() / 2)) +
                        kSendMessageVOffset);
                Label::recalc_size(g.send_label);
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
                width = sys.fonts.tactical->string_width(*message);
                strlen = viewport.left + ((viewport.width() / 2) - (width / 2));
                if ((strlen + width) > (viewport.right))
                {
                    strlen -= (strlen + width) - (viewport.right);
                }
                Label::recalc_size(g.send_label);
                Label::set_position(g.send_label, strlen, viewport.top +
                    ((play_screen.height() / 2) + kSendMessageVOffset));
            }
        } else {
            if ((mReturnKey(*bufMap)) && (!(g.key_mask & kReturnKeyMask))) {
                *enterMessage = true;
            }
        }
        globals()->gLastMessageKeyMap.copy(*bufMap);
    }
    */

    auto theShip = g.ship;

    if (!theShip->active) {
        return;
    }

    if (theShip->health() < (theShip->baseType->health >> 2L)) {
        if (g.time > globals()->next_klaxon) {
            if (globals()->next_klaxon == game_ticks()) {
                sys.sound.loud_klaxon();
            } else {
                sys.sound.klaxon();
            }
            Messages::set_status("WARNING: Shields Low", kStatusWarnColor);
            globals()->next_klaxon = g.time + kKlaxonInterval;
        }
    } else {
        globals()->next_klaxon = game_ticks();
    }

    if (!(theShip->attributes & kIsHumanControlled)) {
        return;
    }

    minicomputer_handle_keys(_key_presses, _key_releases);

    if (gTheseKeys & kDestinationKey) {
        if (gDestKeyState == DEST_KEY_UP) {
            gDestKeyState = DEST_KEY_DOWN;
            gDestKeyTime  = now();
        }
    } else {
        if ((gDestKeyState == DEST_KEY_DOWN) && (now() >= (gDestKeyTime + kDestKeyHoldDuration)) &&
            (theShip->attributes & kCanBeDestination)) {
            target_self();
        }
        gDestKeyState = DEST_KEY_UP;
    }

    // NEW -- do hot key selection
    int hot_key = -1;
    for (int i = 0; i < kHotKeyNum; i++) {
        if (mCheckKeyMap(_keys, kFirstHotKeyNum + i)) {
            hot_key = i;
        }
    }

    if (hot_key >= 0) {
        if (gHotKeyState == HOT_KEY_UP) {
            gHotKeyTime = now();
            gHotKeyNum  = hot_key;
            if (gTheseKeys & kDestinationKey) {
                gHotKeyState = HOT_KEY_TARGET;
            } else {
                gHotKeyState = HOT_KEY_SELECT;
            }
        }
    } else if (gHotKeyState != HOT_KEY_UP) {
        hot_key      = gHotKeyNum;
        bool target  = gHotKeyState == HOT_KEY_TARGET;
        gHotKeyState = HOT_KEY_UP;

        if (now() >= gHotKeyTime + kHotKeyHoldDuration) {
            if (globals()->lastSelectedObject.get()) {
                auto selectShip = globals()->lastSelectedObject;

                if (selectShip->active) {
                    globals()->hotKey[hot_key].object   = globals()->lastSelectedObject;
                    globals()->hotKey[hot_key].objectID = globals()->lastSelectedObjectID;
                    Update_LabelStrings_ForHotKeyChange();
                    sys.sound.select();
                }
            }
        } else {
            gDestKeyState = DEST_KEY_BLOCKED;
            if (globals()->hotKey[hot_key].object.get()) {
                auto selectShip = globals()->hotKey[hot_key].object;
                if ((selectShip->active) &&
                    (selectShip->id == globals()->hotKey[hot_key].objectID)) {
                    bool is_target = (gTheseKeys & kDestinationKey) ||
                                     (selectShip->owner != g.admiral) || target;
                    SetPlayerSelectShip(globals()->hotKey[hot_key].object, is_target, g.admiral);
                } else {
                    globals()->hotKey[hot_key].object = SpaceObject::none();
                }
            }
        }
    }
    // end new hotkey selection

    // for this we check lastKeys against theseKeys & relevent keys now being pressed
    uint32_t select_keys = _key_presses & (kSelectFriendKey | kSelectFoeKey | kSelectBaseKey);
    if (select_keys && !_cursor.active()) {
        gDestKeyState = DEST_KEY_BLOCKED;
        if (_key_presses & kSelectFriendKey) {
            if (!(gTheseKeys & kDestinationKey)) {
                select_friendly(theShip, theShip->direction);
            } else {
                target_friendly(theShip, theShip->direction);
            }
        } else if (_key_presses & kSelectFoeKey) {
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
        if ((gTheseKeys | _gamepad_keys) & (kUpKey | kDownKey | kLeftKey | kRightKey)) {
            theShip->keysDown = gTheseKeys | kAutoPilotKey;
        }
    } else {
        theShip->keysDown = gTheseKeys | _gamepad_keys;
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

    if (_key_presses & kOrderKey) {
        theShip->keysDown |= kGiveCommandKey;
    }

    if ((gTheseKeys & kWarpKey) && (gTheseKeys & kDestinationKey)) {
        gDestKeyState = DEST_KEY_BLOCKED;
        if (_key_presses & kWarpKey) {
            engage_autopilot();
        }
        theShip->keysDown &= ~kWarpKey;
    }

    _key_presses  = 0;
    _key_releases = 0;
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

void PlayerShipHandleClick(Point where, int button) {
    if (g.key_mask & kMouseMask) {
        return;
    }

    gDestKeyState = DEST_KEY_BLOCKED;
    if (g.ship.get()) {
        if ((g.ship->active) && (g.ship->attributes & kIsHumanControlled)) {
            Rect bounds = {
                    where.h - kCursorBoundsSize, where.v - kCursorBoundsSize,
                    where.h + kCursorBoundsSize, where.v + kCursorBoundsSize,
            };

            if ((g.ship->keysDown & kDestinationKey) || (button == 1)) {
                auto target        = g.admiral->target();
                auto selectShipNum = GetSpritePointSelectObject(
                        &bounds, g.ship, kCanBeDestination | kIsDestination, target,
                        FRIENDLY_OR_HOSTILE);
                if (selectShipNum.get()) {
                    SetPlayerSelectShip(selectShipNum, true, g.admiral);
                }
            } else {
                auto control       = g.admiral->control();
                auto selectShipNum = GetSpritePointSelectObject(
                        &bounds, g.ship, kCanBeDestination | kCanAcceptBuild, control, FRIENDLY);
                if (selectShipNum.get()) {
                    SetPlayerSelectShip(selectShipNum, false, g.admiral);
                }
            }
        }
    }
}

void SetPlayerSelectShip(Handle<SpaceObject> ship, bool target, Handle<Admiral> adm) {
    Handle<SpaceObject> flagship = adm->flagship();
    Handle<Label>       label;

    if (adm == g.admiral) {
        globals()->lastSelectedObject   = ship;
        globals()->lastSelectedObjectID = ship->id;
        gDestKeyState                   = DEST_KEY_BLOCKED;
    }
    if (target) {
        adm->set_target(ship);
        label = g.target_label;

        if (!(flagship->attributes & kOnAutoPilot)) {
            SetObjectDestination(flagship);
        }
    } else {
        adm->set_control(ship);
        label = g.control_label;
    }

    if (adm == g.admiral) {
        sys.sound.select();
        label->set_object(ship);
        if (ship == g.ship) {
            label->set_age(Label::kVisibleTime);
        }
        String string;
        if (ship->attributes & kIsDestination) {
            string.assign(GetDestBalanceName(ship->asDestination));
        } else {
            string.assign(get_object_name(ship->base));
        }
        print(string, hot_key_suffix(ship));
        label->set_string(string);
    }
}

// ChangePlayerShipNumber()
// assumes that newShipNumber is the number of a valid (legal, living) ship and that
// gPlayerShip already points to the current, legal living ship

void ChangePlayerShipNumber(Handle<Admiral> adm, Handle<SpaceObject> newShip) {
    auto flagship = adm->flagship();
    if (!flagship.get()) {
        throw Exception(format("adm: {0}, newShip: {1}", adm.number(), newShip.number()));
    }

    if (adm == g.admiral) {
        flagship->attributes &= ~(kIsHumanControlled | kIsPlayerShip);
        if (newShip != g.ship) {
            g.ship = newShip;
            globals()->starfield.reset(newShip);
        }

        flagship = g.ship;
        if (!flagship.get()) {
            throw Exception(
                    format("adm: {0}, newShip: {1}, gPlayerShip: {2}", adm.number(),
                           newShip.number(), g.ship.number()));
        }

        flagship->attributes |= kIsHumanControlled | kIsPlayerShip;

        if (newShip == g.admiral->control()) {
            g.control_label->set_age(Label::kVisibleTime);
        }
        if (newShip == g.admiral->target()) {
            g.target_label->set_age(Label::kVisibleTime);
        }
    } else {
        flagship->attributes &= ~(kIsRemote | kIsPlayerShip);
        flagship = newShip;
        flagship->attributes |= (kIsRemote | kIsPlayerShip);
    }
    adm->set_flagship(newShip);
}

void TogglePlayerAutoPilot(Handle<SpaceObject> flagship) {
    if (flagship->attributes & kOnAutoPilot) {
        flagship->attributes &= ~kOnAutoPilot;
        if ((flagship->owner == g.admiral) && (flagship->attributes & kIsHumanControlled)) {
            StringList  strings(kMessageStringID);
            StringSlice string = strings.at(kAutoPilotOffString - 1);
            Messages::set_status(string, kStatusLabelColor);
        }
    } else {
        SetObjectDestination(flagship);
        flagship->attributes |= kOnAutoPilot;
        if ((flagship->owner == g.admiral) && (flagship->attributes & kIsHumanControlled)) {
            StringList  strings(kMessageStringID);
            StringSlice string = strings.at(kAutoPilotOnString - 1);
            Messages::set_status(string, kStatusLabelColor);
        }
    }
}

bool IsPlayerShipOnAutoPilot() {
    return g.ship.get() && (g.ship->attributes & kOnAutoPilot);
}

void PlayerShipGiveCommand(Handle<Admiral> whichAdmiral) {
    auto control = whichAdmiral->control();

    if (control.get()) {
        SetObjectDestination(control);
        if (whichAdmiral == g.admiral) {
            sys.sound.order();
        }
    }
}

void PlayerShipBodyExpire(Handle<SpaceObject> flagship) {
    auto selectShip = flagship->owner->control();

    if (selectShip.get()) {
        if ((selectShip->active != kObjectInUse) || (!(selectShip->attributes & kCanThink)) ||
            (selectShip->attributes & kStaticDestination) ||
            (selectShip->owner != flagship->owner) ||
            (!(selectShip->attributes & kCanAcceptDestination)))
            selectShip = SpaceObject::none();
    }
    if (!selectShip.get()) {
        selectShip = g.root;
        while (selectShip.get() && ((selectShip->active != kObjectInUse) ||
                                    (selectShip->attributes & kStaticDestination) ||
                                    (!((selectShip->attributes & kCanThink) &&
                                       (selectShip->attributes & kCanAcceptDestination))) ||
                                    (selectShip->owner != flagship->owner))) {
            selectShip = selectShip->nextObject;
        }
    }
    if (selectShip.get()) {
        ChangePlayerShipNumber(flagship->owner, selectShip);
    } else {
        if (!g.game_over) {
            g.game_over    = true;
            g.game_over_at = g.time + secs(3);
        }
        if (flagship->owner == g.admiral) {
            g.victory_text = kLevelNoShipTextID + g.level->levelNameStrNum;
        } else {
            g.victory_text = 10050 + g.level->levelNameStrNum;
        }
        if (flagship->owner.get()) {
            flagship->owner->set_flagship(SpaceObject::none());
        }
    }
}

int32_t HotKey_GetFromObject(Handle<SpaceObject> object) {
    if (!object.get() && !object->active) {
        return -1;
    }
    for (int32_t i = 0; i < kHotKeyNum; ++i) {
        if (globals()->hotKey[i].object == object) {
            if (globals()->hotKey[i].objectID == object->id) {
                return i;
            }
        }
    }
    return -1;
}

void Update_LabelStrings_ForHotKeyChange(void) {
    auto target = g.admiral->target();
    if (target.get()) {
        g.target_label->set_object(target);
        if (target == g.ship) {
            g.target_label->set_age(Label::kVisibleTime);
        }
        if (target->attributes & kIsDestination) {
            String string(GetDestBalanceName(target->asDestination));
            print(string, hot_key_suffix(target));
            g.target_label->set_string(string);
        } else {
            String string(get_object_name(target->base));
            print(string, hot_key_suffix(target));
            g.target_label->set_string(string);
        }
    }

    auto control = g.admiral->control();
    if (control.get()) {
        g.control_label->set_object(control);
        if (control == g.ship) {
            g.control_label->set_age(Label::kVisibleTime);
        }
        sys.sound.select();
        if (control->attributes & kIsDestination) {
            String string(GetDestBalanceName(control->asDestination));
            print(string, hot_key_suffix(control));
            g.control_label->set_string(string);
        } else {
            String string(get_object_name(control->base));
            print(string, hot_key_suffix(control));
            g.control_label->set_string(string);
        }
    }
}

}  // namespace antares
