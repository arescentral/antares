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

#ifndef ANTARES_PREFERENCES_HPP_
#define ANTARES_PREFERENCES_HPP_

#include "sfz/SmartPtr.hpp"
#include "Base.h"

namespace antares {

#define kUsePublicCopyProtection

class Preferences {
  public:
    static Preferences* preferences();
    static void set_preferences(Preferences* preferences);

    Preferences();
    ~Preferences();

    void reset();
    void copy(const Preferences& preferences);

    uint32_t key(size_t index) const;
    bool play_idle_music() const;
    bool play_music_in_game() const;
    bool speech_on() const;
    int volume() const;

    void set_key(size_t index, uint32_t key);
    void set_play_idle_music(bool on);
    void set_play_music_in_game(bool on);
    void set_speech_on(bool on);
    void set_volume(int volume);

    size_t load_data(const char* data, size_t len);

  private:
    static sfz::scoped_ptr<Preferences> _preferences;

    int16_t             _key_map[44];
    uint32_t            _options;
    int16_t             _volume;
};

}  // namespace antares

#endif // ANTARES_PREFERENCES_HPP_
