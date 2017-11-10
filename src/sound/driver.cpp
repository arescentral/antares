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

#include "sound/driver.hpp"

#include <fcntl.h>
#include <pn/file>
#include <sfz/sfz.hpp>

#include "data/pn.hpp"
#include "game/sys.hpp"
#include "game/time.hpp"
#include "lang/casts.hpp"
#include "lang/defines.hpp"
#include "video/driver.hpp"

using sfz::Bytes;
using sfz::ScopedFd;
using sfz::write;
using std::unique_ptr;

namespace utf8 = sfz::utf8;

namespace antares {

///////////////////////////////////////////////////////////////////////////////////////////////////
// SoundDriver

SoundDriver::SoundDriver() {
    if (sys.audio) {
        throw std::runtime_error("SoundDriver is a singleton");
    }
    sys.audio = this;
}

SoundDriver::~SoundDriver() { sys.audio = NULL; }

///////////////////////////////////////////////////////////////////////////////////////////////////
// NullSoundDriver

namespace {

class NullChannel : public SoundChannel {
  public:
    NullChannel() {}

    virtual void activate() {}

    virtual void amp(uint8_t volume) { static_cast<void>(volume); }

    virtual void quiet() {}

  private:
    DISALLOW_COPY_AND_ASSIGN(NullChannel);
};

class NullSound : public Sound {
  public:
    NullSound() {}

    virtual void play() {}
    virtual void loop() {}

  private:
    DISALLOW_COPY_AND_ASSIGN(NullSound);
};

}  // namespace

unique_ptr<SoundChannel> NullSoundDriver::open_channel() {
    return unique_ptr<SoundChannel>(new NullChannel);
}

unique_ptr<Sound> NullSoundDriver::open_sound(pn::string_view path) {
    static_cast<void>(path);
    return unique_ptr<Sound>(new NullSound);
}

void NullSoundDriver::set_global_volume(uint8_t volume) { static_cast<void>(volume); }

///////////////////////////////////////////////////////////////////////////////////////////////////
// LogSoundDriver

class LogSoundDriver::LogChannel : public SoundChannel {
  public:
    LogChannel(LogSoundDriver& driver) : _id(++driver._last_id), _driver(driver) {}

    virtual void activate() { _driver._active_channel = this; }

    void play(pn::string_view sound_path) {
        auto       t    = std::chrono::time_point_cast<ticks>(now()).time_since_epoch().count();
        pn::string line = pn::format("play\t{0}\t{1}\t{2}\n", _id, t, sound_path);
        write(_driver._sound_log, line.data(), line.size());
    }

    void loop(pn::string_view sound_path) {
        auto       t    = std::chrono::time_point_cast<ticks>(now()).time_since_epoch().count();
        pn::string line = pn::format("loop\t{0}\t{1}\t{2}\n", _id, t, sound_path);
        write(_driver._sound_log, line.data(), line.size());
    }

    virtual void amp(uint8_t volume) {
        auto       t    = std::chrono::time_point_cast<ticks>(now()).time_since_epoch().count();
        pn::string line = pn::format("amp\t{0}\t{1}\t{2}\n", _id, t, volume);
        write(_driver._sound_log, line.data(), line.size());
    }

    virtual void quiet() {
        auto       t    = std::chrono::time_point_cast<ticks>(now()).time_since_epoch().count();
        pn::string line = pn::format("quiet\t{0}\t{1}\n", _id, t);
        write(_driver._sound_log, line.data(), line.size());
    }

  private:
    int             _id;
    LogSoundDriver& _driver;

    DISALLOW_COPY_AND_ASSIGN(LogChannel);
};

class LogSoundDriver::LogSound : public Sound {
  public:
    LogSound(const LogSoundDriver& driver, pn::string_view path)
            : _driver(driver), _path(path.copy()) {}

    virtual void play() { _driver._active_channel->play(_path); }

    virtual void loop() { _driver._active_channel->loop(_path); }

  private:
    const LogSoundDriver& _driver;
    const pn::string      _path;

    DISALLOW_COPY_AND_ASSIGN(LogSound);
};

LogSoundDriver::LogSoundDriver(pn::string_view path)
        : _sound_log(open(pn2sfz(path), O_CREAT | O_WRONLY | O_TRUNC, 0644)),
          _last_id(-1),
          _active_channel(NULL) {}

unique_ptr<SoundChannel> LogSoundDriver::open_channel() {
    return unique_ptr<SoundChannel>(new LogChannel(*this));
}

unique_ptr<Sound> LogSoundDriver::open_sound(pn::string_view path) {
    return unique_ptr<Sound>(new LogSound(*this, path));
}

void LogSoundDriver::set_global_volume(uint8_t volume) { static_cast<void>(volume); }

}  // namespace antares
