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

#include <string>
#include <vector>
#include "AresGlobalType.hpp"
#include "ColorTranslation.hpp"
#include "DirectText.hpp"
#include "FakeDrawing.hpp"
#include "MessageScreen.hpp"
#include "Music.hpp"
#include "Options.hpp"
#include "Picture.hpp"
#include "RetroText.hpp"

namespace antares {

namespace {

const int kScrollTextHeight = 200;

double now() {
    uint64_t usecs;
    Microseconds(&usecs);
    return usecs / 1000000.0;
}

int string_to_int(const std::string str) {
    if (str.size() > 0) {
        char* end;
        int value = strtol(str.c_str(), &end, 10);
        if (end == str.c_str() + str.size()) {
            return value;
        }
    }
    fprintf(stderr, "Couldn't parse '%s' as an integer\n", str.c_str());
    exit(1);
}

class ScrollTextPixBuilder {
  public:
    ScrollTextPixBuilder(PixMap* pix)
            : _pix(pix) { }

    void set_background(int id) {
        _background.reset(new Picture(id));
        _background_start = _pix->bounds.bottom;
    }

    void add_picture(int id) {
        Picture pict(id);
        extend(pict.frame().bottom);
        Rect dest = pict.frame();
        Rect surround(
                0, _pix->bounds.bottom - pict.frame().height(),
                _pix->bounds.right, _pix->bounds.bottom);
        dest.center_in(surround);
        pict.draw_to(_pix, pict.frame(), dest);
    }

    void add_text(const std::string& text) {
        uint8_t black = 0xFF;
        uint8_t red = GetTranslateColorShade(RED, VERY_LIGHT);
        RetroText retro(text.c_str(), text.size(), kTitleFontNum, red, black);
        int text_height = retro.height_for_width(_pix->bounds.right - 12);

        Rect dest(0, 0, _pix->bounds.right, text_height);
        dest.offset(0, _pix->bounds.bottom);
        dest.inset(6, 0);

        extend(text_height);
        retro.draw(_pix, dest);
    }

  private:
    void extend(int height) {
        int old_height = _pix->bounds.bottom;
        int new_height = old_height + height;
        _pix->resize(Rect(0, 0, _pix->bounds.right, new_height));

        if (_background.get()) {
            Rect new_area(0, old_height, _pix->bounds.right, new_height);
            Rect dest = _background->frame();
            dest.offset(0, _background_start);
            while (dest.top < _pix->bounds.bottom) {
                if (dest.bottom >= old_height) {
                    ClippedTransfer transfer(_background->frame(), dest);
                    transfer.ClipDestTo(new_area);
                    _background->draw_to(_pix, transfer.from(), transfer.to());
                }
                dest.offset(0, dest.height());
            }
        }
    }

    PixMap* _pix;

    scoped_ptr<Picture> _background;
    int _background_start;
};

PixMap* build_pix(int text_id, int width) {
    scoped_ptr<PixMap> pix(new PixMap(width, 0));
    ScrollTextPixBuilder build(pix.get());
    Resource text('TEXT', text_id);

    std::vector<std::string> lines;
    const char* start = text.data();
    const char* end = strchr(start, '\r');;
    while (end != NULL) {
        lines.push_back(std::string(start, end - start));
        start = end + 1;
        end = strchr(start, '\r');
    }
    lines.push_back(std::string(start, text.size() - (start - text.data())));

    for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); ++it) {
        if (it->substr(0, 2) == "#+") {
            if (it->size() > 2) {
                if ((*it)[2] == 'B') {
                    int id = string_to_int(it->substr(3));
                    build.set_background(id);
                } else {
                    int id = string_to_int(it->substr(2));
                    build.add_picture(id);
                }
            }
        } else {
            build.add_text(*it);
        }
    }

    return pix.release();
}

}  // namespace

ScrollTextScreen::ScrollTextScreen(int text_id, int width, double speed)
        : _pix_map(build_pix(text_id, width)),
          _speed(speed),
          _play_song(false),
          _song_id(0) { }

ScrollTextScreen::ScrollTextScreen(int text_id, int width, double speed, int song_id)
        : _pix_map(build_pix(text_id, width)),
          _speed(speed),
          _play_song(true),
          _song_id(song_id) { }

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

    ClearScreen();
    _start = now();
    _window = Rect(0, -kScrollTextHeight, _pix_map->bounds.right, 0);
}

void ScrollTextScreen::resign_front() {
    ClearScreen();

    // If a song was requested, stop it.
    if (_play_song && SongIsPlaying()) {
        StopAndUnloadSong();
    }
}

bool ScrollTextScreen::mouse_down(int button, const Point& where) {
    (void)button;
    (void)where;
    VideoDriver::driver()->pop_listener(this);
    return true;
}

bool ScrollTextScreen::key_down(int key) {
    (void)key;
    VideoDriver::driver()->pop_listener(this);
    return true;
}

double ScrollTextScreen::delay() {
    return 1.0 / 60.0;
}

void ScrollTextScreen::fire_timer() {
    int top = ((now() - _start) * _speed) - kScrollTextHeight;
    if (top > _window.top) {
        _window.offset(0, top - _window.top);
        Rect dest = _window;
        dest.center_in(gRealWorld->bounds);

        if (_window.intersects(_pix_map->bounds)) {
            ClearScreen();
            CopyBits(_pix_map.get(), gRealWorld, &_window, &dest, 0, NULL);
        } else {
            VideoDriver::driver()->pop_listener(this);
        }
    }
}

}  // namespace antares
