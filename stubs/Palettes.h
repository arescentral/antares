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

PaletteHandle NewPalette(int colors, CTabHandle clut, int options, int);
PaletteHandle GetPalette(WindowPtr window);
void DisposePalette(PaletteHandle palette);
void SetPalette(WindowPtr window, PaletteHandle palette, bool);
void ActivatePalette(WindowPtr window);

void CTab2Palette(CTab** clut, Palette** palette, int options, int);
void NSetPalette(Window* window, Palette** palette, int options);

#endif // ANTARES_STUB_PALETTES_H_
