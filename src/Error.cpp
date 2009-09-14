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

// Error Handling

#include "Error.hpp"

#include <Fonts.h>
#include <Palettes.h>
#include <Quickdraw.h>

#include "ConditionalMacros.h"
#include "DialogFont.h"
#include "Resources.h"
#include "SetFontByString.h"
#include "StringNumerics.hpp"

#define ERROR_STR_ID    800
#define ERROR_ALERT_ID  800

//#define   kDebugError // comment out to not include internal code

#ifdef kDebugError
#define kAnyAlertID         806
#define kAnyErrorDialogID   811
#else
#define kAnyAlertID         805
#define kAnyErrorDialogID   810
#endif
extern GDHandle                 theDevice;

#define kContinueButton     1
#define kQuitButton         2

void ErrPStringFromCString( unsigned char *, unsigned char *);
void ErrConcatenatePString(unsigned char*, unsigned char*);

void ShowErrorNoRecover( int whichError, const unsigned char* sourceCode, int sourceNum)

{
    Str255  s1, s3;

//  if ( theDevice != nil) RestoreDeviceClut( theDevice);

    GetIndString( s1, ERROR_STR_ID, whichError);
    if ( sourceCode == nil)
        ParamText( s1, nil, nil, nil);
     else
     {
        NumToString(sourceNum, s3);
        ParamText( s1, sourceCode, s3, nil);
     }
    StopAlert( ERROR_ALERT_ID, nil);
    ExitToShell();
}

void ShowErrorRecover( int whichError, const unsigned char* sourceCode, int sourceNum)

{
    Str255  s1, s3;

//  if ( theDevice != nil) RestoreDeviceClut( theDevice);

    GetIndString( s1, ERROR_STR_ID, whichError);
    if ( sourceCode == nil)
        ParamText( s1, nil, nil, nil);
     else
     {
        NumToString(sourceNum, s3);
        ParamText( s1, sourceCode, s3, nil);
     }
    StopAlert( ERROR_ALERT_ID, nil);
}

void ShowSimpleStringAlert(
    const unsigned char* string1, const unsigned char* string2,
    const unsigned char* string3, const unsigned char* string4)
{
//  if ( theDevice != nil) RestoreDeviceClut( theDevice);

    ParamText( string1, string2, string3, string4);
    StopAlert( 805, nil);
}

void ShowSimpleStrResAlert( short ResID, short num1, short num2, short num3, short num4)

{
    Str255      s1, s2, s3, s4;
    unsigned char* sp1 = nil;
    unsigned char* sp2 = nil;
    unsigned char* sp3 = nil;
    unsigned char* sp4 = nil;

    if ( ResID >= 0)
    {
        if ( num1 > 0)
        {
            GetIndString( s1, ResID, num1);
            sp1 = s1;
        }
        if ( num2 > 0)
        {
            GetIndString( s2, ResID, num2);
            sp2 = s2;
        }
        if ( num3 > 0)
        {
            GetIndString( s3, ResID, num3);
            sp3 = s3;
        }
        if ( num4 > 0)
        {
            GetIndString( s4, ResID, num4);
            sp4 = s4;
        }
    }
    ShowSimpleStringAlert( sp1, sp2, sp3, sp4);
}

//
// template:
// ShowErrorAny( false, kErrorStrID, nil, nil, nil, nil, -1, -1, -1, -1, __FILE__, 0);
//

void ShowErrorAny(  errorRecoverType recover,
                        short ResID,
                        const unsigned char* sp1,
                        const unsigned char* sp2,
                        const unsigned char* sp3,
                        const unsigned char* sp4,
                        long num1,
                        long num2,
                        long num3,
                        long num4,
                        char *caller, // pass __FILE__
                        long callerNum)

