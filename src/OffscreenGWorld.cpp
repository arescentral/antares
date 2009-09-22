// Ares, a tactical space combat game.
// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

// Offscreen Graphics: new & clean, using GWorlds etc

#include "OffscreenGWorld.hpp"

#include <QDOffscreen.h>

#include "AresGlobalType.hpp"
#include "ConditionalMacros.h"
#include "Debug.hpp"
#include "Error.hpp"
#include "GDeviceHandling.hpp"
#include "Options.hpp"

//#define   kByteLevelTesting

#ifdef kByteLevelTesting
#include "SpriteHandling.hpp"    // for testbyte debigging kludge
#endif

#define kOffscreenError     "\pGWLD"

extern aresGlobalType   *gAresGlobal;
extern long             WORLD_WIDTH, WORLD_HEIGHT;
extern GDHandle         theDevice;
//extern unsigned long  gAresGlobal->gOptions;

GWorldPtr       gOffWorld, gRealWorld, gSaveWorld;
long            gNatePortLeft, gNatePortTop;
//PixPatHandle  gWhitePattern = nil, gBlackPattern = nil;


int CreateOffscreenWorld(const Rect& bounds, const ColorTable& theClut) {
    QDErr           error;
    PixMapHandle    pixBase;
    Rect            tRect;
    GDHandle        originalDevice;

    //
    //  NewGWorld creates the world.  See p.21-12--13 in IM VI.  Note that
    //  by setting pix depth to 0, we are using bounds as global rect which
    //  determines the depth of the GWorld by using the deepest depth of any
    //  device that rect intersects.  We should use the bounding rect of our
    //  device of choice, but I'm too lazy.
    //

    GetGWorld( &gRealWorld, &originalDevice);

    pixBase = GetGWorldPixMap( gRealWorld);
    error = NewGWorld(&gOffWorld, 8, bounds, theClut, theDevice, 0);
    if ( error)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, OFFSCREEN_GRAPHICS_ERROR, -1, -1, -1, __FILE__, 1);
        return( OFFSCREEN_GRAPHICS_ERROR);
    }

    error = NewGWorld(&gSaveWorld, 8, bounds, theClut, theDevice, 0);
    if ( error)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, OFFSCREEN_GRAPHICS_ERROR, -1, -1, -1, __FILE__, 2);

        DisposeGWorld( gOffWorld);
        return( OFFSCREEN_GRAPHICS_ERROR);
    }

    pixBase = GetGWorldPixMap( gSaveWorld);
    error = LockPixels( pixBase);
    if ( !error)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, OFFSCREEN_GRAPHICS_ERROR, -1, -1, -1, __FILE__, 3);
        DisposeGWorld( gOffWorld);
        DisposeGWorld( gSaveWorld);
        return( OFFSCREEN_GRAPHICS_ERROR);
    }

    pixBase = GetGWorldPixMap( gOffWorld);
    error = LockPixels( pixBase);
    if ( !error)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, OFFSCREEN_GRAPHICS_ERROR, -1, -1, -1, __FILE__, 4);
        DisposeGWorld( gOffWorld);
        DisposeGWorld( gSaveWorld);
        return( OFFSCREEN_GRAPHICS_ERROR);
    }

//  SetRect( &tRect, 0, 0, WORLD_WIDTH, WORLD_HEIGHT);
    tRect = bounds;
    CenterRectInDevice( theDevice, &tRect);
    gNatePortLeft = /*(*(*theDevice)->gdPMap)->bounds.left + */tRect.left - (*theDevice)->gdRect.left;
    gNatePortLeft /= 4;

    mWriteDebugString("\pNatePortLeft:");
    WriteDebugLong( gNatePortLeft);

    gNatePortTop = /*(*(*theDevice)->gdPMap)->bounds.top + */tRect.top - (*theDevice)->gdRect.top;

//  gNatePortLeft = tRect.left / 4;
//  gNatePortTop = tRect.top;
    EraseOffWorld();
    EraseSaveWorld();

    return ( kNoError);
}

void CleanUpOffscreenWorld( void)

{
    PixMapHandle    pixBase;

    DrawInRealWorld();
    if ( gOffWorld != nil)
    {
        pixBase = GetGWorldPixMap( gOffWorld);
        UnlockPixels( pixBase);
        DisposeGWorld( gOffWorld);
    }
    WriteDebugLine("\p<OffWorld");

    if ( gSaveWorld != nil)
    {
        pixBase = GetGWorldPixMap( gSaveWorld);
        UnlockPixels( pixBase);
        DisposeGWorld( gSaveWorld);
    }
    WriteDebugLine("\p<SaveWorld");
}

void DrawInRealWorld( void)

{
    SetGWorld( gRealWorld, nil);
}

void DrawInOffWorld ( void)

{
    SetGWorld( gOffWorld, nil);
}

void DrawInSaveWorld ( void)

{
    SetGWorld( gSaveWorld, nil);
}

void EraseOffWorld( void)

