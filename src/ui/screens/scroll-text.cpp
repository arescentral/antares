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

#include "ui/screens/scroll-text.hpp"

#include "config/preferences.hpp"
#include "drawing/build-pix.hpp"
#include "drawing/pix-map.hpp"
#include "game/globals.hpp"
#include "game/sys.hpp"
#include "game/time.hpp"
#include "sound/music.hpp"
#include "ui/card.hpp"
#include "video/driver.hpp"

using std::unique_ptr;

namespace antares {
namespace {

const int kScrollTextHeight = 200;

}  // namespace

ScrollTextScreen::ScrollTextScreen(pn::string_view text, int width, ticks interval)
        : _build_pix(BuildPix(text, width)), _interval(interval), _play_song(false) {}

ScrollTextScreen::ScrollTextScreen(
        pn::string_view text, int width, ticks interval, pn::string_view song)
        : _build_pix(BuildPix(text, width)),
          _interval(interval),
          _play_song(true),
          _song(song.copy()) {}

void ScrollTextScreen::become_front() {
    // If a song was requested, play it.
    if (_play_song) {
        sys.music.play(Music::IDLE, _song);
    }

    _start      = now();
    _next_shift = _start;
    _position   = -kScrollTextHeight;
}

void ScrollTextScreen::resign_front() {
    // If a song was requested, stop it.
    if (_play_song) {
        sys.music.stop();
    }
}

void ScrollTextScreen::mouse_down(const MouseDownEvent& event) {
    static_cast<void>(event);
    stack()->pop(this);
}

void ScrollTextScreen::key_down(const KeyDownEvent& event) {
    static_cast<void>(event);
    stack()->pop(this);
}

void ScrollTextScreen::gamepad_button_down(const GamepadButtonDownEvent& event) {
    static_cast<void>(event);
    stack()->pop(this);
}

bool ScrollTextScreen::next_timer(wall_time& time) {
    time = _next_shift;
    return true;
}

void ScrollTextScreen::fire_timer() {
    wall_time now = antares::now();
    while (_next_shift < now) {
        _next_shift += _interval;
        ++_position;
    }

    if (_position >= _build_pix.size().height) {
        stack()->pop(this);
    }
}

void ScrollTextScreen::draw() const {
    Rect world = sys.video->screen_size().as_rect();
    Rect clip  = Rect(0, 0, world.width(), kScrollTextHeight);
    clip.center_in(world);

    Rect position = _build_pix.size().as_rect();
    position.center_in(clip);
    position.offset(0, clip.top - position.top);
    position.offset(0, -_position);
    _build_pix.draw(position.origin());
    Rects rects;
    rects.fill(Rect(world.left, world.top, world.right, clip.top), RgbColor::black());
    rects.fill(Rect(world.left, clip.bottom, world.right, world.bottom), RgbColor::black());
}

}  // namespace antares
