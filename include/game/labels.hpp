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

#ifndef ANTARES_GAME_LABELS_HPP_
#define ANTARES_GAME_LABELS_HPP_

#include <sfz/sfz.hpp>

#include "data/space-object.hpp"

namespace antares {

class Labels {
  public:
    static const int32_t kNone = -1;
    static const int32_t kVisibleTime = 60;

    static void init();
    static void reset();
    static short add(
            short h, short v, short hoff, short voff, spaceObjectType* object, bool objectLink,
            unsigned char color);
    static void remove( long);
    static void draw();
    static void update_contents(int32_t units_done);
    static void update_positions(int32_t units_done);
    static void show_all();

    static void set_position( long, short, short);
    static void set_object( long, spaceObjectType *);
    static void set_age( long, long);
    static void set_string(long which, const sfz::StringSlice& string);
    static void clear_string(long which);
    static void set_color( long, unsigned char);
    static void set_offset( long which, long hoff, long voff);
    static long get_width( long which);
    static void set_keep_on_screen_anyway( long which, bool keepOnScreenAnyWay);
    static void set_attached_hint_line( long which, bool attachedHintLine, Point toWhere);
    static sfz::String* get_string(long);  // TODO(sfiera): encapsulate.

    static void recalc_size( long);

  private:
    struct screenLabelType;
    static void zero(screenLabelType& label);
    static screenLabelType* data;
};

}  // namespace antares

#endif // ANTARES_GAME_LABELS_HPP_
