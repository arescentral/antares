#ifndef ANTARES_STUB_FILES_H_
#define ANTARES_STUB_FILES_H_

#include <Base.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef SInt16 FSVolumeRefNum;

typedef struct {
    FSVolumeRefNum vRefNum;
    SInt32 parID;
    StrFileName name;
} FSSpec;
typedef FSSpec* FSSpecPtr;

OSErr FSMakeFSSpec(int, int, const unsigned char* forceName, FSSpec* fsspec);
OSErr FSWrite(short fileno, Size* size, void* data);
OSErr FSClose(short fileno);
OSErr FSpOpenDF(FSSpec* fsspec, int, short* fileno);
OSErr FSpCreate(FSSpec* fsspec, FourCharCode, FourCharCode, int);
OSErr FSpDelete(FSSpec* fsspec);

enum {
    smSystemScript = 4000,

    fsCurPerm = 4100,
    fsRdPerm = 4101,

    fnfErr = 4200,  // File not found?
    dupFNErr = 4201,  // Duplicate file name?
};

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ANTARES_STUB_FILES_H_
