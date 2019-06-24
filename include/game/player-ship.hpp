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
    ACCEL_ON   = 0x00,
    ACCEL_OFF  = 0x01,
    DECEL_ON   = 0x02,
    DECEL_OFF  = 0x03,
    CCW_ON     = 0x04,
    CCW_OFF    = 0x05,
    CW_ON      = 0x06,
    CW_OFF     = 0x07,
    FIRE_1_ON  = 0x08,
    FIRE_1_OFF = 0x09,
    FIRE_2_ON  = 0x0a,
    FIRE_2_OFF = 0x0b,
    FIRE_S_ON  = 0x0c,
    FIRE_S_OFF = 0x0d,
    WARP_ON    = 0x0e,
    WARP_OFF   = 0x0f,

    ZOOM_IN  = 0x010,
    ZOOM_OUT = 0x011,
    ZOOM_1X  = 0x012,
    ZOOM_2X  = 0x013,
    ZOOM_4X  = 0x014,
    ZOOM_16X = 0x015,
    ZOOM_FOE = 0x016,
    ZOOM_OBJ = 0x017,
    ZOOM_ALL = 0x018,

    SELECT_FRIEND = 0x020,
    TARGET_FRIEND = 0x021,
    TARGET_FOE    = 0x022,
    SELECT_BASE   = 0x023,
    TARGET_BASE   = 0x024,
    TARGET_SELF   = 0x025,

    AUTOPILOT     = 0x030,
    ORDER         = 0x031,
    TRANSFER      = 0x032,
    MINI_TRANSFER = 0x033,
    MINI_HOLD     = 0x034,
    MINI_COME     = 0x035,
    MINI_FIRE_1   = 0x036,
    MINI_FIRE_2   = 0x037,
    MINI_FIRE_S   = 0x038,

    NEXT_PAGE         = 0x040,
    MINI_NEXT_PAGE    = 0x041,
    MINI_PREV_PAGE    = 0x042,
    MINI_LAST_MESSAGE = 0x043,

    MINI_BUILD    = 0x050,
    HOTKEY_SET    = 0x051,
    HOTKEY_SELECT = 0x052,
    HOTKEY_TARGET = 0x053,
};

struct PlayerEvent {
    PlayerEventType type;
    int             data;

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

    void update();

    bool    show_select() const;
    bool    show_target() const;
    int32_t control_direction() const;

    GameCursor&       cursor() { return _cursor; }
    const GameCursor& cursor() const { return _cursor; }

    bool entering_message() const { return _message.editing(); }

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

    class MessageTextReceiver : public TextReceiver {
      public:
        using TextReceiver::range;

        MessageTextReceiver();

        void start_editing();
        void stop_editing();
        bool editing() const { return _editing; }

        virtual void replace(range<int> replace, pn::string_view text);
        virtual void select(range<int> select);
        virtual void mark(range<int> mark);
        virtual void newline();
        virtual void tab();
        virtual void escape();

        virtual int        offset(int origin, int by) const;
        virtual int        size() const;
        virtual range<int> selection() const;
        virtual range<int> mark() const;

      private:
        bool       _editing = false;
        pn::string _text;
        range<int> _selection;
        range<int> _mark;
    };
    MessageTextReceiver _message;
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
