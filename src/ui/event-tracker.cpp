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

#include "ui/event-tracker.hpp"

using sfz::Exception;
using sfz::format;

namespace antares {

void EventTracker::key_down(const KeyDownEvent& event) {
    _input_mode = KEYBOARD_MOUSE;
}

void EventTracker::gamepad_button_down(const GamepadButtonDownEvent& event) {
    _input_mode = GAMEPAD;
}

void EventTracker::gamepad_stick(const GamepadStickEvent& event) {
    _input_mode = GAMEPAD;
}

void EventTracker::mouse_down(const MouseDownEvent& event) {
    _input_mode = KEYBOARD_MOUSE;
    _mouse = event.where();
}

void EventTracker::mouse_up(const MouseUpEvent& event) {
    _mouse = event.where();
}

void EventTracker::mouse_move(const MouseMoveEvent& event) {
    _input_mode = KEYBOARD_MOUSE;
    _mouse = event.where();
}

}  // namespace antares
