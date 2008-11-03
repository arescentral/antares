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

#ifndef ANTARES_NET_SPROCKET_LIB_GLUE_HPP_
#define ANTARES_NET_SPROCKET_LIB_GLUE_HPP_

//
// NetSprocketLibGlue.h
// The 68K version of NetSprocket is a 68K-CFM library, which normally can only
// be used with a 68K-CFM application. "Classic" 68K libraries cannot be used
// with CFM-68K applications.
//
// This is a problem for Ares for 2 reasons:
// 1. the music library (PlayerPro) is a classic library, and thus can not be used
// with a CFM-68K application.
//
// 2. CW11 cannot correctly compile 68K assembly for CFM-68K applications. It
// incorrectly compiles the fralloc directive, and so is almost useless.
//
// However, TechNote #1077 describes how to call CFM-68K libraries from Classic
// 68K applications. To achieve this, I need to manually create this "glue"
// to the NetSprocket functions I use. For details, see TN1077.
//

#include <NetSprocket.h>

#ifndef powerc
#pragma mpwc on // NetSprocket returns pointers in D0
#endif

//
//  the routines I'm using:
//

enum {
/*
OSStatus NSpInitialize(
    UInt32                      inStandardMessageSize,
    UInt32                      inBufferSize,
    UInt32                      inQElements,
    NSpGameID                   inGameID,
    UInt32                      inTimeout);
*/
    kNSpInitialize_ProcInfo = kCStackBased
        | RESULT_SIZE( SIZE_CODE(sizeof( OSStatus)))
        | STACK_ROUTINE_PARAMETER( 1, SIZE_CODE(sizeof(UInt32)))
        | STACK_ROUTINE_PARAMETER( 2, SIZE_CODE(sizeof(UInt32)))
        | STACK_ROUTINE_PARAMETER( 3, SIZE_CODE(sizeof(UInt32)))
        | STACK_ROUTINE_PARAMETER( 4, SIZE_CODE(sizeof(NSpGameID)))
        | STACK_ROUTINE_PARAMETER( 5, SIZE_CODE(sizeof(UInt32))),

/*
OSStatus NSpGame_Dispose(
    NSpGameReference            inGame,
    NSpFlags                    inFlags);
*/
    kNSpGame_Dispose_ProcInfo = kCStackBased
        | RESULT_SIZE( SIZE_CODE(sizeof( OSStatus)))
        | STACK_ROUTINE_PARAMETER( 1, SIZE_CODE(sizeof(NSpGameReference)))
        | STACK_ROUTINE_PARAMETER( 2, SIZE_CODE(sizeof(NSpFlags))),

/*
OSStatus NSpPlayer_GetInfo(
    NSpGameReference            inGame,
    NSpPlayerID                 inPlayerID,
    NSpPlayerInfoPtr*           outInfo);
*/
    kNSpPlayer_GetInfo_ProcInfo = kCStackBased
        | RESULT_SIZE( SIZE_CODE(sizeof( OSStatus)))
        | STACK_ROUTINE_PARAMETER( 1, SIZE_CODE(sizeof(NSpGameReference)))
        | STACK_ROUTINE_PARAMETER( 2, SIZE_CODE(sizeof(NSpPlayerID)))
        | STACK_ROUTINE_PARAMETER( 3, SIZE_CODE(sizeof(NSpPlayerInfoPtr *))),

/*
OSStatus NSpProtocol_ExtractDefinitionString(
    NSpProtocolReference        inProtocolRef,
    char*                       outDefinitionString);
*/
    kNSpProtocol_ExtractDefinitionString_ProcInfo = kCStackBased
        | RESULT_SIZE( SIZE_CODE(sizeof( OSStatus)))
        | STACK_ROUTINE_PARAMETER( 1, SIZE_CODE(sizeof(NSpProtocolReference)))
        | STACK_ROUTINE_PARAMETER( 2, SIZE_CODE(sizeof(char *))),

/*
OSStatus NSpProtocolList_New(
    NSpProtocolReference        inProtocolRef,
    NSpProtocolListReference*   outList);
*/
    kNSpProtocolList_New_ProcInfo = kCStackBased
        | RESULT_SIZE( SIZE_CODE(sizeof( OSStatus)))
        | STACK_ROUTINE_PARAMETER( 1, SIZE_CODE(sizeof(NSpProtocolReference)))
        | STACK_ROUTINE_PARAMETER( 2, SIZE_CODE(sizeof(NSpProtocolListReference *))),

/*
OSStatus NSpProtocolList_Append(
    NSpProtocolListReference    inProtocolList,
    NSpProtocolReference        inProtocolRef);
*/
    kNSpProtocolList_Append_ProcInfo = kCStackBased
        | RESULT_SIZE( SIZE_CODE(sizeof( OSStatus)))
        | STACK_ROUTINE_PARAMETER( 1, SIZE_CODE(sizeof(NSpProtocolListReference)))
        | STACK_ROUTINE_PARAMETER( 2, SIZE_CODE(sizeof(NSpProtocolReference))),

/*
void NSpProtocolList_Dispose(
    NSpProtocolListReference    inProtocolList);
*/
    kNSpProtocolList_Dispose_ProcInfo = kCStackBased
        | STACK_ROUTINE_PARAMETER( 1, SIZE_CODE(sizeof(NSpProtocolListReference))),

/*
NSpProtocolReference NSpProtocol_CreateAppleTalk(
    ConstStr31Param             inNBPName,
    ConstStr31Param             inNBPType,
    UInt32                      inMaxRTT,
    UInt32                      inMinThruput);
*/
    kNSpProtocol_CreateAppleTalk_ProcInfo = kCStackBased
        | RESULT_SIZE( SIZE_CODE(sizeof( NSpProtocolReference)))
        | STACK_ROUTINE_PARAMETER( 1, SIZE_CODE(sizeof(ConstStr31Param)))
        | STACK_ROUTINE_PARAMETER( 2, SIZE_CODE(sizeof(ConstStr31Param)))
        | STACK_ROUTINE_PARAMETER( 3, SIZE_CODE(sizeof(UInt32)))
        | STACK_ROUTINE_PARAMETER( 4, SIZE_CODE(sizeof(UInt32))),

/*
NSpProtocolReference NSpProtocol_CreateIP(
    InetPort                    inPort,
    UInt32                      inMaxRTT,
    UInt32                      inMinThruput);
*/
    kNSpProtocol_CreateIP_ProcInfo = kCStackBased
        | RESULT_SIZE( SIZE_CODE(sizeof( NSpProtocolReference)))
        | STACK_ROUTINE_PARAMETER( 1, SIZE_CODE(sizeof(InetPort)))
        | STACK_ROUTINE_PARAMETER( 2, SIZE_CODE(sizeof(UInt32)))
        | STACK_ROUTINE_PARAMETER( 3, SIZE_CODE(sizeof(UInt32))),

/*
UInt32 NSpProtocolList_GetCount(
    NSpProtocolListReference    inProtocolList);
*/
    kNSpProtocolList_GetCount_ProcInfo = kCStackBased
        | RESULT_SIZE( SIZE_CODE( sizeof( UInt32)))
        | STACK_ROUTINE_PARAMETER( 1, SIZE_CODE( sizeof( NSpProtocolListReference))),
/*
NSpProtocolReference NSpProtocolList_GetIndexedRef(
    NSpProtocolListReference    inProtocolList,
    UInt32                      inIndex);
*/

