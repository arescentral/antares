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
    if (_strict && _keys.get(event.key())) {
        throw Exception(format("Received KeyDownEvent but key {0} already down.", event.key()));
    }
    _keys.set(event.key(), true);
}

void EventTracker::key_up(const KeyUpEvent& event) {
    if (_strict && !_keys.get(event.key())) {
        throw Exception(format("Received KeyUpEvent but key {0} already up.", event.key()));
    }
    _keys.set(event.key(), false);
}

void EventTracker::mouse_down(const MouseDownEvent& event) {
    if (_strict && _button) {
        throw Exception("Received MouseDownEvent when mouse already down.");
    }
    _button = true;
    _mouse = event.where();
}

void EventTracker::mouse_up(const MouseUpEvent& event) {
    if (_strict && !_button) {
        throw Exception("Received MouseUpEvent when mouse already up.");
    }
    _button = false;
    _mouse = event.where();
}

void EventTracker::mouse_move(const MouseMoveEvent& event) {
    _mouse = event.where();
}

}  // namespace antares
