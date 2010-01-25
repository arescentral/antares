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

#ifndef ANTARES_FAKE_SOUNDS_HPP_
#define ANTARES_FAKE_SOUNDS_HPP_

#include "sfz/String.hpp"
#include "sfz/ScopedFd.hpp"
#include "Base.h"

namespace antares {

class SoundDriver {
  public:
    virtual ~SoundDriver() { }
    virtual void play(int32_t channel, int32_t id) = 0;
    virtual void amp(int32_t channel, uint8_t volume) = 0;
    virtual void quiet(int32_t channel) = 0;

    static void set_driver(SoundDriver* driver);
};

class NullSoundDriver : public SoundDriver {
  public:
    virtual void play(int32_t, int32_t);
    virtual void amp(int32_t, uint8_t);
    virtual void quiet(int32_t);
};

class LogSoundDriver : public SoundDriver {
  public:
    LogSoundDriver(const sfz::StringPiece& path);
    virtual void play(int32_t channel, int32_t id);
    virtual void amp(int32_t channel, uint8_t volume);
    virtual void quiet(int32_t channel);

  private:
    sfz::ScopedFd _sound_log;
};

struct Sound {
    Sound(int id) : id(id) { }

    int id;
};

struct SndChannel;

Sound* GetSound(int id);
OSErr SndPlay(SndChannel* channel, Sound* sound, bool);

}  // namespace antares

#endif  // ANTARES_FAKE_SOUNDS_HPP_