    kNSpProtocolList_GetIndexedRef_ProcInfo = kCStackBased
        | RESULT_SIZE( SIZE_CODE(sizeof( NSpProtocolReference)))
        | STACK_ROUTINE_PARAMETER( 1, SIZE_CODE(sizeof(NSpProtocolListReference)))
        | STACK_ROUTINE_PARAMETER( 2, SIZE_CODE(sizeof(UInt32))),

/*
NSpPlayerID NSpPlayer_GetMyID(
    NSpGameReference            inGame);
*/
    kNSpPlayer_GetMyID_ProcInfo = kCStackBased
        | RESULT_SIZE( SIZE_CODE(sizeof( NSpPlayerID)))
        | STACK_ROUTINE_PARAMETER( 1, SIZE_CODE(sizeof(NSpGameReference))),

/*
void NSpPlayer_ReleaseInfo(
    NSpGameReference            inGame,
    NSpPlayerInfoPtr            inInfo);
*/
    kNSpPlayer_ReleaseInfo_ProcInfo = kCStackBased
        | STACK_ROUTINE_PARAMETER( 1, SIZE_CODE(sizeof(NSpGameReference)))
        | STACK_ROUTINE_PARAMETER( 2, SIZE_CODE(sizeof(NSpPlayerInfoPtr))),

/*
UInt32 NSpPlayer_GetRoundTripTime(
    NSpGameReference            inGame,
    NSpPlayerID                 inPlayer);
*/
    kNSpPlayer_GetRoundTripTime_ProcInfo = kCStackBased
        | RESULT_SIZE( SIZE_CODE(sizeof( UInt32)))
        | STACK_ROUTINE_PARAMETER( 1, SIZE_CODE(sizeof(NSpGameReference)))
        | STACK_ROUTINE_PARAMETER( 2, SIZE_CODE(sizeof(NSpPlayerID))),

/*
UInt32 NSpPlayer_GetThruput(
    NSpGameReference            inGame,
    NSpPlayerID                 inPlayer);
*/
    kNSpPlayer_GetThruput_ProcInfo = kCStackBased
        | RESULT_SIZE( SIZE_CODE(sizeof( UInt32)))
        | STACK_ROUTINE_PARAMETER( 1, SIZE_CODE(sizeof(NSpGameReference)))
        | STACK_ROUTINE_PARAMETER( 2, SIZE_CODE(sizeof(NSpPlayerID))),

/*
NSpAddressReference NSpDoModalJoinDialog(
    ConstStr31Param             inGameType,
    ConstStr255Param            inEntityListLabel,
    Str31                       ioName,
    Str31                       ioPassword,
    NSpEventProcPtr             inEventProcPtr);
*/
    kNSpDoModalJoinDialog_ProcInfo = kCStackBased
        | RESULT_SIZE( SIZE_CODE(sizeof( NSpAddressReference)))
        | STACK_ROUTINE_PARAMETER( 1, SIZE_CODE(sizeof(ConstStr31Param)))
        | STACK_ROUTINE_PARAMETER( 2, SIZE_CODE(sizeof(ConstStr255Param)))
        | STACK_ROUTINE_PARAMETER( 3, SIZE_CODE(sizeof(StringPtr)))
        | STACK_ROUTINE_PARAMETER( 4, SIZE_CODE(sizeof(StringPtr)))
        | STACK_ROUTINE_PARAMETER( 5, SIZE_CODE(sizeof(NSpEventProcPtr))),

/*
Boolean NSpDoModalHostDialog(
    NSpProtocolListReference    ioProtocolList,
    Str31                       ioGameName,
    Str31                       ioPlayerName,
    Str31                       ioPassword,
    NSpEventProcPtr             inEventProcPtr);
*/
    kNSpDoModalHostDialog_ProcInfo = kCStackBased
        | RESULT_SIZE( SIZE_CODE(sizeof( Boolean)))
        | STACK_ROUTINE_PARAMETER( 1, SIZE_CODE(sizeof(NSpProtocolListReference)))
        | STACK_ROUTINE_PARAMETER( 2, SIZE_CODE(sizeof(StringPtr)))
        | STACK_ROUTINE_PARAMETER( 3, SIZE_CODE(sizeof(StringPtr)))
        | STACK_ROUTINE_PARAMETER( 4, SIZE_CODE(sizeof(StringPtr)))
        | STACK_ROUTINE_PARAMETER( 5, SIZE_CODE(sizeof(NSpEventProcPtr))),

/*
OSStatus NSpGame_Host(
    NSpGameReference*           outGame,
    NSpProtocolListReference    inProtocolList,
    UInt32                      inMaxPlayers,
    ConstStr31Param             inGameName,
    ConstStr31Param             inPassword,
    ConstStr31Param             inPlayerName,
    NSpPlayerType               inPlayerType,
    NSpTopology                 inTopology,
    NSpFlags                    inFlags);
*/
    kNSpGame_Host_ProcInfo = kCStackBased
        | RESULT_SIZE( SIZE_CODE(sizeof( OSStatus)))
        | STACK_ROUTINE_PARAMETER( 1, SIZE_CODE(sizeof(NSpGameReference *)))
        | STACK_ROUTINE_PARAMETER( 2, SIZE_CODE(sizeof(NSpProtocolListReference)))
        | STACK_ROUTINE_PARAMETER( 3, SIZE_CODE(sizeof(UInt32)))
        | STACK_ROUTINE_PARAMETER( 4, SIZE_CODE(sizeof(ConstStr31Param)))
        | STACK_ROUTINE_PARAMETER( 5, SIZE_CODE(sizeof(ConstStr31Param)))
        | STACK_ROUTINE_PARAMETER( 6, SIZE_CODE(sizeof(ConstStr31Param)))
        | STACK_ROUTINE_PARAMETER( 7, SIZE_CODE(sizeof(NSpPlayerType)))
        | STACK_ROUTINE_PARAMETER( 8, SIZE_CODE(sizeof(NSpTopology)))
        | STACK_ROUTINE_PARAMETER( 9, SIZE_CODE(sizeof(NSpFlags))),
/*
OSStatus NSpGame_Join(
    NSpGameReference*           outGame,
    NSpAddressReference         inAddress,
    ConstStr31Param             inName,
    ConstStr31Param             inPassword,
    NSpPlayerType               inType,
    void*                       inCustomData,
    UInt32                      inCustomDataLen,
    NSpFlags                    inFlags);
*/
    kNSpGame_Join_ProcInfo = kCStackBased
        | RESULT_SIZE( SIZE_CODE(sizeof( OSStatus)))
        | STACK_ROUTINE_PARAMETER( 1, SIZE_CODE(sizeof(NSpGameReference *)))
        | STACK_ROUTINE_PARAMETER( 2, SIZE_CODE(sizeof(NSpAddressReference)))
        | STACK_ROUTINE_PARAMETER( 3, SIZE_CODE(sizeof(ConstStr31Param)))
        | STACK_ROUTINE_PARAMETER( 4, SIZE_CODE(sizeof(ConstStr31Param)))
        | STACK_ROUTINE_PARAMETER( 5, SIZE_CODE(sizeof(NSpPlayerType)))
        | STACK_ROUTINE_PARAMETER( 6, SIZE_CODE(sizeof(void *)))
        | STACK_ROUTINE_PARAMETER( 7, SIZE_CODE(sizeof(UInt32)))
        | STACK_ROUTINE_PARAMETER( 8, SIZE_CODE(sizeof(NSpFlags))),

/*
void NSpReleaseAddressReference(
    NSpAddressReference         inAddress);
*/
    kNSpReleaseAddressReference_ProcInfo = kCStackBased
        | STACK_ROUTINE_PARAMETER( 1, SIZE_CODE(sizeof(NSpAddressReference))),
/*
void NSpClearMessageHeader(
    NSpMessageHeader*           inMessage);
*/
    kNSpClearMessageHeader_ProcInfo = kCStackBased
        | STACK_ROUTINE_PARAMETER( 1, SIZE_CODE(sizeof(NSpMessageHeader *))),

/*
OSStatus NSpMessage_Send(
    NSpGameReference            inGame,
    NSpMessageHeader*           inMessage,
    NSpFlags                    inFlags);
*/
    kNSpMessage_Send_ProcInfo = kCStackBased
        | RESULT_SIZE( SIZE_CODE(sizeof( OSStatus)))
        | STACK_ROUTINE_PARAMETER( 1, SIZE_CODE(sizeof(NSpGameReference)))
        | STACK_ROUTINE_PARAMETER( 2, SIZE_CODE(sizeof(NSpMessageHeader *)))
        | STACK_ROUTINE_PARAMETER( 3, SIZE_CODE(sizeof(NSpFlags))),

/*
NSpMessageHeader *NSpMessage_Get(
    NSpGameReference            inGame);
*/
    kNSpMessage_Get_ProcInfo = kCStackBased
        | RESULT_SIZE( SIZE_CODE(sizeof( NSpMessageHeader *)))
        | STACK_ROUTINE_PARAMETER( 1, SIZE_CODE(sizeof(NSpGameReference))),

/*
void NSpMessage_Release(
    NSpGameReference            inGame,
    NSpMessageHeader*           inMessage);
*/
    kNSpMessage_Release_ProcInfo = kCStackBased
        | STACK_ROUTINE_PARAMETER( 1, SIZE_CODE(sizeof(NSpGameReference)))
        | STACK_ROUTINE_PARAMETER( 2, SIZE_CODE(sizeof(NSpMessageHeader *)))
};

