// Ares, a tactical space combat game.
// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#ifndef ANTARES_OPTIONS_SCREEN_HPP_
#define ANTARES_OPTIONS_SCREEN_HPP_

#include <utility>
#include <vector>
#include "sfz/SmartPtr.hpp"
#include "InterfaceScreen.hpp"
#include "Preferences.hpp"

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
    State _state;
    sfz::scoped_ptr<Preferences> _preferences;
};

class SoundControlScreen : public InterfaceScreen {
  public:
    SoundControlScreen(OptionsScreen::State* state, Preferences* preferences);
    ~SoundControlScreen();

    virtual void become_front();

  protected:
    virtual void adjust_interface();
    virtual void handle_button(int button);
    virtual void draw() const;

  private:
    enum Item {
        // Checkboxes
        GAME_MUSIC = 0,
        IDLE_MUSIC = 1,
        SPEECH_ON = 4,

        // Volume Control
        VOLUME_UP = 2,
        VOLUME_DOWN = 3,

        // Buttons
        CANCEL = 5,
        DONE = 6,
        KEY_CONTROL = 7,

        // Other
        VOLUME_BOX = 13,
    };

    OptionsScreen::State button_state(int button);

    OptionsScreen::State* const _state;
    Preferences* const _preferences;

    DISALLOW_COPY_AND_ASSIGN(SoundControlScreen);
};

class KeyControlScreen : public InterfaceScreen {
  public:
    KeyControlScreen(OptionsScreen::State* state, Preferences* preferences);
    ~KeyControlScreen();

    virtual void become_front();

    virtual void key_down(const KeyDownEvent& event);
    virtual void key_up(const KeyUpEvent& event);

    virtual double next_timer();
    virtual void fire_timer();

  protected:
    virtual void adjust_interface();
    virtual void handle_button(int button);
    virtual void draw() const;

  private:
    enum Item {
        CANCEL = 0,
        DONE = 1,
        SOUND_CONTROL = 2,

        SHIP_TAB = 3,
        COMMAND_TAB = 4,
        SHORTCUT_TAB = 5,
        UTILITY_TAB = 6,
        HOT_KEY_TAB = 7,

        CONFLICT_TEXT = 10,
        TAB_BOX = 8,
    };

    enum Tab {
        SHIP,
        COMMAND,
        SHORTCUT,
        UTILITY,
        HOT_KEY,
    };

    OptionsScreen::State button_state(int button);
    Tab button_tab(int button);
    void set_tab(Tab tab);
    void update_conflicts();
    void flash_on(size_t key);

    OptionsScreen::State* const _state;
    Preferences* const _preferences;

    Tab _tab;
    const size_t _key_start;
    size_t _selected_key;
    std::vector<std::pair<size_t, size_t> > _conflicts;

    double _next_flash;
    bool _flashed_on;

    DISALLOW_COPY_AND_ASSIGN(KeyControlScreen);
};

}  // namespace antares

#endif  // ANTARES_OPTIONS_SCREEN_HPP_
