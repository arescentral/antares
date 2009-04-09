#ifndef ANTARES_STUB_SOUND_H_
#define ANTARES_STUB_SOUND_H_

#include <Base.h>

typedef OSErr OSStatus;

typedef Handle SndListHandle;

struct SCStatus {
    bool scChannelBusy;
};
typedef SCStatus* SCStatusPtr;

struct SndChannel { };
typedef SndChannel* SndChannelPtr;

enum {
    sampledSynth,
    initMono,
};

STUB4(SndNewChannel, OSErr(SndChannel** chan, long, long, void*), noErr);
STUB2(SndDisposeChannel, OSErr(SndChannel* chan, bool), noErr);
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

STUB2(SndDoImmediate, OSErr(SndChannel* chan, SndCommand* cmd), noErr);
STUB3(SndDoCommand, OSErr(SndChannel* chan, SndCommand* cmd, bool), noErr);
STUB3(SndPlay, OSErr(SndChannel* channel, Handle sound, bool), noErr);

STUB2(GetSoundHeaderOffset, OSErr(Handle sound, long* offset), noErr);

#endif // ANTARES_STUB_SOUND_H_
