#ifndef ANTARES_STUB_RESOURCES_H_
#define ANTARES_STUB_RESOURCES_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <Base.h>
#include <Files.h>

typedef void* ResFile;

int CurResFile();
void CloseResFile(int res_file);
void UseResFile(int res_file);
int HomeResFile(Handle handle);

Handle GetResource(FourCharCode code, int id);
void ReleaseResource(Handle handle);
void DetachResource(Handle handle);

Handle GetStringList(int id);
int FindStringList(Handle handle, unsigned char* result);

int FSpOpenResFile(FSSpec* spec, int options);

enum {
    fsRdPerm = 2000,
    resNotFound = 2001,
};

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ANTARES_STUB_RESOURCES_H_
