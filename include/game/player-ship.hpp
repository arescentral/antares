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

#ifndef ANTARES_GAME_PLAYER_SHIP_HPP_
#define ANTARES_GAME_PLAYER_SHIP_HPP_

#include "data/space-object.hpp"

namespace antares {

class GameCursor;
class InputSource;

class PlayerShip : public EventReceiver {
  public:
    PlayerShip();

    void update_keys(const KeyMap& keys);
    virtual void key_down(const KeyDownEvent& event);
    virtual void key_up(const KeyUpEvent& event);

    virtual void gamepad_button_down(const GamepadButtonDownEvent& event);
    virtual void gamepad_button_up(const GamepadButtonUpEvent& event);
    virtual void gamepad_stick(const GamepadStickEvent& event);

    void update(int64_t timePass, const GameCursor& cursor, bool enter_message);

    bool show_select() const;
    bool show_target() const;
    int32_t control_direction() const;
    bool show_right_stick() const;
    int32_t goal_direction() const;

  private:
    bool active() const;

    uint32_t gTheseKeys;
    uint32_t gLastKeys;
    KeyMap _keys;

    enum GamepadState {
        NO_BUMPER               = 0,
        SELECT_BUMPER           = 1,
        TARGET_BUMPER           = 2,
        EITHER_BUMPER           = SELECT_BUMPER | TARGET_BUMPER,
        OVERRIDE                = 4,
        SELECT_BUMPER_OVERRIDE  = SELECT_BUMPER | OVERRIDE,
        TARGET_BUMPER_OVERRIDE  = TARGET_BUMPER | OVERRIDE,
    };
    GamepadState _gamepad_state;
    bool _control_active;
    int32_t _control_direction;
};

void ResetPlayerShip(int32_t);
void PlayerShipHandleClick(Point where, int button);
void SetPlayerSelectShip(int32_t, bool, int32_t);
void ChangePlayerShipNumber(int32_t, int32_t);
void TogglePlayerAutoPilot( spaceObjectType *);
bool IsPlayerShipOnAutoPilot( void);
void PlayerShipGiveCommand(int32_t);
void PlayerShipBodyExpire( spaceObjectType *, bool);
void HandleTextMessageKeys(const KeyMap&, const KeyMap&, bool *);

}  // namespace antares

#endif // ANTARES_GAME_PLAYER_SHIP_HPP_
