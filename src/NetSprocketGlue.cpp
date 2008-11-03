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

// NetSprocketGlue.h

#include "NetSprocketGlue.h"

#include "GameRanger.h"
#include "Processor.h"

#ifndef powerc
#pragma mpwc on
#endif

#ifdef kUseCFMGlue
typedef struct NetSprocketGlueProcPtrType
{
    UniversalProcPtr            NSpInitialize_Proc;
    UniversalProcPtr            NSpGame_Dispose_Proc;
    UniversalProcPtr            NSpPlayer_GetInfo_Proc;
    UniversalProcPtr            NSpProtocol_ExtractDefinitionString_Proc;
    UniversalProcPtr            NSpProtocolList_Append_Proc;
    UniversalProcPtr            NSpProtocolList_New_Proc;
    UniversalProcPtr            NSpProtocolList_Dispose_Proc;
    UniversalProcPtr            NSpProtocol_CreateAppleTalk_Proc;
    UniversalProcPtr            NSpProtocol_CreateIP_Proc;
    UniversalProcPtr            NSpProtocolList_GetCount_Proc;
    UniversalProcPtr            NSpProtocolList_GetIndexedRef_Proc;
    UniversalProcPtr            NSpPlayer_GetMyID_Proc;
    UniversalProcPtr            NSpPlayer_ReleaseInfo_Proc;
    UniversalProcPtr            NSpPlayer_GetRoundTripTime_Proc;
    UniversalProcPtr            NSpPlayer_GetThruput_Proc;
    UniversalProcPtr            NSpDoModalJoinDialog_Proc;
    UniversalProcPtr            NSpDoModalHostDialog_Proc;
    UniversalProcPtr            NSpGame_Host_Proc;
    UniversalProcPtr            NSpGame_Join_Proc;
    UniversalProcPtr            NSpReleaseAddressReference_Proc;
    UniversalProcPtr            NSpClearMessageHeader_Proc;
    UniversalProcPtr            NSpMessage_Send_Proc;
    UniversalProcPtr            NSpMessage_Get_Proc;
    UniversalProcPtr            NSpMessage_Release_Proc;
} NetSprocketGlueProcPtrType;

NetSprocketGlueProcPtrType  *gNSpGlue;
#endif

static OSErr Find_Symbol(Ptr* pSymAddr,
                         Str255 pSymName,
                         ProcInfoType pProcInfo);

static pascal OSErr GetSystemArchitecture(OSType *archType);


