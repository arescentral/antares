#ifndef ANTARES_STUB_QDOFFSCREEN_H_
#define ANTARES_STUB_QDOFFSCREEN_H_

#include <Quickdraw.h>

typedef OSErr QDErr;

STUB6(NewGWorld,
    QDErr(GWorld** world, int, Rect* bounds, CTabHandle clut, GDHandle device, int pixDepth),
    noErr);
STUB1(DisposeGWorld, void(GWorld* world));
STUB2(GetGWorld, void(GWorld** world, GDHandle* device));
STUB2(SetGWorld, void(GWorld* world, GDHandle* device));

#endif // ANTARES_STUB_QDOFFSCREEN_H_
