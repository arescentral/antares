#ifndef ANTARES_STUB_PALETTES_H_
#define ANTARES_STUB_PALETTES_H_

#include <Base.h>

typedef struct { } Palette;
typedef Palette* PalettePtr;
typedef Palette** PaletteHandle;

enum {
    pmExplicit = 7000,
    pmTolerant = 7001,
    pmAllUpdates = 7002,
};

STUB4(NewPalette,
    PaletteHandle(int colors, CTabHandle clut, int options, int),
    new Palette*(new Palette));
STUB1(GetPalette, PaletteHandle(WindowPtr window), new Palette*(new Palette));
STUB1(DisposePalette, void(PaletteHandle palette));
STUB3(SetPalette, void(WindowPtr window, PaletteHandle palette, bool));
STUB1(ActivatePalette, void(WindowPtr window));

STUB4(CTab2Palette, void(CTab** clut, Palette** palette, int options, int));
STUB3(NSetPalette, void(Window* window, Palette** palette, int options));

#endif // ANTARES_STUB_PALETTES_H_
