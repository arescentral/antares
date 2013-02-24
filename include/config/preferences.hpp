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

#ifndef ANTARES_CONFIG_PREFERENCES_HPP_
#define ANTARES_CONFIG_PREFERENCES_HPP_

#include <sfz/sfz.hpp>

#include "math/geometry.hpp"

namespace antares {

class Preferences {
  public:
    static Preferences* preferences();

    Preferences();
    Preferences(const Preferences& other);
    Preferences& operator=(const Preferences& other);
    ~Preferences();

    void reset();
    void copy(const Preferences& preferences);

    uint32_t key(size_t index) const;
    bool play_idle_music() const;
    bool play_music_in_game() const;
    bool speech_on() const;
    int volume() const;
    bool fullscreen() const;
    Size screen_size() const;
    sfz::StringSlice scenario_identifier() const;

    void set_key(size_t index, uint32_t key);
    void set_play_idle_music(bool on);
    void set_play_music_in_game(bool on);
    void set_speech_on(bool on);
    void set_volume(int volume);
    void set_fullscreen(bool fullscreen);
    void set_screen_size(Size size);
    void set_scenario_identifier(sfz::StringSlice id);

  private:
    static std::unique_ptr<Preferences> _preferences;

    int16_t             _key_map[44];
    bool                _play_idle_music;
    bool                _play_music_in_game;
    bool                _speech_on;
    int16_t             _volume;
    bool                _fullscreen;
    Size                _screen_size;
    sfz::String         _scenario_identifier;
};

class PrefsDriver {
  public:
    PrefsDriver();
    virtual ~PrefsDriver();

    virtual void load(Preferences* preferences) = 0;
    virtual void save(const Preferences& preferences) = 0;

    static PrefsDriver* driver();
};

class NullPrefsDriver : public PrefsDriver {
  public:
    NullPrefsDriver();
    NullPrefsDriver(Preferences defaults);

    virtual void load(Preferences* preferences);
    virtual void save(const Preferences& preferences);

  private:
    Preferences _saved;
};

}  // namespace antares

#endif // ANTARES_CONFIG_PREFERENCES_HPP_
