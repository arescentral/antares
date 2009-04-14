#ifndef ANTARES_STUB_QDOFFSCREEN_H_
#define ANTARES_STUB_QDOFFSCREEN_H_

#include <Quickdraw.h>

typedef OSErr QDErr;

OSErr NewGWorld(GWorld** world, int, Rect* bounds, CTabHandle clut, GDHandle device, int pixDepth);
void DisposeGWorld(GWorld* world);
void GetGWorld(GWorld** world, GDHandle* device);
void SetGWorld(GWorld* world, GDHandle* device);

#endif // ANTARES_STUB_QDOFFSCREEN_H_
