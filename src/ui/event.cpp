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

#include "ui/event.hpp"

#include "game/time.hpp"

namespace antares {

Event::Event(wall_time at) : _at(at) {}

Event::~Event() {}

wall_time Event::at() const { return _at; }

void KeyDownEvent::send(EventReceiver* receiver) const { receiver->key_down(*this); }

void KeyUpEvent::send(EventReceiver* receiver) const { receiver->key_up(*this); }

void GamepadButtonDownEvent::send(EventReceiver* receiver) const {
    receiver->gamepad_button_down(*this);
}

void GamepadButtonUpEvent::send(EventReceiver* receiver) const {
    receiver->gamepad_button_up(*this);
}

void GamepadStickEvent::send(EventReceiver* receiver) const { receiver->gamepad_stick(*this); }

void MouseDownEvent::send(EventReceiver* receiver) const { receiver->mouse_down(*this); }

void MouseUpEvent::send(EventReceiver* receiver) const { receiver->mouse_up(*this); }

void MouseMoveEvent::send(EventReceiver* receiver) const { receiver->mouse_move(*this); }

EventReceiver::~EventReceiver() {}

void EventReceiver::key_down(const KeyDownEvent& event) {}
void EventReceiver::key_up(const KeyUpEvent& event) {}
void EventReceiver::gamepad_button_down(const GamepadButtonDownEvent& event) {}
void EventReceiver::gamepad_button_up(const GamepadButtonUpEvent& event) {}
void EventReceiver::gamepad_stick(const GamepadStickEvent& event) {}
void EventReceiver::mouse_down(const MouseDownEvent& event) {}
void EventReceiver::mouse_up(const MouseUpEvent& event) {}
void EventReceiver::mouse_move(const MouseMoveEvent& event) {}

}  // namespace antares
