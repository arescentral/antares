#ifndef ANTARES_STUB_FONTS_H_
#define ANTARES_STUB_FONTS_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void InitFonts();
void SetFontByString(const unsigned char* name);
void TextSize(int size);
void TextFont(int font);
void TextFace(int face);  // bold, etc.
int StringWidth(const unsigned char* string);
int GetFontNumByString(const unsigned char* string);

enum {
    bold = 8000,
};

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ANTARES_STUB_FONTS_H_
