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

#ifndef ANTARES_VIDEO_OFFSCREEN_DRIVER_HPP_
#define ANTARES_VIDEO_OFFSCREEN_DRIVER_HPP_

#include <pn/string>
#include <sfz/sfz.hpp>

#include "config/keys.hpp"
#include "ui/event-scheduler.hpp"
#include "video/opengl-driver.hpp"

namespace antares {

class OffscreenVideoDriver : public OpenGlVideoDriver {
    class MainLoop;

  public:
    OffscreenVideoDriver(Size screen_size, const sfz::optional<pn::string>& output_dir);

    virtual Size viewport_size() const { return _screen_size; }
    virtual Size screen_size() const { return _screen_size; }

    virtual Point     get_mouse() { return _scheduler->get_mouse(); }
    virtual InputMode input_mode() const { return _scheduler->input_mode(); }

    virtual wall_time now() const { return _scheduler->now(); }

    void loop(Card* initial, EventScheduler& scheduler);
    void capture(std::vector<std::pair<std::unique_ptr<Card>, pn::string>>& pix);
    void set_capture_rect(Rect r) { _capture_rect = r; }

  private:
    const Size                _screen_size;
    sfz::optional<pn::string> _output_dir;
    Rect                      _capture_rect;

    EventScheduler* _scheduler = nullptr;
};

}  // namespace antares

#endif  // ANTARES_VIDEO_OFFSCREEN_DRIVER_HPP_