Boolean NetSprocketGlueInit( void)
{
#ifdef kUseCFMGlue
    long    gestaltInfo;

    if ( Gestalt( gestaltCFMAttr, &gestaltInfo) != noErr)
    {
        return( false);
    } else
    {
        if (!( gestaltInfo & (1 << gestaltCFMPresent))) return( false);
    }

    gNSpGlue = (NetSprocketGlueProcPtrType *)NewPtr( sizeof( NetSprocketGlueProcPtrType));
    if ( gNSpGlue == nil) return( false);

//  NSpInitialize
    gNSpGlue->NSpInitialize_Proc = (UniversalProcPtr)kUnresolvedCFragSymbolAddress;
    Find_Symbol( (Ptr *)&gNSpGlue->NSpInitialize_Proc, "\pNSpInitialize", kNSpInitialize_ProcInfo);
    if ( (Ptr)gNSpGlue->NSpInitialize_Proc == (Ptr)kUnresolvedCFragSymbolAddress) return( false);

//  NSpGame_Dispose
    gNSpGlue->NSpGame_Dispose_Proc = (UniversalProcPtr)kUnresolvedCFragSymbolAddress;
    Find_Symbol( (Ptr *)&gNSpGlue->NSpGame_Dispose_Proc, "\pNSpGame_Dispose", kNSpGame_Dispose_ProcInfo);
    if ( (Ptr)gNSpGlue->NSpGame_Dispose_Proc == (Ptr)kUnresolvedCFragSymbolAddress) return( false);

//  NSpPlayer_GetInfo
    gNSpGlue->NSpPlayer_GetInfo_Proc = (UniversalProcPtr)kUnresolvedCFragSymbolAddress;
    Find_Symbol( (Ptr *)&gNSpGlue->NSpPlayer_GetInfo_Proc, "\pNSpPlayer_GetInfo", kNSpPlayer_GetInfo_ProcInfo);
    if ( (Ptr)gNSpGlue->NSpPlayer_GetInfo_Proc == (Ptr)kUnresolvedCFragSymbolAddress) return( false);

//  NSpProtocol_ExtractDefinitionString
    gNSpGlue->NSpProtocol_ExtractDefinitionString_Proc = (UniversalProcPtr)kUnresolvedCFragSymbolAddress;
    Find_Symbol( (Ptr *)&gNSpGlue->NSpProtocol_ExtractDefinitionString_Proc, "\pNSpProtocol_ExtractDefinitionString", kNSpProtocol_ExtractDefinitionString_ProcInfo);
    if ( (Ptr)gNSpGlue->NSpProtocol_ExtractDefinitionString_Proc == (Ptr)kUnresolvedCFragSymbolAddress) return( false);

//  NSpProtocolList_Append
    gNSpGlue->NSpProtocolList_Append_Proc = (UniversalProcPtr)kUnresolvedCFragSymbolAddress;
    Find_Symbol( (Ptr *)&gNSpGlue->NSpProtocolList_Append_Proc, "\pNSpProtocolList_Append", kNSpProtocolList_Append_ProcInfo);
    if ( (Ptr)gNSpGlue->NSpProtocolList_Append_Proc == (Ptr)kUnresolvedCFragSymbolAddress) return( false);

//  NSpProtocolList_New
    gNSpGlue->NSpProtocolList_New_Proc = (UniversalProcPtr)kUnresolvedCFragSymbolAddress;
    Find_Symbol( (Ptr *)&gNSpGlue->NSpProtocolList_New_Proc, "\pNSpProtocolList_New", kNSpProtocolList_New_ProcInfo);
    if ( (Ptr)gNSpGlue->NSpProtocolList_New_Proc == (Ptr)kUnresolvedCFragSymbolAddress) return( false);

//  NSpProtocolList_Dispose
    gNSpGlue->NSpProtocolList_Dispose_Proc = (UniversalProcPtr)kUnresolvedCFragSymbolAddress;
    Find_Symbol( (Ptr *)&gNSpGlue->NSpProtocolList_Dispose_Proc, "\pNSpProtocolList_Dispose", kNSpProtocolList_Dispose_ProcInfo);
    if ( (Ptr)gNSpGlue->NSpProtocolList_Dispose_Proc == (Ptr)kUnresolvedCFragSymbolAddress) return( false);

//  NSpProtocol_CreateAppleTalk
    gNSpGlue->NSpProtocol_CreateAppleTalk_Proc = (UniversalProcPtr)kUnresolvedCFragSymbolAddress;
    Find_Symbol( (Ptr *)&gNSpGlue->NSpProtocol_CreateAppleTalk_Proc, "\pNSpProtocol_CreateAppleTalk", kNSpProtocol_CreateAppleTalk_ProcInfo);
    if ( (Ptr)gNSpGlue->NSpProtocol_CreateAppleTalk_Proc == (Ptr)kUnresolvedCFragSymbolAddress) return( false);

//  NSpProtocol_CreateIP
    gNSpGlue->NSpProtocol_CreateIP_Proc = (UniversalProcPtr)kUnresolvedCFragSymbolAddress;
    Find_Symbol( (Ptr *)&gNSpGlue->NSpProtocol_CreateIP_Proc, "\pNSpProtocol_CreateIP", kNSpProtocol_CreateIP_ProcInfo);
    if ( (Ptr)gNSpGlue->NSpProtocol_CreateIP_Proc == (Ptr)kUnresolvedCFragSymbolAddress) return( false);

//  NSpProtocolList_GetCount
    gNSpGlue->NSpProtocolList_GetCount_Proc = (UniversalProcPtr)kUnresolvedCFragSymbolAddress;
    Find_Symbol( (Ptr *)&gNSpGlue->NSpProtocolList_GetCount_Proc, "\pNSpProtocolList_GetCount", kNSpProtocolList_GetCount_ProcInfo);
    if ( (Ptr)gNSpGlue->NSpProtocolList_GetCount_Proc == (Ptr)kUnresolvedCFragSymbolAddress) return( false);

//  NSpProtocolList_GetIndexedRef
    gNSpGlue->NSpProtocolList_GetIndexedRef_Proc = (UniversalProcPtr)kUnresolvedCFragSymbolAddress;
    Find_Symbol( (Ptr *)&gNSpGlue->NSpProtocolList_GetIndexedRef_Proc, "\pNSpProtocolList_GetIndexedRef", kNSpProtocolList_GetIndexedRef_ProcInfo);
    if ( (Ptr)gNSpGlue->NSpProtocolList_GetIndexedRef_Proc == (Ptr)kUnresolvedCFragSymbolAddress) return( false);

//  NSpPlayer_GetMyID
    gNSpGlue->NSpPlayer_GetMyID_Proc = (UniversalProcPtr)kUnresolvedCFragSymbolAddress;
    Find_Symbol( (Ptr *)&gNSpGlue->NSpPlayer_GetMyID_Proc, "\pNSpPlayer_GetMyID", kNSpPlayer_GetMyID_ProcInfo);
    if ( (Ptr)gNSpGlue->NSpPlayer_GetMyID_Proc == (Ptr)kUnresolvedCFragSymbolAddress) return( false);

//  NSpPlayer_ReleaseInfo
    gNSpGlue->NSpPlayer_ReleaseInfo_Proc = (UniversalProcPtr)kUnresolvedCFragSymbolAddress;
    Find_Symbol( (Ptr *)&gNSpGlue->NSpPlayer_ReleaseInfo_Proc, "\pNSpPlayer_ReleaseInfo", kNSpPlayer_ReleaseInfo_ProcInfo);
    if ( (Ptr)gNSpGlue->NSpPlayer_ReleaseInfo_Proc == (Ptr)kUnresolvedCFragSymbolAddress) return( false);

//  NSpPlayer_GetRoundTripTime
    gNSpGlue->NSpPlayer_GetRoundTripTime_Proc = (UniversalProcPtr)kUnresolvedCFragSymbolAddress;
    Find_Symbol( (Ptr *)&gNSpGlue->NSpPlayer_GetRoundTripTime_Proc, "\pNSpPlayer_GetRoundTripTime", kNSpPlayer_GetRoundTripTime_ProcInfo);
    if ( (Ptr)gNSpGlue->NSpPlayer_GetRoundTripTime_Proc == (Ptr)kUnresolvedCFragSymbolAddress) return( false);

//  NSpPlayer_GetThruput
    gNSpGlue->NSpPlayer_GetThruput_Proc = (UniversalProcPtr)kUnresolvedCFragSymbolAddress;
    Find_Symbol( (Ptr *)&gNSpGlue->NSpPlayer_GetThruput_Proc, "\pNSpPlayer_GetThruput", kNSpPlayer_GetThruput_ProcInfo);
    if ( (Ptr)gNSpGlue->NSpPlayer_GetThruput_Proc == (Ptr)kUnresolvedCFragSymbolAddress) return( false);

//  NSpDoModalJoinDialog
    gNSpGlue->NSpDoModalJoinDialog_Proc = (UniversalProcPtr)kUnresolvedCFragSymbolAddress;
    Find_Symbol( (Ptr *)&gNSpGlue->NSpDoModalJoinDialog_Proc, "\pNSpDoModalJoinDialog", kNSpDoModalJoinDialog_ProcInfo);
    if ( (Ptr)gNSpGlue->NSpDoModalJoinDialog_Proc == (Ptr)kUnresolvedCFragSymbolAddress) return( false);

//  NSpDoModalHostDialog
    gNSpGlue->NSpDoModalHostDialog_Proc = (UniversalProcPtr)kUnresolvedCFragSymbolAddress;
    Find_Symbol( (Ptr *)&gNSpGlue->NSpDoModalHostDialog_Proc, "\pNSpDoModalHostDialog", kNSpDoModalHostDialog_ProcInfo);
    if ( (Ptr)gNSpGlue->NSpDoModalHostDialog_Proc == (Ptr)kUnresolvedCFragSymbolAddress) return( false);

//  NSpGame_Host
    gNSpGlue->NSpGame_Host_Proc = (UniversalProcPtr)kUnresolvedCFragSymbolAddress;
    Find_Symbol( (Ptr *)&gNSpGlue->NSpGame_Host_Proc, "\pNSpGame_Host", kNSpGame_Host_ProcInfo);
    if ( (Ptr)gNSpGlue->NSpGame_Host_Proc == (Ptr)kUnresolvedCFragSymbolAddress) return( false);

//  NSpGame_Join
    gNSpGlue->NSpGame_Join_Proc = (UniversalProcPtr)kUnresolvedCFragSymbolAddress;
    Find_Symbol( (Ptr *)&gNSpGlue->NSpGame_Join_Proc, "\pNSpGame_Join", kNSpGame_Join_ProcInfo);
    if ( (Ptr)gNSpGlue->NSpGame_Join_Proc == (Ptr)kUnresolvedCFragSymbolAddress) return( false);

//  NSpReleaseAddressReference
    gNSpGlue->NSpReleaseAddressReference_Proc = (UniversalProcPtr)kUnresolvedCFragSymbolAddress;
    Find_Symbol( (Ptr *)&gNSpGlue->NSpReleaseAddressReference_Proc, "\pNSpReleaseAddressReference", kNSpReleaseAddressReference_ProcInfo);
    if ( (Ptr)gNSpGlue->NSpReleaseAddressReference_Proc == (Ptr)kUnresolvedCFragSymbolAddress) return( false);

//  NSpClearMessageHeader
    gNSpGlue->NSpClearMessageHeader_Proc = (UniversalProcPtr)kUnresolvedCFragSymbolAddress;
    Find_Symbol( (Ptr *)&gNSpGlue->NSpClearMessageHeader_Proc, "\pNSpClearMessageHeader", kNSpClearMessageHeader_ProcInfo);
    if ( (Ptr)gNSpGlue->NSpClearMessageHeader_Proc == (Ptr)kUnresolvedCFragSymbolAddress) return( false);

//  NSpMessage_Send
    gNSpGlue->NSpMessage_Send_Proc = (UniversalProcPtr)kUnresolvedCFragSymbolAddress;
    Find_Symbol( (Ptr *)&gNSpGlue->NSpMessage_Send_Proc, "\pNSpMessage_Send", kNSpMessage_Send_ProcInfo);
    if ( (Ptr)gNSpGlue->NSpMessage_Send_Proc == (Ptr)kUnresolvedCFragSymbolAddress) return( false);

//  NSpMessage_Get
    gNSpGlue->NSpMessage_Get_Proc = (UniversalProcPtr)kUnresolvedCFragSymbolAddress;
    Find_Symbol( (Ptr *)&gNSpGlue->NSpMessage_Get_Proc, "\pNSpMessage_Get", kNSpMessage_Get_ProcInfo);
    if ( (Ptr)gNSpGlue->NSpMessage_Get_Proc == (Ptr)kUnresolvedCFragSymbolAddress) return( false);

//  NSpMessage_Release
    gNSpGlue->NSpMessage_Release_Proc = (UniversalProcPtr)kUnresolvedCFragSymbolAddress;
    Find_Symbol( (Ptr *)&gNSpGlue->NSpMessage_Release_Proc, "\pNSpMessage_Release", kNSpMessage_Release_ProcInfo);
    if ( (Ptr)gNSpGlue->NSpMessage_Release_Proc == (Ptr)kUnresolvedCFragSymbolAddress) return( false);

    return( true);
#endif
    return( false); // shouldn't be called if PPC or CFM-68K
}

