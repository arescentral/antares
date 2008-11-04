#ifndef ANTARES_STUB_BASE_H_
#define ANTARES_STUB_BASE_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdlib.h>
#include <stdint.h>

#define pascal

typedef bool Boolean;

typedef uint16_t Byte;
typedef int16_t SInt16;
typedef uint16_t UInt16;
typedef int32_t SInt32;
typedef uint32_t UInt32;
typedef struct { SInt32 hi; UInt32 lo; } wide;
typedef struct { UInt32 hi; UInt32 lo; } UnsignedWide;

typedef int32_t Fixed;

typedef char* Ptr;
Ptr NewPtr(size_t size);
void DisposePtr(Ptr ptr);

typedef Ptr* Handle;
typedef long Size;
Handle NewHandle(size_t size);
void DisposeHandle(Handle handle);
int GetHandleSize(Handle handle);
void MoveHHi(Handle handle);
void HLock(Handle handle);
void HLockHi(Handle handle);
void HUnlock(Handle handle);

void BlockMove(Ptr, void*, size_t);

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

typedef int16_t OSErr;
typedef uint32_t FourCharCode;
typedef FourCharCode OSType;
typedef FourCharCode ResType;

enum {
    noErr = 0,
    paramErr = 1,
};

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
    int right;
    int bottom;
    int left;
} Rect;

////////////////////////////

typedef struct { } Window;
typedef Window* WindowPtr;

typedef struct { } CWindow;
typedef CWindow* CWindowPtr;

typedef struct { } CTab;
typedef CTab* CTabPtr;
typedef CTab** CTabHandle;

typedef int KeyMap[4];

typedef struct { } ICInstance;

typedef struct { } EventRecord;

typedef void** RgnHandle;

typedef void** MenuHandle;

typedef void** TEHandle;

// Gets STR# from resource fork
void GetIndString( const unsigned char*, int, int);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ANTARES_STUB_BASE_H_
