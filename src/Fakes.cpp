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

#include "Fakes.hpp"

#include "FakeDrawing.hpp"
#include "FakeHandles.hpp"
#include "FakeMath.hpp"
#include "FakeSounds.hpp"
#include "FakeTime.hpp"

void ModalDialog(void*, short* item) {
    *item = 1;
}

bool WaitNextEvent(long mask, EventRecord* evt, unsigned long sleep, Rgn** mouseRgn) {
    evt->what = 0;
    return true;
}

// Perform this many clicks in succession at the start of the game.
const int kClickCount = 2;

bool Button() {
    static int current_click = 0;
    static bool clicked = false;
    if (current_click < kClickCount) {
        if (clicked) {
            ++current_click;
        }
        clicked = !clicked;
    }
    return clicked;
}

void GetKeys(KeyMap keys) {
    bzero(keys, sizeof(KeyMap));
}

void FakeInit(int argc, const char** argv) {
    (void)argc;
    (void)argv;
    FakeDrawingInit();
    FakeHandlesInit();
    FakeMathInit();
    FakeSoundsInit();
    FakeTimeInit();
}