void NetSprocketGlueCleanup( void)
{
#ifdef kUseCFMGlue

//  NSpInitialize
    if ( (Ptr)gNSpGlue->NSpInitialize_Proc != (Ptr)kUnresolvedCFragSymbolAddress)
        DisposeRoutineDescriptorTrap( gNSpGlue->NSpInitialize_Proc);

//  NSpGame_Dispose
    if ( (Ptr)gNSpGlue->NSpGame_Dispose_Proc != (Ptr)kUnresolvedCFragSymbolAddress)
        DisposeRoutineDescriptorTrap( gNSpGlue->NSpGame_Dispose_Proc);

//  NSpPlayer_GetInfo
    if ( (Ptr)gNSpGlue->NSpPlayer_GetInfo_Proc != (Ptr)kUnresolvedCFragSymbolAddress)
        DisposeRoutineDescriptorTrap( gNSpGlue->NSpPlayer_GetInfo_Proc);

//  NSpProtocol_ExtractDefinitionString
    if ( (Ptr)gNSpGlue->NSpProtocol_ExtractDefinitionString_Proc != (Ptr)kUnresolvedCFragSymbolAddress)
        DisposeRoutineDescriptorTrap( gNSpGlue->NSpProtocol_ExtractDefinitionString_Proc);

//  NSpProtocolList_Append
    if ( (Ptr)gNSpGlue->NSpProtocolList_Append_Proc != (Ptr)kUnresolvedCFragSymbolAddress)
        DisposeRoutineDescriptorTrap( gNSpGlue->NSpProtocolList_Append_Proc);

//  NSpProtocolList_New
    if ( (Ptr)gNSpGlue->NSpProtocolList_New_Proc != (Ptr)kUnresolvedCFragSymbolAddress)
        DisposeRoutineDescriptorTrap( gNSpGlue->NSpProtocolList_New_Proc);

//  NSpProtocolList_Dispose
    if ( (Ptr)gNSpGlue->NSpProtocolList_Dispose_Proc != (Ptr)kUnresolvedCFragSymbolAddress)
        DisposeRoutineDescriptorTrap( gNSpGlue->NSpProtocolList_Dispose_Proc);

//  NSpProtocol_CreateAppleTalk
    if ( (Ptr)gNSpGlue->NSpProtocol_CreateAppleTalk_Proc != (Ptr)kUnresolvedCFragSymbolAddress)
        DisposeRoutineDescriptorTrap( gNSpGlue->NSpProtocol_CreateAppleTalk_Proc);

//  NSpProtocol_CreateIP
    if ( (Ptr)gNSpGlue->NSpProtocol_CreateIP_Proc != (Ptr)kUnresolvedCFragSymbolAddress)
        DisposeRoutineDescriptorTrap( gNSpGlue->NSpProtocol_CreateIP_Proc);

//  NSpProtocolList_GetCount
    if ( (Ptr)gNSpGlue->NSpProtocolList_GetCount_Proc != (Ptr)kUnresolvedCFragSymbolAddress)
        DisposeRoutineDescriptorTrap( gNSpGlue->NSpProtocolList_GetCount_Proc);

//  NSpProtocolList_GetIndexedRef
    if ( (Ptr)gNSpGlue->NSpProtocolList_GetIndexedRef_Proc != (Ptr)kUnresolvedCFragSymbolAddress)
        DisposeRoutineDescriptorTrap( gNSpGlue->NSpProtocolList_GetIndexedRef_Proc);

//  NSpPlayer_GetMyID
    if ( (Ptr)gNSpGlue->NSpPlayer_GetMyID_Proc != (Ptr)kUnresolvedCFragSymbolAddress)
        DisposeRoutineDescriptorTrap( gNSpGlue->NSpPlayer_GetMyID_Proc);

//  NSpPlayer_ReleaseInfo
    if ( (Ptr)gNSpGlue->NSpPlayer_ReleaseInfo_Proc != (Ptr)kUnresolvedCFragSymbolAddress)
        DisposeRoutineDescriptorTrap( gNSpGlue->NSpPlayer_ReleaseInfo_Proc);

//  NSpPlayer_GetRoundTripTime
    if ( (Ptr)gNSpGlue->NSpPlayer_GetRoundTripTime_Proc != (Ptr)kUnresolvedCFragSymbolAddress)
        DisposeRoutineDescriptorTrap( gNSpGlue->NSpPlayer_GetRoundTripTime_Proc);

//  NSpPlayer_GetThruput
    if ( (Ptr)gNSpGlue->NSpPlayer_GetThruput_Proc != (Ptr)kUnresolvedCFragSymbolAddress)
        DisposeRoutineDescriptorTrap( gNSpGlue->NSpPlayer_GetThruput_Proc);

//  NSpDoModalJoinDialog
    if ( (Ptr)gNSpGlue->NSpDoModalJoinDialog_Proc != (Ptr)kUnresolvedCFragSymbolAddress)
        DisposeRoutineDescriptorTrap( gNSpGlue->NSpDoModalJoinDialog_Proc);

//  NSpDoModalHostDialog
    if ( (Ptr)gNSpGlue->NSpDoModalHostDialog_Proc != (Ptr)kUnresolvedCFragSymbolAddress)
        DisposeRoutineDescriptorTrap( gNSpGlue->NSpDoModalHostDialog_Proc);

//  NSpGame_Host
    if ( (Ptr)gNSpGlue->NSpGame_Host_Proc != (Ptr)kUnresolvedCFragSymbolAddress)
        DisposeRoutineDescriptorTrap( gNSpGlue->NSpGame_Host_Proc);

//  NSpGame_Join
    if ( (Ptr)gNSpGlue->NSpGame_Join_Proc != (Ptr)kUnresolvedCFragSymbolAddress)
        DisposeRoutineDescriptorTrap( gNSpGlue->NSpGame_Join_Proc);

//  NSpReleaseAddressReference
    if ( (Ptr)gNSpGlue->NSpReleaseAddressReference_Proc != (Ptr)kUnresolvedCFragSymbolAddress)
        DisposeRoutineDescriptorTrap( gNSpGlue->NSpReleaseAddressReference_Proc);

//  NSpClearMessageHeader
    if ( (Ptr)gNSpGlue->NSpClearMessageHeader_Proc != (Ptr)kUnresolvedCFragSymbolAddress)
        DisposeRoutineDescriptorTrap( gNSpGlue->NSpClearMessageHeader_Proc);

//  NSpMessage_Send
    if ( (Ptr)gNSpGlue->NSpMessage_Send_Proc != (Ptr)kUnresolvedCFragSymbolAddress)
        DisposeRoutineDescriptorTrap( gNSpGlue->NSpMessage_Send_Proc);

//  NSpMessage_Get
    if ( (Ptr)gNSpGlue->NSpMessage_Get_Proc != (Ptr)kUnresolvedCFragSymbolAddress)
        DisposeRoutineDescriptorTrap( gNSpGlue->NSpMessage_Get_Proc);

//  NSpMessage_Release
    if ( (Ptr)gNSpGlue->NSpMessage_Release_Proc != (Ptr)kUnresolvedCFragSymbolAddress)
        DisposeRoutineDescriptorTrap( gNSpGlue->NSpMessage_Release_Proc);
#endif
}