{
    RGBColor    c;
    PixMapHandle pixBase;

    DrawInOffWorld();
    pixBase = GetGWorldPixMap( gOffWorld);
    EraseRect( &((*pixBase)->bounds));
    c.red = c.blue = c.green = 0;
    RGBForeColor( &c);
    PaintRect( &((*pixBase)->bounds));
    NormalizeColors();
    DrawInRealWorld();
    NormalizeColors();
}

void EraseSaveWorld( void)

{
    PixMapHandle pixBase;
    RGBColor    c;

    DrawInSaveWorld();
    pixBase = GetGWorldPixMap( gSaveWorld);
    NormalizeColors();
    EraseRect( &((*pixBase)->bounds));
    c.red = c.blue = c.green = 0;
    RGBForeColor( &c);
    PaintRect( &((*pixBase)->bounds));
    NormalizeColors();
    DrawInRealWorld();
}

void CopyOffWorldToRealWorld( WindowPtr port, Rect *bounds)

{
    PixMapHandle pixBase;

    pixBase = GetGWorldPixMap( gOffWorld);
    NormalizeColors();
    CopyBits( *pixBase, &(port->portBits), bounds, bounds,
        srcCopy, nil);
}

void CopyRealWorldToSaveWorld( WindowPtr port, Rect *bounds)

{
    PixMapHandle pixBase;

    pixBase = GetGWorldPixMap( gSaveWorld);
    NormalizeColors();
    CopyBits( &(port->portBits), *pixBase, bounds, bounds,
        srcCopy, nil);
}

void CopyRealWorldToOffWorld( WindowPtr port, Rect *bounds)

{
    PixMapHandle pixBase;

    pixBase = GetGWorldPixMap( gOffWorld);
    NormalizeColors();
    CopyBits( &(port->portBits), *pixBase, bounds, bounds,
        srcCopy, nil);
}

void CopySaveWorldToOffWorld( Rect *bounds)

{
    PixMapHandle savePixBase, offPixBase;

    savePixBase = GetGWorldPixMap( gSaveWorld);
    offPixBase = GetGWorldPixMap( gOffWorld);
    NormalizeColors();
    CopyBits( *savePixBase, *offPixBase, bounds, bounds,
        srcCopy, nil);
}

void CopyOffWorldToSaveWorld( Rect *bounds)

{
    PixMapHandle savePixBase, offPixBase;

    savePixBase = GetGWorldPixMap( gSaveWorld);
    offPixBase = GetGWorldPixMap( gOffWorld);
    NormalizeColors();
    CopyBits( *offPixBase, *savePixBase, bounds, bounds,
        srcCopy, nil);
}


void NormalizeColors( void)

{
    RGBColor    c;

    c.red = c.blue = c.green = 0;
    RGBForeColor ( &c);
    // c.red = c.blue = c.green = 65535;
    c.red = c.blue = c.green = 255;
    RGBBackColor( &c);
}

void SetBlackBack( void)

{
//  BackPixPat( gBlackPattern);
}

void GWorldExperiment( void)

{
    PixMapHandle    pixBase;
    int             x;
    unsigned char   *p;

    pixBase = GetGWorldPixMap( gOffWorld);
    p = (*pixBase)->baseAddr;
    for ( x = 0; x < 200; x++)
        *p++ = 0x80;
    DrawInRealWorld();
    NormalizeColors();
}

void ChunkCopyPixMapToScreenPixMap( PixMap *sourcePix, Rect *sourceRect, PixMap *destMap)
{
    int     x, y;
    long    *sword, *dword, srowplus, drowplus, srowbytes, drowbytes, sright;
    Rect    fixRect;

    if (( sourceRect->right <= sourceRect->left) || ( sourceRect->bottom <= sourceRect->top))
    {
        // rect does not really exist
    } else
    {
        if (( gAresGlobal->gOptions & kOptionQDOnly) || ( 1))
        {
            fixRect = *sourceRect;
            MacOffsetRect( &fixRect, gNatePortLeft << 2, gNatePortTop);
            CopyBits( sourcePix, destMap, sourceRect, &fixRect,
                srcCopy, nil);
        } else
        {
            fixRect = *sourceRect;
            fixRect.left /= 4;
            if (( fixRect.right % 4) == 0)
                fixRect.right /= 4;
            else fixRect.right = fixRect.right / 4 + 1;
            srowbytes = sourcePix->rowBytes & 0x3fff;
            srowbytes /= 4;
            drowbytes = destMap->rowBytes & 0x3fff;
            drowbytes /= 4;
            sright = sourcePix->bounds.right / 4;
            if ( fixRect.left < 0)
                fixRect.left = 0;
            if ( fixRect.right > sright)
                fixRect.right = sright;
            if ( fixRect.right > drowbytes)
                fixRect.right = drowbytes;
            if ( fixRect.top < 0)
                fixRect.top = 0;
            if ( fixRect.bottom > sourcePix->bounds.bottom)
                fixRect.bottom = sourcePix->bounds.bottom;
            if ( fixRect.bottom > ( destMap->bounds.bottom - destMap->bounds.top))
                fixRect.bottom = ( destMap->bounds.bottom - destMap->bounds.top);
            srowplus = srowbytes - (fixRect.right - fixRect.left);
            drowplus = drowbytes - (fixRect.right - fixRect.left);
            sword = reinterpret_cast<long *>(sourcePix->baseAddr) + fixRect.top * srowbytes +
                    fixRect.left;
            dword = reinterpret_cast<long *>(destMap->baseAddr) + (fixRect.top + gNatePortTop) * drowbytes +
                    (fixRect.left + gNatePortLeft);
            for ( y = fixRect.top; y < fixRect.bottom; y++)
            {
                for ( x = fixRect.left; x < fixRect.right; x++)
                {
        #ifdef kByteLevelTesting
                    TestByte( reinterpret_cast<char*>(dword), destMap, "\pPIX2SCRN");
        #endif
                    *dword++ = *sword++;
                }
                dword += drowplus;
                sword += srowplus;
            }
        }
    }
}