{
    Str255          s1, s2, s3, s4;
    GrafPtr         oldPort = nil;
    DialogPtr       theDialog = nil;
    short           itemHit, itemType;
    Boolean         done = false, quit = false;
    Rect            itemRect;
    Handle          itemHandle;

#pragma unused ( caller, callerNum)
    ParamText( "\p", "\p", "\p", "\p");
    InitCursor();
    FlushEvents(everyEvent, 0);
    if ( ResID >= 0)
    {
        if ( num1 > 0)
        {
            GetIndString( s1, ResID, num1);
            sp1 = s1;
        }
        if ( num2 > 0)
        {
            GetIndString( s2, ResID, num2);
            sp2 = s2;
        }
        if ( num3 > 0)
        {
            GetIndString( s3, ResID, num3);
            sp3 = s3;
        }
        if ( num4 > 0)
        {
            GetIndString( s4, ResID, num4);
            sp4 = s4;
        }
    }
#ifdef kDebugError

    ErrPStringFromCString( (unsigned char *)callerString, (unsigned char *)caller);
    ErrConcatenatePString( callerString, "\p, #");
    NumToString( callerNum, s4);
    ErrConcatenatePString( callerString, s4);
    sp4 = callerString;
#endif

//  if ( theDevice != nil) RestoreDeviceClut( theDevice);

    ParamText( sp1, sp2, sp3, sp4);
//  StopAlert( kAnyAlertID, nil);
    GetPort( &oldPort);
    theDialog = GetNewDialog( kAnyErrorDialogID, nil, reinterpret_cast<WindowPtr>(-1L));

    if ( theDialog == nil) DebugStr("\pNo Error Dialog!");
    SetWRefCon( theDialog, kAnyErrorDialogID);
    MacSetPort(reinterpret_cast<Window*>(theDialog));
    SetDialogFontAndSize( theDialog, GetFontNumByString("\pgeneva"), 10);
    if (( recover != eContinueErr) && ( recover != eContinueOnlyErr))
    {
        SetDialogDefaultItem( theDialog, kQuitButton);
        GetDialogItem( theDialog, kContinueButton, &itemType, &itemHandle, &itemRect);
//      SetControlTitle( (ControlHandle)itemHandle, "\pDebugger");
        HiliteControl( reinterpret_cast<ControlHandle>(itemHandle), 255);
    } else
    {
        if ( recover == eContinueOnlyErr)
        {
            GetDialogItem( theDialog, kQuitButton, &itemType, &itemHandle, &itemRect);
            HiliteControl( reinterpret_cast<ControlHandle>(itemHandle), 255);
        }
        SetDialogDefaultItem( theDialog, kContinueButton);
    }

    MacShowWindow(reinterpret_cast<Window*>(theDialog));

    done = false;
    while ( done == false)
    {
        ModalDialog( nil, &itemHit);
        switch( itemHit)
        {
            case kContinueButton:
                done = true;
                break;

            case kQuitButton:
                done = true;
                quit = true;
                break;

        }
    }
    if ( theDialog != nil) DisposeDialog( theDialog);
    MacSetPort( oldPort);
    if (( recover == eExitToShellErr) || (( recover == eContinueErr) && ( quit)))
    {
        ExitToShell();
    }
}

//
// shows an alert which reads:
// [string] [error#] occured.
// where [string] is specified by an indexed string and error # is an error result
// code. Use it to show simple errors like:
// Couldn't create the monkey because an error of type -234 occured.
// template:
// ShowErrorOfTypeOccurred( false, kErrorStrID, -1, status, __FILE__, 0);
//

void ShowErrorOfTypeOccurred( errorRecoverType recover, short resID, short stringNum,
    OSErr error, char *caller, long callerNum)
{
    Str255  occurredString, errorString;

    GetIndString( occurredString, kErrorStrID, kOccurredError);
    NumToString( error, errorString);
    ErrConcatenatePString( errorString, occurredString);
    ShowErrorAny( recover, resID, nil, errorString, nil, nil, stringNum,
        -1, -1, -1, caller, callerNum);
}

void ErrPStringFromCString( unsigned char *pString, unsigned char *cString)

{
    unsigned char   *len;

    len = pString;
    pString++;
    *len = 0;
    while( *cString != '\0')
    {
        *pString = *cString;
        (*len)++;
        pString++;
        cString++;
    }
}

void ErrConcatenatePString(unsigned char* dString, unsigned char* sString) {
    unsigned char   *dc, *sc;
    int     i;

    dc = dString + implicit_cast<long>(*dString) + 1L;
    sc = sString + 1L;
    for ( i = 0; i < *sString; i++)
    {
        *dc = *sc;
        (*dString)++;
        dc++;
        sc++;
    }
}

void MyDebugString( const unsigned char* s)
{
#ifdef kDebugError
    DebugStr( s);
#else
    ShowSimpleStringAlert( s, "\p", "\p", "\p");
#endif
}
