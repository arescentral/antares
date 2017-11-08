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

#ifndef ANTARES_GAME_MESSAGES_HPP_
#define ANTARES_GAME_MESSAGES_HPP_

#include <queue>
#include <sfz/sfz.hpp>

#include "data/handle.hpp"
#include "drawing/color.hpp"
#include "drawing/styled-text.hpp"
#include "math/geometry.hpp"

namespace antares {

const int16_t kMessageStringID    = 3100;
const int16_t kZoomStringOffset   = 1;
const int16_t kAutoPilotOnString  = 9;
const int16_t kAutoPilotOffString = 10;

const uint8_t kStatusLabelColor = AQUA;
const uint8_t kStatusWarnColor  = PINK;

class Messages {
  public:
    static void init();
    static void clear();
    static void add(const sfz::PrintItem& message);
    static void start(int16_t, int16_t);
    static void clip();
    static void end();
    static void advance();
    static void previous();
    static void replay();
    static void set_status(const sfz::StringSlice& status, uint8_t color);
    static int16_t current();

    static void draw_long_message(ticks time_pass);
    static void draw_message_screen(ticks by_units);
    static void draw_message();

  private:
    struct longMessageType;

    static std::queue<sfz::String> message_data;
    static longMessageType*        long_message_data;
    static ticks                   time_count;
};

}  // namespace antares

#endif  // ANTARES_GAME_MESSAGES_HPP_
