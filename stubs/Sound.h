#ifndef ANTARES_STUB_SOUND_H_
#define ANTARES_STUB_SOUND_H_

#include <Base.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

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

OSErr SndNewChannel(SndChannel** chan, long, long, void*);
OSErr SndDisposeChannel(SndChannel* chan, bool);
OSErr SndChannelStatus(SndChannel* chan, int status_size, SCStatus* status);

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

OSErr GetSoundHeaderOffset(Handle sound, long* offset);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ANTARES_STUB_SOUND_H_
