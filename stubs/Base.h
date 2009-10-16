#ifndef ANTARES_STUB_BASE_H_
#define ANTARES_STUB_BASE_H_

#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <Stub.h>
#include "Geometry.hpp"
#include "SmartPtr.hpp"

namespace antares {

#define pascal

enum {
    noErr = 0,
    paramErr = 1,

    plainDBox = 200,

    nullEvent = 0,
    everyEvent = 300,
    mouseDown = 303,
    mouseUp = 304,
    keyDown = 305,
    autoKey = 306,
    osEvt = 307,

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

void Microseconds(uint64_t* wide);

void BlockMove(void* src, void* dst, size_t size);

typedef void* AddrBlock;

#define nil NULL

typedef unsigned char Str255[256];
typedef unsigned char Str31[32];

typedef uint32_t FourCharCode;
typedef FourCharCode OSType;
typedef FourCharCode ResType;

////////////////////////////

struct Window;
typedef Window* WindowPtr;
typedef Window** WindowRef;

STUB1(GlobalToLocal, void(Point*));

typedef struct {
    int red;
    int green;
    int blue;
} RGBColor;

void Index2Color(long index, RGBColor* color);

typedef int KeyMap[4];
void GetKeys(KeyMap keys);

struct EventRecord {
    EventRecord()
            : what(nullEvent),
              message(0),
              where(0, 0),
              modifiers(0) { }

    int what;
    int message;
    Point where;
    int modifiers;
};

struct Rgn { };
typedef Rgn* RgnPtr;
typedef Rgn** RgnHandle;

STUB2(FlushEvents, void(int mask, int));
bool WaitNextEvent(long mask, EventRecord* evt, unsigned long sleep, Rgn** mouseRgn);

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
STUB0(GetDblTime, double(), 0.0);

STUB1(mAssert, void(bool condition));

STUB4(ParamText, void(const unsigned char*, const unsigned char*,
      const unsigned char*, const unsigned char*));

STUB2(StopAlert, void(int id, void*));

STUB1(DebugStr, void(const unsigned char* str));

typedef struct { } Dialog;
typedef Dialog* DialogPtr;

STUB3(GetNewDialog, Dialog*(int id, void*, Window* window), NULL);
STUB2(SetDialogDefaultItem, void(Dialog*, int item));
STUB5(GetDialogItem, void(Dialog*, int item, short* type, Handle* handle, const Rect& rect));
void ModalDialog(void*, short* item);
STUB1(DisposeDialog, void(Dialog* dialog));
STUB1(DrawDialog, void(Dialog* dialog));

typedef struct { } Control;
typedef Control* ControlPtr;
typedef Control** ControlHandle;

STUB2(HiliteControl, void(Control**, int));

STUB2(SetWRefCon, void(Dialog*, long));

long AngleFromSlope(Fixed slope);
long Random();

void StringToNum(unsigned char* string, long* value);

STUB1(GetScriptManagerVariable, Ptr(int cache), NULL);
STUB3(KeyTranslate, long(Ptr kchr, short keyCode, unsigned long* keyTranslateState), 0);

class ColorTable;
struct PixMap {
    PixMap(int32_t width, int32_t height);
    ~PixMap();

    void resize(const Rect& r);

    void set(int x, int y, uint8_t color);
    uint8_t get(int x, int y) const;

    Rect bounds;
    ColorTable* colors;
    long rowBytes;
    unsigned char* baseAddr;
    int pixelSize;

    DISALLOW_COPY_AND_ASSIGN(PixMap);
};
typedef PixMap* PixMapPtr;
typedef PixMap** PixMapHandle;

}  // namespace antares

#endif // ANTARES_STUB_BASE_H_
