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

class Label {
  public:
    static const int32_t kNone = -1;
    static const int32_t kMaxLabelNum = 16;
    static const int32_t kVisibleTime = 60;

    static Label* get(int number);
    static Handle<Label> none() { return Handle<Label>(-1); }
    static HandleList<Label> all() { return {0, kMaxLabelNum}; }

    static void init();
    static void reset();
    static Handle<Label> add(
            int16_t h, int16_t v, int16_t hoff, int16_t voff, Handle<SpaceObject> object,
            bool objectLink, uint8_t color);
    static void remove(Handle<Label> which);
    static void draw();
    static void update_contents(int32_t units_done);
    static void update_positions(int32_t units_done);
    static void show_all();

    static void set_position(Handle<Label> which, int16_t h, int16_t v);
    static void set_object(Handle<Label> which, Handle<SpaceObject> object);
    static void set_age(Handle<Label> which, int32_t age);
    static void set_string(Handle<Label> which, const sfz::StringSlice& string);
    static void clear_string(Handle<Label> which);
    static void set_color(Handle<Label> which, uint8_t color);
    static void set_offset(Handle<Label> which, int32_t hoff, int32_t voff);
    static int32_t get_width(Handle<Label> which);
    static void set_keep_on_screen_anyway(Handle<Label> which, bool keepOnScreenAnyWay);
    static void set_attached_hint_line(Handle<Label> which, bool attachedHintLine, Point toWhere);
    static sfz::String* get_string(Handle<Label> which);  // TODO(sfiera): encapsulate.

    static void recalc_size(Handle<Label> which);

  private:
    static Handle<Label> next_free_label();

    Point               where;
    Point               offset;
    Rect                thisRect = Rect(0, 0, -1, -1);
    int32_t             width;
    int32_t             height;
    int32_t             age = 0;
    sfz::String         text;
    uint8_t             color;
    bool                active = false;
    bool                killMe = false;
    bool                visible = false;
    Handle<SpaceObject> object;
    bool                objectLink = true;     // true if label requires an object to be seen
    int32_t             lineNum = 1;
    int32_t             lineHeight;
    bool                keepOnScreenAnyway = false; // if not attached to object, keep on screen if it's off
    bool                attachedHintLine = false;
    Point               attachedToWhere;
    int32_t             retroCount = -1;
};

}  // namespace antares

#endif // ANTARES_GAME_LABELS_HPP_
