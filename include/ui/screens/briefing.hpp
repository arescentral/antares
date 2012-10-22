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

#ifndef ANTARES_UI_SCREENS_BRIEFING_HPP_
#define ANTARES_UI_SCREENS_BRIEFING_HPP_

#include <vector>
#include <sfz/sfz.hpp>

#include "math/geometry.hpp"
#include "drawing/interface.hpp"
#include "ui/screen.hpp"

namespace antares {

class Scenario;
class Sprite;

class BriefingScreen : public InterfaceScreen {
  public:
    BriefingScreen(const Scenario* scenario, bool* cancelled);
    ~BriefingScreen();

    virtual void become_front();
    virtual void draw() const;

    virtual void mouse_down(const MouseDownEvent& event);
    virtual void key_down(const KeyDownEvent& event);

  protected:
    virtual void adjust_interface();
    virtual void handle_button(int button);

  private:
    enum Item {
        // Buttons:
        DONE = 0,
        PREVIOUS = 1,
        NEXT = 2,

        // Map area:
        MAP_RECT = 7,
    };

    void build_star_map();
    void build_system_map();
    void build_brief_point();

    void show_object_data_key(int index, int key);

    const Scenario* const _scenario;
    bool* const _cancelled;
    int _briefing_point;
    const int _briefing_point_count;
    mutable interfaceItemType _data_item;

    Rect _bounds;
    sfz::scoped_ptr<Sprite> _star_map;
    sfz::scoped_ptr<Sprite> _system_map;
    sfz::scoped_ptr<Sprite> _brief_point;
    std::vector<inlinePictType> _inline_pict;
    sfz::String _text;

    DISALLOW_COPY_AND_ASSIGN(BriefingScreen);
};

}  // namespace antares

#endif  // ANTARES_UI_SCREENS_BRIEFING_HPP_
