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

#ifndef ANTARES_GAME_PLAYER_SHIP_HPP_
#define ANTARES_GAME_PLAYER_SHIP_HPP_

#include <vector>

#include "config/keys.hpp"
#include "data/base-object.hpp"
#include "game/cursor.hpp"
#include "ui/event.hpp"

namespace antares {

enum class PlayerKeyNum {
    ACCEL  = kUpKeyNum,
    DECEL  = kDownKeyNum,
    CCW    = kLeftKeyNum,
    CW     = kRightKeyNum,
    FIRE_1 = kOneKeyNum,
    FIRE_2 = kTwoKeyNum,
    FIRE_S = kEnterKeyNum,
    WARP   = kWarpKeyNum,

    SELECT_FRIEND = kSelectFriendKeyNum,
    SELECT_FOE    = kSelectFoeKeyNum,
    SELECT_BASE   = kSelectBaseKeyNum,

    ORDER = kOrderKeyNum,

    ZOOM_IN  = kZoomInKeyNum,
    ZOOM_OUT = kZoomOutKeyNum,

    COMP_UP     = kCompUpKeyNum,
    COMP_DOWN   = kCompDownKeyNum,
    COMP_ACCEPT = kCompAcceptKeyNum,
    COMP_CANCEL = kCompCancelKeyNum,

    TRANSFER     = kTransferKeyNum,
    ZOOM_1X      = kScale121KeyNum,
    ZOOM_2X      = kScale122KeyNum,
    ZOOM_4X      = kScale124KeyNum,
    ZOOM_16X     = kScale1216KeyNum,
    ZOOM_FOE     = kScaleHostileKeyNum,
    ZOOM_OBJ     = kScaleObjectKeyNum,
    ZOOM_ALL     = kScaleAllKeyNum,
    MESSAGE_NEXT = kMessageNextKeyNum,

    AUTOPILOT     = 0x0100,
    TARGET_FRIEND = 0x0101,
    TARGET_BASE   = 0x0102,
    TARGET_SELF   = 0x0103,

    SET_HOTKEY_1  = 0x0201,
    SET_HOTKEY_2  = 0x0202,
    SET_HOTKEY_3  = 0x0203,
    SET_HOTKEY_4  = 0x0204,
    SET_HOTKEY_5  = 0x0205,
    SET_HOTKEY_6  = 0x0206,
    SET_HOTKEY_7  = 0x0207,
    SET_HOTKEY_8  = 0x0208,
    SET_HOTKEY_9  = 0x0209,
    SET_HOTKEY_10 = 0x020a,

    SELECT_HOTKEY_1  = 0x0301,
    SELECT_HOTKEY_2  = 0x0302,
    SELECT_HOTKEY_3  = 0x0303,
    SELECT_HOTKEY_4  = 0x0304,
    SELECT_HOTKEY_5  = 0x0305,
    SELECT_HOTKEY_6  = 0x0306,
    SELECT_HOTKEY_7  = 0x0307,
    SELECT_HOTKEY_8  = 0x0308,
    SELECT_HOTKEY_9  = 0x0309,
    SELECT_HOTKEY_10 = 0x030a,

    TARGET_HOTKEY_1  = 0x0401,
    TARGET_HOTKEY_2  = 0x0402,
    TARGET_HOTKEY_3  = 0x0403,
    TARGET_HOTKEY_4  = 0x0404,
    TARGET_HOTKEY_5  = 0x0405,
    TARGET_HOTKEY_6  = 0x0406,
    TARGET_HOTKEY_7  = 0x0407,
    TARGET_HOTKEY_8  = 0x0408,
    TARGET_HOTKEY_9  = 0x0409,
    TARGET_HOTKEY_10 = 0x040a,
};

struct PlayerEvent {
    enum Type {
        KEY_DOWN,
        KEY_UP,
    } type;
    union {
        PlayerKeyNum key;
    };

    static PlayerEvent key_down(PlayerKeyNum k) { return PlayerEvent{KEY_DOWN, {k}}; }
    static PlayerEvent key_up(PlayerKeyNum k) { return PlayerEvent{KEY_UP, {k}}; }

    bool operator==(PlayerEvent other) const;
    bool operator!=(PlayerEvent other) const { return !(*this == other); }
    bool operator<(PlayerEvent other) const;
    bool operator<=(PlayerEvent other) const { return !(other < *this); }
    bool operator>(PlayerEvent other) const { return (other < *this); }
    bool operator>=(PlayerEvent other) const { return !(*this < other); }
};

class GameCursor;
class InputSource;

class PlayerShip : public EventReceiver {
  public:
    PlayerShip();

    virtual void key_down(const KeyDownEvent& event);
    virtual void key_up(const KeyUpEvent& event);

    virtual void mouse_down(const MouseDownEvent& event);
    virtual void mouse_up(const MouseUpEvent& event);
    virtual void mouse_move(const MouseMoveEvent& event);

    virtual void gamepad_button_down(const GamepadButtonDownEvent& event);
    virtual void gamepad_button_up(const GamepadButtonUpEvent& event);
    virtual void gamepad_stick(const GamepadStickEvent& event);

    void update(bool enter_message);

    bool    show_select() const;
    bool    show_target() const;
    int32_t control_direction() const;

    GameCursor&       cursor() { return _cursor; }
    const GameCursor& cursor() const { return _cursor; }

  private:
    bool active() const;

    uint32_t                 gTheseKeys;
    uint32_t                 _gamepad_keys;
    std::vector<PlayerEvent> _player_events;
    KeyMap                   _keys;

    enum GamepadState {
        NO_BUMPER              = 0,
        SELECT_BUMPER          = 1,
        TARGET_BUMPER          = 2,
        EITHER_BUMPER          = SELECT_BUMPER | TARGET_BUMPER,
        OVERRIDE               = 4,
        SELECT_BUMPER_OVERRIDE = SELECT_BUMPER | OVERRIDE,
        TARGET_BUMPER_OVERRIDE = TARGET_BUMPER | OVERRIDE,
    };
    GamepadState _gamepad_state;
    bool         _control_active;
    int32_t      _control_direction;
    GameCursor   _cursor;
};

void ResetPlayerShip();
void PlayerShipHandleClick(Point where, int button);
void ChangePlayerShipNumber(Handle<Admiral> whichAdmiral, Handle<SpaceObject> newShip);
void TogglePlayerAutoPilot(Handle<SpaceObject> theShip);
void PlayerShipGiveCommand(Handle<Admiral> whichAdmiral);
void PlayerShipBodyExpire(Handle<SpaceObject> theShip);
void HandleTextMessageKeys(const KeyMap&, const KeyMap&, bool*);

}  // namespace antares

#endif  // ANTARES_GAME_PLAYER_SHIP_HPP_