typedef OSStatus ( *NSpInitialize_ProcPtr)(
    UInt32                      inStandardMessageSize,
    UInt32                      inBufferSize,
    UInt32                      inQElements,
    NSpGameID                   inGameID,
    UInt32                      inTimeout);

typedef OSStatus (*NSpGame_Dispose_ProcPtr)(
    NSpGameReference            inGame,
    NSpFlags                    inFlags);

typedef OSStatus (*NSpPlayer_GetInfo_ProcPtr)(
    NSpGameReference            inGame,
    NSpPlayerID                 inPlayerID,
    NSpPlayerInfoPtr*           outInfo);

typedef OSStatus (*NSpProtocol_ExtractDefinitionString_ProcPtr)(
    NSpProtocolReference        inProtocolRef,
    char*                       outDefinitionString);

typedef OSStatus (*NSpProtocolList_Append_ProcPtr)(
    NSpProtocolListReference    inProtocolList,
    NSpProtocolReference        inProtocolRef);

typedef OSStatus (*NSpProtocolList_New_ProcPtr)(
    NSpProtocolReference        inProtocolRef,
    NSpProtocolListReference*   outList);

typedef void (*NSpProtocolList_Dispose_ProcPtr)(
    NSpProtocolListReference    inProtocolList);

typedef NSpProtocolReference (*NSpProtocol_CreateAppleTalk_ProcPtr)(
    ConstStr31Param             inNBPName,
    ConstStr31Param             inNBPType,
    UInt32                      inMaxRTT,
    UInt32                      inMinThruput);

