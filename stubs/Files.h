#ifndef ANTARES_STUB_FILES_H_
#define ANTARES_STUB_FILES_H_

#include <Base.h>

typedef SInt16 FSVolumeRefNum;

typedef struct {
    FSVolumeRefNum vRefNum;
    SInt32 parID;
    StrFileName name;
} FSSpec;
typedef FSSpec* FSSpecPtr;

STUB4(FSMakeFSSpec, OSErr(int, int, const unsigned char* forceName, FSSpec* fsspec), noErr);
STUB3(FSWrite, OSErr(short fileno, Size* size, void* data), noErr);
STUB1(FSClose, OSErr(short fileno), noErr);
STUB3(FSpOpenDF, OSErr(FSSpec* fsspec, int, short* fileno), noErr);
STUB4(FSpCreate, OSErr(FSSpec* fsspec, FourCharCode, FourCharCode, int), noErr);
STUB1(FSpDelete, OSErr(FSSpec* fsspec), noErr);

enum {
    smSystemScript = 4000,

    fsCurPerm = 4100,
    fsRdPerm = 4101,

    fnfErr = 4200,  // File not found?
    dupFNErr = 4201,  // Duplicate file name?
};

#endif // ANTARES_STUB_FILES_H_