OSStatus Glue_NSpInitialize(
    UInt32                      inStandardMessageSize,
    UInt32                      inBufferSize,
    UInt32                      inQElements,
    NSpGameID                   inGameID,
    UInt32                      inTimeout)
{
#ifndef kUseCFMGlue
    return( NSpInitialize(
            inStandardMessageSize,
            inBufferSize,
            inQElements,
            inGameID,
            inTimeout));
#else
    return( ((NSpInitialize_ProcPtr)gNSpGlue->NSpInitialize_Proc)(
            inStandardMessageSize,
            inBufferSize,
            inQElements,
            inGameID,
            inTimeout));
#endif
}

OSStatus Glue_NSpGame_Dispose(
    NSpGameReference            inGame,
    NSpFlags                    inFlags)
{
#ifndef kUseCFMGlue
    return( NSpGame_Dispose(
            inGame,
            inFlags));
#else
    return( ((NSpGame_Dispose_ProcPtr)gNSpGlue->NSpGame_Dispose_Proc)(
            inGame,
            inFlags));
#endif
}

OSStatus Glue_NSpPlayer_GetInfo(
    NSpGameReference            inGame,
    NSpPlayerID                 inPlayerID,
    NSpPlayerInfoPtr*           outInfo)
{
#ifndef kUseCFMGlue
    return( NSpPlayer_GetInfo(
            inGame,
            inPlayerID,
            outInfo));
#else
    return( ((NSpPlayer_GetInfo_ProcPtr)gNSpGlue->NSpPlayer_GetInfo_Proc)(
            inGame,
            inPlayerID,
            outInfo));
#endif
}

