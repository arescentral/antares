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

bool do_sounds = false;
void SetDoSounds(bool flag) {
    do_sounds = flag;
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

static FILE* sound_log;

OSErr SndDoImmediate(SndChannel* chan, SndCommand* cmd) {
    if (do_sounds) {
        switch (cmd->cmd) {
        case quietCmd:
            if (globals()->gGameTime > 0) {
                fprintf(sound_log, "quiet\t%d\t%ld\n", chan->id, globals()->gGameTime);
            }
            break;

        case flushCmd:
            break;

        case ampCmd:
            if (globals()->gGameTime > 0) {
                fprintf(sound_log, "amp\t%d\t%ld\t%d\n",
                        chan->id, globals()->gGameTime, cmd->param1);
            }
            break;

        default:
            assert(false);
        }
    }
    return noErr;
}

OSErr SndDoCommand(SndChannel* chan, SndCommand* cmd, bool) {
    return SndDoImmediate(chan, cmd);
}

OSErr SndPlay(SndChannel* channel, TypedHandle<Sound> sound, bool) {
    if (do_sounds && globals()->gGameTime > 0) {
        int sound_id = (*sound)->id;
        fprintf(sound_log, "play\t%d\t%ld\t%d\n", channel->id, globals()->gGameTime, sound_id);
    }
    return noErr;
}

TypedHandle<Sound> GetSound(int id) {
    TypedHandle<Sound> result;
    result.create(1);
    (*result)->id = id;
    return result;
}

void FakeSoundsInit() {
    if (do_sounds) {
        std::string filename = GetOutputDir() + "/sound.log";
        sound_log = fopen(filename.c_str(), "w");
        setbuf(sound_log, NULL);
        assert(sound_log);
    }
}
