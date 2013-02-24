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

#include "ui/screens/scroll-text.hpp"

#include "config/preferences.hpp"
#include "drawing/build-pix.hpp"
#include "drawing/pix-map.hpp"
#include "game/globals.hpp"
#include "game/time.hpp"
#include "sound/music.hpp"
#include "ui/card.hpp"
#include "video/driver.hpp"

using sfz::format;
using std::unique_ptr;

namespace antares {
namespace {

const int kScrollTextHeight = 200;

}  // namespace

ScrollTextScreen::ScrollTextScreen(int text_id, int width, double speed)
        : _speed(speed),
          _play_song(false),
          _song_id(0) {
    unique_ptr<PixMap> pix_map(build_pix(text_id, width));
    _sprite.reset(VideoDriver::driver()->new_sprite(format("/x/scroll_text/{0}", text_id), *pix_map));
}

ScrollTextScreen::ScrollTextScreen(int text_id, int width, double speed, int song_id)
        : _speed(speed),
          _play_song(true),
          _song_id(song_id) {
    unique_ptr<PixMap> pix_map(build_pix(text_id, width));
    _sprite.reset(VideoDriver::driver()->new_sprite(format("/x/scroll_text/{0}", text_id), *pix_map));
}

void ScrollTextScreen::become_front() {
    // If a song was requested, play it.
    if (_play_song && Preferences::preferences()->play_idle_music()) {
        if (SongIsPlaying()) {
            StopAndUnloadSong();
        }
        LoadSong(_song_id);
        SetSongVolume(kMaxMusicVolume);
        PlaySong();
    }

    _start = now_usecs();
    _next_shift = _start;

    _clip = Rect(0, 0, world.width(), kScrollTextHeight);
    _clip.center_in(world);

    _position = _sprite->size().as_rect();
    _position.center_in(_clip);
    _position.offset(0, _clip.bottom - _position.top);
}

void ScrollTextScreen::resign_front() {
    // If a song was requested, stop it.
    if (_play_song && SongIsPlaying()) {
        StopAndUnloadSong();
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

bool ScrollTextScreen::next_timer(int64_t& time) {
    time = _next_shift;
    return true;
}

void ScrollTextScreen::fire_timer() {
    int64_t now = now_usecs();
    while (_next_shift < now) {
        _next_shift += (1e6 / _speed);
        _position.offset(0, -1);
    }

    if (!_position.intersects(_clip)) {
        stack()->pop(this);
    }
}

void ScrollTextScreen::draw() const {
    _sprite->draw(_position.left, _position.top);
    VideoDriver::driver()->fill_rect(
            Rect(world.left, world.top, world.right, _clip.top), RgbColor::kBlack);
    VideoDriver::driver()->fill_rect(
            Rect(world.left, _clip.bottom, world.right, world.bottom), RgbColor::kBlack);
}

}  // namespace antares
