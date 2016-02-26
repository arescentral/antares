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

#ifndef ANTARES_MAC_VIDEO_DRIVER_HPP_
#define ANTARES_MAC_VIDEO_DRIVER_HPP_

#include <queue>
#include <stack>
#include <sfz/sfz.hpp>

#include "config/keys.hpp"
#include "drawing/color.hpp"
#include "mac/c/CocoaVideoDriver.h"
#include "math/geometry.hpp"
#include "ui/event-tracker.hpp"
#include "video/opengl-driver.hpp"

namespace antares {

class Event;

class CocoaVideoDriver : public OpenGlVideoDriver {
  public:
    CocoaVideoDriver(Size screen_size);

    virtual Size viewport_size() const { return _viewport_size; }
    virtual Size screen_size() const { return _screen_size; }

    virtual Point get_mouse();
    virtual void get_keys(KeyMap* k);
    virtual InputMode input_mode() const;

    virtual int ticks() const;
    virtual int usecs() const;

    void loop(Card* initial);

  private:
    const Size _screen_size;
    Size _viewport_size;
    int64_t _start_time;

    struct EventBridge;

    class EventTranslator {
      public:
        EventTranslator(): _c_obj(antares_event_translator_create()) { }
        ~EventTranslator() { antares_event_translator_destroy(_c_obj); }
        AntaresEventTranslator* c_obj() const { return _c_obj; }
      private:
        AntaresEventTranslator* _c_obj;
        DISALLOW_COPY_AND_ASSIGN(EventTranslator);
    };
    EventTranslator _translator;

    EventTracker _event_tracker;
    AntaresWindow* _window = nullptr;

    DISALLOW_COPY_AND_ASSIGN(CocoaVideoDriver);
};

}  // namespace antares

#endif  // ANTARES_MAC_VIDEO_DRIVER_HPP_
