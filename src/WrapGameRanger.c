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

/******************************************\
|**| Wrap_GameRanger.c
\******************************************/

#pragma mark **INCLUDES**
/******************************************\
|**| #includes
\******************************************/

#pragma mark _system includes_
/* - system
*******************************************/

#pragma mark _third party includes_
/* - third party libraries
*******************************************/
#include "GameRanger.h"

#pragma mark _bp libraries includes_
/* - bp libraries
*******************************************/

#pragma mark _this library includes_
/* - this project
*******************************************/
#include "Wrap_GameRanger.h"
#include "Debug.h"

#pragma mark **DEFINITIONS**
/******************************************\
|**| #defines
\******************************************/

/* - definitions
*******************************************/

#pragma mark _macros_
/* - macros
*******************************************/

#pragma mark **TYPEDEFS**
/******************************************\
|**| typedefs
\******************************************/

#pragma mark **EXTERNAL GLOBALS**
/******************************************\
|**| external globals
\******************************************/

#pragma mark **PRIVATE GLOBALS**
/******************************************\
|**| private globals
\******************************************/

#pragma mark **PRIVATE PROTOTYPES**
/******************************************\
|**| private function prototypes
\******************************************/

#pragma mark **PRIVATE FUNCTIONS**
/******************************************\
|**| private functions
\******************************************/

#pragma mark **PUBLIC FUNCTIONS**
/******************************************\
|**| public functions
\******************************************/

Boolean Wrap_UseGameRanger( void)
{
    return true;
}

OSErr       Wrap_GRInstallStartupHandler(void)
{
    OSErr   result;

    mWriteDebugString("\p>GRInstallStartupHandler");

    result =  GRInstallStartupHandler();
    if ( result != noErr) DebugStr("\perr GRInstallStartupHandler");
    return result;
}

OSErr       Wrap_GRInstallResumeHandler(void)
{
    OSErr   result;

    mWriteDebugString("\p>GRInstallResumeHandler");

    result =  GRInstallResumeHandler();
    if ( result != noErr) DebugStr("\perr GRInstallResumeHandler");
    return result;
}

Boolean     Wrap_GRCheckAEForCmd(const AppleEvent *theEvent)
{
    mWriteDebugString("\p>GRCheckAEForCmd");

    return GRCheckAEForCmd( theEvent);
}

Boolean     Wrap_GRCheckFileForCmd(void)
{
    mWriteDebugString("\p>GRCheckFileForCmd");

    return GRCheckFileForCmd();
}

Boolean     Wrap_GRCheckForAE(void)
{
    mWriteDebugString("\p>GRCheckForAE");

    return GRCheckForAE();
}

Boolean     Wrap_GRIsWaitingCmd(void)
{
//  mWriteDebugString("\p>GRIsWaitingCmd");

    return GRIsWaitingCmd();
}

void        Wrap_GRGetWaitingCmd(void)
{
    mWriteDebugString("\p>GRGetWaitingCmd");

    GRGetWaitingCmd();
}

Boolean     Wrap_GRIsCmd(void)
{
    mWriteDebugString("\p>GRIsCmd");

    return GRIsCmd();
}

Boolean     Wrap_GRIsHostCmd(void)
{
    mWriteDebugString("\p>GRIsHostCmd");

    return GRIsHostCmd();
}

Boolean     Wrap_GRIsJoinCmd(void)
{
    mWriteDebugString("\p>GRIsJoinCmd");

    return GRIsJoinCmd();
}


char*       Wrap_GRGetHostGameName(void)
{
    mWriteDebugString("\p>GRGetHostGameName");

    return GRGetHostGameName();
}

UInt16      Wrap_GRGetHostMaxPlayers(void)
{
    mWriteDebugString("\p>GRGetHostMaxPlayers");

    return GRGetHostMaxPlayers();
}

UInt32      Wrap_GRGetJoinAddress(void)
{
    mWriteDebugString("\p>GRGetJoinAddress");

    return GRGetJoinAddress();
}

char*       Wrap_GRGetPlayerName(void)
{
    mWriteDebugString("\p>Wrap_GRGetPlayerName");

    return GRGetPlayerName();
}

UInt16      Wrap_GRGetPortNumber(void)
{
    mWriteDebugString("\p>GRGetPortNumber");

    return GRGetPortNumber();
}


void        Wrap_GRReset(void)
{
    mWriteDebugString("\p>GRReset");

    GRReset();
}


void        Wrap_GRHostReady(void)
{
    mWriteDebugString("\p>GRHostReady");

    GRHostReady();
}

void        Wrap_GRGameBegin(void)
{
    mWriteDebugString("\p>GRGameBegin");

    GRGameBegin();
}

void        Wrap_GRStatScore(SInt32 score)
{
    mWriteDebugString("\p>GRStatScore");

    GRStatScore( score);
}

void        Wrap_GRStatOtherScore(SInt32 score)
{
    mWriteDebugString("\p>GRStatOtherScore");

    GRStatOtherScore( score);
}

void        Wrap_GRGameEnd(void)
{
    mWriteDebugString("\p>GRGameEnd");

    GRGameEnd();
}

void        Wrap_GRHostClosed(void)
{
    mWriteDebugString("\p>GRHostClosed");

    GRHostClosed();
}

OSErr       Wrap_GROpenGameRanger(void)
{
    mWriteDebugString("\p>GROpenGameRanger");

    return GROpenGameRanger();
}

Boolean
Wrap_GRNSpDoModalHostDialog         (NSpProtocolListReference  ioProtocolList,
                                 Str31                  ioGameName,
                                 Str31                  ioPlayerName,
                                 Str31                  ioPassword,
                                 NSpEventProcPtr        inEventProcPtr)
{
    mWriteDebugString("\p>GRNSpDoModalHostDialog");

    return GRNSpDoModalHostDialog( ioProtocolList, ioGameName, ioPlayerName, ioPassword,
        inEventProcPtr);
}

NSpAddressReference
Wrap_GRNSpDoModalJoinDialog         (ConstStr31Param        inGameType,
                                 ConstStr255Param       inEntityListLabel,
                                 Str31                  ioName,
                                 Str31                  ioPassword,
                                 NSpEventProcPtr        inEventProcPtr)
{
    mWriteDebugString("\p>GRNSpDoModalJoinDialog");

    return GRNSpDoModalJoinDialog( inGameType, inEntityListLabel, ioName, ioPassword,
        inEventProcPtr);
}

void
Wrap_GRNSpReleaseAddressReference   (NSpAddressReference    inAddr)
{
    mWriteDebugString("\p>GRNSpReleaseAddressReference");

    GRNSpReleaseAddressReference    (inAddr);
}

