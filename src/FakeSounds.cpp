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

#include "FakeHandles.hpp"
#include "Fakes.hpp"

struct SndChannel {
    int volume;
};

OSErr SndNewChannel(SndChannel** chan, long, long, void*) {
    gAresGlobal->gSoundVolume = 8;
    *chan = new SndChannel;
    return noErr;
}

OSErr SndDisposeChannel(SndChannel* chan, bool) {
    delete chan;
    return noErr;
}

static FILE* sound_log;

OSErr SndDoImmediate(SndChannel* chan, SndCommand* cmd) {
    switch (cmd->cmd) {
      case quietCmd:
        break;

      case flushCmd:
        break;

      case ampCmd:
        chan->volume = cmd->param1;
        break;

      default:
        assert(false);
    }
    return noErr;
}

OSErr SndDoCommand(SndChannel* chan, SndCommand* cmd, bool) {
    return SndDoImmediate(chan, cmd);
}

OSErr SndPlay(SndChannel* channel, Handle sound, bool) {
    if (gAresGlobal->gGameTime > 0) {
        int sound_id = **reinterpret_cast<int**>(sound);
        fprintf(sound_log, "%ld\t%d\t%d\n", gAresGlobal->gGameTime, sound_id, channel->volume);
    }
    return noErr;
}

Handle GetSound(int id) {
    return (new HandleData<int>(id))->ToHandle();
}

void FakeSoundsInit() {
    sound_log = fopen("sound.log", "w");
    setbuf(sound_log, NULL);
    assert(sound_log);
}
