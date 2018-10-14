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

#ifndef ANTARES_MAC_VIDEO_DRIVER_HPP_
#define ANTARES_MAC_VIDEO_DRIVER_HPP_

#include <queue>
#include <stack>

#include "config/keys.hpp"
#include "drawing/color.hpp"
#include "mac/c/CocoaVideoDriver.h"
#include "math/geometry.hpp"
#include "video/opengl-driver.hpp"

namespace antares {

class Event;

class CocoaVideoDriver : public OpenGlVideoDriver {
  public:
    CocoaVideoDriver();

    virtual Size viewport_size() const;
    virtual Size screen_size() const;

    virtual Point     get_mouse();
    virtual InputMode input_mode() const;

    virtual wall_time now() const;

    void loop(Card* initial);

  private:
    static wall_time _now();

    struct EventBridge;

    class EventTranslator {
      public:
        EventTranslator() : _c_obj(antares_event_translator_create()) {}
        EventTranslator(const EventTranslator&) = delete;
        EventTranslator& operator=(const EventTranslator&) = delete;
        ~EventTranslator() { antares_event_translator_destroy(_c_obj); }
        AntaresEventTranslator* c_obj() const { return _c_obj; }

      private:
        AntaresEventTranslator* _c_obj;
    };
    EventTranslator _translator;

    InputMode      _input_mode = KEYBOARD_MOUSE;
    AntaresWindow* _window     = nullptr;
};

}  // namespace antares

#endif  // ANTARES_MAC_VIDEO_DRIVER_HPP_
