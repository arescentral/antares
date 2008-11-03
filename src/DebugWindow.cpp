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

/* DebugWindow */

#include "Debug.h"

#include "ConditionalMacros.h"
#include "Error.h"
#include "SetFontByString.h"
#include "StringHandling.h"
#include "StringNumerics.h"

#define LINE_HEIGHT         11
#define WINDOW_HEIGHT       50      /* Window height in lines */
#define WINDOW_WIDTH        88

#define kMaxDebugFileSize   4098000//1024000
#define kMinDebugFileSize   3072000//512000

#define monaco              2

//#define   kDebugWindowActive
//#define   kDebugFileActive
#define kDebugError         "\pDEBG"

#define mFixDebugFileSize( moldsize, mplussize, muselen)\
moldsize = GetHandleSize( gDebugData);\
if ( (moldsize + mplussize) > kMaxDebugFileSize)\
{\
    muselen = kMinDebugFileSize - mplussize;\
    BlockMove( *gDebugData + ((moldsize + mplussize) - kMinDebugFileSize), *gDebugData,\
        muselen);\
    SetHandleSize( gDebugData, kMinDebugFileSize);\
} else\
{\
    muselen = moldsize;\
    SetHandleSize( gDebugData, fileLen + (long)*sc);\
}

extern GDHandle     theDevice;

WindowPtr   gDebugWindow = nil;
#ifdef kDebugFileActive
Handle      gDebugData = nil;
#endif

void GetDeviceRect( GDHandle, Rect *);


void DebugWindowInit( WindowPtr behindWindow)

{
    Rect    tRect, dRect;

    GetDeviceRect( theDevice, &dRect);
    MacSetRect( &tRect, dRect.right - WINDOW_WIDTH - 4, dRect.bottom - WINDOW_HEIGHT *
            LINE_HEIGHT - 4, dRect.right - 4, dRect.bottom - 4);
    gDebugWindow = NewWindow( nil, &tRect, "\pDebug", FALSE, noGrowDocProc, behindWindow,
            TRUE, 800);

#ifdef kDebugWindowActive
    ShowHide (gDebugWindow, TRUE);
#else
    ShowHide (gDebugWindow, FALSE);
#endif
}

void BringDebugToFront( void)
{
#ifdef kDebugWindowActive
    SelectWindow( gDebugWindow);
#endif
}

void DebugWindowCleanup( void)

{
    DisposeWindow( gDebugWindow);
}

void ScrollDebugWindowUp( void)

{
#ifdef kDebugWindowActive
    GrafPtr     oldPort;
    RgnHandle   tRgn;
    Rect        tRect;

    SetRect( &tRect, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT * LINE_HEIGHT);
    GetPort( &oldPort);
    SetPort( gDebugWindow);
    tRgn = NewRgn();
    OpenRgn();
    FrameRect( &tRect);
    CloseRgn( tRgn);
    ScrollRect( &tRect, 0, -LINE_HEIGHT, tRgn);
    DisposeRgn( tRgn);
    SetPort( oldPort);
#endif
}

void WriteDebugLine( char *text)

{
#ifdef kDebugWindowActive
    GrafPtr     oldPort;

    GetPort( &oldPort);
    SetPort( gDebugWindow);
    SetFontByString( "\pgeneva");
    TextSize( 9);
    ScrollDebugWindowUp();
    MoveTo( 0, WINDOW_HEIGHT * LINE_HEIGHT - 4);
    DrawString( (StringPtr)text);
    SetPort( oldPort);
#endif
}

void WriteDebugChar( char c)

{
#ifdef kDebugWindowActive
    GrafPtr     oldPort;

    GetPort( &oldPort);
    SetPort( gDebugWindow);
    TextFont( monaco);
    TextSize( 9);
    ScrollDebugWindowUp();
    MoveTo( 0, WINDOW_HEIGHT * LINE_HEIGHT - 4);
    DrawChar( c);
    SetPort( oldPort);
#endif
}

void WriteDebugHex( unsigned long val, unsigned long places)