OSStatus Glue_NSpProtocol_ExtractDefinitionString(
    NSpProtocolReference        inProtocolRef,
    char*                       outDefinitionString)
{
#ifndef kUseCFMGlue
    return( NSpProtocol_ExtractDefinitionString(
            inProtocolRef,
            outDefinitionString));
#else
    return( ((NSpProtocol_ExtractDefinitionString_ProcPtr)gNSpGlue->NSpProtocol_ExtractDefinitionString_Proc)(
            inProtocolRef,
            outDefinitionString));
#endif
}

OSStatus Glue_NSpProtocolList_New(
    NSpProtocolReference        inProtocolRef,
    NSpProtocolListReference*   outList)
{
#ifndef kUseCFMGlue
    return( NSpProtocolList_New(
            inProtocolRef,
            outList));
#else
    return( ((NSpProtocolList_New_ProcPtr)gNSpGlue->NSpProtocolList_New_Proc)(
            inProtocolRef,
            outList));
#endif
}

OSStatus Glue_NSpProtocolList_Append(
    NSpProtocolListReference    inProtocolList,
    NSpProtocolReference        inProtocolRef)
{
#ifndef kUseCFMGlue
    return( NSpProtocolList_Append(
            inProtocolList,
            inProtocolRef));
#else
    return( ((NSpProtocolList_Append_ProcPtr)gNSpGlue->NSpProtocolList_Append_Proc)(
            inProtocolList,
            inProtocolRef));
#endif
}