void ChunkCopyPixMapToPixMap( PixMap *sourcePix, Rect *sourceRect, PixMap *destMap)

{
    int     x, y;
    long    *sword, *dword, srowplus, drowplus, srowbytes, drowbytes, sright;
    Rect    fixRect;

    fixRect = *sourceRect;
    fixRect.left >>= 2;
    if (( fixRect.right & 0x0003) == 0)
        fixRect.right >>= 2;
    else fixRect.right = (fixRect.right >> 2) + 1;
    srowbytes = sourcePix->rowBytes;
    srowbytes &= 0x00003fff;
    srowbytes >>= 2;
    drowbytes = destMap->rowBytes;
    drowbytes &= 0x00003fff;
    drowbytes >>= 2;
    sright = sourcePix->bounds.right;
    sright >>= 2;
    if ( fixRect.left < 0)
        fixRect.left = 0;
    if ( fixRect.right > sright)
        fixRect.right = sright;
    if ( fixRect.right > drowbytes)
        fixRect.right = drowbytes;
    if ( fixRect.top < 0)
        fixRect.top = 0;
    if ( fixRect.bottom > sourcePix->bounds.bottom)
        fixRect.bottom = sourcePix->bounds.bottom;
    if ( fixRect.bottom > destMap->bounds.bottom)
        fixRect.bottom = destMap->bounds.bottom;
    srowplus = srowbytes - (fixRect.right - fixRect.left);
    drowplus = drowbytes - (fixRect.right - fixRect.left);
    sword = reinterpret_cast<long *>(sourcePix->baseAddr) + fixRect.top * srowbytes +
            fixRect.left;
    dword = reinterpret_cast<long *>(destMap->baseAddr) + (fixRect.top) * drowbytes +
            (fixRect.left);
    for ( y = fixRect.top; y < fixRect.bottom; y++)
    {
        for ( x = fixRect.left; x < fixRect.right; x++)
        {
#ifdef kByteLevelTesting
            TestByte( reinterpret_cast<char*>(dword), destMap, "\pPIX2PIX");
#endif
            *dword++ = *sword++;
        }
        dword += drowplus;
        sword += srowplus;
    }
}

void ChunkErasePixMap( PixMap *destMap, Rect *sourceRect)

{
    int     x, y;
    long    *dword, drowplus, drowbytes, sright;
    Rect    fixRect;

    fixRect = *sourceRect;
    fixRect.left >>= 2;
    if (( fixRect.right & 0x0003) == 0)
        fixRect.right >>= 2;
    else fixRect.right = (fixRect.right >> 2) + 1;
    drowbytes = destMap->rowBytes;
    drowbytes &= 0x00003fff;
    drowbytes >>= 2;
    sright = destMap->bounds.right;
    sright >>= 2;
    if ( fixRect.left < 0)
        fixRect.left = 0;
    if ( fixRect.right > sright)
        fixRect.right = sright;
    if ( fixRect.right > drowbytes)
        fixRect.right = drowbytes;
    if ( fixRect.top < 0)
        fixRect.top = 0;
    if ( fixRect.bottom > destMap->bounds.bottom)
        fixRect.bottom = destMap->bounds.bottom;
    if ( fixRect.bottom > destMap->bounds.bottom)
        fixRect.bottom = destMap->bounds.bottom;
    drowplus = drowbytes - (fixRect.right - fixRect.left);
    dword = reinterpret_cast<long *>(destMap->baseAddr) + (fixRect.top) * drowbytes +
            (fixRect.left);
    for ( y = fixRect.top; y < fixRect.bottom; y++)
    {
        for ( x = fixRect.left; x < fixRect.right; x++)
        {
#ifdef kByteLevelTesting
            TestByte( reinterpret_cast<char*>(dword), destMap, "\pPIXERSE");
#endif
            *dword++ = 0xffffffff;
        }
        dword += drowplus;
    }
}
