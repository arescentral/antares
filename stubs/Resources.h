#ifndef ANTARES_STUB_RESOURCES_H_
#define ANTARES_STUB_RESOURCES_H_

#include <Base.h>
#include <Files.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

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
    resNotFound = 2000,
};

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ANTARES_STUB_RESOURCES_H_
