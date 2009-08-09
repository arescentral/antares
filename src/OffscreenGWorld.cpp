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
#include <Palettes.h>

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


int CreateOffscreenWorld ( Rect *bounds, CTabHandle theClut)

{
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
    error = NewGWorld ( &gOffWorld, 8, bounds, theClut, theDevice, 0);
    if ( error)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, OFFSCREEN_GRAPHICS_ERROR, -1, -1, -1, __FILE__, 1);
        return( OFFSCREEN_GRAPHICS_ERROR);
    }

    error = NewGWorld ( &gSaveWorld, 8, bounds, theClut, theDevice, 0);
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
    tRect = *bounds;
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
    WriteDebugLine(reinterpret_cast<const char*>("\p<OffWorld"));

    if ( gSaveWorld != nil)
    {
        pixBase = GetGWorldPixMap( gSaveWorld);
        UnlockPixels( pixBase);
        DisposeGWorld( gSaveWorld);
    }
    WriteDebugLine(reinterpret_cast<const char*>("\p<SaveWorld"));
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
    char            *p;

    pixBase = GetGWorldPixMap( gOffWorld);
    p = (*pixBase)->baseAddr;
    for ( x = 0; x < 200; x++)
        *p++ = 0x80;
    DrawInRealWorld();
    NormalizeColors();
}

#ifdef kDontDoLong68KAssem  //powercc

void ChunkCopyPixMapToScreenPixMap( PixMap *sourcePix, Rect *sourceRect, PixMap *destMap)

