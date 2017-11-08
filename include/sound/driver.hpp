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

#ifndef ANTARES_SOUND_DRIVER_HPP_
#define ANTARES_SOUND_DRIVER_HPP_

#include <sfz/sfz.hpp>

namespace antares {

class Sound {
  public:
    Sound() {}
    virtual ~Sound() {}

    virtual void play() = 0;
    virtual void loop() = 0;

  private:
    DISALLOW_COPY_AND_ASSIGN(Sound);
};

class SoundChannel {
  public:
    SoundChannel() {}
    virtual ~SoundChannel() {}

    virtual void activate()          = 0;
    virtual void amp(uint8_t volume) = 0;
    virtual void quiet()             = 0;

  private:
    DISALLOW_COPY_AND_ASSIGN(SoundChannel);
};

class SoundDriver {
  public:
    SoundDriver();
    virtual ~SoundDriver();

    virtual std::unique_ptr<SoundChannel> open_channel()           = 0;
    virtual std::unique_ptr<Sound> open_sound(sfz::PrintItem path) = 0;
    virtual void set_global_volume(uint8_t volume)                 = 0;

    static SoundDriver* driver();

  private:
    DISALLOW_COPY_AND_ASSIGN(SoundDriver);
};

class NullSoundDriver : public SoundDriver {
  public:
    NullSoundDriver() {}

    virtual std::unique_ptr<SoundChannel> open_channel();
    virtual std::unique_ptr<Sound> open_sound(sfz::PrintItem path);
    virtual void set_global_volume(uint8_t volume);

  private:
    DISALLOW_COPY_AND_ASSIGN(NullSoundDriver);
};

class LogSoundDriver : public SoundDriver {
  public:
    LogSoundDriver(const sfz::StringSlice& path);

    virtual std::unique_ptr<SoundChannel> open_channel();
    virtual std::unique_ptr<Sound> open_sound(sfz::PrintItem path);
    virtual void set_global_volume(uint8_t volume);

  private:
    class LogSound;
    class LogChannel;

    sfz::ScopedFd _sound_log;
    int           _last_id;
    LogChannel*   _active_channel;
};

}  // namespace antares

#endif  // ANTARES_SOUND_DRIVER_HPP_
