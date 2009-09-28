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

#include "GDeviceHandling.hpp"

#include "Quickdraw.h"

PixMap** thePixMapHandle;
GDevice** theDevice;

void CenterRectInDevice(GDevice** device, Rect* rect) {
    Rect dRect =(*device)->gdRect;
    int w = rect->right - rect->left;
    int h = rect->bottom - rect->top;
    rect->left =  dRect.left + ( dRect.right - dRect.left) / 2 - w / 2;
    rect->top = dRect.top + ( dRect.bottom - dRect.top) / 2 - h / 2;
    rect->right = rect->left + w;
    rect->bottom = rect->top + h;
}
