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

#ifndef ANTARES_UI_SCREENS_OPTIONS_HPP_
#define ANTARES_UI_SCREENS_OPTIONS_HPP_

#include <utility>
#include <vector>

#include "config/preferences.hpp"
#include "math/units.hpp"
#include "ui/screen.hpp"

namespace antares {

class OptionsScreen : public Card {
  public:
    enum State {
        SOUND_CONTROL,
        KEY_CONTROL,
        ACCEPT,
        CANCEL,
    };

    OptionsScreen();
    virtual void become_front();

  private:
    State       _state;
    Preferences _revert;
};

class SoundControlScreen : public InterfaceScreen {
  public:
    SoundControlScreen(OptionsScreen::State* state);
    ~SoundControlScreen();

    virtual void overlay() const;

  private:
    OptionsScreen::State* const _state;
};

class KeyControlScreen : public InterfaceScreen {
  public:
    KeyControlScreen(OptionsScreen::State* state);
    ~KeyControlScreen();

    virtual void key_down(const KeyDownEvent& event);
    virtual void key_up(const KeyUpEvent& event);

    virtual bool next_timer(wall_time& time);
    virtual void fire_timer();

    virtual void overlay() const;

  private:
    void adjust_interface();

    void update_conflicts();
    void flash_on(size_t key);

    OptionsScreen::State* const _state;

    int32_t                                _selected_key;
    std::vector<std::pair<size_t, size_t>> _conflicts;

    wall_time _next_flash;
    bool      _flashed_on;

    std::vector<pn::string> _tabs;
    std::vector<pn::string> _keys;
};

}  // namespace antares

#endif  // ANTARES_UI_SCREENS_OPTIONS_HPP_