{
    int     x, y, width, height;
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

#else

void asm ChunkCopyPixMapToScreenPixMap( PixMap *sourcePix, Rect *sourceRect, PixMap *destMap)

{
#ifdef kAllowAssem

    register long   dr3, dr4, dr5, dr6, dr7;
    register long   *ar2, *ar3, *ar4;

    long    *sword, *dword, srowplus, drowplus, srowbytes, drowbytes, sright, x, y, width,
            height;
    Rect    fixRect;

    fralloc +

        movea.l     sourceRect, ar3         // ra0 = sourceRect

        move.w      struct(Rect.right)(ar3), dr3    // if sourceRect.right
        cmp.w       struct(Rect.left)(ar3), dr3     // <= sourceRect.left
        ble         donotdraw                       // don't draw
        move.w      struct(Rect.bottom)(ar3), dr3   // if sourceRect.bottom
        cmp.w       struct(Rect.top)(ar3), dr3      // <= sourceRect.top
        ble         donotdraw                       // don't draw

//      moveq       #0x40, dr3              // if gAresGlobal->gOptions & kOptionsQDOnly
//      and.l       gAresGlobal->gOptions, dr3
//      bne         chunkquickdrawcopy      // then copybits

        movea.l     destMap, ar4                // rar4 = destMap

        move.l      (ar3), fixRect.top      // fixRect.left = sourceRect.left &
                                            // fixRect.top = sourceRect.top
        move.l      0x04(ar3), fixRect.bottom   // fixRect.right = source.right &
                                                // fix.bottom = source.bottom
        movea.l     sourcePix, ar3          // rar3 = sourcePix
        move.w      fixRect.left, dr3       // fixRect.left /= 4
        lsr.w       #0x02, dr3
        move.w      dr3, fixRect.left
        move.w      fixRect.right, dr3      // dr3 = fixRect.right
        addi.w      #0x03, dr3              // we want to round up
        lsr.w       #0x02, dr3              // dr3 /= 4
        addq.l      #0x1, dr3
        move.w      dr3, fixRect.right      // fixRect.right = dr3
        ext.l       dr3

        move.w      struct(PixMap.rowBytes)(ar3), dr7           // dr7 = sourcePix->rowBytes
        andi.l      #0x00003fff, dr7            // dr7 &= 0x3FFFF
        lsr.l       #0x02, dr7              // dr7 /= 4 (srowbytes)

        move.w      struct(PixMap.rowBytes)(ar4), dr4           // dr4 = destMap->rowBytes // 0x04
        andi.l      #0x00003fff, dr4            // dr4 &= 0x3FFFF
        lsr.l       #0x02, dr4              // dr4 /= 4 (drowbytes)

        move.w      0x000c(ar3), dr5            // dr5 = sourcePix->bounds.right
        ext.l       dr5
        lsr         #2, dr5             // dr5 /= 4 (sright)

        tst.w       fixRect.left            // if fixRect.left < 0
        bge         leftIsGreaterThan0
        clr.w       fixRect.left            // then fixRect.left = 0

    leftIsGreaterThan0:
        cmp.l       dr5, dr3                    // if fixRect.right > sright
        ble         rightIsLessThanSRight
        move.l      dr5, dr3                    // then fixRect.right = sright

    rightIsLessThanSRight:
        cmp.l       dr4, dr3                    // if fixRect.right > drowbytes
        ble         rightIsLessThanDRowBytes
        move.l      dr4, dr3

    rightIsLessThanDRowBytes:
        move.w      dr3, fixRect.right
        move.w      fixRect.bottom, dr3     // dr3 = fixRect.bottom
        tst.w       fixRect.top             // if fixRect.top < 0
        bge         topIsGreaterThan0
        clr.w       fixRect.top             // then fixRect.top = 0

    topIsGreaterThan0:
        cmp.w       struct(PixMap.bounds.bottom)(ar3), dr3  // if fixRect.bottom > sourcePix->bounds.bottom
        ble         bottomIsLessThanSourceBottom
        move.w      struct(PixMap.bounds.bottom)(ar3),dr3   // then fixRect.bottom = sourcePix->bounds.bottom

    bottomIsLessThanSourceBottom:
        move.w      struct(PixMap.bounds.bottom)(ar4), dr6  // dr6 = destMap->bounds.bottom
        sub.w       struct(PixMap.bounds.top)(ar4), dr6     // dr6 -= destMap->bounds.top ( = height of destMap)
        cmp.w       dr6,dr3 // if fixRect.bottom > destMap->bounds.bottom
        ble         bottomIsLessThanDestBottom
        move.w      dr6,dr3

/*      cmp.w       struct(PixMap.bounds.bottom)(ar4),dr3   // if fixRect.bottom > destMap->bounds.bottom
        ble         bottomIsLessThanDestBottom
        move.w      struct(PixMap.bounds.bottom)(ar4),dr3
*/
    bottomIsLessThanDestBottom:
        move.w      dr3, fixRect.bottom;

        move.w      fixRect.top, dr3            // dr3 = fixRect.top
        ext.l       dr3
        mulu.w      dr7, dr3                    // dr3 *= srowbytes
        lsl.l       #0x02, dr3              // dr3 *= 4 b/c we're dealing with a long*
        movea.l     (ar3),ar2                   // ar2 = sourcePix->baseAddr
        adda.l      dr3, ar2                    // ar2 += dr3
        move.w      fixRect.left, dr3       // dr3 = fixRect.left
        ext.l       dr3
        lsl.l       #0x02, dr3              // dr3 *= 4 because wer'e dealing with a long*
        adda.l      dr3, ar2                    // ar2 += dr3

        movea.l     (ar4), ar3              // ar3 = destMap->baseAddr (we're done with sourcePix)
        adda.l      dr3, ar3                    // ar3 += fixRect.left * 4
        move.l      gNatePortLeft, dr3
        lsl         #0x02, dr3
        adda.l      dr3, ar3                    // ar3 += gNatePortLeft
        move.w      fixRect.top, dr3            // dr3 = fixRect.top
        ext.l       dr3
        add.l       gNatePortTop, dr3       // dr3 += gNatePortTop
        mulu.w      dr4, dr3                    // dr3 *= drowbytes
        lsl.l       #0x02, dr3              // dr3 *= 4
        adda.l      dr3, ar3

        move.w      fixRect.right, dr3      // dr3 = fixRect.right
        cmp.w       fixRect.left, dr3
        ble         donotdraw

        sub.w       fixRect.left, dr3       // dr3 -= fixRect.left

        ext.l       dr3
//      _Debugger
        move.w      fixRect.bottom, dr6     // dr6 = fixRect.bottom
        cmp.w       fixRect.top, dr6
        ble         donotdraw

        sub.w       fixRect.top, dr6            // dr6 -= fixRect.top
        subi.w      #1, dr6
//      tst.w       dr6
//      ble         donotdraw
//      move.w      fixRect.top, dr6        // testing
        ext.l       dr6

        sub.l       dr3, dr7                    // dr7 (srowbytes) -= dr3
        sub.l       dr3, dr4                    // dr4 (drowbytes) -= dr3
        subi.l      #0x01, dr3
        tst.l       dr3
        ble         donotdraw

        lsl.l       #2, dr7
        lsl.l       #2, dr4

    outerloop:
        move.l      dr3, dr5                    // dr5 = dr3 (width)
    innerloop:
        move.l      (ar2)+, (ar3)+
        dbra        dr5, innerloop
        adda.l      dr7, ar2
        adda.l      dr4, ar3
        dbra        dr6, outerloop
//      addi.w      #1, dr6
//      cmp.w       fixRect.bottom, dr6
//      blt         outerloop
        jmp         donotdraw

/*
00000004: 48E7 1F38          MOVEM.L   D3-D7/A2-A4,-(A7)
00000008: 266E 0008          MOVEA.L   $0008(A6),A3
0000000C: 246E 000C          MOVEA.L   $000C(A6),A2
00000010: 286E 0010          MOVEA.L   $0010(A6),A4
00000014: 302A 0006          MOVE.W    $0006(A2),D0
00000018: B06A 0002          CMP.W     $0002(A2),D0
0000001C: 6F00 019E          BLE       *+$01A0        ; 000001BC
00000020: 302A 0004          MOVE.W    $0004(A2),D0
00000024: B052               CMP.W     (A2),D0
00000026: 6F00 0194          BLE       *+$0196        ; 000001BC
0000002A: 7040               MOVEQ     #$40,D0        ; '@'
0000002C: C0AD 0000          AND.L     gAresGlobal->gOptions,D0
00000030: 6712               BEQ.S     *+$0014        ; 00000044
00000032: 2F0B               MOVE.L    A3,-(A7)
00000034: 2F0C               MOVE.L    A4,-(A7)
00000036: 2F0A               MOVE.L    A2,-(A7)
00000038: 2F0A               MOVE.L    A2,-(A7)
0000003A: 4267               CLR.W     -(A7)
0000003C: 42A7               CLR.L     -(A7)
0000003E: A8EC               _CopyBits
00000040: 6000 017A          BRA       *+$017C        ; 000001BC
*/
/* we know that ar3 = sourcerect
*/
    chunkquickdrawcopy:
        move.l      (ar3), fixRect.top      // fixRect.left = sourceRect.left &
                                            // fixRect.top = sourceRect.top
        move.l      0x04(ar3), fixRect.bottom   // fixRect.right = source.right &
                                                // fix.bottom = source.bottom
        move.w      fixRect.left, dr5

        pea         fixRect
        move.l      gNatePortLeft, dr3
        lsl         #0x02, dr3
        move.w      dr3, -(A7)
        move.l      gNatePortTop, dr3
        move.w      dr3, -(A7)
        _OffsetRect

        move.w      fixRect.left, dr5

        movea.l     sourcePix, ar2
        movea.l     destMap, ar4
        move.l      ar2, -(A7)
        move.l      ar4, -(A7)
        move.l      ar3, -(A7)
        pea         fixRect
        clr.w       -(A7)
        clr.l       -(A7)
        _CopyBits

    donotdraw:

        frfree

#endif // kAllowAssem
        rts
}

#endif


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

// from p104 of Tricks of the Mac Game Programming Gurus
void SetWindowPaletteFromClut( CWindowPtr theWindow, CTabHandle theClut)

{
    PaletteHandle   thePalette;

    thePalette = GetPalette( theWindow); // get the windows current palette
    if ( thePalette == nil) // if it doesn't exist
    {
        thePalette = NewPalette( (**theClut).ctSize + 1, theClut, pmTolerant + pmExplicit, 0x000); // make new palette
        // note that ctSize is zero-based
    } else
    {
        // change window's palette to match clut
        CTab2Palette( theClut, thePalette, pmTolerant + pmExplicit, 0x0000);
    }
    NSetPalette( theWindow, thePalette, pmAllUpdates);
    ActivatePalette( theWindow);
}

void ColorTest( void)

{

    CTabHandle      offCLUT, onCLUT;
    PixMapHandle    pixMap, onPixMap;
    Rect            tRect, uRect;
    int             i, j;

//  DrawInOffWorld();
//  SetPort( gTheWindow);
    pixMap = GetGWorldPixMap( gOffWorld);
    onPixMap = (*theDevice)->gdPMap;
//  onPixMap = GetGWorldPixMap( gRealWorld);
    if ( pixMap != nil)
    {
        offCLUT = (**pixMap).pmTable;
        onCLUT = (**onPixMap).pmTable;
        if ( offCLUT != nil)
        {
            MacSetRect( &tRect, 0, 0, WORLD_WIDTH / 16, WORLD_HEIGHT / 16);
            for ( j = 0; j < 16; j++)
            {
                for ( i = 0; i < 16; i++)
                {
                    MacSetRect( &uRect, tRect.left, tRect.top, tRect.left + WORLD_WIDTH / 32,
                            tRect.bottom);
//                  Index2Color( (j * 16 + i), &color);
//                  RGBForeColor( &color);
                    RGBForeColor( &((**offCLUT).ctTable[j * 16 + i].rgb));
                    PaintRect( &uRect);
                    MacSetRect( &uRect, uRect.right, uRect.top, tRect.right, tRect.bottom);

                    RGBForeColor( &((**onCLUT).ctTable[j * 16 + i].rgb));
//                  Index2Color( (j * 16 + i), &color);
//                  RGBForeColor( &color);
                    PaintRect( &uRect);
                    MacOffsetRect( &tRect, WORLD_WIDTH / 16, 0);
                }
                MacOffsetRect( &tRect, -((WORLD_WIDTH / 16) * 16), WORLD_HEIGHT / 16);
            }
        }
    }
    NormalizeColors();
}

