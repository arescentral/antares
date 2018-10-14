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

#ifndef ANTARES_UI_SCREENS_BRIEFING_HPP_
#define ANTARES_UI_SCREENS_BRIEFING_HPP_

#include <vector>

#include "drawing/interface.hpp"
#include "math/geometry.hpp"
#include "ui/screen.hpp"
#include "ui/screens/object-data.hpp"

namespace antares {

union Level;

class BriefingScreen : public InterfaceScreen {
  public:
    BriefingScreen(const Level& level, bool* cancelled);
    ~BriefingScreen();

    virtual void become_front();
    virtual void overlay() const;

    virtual void mouse_down(const MouseDownEvent& event);
    virtual void key_down(const KeyDownEvent& event);
    virtual void gamepad_button_down(const GamepadButtonDownEvent& event);

  private:
    enum Item {
        // Buttons:
        DONE     = 0,
        PREVIOUS = 1,
        NEXT     = 2,

        // Map area:
        MAP_RECT = 7,
    };

    void build_star_map();
    void build_brief_point();

    void draw_system_map() const;
    void draw_brief_point() const;

    void show_object_data(int index, const KeyDownEvent& event);
    void show_object_data(int index, const GamepadButtonDownEvent& event);
    void show_object_data(
            int index, ObjectDataScreen::Trigger trigger, int mouse, Key key,
            Gamepad::Button gamepad);

    const Level&    _level;
    bool* const     _cancelled;
    int             _briefing_point;
    const int       _briefing_point_start;
    const int       _briefing_point_end;
    mutable BoxRect _data_item;

    Rect _bounds;
    Rect _star_rect;

    struct Star {
        Point   location;
        uint8_t shade;
    };
    std::vector<Star>                    _system_stars;
    std::vector<inlinePictType>          _inline_pict;
    Rect                                 _highlight_rect;
    std::vector<std::pair<Point, Point>> _highlight_lines;
    pn::string                           _text;
};

}  // namespace antares

#endif  // ANTARES_UI_SCREENS_BRIEFING_HPP_