typedef NSpProtocolReference (*NSpProtocol_CreateIP_ProcPtr)(
    InetPort                    inPort,
    UInt32                      inMaxRTT,
    UInt32                      inMinThruput);

typedef UInt32 (*NSpProtocolList_GetCount_ProcPtr)(
    NSpProtocolListReference    inProtocolList);

typedef NSpProtocolReference (*NSpProtocolList_GetIndexedRef_ProcPtr)(
    NSpProtocolListReference    inProtocolList,
    UInt32                      inIndex);

typedef NSpPlayerID (*NSpPlayer_GetMyID_ProcPtr)(
    NSpGameReference            inGame);

typedef void (*NSpPlayer_ReleaseInfo_ProcPtr)(
    NSpGameReference            inGame,
    NSpPlayerInfoPtr            inInfo);

typedef UInt32 (*NSpPlayer_GetRoundTripTime_ProcPtr)(
    NSpGameReference            inGame,
    NSpPlayerID                 inPlayer);

typedef UInt32 (*NSpPlayer_GetThruput_ProcPtr)(
    NSpGameReference            inGame,
    NSpPlayerID                 inPlayer);

typedef NSpAddressReference (*NSpDoModalJoinDialog_ProcPtr)(
    ConstStr31Param             inGameType,
    ConstStr255Param            inEntityListLabel,
    Str31                       ioName,
    Str31                       ioPassword,
    NSpEventProcPtr             inEventProcPtr);

