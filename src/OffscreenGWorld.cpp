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

#include "OffscreenGWorld.hpp"

#include <QDOffscreen.h>

#include "Debug.hpp"
#include "Error.hpp"
#include "GDeviceHandling.hpp"
#include "NateDraw.hpp"

extern GDHandle         theDevice;
GWorldPtr       gOffWorld, gRealWorld, gSaveWorld;
long            gNatePortLeft, gNatePortTop;

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

    GetGWorld(&gRealWorld, &originalDevice);

    pixBase = GetGWorldPixMap(gRealWorld);
    error = NewGWorld(&gOffWorld, 8, bounds, theClut, theDevice, 0);
    if (error) {
        ShowErrorAny(eQuitErr, kErrorStrID, nil, nil, nil, nil, OFFSCREEN_GRAPHICS_ERROR, -1, -1, -1, __FILE__, 1);
        return OFFSCREEN_GRAPHICS_ERROR;
    }

    error = NewGWorld(&gSaveWorld, 8, bounds, theClut, theDevice, 0);
    if (error) {
        ShowErrorAny(eQuitErr, kErrorStrID, nil, nil, nil, nil, OFFSCREEN_GRAPHICS_ERROR, -1, -1, -1, __FILE__, 2);

        DisposeGWorld(gOffWorld);
        return OFFSCREEN_GRAPHICS_ERROR;
    }

    pixBase = GetGWorldPixMap(gSaveWorld);
    error = LockPixels(pixBase);
    if (!error) {
        ShowErrorAny(eQuitErr, kErrorStrID, nil, nil, nil, nil, OFFSCREEN_GRAPHICS_ERROR, -1, -1, -1, __FILE__, 3);
        DisposeGWorld(gOffWorld);
        DisposeGWorld(gSaveWorld);
        return OFFSCREEN_GRAPHICS_ERROR;
    }

    pixBase = GetGWorldPixMap(gOffWorld);
    error = LockPixels(pixBase);
    if ( !error) {
        ShowErrorAny(eQuitErr, kErrorStrID, nil, nil, nil, nil, OFFSCREEN_GRAPHICS_ERROR, -1, -1, -1, __FILE__, 4);
        DisposeGWorld(gOffWorld);
        DisposeGWorld(gSaveWorld);
        return OFFSCREEN_GRAPHICS_ERROR;
    }

    tRect = bounds;
    CenterRectInDevice(theDevice, &tRect);
    gNatePortLeft = tRect.left - (*theDevice)->gdRect.left;
    gNatePortLeft /= 4;

    mWriteDebugString("\pNatePortLeft:");
    WriteDebugLong(gNatePortLeft);

    gNatePortTop = tRect.top - (*theDevice)->gdRect.top;

    EraseOffWorld();
    EraseSaveWorld();

    return kNoError;
}

void CleanUpOffscreenWorld() {
    DrawInRealWorld();
    if (gOffWorld != nil) {
        PixMap** pixBase = GetGWorldPixMap(gOffWorld);
        UnlockPixels(pixBase);
        DisposeGWorld(gOffWorld);
    }
    WriteDebugLine("\p<OffWorld");

    if (gSaveWorld != nil) {
        PixMap** pixBase = GetGWorldPixMap(gSaveWorld);
        UnlockPixels(pixBase);
        DisposeGWorld(gSaveWorld);
    }
    WriteDebugLine("\p<SaveWorld");
}

void DrawInRealWorld() {
    SetGWorld(gRealWorld, nil);
}

void DrawInOffWorld() {
    SetGWorld(gOffWorld, nil);
}

void DrawInSaveWorld() {
    SetGWorld(gSaveWorld, nil);
}

void EraseOffWorld() {
    DrawInOffWorld();
    PixMap** pixBase = GetGWorldPixMap(gOffWorld);
    EraseRect(&((*pixBase)->bounds));
    RGBColor c = { 0, 0, 0 };
    RGBForeColor(&c);
    PaintRect(&((*pixBase)->bounds));
    NormalizeColors();
    DrawInRealWorld();
    NormalizeColors();
}

void EraseSaveWorld() {
    DrawInSaveWorld();
    PixMap** pixBase = GetGWorldPixMap(gSaveWorld);
    NormalizeColors();
    EraseRect(&((*pixBase)->bounds));
    RGBColor c = { 0, 0, 0 };
    RGBForeColor(&c);
    PaintRect(&((*pixBase)->bounds));
    NormalizeColors();
    DrawInRealWorld();
}

void CopyOffWorldToRealWorld(WindowPtr port, Rect *bounds) {
    PixMap** pixBase = GetGWorldPixMap(gOffWorld);
    NormalizeColors();
    CopyBits(*pixBase, &(port->portBits), bounds, bounds, srcCopy, nil);
}

void CopyRealWorldToSaveWorld(WindowPtr port, Rect *bounds) {
    PixMap** pixBase = GetGWorldPixMap(gSaveWorld);
    NormalizeColors();
    CopyBits(&(port->portBits), *pixBase, bounds, bounds, srcCopy, nil);
}

void CopyRealWorldToOffWorld(WindowPtr port, Rect *bounds) {
    PixMap** pixBase = GetGWorldPixMap(gOffWorld);
    NormalizeColors();
    CopyBits(&(port->portBits), *pixBase, bounds, bounds, srcCopy, nil);
}

void CopySaveWorldToOffWorld(Rect *bounds) {
    PixMap** savePixBase = GetGWorldPixMap(gSaveWorld);
    PixMap** offPixBase = GetGWorldPixMap(gOffWorld);
    NormalizeColors();
    CopyBits(*savePixBase, *offPixBase, bounds, bounds, srcCopy, nil);
}

void CopyOffWorldToSaveWorld(Rect *bounds) {
    PixMap** savePixBase = GetGWorldPixMap(gSaveWorld);
    PixMap** offPixBase = GetGWorldPixMap(gOffWorld);
    NormalizeColors();
    CopyBits(*offPixBase, *savePixBase, bounds, bounds, srcCopy, nil);
}

void NormalizeColors() {
    RGBColor c = { 0, 0, 0 };
    RGBForeColor (&c);
    c.red = c.blue = c.green = 255;
    RGBBackColor(&c);
}

void ChunkCopyPixMapToScreenPixMap(PixMap *sourcePix, Rect *sourceRect, PixMap *destMap) {
    CopyBits(sourcePix, destMap, sourceRect, sourceRect, 0, NULL);
}

void ChunkCopyPixMapToPixMap(PixMap *sourcePix, Rect *sourceRect, PixMap *destMap) {
    CopyBits(sourcePix, destMap, sourceRect, sourceRect, 0, NULL);
}

void ChunkErasePixMap(PixMap *destMap, Rect *sourceRect) {
    DrawNateRect(destMap, sourceRect, 0, 0, 0xFF);
}