{
#ifdef kDebugWindowActive
    GrafPtr         oldPort;
    unsigned long   count, digit;

    GetPort( &oldPort);
    SetPort( gDebugWindow);
    TextFont( monaco);
    TextSize( 9);
    ScrollDebugWindowUp();
    MoveTo( 0, WINDOW_HEIGHT * LINE_HEIGHT - 4);
    places *= 2;
    for ( count = places; count > 0; count--)
    {
        digit = val >> (( count - 1) << 2);
        digit &= 0x0000000f;
        if ( digit < 10)
            DrawChar('0' + digit);
        else if ( digit < 16)
            DrawChar('A' + digit - 10);
        else DrawChar('*');
    }
    SetPort( oldPort);
#endif
}


void WriteDebugFixed( Fixed f)
{
#ifdef kDebugWindowActive
    Str255  s;
    GrafPtr         oldPort;

    GetPort( &oldPort);
    SetPort( gDebugWindow);
    TextFont( monaco);
    TextSize( 9);
    ScrollDebugWindowUp();
    MoveTo( 0, WINDOW_HEIGHT * LINE_HEIGHT - 4);
    if ( f < 0)
    {
        NumToString( (f >> 16L) + 1, s);
        if ( ((f >> 16L) + 1) == 0)
            DrawChar('-');
    }
    else
        NumToString( f >> 16L, s);
    DrawString( s);
    DrawChar('.');
    if ( f < 0)
        NumToString( (( 0x0000ffff - (f & 0x0000ffff)) * 100) / 6554L, s);
    else NumToString( ((f & 0x0000ffff) * 100) / 6554L, s);
    DrawString( s);
    SetPort( oldPort);
#endif
}


void WriteDebugSmallFixed( smallFixedType f)
{
#ifdef kDebugWindowActive
    Str255  s;
    GrafPtr         oldPort;

    GetPort( &oldPort);
    SetPort( gDebugWindow);
    TextFont( monaco);
    TextSize( 9);
    ScrollDebugWindowUp();
    MoveTo( 0, WINDOW_HEIGHT * LINE_HEIGHT - 4);
    SmallFixedToString( f, s);
    DrawString( s);
    SetPort( oldPort);
#endif
}


void WriteDebugInt( int i)

{
#ifdef kDebugWindowActive
    Str255  s;
    RGBColor    c;

    c.red = c.blue = c.green = 0;
    RGBForeColor ( &c);
    c.red = c.blue = c.green = 65535;
    RGBBackColor( &c);
    NumToString( (long)i, s);
    WriteDebugLine( (char *)s);
#endif
}


void WriteDebug2Int( int i, int j)

{
#ifdef kDebugWindowActive
    Str255  s;
    GrafPtr     oldPort;

    GetPort( &oldPort);
    SetPort( gDebugWindow);
    TextFont( monaco);
    TextSize( 9);
    ScrollDebugWindowUp();
    MoveTo( 0, WINDOW_HEIGHT * LINE_HEIGHT - 4);
    NumToString( (long)i, s);
    DrawString( s);
    DrawChar(':');
    NumToString( (long)j, s);
    DrawString( s);
    SetPort( oldPort);
#endif
}


void WriteDebugLong( long l)
{
#ifdef kDebugWindowActive
    Str255  s;

    NumToString( l, s);
    WriteDebugLine( (char *)s);
#endif
}


void MoveDebugToFront( void)

{
#ifdef kDebugWindowActive
    GrafPtr     oldPort;

    GetPort( &oldPort);
    SelectWindow( gDebugWindow);
    SetPort( oldPort);
#endif
}


void WriteDebugDivider( void)

{
#ifdef kDebugWindowActive
    GrafPtr     oldPort;

    GetPort( &oldPort);
    SetPort( gDebugWindow);
    TextFont( monaco);
    TextSize( 9);
    ScrollDebugWindowUp();
    MoveTo( 0, WINDOW_HEIGHT * LINE_HEIGHT - 4);
    LineTo( WINDOW_WIDTH, WINDOW_HEIGHT * LINE_HEIGHT - 4);
    SetPort( oldPort);
#endif
}


void WriteDebugHexDump( Ptr data, long len)

