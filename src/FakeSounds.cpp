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

#include "FakeSounds.hpp"

#include <fcntl.h>
#include "sfz/sfz.hpp"
#include "Error.hpp"
#include "Fakes.hpp"
#include "Preferences.hpp"

using sfz::Bytes;
using sfz::Exception;
using sfz::ScopedFd;
using sfz::String;
using sfz::StringPiece;
using sfz::format;
using sfz::write;
using sfz::scoped_ptr;

namespace utf8 = sfz::utf8;

namespace antares {

namespace {

scoped_ptr<SoundDriver> sound_driver;

class NullSndChannel : public SndChannel {
  public:
    virtual void play(Sound* sound) {
        static_cast<void>(sound);
    }

    virtual void amp(uint8_t volume) {
        static_cast<void>(volume);
    }

    virtual void quiet() { }
};

class LogSndChannel : public SndChannel {
  public:
    LogSndChannel(int id, ScopedFd* fd)
        : _id(id),
          _fd(fd) { }

    virtual void play(Sound* sound) {
        if (globals()->gGameTime > 0) {
            String line(format("play\t{0}\t{1}\t{2}\n", _id, globals()->gGameTime, sound->id));
            write(_fd, utf8::encode(line));
        }
    }

    virtual void amp(uint8_t volume) {
        if (globals()->gGameTime > 0) {
            String line(format("amp\t{0}\t{1}\t{2}\n", _id, globals()->gGameTime, volume));
            write(_fd, utf8::encode(line));
        }
    }

    virtual void quiet() {
        if (globals()->gGameTime > 0) {
            String line(format("quiet\t{0}\t{1}\n", _id, globals()->gGameTime));
            write(_fd, utf8::encode(line));
        }
    }

  private:
    int _id;
    ScopedFd* const _fd;
};

}  // namespace

SoundDriver* SoundDriver::driver() {
    return sound_driver.get();
}

void SoundDriver::set_driver(SoundDriver* driver) {
    if (!driver) {
        throw Exception("tried to set NULL SoundDriver");
    }
    sound_driver.reset(driver);
}

SndChannel* NullSoundDriver::new_channel() {
    return new NullSndChannel();
}

LogSoundDriver::LogSoundDriver(const StringPiece& path)
        : _sound_log(open(path, O_CREAT | O_WRONLY, 0644)),
          _last_id(-1) {
    if (_sound_log.get() < 0) {
        throw Exception("Couldn't open sound log");
    }
}

SndChannel* LogSoundDriver::new_channel() {
    return new LogSndChannel(++_last_id, &_sound_log);
}

}  // namespace antares
