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

// Graphics Device Interface

#include "GDeviceHandling.hpp"

#include "ConditionalMacros.h"
#include "Debug.hpp"
#include "Error.hpp"
#include "PlayerInterface.hpp"
#include "Sound.h"

#define kMaxDevice  4

#define kGDeviceError       "\pGDEV"

struct whichDeviceType {
    GDHandle    whichDevice;
    DialogPtr   window;
};

extern long             WORLD_WIDTH, WORLD_HEIGHT;

GDHandle                theDevice = nil;
int                     gOriginalDeviceDepth = 0;
PixMapHandle            thePixMapHandle = nil;

Boolean ChooseTheDevice ( int depth, Boolean setDepth)

{
    gOriginalDeviceDepth = depth;
    theDevice = GetBestDevice ( &gOriginalDeviceDepth, WORLD_WIDTH, WORLD_HEIGHT);
    if ( theDevice != nil)
    {
        if ( setDepth) SetColorDepth( theDevice, depth);
        thePixMapHandle = (*theDevice)->gdPMap;

    }
    return ( theDevice != nil);
}

Boolean UserChooseTheDevice( int depth, Boolean setDepth, Rect *bounds)

{
    whichDeviceType deviceWindow[kMaxDevice];
    short           i, deviceNum = 0, chosenWindow = -1, whichPart;
    GDHandle        currentDevice;
    Rect            deviceRect;
    EventRecord     theEvent;
    WindowPtr       whichWindow;

    for ( i = 0; i < kMaxDevice; i++)
    {
        deviceWindow[i].whichDevice = nil;
        deviceWindow[i].window = nil;
    }
    currentDevice = GetDeviceList();
    deviceNum = 0;

    // how many usable devices do we have?
    while (( currentDevice != nil) && ( deviceNum < kMaxDevice))
    {
        if ( HasDepth( currentDevice, depth, 1, 1))
        {
            GetDeviceRect( currentDevice, &deviceRect);
            if (( (deviceRect.right - deviceRect.left) >= (bounds->right - bounds->left)) &&
                ( (deviceRect.bottom - deviceRect.top) >= (bounds->bottom - bounds->top)))
            {
                deviceWindow[deviceNum].whichDevice = currentDevice;
                deviceNum++;
            }
        }
        currentDevice = GetNextDevice( currentDevice);
    }

    // if there's no devices, we cannot go on
    if ( deviceNum < 1)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, PIX_DEPTH_ERROR, -1, -1, -1, __FILE__, 100);
        return( false);
    } // if there's one device, then use it
    else if ( deviceNum == 1)
    {
        SetTheDevice( deviceWindow[0].whichDevice, depth, setDepth);
        GetDeviceRect( deviceWindow[0].whichDevice, bounds);
        return( true);
    }
    // otherwise, go on

    for ( i = 0; i < deviceNum; i++)
    {
        MacSetRect( &deviceRect, 0, 0, 200, 200);
        CenterRectInDevice( deviceWindow[i].whichDevice, &deviceRect);
        deviceWindow[i].window = GetNewDialog( 703, nil, reinterpret_cast<WindowPtr>(-1));
        MacMoveWindow(reinterpret_cast<WindowPtr>(deviceWindow[i].window), deviceRect.left, deviceRect.top, true);
        MacShowWindow(reinterpret_cast<WindowPtr>(deviceWindow[i].window));
    }

    while ( chosenWindow == -1)
    {
        Ares_WaitNextEvent (everyEvent, &theEvent, 3, nil);

        switch ( theEvent.what )
        {
            case nullEvent:
                break;

            case mouseDown:
                whichPart = MacFindWindow (theEvent.where,  &whichWindow);
                switch (whichPart)
                {
                    case inMenuBar:
                        SysBeep( 20);
                        break;

                    case inSysWindow:
                        SysBeep(20);
                        break;

                    case inContent:
                        if ( whichWindow != nil)
                        {
                            for ( i = 0; i < kMaxDevice; i++)
                            {
                                if ( whichWindow == reinterpret_cast<WindowPtr>(deviceWindow[i].window))
                                    chosenWindow = i;
                            }
                        }
                        if ( chosenWindow < 0) SysBeep(20);
                        break;

                    case inDrag:
                        SysBeep(20);
                        break;

                    case inGoAway:
                        SysBeep(20);
                        break;
                }
                break;

            case mouseUp:
                break;

            case keyDown:
            case autoKey:
                break;
        }
    }

    for ( i = 0; i < kMaxDevice; i++)
    {
        if ( deviceWindow[i].window != nil)
        {
            DisposeWindow(reinterpret_cast<WindowPtr>(deviceWindow[i].window));
        }
    }

    if ( chosenWindow >= 0)
    {
        SetTheDevice( deviceWindow[chosenWindow].whichDevice, depth, setDepth);
        GetDeviceRect( deviceWindow[chosenWindow].whichDevice, bounds);
        return( true);
    } else return( false);
}

