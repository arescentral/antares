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

#ifndef ANTARES_VIDEO_OFFSCREEN_DRIVER_HPP_
#define ANTARES_VIDEO_OFFSCREEN_DRIVER_HPP_

#include <queue>
#include <stack>
#include <sfz/sfz.hpp>

#include "config/keys.hpp"
#include "math/geometry.hpp"
#include "ui/card.hpp"
#include "ui/event-tracker.hpp"
#include "ui/event.hpp"
#include "video/opengl-driver.hpp"

namespace antares {

class OffscreenVideoDriver : public OpenGlVideoDriver {
  public:
    OffscreenVideoDriver(Size screen_size, const sfz::Optional<sfz::String>& output_dir);

    virtual bool button() { return _event_tracker.button(); }
    virtual Point get_mouse() { return _event_tracker.mouse(); }
    virtual void get_keys(KeyMap* k) { k->copy(_event_tracker.keys()); }

    virtual int ticks() { return _ticks; }
    virtual int64_t double_click_interval_usecs() { return 0.5e6; }

    void loop(Card* initial);

    void schedule_snapshot(int64_t at);
    void schedule_event(sfz::linked_ptr<Event> event);
    void schedule_key(int32_t key, int64_t down, int64_t up);
    void schedule_mouse(int button, const Point& where, int64_t down, int64_t up);

  private:
    class MainLoop;

    void advance_tick_count(MainLoop* loop, int64_t ticks);
    bool have_snapshots_before(int64_t ticks) const;

    const sfz::Optional<sfz::String> _output_dir;
    int64_t _ticks;

    EventTracker _event_tracker;

    static bool is_later(const sfz::linked_ptr<Event>& x, const sfz::linked_ptr<Event>& y);
    std::vector<sfz::linked_ptr<Event> > _event_heap;

    std::vector<int64_t> _snapshot_times;

    DISALLOW_COPY_AND_ASSIGN(OffscreenVideoDriver);
};

}  // namespace antares

#endif  // ANTARES_VIDEO_OFFSCREEN_DRIVER_HPP_