void Glue_NSpProtocolList_Dispose(
    NSpProtocolListReference    inProtocolList)
{
#ifndef kUseCFMGlue
    NSpProtocolList_Dispose(
            inProtocolList);
#else
    ((NSpProtocolList_Dispose_ProcPtr)gNSpGlue->NSpProtocolList_Dispose_Proc)(
            inProtocolList);
#endif
}

NSpProtocolReference Glue_NSpProtocol_CreateAppleTalk(
    ConstStr31Param             inNBPName,
    ConstStr31Param             inNBPType,
    UInt32                      inMaxRTT,
    UInt32                      inMinThruput)
{
#ifndef kUseCFMGlue
    return( NSpProtocol_CreateAppleTalk(
            inNBPName,
            inNBPType,
            inMaxRTT,
            inMinThruput));
#else
    return (((NSpProtocol_CreateAppleTalk_ProcPtr)gNSpGlue->NSpProtocol_CreateAppleTalk_Proc)(
            inNBPName,
            inNBPType,
            inMaxRTT,
            inMinThruput));
#endif
}

NSpProtocolReference Glue_NSpProtocol_CreateIP(
    InetPort                    inPort,
    UInt32                      inMaxRTT,
    UInt32                      inMinThruput)
{
#ifndef kUseCFMGlue
    return( NSpProtocol_CreateIP(
            inPort,
            inMaxRTT,
            inMinThruput));
#else
    return( ((NSpProtocol_CreateIP_ProcPtr)gNSpGlue->NSpProtocol_CreateIP_Proc)(
            inPort,
            inMaxRTT,
            inMinThruput));
#endif
}

UInt32 Glue_NSpProtocolList_GetCount(
    NSpProtocolListReference    inProtocolList)
{
#ifndef kUseCFMGlue
    return( NSpProtocolList_GetCount(
            inProtocolList));
#else
    return( ((NSpProtocolList_GetCount_ProcPtr)gNSpGlue->NSpProtocolList_GetCount_Proc)(
            inProtocolList));
#endif
}

NSpProtocolReference Glue_NSpProtocolList_GetIndexedRef(
    NSpProtocolListReference    inProtocolList,
    UInt32                      inIndex)
{
#ifndef kUseCFMGlue
    return( NSpProtocolList_GetIndexedRef(
            inProtocolList,
            inIndex));
#else
    return( ((NSpProtocolList_GetIndexedRef_ProcPtr)gNSpGlue->NSpProtocolList_GetIndexedRef_Proc)(
            inProtocolList,
            inIndex));
#endif
}

NSpPlayerID Glue_NSpPlayer_GetMyID(
    NSpGameReference            inGame)
{
#ifndef kUseCFMGlue
    return( NSpPlayer_GetMyID(
            inGame));
#else
    return( ((NSpPlayer_GetMyID_ProcPtr)gNSpGlue->NSpPlayer_GetMyID_Proc)(
            inGame));
#endif
}

void Glue_NSpPlayer_ReleaseInfo(
    NSpGameReference            inGame,
    NSpPlayerInfoPtr            inInfo)
{
#ifndef kUseCFMGlue

    NSpPlayer_ReleaseInfo(
            inGame,
            inInfo);
#else
    (( NSpPlayer_ReleaseInfo_ProcPtr)gNSpGlue->NSpPlayer_ReleaseInfo_Proc)(
            inGame,
            inInfo);
#endif
}

