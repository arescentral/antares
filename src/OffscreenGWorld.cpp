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

#include "Debug.hpp"
#include "Error.hpp"
#include "GDeviceHandling.hpp"
#include "NateDraw.hpp"

extern GDHandle         theDevice;
PixMap*         gActiveWorld;
PixMap*         gOffWorld;
PixMap*         gRealWorld;
PixMap*         gSaveWorld;
long            gNatePortLeft, gNatePortTop;

int CreateOffscreenWorld(const Rect& bounds, const ColorTable&) {
    Rect            tRect;

    //
    //  NewGWorld creates the world.  See p.21-12--13 in IM VI.  Note that
    //  by setting pix depth to 0, we are using bounds as global rect which
    //  determines the depth of the GWorld by using the deepest depth of any
    //  device that rect intersects.  We should use the bounding rect of our
    //  device of choice, but I'm too lazy.
    //

    theDevice = GetGDevice();
    gRealWorld = (*theDevice)->gdPMap;
    gOffWorld = new PixMap(bounds.right, bounds.bottom);
    gSaveWorld = new PixMap(bounds.right, bounds.bottom);
    gActiveWorld = gRealWorld;

    tRect = bounds;
    CenterRectInDevice(theDevice, &tRect);
    gNatePortLeft = tRect.left - (*theDevice)->gdRect.left;
    gNatePortLeft /= 4;

    gNatePortTop = tRect.top - (*theDevice)->gdRect.top;

    EraseOffWorld();
    EraseSaveWorld();

    return kNoError;
}

void CleanUpOffscreenWorld() {
    DrawInRealWorld();
    if (gOffWorld != nil) {
        delete gOffWorld;
    }
    if (gSaveWorld != nil) {
        delete gSaveWorld;
    }
}

void DrawInRealWorld() {
    gActiveWorld = gRealWorld;
}

void DrawInOffWorld() {
    gActiveWorld = gOffWorld;
}

void DrawInSaveWorld() {
    gActiveWorld = gSaveWorld;
}

void EraseOffWorld() {
    DrawInOffWorld();
    EraseRect(&gOffWorld->bounds);
    RGBColor c = { 0, 0, 0 };
    RGBForeColor(&c);
    PaintRect(&gOffWorld->bounds);
    NormalizeColors();
    DrawInRealWorld();
    NormalizeColors();
}

void EraseSaveWorld() {
    DrawInSaveWorld();
    NormalizeColors();
    EraseRect(&gSaveWorld->bounds);
    RGBColor c = { 0, 0, 0 };
    RGBForeColor(&c);
    PaintRect(&gSaveWorld->bounds);
    NormalizeColors();
    DrawInRealWorld();
}

void CopyOffWorldToRealWorld(Rect *bounds) {
    NormalizeColors();
    CopyBits(gOffWorld, gRealWorld, bounds, bounds, srcCopy, nil);
}

void CopyRealWorldToSaveWorld(Rect *bounds) {
    NormalizeColors();
    CopyBits(gRealWorld, gSaveWorld, bounds, bounds, srcCopy, nil);
}

void CopyRealWorldToOffWorld(Rect *bounds) {
    NormalizeColors();
    CopyBits(gRealWorld, gOffWorld, bounds, bounds, srcCopy, nil);
}

void CopySaveWorldToOffWorld(Rect *bounds) {
    NormalizeColors();
    CopyBits(gSaveWorld, gOffWorld, bounds, bounds, srcCopy, nil);
}

void CopyOffWorldToSaveWorld(Rect *bounds) {
    NormalizeColors();
    CopyBits(gOffWorld, gSaveWorld, bounds, bounds, srcCopy, nil);
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
