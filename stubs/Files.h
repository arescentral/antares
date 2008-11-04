#ifndef ANTARES_STUB_FILES_H_
#define ANTARES_STUB_FILES_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <Base.h>

typedef SInt16 FSVolumeRefNum;

typedef struct {
    FSVolumeRefNum vRefNum;
    SInt32 parID;
    StrFileName name;
} FSSpec;
typedef FSSpec* FSSpecPtr;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ANTARES_STUB_FILES_H_
