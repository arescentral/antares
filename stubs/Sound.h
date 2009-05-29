#ifndef ANTARES_STUB_SOUND_H_
#define ANTARES_STUB_SOUND_H_

#include <Base.h>

typedef OSErr OSStatus;

typedef Handle SndListHandle;

struct SCStatus {
    bool scChannelBusy;
};
typedef SCStatus* SCStatusPtr;

struct SndChannel;
typedef SndChannel* SndChannelPtr;

enum {
    sampledSynth,
    initMono,
};

OSErr SndNewChannel(SndChannel** chan, long, long, void*);
OSErr SndDisposeChannel(SndChannel* chan, bool);
STUB3(SndChannelStatus, OSErr(SndChannel* chan, int status_size, SCStatus* status), noErr);

enum SndCommandEnum {
    quietCmd,
    flushCmd,
    ampCmd,
};

struct SndCommand {
    SndCommandEnum cmd;
    int param1;
    int param2;
};

OSErr SndDoImmediate(SndChannel* chan, SndCommand* cmd);
OSErr SndDoCommand(SndChannel* chan, SndCommand* cmd, bool);
OSErr SndPlay(SndChannel* channel, Handle sound, bool);

STUB2(GetSoundHeaderOffset, OSErr(Handle sound, long* offset), noErr);

#endif // ANTARES_STUB_SOUND_H_