UInt32 Glue_NSpPlayer_GetRoundTripTime(
    NSpGameReference            inGame,
    NSpPlayerID                 inPlayer)
{
#ifndef kUseCFMGlue
    return( NSpPlayer_GetRoundTripTime(
            inGame,
            inPlayer));
#else
    return( ((NSpPlayer_GetRoundTripTime_ProcPtr)gNSpGlue->NSpPlayer_GetRoundTripTime_Proc)(
            inGame,
            inPlayer));
#endif
}

UInt32 Glue_NSpPlayer_GetThruput(
    NSpGameReference            inGame,
    NSpPlayerID                 inPlayer)
{
#ifndef kUseCFMGlue
    return( NSpPlayer_GetThruput(
            inGame,
            inPlayer));
#else
    return( ((NSpPlayer_GetThruput_ProcPtr)gNSpGlue->NSpPlayer_GetThruput_Proc)(
            inGame,
            inPlayer));
#endif
}

NSpAddressReference Glue_NSpDoModalJoinDialog(
    ConstStr31Param             inGameType,
    ConstStr255Param            inEntityListLabel,
    Str31                       ioName,
    Str31                       ioPassword,
    NSpEventProcPtr             inEventProcPtr)
{
#ifndef kUseCFMGlue
    return( GRNSpDoModalJoinDialog(
            inGameType,
            inEntityListLabel,
            ioName,
            ioPassword,
            inEventProcPtr));
//  return( NSpDoModalJoinDialog(
//          inGameType,
//          inEntityListLabel,
//          ioName,
//          ioPassword,
//          inEventProcPtr));
#else
    return( ((NSpDoModalJoinDialog_ProcPtr)gNSpGlue->NSpDoModalJoinDialog_Proc)(
            inGameType,
            inEntityListLabel,
            ioName,
            ioPassword,
            inEventProcPtr));
#endif
}

Boolean Glue_NSpDoModalHostDialog(
    NSpProtocolListReference    ioProtocolList,
    Str31                       ioGameName,
    Str31                       ioPlayerName,
    Str31                       ioPassword,
    NSpEventProcPtr             inEventProcPtr)
{
#ifndef kUseCFMGlue
        return( GRNSpDoModalHostDialog(
                ioProtocolList,
                ioGameName,
                ioPlayerName,
                ioPassword,
                inEventProcPtr));
//      return( NSpDoModalHostDialog(
//              ioProtocolList,
//              ioGameName,
//              ioPlayerName,
//              ioPassword,
//              inEventProcPtr));
#else
    return( ((NSpDoModalHostDialog_ProcPtr)gNSpGlue->NSpDoModalHostDialog_Proc)(
            ioProtocolList,
            ioGameName,
            ioPlayerName,
            ioPassword,
            inEventProcPtr));
#endif
}

OSStatus Glue_NSpGame_Host(
    NSpGameReference*           outGame,
    NSpProtocolListReference    inProtocolList,
    UInt32                      inMaxPlayers,
    ConstStr31Param             inGameName,
    ConstStr31Param             inPassword,
    ConstStr31Param             inPlayerName,
    NSpPlayerType               inPlayerType,
    NSpTopology                 inTopology,
    NSpFlags                    inFlags)
{
#ifndef kUseCFMGlue
    return( NSpGame_Host(
            outGame,
            inProtocolList,
            inMaxPlayers,
            inGameName,
            inPassword,
            inPlayerName,
            inPlayerType,
            inTopology,
            inFlags));
#else
    return( ((NSpGame_Host_ProcPtr)gNSpGlue->NSpGame_Host_Proc)(
            outGame,
            inProtocolList,
            inMaxPlayers,
            inGameName,
            inPassword,
            inPlayerName,
            inPlayerType,
            inTopology,
            inFlags));
#endif
}

OSStatus Glue_NSpGame_Join(
    NSpGameReference*           outGame,
    NSpAddressReference         inAddress,
    ConstStr31Param             inName,
    ConstStr31Param             inPassword,
    NSpPlayerType               inType,
    void*                       inCustomData,
    UInt32                      inCustomDataLen,
    NSpFlags                    inFlags)
{
#ifndef kUseCFMGlue
    return( NSpGame_Join(
            outGame,
            inAddress,
            inName,
            inPassword,
            inType,
            inCustomData,
            inCustomDataLen,
            inFlags));
#else
    return( ((NSpGame_Join_ProcPtr)gNSpGlue->NSpGame_Join_Proc)(
            outGame,
            inAddress,
            inName,
            inPassword,
            inType,
            inCustomData,
            inCustomDataLen,
            inFlags));
#endif
}

