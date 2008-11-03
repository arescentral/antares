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

// Ares Title Screen
#ifndef __CONDITIONALMACROS__
#include "ConditionalMacros.h"
#endif // __CONDITIONALMACROS__

#if TARGET_OS_WIN32

    #ifndef __QUICKTIMEVR__
    #include <QuickTimeVR.h>
    #endif

    #ifndef __QTUtilities__
    #include "QTUtilities.h"
    #endif

    #ifndef __QTVRUtilities__
    #include "QTVRUtilities.h"
    #endif

    #include <TextUtils.h>
    #include <Script.h>
    #include <string.h>
#endif // TARGET_OS_WIN32

#include "Resources.h"

#include "Offscreen GWorld.h"

#include "Title Screen.h"
#include "Handle Handling.h"

#include "Error.h"
#include "Debug.h"

#define kTitleScreenID      502
#define kPublisherScreenID  2000
#define kEgoScreenID        2001

#define kTitleScreenError   "\pTITL"

extern CWindowPtr       gTheWindow;

void DrawTitleScreen( void)

{
    PicHandle       pict;
    Rect            tRect;

    MacSetPort( (WindowPtr)gTheWindow);
//  pict = GetPicture( kTitleScreenID);
    pict = (PicHandle)HHGetResource( 'PICT', kTitleScreenID);
    if ( pict == nil)
    {
        ShowErrorAny( eContinueOnlyErr, kErrorStrID, nil, nil, nil, nil, kLoadPictError, -1, -1, -1, __FILE__, 1);
    } else
    {
        DrawInRealWorld();
        tRect = (**pict).picFrame;
        CenterRectInRect( &tRect, &(gTheWindow->portRect));
        DrawPicture( pict, &tRect);
        ReleaseResource( (Handle)pict);
    }
}

void DrawPublisherScreen( void)

{
    PicHandle       pict;
    Rect            tRect;

    MacSetPort( (WindowPtr)gTheWindow);
    pict = (PicHandle)HHGetResource( 'PICT', kPublisherScreenID);
    if ( pict == nil)
    {
        ShowErrorAny( eContinueOnlyErr, kErrorStrID, nil, nil, nil, nil, kLoadPictError, -1, -1, -1, __FILE__, 2);
    } else
    {
        DrawInRealWorld();
        tRect = (**pict).picFrame;
        CenterRectInRect( &tRect, &(gTheWindow->portRect));
        PaintRect(  &(gTheWindow->portRect));
        DrawPicture( pict, &tRect);
        ReleaseResource( (Handle)pict);
    }
}

void DrawEgoScreen( void)

{
    PicHandle       pict;
    Rect            tRect;

    MacSetPort( (WindowPtr)gTheWindow);
    pict = (PicHandle)HHGetResource( 'PICT', kEgoScreenID);
    if ( pict == nil)
    {
        ShowErrorAny( eContinueOnlyErr, kErrorStrID, nil, nil, nil, nil, kLoadPictError, -1, -1, -1, __FILE__, 3);
    } else
    {
        DrawInRealWorld();
        tRect = (**pict).picFrame;
        CenterRectInRect( &tRect, &(gTheWindow->portRect));
        PaintRect(  &(gTheWindow->portRect));
        DrawPicture( pict, &tRect);
        ReleaseResource( (Handle)pict);
    }
}

void BlackTitleScreen( void)

{
    RGBColor    black = {0, 0, 0};

    MacSetPort( (WindowPtr)gTheWindow);
    DrawInRealWorld();
    MacSetPort( (WindowPtr)gTheWindow);
    PenPat( &qd.black);
    RGBForeColor( &black);

    PaintRect(  &(gTheWindow->portRect));
}

void CenterRectInRect( Rect *dRect, Rect *sRect)

{
    short   w, h;

    w = dRect->right - dRect->left;
    h = dRect->bottom - dRect->top;
    dRect->left =  sRect->left + ( sRect->right - sRect->left) / 2 - w / 2;
    dRect->top = sRect->top + ( sRect->bottom - sRect->top) / 2 - h / 2;
    dRect->right = dRect->left + w;
    dRect->bottom = dRect->top + h;
}