typedef Boolean (*NSpDoModalHostDialog_ProcPtr)(
    NSpProtocolListReference    ioProtocolList,
    Str31                       ioGameName,
    Str31                       ioPlayerName,
    Str31                       ioPassword,
    NSpEventProcPtr             inEventProcPtr);

typedef OSStatus (*NSpGame_Host_ProcPtr)(
    NSpGameReference*           outGame,
    NSpProtocolListReference    inProtocolList,
    UInt32                      inMaxPlayers,
    ConstStr31Param             inGameName,
    ConstStr31Param             inPassword,
    ConstStr31Param             inPlayerName,
    NSpPlayerType               inPlayerType,
    NSpTopology                 inTopology,
    NSpFlags                    inFlags);

typedef OSStatus (*NSpGame_Join_ProcPtr)(
    NSpGameReference*           outGame,
    NSpAddressReference         inAddress,
    ConstStr31Param             inName,
    ConstStr31Param             inPassword,
    NSpPlayerType               inType,
    void*                       inCustomData,
    UInt32                      inCustomDataLen,
    NSpFlags                    inFlags);

typedef void (*NSpReleaseAddressReference_ProcPtr)(
    NSpAddressReference         inAddress);

typedef void (*NSpClearMessageHeader_ProcPtr)(
    NSpMessageHeader*           inMessage);

typedef OSStatus (*NSpMessage_Send_ProcPtr)(
    NSpGameReference            inGame,
    NSpMessageHeader*           inMessage,
    NSpFlags                    inFlags);

typedef NSpMessageHeader *(*NSpMessage_Get_ProcPtr)(
    NSpGameReference            inGame);

typedef void (*NSpMessage_Release_ProcPtr)(
    NSpGameReference            inGame,
    NSpMessageHeader*           inMessage);

Boolean NetSprocketGlueInit( void);
void NetSprocketGlueCleanup( void);

