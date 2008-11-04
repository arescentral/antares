#ifndef ANTARES_STUB_QUICKDRAW_H_
#define ANTARES_STUB_QUICKDRAW_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// Content here
typedef struct { } PixMap;
typedef PixMap* PixMapPtr;
typedef PixMap** PixMapHandle;

typedef void** PicHandle;
typedef void** GDHandle;

typedef void* GWorldPtr;

typedef struct { } RGBColor;

PixMapHandle GetGWorldPixMap(GWorldPtr world);

extern GWorldPtr gOffWorld;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ANTARES_STUB_QUICKDRAW_H_
