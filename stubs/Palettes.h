#ifndef ANTARES_STUB_PALETTES_H_
#define ANTARES_STUB_PALETTES_H_

#include <Base.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef void** PaletteHandle;

enum {
    pmExplicit = 7000,
    pmTolerant = 7001,
};

PaletteHandle NewPalette(int colors, CTabHandle clut, int options, int);
PaletteHandle GetPalette(WindowPtr window);
void DisposePalette(PaletteHandle palette);
void SetPalette(WindowPtr window, PaletteHandle palette, bool);
void ActivatePalette(WindowPtr window);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ANTARES_STUB_PALETTES_H_
