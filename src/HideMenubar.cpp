/*
Ares, a tactical space combat game.
Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

// Hide Menubar.c

#include "ConditionalMacros.h"

// also hides desktop corners

#include "HideMenubar.h"

// ShowHideMBar.c -- from Hide Menubar Etc. from Feb 96 Developer Toolchest Disc
//
// David Hayward
// Developer Technical Support
// AppleLink: DEVSUPPORT
//
// Copyrite 1993, Apple Computer,Inc
//
// This file contains routines to sho/hide the
// rounded corners in each monitor.
//
// 12/10/93 david   first cut


#include <QuickDraw.h>
#include <LowMem.h>

/**\
|**| ==============================================================================
|**| GLOBALS
|**| ==============================================================================
\**/
short           gOldBarHgt = 20;
RgnHandle       gMenuBarRegion = nil, gCornerRegion = nil;
Boolean         gMenubarIsVisible = true;

/**\
|**| ==============================================================================
|**| PRIVATE FUNCTION PROTOTYPES
|**| ==============================================================================
\**/
void GetMBarRgn ( RgnHandle);

void SH_ForceUpdate (RgnHandle);

void GetCornerRgn (RgnHandle, GDHandle);

/**\
|**| ==============================================================================
|**| PUBLIC FUNCTIONS
|**| ==============================================================================
\**/

#define kCanHideMenuBar

// InitHideMenubar -- sets up old menubar height
void InitHideMenubar( void)
{
    gOldBarHgt = GetMBarHeight();
}

/*------------------------------------------------------------------------------*\
    SetMBarState
 *------------------------------------------------------------------------------*
        changes the menubar state to either SHOW or HIDE
\*------------------------------------------------------------------------------*/
// NL -- it's now up to the caller to make sure same state is not called twice!
void SetMBarState (Boolean vis, GDHandle theDevice)
{
#ifdef kCanHideMenuBar
    RgnHandle       GrayRgn = LMGetGrayRgn();
    GrafPtr         savePort;

    if ((!vis) && ( gMenubarIsVisible))                     /* if HIDE */
    {
        if ( theDevice == GetMainDevice())
        {
            LMSetMBarHeight(0);                 /* make the Menu Bar's height zero */

            gMenuBarRegion = NewRgn();
            GetMBarRgn(gMenuBarRegion);             /* make a region for the mbar */
            MacUnionRgn(GrayRgn,gMenuBarRegion,GrayRgn);    /* tell the desktop it covers the menu bar */
            SH_ForceUpdate(gMenuBarRegion);
        }
        gCornerRegion = NewRgn();
        GetCornerRgn(gCornerRegion, theDevice);                 /* make a region for the corners */
        MacUnionRgn(GrayRgn,gCornerRegion,GrayRgn); /* tell the desktop it covers the corners */

        SH_ForceUpdate(gCornerRegion);
        gMenubarIsVisible = false;

    } else if ((vis) && (!gMenubarIsVisible))                               /* if SHOW */
    {
        // make sure someone else didn't show it already and screw us up
/*      if ( GetMBarHeight() > 0)
        {
            gOldBarHgt = GetMBarHeight();
            if ( gMenuBarRegion != nil)
                DisposeRgn(gMenuBarRegion);             // dispose to the bar region
            gMenuBarRegion = nil;
            if ( gCornerRegion != nil)
                DisposeRgn(gCornerRegion);                      // dispose to the corners region
            gCornerRegion = nil;
            gMenubarIsVisible = true;

        } else
*/      {
            if ( theDevice == GetMainDevice())
            {
                LMSetMBarHeight(gOldBarHgt);        /* make the menu bar's height normal */

                DiffRgn(GrayRgn, gMenuBarRegion, GrayRgn);  /* remove the menu bar from the desktop  */
                SH_ForceUpdate(gMenuBarRegion);
                DisposeRgn(gMenuBarRegion);             /* dispose to the bar region */
                gMenuBarRegion = nil;
                MacDrawMenuBar();                       /* redraw the menu bar */
            }

            GetPort(&savePort);

            MacSetPort(LMGetWMgrPort());
            SetClip(gCornerRegion);
            MacFillRgn(gCornerRegion,&(qd.black));          /* redraw the corners */

            MacSetPort(savePort);

            DiffRgn(GrayRgn, gCornerRegion, GrayRgn);   /* remove the corners from the desktop rgn */
            DisposeRgn(gCornerRegion);                      /* dispose to the corners region */
            gCornerRegion = nil;
            gMenubarIsVisible = true;
        }

    }
#endif
}


