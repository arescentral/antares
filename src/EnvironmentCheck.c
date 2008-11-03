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

/*
    Witch Doctor Tester.c
    Written by Hiep Dam
    From The Witches' Brew
    (it's just a name, folks. I'm not into the occult or anything like that!)
    12/2/95

    Shows sample usage of the Witch Doctor.
*/

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

#include "Ares Global Type.h"
#include "Environment Check.h"
#include <Sound.h>
#include "Processor.h"
#include "Options.h"
#include "Key Map Translation.h"

#define kEnvironmentErrorAlert      801
#define kEnvironmentWarningAlert    802
#define kEnvironmentStringID        800

//#pragma code68020 off

extern aresGlobalType *gAresGlobal;
//extern unsigned long  gAresGlobal->gOptions;

Boolean EnvironmentCheck( void)

{
#if TARGET_OS_MAC
    long        gestaltInfo;
    NumVersion  *aVersion;

    gAresGlobal->gOptions |= kOptionMusicDriver | kOptionQuicktime | kOptionSpeechAvailable;
    if ( ShiftKey()) gAresGlobal->gOptions &= ~kOptionMusicDriver;

//  Debugger();
// check System 7 or later
    if ( !(MySWRoutineAvailable( _Gestalt)))
    {
        EnvironmentError( 14);
        return( false);
    }

    if ( Gestalt(gestaltSystemVersion, &gestaltInfo) != noErr)
    {
        SysBeep( 20);
        EnvironmentError( 14);
        return( false);
    }
    if (gestaltInfo < 0x0700)
    {
        SysBeep( 20);
        SysBeep( 20);
        EnvironmentError( 14);
        return( false);
    }

    if ( Gestalt( gestaltMenuMgrAttr, &gestaltInfo) != noErr)
    {
    } else
    {
        if ( gestaltInfo & gestaltMenuMgrPresent)
        {
            gAresGlobal->gOptions |= kOptionUseSystemHideMenuBar;
        }
    }

// check Color QuickDraw
    if ( Gestalt(gestaltQuickdrawVersion, &gestaltInfo) != noErr)
    {
        EnvironmentError(  15);
        return( false);
    }
    if (gestaltInfo < gestalt8BitQD)
    {
        EnvironmentError( 15);
        return( false);
    }

// check 68020 or better
    if ( Gestalt(gestaltProcessorType, &gestaltInfo) != noErr)
    {
        EnvironmentError( 16);
        return( false);
    }
    if (gestaltInfo < gestalt68020)
    {
        EnvironmentError( 16);
        return( false);
    }

// check for 32 bit addressing
    if ( Gestalt(gestaltAddressingModeAttr, &gestaltInfo) != noErr)
    {
        EnvironmentError( 17);
        return( false);
    }
    if (!BitTst(&gestaltInfo, 31-gestalt32BitAddressing))
    {
        EnvironmentError( 17);
        return( false);
    }

// check for QuickTime 2.1 or better (warn if does not exist)
    if ( Gestalt( gestaltQuickTimeVersion, &gestaltInfo) != noErr)
    {
        if (EnvironmentWarning( 19)) return( false);
        else gAresGlobal->gOptions &= ~kOptionQuicktime;
    } else
    {
        aVersion = (NumVersion *)&gestaltInfo;
        if (( aVersion->majorRev < 2) || (( aVersion->majorRev == 2) &&
            ( aVersion->minorAndBugRev < 1)))
        {
            if (EnvironmentWarning(19)) return( false);
            else gAresGlobal->gOptions &= ~kOptionQuicktime;
        }
    }

// make sure QuickTime is native, too

#ifdef powercc
    if ( Gestalt(gestaltQuickTimeFeatures, &gestaltInfo) != noErr)
    {
        if (EnvironmentWarning( 20)) return( false);
        else gAresGlobal->gOptions &= ~kOptionQuicktime;
    } else
    {
        if (!BitTst(&gestaltInfo, 31-gestaltPPCQuickTimeLibPresent))
        {
            if (EnvironmentWarning( 20)) return( false);
            else gAresGlobal->gOptions &= ~kOptionQuicktime;
        }
    }
#endif

    if ( Gestalt( gestaltSpeechAttr, &gestaltInfo) != noErr)
    {
        gAresGlobal->gOptions &= ~kOptionSpeechAvailable;
    } else
    {
        if (!( gestaltInfo & ( 1 << gestaltSpeechMgrPresent)))
        {
            gAresGlobal->gOptions &= ~kOptionSpeechAvailable;
        } else
        {
            gAresGlobal->gOptions |= kOptionSpeechAvailable;
        }
    }

/* DISABLED--PROBLEM IN SOUND HEADER FILE
// check for SoundManager 3 (warn if not 3.1 & PPC)
    if ( !(MySWRoutineAvailable( _SoundDispatch)))
    {
        EnvironmentError( 13);
        return( false);
    }
    soundVersion = SndSoundManagerVersion();
    if ((*(NumVersion*)&soundVersion).majorRev < 3)
    {
        EnvironmentError( 13);
        return( false);
    } else if (((*(NumVersion*)&soundVersion).majorRev == 3) &&
                ((*(NumVersion*)&soundVersion).minorAndBugRev < 0x10))
    {
        #ifdef powercc
        if ( EnvironmentWarning( 18)) return( false);
        #endif
    }
*/
#endif TARGET_OS_MAC
    return( true);
}

void EnvironmentError( short stringNum)

{
    Str255      s1;

    GetIndString( s1, kEnvironmentStringID, stringNum);
//  NumToString( stringNum, s1);
    ParamText( s1, nil, nil, nil);

    Alert( kEnvironmentErrorAlert, nil);
}

Boolean EnvironmentWarning( short stringNum)

{
    Str255      s1;

    GetIndString( s1, kEnvironmentStringID, stringNum);
    ParamText( s1, nil, nil, nil);

    if ( Alert( kEnvironmentWarningAlert, nil) != 1) return( true);
    else return( false);
}

// From IM:OS Utilities p 8-22

/*FUNCTION MySWRoutineAvailable (trapWord: Integer): Boolean;
VAR
trType: TrapType;
BEGIN
{first determine whether it is an Operating System or Toolbox routine}
IF ORD(BAND(trapWord, $0800)) = 0 THEN
trType := OSTrap
ELSE
trType := ToolTrap;
{filter cases where older systems mask with $1FF rather than $3FF}
IF (trType = ToolTrap) AND (ORD(BAND(trapWord, $03FF)) >= $200) AND
(GetToolboxTrapAddress($A86E) = GetToolboxTrapAddress($AA6E)) THEN
MySWRoutineAvailable := FALSE
ELSE
MySWRoutineAvailable := (NGetTrapAddress(trapWord, trType) <>
GetToolboxTrapAddress(_Unimplemented));
END;
*/
Boolean MySWRoutineAvailable( short trapWord)
{
#if TARGET_OS_MAC
    TrapType    trType;

// first determine whether it is an Operating System or Toolbox routine
    if ( trapWord & 0x0800) trType = ToolTrap;
    else trType = OSTrap;
// filter cases where older systems mask with $1FF rather than $3FF
    if (( trType == ToolTrap) && (( trapWord & 0x03ff) >= 0x200) &&
        GetToolboxTrapAddress( 0xa86e) == GetToolboxTrapAddress( 0xaa6e))
            return( false);
    else return ( ( NGetTrapAddress( trapWord, trType) !=
        GetToolboxTrapAddress( _Unimplemented)));
#else
    return( true);
#endif TARGET_OS_MAC
}
//#pragma code68020 reset

