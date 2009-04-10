#ifndef ANTARES_STUB_RESOURCES_H_
#define ANTARES_STUB_RESOURCES_H_

#include <Base.h>
#include <Files.h>

typedef void* ResFile;

STUB0(CurResFile, int(), 0);
STUB1(CloseResFile, void(int res_file));
STUB1(UseResFile, void(int res_file));
STUB1(HomeResFile, int(Handle handle), 0);

inline Handle GetResource(FourCharCode code, int id) {
  gdb();
  return new char*(new char[1024]);
}
STUB1(ReleaseResource, void(Handle handle));
STUB1(DetachResource, void(Handle handle));

STUB1(GetStringList, Handle(int id), NULL);
STUB2(FindStringList, int(Handle handle, unsigned char* result), 0);
STUB1(DisposeStringList, void(Handle handle));
STUB1(StringListSize, int(Handle handle), 0);
STUB3(RetrieveIndString, void(Handle handle, int index, unsigned char* result));

STUB2(FSpOpenResFile, int(FSSpec* spec, int options), 0);

enum {
    resNotFound = 2000,
};

#endif // ANTARES_STUB_RESOURCES_H_
