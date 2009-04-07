#ifndef ANTARES_STUB_BASE_H_
#define ANTARES_STUB_BASE_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdlib.h>
#include <stdint.h>

#define pascal

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

void WideSubtract(wide* value, wide* difference);
void WideMultiply(long a, long b, wide* c);

typedef int32_t Fixed;
typedef long Size;

typedef char* Ptr;
Ptr NewPtr(size_t size);
void DisposePtr(Ptr ptr);

typedef Ptr* Handle;
Handle NewHandle(size_t size);
void DisposeHandle(Handle handle);
int GetHandleSize(Handle handle);
void SetHandleSize(Handle handle, Size size);
void MoveHHi(Handle handle);
void HLock(Handle handle);
void HLockHi(Handle handle);
void HUnlock(Handle handle);
int8_t HGetState(Handle handle);
void HSetState(Handle handle, int8_t state);

int HiWord(long value);
int LoWord(long value);

void Microseconds(UnsignedWide* wide);

Size MaxMem(Size*);
Size CompactMem(int);
void BlockMove(Ptr, void*, size_t);
OSErr HandToHand(Handle* handle);
void HandAndHand(Handle src, Handle dst);

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

    inMenuBar = 400,
    inSysWindow = 401,
    inContent = 402,
    inDrag = 403,
    inGoAway = 404,

    cmdKey = 500,

    charCodeMask = 600,

    noGrowDocProc = 700,
};

OSErr MemError();
OSErr ResError();

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

void MacSetRect(Rect* rect, int top, int right, int bottom, int left);
void OffsetRect(Rect* rect, int h, int v);
void MacOffsetRect(Rect* rect, int h, int v);

////////////////////////////

typedef struct {
    Rect portRect;
} Window;
typedef Window* WindowPtr;
typedef Window** WindowRef;

void BeginUpdate(WindowPtr window);
void EndUpdate(WindowPtr window);
WindowPtr NewWindow(void*, Rect* rect, const unsigned char* title, bool, int,
                    WindowPtr behind, bool, int);
void MacShowWindow(WindowPtr window);
void DisposeWindow(WindowPtr window);
short MacFindWindow(Point where, WindowPtr* window);
void MacMoveWindow(WindowPtr window, int x, int y, bool);
void ShowHide(WindowPtr window, bool hide);
void DragWindow(WindowPtr window, Point where, Rect* bounds);
bool TrackGoAway(WindowPtr window, Point where);
void GlobalToLocal(Point* point);

typedef struct {
    Rect portRect;
} CWindow;
typedef CWindow* CWindowPtr;

CWindowPtr NewCWindow(void*, Rect* size, const unsigned char*, bool, int,
                      WindowPtr, bool, int id);

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

CTabHandle GetCTable(int id);
void DisposeCTable(CTabHandle handle);
void CTabChanged(CTabHandle handle);
void Index2Color(long index, RGBColor* color);

typedef void** WCTabHandle;

typedef struct {
    WCTabHandle awCTable;
} AuxWin;
typedef AuxWin** AuxWinHandle;

typedef int KeyMap[4];

void GetKeys(KeyMap keys);

typedef struct { } ICInstance;

typedef struct {
    int what;
    int message;
    Point where;
    int modifiers;
} EventRecord;

void FlushEvents(int mask, int);
void SetEventMask(int mask);

typedef Handle MenuHandle;

MenuHandle GetNewMBar(int id);
void SetMenuBar(MenuHandle menu);
MenuHandle GetMenuHandle(int id);
void AppendResMenu(MenuHandle handle, FourCharCode);
void MacDrawMenuBar();
void HiliteMenu(int);
long MenuSelect(Point where);
void SystemClick(EventRecord* event, WindowPtr window);
void GetMenuItemText(MenuHandle, int item, unsigned char* name);
bool HandleMenuChoice(int key);
int MenuKey(char which_char);
int OpenDeskAcc(const unsigned char* name);

void NoteAlert(int type, void*);

typedef void** TEHandle;

// Gets STR# from resource fork
void GetIndString( const unsigned char* result, int id, int index);

Handle GetIndResource(FourCharCode type, int id);

void Debugger();

void Munger(Handle, int, const unsigned char* dest, size_t dest_length,
            const void* source, size_t source_length);

void SysBeep(int);

void ExitToShell();

void GetDateTime(unsigned long* time);

void InitCursor();
void MacShowCursor();
void HideCursor();
void ShieldCursor(Rect* rect, Point point);

void InitWindows();
void InitMenus();
void TEInit();
void InitDialogs(void*);
void MoreMasters();
void MaxApplZone();

int TickCount();

void GetResInfo(Handle resource, short* id, FourCharCode* type,
                unsigned char* name);
void RemoveResource(Handle resource);
void UpdateResFile(int file);
void AddResource(Handle resource, FourCharCode type, int id,
                 const unsigned char* name);
void ChangedResource(Handle resource);
void WriteResource(Handle resource);

bool Button();
double GetDblTime();

void GetAuxWin(WindowPtr window, AuxWinHandle* handle);
void SetWinColor(WindowPtr window, WCTabHandle handle);

typedef struct { } NumVersion;

bool EasyOpenPreferenceFile(const unsigned char* file_name, int creator,
                            int type, short* ref_num);

void mAssert(bool condition);

void ParamText(const unsigned char* str1, const unsigned char* str2,
               const unsigned char* str3, const unsigned char* str4);
void StopAlert(int id, void*);

void DebugStr(const unsigned char* str);

typedef struct { } Dialog;
typedef Dialog* DialogPtr;

DialogPtr GetNewDialog(int id, void*, WindowPtr window);
void SetDialogFontAndSize(DialogPtr dialog, int font, int size);
void SetDialogDefaultItem(DialogPtr dialog, int item);
void GetDialogItem(DialogPtr dialog, int item, short* type, Handle* handle,
                   Rect* rect);
void ModalDialog(void*, short* item);
void DisposeDialog(DialogPtr dialog);
void DrawDialog(DialogPtr dialog);

typedef struct { } Control;
typedef Control* ControlPtr;
typedef Control** ControlHandle;
void HiliteControl(ControlHandle control, int);

void SetWRefCon(DialogPtr, long);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ANTARES_STUB_BASE_H_
