// Ares, a tactical space combat game.
// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#include "Event.hpp"

namespace antares {

Event::~Event() { }

void KeyDownEvent::send(EventReceiver* receiver) const {
    receiver->key_down(*this);
}

void KeyUpEvent::send(EventReceiver* receiver) const {
    receiver->key_up(*this);
}

void MouseDownEvent::send(EventReceiver* receiver) const {
    receiver->mouse_down(*this);
}

void MouseUpEvent::send(EventReceiver* receiver) const {
    receiver->mouse_up(*this);
}

void MouseMoveEvent::send(EventReceiver* receiver) const {
    receiver->mouse_move(*this);
}

EventReceiver::~EventReceiver() { }

void EventReceiver::key_down(const KeyDownEvent& event) { }
void EventReceiver::key_up(const KeyUpEvent& event) { }
void EventReceiver::mouse_down(const MouseDownEvent& event) { }
void EventReceiver::mouse_up(const MouseUpEvent& event) { }
void EventReceiver::mouse_move(const MouseMoveEvent& event) { }

}  // namespace antares