{
#ifdef kDebugWindowActive
    unsigned long   *dp;

    len >>= 2;

    dp = (unsigned long *)data;

    while ( len > 0)
    {
        WriteDebugHex( *dp++, 4);
        len--;
    }
#endif
}


unsigned long powerto ( unsigned long x, unsigned long exponent)

{
    unsigned long   count;

    if ( exponent == 0) return ( 1);
    for ( count = 1; count < exponent; count ++)
        x *= x;
    return ( x);
}


#if TARGET_OS_MAC
Boolean CommandPeriod( void)

{
    KeyMap  keyMap;

    GetKeys( keyMap);
    if ( ((keyMap[1] >> 23) & 0x01) && ((keyMap[1] >> 15) & 0x01)) return ( TRUE);
    else return ( FALSE);
}
#endif TARGET_OS_MAC

void DebugFileInit( void)

{
#ifdef kDebugFileActive
    char        *c;

    gDebugData = NewHandle( 1);
    if ( gDebugData != nil)
    {
        HLock( gDebugData);
        c = (char *)*gDebugData;
        c = 0;
        HUnlock( gDebugData);
    } else ShowSimpleStringAlert( "\pCouldn't initialize debug file.", nil, nil, nil);
#endif
}

void DebugFileCleanup( void)
{
#ifdef kDebugFileActive
    if ( gDebugData != nil)
    {
        DisposeHandle( gDebugData);
        gDebugData = nil;
    }
#endif
}

void DebugFileAppendString( StringPtr s)
{
#ifdef kDebugFileActive
    char    *c, *sc, lenCount;
    long    fileLen, scrap;

#pragma unused ( s)

    if ( gDebugData != nil)
    {
        sc = (char *)s;
#ifdef kMaxDebugFileSize
        mFixDebugFileSize( scrap, ((long)*sc), fileLen)
#else
        fileLen = GetHandleSize( gDebugData);
        SetHandleSize( gDebugData, fileLen + (long)*sc);
#endif
        if ( MemError() == noErr)
        {
//          HLock( gDebugData);
            c = ( char *)*gDebugData + (fileLen - 1);
            lenCount = *sc;
            sc++;
            while( lenCount > 0)
            {
                *c = *sc;
                c++;
                sc++;
                lenCount--;
            }
//          HUnlock( gDebugData);
        }
    }
#else
#pragma unused ( s)
#endif
}

void DebugFileAppendCString( char *s)
{
#ifdef kDebugFileActive

    char    *c, *sc, lenCount = 0;
    long    fileLen, scrap;

    if ( gDebugData != nil)
    {
        sc = (char *)s;
        while ( *sc != 0)
        {
            sc++;
            lenCount++;
        }
#ifdef kMaxDebugFileSize
        mFixDebugFileSize( scrap, ((long)lenCount), fileLen)
#else
        fileLen = GetHandleSize( gDebugData);
        SetHandleSize( gDebugData, fileLen + (long)lenCount);
#endif
        if ( MemError() == noErr)
        {
//          HLock( gDebugData);
            c = ( char *)*gDebugData + (fileLen - 1);
            sc = (char *)s;
            do
            {
                *c = *sc;
                c++;
                sc++;
                lenCount--;
            } while ( *sc != 0);
//          HUnlock( gDebugData);
        }
    }
#else
#pragma unused ( s)
#endif
}

void DebugFileAppendLong( long l)
{
#ifdef kDebugFileActive
    if ( gDebugData != nil)
    {
        char    *c, *sc, lenCount;
        long    fileLen, scrap;
        Str255  s;

        NumToString( l, s);
        sc = (char *)s;
#ifdef kMaxDebugFileSize
        mFixDebugFileSize( scrap, ((long)*sc), fileLen)
#else
        fileLen = GetHandleSize( gDebugData);
        SetHandleSize( gDebugData, fileLen + (long)*sc);
#endif
        if ( MemError() == noErr)
        {
//          HLock( gDebugData);
            c = ( char *)*gDebugData + (fileLen - 1);
            lenCount = *sc;
            sc++;
            while( lenCount > 0)
            {
                *c = *sc;
                c++;
                sc++;
                lenCount--;
            }
//          HUnlock( gDebugData);
        }
    }
#else
#pragma unused ( l)
#endif
}

