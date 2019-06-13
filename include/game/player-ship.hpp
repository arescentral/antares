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
    ACCEL  = 0x00,
    DECEL  = 0x01,
    CCW    = 0x02,
    CW     = 0x03,
    FIRE_1 = 0x04,
    FIRE_2 = 0x05,
    FIRE_S = 0x06,
    WARP   = 0x07,

    COMP_UP      = 0x10,
    COMP_DOWN    = 0x11,
    COMP_ACCEPT  = 0x12,
    COMP_CANCEL  = 0x13,
    MESSAGE_NEXT = 0x14,
    AUTOPILOT    = 0x15,
    ORDER        = 0x16,
    TRANSFER     = 0x17,

    ZOOM_IN  = 0x20,
    ZOOM_OUT = 0x21,
    ZOOM_1X  = 0x22,
    ZOOM_2X  = 0x23,
    ZOOM_4X  = 0x24,
    ZOOM_16X = 0x25,
    ZOOM_FOE = 0x26,
    ZOOM_OBJ = 0x27,
    ZOOM_ALL = 0x28,

    SELECT_FRIEND = 0x30,
    TARGET_FRIEND = 0x31,
    TARGET_FOE    = 0x32,
    SELECT_BASE   = 0x33,
    TARGET_BASE   = 0x34,
    TARGET_SELF   = 0x35,

    SET_HOTKEY_1  = 0x40,
    SET_HOTKEY_2  = 0x41,
    SET_HOTKEY_3  = 0x42,
    SET_HOTKEY_4  = 0x43,
    SET_HOTKEY_5  = 0x44,
    SET_HOTKEY_6  = 0x45,
    SET_HOTKEY_7  = 0x46,
    SET_HOTKEY_8  = 0x47,
    SET_HOTKEY_9  = 0x48,
    SET_HOTKEY_10 = 0x49,

    SELECT_HOTKEY_1  = 0x50,
    SELECT_HOTKEY_2  = 0x51,
    SELECT_HOTKEY_3  = 0x52,
    SELECT_HOTKEY_4  = 0x53,
    SELECT_HOTKEY_5  = 0x54,
    SELECT_HOTKEY_6  = 0x55,
    SELECT_HOTKEY_7  = 0x56,
    SELECT_HOTKEY_8  = 0x57,
    SELECT_HOTKEY_9  = 0x58,
    SELECT_HOTKEY_10 = 0x59,

    TARGET_HOTKEY_1  = 0x60,
    TARGET_HOTKEY_2  = 0x61,
    TARGET_HOTKEY_3  = 0x62,
    TARGET_HOTKEY_4  = 0x63,
    TARGET_HOTKEY_5  = 0x64,
    TARGET_HOTKEY_6  = 0x65,
    TARGET_HOTKEY_7  = 0x66,
    TARGET_HOTKEY_8  = 0x67,
    TARGET_HOTKEY_9  = 0x68,
    TARGET_HOTKEY_10 = 0x69,
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
