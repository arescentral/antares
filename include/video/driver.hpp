// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2011 Ares Central
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
// License along with this program.  If not, see
// <http://www.gnu.org/licenses/>.

#ifndef ANTARES_VIDEO_DRIVER_HPP_
#define ANTARES_VIDEO_DRIVER_HPP_

#include <stdint.h>
#include <sfz/sfz.hpp>

#include "drawing/color.hpp"
#include "math/geometry.hpp"

namespace antares {

class Card;
class KeyMap;
class PixMap;
class Sprite;

enum GameState {
    UNKNOWN,
    MAIN_SCREEN_INTERFACE,
    OPTIONS_INTERFACE,
    KEY_CONTROL_INTERFACE,
    SELECT_LEVEL_INTERFACE,
    MISSION_INTERFACE,
    PLAY_GAME,
    GAME_PAUSED,
    DONE_GAME,
};

class VideoDriver {
  public:
    VideoDriver();
    virtual ~VideoDriver();
    virtual bool button() = 0;
    virtual Point get_mouse() = 0;
    virtual void get_keys(KeyMap* k) = 0;

    virtual int ticks() = 0;
    virtual int usecs() = 0;
    virtual int64_t double_click_interval_usecs() = 0;

    virtual Sprite* new_sprite(sfz::PrintItem name, const PixMap& content) = 0;
    virtual void fill_rect(const Rect& rect, const RgbColor& color) = 0;
    virtual void draw_point(const Point& at, const RgbColor& color) = 0;
    virtual void draw_line(const Point& from, const Point& to, const RgbColor& color) = 0;

    virtual void start_stencil() = 0;
    virtual void set_stencil_threshold(uint8_t alpha) = 0;
    virtual void apply_stencil() = 0;
    virtual void end_stencil() = 0;

    static VideoDriver* driver();
};

class Stencil {
  public:
    Stencil(VideoDriver* driver);
    ~Stencil();

    void set_threshold(uint8_t alpha);
    void apply();

  private:
    VideoDriver* _driver;

    DISALLOW_COPY_AND_ASSIGN(Stencil);
};

class Sprite {
  public:
    virtual ~Sprite();
    virtual sfz::StringSlice name() const = 0;
    virtual void draw(int32_t x, int32_t y, const RgbColor& = RgbColor::kWhite) const = 0;
    virtual void draw(const Rect& draw_rect, const RgbColor& = RgbColor::kWhite) const = 0;
    virtual const Size& size() const = 0;
};

}  // namespace antares

#endif  // ANTARES_VIDEO_DRIVER_HPP_
