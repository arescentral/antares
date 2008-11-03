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

#include "Icl8ToCicn.h"

#include "ConditionalMacros.h"
#include "Resources.h"

/* icl8 to cicn.c -- based on Apple code snippet of same name
why is this needed? For the most woeful of reasons.
It appears that PlotIconID & PlotIconSuite don't actually use the current
device as advertised but use instead the physical device somehow.

Even when a new offscreen GWorld is created, with its own device created for
it, and that device's pixel depth is confirmed as being 32-bit, in my case
Ares' custom color palette still forces an icon plotted into that GWorld to
be plotted at 4 bits.

Therefore, I've converted this code for my purposes to attempt to force
draw the 8-bit version of the icon.

I check against the resource files refNum just to make sure I'm not accidentally
delving into the System.
*/

Boolean Ploticl8ToCICN( short resID, short resRefNum, Rect *destRect,
    Handle icl8Handle, Handle ICNnHandle)
{
    Handle      icn;            /* Handle to the icon's bitmap image and mask. */
    Handle      icl8;           /* Handle to the icl8 resource. */
    char        depth;          /* Depth of the icl8 pixel image. */
    Rect        bounds;         /* Bounding rect for the icon. */
    long        bitmapSize;     /* Size of the icon's bitmap. */
    short       iconRefNum;
    CIconHandle thecicn;
    SignedByte  icnState, icl8State;

    MacSetRect( &bounds, 0, 0, 32, 32 );    /* 'icl8' are 32x32 pixels. */
    depth = 8;                          /* 8-bit deep pixel image. */
    bitmapSize = 4 * 32;                /* 4 bytesPerRow * 32 rows. */

    /* Load and lock the 'icl8' and 'ICN#' resources used to build the 'cicn'. */

    if ( ICNnHandle == nil)
    {
        icn = GetResource( 'ICN#', resID );
        if ( icn == nil) return false;
        iconRefNum = HomeResFile( icn);
        if (( ResError() != noErr) || ( iconRefNum != resRefNum) || ( iconRefNum == -1))
        {
            ReleaseResource( icn);
            return false;
        }
    } else icn = ICNnHandle;
    if ( icl8Handle == nil)
    {
        icl8 = GetResource( 'icl8', resID );
        if ( icl8 == nil) return false;
        iconRefNum = HomeResFile( icl8);
        if (( ResError() != noErr) || ( iconRefNum != resRefNum) || ( iconRefNum == -1))
        {
            ReleaseResource( icn);
            ReleaseResource( icl8);
            return false;
        }
    } else icl8 = icl8Handle;

    icnState = HGetState( icn);
    HLock( icn );
    HNoPurge( icn );

    icl8State = HGetState( icl8);
    HLock( icl8 );
    HNoPurge( icl8 );

    /* Allocate memory for the 'cicn'. */

    thecicn = (CIconHandle)NewHandleClear( (long)sizeof( CIcon ) );

    /* Fill in the cicn's bitmap fields. */

    (**thecicn).iconBMap.baseAddr           = nil;
    (**thecicn).iconBMap.rowBytes           = 4;
    (**thecicn).iconBMap.bounds             = bounds;

    /* Fill in the cicn's mask bitmap fields. */

    (**thecicn).iconMask.baseAddr           = nil;
    (**thecicn).iconMask.rowBytes           = 4;
    (**thecicn).iconMask.bounds             = bounds;

    /* Fill in the cicn's pixmap fields. */

    (**thecicn).iconPMap.baseAddr           = nil;
    (**thecicn).iconPMap.rowBytes           = (((bounds.right - bounds.left) *
                                                depth) / 8) | 0x8000;
    (**thecicn).iconPMap.bounds             = bounds;
    (**thecicn).iconPMap.pmVersion          = 0;
    (**thecicn).iconPMap.packType           = 0;
    (**thecicn).iconPMap.packSize           = 0;
    (**thecicn).iconPMap.hRes               = 72;
    (**thecicn).iconPMap.vRes               = 72;
    (**thecicn).iconPMap.pixelSize          = depth;
#if TARGET_OS_MAC
    (**thecicn).iconPMap.planeBytes         = 0;
    (**thecicn).iconPMap.pmReserved         = 0;
#endif TARGET_OS_MAC
    (**thecicn).iconPMap.pixelType          = 0;
    (**thecicn).iconPMap.cmpCount           = 1;
    (**thecicn).iconPMap.cmpSize            = depth;
    (**thecicn).iconPMap.pmTable            = GetCTable( depth );

    /* Set the 'icl8' pixel image to the iconData field. */

    (**thecicn).iconData = (Handle)icl8;

    /* Resize the 'cicn' for the bitmap image and mask. */

    SetHandleSize( (Handle)thecicn, sizeof( CIcon ) + (bitmapSize * 2) );

    /* Copy the 'ICN#' data into the iconMaskData array. */
    /* Note1: This is an array of shorts, so divide bitmapSize by 2. */
    /* Note2: The mask comes before the image.  The is opposite of an 'ICN#' */

    BlockMove( *icn, &(**thecicn).iconMaskData[bitmapSize / 2], bitmapSize );       /* The 1bit image. */
    BlockMove( *icn + (long)bitmapSize, (**thecicn).iconMaskData, bitmapSize ); /* The mask. */

    PlotCIcon( destRect, thecicn );

    DisposeCTable( (**thecicn).iconPMap.pmTable);
    if ( icl8Handle == nil)
        ReleaseResource( icl8);
    else
    {
        HSetState( icl8, icl8State);
    }

    if ( ICNnHandle == nil)
        ReleaseResource( icn);
    else
    {
        HSetState( icn, icnState);
    }
    DisposeHandle( (Handle)thecicn);

    return( true);
}
