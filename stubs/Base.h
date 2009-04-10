#ifndef ANTARES_STUB_BASE_H_
#define ANTARES_STUB_BASE_H_

#include <stdlib.h>
#include <stdint.h>
#include <Stub.h>

#define pascal

enum {
    noErr = 0,
    paramErr = 1,
    memFullErr = 2,

    maxSize = 100,

    plainDBox = 200,

    everyEvent = 300,
    updateEvt = 301,
    nullEvent = 302,
    mouseDown = 303,
    mouseUp = 304,
    keyDown = 305,
    autoKey = 306,
    osEvt = 307,
    kHighLevelEvent = 308,

    inMenuBar = 400,
    inSysWindow = 401,
    inContent = 402,
    inDrag = 403,
    inGoAway = 404,

    cmdKey = 500,
    optionKey = 501,
    shiftKey = 502,
    alphaLock = 503,
    smKCHRCache = 504,

    charCodeMask = 600,
    keyCodeMask = 601,
    resumeFlag = 602,
    convertClipboardFlag = 603,

    noGrowDocProc = 700,

    mouseMovedMessage = 800,
    suspendResumeMessage = 801,
};

typedef bool Boolean;
typedef int16_t OSErr;

typedef int8_t SignedByte;
typedef uint8_t Byte;
typedef int8_t SInt8;
typedef uint8_t UInt8;
typedef int16_t SInt16;
typedef uint16_t UInt16;
typedef int32_t SInt32;
typedef uint32_t UInt32;
typedef struct { SInt32 hi; UInt32 lo; } wide;
typedef struct { UInt32 hi; UInt32 lo; } UnsignedWide;

STUB2(WideAdd, void(wide* value, wide* summand));
STUB2(WideSubtract, void(wide* value, wide* difference));
STUB3(WideMultiply, void(long a, long b, wide* c));

typedef int32_t Fixed;
typedef long Size;

typedef char* Ptr;
STUB1(NewPtr, Ptr(size_t size), new char[a0]);
STUB1(DisposePtr, void(Ptr ptr));

typedef Ptr* Handle;
STUB1(NewHandle, Handle(size_t size), new char*(new char[a0]));
STUB1(DisposeHandle, void(Handle handle));
STUB1(GetHandleSize, int(Handle handle), 0);
STUB2(SetHandleSize, void(Handle handle, Size size));
STUB1(MoveHHi, void(Handle handle));
STUB1(HLock, void(Handle handle));
STUB1(HLockHi, void(Handle handle));
STUB1(HUnlock, void(Handle handle));
STUB1(HGetState, int8_t(Handle handle), 0);
STUB2(HSetState, void(Handle handle, int8_t state));
STUB1(HNoPurge, void(Handle handle));
STUB3(PtrToHand, OSErr(void* ptr, Handle* handle, int len), noErr);

STUB1(HiWord, int(long value), 0);
STUB1(LoWord, int(long value), 0);

STUB1(Microseconds, void(UnsignedWide* wide));

STUB1(MaxMem, Size(Size*), 0);
STUB1(CompactMem, Size(int), 0);
STUB3(BlockMove, void(void*, void*, size_t));
STUB1(HandToHand, OSErr(Handle* handle), noErr);
STUB2(HandAndHand, void(Handle src, Handle dst));

typedef void* AddrBlock;

#define nil NULL

typedef unsigned char Str255[256];
typedef unsigned char Str63[64];
typedef unsigned char Str31[32];
typedef unsigned char* StringPtr;
typedef const unsigned char* ConstStringPtr;
typedef const unsigned char* ConstStr255Param;
typedef const unsigned char* ConstStr31Param;
typedef Str63 StrFileName;

typedef uint32_t FourCharCode;
typedef FourCharCode OSType;
typedef FourCharCode ResType;

STUB0(MemError, OSErr(), noErr);
STUB0(ResError, OSErr(), noErr);

enum {
    TRUE = true,
    FALSE = false,
};

typedef struct {
    int v;
    int h;
} Point;

typedef struct {
    int top;
    int left;
    int bottom;
    int right;
} Rect;

STUB5(SetRect, void(Rect*, int, int, int, int));
STUB5(MacSetRect, void(Rect*, int, int, int, int));
STUB3(OffsetRect, void(Rect*, int, int));
STUB3(MacOffsetRect, void(Rect*, int, int));
STUB2(MacPtInRect, bool(Point, Rect*), false);
STUB3(MacInsetRect, void(Rect*, int, int));

////////////////////////////

typedef struct { } BitMap;

typedef struct {
    Rect portRect;
    BitMap portBits;
} Window;
typedef Window* WindowPtr;
typedef Window** WindowRef;

STUB1(BeginUpdate, void(Window* window));
STUB1(EndUpdate, void(Window* window));
STUB8(NewWindow, Window*(void*, Rect*, const unsigned char* title, bool, int,
      Window* behind, bool, int), new Window)
STUB1(MacShowWindow, void(Window*));
STUB1(DisposeWindow, void(Window*));
STUB2(MacFindWindow, short(Point where, Window** window), 0);
STUB4(MacMoveWindow, void(Window*, int x, int y, bool));
STUB2(ShowHide, void(Window*, bool hide));
STUB3(DragWindow, void(Window*, Point where, Rect* bounds));
STUB2(TrackGoAway, bool(Window*, Point where), false);
STUB1(GlobalToLocal, void(Point*));
STUB1(SelectWindow, void(Window*));

typedef struct {
    Rect portRect;
} CWindow;
typedef CWindow* CWindowPtr;

