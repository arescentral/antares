#ifndef ANTARES_STUB_PALETTES_H_
#define ANTARES_STUB_PALETTES_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef void** PaletteHandle;

enum {
    pmExplicit = 7000,
    pmTolerant = 7001,
};

PaletteHandle NewPalette(int colors, CTabHandle clut, int options, int);
void SetPalette(WindowPtr window, PaletteHandle palette, bool);
void ActivatePalette(WindowPtr window);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ANTARES_STUB_PALETTES_H_
