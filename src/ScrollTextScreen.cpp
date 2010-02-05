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

#include "ScrollTextScreen.hpp"

#include "AresGlobalType.hpp"
#include "BuildPix.hpp"
#include "CardStack.hpp"
#include "FakeDrawing.hpp"
#include "Music.hpp"
#include "Options.hpp"
#include "Time.hpp"

using sfz::scoped_ptr;

namespace antares {

namespace {

const int kScrollTextHeight = 200;

}  // namespace

ScrollTextScreen::ScrollTextScreen(int text_id, int width, double speed)
        : _pix_map(build_pix(text_id, width)),
          _speed(speed),
          _play_song(false),
          _song_id(0),
          _next_shift(now_secs() + (1.0 / 60.0)) { }

ScrollTextScreen::ScrollTextScreen(int text_id, int width, double speed, int song_id)
        : _pix_map(build_pix(text_id, width)),
          _speed(speed),
          _play_song(true),
          _song_id(song_id),
          _next_shift(now_secs() + (1.0 / 60.0)) { }

void ScrollTextScreen::become_front() {
    // If a song was requested, play it.
    if (_play_song && (globals()->gOptions & kOptionMusicIdle)) {
        if (SongIsPlaying()) {
            StopAndUnloadSong();
        }
        LoadSong(_song_id);
        SetSongVolume(kMaxMusicVolume);
        PlaySong();
    }

    gActiveWorld->fill(RgbColor::kBlack);
    _start = now_secs();
    _window = Rect(0, -kScrollTextHeight, _pix_map->bounds().right, 0);
}

void ScrollTextScreen::resign_front() {
    gActiveWorld->fill(RgbColor::kBlack);

    // If a song was requested, stop it.
    if (_play_song && SongIsPlaying()) {
        StopAndUnloadSong();
    }
}

bool ScrollTextScreen::mouse_down(int button, const Point& where) {
    (void)button;
    (void)where;
    stack()->pop(this);
    return true;
}

bool ScrollTextScreen::key_down(int key) {
    (void)key;
    stack()->pop(this);
    return true;
}

double ScrollTextScreen::next_timer() {
    return _next_shift;
}

void ScrollTextScreen::fire_timer() {
    double now = now_secs();
    while (_next_shift < now) {
        _next_shift += (1.0 / _speed);
    }
    int top = ((now - _start) * _speed) - kScrollTextHeight;
    if (top > _window.top) {
        _window.offset(0, top - _window.top);
        Rect dest = _window;
        dest.center_in(gRealWorld->bounds());

        if (_window.intersects(_pix_map->bounds())) {
            gActiveWorld->fill(RgbColor::kBlack);
            CopyBits(_pix_map.get(), gRealWorld, _window, dest);
        } else {
            stack()->pop(this);
        }
    }
}

}  // namespace antares
