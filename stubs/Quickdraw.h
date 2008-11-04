#ifndef ANTARES_STUB_QUICKDRAW_H_
#define ANTARES_STUB_QUICKDRAW_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <Files.h>

// Content here
typedef struct { } PixMap;
typedef PixMap* PixMapPtr;
typedef PixMap** PixMapHandle;

typedef struct {
    Rect picFrame;
} Pic;
typedef Pic** PicHandle;

typedef struct { } BitMap;

typedef void** GDHandle;

typedef void* GWorldPtr;

typedef void* GrafPtr;

typedef struct { } RGBColor;

PixMapHandle GetGWorldPixMap(GWorldPtr world);

extern GWorldPtr gOffWorld;

void GetPort(GrafPtr* port);
void MacSetPort(GrafPtr port);
void PaintRect(Rect* rect);

void CopyBits(BitMap* source, BitMap* source2, Rect* source_rect,
              Rect* source_rect2, int mode, void*);

PicHandle GetPicture(int id);
PicHandle OpenPicture(Rect* source);
void KillPicture(PicHandle pic);
void ClosePicture();

OSErr ConvertPictToGIFFile(PicHandle pic, FSSpec* fsspec, int interlaced,
                           int transparencyNo, int depth, int palette);

enum {
    colorPaletteSystem = 1000,

    transparencyNo = 1100,

    srcCopy = 1200,
};

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ANTARES_STUB_QUICKDRAW_H_
