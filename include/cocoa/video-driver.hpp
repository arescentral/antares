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

#ifndef ANTARES_COCOA_VIDEO_DRIVER_HPP_
#define ANTARES_COCOA_VIDEO_DRIVER_HPP_

#include <queue>
#include <stack>
#include <sfz/sfz.hpp>

#include "cocoa/c/CocoaVideoDriver.h"
#include "config/keys.hpp"
#include "drawing/color.hpp"
#include "math/geometry.hpp"
#include "ui/event-tracker.hpp"
#include "video/opengl-driver.hpp"

namespace antares {

class Event;

class CocoaVideoDriver : public OpenGlVideoDriver {
  public:
    CocoaVideoDriver(bool fullscreen, Size screen_size);
    virtual bool button();
    virtual Point get_mouse();
    virtual void get_keys(KeyMap* k);

    virtual int ticks();
    virtual int usecs();
    virtual int64_t double_click_interval_usecs();

    void loop(Card* initial);

  private:
    bool wait_next_event(int64_t until, sfz::scoped_ptr<Event>& event);
    void enqueue_events(int64_t until);

    const bool _fullscreen;
    int64_t _start_time;

    class EventTranslator {
      public:
        EventTranslator(int32_t screen_width, int32_t screen_height):
                _c_obj(antares_event_translator_create(screen_width, screen_height)) { }
        ~EventTranslator() { antares_event_translator_destroy(_c_obj); }
        AntaresEventTranslator* c_obj() const { return _c_obj; }
      private:
        AntaresEventTranslator* _c_obj;
        DISALLOW_COPY_AND_ASSIGN(EventTranslator);
    };
    EventTranslator _translator;

    EventTracker _event_tracker;
    std::queue<Event*> _event_queue;

    DISALLOW_COPY_AND_ASSIGN(CocoaVideoDriver);
};

}  // namespace antares

#endif  // ANTARES_COCOA_VIDEO_DRIVER_HPP_