void Glue_NSpReleaseAddressReference(
    NSpAddressReference         inAddress)
{
#ifndef kUseCFMGlue
    NSpReleaseAddressReference(
            inAddress);
#else
    ((NSpReleaseAddressReference_ProcPtr)gNSpGlue->NSpReleaseAddressReference_Proc)(
            inAddress);
#endif
}

void Glue_NSpClearMessageHeader(
    NSpMessageHeader*           inMessage)
{
#ifndef kUseCFMGlue
    NSpClearMessageHeader(
            inMessage);
#else
    ((NSpClearMessageHeader_ProcPtr)gNSpGlue->NSpClearMessageHeader_Proc)(
            inMessage);
#endif
}

OSStatus Glue_NSpMessage_Send(
    NSpGameReference            inGame,
    NSpMessageHeader*           inMessage,
    NSpFlags                    inFlags)
{
#ifndef kUseCFMGlue
    return( NSpMessage_Send(
            inGame,
            inMessage,
            inFlags));
#else
    return( ((NSpMessage_Send_ProcPtr)gNSpGlue->NSpMessage_Send_Proc)(
            inGame,
            inMessage,
            inFlags));
#endif
}

NSpMessageHeader *Glue_NSpMessage_Get(
    NSpGameReference            inGame)
{
#ifndef kUseCFMGlue
    return( NSpMessage_Get(
            inGame));
#else
    return( ((NSpMessage_Get_ProcPtr)gNSpGlue->NSpMessage_Get_Proc)(
            inGame));
#endif
}

void Glue_NSpMessage_Release(
    NSpGameReference            inGame,
    NSpMessageHeader*           inMessage)
{
#ifndef kUseCFMGlue
    NSpMessage_Release(
            inGame,
            inMessage);
#else
    ((NSpMessage_Release_ProcPtr)gNSpGlue->NSpMessage_Release_Proc)(
            inGame,
            inMessage);
#endif
}

/*
 **     PRIVATE FUNCTIONS
\**/

static OSErr Find_Symbol(Ptr* pSymAddr,
                         Str255 pSymName,
                         ProcInfoType pProcInfo)
{
    static CFragConnectionID sCID = 0;
    static OSType sArchType = kAnyCFragArch;
    static OSErr sOSErr = noErr;

    Str255 errMessage;
    Ptr mainAddr;
    CFragSymbolClass symClass;
    ISAType tISAType;

    if (sArchType == kAnyCFragArch)             // if architecture is still undefined...
    {
        sCID = 0;                               // ...force (re)connect to library
        sOSErr = GetSystemArchitecture(&sArchType); // & determine current atchitecture.
        if (sOSErr != noErr)
            return sOSErr;
    }
    switch (sArchType)
    {
        case    kMotorola68KCFragArch:
            tISAType = kM68kISA | kCFM68kRTA;
            break;
        case    kPowerPCCFragArch:
            tISAType = kPowerPCISA | kPowerPCRTA;
            break;
        default:    // who knows what might be next?
            sOSErr = gestaltUnknownErr;
            break;
    }

    if (sCID == 0)                              // If we haven't connected to the library yet...
    {
        sOSErr = GetSharedLibrary("\pNetSprocketLib",sArchType,kLoadCFrag,&sCID,&mainAddr,errMessage);
        if (sOSErr != noErr)
            return sOSErr;
    }

    // If we haven't looked up this symbol yet...
    if ((Ptr) * pSymAddr == (Ptr) kUnresolvedCFragSymbolAddress)
    {
        // ...look it up now
        sOSErr = FindSymbol(sCID,pSymName,pSymAddr,&symClass);
        if (sOSErr != noErr)    // in case of error...
            *(Ptr*) &pSymAddr = (Ptr) kUnresolvedCFragSymbolAddress;    // ...clear the procedure pointer
#if !GENERATINGCFM  // if this is classic 68k code...
        else        // ...create a routine descriptor...
            *pSymAddr = (Ptr)NewRoutineDescriptorTrap((ProcPtr) * pSymAddr,pProcInfo,tISAType);
#endif
    }
    return sOSErr;
}

static pascal OSErr GetSystemArchitecture(OSType *archType)
{
    static long sSysArchitecture = 0;   // static so we only Gestalt the first time
    OSErr   tOSErr = noErr;

    *archType = kAnyCFragArch;          // assume wild architecture

    // If we don't know the system architecture yet...
    if (sSysArchitecture == 0)
        // ...Ask Gestalt what kind of machine we are running on.
        tOSErr = Gestalt(gestaltSysArchitecture, &sSysArchitecture);

    if (tOSErr == noErr)    // if no errors
    {
        if (sSysArchitecture == gestalt68k)             // 68k?
            *archType = kMotorola68KCFragArch;
        else if (sSysArchitecture == gestaltPowerPC)    // PPC?
            *archType = kPowerPCCFragArch;
        else
            tOSErr = gestaltUnknownErr;                 // who knows what might be next?
    }
    return tOSErr;
}

#ifndef powerc
#pragma mpwc reset
#endif
