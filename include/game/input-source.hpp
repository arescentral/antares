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

#ifndef ANTARES_GAME_INPUT_SOURCE_HPP_
#define ANTARES_GAME_INPUT_SOURCE_HPP_

#include <stdint.h>
#include <sfz/sfz.hpp>

#include "config/keys.hpp"
#include "data/handle.hpp"
#include "ui/event.hpp"

namespace antares {

struct ReplayData;

class InputSource : public EventReceiver {
  public:
    virtual ~InputSource();

    virtual void start() = 0;
    virtual bool get(Handle<Admiral> admiral, game_ticks at, EventReceiver& key_map) = 0;
};

class RealInputSource : public InputSource {
  public:
    virtual void start();
    virtual bool get(Handle<Admiral> admiral, game_ticks at, EventReceiver& key_map);

    virtual void key_down(const KeyDownEvent& event);
    virtual void key_up(const KeyUpEvent& event);
    virtual void gamepad_button_down(const GamepadButtonDownEvent& event);
    virtual void gamepad_button_up(const GamepadButtonUpEvent& event);
    virtual void gamepad_stick(const GamepadStickEvent& event);
    virtual void mouse_down(const MouseDownEvent& event);
    virtual void mouse_up(const MouseUpEvent& event);
    virtual void mouse_move(const MouseMoveEvent& event);

  private:
    static game_ticks at();

    std::multimap<std::pair<int, game_ticks>, std::unique_ptr<Event>> _events;
};

class ReplayInputSource : public InputSource {
  public:
    explicit ReplayInputSource(ReplayData* data);

    virtual void start();
    virtual bool get(Handle<Admiral> admiral, game_ticks at, EventReceiver& key_map);

    virtual void key_down(const KeyDownEvent& event);
    virtual void gamepad_button_down(const GamepadButtonDownEvent& event);
    virtual void mouse_down(const MouseDownEvent& event);

  private:
    bool advance(EventReceiver& receiver);

    game_ticks _duration;
    std::multimap<std::pair<int, game_ticks>, std::unique_ptr<Event>> _events;
    bool _exit;

    DISALLOW_COPY_AND_ASSIGN(ReplayInputSource);
};

}  // namespace antares

#endif  // ANTARES_GAME_INPUT_SOURCE_HPP_
