#ifndef ANTARES_STUB_QDOFFSCREEN_H_
#define ANTARES_STUB_QDOFFSCREEN_H_

#include <Quickdraw.h>

typedef OSErr QDErr;

class ColorTable;

OSErr NewGWorld(GWorld** world, int, const Rect& bounds, const ColorTable& clut, GDHandle device, int pixDepth);
void DisposeGWorld(GWorld* world);
void GetGWorld(GWorld** world, GDHandle* device);
void SetGWorld(GWorld* world, GDHandle* device);

#endif // ANTARES_STUB_QDOFFSCREEN_H_
