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

#ifndef ANTARES_SOUND_DRIVER_HPP_
#define ANTARES_SOUND_DRIVER_HPP_

#include <sfz/sfz.hpp>

namespace antares {

class SndChannel;
class Sound;

class SoundDriver {
  public:
    virtual ~SoundDriver() { }

    virtual SndChannel* new_channel() = 0;
    virtual Sound* new_sound(int id) = 0;
    virtual Sound* new_song(int id) = 0;

    static SoundDriver* driver();
    static void set_driver(SoundDriver* driver);
};

class Sound {
  public:
    Sound(int id) : _id(id) { }
    virtual ~Sound() { }

    int id() const { return _id; }

  private:
    int _id;

    DISALLOW_COPY_AND_ASSIGN(Sound);
};

class SndChannel {
  public:
    virtual ~SndChannel() { }

    virtual void play(Sound* sound) = 0;
    virtual void loop(Sound* sound) = 0;
    virtual void amp(uint8_t volume) = 0;
    virtual void quiet() = 0;
};

class NullSoundDriver : public SoundDriver {
  public:
    virtual SndChannel* new_channel();
    virtual Sound* new_sound(int id);
    virtual Sound* new_song(int id);
};

class LogSoundDriver : public SoundDriver {
  public:
    LogSoundDriver(const sfz::StringPiece& path);
    virtual SndChannel* new_channel();
    virtual Sound* new_sound(int id);
    virtual Sound* new_song(int id);

  private:
    sfz::ScopedFd _sound_log;
    int _last_id;
};

}  // namespace antares

#endif  // ANTARES_SOUND_DRIVER_HPP_
