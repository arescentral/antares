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

/* Handle Handling.c
If you need to allocate a handle or get a resource, do it through here.
If there's not enough free memory, all handles are unlocked, moved hi, and any pointers
into those handles are reset.

If you need to allocate a handle and keep it locked all the time, do it through here.
*/

#include "ConditionalMacros.h"

#if TARGET_OS_WIN32
    #include <QuickTimeVR.h>
    #include "QTUtilities.h"
    #include "QTVRUtilities.h"
    #include <TextUtils.h>
    #include <Script.h>
    #include <string.h>
#endif // TARGET_OS_WIN32

#include "Resources.h"
#include <QDOffscreen.h>
#include "DirectText.h"
#include "StringHandling.h"
#include "SoundFX.h"
#include "SpriteHandling.h"
#include "ScreenLabel.h"
#include "HandleHandling.h"
#include "Error.h"
#include "Debug.h"
#include "Randomize.h"

// HACK FOR FINDING MOTION BUG:
#include "Motion.h"

#define kHandleHandleError      "\pHNHN"
#define kMaxHandleHandleNum     256L//128L

#define mCheckLock( mhandle) if ( mhandle != nil) HLock( mhandle)
#define mCheckUnlock( mhandle) if ( mhandle != nil) HUnlock( mhandle)

typedef struct
{
    Handle          *hand;                  // if = nil, then no handle
    void            (*unlockData)( Handle); // if = nil, then call HUnlock
    void            (*lockData)( Handle);       // if = nil, then call HLock
    void            (*resolveData)( Handle);    // if = nil, then do nothing if moved
    Handle          checkHandle;
    Str255          name;
} handleDataType;

Handle      gHandleData = nil;
//Boolean       EMERGENCYHACKTEST = false;

void HHBetterCheckHandle( Handle, Handle, StringPtr);

short HandleHandlerInit( void)

{
    gHandleData = NewHandle( sizeof( handleDataType) * kMaxHandleHandleNum);
    if ( gHandleData == nil)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, MEMORY_ERROR, -1, -1, -1, __FILE__, 1);
        return( MEMORY_ERROR);
    }

//  mHandleLockAndRegister( gHandleData, nil, nil, nil)
    MoveHHi( gHandleData);
    HLock( gHandleData);
    ResetAllHandleData();

    return( kNoError);
}

void HandleHandlerCleanup( void)

{
    if ( gHandleData != nil) DisposeHandle( gHandleData);
}

void ResetAllHandleData( void)

{
    short           i = 0;
    handleDataType  *h = (handleDataType *)*gHandleData;

    for ( i = 0; i < kMaxHandleHandleNum; i++)
    {
        h->hand = nil;
        h->unlockData = h->lockData = h->resolveData = nil;
        h->checkHandle = nil;
        h->name[0] = 0;
        h++;
    }
}

short HHRegisterHandle( Handle *newHandle,
            void            (*unlockData)( Handle),
            void            (*lockData)( Handle),
            void            (*resolveData)( Handle),
            Boolean         checkMe,
            StringPtr       name)

{
    short            i = 0;
    handleDataType  *h = (handleDataType *)*gHandleData;
    OSErr           err;
    SignedByte      hstate;

    hstate = HGetState( *newHandle);
    if ( hstate & 0x40) ShowErrorAny( eContinueErr, -1, "\pPurgable Handle: ", name, nil, nil, -1, -1, -1, -1, __FILE__, 0);
    HHMaxMem();
    h = (handleDataType *)*gHandleData;
    while (( h->hand != nil) && ( i < kMaxHandleHandleNum)) { h++; i++;}
    if ( i == kMaxHandleHandleNum)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kNoMoreHandlesError, -1, -1, -1, __FILE__, 2);
        return( MEMORY_ERROR);
    }

//  WriteDebugLine((char *)"\pNewHan:");
//  WriteDebugLong( i);

    h->hand = newHandle;
    h->unlockData = unlockData;
    h->lockData = lockData;
    h->resolveData = resolveData;
    if ( name != nil)
        CopyPString( h->name, name);
    else h->name[0] = 0;
    if ( checkMe)
    {
        h->checkHandle = (Handle)*newHandle;
        err = HandToHand( &h->checkHandle);
        if ( err != noErr) MyDebugString("\pError Making Check Copy of Handle");
        MoveHHi( h->checkHandle);
        HHBetterCheckHandle( *h->hand, h->checkHandle, h->name);
    } else h->checkHandle = nil;

