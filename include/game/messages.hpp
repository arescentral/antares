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

#include <pn/string>
#include <queue>

#include "data/handle.hpp"
#include "drawing/color.hpp"
#include "drawing/styled-text.hpp"
#include "game/globals.hpp"
#include "math/geometry.hpp"

namespace antares {

class Messages {
  public:
    static void init();
    static void clear();
    static void add(pn::string_view message);
    static void start(sfz::optional<int64_t> start_id, const std::vector<pn::string>* pages);
    static void clip();
    static void end();
    static void advance();
    static void previous();
    static void replay();
    static std::pair<sfz::optional<int64_t>, int> current();

    static void zoom(Zoom zoom);
    static void autopilot(bool on);
    static void shields_low();
    static void max_ships_built();

    static void draw_long_message(ticks time_pass);
    static void draw_message_screen(ticks by_units);
    static void draw_message();

    static pn::string_view pause_string();

  private:
    struct longMessageType;

    static void set_status(pn::string_view status, Hue hue);

    static std::queue<pn::string> message_data;
    static longMessageType*       long_message_data;
    static ticks                  time_count;
};

}  // namespace antares

#endif  // ANTARES_GAME_MESSAGES_HPP_