/**\
|**| ==============================================================================
|**| PRIVATE FUNCTIONS
|**| ==============================================================================
\**/


/*------------------------------------------------------------------------------*\
    GetMBarRgn
 *------------------------------------------------------------------------------*
        uses globals to calculate the region for the MenuBar
        the RgnHandle mBarRgn must be allocated with NewRgn() before calling
\*------------------------------------------------------------------------------*/
void GetMBarRgn (RgnHandle mBarRgn)
{
    Rect            mBarRect;

    mBarRect = qd.screenBits.bounds;            /* create a rect for the mbar */
    mBarRect.bottom = mBarRect.top + gOldBarHgt;
    RectRgn(mBarRgn, &mBarRect);                /* make a region for the mbar */
}

void SH_ForceUpdate (RgnHandle rgn)
{
    WindowRef   wpFirst = LMGetWindowList();

    PaintBehind(wpFirst, rgn);                      /* redraw windows behind front */
    CalcVisBehind(wpFirst, rgn);                    /* redraw windows behind front */
}


/*------------------------------------------------------------------------------*\
    GetCornerRgn
 *------------------------------------------------------------------------------*
        uses globals to calculate the region for the rounred corners
        the RgnHandle crnrRgn must be allocated with NewRgn() before calling
\*------------------------------------------------------------------------------*/
void GetCornerRgn (RgnHandle crnrRgn, GDHandle theDevice)
{
    Rect            mBarRect;
    RgnHandle       tmpRgn = nil;
    Rect            theDeviceRect;
    RgnHandle       GrayRgn = LMGetGrayRgn();

    tmpRgn = NewRgn();
    if ( tmpRgn == nil) return;

    /* Loop through all the devices in the list in order */
    /* to create a region for all the screens' boundaries*/
    if (theDevice != nil)
    {
        theDeviceRect = (**theDevice).gdRect;           /* the bounding rect of this device */
        RectRgn(tmpRgn, &theDeviceRect);                /* convert rect to a region */
        MacUnionRgn(crnrRgn,tmpRgn,crnrRgn);            /* add device's rect to region */
    }

    /* subtract the GrayRgn from the above. This leaves a region */
    /* which contains the menuBar and any rounded corners. */
    DiffRgn(crnrRgn,GrayRgn,crnrRgn);               /* remove GrayRgn from crnrRgn */

    DisposeRgn(tmpRgn);
}

/* AutoShowHide (by NL)
Given a mouse location in global coords, automatically shows/hides the menu bar if
location is in menu bar region. Returns true is mouse is in menubar region
*/

Boolean AutoShowHideMenubar( Point where, GDHandle theDevice)
{
    RgnHandle   tempRgn = nil;
    Boolean     result = false;

    if ( theDevice == GetMainDevice())
    {
        if ( gMenuBarRegion != nil) // if the gMenuBarRegion exists, menubar is hidden
        {
            if ( PtInRgn( where, gMenuBarRegion))
            {
                SetMBarState( true, theDevice);
                result = true;
            }
        } else
        {
            tempRgn = NewRgn();
            if ( tempRgn != nil)
            {
                GetMBarRgn( tempRgn);
                if ( !PtInRgn( where, tempRgn))
                {
                    SetMBarState( false, theDevice);
                } else result = true;
                DisposeRgn( tempRgn);
            }
        }
    }
    return( result);
}