//  HHMaxMem();
//  HHCheckAllHandles();

    return( kNoError);
}

void HHDeregisterHandle( Handle *killHandle)

{
    short            i = 0;
    handleDataType  *h = (handleDataType *)*gHandleData;

    while (( h->hand != killHandle) && ( i < kMaxHandleHandleNum)) { h++; i++;}
    if ( i < kMaxHandleHandleNum)
    {
        if ( h->checkHandle != nil) DisposeHandle( h->checkHandle);
        h->hand = nil;
        h->unlockData = h->lockData = h->resolveData = nil;
        h->checkHandle = nil;
    }
    HHMaxMem();
}

void HHMaxMem( void)

{
    Size            grow = 0, free = 0;
    short           i = 0;
    handleDataType  *h = (handleDataType *)*gHandleData;
    Str255          debug;

//  WriteDebugLine((char *)"\p<MAXMEM");

//  HHCheckAllHandles();
    for ( i = 0; i < kMaxHandleHandleNum; i++)
    {
        if ( h->hand != nil)
        {
            if ( h->unlockData == nil)
            {
                HUnlock( *(h->hand));
/*              thandle = *(h->hand);
                HandToHand( &thandle);
                if (thandle != nil)
                {
                    c = (unsigned char *)**(h->hand),
                    *c = 0xff;
                    HHClearHandle( *(h->hand));
                    DisposeHandle( *(h->hand));
                    *(h->hand) = thandle;
                } else DebugStr("\pHandToHand Failed.");

*/          } else
            {
                CopyPString( debug, "\pUNLOCKING: ");
                ConcatenatePString( debug, h->name);
//              if ( EMERGENCYHACKTEST) DebugStr( debug);
                (*(h->unlockData))( *(h->hand));
            }
        }
        h++;
    }

    HUnlock( gHandleData);

    free = MaxMem( &grow);

    HLock( gHandleData);

    h = (handleDataType *)*gHandleData;
    for ( i = 0; i < kMaxHandleHandleNum; i++)
    {
        if ( h->hand != nil)
        {
            if ( h->lockData == nil)
            {
                HLock( *(h->hand));
            } else
            {
                CopyPString( debug, "\pLOCKING: ");
                ConcatenatePString( debug, h->name);
//              if ( EMERGENCYHACKTEST) DebugStr( debug);
                (*(h->lockData))( *(h->hand));
            }
        }
        h++;
    }

    h = (handleDataType *)*gHandleData;
    for ( i = 0; i < kMaxHandleHandleNum; i++)
    {
        if ( h->hand != nil)
        {
            if ( h->resolveData != nil)
            {
                (*(h->resolveData))( *(h->hand));
            }
        }
        h++;
    }
//  HHCheckAllHandles();
}

/*
maybe I should consider callback routines?  So, when I make a new handle, I just pass the
handle and a call-back routine for resetting the data.  It unlocks it, calls MaxMem, calls
my call back, if any, then locks it.  Then, all I'd have to keep track of was a list of
handles.  A Handle of Handles.  Maybe that's the best way.  More universal, less specific.
Hmmm.  Unless I try to remove a handle.  Stoopid IDs?  That'd be another global for every
freakin handle!  Assume handle is locked--then, just say if handle you're passing for disposal
== handle n in my list, then I remove it, and all's dandy.
*/

Handle HHNewHandle( long newSize)

{
    Handle  aNewHandle = nil;

    aNewHandle = NewHandle( newSize);
    if ( aNewHandle == nil)
    {
        HHMaxMem();
        aNewHandle = NewHandle( newSize);
    }

    return ( aNewHandle);
}

Handle HHGetResource( ResType resourceType, short resID)

{
    Handle  aNewHandle = nil;

//  HHMaxMem();
    aNewHandle = GetResource( resourceType, resID);
    if ( aNewHandle == nil)
    {
        WriteDebugLine((char *)"\pNILRES!");
        HHMaxMem();
        aNewHandle = GetResource( resourceType, resID);
    }
//  HHCheckAllHandles();
    return ( aNewHandle);

}

