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

#ifndef ANTARES_VIDEO_TEXT_DRIVER_HPP_
#define ANTARES_VIDEO_TEXT_DRIVER_HPP_

#include <sfz/sfz.hpp>
#include <vector>

#include "config/keys.hpp"
#include "ui/event-scheduler.hpp"
#include "video/driver.hpp"

namespace antares {

class TextVideoDriver : public VideoDriver {
  public:
    TextVideoDriver(Size screen_size, const sfz::optional<pn::string>& output_dir);

    virtual Point     get_mouse() { return _scheduler->get_mouse(); }
    virtual InputMode input_mode() const { return KEYBOARD_MOUSE; }
    virtual int       scale() const;
    virtual Size      screen_size() const { return _size; }

    virtual bool start_editing(TextReceiver* text);
    virtual void stop_editing(TextReceiver* text);

    virtual wall_time now() const { return _scheduler->now(); }

    virtual Texture texture(pn::string_view name, const PixMap& content, int scale);
    virtual void    dither_rect(const Rect& rect, const RgbColor& color);
    virtual void    draw_triangle(const Rect& rect, const RgbColor& color);
    virtual void    draw_diamond(const Rect& rect, const RgbColor& color);
    virtual void    draw_plus(const Rect& rect, const RgbColor& color);

    void loop(Card* initial, EventScheduler& scheduler);
    void capture(std::vector<std::pair<std::unique_ptr<Card>, pn::string>>& pix);

  private:
    class MainLoop;
    class TextureImpl;

    virtual void batch_point(const Point& at, const RgbColor& color);
    virtual void batch_line(const Point& from, const Point& to, const RgbColor& color);
    virtual void batch_rect(const Rect& rect, const RgbColor& color);

    void            add_arg(pn::string_view arg, std::vector<std::pair<size_t, size_t>>& args);
    void            dup_arg(size_t index, std::vector<std::pair<size_t, size_t>>& args);
    pn::string_view last_arg(size_t index) const;

    template <typename... Args>
    void log(pn::string_view command, const Args&... args);

    const Size                _size;
    sfz::optional<pn::string> _output_dir;

    pn::string                             _log;
    std::vector<std::pair<size_t, size_t>> _last_args;

    EventScheduler* _scheduler = nullptr;
};

}  // namespace antares

#endif  // ANTARES_VIDEO_TEXT_DRIVER_HPP_
