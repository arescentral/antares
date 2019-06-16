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

enum class PlayerEventType {
    ACCEL_ON   = 0x000,
    ACCEL_OFF  = 0x100,
    DECEL_ON   = 0x001,
    DECEL_OFF  = 0x101,
    CCW_ON     = 0x002,
    CCW_OFF    = 0x102,
    CW_ON      = 0x003,
    CW_OFF     = 0x103,
    FIRE_1_ON  = 0x004,
    FIRE_1_OFF = 0x104,
    FIRE_2_ON  = 0x005,
    FIRE_2_OFF = 0x105,
    FIRE_S_ON  = 0x006,
    FIRE_S_OFF = 0x106,
    WARP_ON    = 0x007,
    WARP_OFF   = 0x107,

    MESSAGE_NEXT = 0x010,
    AUTOPILOT    = 0x011,
    ORDER        = 0x012,
    TRANSFER     = 0x013,

    ZOOM_IN  = 0x020,
    ZOOM_OUT = 0x021,
    ZOOM_1X  = 0x022,
    ZOOM_2X  = 0x023,
    ZOOM_4X  = 0x024,
    ZOOM_16X = 0x025,
    ZOOM_FOE = 0x026,
    ZOOM_OBJ = 0x027,
    ZOOM_ALL = 0x028,

    SELECT_FRIEND = 0x030,
    TARGET_FRIEND = 0x031,
    TARGET_FOE    = 0x032,
    SELECT_BASE   = 0x033,
    TARGET_BASE   = 0x034,
    TARGET_SELF   = 0x035,

    SET_HOTKEY_1  = 0x040,
    SET_HOTKEY_3  = 0x042,
    SET_HOTKEY_4  = 0x043,
    SET_HOTKEY_5  = 0x044,
    SET_HOTKEY_6  = 0x045,
    SET_HOTKEY_7  = 0x046,
    SET_HOTKEY_8  = 0x047,
    SET_HOTKEY_9  = 0x048,
    SET_HOTKEY_10 = 0x049,

    SELECT_HOTKEY_1  = 0x050,
    SELECT_HOTKEY_3  = 0x052,
    SELECT_HOTKEY_4  = 0x053,
    SELECT_HOTKEY_5  = 0x054,
    SELECT_HOTKEY_6  = 0x055,
    SELECT_HOTKEY_7  = 0x056,
    SELECT_HOTKEY_8  = 0x057,
    SELECT_HOTKEY_9  = 0x058,
    SELECT_HOTKEY_10 = 0x059,

    TARGET_HOTKEY_1  = 0x060,
    TARGET_HOTKEY_2  = 0x061,
    TARGET_HOTKEY_3  = 0x062,
    TARGET_HOTKEY_4  = 0x063,
    TARGET_HOTKEY_5  = 0x064,
    TARGET_HOTKEY_6  = 0x065,
    TARGET_HOTKEY_7  = 0x066,
    TARGET_HOTKEY_8  = 0x067,
    TARGET_HOTKEY_9  = 0x068,
    TARGET_HOTKEY_10 = 0x069,

    MINI_BUILD_1 = 0x070,
    MINI_BUILD_2 = 0x071,
    MINI_BUILD_3 = 0x072,
    MINI_BUILD_4 = 0x073,
    MINI_BUILD_5 = 0x074,
    MINI_BUILD_6 = 0x075,

    MINI_TRANSFER = 0x080,
    MINI_HOLD     = 0x081,
    MINI_COME     = 0x082,
    MINI_FIRE_1   = 0x083,
    MINI_FIRE_2   = 0x084,
    MINI_FIRE_S   = 0x085,

    MINI_NEXT_PAGE    = 0x090,
    MINI_PREV_PAGE    = 0x091,
    MINI_LAST_MESSAGE = 0x092,
};

struct PlayerEvent {
    PlayerEventType key;

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
