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

// Wrap_GameRanger.c

#include "WrapGameRanger.hpp"

#include "GameRanger.h"
#include "NetSprocketGlue.hpp"

namespace antares {

bool Wrap_UseGameRanger( void)
{
    return true;
}

OSErr       Wrap_GRInstallStartupHandler(void)
{
    return noErr;
}

OSErr       Wrap_GRInstallResumeHandler(void)
{
    return noErr;
}

bool     Wrap_GRCheckAEForCmd(const AppleEvent *theEvent)
{
    #pragma unused( theEvent)

    return false;
}

bool     Wrap_GRCheckFileForCmd(void)
{
    return false;
}

bool     Wrap_GRCheckForAE(void)
{
    return false;
}

bool     Wrap_GRIsWaitingCmd(void)
{
    return false;
}

void        Wrap_GRGetWaitingCmd(void)
{

}

bool     Wrap_GRIsCmd(void)
{
    return false;
}

bool     Wrap_GRIsHostCmd(void)
{
    return false;
}

bool     Wrap_GRIsJoinCmd(void)
{
    return false;
}


char*       Wrap_GRGetHostGameName(void)
{
    return nil;
}

UInt16      Wrap_GRGetHostMaxPlayers(void)
{
    return 0;
}

UInt32      Wrap_GRGetJoinAddress(void)
{
    return 0;
}

char*       Wrap_GRGetPlayerName(void)
{
    return nil;
}

UInt16      Wrap_GRGetPortNumber(void)
{
    return 0;
}


void        Wrap_GRReset(void)
{

}


void        Wrap_GRHostReady(void)
{

}

void        Wrap_GRGameBegin(void)
{

}

void        Wrap_GRStatScore(SInt32 score)
{
    #pragma unused( score)
}

void        Wrap_GRStatOtherScore(SInt32 score)
{
    #pragma unused( score)
}

void        Wrap_GRGameEnd(void)
{

}

void        Wrap_GRHostClosed(void)
{

}


OSErr       Wrap_GROpenGameRanger(void)
{
    return noErr;
}

bool
Wrap_GRNSpDoModalHostDialog         (NSpProtocolListReference  ioProtocolList,
                                 Str31                  ioGameName,
                                 Str31                  ioPlayerName,
                                 Str31                  ioPassword,
                                 NSpEventProcPtr        inEventProcPtr)
{
    return Glue_NSpDoModalHostDialog( ioProtocolList, ioGameName, ioPlayerName, ioPassword,
        inEventProcPtr);
}

NSpAddressReference
Wrap_GRNSpDoModalJoinDialog         (ConstStr31Param        inGameType,
                                 ConstStr255Param       inEntityListLabel,
                                 Str31                  ioName,
                                 Str31                  ioPassword,
                                 NSpEventProcPtr        inEventProcPtr)
{
    return Glue_NSpDoModalJoinDialog( inGameType, inEntityListLabel, ioName, ioPassword,
        inEventProcPtr);
}

void
Wrap_GRNSpReleaseAddressReference   (NSpAddressReference    inAddr)
{
    Glue_NSpReleaseAddressReference( inAddr);
}

}  // namespace antares
