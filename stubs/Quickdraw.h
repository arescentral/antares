#ifndef ANTARES_STUB_QUICKDRAW_H_
#define ANTARES_STUB_QUICKDRAW_H_

#include <Base.h>
#include <Files.h>

// Content here
typedef struct {
    Rect bounds;
    CTabHandle pmTable;
    long rowBytes;
    char* baseAddr;
    int pixelSize;
} PixMap;
typedef PixMap* PixMapPtr;
typedef PixMap** PixMapHandle;

typedef struct {
    Rect picFrame;
} Pic;
typedef Pic** PicHandle;

typedef struct {
    PixMapHandle gdPMap;
    Rect gdRect;
} GDevice;
typedef GDevice** GDHandle;

GDevice** GetGDevice();
void SetGDevice(GDevice** device);

typedef struct { } GWorld;
typedef GWorld* GWorldPtr;

typedef void* GrafPtr;
typedef GrafPtr CGrafPtr;

typedef struct { } Pattern;

void BackPat(Pattern* pattern);
void PenPat(Pattern* pattern);

typedef void* Port;

void RGBBackColor(RGBColor* color);
void RGBForeColor(RGBColor* color);
void HiliteColor(RGBColor* color);

PixMapHandle GetGWorldPixMap(GWorldPtr world);
OSErr LockPixels(PixMap** pix);
void UnlockPixels(PixMap** pix);

void InitGraf(GrafPtr* port);
void GetPort(GrafPtr* port);
void MacSetPort(GrafPtr port);
void PaintRect(Rect* rect);
void InvalRect(Rect* rect);
void ClipRect(Rect* rect);

void CopyBits(BitMap* source, BitMap* source2, Rect* source_rect,
              Rect* source_rect2, int mode, void*);

PicHandle GetPicture(int id);
PicHandle OpenPicture(Rect* source);
void KillPicture(PicHandle pic);
void DrawPicture(PicHandle pic, Rect*);
void ClosePicture();

OSErr ConvertPictToGIFFile(PicHandle pic, FSSpec* fsspec, int interlaced,
                           int transparencyNo, int depth, int palette);

GDHandle GetMainDevice();
GDHandle GetDeviceList();
GDHandle GetNextDevice(GDHandle gd);

Rgn** NewRgn();
void DisposeRgn(Rgn** rgn);
void RectRgn(Rgn** src, Rect* dst);
void GetMBarRgn(Rgn** rgn);
bool PtInRgn(Point p, Rgn** rgn);
void DiffRgn(Rgn**, Rgn**, Rgn**);
void MacUnionRgn(Rgn**, Rgn**, Rgn**);
void OpenRgn();
void CloseRgn(Rgn** rgn);
void ScrollRect(Rect* rect, int x, int y, Rgn** clip);

void MacFillRect(Rect* rect, Pattern* pattern);
void FrameRect(Rect* rect);
void MacFrameRect(Rect* rect);
void EraseRect(Rect* rect);

void FrameOval(Rect* rect);
void PaintOval(Rect* rect);

void MacLineTo(int x, int y);

void MoveTo(int x, int y);

void PenNormal();
void GetPen(Point* pen);
void DrawString(const unsigned char* string);

bool HasDepth(GDHandle device, int depth, int, int);
void SetDepth(GDHandle device, int depth, int, int);

enum {
    colorPaletteSystem = 1000,

    transparencyNo = 1100,

    srcCopy = 1200,
};

void SetClip(Rgn** clip);
void GetClip(Rgn** clip);
void PaintBehind(Window**, Rgn**);
void CalcVisBehind(Window**, Rgn**);

Rgn** LMGetGrayRgn();
void LMSetMBarHeight(int height);
Port* LMGetWMgrPort();
Window** LMGetWindowList();
int GetMBarHeight();

void ShowMenuBar();
void HideMenuBar();
bool IsMenuBarVisible();

typedef struct { } ReqListRec;
void RestoreEntries(CTab** table, void*, ReqListRec* recList);

#endif // ANTARES_STUB_QUICKDRAW_H_
