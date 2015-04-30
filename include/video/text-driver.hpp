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

#ifndef ANTARES_VIDEO_TEXT_DRIVER_HPP_
#define ANTARES_VIDEO_TEXT_DRIVER_HPP_

#include <sfz/sfz.hpp>

#include "config/keys.hpp"
#include "ui/event-scheduler.hpp"
#include "video/driver.hpp"

namespace antares {

class TextVideoDriver : public VideoDriver {
  public:
    TextVideoDriver(
            Size screen_size, EventScheduler& scheduler,
            const sfz::Optional<sfz::String>& output_dir);

    virtual bool button(int which) { return _scheduler.button(which); }
    virtual Point get_mouse() { return _scheduler.get_mouse(); }
    virtual void get_keys(KeyMap* k) { _scheduler.get_keys(k); }
    virtual InputMode input_mode() const { return KEYBOARD_MOUSE; }

    virtual int ticks() const { return _scheduler.ticks(); }
    virtual int usecs() const { return _scheduler.usecs(); }
    virtual int64_t double_click_interval_usecs() const { return 0.5e6; }

    virtual std::unique_ptr<antares::Sprite> new_sprite(sfz::PrintItem name, const PixMap& content);
    virtual void fill_rect(const Rect& rect, const RgbColor& color);
    virtual void dither_rect(const Rect& rect, const RgbColor& color);
    virtual void draw_point(const Point& at, const RgbColor& color);
    virtual void draw_line(const Point& from, const Point& to, const RgbColor& color);
    virtual void draw_triangle(const Rect& rect, const RgbColor& color);
    virtual void draw_diamond(const Rect& rect, const RgbColor& color);
    virtual void draw_plus(const Rect& rect, const RgbColor& color);

    void loop(Card* initial);

  private:
    class MainLoop;
    class Sprite;

    virtual void batch_line(const Point& from, const Point& to, const RgbColor& color);

    void add_arg(sfz::StringSlice arg, std::vector<std::pair<size_t, size_t>>& args);
    void dup_arg(size_t index, std::vector<std::pair<size_t, size_t>>& args);
    sfz::StringSlice last_arg(size_t index) const;

    template <int size>
    void log(sfz::StringSlice command, sfz::PrintItem (&args)[size]);

    const Size _size;
    EventScheduler& _scheduler;
    const sfz::Optional<sfz::String> _output_dir;

    sfz::String _log;
    std::vector<std::pair<size_t, size_t>> _last_args;

    DISALLOW_COPY_AND_ASSIGN(TextVideoDriver);
};

}  // namespace antares

#endif  // ANTARES_VIDEO_TEXT_DRIVER_HPP_
