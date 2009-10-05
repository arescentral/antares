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

#ifndef ANTARES_WRAP_GAME_RANGER_HPP_
#define ANTARES_WRAP_GAME_RANGER_HPP_

// Wrap_GameRanger.h

#include <Base.h>
#include <NetSprocket.h>

namespace antares {

STUB0(Wrap_GRCheckForAE, bool(), false);
STUB0(Wrap_GRGetWaitingCmd, void());
STUB0(Wrap_GRInstallResumeHandler, OSErr(), noErr);
STUB0(Wrap_GRIsWaitingCmd, bool(), false);
STUB0(Wrap_UseGameRanger, bool(), false);

#if 0
bool Wrap_UseGameRanger( void);

OSErr       Wrap_GRInstallStartupHandler(void);
OSErr       Wrap_GRInstallResumeHandler(void);

bool     Wrap_GRCheckAEForCmd(const AppleEvent *theEvent);
bool     Wrap_GRCheckFileForCmd(void);
bool     Wrap_GRCheckForAE(void);

bool     Wrap_GRIsWaitingCmd(void);
void        Wrap_GRGetWaitingCmd(void);

bool     Wrap_GRIsCmd(void);
bool     Wrap_GRIsHostCmd(void);
bool     Wrap_GRIsJoinCmd(void);

char*       Wrap_GRGetHostGameName(void);
UInt16      Wrap_GRGetHostMaxPlayers(void);
UInt32      Wrap_GRGetJoinAddress(void);
char*       Wrap_GRGetPlayerName(void);
UInt16      Wrap_GRGetPortNumber(void);

void        Wrap_GRReset(void);

void        Wrap_GRHostReady(void);
void        Wrap_GRGameBegin(void);
void        Wrap_GRStatScore(SInt32 score);
void        Wrap_GRStatOtherScore(SInt32 score);
void        Wrap_GRGameEnd(void);
void        Wrap_GRHostClosed(void);

OSErr       Wrap_GROpenGameRanger(void);

bool
Wrap_GRNSpDoModalHostDialog         (NSpProtocolListReference  ioProtocolList,
                                 Str31                  ioGameName,
                                 Str31                  ioPlayerName,
                                 Str31                  ioPassword,
                                 NSpEventProcPtr        inEventProcPtr);

NSpAddressReference
Wrap_GRNSpDoModalJoinDialog         (ConstStr31Param        inGameType,
                                 ConstStr255Param       inEntityListLabel,
                                 Str31                  ioName,
                                 Str31                  ioPassword,
                                 NSpEventProcPtr        inEventProcPtr);

void
Wrap_GRNSpReleaseAddressReference   (NSpAddressReference    inAddr);
#endif

}  // namespace antares

#endif // ANTARES_WRAP_GAME_RANGER_HPP_