OSStatus Glue_NSpInitialize(
    UInt32                      inStandardMessageSize,
    UInt32                      inBufferSize,
    UInt32                      inQElements,
    NSpGameID                   inGameID,
    UInt32                      inTimeout);

OSStatus Glue_NSpGame_Dispose(
    NSpGameReference            inGame,
    NSpFlags                    inFlags);

OSStatus Glue_NSpPlayer_GetInfo(
    NSpGameReference            inGame,
    NSpPlayerID                 inPlayerID,
    NSpPlayerInfoPtr*           outInfo);

OSStatus Glue_NSpProtocol_ExtractDefinitionString(
    NSpProtocolReference        inProtocolRef,
    char*                       outDefinitionString);

OSStatus Glue_NSpProtocolList_New(
    NSpProtocolReference        inProtocolRef,
    NSpProtocolListReference*   outList);

OSStatus Glue_NSpProtocolList_Append(
    NSpProtocolListReference    inProtocolList,
    NSpProtocolReference        inProtocolRef);

void Glue_NSpProtocolList_Dispose(
    NSpProtocolListReference    inProtocolList);

NSpProtocolReference Glue_NSpProtocol_CreateAppleTalk(
    ConstStr31Param             inNBPName,
    ConstStr31Param             inNBPType,
    UInt32                      inMaxRTT,
    UInt32                      inMinThruput);

NSpProtocolReference Glue_NSpProtocol_CreateIP(
    InetPort                    inPort,
    UInt32                      inMaxRTT,
    UInt32                      inMinThruput);

UInt32 Glue_NSpProtocolList_GetCount(
    NSpProtocolListReference    inProtocolList);

NSpProtocolReference Glue_NSpProtocolList_GetIndexedRef(
    NSpProtocolListReference    inProtocolList,
    UInt32                      inIndex);

NSpPlayerID Glue_NSpPlayer_GetMyID(
    NSpGameReference            inGame);

void Glue_NSpPlayer_ReleaseInfo(
    NSpGameReference            inGame,
    NSpPlayerInfoPtr            inInfo);

UInt32 Glue_NSpPlayer_GetRoundTripTime(
    NSpGameReference            inGame,
    NSpPlayerID                 inPlayer);

UInt32 Glue_NSpPlayer_GetThruput(
    NSpGameReference            inGame,
    NSpPlayerID                 inPlayer);

NSpAddressReference Glue_NSpDoModalJoinDialog(
    ConstStr31Param             inGameType,
    ConstStr255Param            inEntityListLabel,
    Str31                       ioName,
    Str31                       ioPassword,
    NSpEventProcPtr             inEventProcPtr);

Boolean Glue_NSpDoModalHostDialog(
    NSpProtocolListReference    ioProtocolList,
    Str31                       ioGameName,
    Str31                       ioPlayerName,
    Str31                       ioPassword,
    NSpEventProcPtr             inEventProcPtr);

OSStatus Glue_NSpGame_Host(
    NSpGameReference*           outGame,
    NSpProtocolListReference    inProtocolList,
    UInt32                      inMaxPlayers,
    ConstStr31Param             inGameName,
    ConstStr31Param             inPassword,
    ConstStr31Param             inPlayerName,
    NSpPlayerType               inPlayerType,
    NSpTopology                 inTopology,
    NSpFlags                    inFlags);

OSStatus Glue_NSpGame_Join(
    NSpGameReference*           outGame,
    NSpAddressReference         inAddress,
    ConstStr31Param             inName,
    ConstStr31Param             inPassword,
    NSpPlayerType               inType,
    void*                       inCustomData,
    UInt32                      inCustomDataLen,
    NSpFlags                    inFlags);

void Glue_NSpReleaseAddressReference(
    NSpAddressReference         inAddress);

void Glue_NSpClearMessageHeader(
    NSpMessageHeader*           inMessage);

OSStatus Glue_NSpMessage_Send(
    NSpGameReference            inGame,
    NSpMessageHeader*           inMessage,
    NSpFlags                    inFlags);

NSpMessageHeader *Glue_NSpMessage_Get(
    NSpGameReference            inGame);

void Glue_NSpMessage_Release(
    NSpGameReference            inGame,
    NSpMessageHeader*           inMessage);

#ifndef powerc
#pragma mpwc reset
#endif

#endif // ANTARES_NET_SPROCKET_LIB_GLUE_HPP_