void HHConcatenateHandle( Handle destHandle, Handle sourceHandle)
{
    SignedByte  sourceHandleState, destHandleState;

    if (( destHandle != nil) && ( sourceHandle != nil))
    {
        sourceHandleState = HGetState( sourceHandle);
        destHandleState = HGetState( destHandle);
        HLock( sourceHandle);
        HUnlock( destHandle);
        HandAndHand( sourceHandle, destHandle);
        if ( MemError() != noErr)
        {
            HHMaxMem();
            HandAndHand( sourceHandle, destHandle);
        }
        HSetState( sourceHandle, sourceHandleState);
        HSetState( destHandle, destHandleState);
    }
}

void HHClearHandle( Handle aHandle)
{
    unsigned char   *c = (unsigned char *)*aHandle, value = 0;
    long            handleSize = GetHandleSize( aHandle);

    while ( handleSize > 0)
    {
        *c = Randomize( 0x100); //value++;//
        c++;
        handleSize--;
    }
//  HHCheckAllHandles();
}

void HHCheckHandle( Handle oneHandle, Handle twoHandle)
{
    unsigned char   *onec = (unsigned char *)*oneHandle, *twoc  = (unsigned char *)*twoHandle;
    long            oneHandleSize = GetHandleSize( oneHandle), twoHandleSize = GetHandleSize( twoHandle);

    if ( oneHandleSize != twoHandleSize) MyDebugString("\pNot Even CLOSE!");

    if ( *onec != *twoc) MyDebugString("\pZoinks!");
    while (( oneHandleSize > 0) && ( *onec == *twoc))
    {
        oneHandleSize--;
        onec++;
        twoc++;

        if ((oneHandleSize > 0) && ( *onec != *twoc))
        {
            MyDebugString("\pZoops!");
            oneHandleSize = 0;
        }
    }
}

void HHBetterCheckHandle( Handle oneHandle, Handle twoHandle, StringPtr s)
{
    unsigned char   *onec = (unsigned char *)*oneHandle, *twoc  = (unsigned char *)*twoHandle;
    long            oneHandleSize = GetHandleSize( oneHandle), twoHandleSize = GetHandleSize( twoHandle);
    Str255          debugstr;
    Boolean         equal = true;

    CopyPString( debugstr, s);
    if ( oneHandleSize != twoHandleSize)
    {
        equal = false;
        ConcatenatePString( s, "\p sizes not equal!");
    }

    if (!(equal) && ( *onec != *twoc))
    {
        equal = false;
        ConcatenatePString( s, "\p first byte not equal!");
    }

    if ( !equal)
    {
        while (( oneHandleSize > 0) && ( *onec == *twoc))
        {
            oneHandleSize--;
            onec++;
            twoc++;

            if ((oneHandleSize > 0) && ( *onec != *twoc))
            {
                equal = false;
                ConcatenatePString( s, "\p found a bad byte!");
                oneHandleSize = 0;
            }
        }
    }
    if ( !equal) MyDebugString( s);
}

void HHCheckAllHandles( void)
{
    handleDataType  *h = (handleDataType *)*gHandleData;
    long            i;

    for ( i = 0; i < kMaxHandleHandleNum; i++)
    {
        if (( h->hand != nil) && ( h->checkHandle != nil))
        {
            HLock( h->checkHandle);
            HHBetterCheckHandle( *h->hand, h->checkHandle, h->name);
            HUnlock( h->checkHandle);
        }
        h++;
    }

}
/*
    gAresGlobal->gAdmiralData
    gAresGlobal->gDestBalanceData
    gHostEntity
    gClientEntity
    gKeyMapData
    gSerialNumber
    gColorTranslateTable
    gFourBitTable
    gAresGlobal->gScenarioBriefData
    gAresGlobal->gScaleList
    gAresGlobal->gSectorLineData
    gInterfaceItemData
    gAresGlobal->gMessageData
    gDestinationString
    gMiniScreenLine
    gAresGlobal->gProximityGrid
    gRandomTable
    gRotTable
    gAresGlobal->gScenarioData
    gAresGlobal->gScreenLabelData
    gAresGlobal->gScrollStarData
    gSpaceObjectData
    gBaseObjectData
    gSpriteTable
    gSpriteTable
    gBothScaleMaps
    gBlipTable
    gDirectText
    for ( i = 0; i < kSoundNum; i++)
    {
        gAresGlobal->gSound[i].soundHandle
    }
//  pixMap = GetGWorldPixMap( gOffWorld);
//  LockPixels( pixMap);
//  pixMap = GetGWorldPixMap( gSaveWorld);
//  LockPixels( pixMap);
*/