void CleanUpTheDevice( Boolean setDepth)

{
    if ( setDepth) RestoreDeviceColorDepth();
}

void SetTheDevice( GDHandle thisdevice, int depth, Boolean setDepth)
{
    theDevice = thisdevice;
    gOriginalDeviceDepth = GetDeviceDepth( thisdevice);
    if ( theDevice != nil)
    {
        if ( setDepth) SetColorDepth( theDevice, depth);
        thePixMapHandle = (*theDevice)->gdPMap;
    }
}

void RestoreDeviceColorDepth ( void)

{
    if ( GetDeviceDepth( theDevice) != gOriginalDeviceDepth)
        SetColorDepth( theDevice, gOriginalDeviceDepth);
}

void CenterRectInDevice( GDHandle device, Rect *rect)

{
    Rect        dRect;
    int         w, h;

//  dRect = (*(*device)->gdPMap)->bounds;
    dRect =(*device)->gdRect;
//  if ( device == GetMainDevice())
//      dRect.top -= GetMBarHeight(); // menu bar height?
    w = rect->right - rect->left;
    h = rect->bottom - rect->top;
    rect->left =  dRect.left + ( dRect.right - dRect.left) / 2 - w / 2;
    rect->top = dRect.top + ( dRect.bottom - dRect.top) / 2 - h / 2;
    rect->right = rect->left + w;
    rect->bottom = rect->top + h;
}

void GetDeviceRect( GDHandle device, Rect *deviceRect)

{
    if ( device == nil) device = GetMainDevice();
//  *deviceRect = (*(*device)->gdPMap)->bounds;
    *deviceRect =(*device)->gdRect;
}

void ShieldCursorInDevice( void)

{
    Rect    tRect;
    Point   p;

    GetDeviceRect( theDevice, &tRect);
    p.h = p.v = 0;
    ShieldCursor( &tRect, p);
}

void SetColorDepth( GDHandle device, int depth)

{
    if ( !HasDepth( device, depth, 1, 1))
    {
        ShowErrorAny( eContinueErr, kErrorStrID, nil, nil, nil, nil, PIX_DEPTH_ERROR, -1, -1, -1, __FILE__, 1);
    } else
        SetDepth( device, depth, 1, 1);
}

//
// GetDeviceDepth- see MacTech December '93, p19
//

int GetDeviceDepth( GDHandle device)

{
    PixMapHandle    pmap;

    pmap = (**device).gdPMap;
    return( (**pmap).pixelSize);

}

GDHandle GetBestDevice( int *trueDepth, int width, int height)

{
    GDHandle    resultDevice = nil, biggestDevice = nil;
    long        area, biggestArea = 0;
    Rect        deviceRect;
    Boolean     bestAtRightDepth = FALSE;
    int         depth = *trueDepth;

    resultDevice = GetMainDevice();
    GetDeviceRect( resultDevice, &deviceRect);
    // if the main device is not at correct depth, or is not of the minimum size
    if (( GetDeviceDepth( resultDevice) != depth) || ((deviceRect.right - deviceRect.left)
            < width) || ((deviceRect.bottom - deviceRect.top) < height))
    {
        resultDevice = GetDeviceList();
        while ( resultDevice != nil)
        {
            if ( HasDepth( resultDevice, depth, 1, 1))
            {
                GetDeviceRect( resultDevice, &deviceRect);
                if (( (deviceRect.right - deviceRect.left) >= width) &&
                    ( (deviceRect.bottom - deviceRect.top) >= height))
                {
                    area = implicit_cast<long>(deviceRect.right - deviceRect.left) *
                        implicit_cast<long>(deviceRect.bottom - deviceRect.top);
                    if (( area > biggestArea) && (( GetDeviceDepth( resultDevice) ==
                        depth) || ( !bestAtRightDepth)))
                    {
                        biggestArea = area;
                        biggestDevice = resultDevice;
                        if ( GetDeviceDepth( resultDevice) == depth)
                            bestAtRightDepth = TRUE;
                    }
                }
            }
            resultDevice = GetNextDevice( resultDevice);
        }
        if ( biggestDevice != nil)
        {
            *trueDepth = GetDeviceDepth( biggestDevice);
            return (biggestDevice);
        }
        else
        {
            return ( nil);
        }
    } else
    {
        *trueDepth = GetDeviceDepth( resultDevice);
        return ( resultDevice);
    }
}

