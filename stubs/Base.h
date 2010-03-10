#ifndef ANTARES_STUB_BASE_H_
#define ANTARES_STUB_BASE_H_

#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include "sfz/SmartPtr.hpp"
#include "Stub.h"
#include "Geometry.hpp"

namespace antares {

#define pascal

enum {
    noErr = 0,
    paramErr = 1,

    nullEvent = 0,
    everyEvent = 300,
    mouseDown = 303,
    mouseUp = 304,
    keyDown = 305,
    autoKey = 306,
    osEvt = 307,
    keyUp = 308,

    cmdKey = 500,
    optionKey = 501,
    shiftKey = 502,
    alphaLock = 503,
    smKCHRCache = 504,

    charCodeMask = 0x00FF,
    keyCodeMask = 0xFF00,
};

typedef int16_t OSErr;

typedef int32_t Fixed;
typedef long Size;

typedef unsigned char* Ptr;
typedef Ptr* Handle;

#ifndef nil
#define nil NULL
#endif

typedef unsigned char Str255[256];
typedef unsigned char Str31[32];

typedef uint32_t FourCharCode;
typedef FourCharCode OSType;
typedef FourCharCode ResType;

////////////////////////////

class RgbColor;
void Index2Color(long index, RgbColor* color);

typedef uint32_t KeyMap[4];
void GetKeys(KeyMap keys);

struct Rgn { };
typedef Rgn* RgnPtr;
typedef Rgn** RgnHandle;

STUB2(FlushEvents, void(int mask, int));

// Gets STR# from resource fork
void GetIndString(unsigned char* result, int id, int index);

STUB1(SysBeep, void(int));

STUB0(ExitToShell, void());

STUB1(GetDateTime, void(unsigned long* time));

STUB0(InitCursor, void());
STUB0(MacShowCursor, void());
STUB0(HideCursor, void());
void GetMouse(Point* point);

int TickCount();

bool Button();

STUB1(DebugStr, void(const unsigned char* str));

long AngleFromSlope(Fixed slope);
long Random();

void StringToNum(unsigned char* string, long* value);

STUB1(GetScriptManagerVariable, Ptr(int cache), NULL);
STUB3(KeyTranslate, long(Ptr kchr, short keyCode, unsigned long* keyTranslateState), 0);

}  // namespace antares

#endif // ANTARES_STUB_BASE_H_
