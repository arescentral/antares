#ifndef ANTARES_STUB_QUICKDRAW_H_
#define ANTARES_STUB_QUICKDRAW_H_

#include <Base.h>

namespace antares {

typedef Window* GrafPtr;
typedef GrafPtr CGrafPtr;

typedef struct { } Pattern;

typedef Window* Port;

void RGBBackColor(RGBColor* color);
void RGBForeColor(RGBColor* color);
STUB1(HiliteColor, void(RGBColor* color));

STUB1(LockPixels, bool(PixMap** pix), true);
STUB1(UnlockPixels, void(PixMap** pix));

void GetPort(GrafPtr* port);
void MacSetPort(GrafPtr port);
STUB1(InvalRect, void(const Rect& rect));
STUB1(ClipRect, void(const Rect& rect));

void CopyBits(PixMap* source, PixMap* source2, const Rect& source_rect,
      const Rect& source_rect2, int mode, void*);

STUB0(NewRgn, Rgn**(), new Rgn*(new Rgn));
STUB1(DisposeRgn, void(Rgn** rgn));

void ScrollRect(const Rect& rect, int x, int y, const Rect& clip);

void PaintRect(const Rect& rect);
void FrameRect(const Rect& rect);
void MacFrameRect(const Rect& rect);
void EraseRect(const Rect& rect);

STUB1(FrameOval, void(const Rect& rect));
STUB1(PaintOval, void(const Rect& rect));

void MacLineTo(int x, int y);

void MoveTo(int x, int y);
void GetPen(Point* pen);

STUB0(PenNormal, void());

enum {
    colorPaletteSystem = 1000,

    transparencyNo = 1100,

    srcCopy = 1200,
};

STUB1(SetClip, void(Rgn** clip));
STUB1(GetClip, void(Rgn** clip));

class ColorTable;
void RestoreEntries(const ColorTable& table);

}  // namespace antares

#endif // ANTARES_STUB_QUICKDRAW_H_