void DebugFileAppendLongHex( long l)
{
#ifdef kDebugFileActive
    char    *c, *sc, lenCount;
    long    fileLen, scrap;
    Str255  s;

    if ( gDebugData != nil)
    {
        NumToHexString( l, s, 4);
        sc = (char *)s;
#ifdef kMaxDebugFileSize
        mFixDebugFileSize( scrap, ((long)*sc), fileLen)
#else
        fileLen = GetHandleSize( gDebugData);
        SetHandleSize( gDebugData, fileLen + (long)*sc);
#endif
        if ( MemError() == noErr)
        {
//          HLock( gDebugData);
            c = ( char *)*gDebugData + (fileLen - 1);
            lenCount = *sc;
            sc++;
            while( lenCount > 0)
            {
                *c = *sc;
                c++;
                sc++;
                lenCount--;
            }
//          HUnlock( gDebugData);
        }
    }
#else
#pragma unused ( l)
#endif
}

void DebugFileAppendSmallFixed( smallFixedType f)
{
#ifdef kDebugFileActive
    char    *c, *sc, lenCount;
    long    fileLen, scrap;
    Str255  s;

    if ( gDebugData != nil)
    {
        SmallFixedToString( f, s);
        sc = (char *)s;
#ifdef kMaxDebugFileSize
        mFixDebugFileSize( scrap, ((long)*sc), fileLen)
#else
        fileLen = GetHandleSize( gDebugData);
        SetHandleSize( gDebugData, fileLen + (long)*sc);
#endif
        if ( MemError() == noErr)
        {
//          HLock( gDebugData);
            c = ( char *)*gDebugData + (fileLen - 1);
            lenCount = *sc;
            sc++;
            while( lenCount > 0)
            {
                *c = *sc;
                c++;
                sc++;
                lenCount--;
            }
//          HUnlock( gDebugData);
        }
    }
#else
#pragma unused( f)
#endif
}

void DebugFileSave( StringPtr fileName)
{
#ifdef kDebugFileActive
    OSErr   err;
    FSSpec  newFile;
    short   newRefNum;
    long    count, rawResult, range = 32768;
    Str255  timeName, timeString;

    rawResult = Random();
    if (rawResult < 0) rawResult *= -1;
    rawResult =  (rawResult * range) / 32768;

    if ( gDebugData != nil)
    {
        CopyPString( (unsigned char *)timeName, (unsigned char *)fileName);
//      PStringFromCString( (char *)timeString, __TIME__);
//      ReplacePStringChar( timeString, '-', ':');
        NumToString( rawResult, timeString);
        ConcatenatePString( (unsigned char *)timeName, (unsigned char *)timeString);

        err = FSMakeFSSpec( 0, 0, timeName, &newFile);
//      if ( err != noErr) ShowSimpleStringAlert( "\pCouldn't make FSSpec out of", timeName, nil, nil);
        err = FSpCreate( &newFile, 'CWIE', 'TEXT', smSystemScript);
        if ( err == dupFNErr)
        {
            err = FSpDelete( &newFile);
            if ( err == noErr) err = FSpCreate( &newFile, 'CWIE', 'TEXT', smSystemScript);
            else SysBeep(20);
        }
        if ( err == noErr)
        {
            err = FSpOpenDF( &newFile, fsCurPerm, &newRefNum);
            if ( err == noErr)
            {
                count = GetHandleSize( gDebugData);
                MoveHHi( gDebugData);
                HLock( gDebugData);
                err = FSWrite( newRefNum, &count, *gDebugData);
                if ( err == noErr)
                {
                    err = FSClose( newRefNum);
                } else ShowErrorRecover ( RESOURCE_ERROR, "\pClose", err);

                HUnlock( gDebugData);
            } else ShowErrorRecover ( RESOURCE_ERROR, "\pOpen", err);
        } else ShowErrorRecover ( RESOURCE_ERROR, "\pCreate", err);
    }
#else
#pragma unused ( fileName)
#endif
}
