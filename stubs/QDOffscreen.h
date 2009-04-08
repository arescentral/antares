#ifndef ANTARES_STUB_QDOFFSCREEN_H_
#define ANTARES_STUB_QDOFFSCREEN_H_

#include <Quickdraw.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef OSErr QDErr;

QDErr NewGWorld(GWorld** world, int, Rect* bounds, CTabHandle clut, GDHandle device, int pixDepth);
void DisposeGWorld(GWorld* world);
void GetGWorld(GWorld** world, GDHandle* device);
void SetGWorld(GWorld* world, GDHandle* device);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ANTARES_STUB_QDOFFSCREEN_H_
