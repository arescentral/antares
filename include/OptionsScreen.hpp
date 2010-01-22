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

#include "sfz/SmartPtr.hpp"
#include "InterfaceScreen.hpp"
#include "AresPreferences.hpp"

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

    OptionsScreen::State* const _state;
    Preferences* const _preferences;

    DISALLOW_COPY_AND_ASSIGN(SoundControlScreen);
};

}  // namespace antares

#endif  // ANTARES_OPTIONS_SCREEN_HPP_
