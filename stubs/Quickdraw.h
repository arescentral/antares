#ifndef ANTARES_STUB_QUICKDRAW_H_
#define ANTARES_STUB_QUICKDRAW_H_

#include "Base.h"
#include "PixMap.hpp"

namespace antares {

typedef Window* Port;

void RGBBackColor(const RgbColor& color);
void RGBForeColor(const RgbColor& color);

STUB1(InvalRect, void(const Rect& rect));
STUB1(ClipRect, void(const Rect& rect));

void CopyBits(PixMap* source, PixMap* source2, const Rect& source_rect,
      const Rect& source_rect2);

STUB0(NewRgn, Rgn**(), new Rgn*(new Rgn));
STUB1(DisposeRgn, void(Rgn** rgn));

void ScrollRect(const Rect& rect, int x, int y, const Rect& clip);

void FrameRect(const Rect& rect);
void MacFrameRect(const Rect& rect);

STUB1(FrameOval, void(const Rect& rect));
STUB1(PaintOval, void(const Rect& rect));

void MacLineTo(int x, int y);

void MoveTo(int x, int y);
void GetPen(Point* pen);

STUB1(SetClip, void(Rgn** clip));
STUB1(GetClip, void(Rgn** clip));

class ColorTable;
void RestoreEntries(const ColorTable& table);

}  // namespace antares

#endif // ANTARES_STUB_QUICKDRAW_H_