STUB8(NewCWindow,
    CWindow*(void*, Rect* size, const unsigned char* title, bool, int,
      Window* behind, bool, int id),
    new CWindow);

typedef struct {
    int red;
    int green;
    int blue;
} RGBColor;

typedef struct {
    int value;
    RGBColor rgb;
} ColorSpec;
typedef ColorSpec* ColorSpecPtr;

typedef struct {
    ColorSpecPtr ctTable;
    int ctSize;
} CTab;
typedef CTab* CTabPtr;
typedef CTab** CTabHandle;

STUB1(GetCTable, CTab**(int id), new CTab*(new CTab));
STUB1(DisposeCTable, void(CTab** handle));
STUB1(CTabChanged, void(CTab** handle));
STUB2(Index2Color, void(long index, RGBColor* color));

typedef void** WCTabHandle;

typedef struct {
    WCTabHandle awCTable;
} AuxWin;
typedef AuxWin** AuxWinHandle;

typedef int KeyMap[4];

STUB1(GetKeys, void(KeyMap keys));

typedef struct { } ICInstance;

typedef struct {
    int what;
    int message;
    Point where;
    int modifiers;
} EventRecord;

struct Rgn { };
typedef Rgn* RgnPtr;
typedef Rgn** RgnHandle;

STUB2(FlushEvents, void(int mask, int));
STUB1(SetEventMask, void(int mask));
STUB4(WaitNextEvent, bool(long mask, EventRecord* evt, unsigned long sleep, Rgn** mouseRgn), false);

typedef Handle MenuHandle;

STUB1(GetNewMBar, MenuHandle(int id), NULL);
STUB1(SetMenuBar, void(MenuHandle menu));
STUB1(GetMenuHandle, MenuHandle(int id), NULL);
STUB2(AppendResMenu, void(MenuHandle handle, FourCharCode));
STUB0(MacDrawMenuBar, void());
STUB1(HiliteMenu, void(int));
STUB1(MenuSelect, long(Point where), 0);
STUB2(SystemClick, void(EventRecord* event, Window* window));
STUB3(GetMenuItemText, void(MenuHandle, int item, unsigned char* name));
STUB1(HandleMenuChoice, bool(int), false);
STUB1(MenuKey, int(char which_char), 0);
STUB1(OpenDeskAcc, int(const unsigned char* name), 0);

STUB2(NoteAlert, void(int type, void*));

// Gets STR# from resource fork
STUB3(GetIndString, void(const unsigned char* result, int id, int index));

STUB2(GetIndResource, Handle(FourCharCode type, int id), NULL);

STUB0(Debugger, void());

STUB6(Munger, int(Handle, int, const unsigned char* dest, size_t dest_length,
      const void* source, size_t source_length), 0);

STUB1(SysBeep, void(int));

STUB0(ExitToShell, void());

STUB1(GetDateTime, void(unsigned long* time));

STUB0(InitCursor, void());
STUB0(MacShowCursor, void());
STUB0(HideCursor, void());
STUB2(ShieldCursor, void(Rect* rect, Point point));
STUB1(GetMouse, void(Point* point));

STUB0(InitWindows, void());
STUB0(InitMenus, void());
STUB1(InitDialogs, void(void*));
STUB0(MoreMasters, void());
STUB0(MaxApplZone, void());

STUB0(TickCount, int(), 1);

STUB4(GetResInfo, void(Handle resource, short* id, FourCharCode* type, unsigned char* name));
STUB1(RemoveResource, void(Handle resource));
STUB1(UpdateResFile, void(int file));
STUB4(AddResource, void(Handle resource, FourCharCode type, int id, const unsigned char* name));
STUB1(ChangedResource, void(Handle resource));
STUB1(WriteResource, void(Handle resource));

STUB0(Button, bool(), false);
STUB0(GetDblTime, double(), 0.0);

STUB2(GetAuxWin, void(Window* window, AuxWinHandle* handle));
STUB2(SetWinColor, void(Window* window, WCTabHandle handle));

typedef struct { } NumVersion;

STUB4(EasyOpenPreferenceFile,
    bool(const unsigned char* file_name, int creator, int type, short* ref_num), true);

STUB1(mAssert, void(bool condition));

inline void ParamText(const unsigned char*, const unsigned char*,
      const unsigned char*, const unsigned char*) {
  gdb();
}

STUB2(StopAlert, void(int id, void*));

STUB1(DebugStr, void(const unsigned char* str));

typedef struct { } Dialog;
typedef Dialog* DialogPtr;

STUB3(GetNewDialog, Dialog*(int id, void*, Window* window), NULL);
STUB3(SetDialogFontAndSize, void(Dialog* dialog, int font, int size));
STUB2(SetDialogDefaultItem, void(Dialog*, int item));
STUB5(GetDialogItem, void(Dialog*, int item, short* type, Handle* handle, Rect* rect));
STUB2(ModalDialog, void(void*, short* item));
STUB1(DisposeDialog, void(Dialog* dialog));
STUB1(DrawDialog, void(Dialog* dialog));

typedef struct { } Control;
typedef Control* ControlPtr;
typedef Control** ControlHandle;

STUB2(HiliteControl, void(Control**, int));

STUB2(SetWRefCon, void(Dialog*, long));

STUB1(AngleFromSlope, long(Fixed slope), 0);
STUB0(Random, long(), 0);

STUB2(StringToNum, void(unsigned char* string, long* value));

STUB1(GetScriptManagerVariable, Ptr(int cache), NULL);
STUB3(KeyTranslate, long(Ptr kchr, short keyCode, unsigned long* keyTranslateState), 0);

#endif // ANTARES_STUB_BASE_H_
