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

#include <Sound.h>

#include <assert.h>
#include <stdio.h>

#include "Fakes.hpp"

namespace {

scoped_ptr<SoundDriver> sound_driver;

}  // namespace

void SoundDriver::set_driver(SoundDriver* driver) {
    assert(driver);
    sound_driver.reset(driver);
}

void NullSoundDriver::play(int32_t, int32_t) { }
void NullSoundDriver::amp(int32_t, uint8_t) { }
void NullSoundDriver::quiet(int32_t) { }

LogSoundDriver::LogSoundDriver(const std::string& path)
        : _sound_log(fopen(path.c_str(), "w")) {
    assert(_sound_log);
    setbuf(_sound_log, NULL);
}

void LogSoundDriver::play(int32_t channel, int32_t id) {
    if (globals()->gGameTime > 0) {
        fprintf(_sound_log, "play\t%d\t%ld\t%d\n", channel, globals()->gGameTime, id);
    }
}

void LogSoundDriver::amp(int32_t channel, uint8_t volume) {
    if (globals()->gGameTime > 0) {
        fprintf(_sound_log, "amp\t%d\t%ld\t%d\n", channel, globals()->gGameTime, volume);
    }
}

void LogSoundDriver::quiet(int32_t channel) {
    if (globals()->gGameTime > 0) {
        fprintf(_sound_log, "quiet\t%d\t%ld\n", channel, globals()->gGameTime);
    }
}

static int last_id = -1;
struct SndChannel {
    SndChannel()
        : id(++last_id) { }
    int id;
};

OSErr SndNewChannel(SndChannel** chan, long, long, void*) {
    globals()->gSoundVolume = 8;
    *chan = new SndChannel;
    return noErr;
}

OSErr SndDisposeChannel(SndChannel* chan, bool) {
    delete chan;
    return noErr;
}

OSErr SndDoImmediate(SndChannel* chan, SndCommand* cmd) {
    switch (cmd->cmd) {
      case ampCmd:
        sound_driver->amp(chan->id, cmd->param1);
        break;
      case quietCmd:
        sound_driver->quiet(chan->id);
        break;
      default:
        break;
    }
    return noErr;
}

OSErr SndDoCommand(SndChannel* chan, SndCommand* cmd, bool) {
    return SndDoImmediate(chan, cmd);
}

OSErr SndPlay(SndChannel* channel, TypedHandle<Sound> sound, bool) {
    sound_driver->play(channel->id, (*sound)->id);
    return noErr;
}

TypedHandle<Sound> GetSound(int id) {
    TypedHandle<Sound> result;
    result.create(1);
    (*result)->id = id;
    return result;
}

void FakeSoundsInit() {
    if (!sound_driver.get()) {
        sound_driver.reset(new NullSoundDriver);
    }
}
