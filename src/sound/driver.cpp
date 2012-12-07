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

#include "sound/driver.hpp"

#include <fcntl.h>
#include <sfz/sfz.hpp>

#include "lang/casts.hpp"
#include "video/driver.hpp"

using sfz::Bytes;
using sfz::Exception;
using sfz::PrintItem;
using sfz::ScopedFd;
using sfz::String;
using sfz::StringSlice;
using sfz::format;
using sfz::write;
using sfz::scoped_ptr;

namespace utf8 = sfz::utf8;

namespace antares {

namespace {

///////////////////////////////////////////////////////////////////////////////////////////////////
// SoundDriver

SoundDriver* sound_driver = NULL;

}  // namespace

SoundDriver::SoundDriver() {
    if (antares::sound_driver) {
        throw Exception("SoundDriver is a singleton");
    }
    antares::sound_driver = this;
}

SoundDriver::~SoundDriver() {
    antares::sound_driver = NULL;
}

SoundDriver* SoundDriver::driver() {
    return sound_driver;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// NullSoundDriver

namespace {

class NullChannel : public SoundChannel {
  public:
    NullChannel() { }

    virtual void activate() { }

    virtual void amp(uint8_t volume) {
        static_cast<void>(volume);
    }

    virtual void quiet() { }

  private:
    DISALLOW_COPY_AND_ASSIGN(NullChannel);
};

class NullSound : public Sound {
  public:
    NullSound() { }

    virtual void play() { }
    virtual void loop() { }

  private:
    DISALLOW_COPY_AND_ASSIGN(NullSound);
};

}  // namespace

void NullSoundDriver::open_channel(scoped_ptr<SoundChannel>& channel) {
    channel.reset(new NullChannel);
}

void NullSoundDriver::open_sound(PrintItem path, scoped_ptr<Sound>& sound) {
    static_cast<void>(path);
    sound.reset(new NullSound);
}

void NullSoundDriver::set_global_volume(uint8_t volume) {
    static_cast<void>(volume);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// LogSoundDriver

class LogSoundDriver::LogChannel : public SoundChannel {
  public:
    LogChannel(LogSoundDriver& driver):
            _id(++driver._last_id),
            _driver(driver) { }

    virtual void activate() {
        _driver._active_channel = this;
    }

    void play(StringSlice sound_path) {
        String line(
                format("play\t{0}\t{1}\t{2}\n", _id, VideoDriver::driver()->ticks(), sound_path));
        write(_driver._sound_log, utf8::encode(line));
    }

    void loop(StringSlice sound_path) {
        String line(
                format("loop\t{0}\t{1}\t{2}\n", _id, VideoDriver::driver()->ticks(), sound_path));
        write(_driver._sound_log, utf8::encode(line));
    }

    virtual void amp(uint8_t volume) {
        String line(format("amp\t{0}\t{1}\t{2}\n", _id, VideoDriver::driver()->ticks(), volume));
        write(_driver._sound_log, utf8::encode(line));
    }

    virtual void quiet() {
        String line(format("quiet\t{0}\t{1}\n", _id, VideoDriver::driver()->ticks()));
        write(_driver._sound_log, utf8::encode(line));
    }

  private:
    int _id;
    LogSoundDriver& _driver;

    DISALLOW_COPY_AND_ASSIGN(LogChannel);
};

class LogSoundDriver::LogSound : public Sound {
  public:
    LogSound(const LogSoundDriver& driver, StringSlice path):
            _driver(driver),
            _path(path) { }

    virtual void play() {
        _driver._active_channel->play(_path);
    }

    virtual void loop() {
        _driver._active_channel->loop(_path);
    }

  private:
    const LogSoundDriver& _driver;
    const String _path;

    DISALLOW_COPY_AND_ASSIGN(LogSound);
};

LogSoundDriver::LogSoundDriver(const StringSlice& path):
        _sound_log(open(path, O_CREAT | O_WRONLY | O_TRUNC, 0644)),
        _last_id(-1),
        _active_channel(NULL) { }

void LogSoundDriver::open_channel(scoped_ptr<SoundChannel>& channel) {
    channel.reset(new LogChannel(*this));
}

void LogSoundDriver::open_sound(PrintItem path, scoped_ptr<Sound>& sound) {
    String path_string(path);
    sound.reset(new LogSound(*this, path_string));
}

void LogSoundDriver::set_global_volume(uint8_t volume) {
    static_cast<void>(volume);
}

}  // namespace antares
