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

#ifndef ANTARES_UI_EVENT_SCHEDULER_HPP_
#define ANTARES_UI_EVENT_SCHEDULER_HPP_

#include <vector>
#include <sfz/sfz.hpp>

#include "config/keys.hpp"
#include "math/units.hpp"
#include "ui/card.hpp"
#include "ui/event-tracker.hpp"
#include "ui/event.hpp"

namespace antares {

class EventScheduler {
  public:
    struct MainLoop {
        virtual ~MainLoop() { }
        virtual bool takes_snapshots() = 0;
        virtual void snapshot(int64_t ticks) = 0;
        virtual void draw() = 0;
        virtual bool done() const = 0;
        virtual Card* top() const = 0;
    };

    EventScheduler();

    void schedule_snapshot(int64_t at);
    void schedule_event(std::unique_ptr<Event> event);
    void schedule_key(int32_t key, int64_t down, int64_t up);
    void schedule_mouse(int button, const Point& where, int64_t down, int64_t up);

    void loop(MainLoop& loop);

    bool button(int which) const { return _event_tracker.button(which); }
    Point get_mouse() const { return _event_tracker.mouse(); }
    void get_keys(KeyMap* k) const { k->copy(_event_tracker.keys()); }
    InputMode input_mode() const { return KEYBOARD_MOUSE; }
    int ticks() const { return _ticks; }
    int64_t usecs() const { return ticks_to_usecs(_ticks); }

  private:
    void advance_tick_count(MainLoop& loop, int64_t ticks);
    bool have_snapshots_before(int64_t ticks) const;

    static bool is_later(const std::unique_ptr<Event>& x, const std::unique_ptr<Event>& y);

    int64_t _ticks;
    std::vector<int64_t> _snapshot_times;
    std::vector<std::unique_ptr<Event>> _event_heap;
    EventTracker _event_tracker;

    DISALLOW_COPY_AND_ASSIGN(EventScheduler);
};

}  // namespace antares

#endif  // ANTARES_UI_EVENT_SCHEDULER_HPP_
