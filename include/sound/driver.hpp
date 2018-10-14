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

#include <memory>
#include <pn/file>
#include <pn/string>

namespace antares {

class Sound {
  public:
    Sound() {}
    Sound(const Sound&) = delete;
    Sound& operator=(const Sound&) = delete;

    virtual ~Sound() {}

    virtual void play(uint8_t volume) = 0;
    virtual void loop(uint8_t volume) = 0;
};

class SoundChannel {
  public:
    SoundChannel() {}
    SoundChannel(const SoundChannel&) = delete;
    SoundChannel& operator=(const SoundChannel&) = delete;

    virtual ~SoundChannel() {}

    virtual void activate() = 0;
    virtual void quiet()    = 0;
};

class SoundDriver {
  public:
    SoundDriver();
    SoundDriver(const SoundDriver&) = delete;
    SoundDriver& operator=(const SoundDriver&) = delete;

    virtual ~SoundDriver();

    virtual std::unique_ptr<SoundChannel> open_channel()                    = 0;
    virtual std::unique_ptr<Sound>        open_sound(pn::string_view path)  = 0;
    virtual std::unique_ptr<Sound>        open_music(pn::string_view path)  = 0;
    virtual void                          set_global_volume(uint8_t volume) = 0;

    static SoundDriver* driver();
};

class NullSoundDriver : public SoundDriver {
  public:
    NullSoundDriver() {}
    NullSoundDriver(const NullSoundDriver&) = delete;
    NullSoundDriver& operator=(const NullSoundDriver&) = delete;

    virtual std::unique_ptr<SoundChannel> open_channel();
    virtual std::unique_ptr<Sound>        open_sound(pn::string_view path);
    virtual std::unique_ptr<Sound>        open_music(pn::string_view path);
    virtual void                          set_global_volume(uint8_t volume);
};

class LogSoundDriver : public SoundDriver {
  public:
    LogSoundDriver(pn::string_view path);

    virtual std::unique_ptr<SoundChannel> open_channel();
    virtual std::unique_ptr<Sound>        open_sound(pn::string_view path);
    virtual std::unique_ptr<Sound>        open_music(pn::string_view path);
    virtual void                          set_global_volume(uint8_t volume);

  private:
    class LogSound;
    class LogChannel;

    pn::file    _sound_log;
    int         _last_id;
    LogChannel* _active_channel;
};

}  // namespace antares

#endif  // ANTARES_SOUND_DRIVER_HPP_
