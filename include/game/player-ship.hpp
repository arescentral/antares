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

    COMP_UP_ON       = 0x010,
    COMP_UP_OFF      = 0x110,
    COMP_DOWN_ON     = 0x011,
    COMP_DOWN_OFF    = 0x111,
    COMP_ACCEPT_ON   = 0x012,
    COMP_ACCEPT_OFF  = 0x112,
    COMP_CANCEL_ON   = 0x013,
    COMP_CANCEL_OFF  = 0x113,
    MESSAGE_NEXT_ON  = 0x014,
    MESSAGE_NEXT_OFF = 0x114,
    AUTOPILOT_ON     = 0x015,
    AUTOPILOT_OFF    = 0x115,
    ORDER_ON         = 0x016,
    ORDER_OFF        = 0x116,
    TRANSFER_ON      = 0x017,
    TRANSFER_OFF     = 0x117,

    ZOOM_IN_ON   = 0x020,
    ZOOM_IN_OFF  = 0x120,
    ZOOM_OUT_ON  = 0x021,
    ZOOM_OUT_OFF = 0x121,
    ZOOM_1X_ON   = 0x022,
    ZOOM_1X_OFF  = 0x122,
    ZOOM_2X_ON   = 0x023,
    ZOOM_2X_OFF  = 0x123,
    ZOOM_4X_ON   = 0x024,
    ZOOM_4X_OFF  = 0x124,
    ZOOM_16X_ON  = 0x025,
    ZOOM_16X_OFF = 0x125,
    ZOOM_FOE_ON  = 0x026,
    ZOOM_FOE_OFF = 0x126,
    ZOOM_OBJ_ON  = 0x027,
    ZOOM_OBJ_OFF = 0x127,
    ZOOM_ALL_ON  = 0x028,
    ZOOM_ALL_OFF = 0x128,

    SELECT_FRIEND_ON  = 0x030,
    SELECT_FRIEND_OFF = 0x130,
    TARGET_FRIEND_ON  = 0x031,
    TARGET_FRIEND_OFF = 0x131,
    TARGET_FOE_ON     = 0x032,
    TARGET_FOE_OFF    = 0x132,
    SELECT_BASE_ON    = 0x033,
    SELECT_BASE_OFF   = 0x133,
    TARGET_BASE_ON    = 0x034,
    TARGET_BASE_OFF   = 0x134,
    TARGET_SELF_ON    = 0x035,
    TARGET_SELF_OFF   = 0x135,

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
};

struct PlayerEvent {
    PlayerKeyNum key;

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
