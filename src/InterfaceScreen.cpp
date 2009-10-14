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

#include "InterfaceScreen.hpp"

#include "FakeDrawing.hpp"
#include "InterfaceHandling.hpp"
#include "PlayerInterface.hpp"

namespace antares {

namespace {

double now() {
    uint64_t usecs;
    Microseconds(&usecs);
    return usecs / 1000000.0;
}

}  // namespace

InterfaceScreen::InterfaceScreen(int id)
        : _id(id),
          _last_event(now()) { }

InterfaceScreen::~InterfaceScreen() { }

void InterfaceScreen::become_front() {
    ClearScreen();
    OpenInterface(_id);
    this->adjust_interface();
    DrawEntireInterface();
    _last_event = now();
    // half-second fade from black.
}

bool InterfaceScreen::mouse_down(int button, const Point& where) {
    int which_item = InterfaceMouseDown(where);
    if (which_item >= 0) {
        this->handle_button(which_item);
    }
    return true;
}

bool InterfaceScreen::mouse_up(int button, const Point& where) {
    (void)button;
    (void)where;
    return true;
}

bool InterfaceScreen::mouse_moved(int button, const Point& where) {
    (void)button;
    (void)where;
    return true;
}

bool InterfaceScreen::key_down(int key) {
    int which_item = InterfaceKeyDown(key);
    if (which_item >= 0) {
        this->handle_button(which_item);
    }
    return true;
}

double InterfaceScreen::last_event() const {
    return _last_event;
}

void InterfaceScreen::adjust_interface() { }

}  // namespace antares
