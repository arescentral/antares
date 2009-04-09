#ifndef ANTARES_STUB_FONTS_H_
#define ANTARES_STUB_FONTS_H_

STUB0(InitFonts, void());
STUB1(SetFontByString, void(const unsigned char* name));
STUB1(TextSize, void(int size));
STUB1(TextFont, void(int font));
STUB1(TextFace, void(int face));  // bold, etc.
STUB1(StringWidth, int(const unsigned char* string), 0);
STUB1(GetFontNumByString, int(const unsigned char* string), 0);

enum {
    bold = 8000,
};

#endif // ANTARES_STUB_FONTS_H_
