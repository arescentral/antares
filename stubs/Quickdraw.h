#ifndef ANTARES_STUB_QUICKDRAW_H_
#define ANTARES_STUB_QUICKDRAW_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <Base.h>
#include <Files.h>

// Content here
typedef struct {
    Rect bounds;
    CTabHandle pmTable;
} PixMap;
typedef PixMap* PixMapPtr;
typedef PixMap** PixMapHandle;

typedef struct {
    Rect picFrame;
} Pic;
typedef Pic** PicHandle;

typedef struct { } BitMap;

typedef struct {
    PixMapHandle gdPMap;
    Rect gdRect;
} GDevice;
typedef GDevice** GDHandle;

typedef void* GWorldPtr;

typedef void* GrafPtr;
typedef GrafPtr CGrafPtr;

typedef struct { } Pattern;

void BackPat(Pattern* pattern);

typedef void* Port;

void RGBBackColor(RGBColor* color);
void RGBForeColor(RGBColor* color);

PixMapHandle GetGWorldPixMap(GWorldPtr world);

extern GWorldPtr gOffWorld;

void InitGraf(GrafPtr* port);
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

GDHandle GetMainDevice();

void MacFillRect(Rect* rect, Pattern* pattern);

void MoveTo(int x, int y);

void GetPen(Point* pen);
void DrawString(const unsigned char* string);

enum {
    colorPaletteSystem = 1000,

    transparencyNo = 1100,

    srcCopy = 1200,
};

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ANTARES_STUB_QUICKDRAW_H_
