#ifndef ANTARES_STUB_QUICKDRAW_H_
#define ANTARES_STUB_QUICKDRAW_H_

#include <Base.h>
#include <Files.h>

struct GWorld;
typedef GWorld* GWorldPtr;

typedef struct {
    PixMapHandle gdPMap;
    Rect gdRect;
    GWorld* world;
} GDevice;
typedef GDevice** GDHandle;

extern GDevice* fakeGDevicePtr;
STUB0(GetGDevice, GDevice**(), &fakeGDevicePtr);
STUB1(SetGDevice, void(GDevice** device));

typedef Window* GrafPtr;
typedef GrafPtr CGrafPtr;

typedef struct { } Pattern;

STUB1(BackPat, void(Pattern*));
STUB1(PenPat, void(Pattern*));

typedef Window* Port;

void RGBBackColor(RGBColor* color);
void RGBForeColor(RGBColor* color);
STUB1(HiliteColor, void(RGBColor* color));

PixMapHandle GetGWorldPixMap(GWorldPtr world);
STUB1(LockPixels, bool(PixMap** pix), true);
STUB1(UnlockPixels, void(PixMap** pix));

STUB1(InitGraf, void(GrafPtr* port));
void GetPort(GrafPtr* port);
void MacSetPort(GrafPtr port);
STUB1(InvalRect, void(Rect* rect));
STUB1(ClipRect, void(Rect* rect));

void CopyBits(BitMap* source, BitMap* source2, Rect* source_rect,
      Rect* source_rect2, int mode, void*);

STUB0(GetMainDevice, GDHandle(), &fakeGDevicePtr);
STUB0(GetDeviceList, GDHandle(), &fakeGDevicePtr);
STUB1(GetNextDevice, GDHandle(GDHandle gd), NULL);

STUB0(NewRgn, Rgn**(), new Rgn*(new Rgn));
STUB1(DisposeRgn, void(Rgn** rgn));
STUB2(RectRgn, void(Rgn** src, Rect* dst));
STUB2(PtInRgn, bool(Point p, Rgn** rgn), false);
STUB3(DiffRgn, void(Rgn**, Rgn**, Rgn**));
STUB3(MacUnionRgn, void(Rgn**, Rgn**, Rgn**));
STUB0(OpenRgn, void());
STUB1(CloseRgn, void(Rgn** rgn));
STUB4(ScrollRect, void(Rect* rect, int x, int y, Rgn** clip));

void MacFillRect(Rect* rect, Pattern* pattern);
void PaintRect(Rect* rect);
void FrameRect(Rect* rect);
void MacFrameRect(Rect* rect);
void EraseRect(Rect* rect);

STUB1(FrameOval, void(Rect* rect));
STUB1(PaintOval, void(Rect* rect));

void MacLineTo(int x, int y);

void MoveTo(int x, int y);
void GetPen(Point* pen);

STUB0(PenNormal, void());
STUB1(DrawString, void(const unsigned char* string));

STUB4(HasDepth, bool(GDHandle device, int depth, int, int), true);
STUB4(SetDepth, void(GDHandle device, int depth, int, int));

enum {
    colorPaletteSystem = 1000,

    transparencyNo = 1100,

    srcCopy = 1200,
};

STUB1(SetClip, void(Rgn** clip));
STUB1(GetClip, void(Rgn** clip));
STUB2(PaintBehind, void(Window**, Rgn**));
STUB2(CalcVisBehind, void(Window**, Rgn**));

STUB0(LMGetGrayRgn, Rgn**(), NULL);
STUB1(LMSetMBarHeight, void(int height));
STUB0(LMGetWMgrPort, Port(), NULL);
STUB0(LMGetWindowList, Window**(), NULL);
STUB0(GetMBarHeight, int(), 0);

STUB0(ShowMenuBar, void());
STUB0(HideMenuBar, void());
STUB0(IsMenuBarVisible, bool(), false);

typedef struct { } ReqListRec;

class ColorTable;
void RestoreEntries(const ColorTable& table, void*, ReqListRec* recList);

#endif // ANTARES_STUB_QUICKDRAW_H_
