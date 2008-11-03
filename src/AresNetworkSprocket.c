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

/* Ares NetworkSprocket.c */

//#ifdef powerc
#define kAllowNetSprocket
//#endif

//#ifdef __CFM68K__
//#define kAllowNetSprocket
//#endif

#include "Wrap_GameRanger.h"

#include "Speech.h"
#include "Ares Global Type.h"
#include "Error.h"
#include "Debug.h"
#include "String Handling.h"
#include "Randomize.h"
#include "Ares Preferences.h"
#include "Ares Cheat.h"
#include "Options.h"

#include "Key Map Translation.h"

#ifndef kSpaceObject
#include "Space Object.h"
#endif
#include "Scenario Maker.h"
#include "Key Codes.h"
#include "Admiral.h"
#include "Minicomputer.h"
#include "Player Ship.h"
#include "Message Screen.h"

#include "NetSprocketGlue.h"
#include "Ares NetworkSprocket.h"
#include "Processor.h"

#define kDebugFileName      "\p_Debug Ares "
#define kAresNetworkError   "\pANET"
#ifdef kBackupData2
#define kMaxMessageQueueLen     (kMaxMessageLatency * 6)    //15
#define kLatencyQueueLen        (kMaxMessageLatency * 12)   //30
#else
    #ifdef kBackupData
#define kMaxMessageQueueLen     (kMaxMessageLatency * 4)    //15
#define kLatencyQueueLen        (kMaxMessageLatency * 4)    //30
    #else
#define kMaxMessageQueueLen     (kMaxMessageLatency * 2)    //15
#define kLatencyQueueLen        (kMaxMessageLatency * 2)    //30
    #endif
#endif

#define kRoundTripSampleNum     30
#define kLatencySampleNum       30

#define kNBPType            'ar12'

#define kNSpAppleTalkDefString  "\patlk"    // hack we use for determining if appletalk was selected
#define kNSpTCPIPDefString      "\pinet"    // hack we use for determining if tcp/ip selected
#define kNSpDefStringIDLen      4

#define kShortAdmiralFlag       0x40000000  // we use this unused character flag to designate
                                            // admiral # 1 or # 2

//#define   kPlayerNumMask          0x0000ffff
//#define   kRandomSeedSynchMask    0xffff0000
#define kTextMessageLength      512

#define kRandomSeedSynchMask    0xf8000000
#define kSynchBitShift          (unsigned long)27
#define kWhichPageMask          0x07c00000
#define kWhichPageBitShift      (unsigned long)22
#define kWhichLineMask          0x003c0000
#define kWhichLineBitShift      (unsigned long)18
#define kGameTimeMask           0x0003ffff  // max game time is 262143

#define kAdmiralMask            0xc0000000
#define kAdmiralBitShift        (unsigned long)30
#define kIsTargetMask           0x20000000
#define kWhichShipMask          0x1fe00000
#define kWhichShipBitShift      (unsigned long)21
#define kKeyStateMask           0x001fffff

#define kTextMessageCharacterBits   0x00380000
/*
overrideWhichship = false;
if ((packedData1 & kWhichPageMask) != kWhichPageMask)
{
    we have a page & a line number to deal with
} else
{
    we might have something else
    if ( ( packedData1 & kWhichLineMask) != kWhichLineMask)
    {
        we have something special
        special = (packeData1 & kWhichLineMask) >> kWhichLineBitShift;
        if ( special == 14)
        {
            we have a character part of a message
            overrideWhichship = true;
            whatCharacter = (packedData2 & kWhichShipMask) >> kWhichShipBitShift;
        } else
        {
            we have a cheat #
        }
    }
}
if ( !overrideWhichship) selectShipNum = (packedData2 & kWhichShipMask) >> kWhichShipBitShift
*/

#define kCorruptNetDataError    66

typedef enum
{
        kSlacking,
        kHosting,
        kJoining,
        kWaiting,
        kStarting,
        kRunning,
        kDenoument
} NetworkState;

typedef struct
{
    packedDataType              data;
    long                        next;
    Boolean                     used;
} latencyQueueNodeType;

typedef struct
{
    long                        id;
    Str31                       name;
    Boolean                     exists;
    short                       race;
    unsigned char               color;
} playerIDType;

typedef struct
{
    NSpGameReference            netGame;
    NSpAddressReference         address;        // temporarily held
    Str31                       gameName;
    Str31                       playerName;
    Str31                       password;
    NetworkState                netState;
    unsigned long               protocolFlags;
    short                       playerNum;
    playerIDType                playerID[kMaxNetPlayerNum];

    // outgoing data -- sent all at once now
    long                        pageNum;
    long                        lineNum;
    long                        whichCheat;
    long                        whichShip;
    Boolean                     target;

    unsigned long               keysDown[kMaxNetPlayerNum];
    latencyQueueNodeType        latencyQueue[kLatencyQueueLen];
    packedDataType              sentMessage[kLatencyQueueLen];
    long                        pregamePreviousMessage;
//#ifdef kBackupData
    packedDataType              backupData;
//#endif
//#ifdef kBackupData2
    packedDataType              backupData2;
//#endif
    long                        queueTop;
    long                        resendDelay;
    long                        registeredSetting;
    unsigned long               registeredFlags;
    long                        calcLatency;
    long                        nextLatency;
    unsigned long               latencySample;
    long                        latencySampleCount;
    long                        preserveSeed;
    Boolean                     inSynch;
    unsigned long               sanityCheckTime;
    unsigned long               lastKeysSent;

    // data to be used -- not executed until all is received
    unsigned long               theseKeys[kMaxNetPlayerNum];
    long                        thisSelectNum[kMaxNetPlayerNum];
    short                       thisMenuPage[kMaxNetPlayerNum];
    short                       thisMenuLine[kMaxNetPlayerNum];
    short                       thisCheat[kMaxNetPlayerNum];

    anyCharType                 incomingMessage[kTextMessageLength + 1];
    anyCharType                 outgoingMessage[kTextMessageLength + 1];
    short                       incomingCharNum;
    short                       outgoingCharNum;
    short                       lastOutgoingCharNum;

    unsigned short              minutesPlayed;
    unsigned short              netKills;
    unsigned short              netLosses;
    short                       netRace;
    short                       netEnemyColor;
    short                       netLevel;
    Boolean                     opponentIsUnregistered;
    Boolean                     haveSeenUnregisteredTimeWarning;

    Boolean                     thisSelectIsTarget[kMaxNetPlayerNum];
    Boolean                     gotMessage[kMaxNetPlayerNum];
    Boolean                     hosting;
    Boolean                     gotEndGameMessage;
    Boolean                     haveEncounteredSynchError;
    long                        netSynchBarfCountDown;
} netDataType;

typedef struct
{
    NSpMessageHeader            header;
    long                        gameTime;
    long                        shortData;
} shortMessageType;

typedef struct
{
    NSpMessageHeader            header;
    long                        gameTime;
    long                        data1;
    long                        data2;
    long                        data3;
    long                        data4;
} verbosePreGameMessageType;

typedef struct
{
    NSpMessageHeader            header;
    Str255                      fileName;
    Str255                      url;
    unsigned long               version;
    unsigned long               checkSum;
} openScenarioPreGameMessageType;

#define kMessageBufferSize      ((sizeof( messageDataType) * kMaxMessageQueueLen * kMaxNetPlayerNum) + 50000)

extern aresGlobalType *gAresGlobal;
extern long     /*gAresGlobal->gThisScenarioNumber, gAresGlobal->gPlayerAdmiralNumber,*/
                gRandomSeed/*, gAresGlobal->gGameTime*/;

netDataType     *gNetData = nil;
long            gNetLatency;

Boolean AddIncomingTextMessageLong( unsigned long, unsigned long);
void AddIncomingTextMessageCharacter( unsigned char);
long GetOtherPlayerNum( void);

Boolean NetSprocketPresent(void)
{
#ifdef kAllowNetSprocket
 #ifdef kUseCFMGlue
    if ( !NetSprocketGlueInit()) return( false);
    else return true;
 #else
    return (NSpInitialize != nil);
 #endif
#else
    return( false);
#endif
}

short InitNetworking(void)
{
    OSStatus    err = noErr;

//  DebugStr("\pInitNetworking");

    if (! NetSprocketPresent())
    {
//      ShowErrorRecover(RESOURCE_ERROR, kAresNetworkError, 1);
        return ( -1);
    }

    gNetData = (netDataType *)NewPtr( sizeof( netDataType));


    if ( gNetData == nil)
    {
        ShowErrorAny( eContinueOnlyErr, kErrorStrID, nil, nil, nil, nil, MEMORY_ERROR, -1, -1, -1, __FILE__, 2);
        return ( -1);
    }

    gNetData->netGame = nil;
    gNetData->netState = kSlacking;
    gNetData->protocolFlags = 0;
    gNetData->resendDelay = 60;
    gNetData->registeredSetting = 1;
    gNetData->registeredFlags = kRegisterResendRequest | kRegisterResend;
    CopyPString( (unsigned char *)gNetData->gameName, (unsigned char *)"\pGame Name");
    CopyPString( (unsigned char *)gNetData->playerName, (unsigned char *)"\pPlayer Name");
    CopyPString( (unsigned char *)gNetData->password, (unsigned char *)"\p");
//  DebugStr("\pBeginNetworking");
    err = BeginNetworking();
//  DebugStr("\pReturning from InitNetworking");
    return( err);
}

short BeginNetworking( void)

{
    OSStatus    status;

#ifdef kAllowNetSprocket
//  DebugStr("\pClearNetData");
    ClearNetData();
    // *    Initialize NetSprocket
//  DebugStr("\pInitialize NetSprocket");
    status = Glue_NSpInitialize( sizeof( messageDataType), kMessageBufferSize,
                kMaxMessageQueueLen, kNBPType, 0);
    if (status != noErr)
    {
        switch( status)
        {
            case kNSpOTNotPresentErr:
                ShowErrorAny( eContinueOnlyErr, kErrorStrID, nil, nil, nil, nil,
                    kOpenTransportError, -1, -1, -1, __FILE__, 2);
                break;

            case kNSpMemAllocationErr:
                ShowErrorAny( eContinueOnlyErr, kErrorStrID, nil, nil, nil, nil,
                    kNSpMemError, -1, -1, -1, __FILE__, 2);
                break;

            default:
                ShowErrorOfTypeOccurred( eContinueOnlyErr, kErrorStrID, kInitNetSprocketError, status, __FILE__, 0);
                break;
        }

        return (status);
    }
#endif
    return ( noErr);

}

void ClearNetData( void)
{
    short   i, shortLatency;

    gNetData->netGame = nil;
    gNetData->hosting = false;
    gNetData->netState = kSlacking;
    gNetData->gotEndGameMessage = false;
    gNetData->playerNum = 0;
    gNetData->queueTop = -1;
    gNetData->nextLatency = gNetData->calcLatency = gNetLatency = kMessageLatency;
    gNetData->latencySampleCount = kLatencySampleNum;
    gNetData->latencySample = 0;
    gNetData->preserveSeed = 0;
    gNetData->inSynch = true;
    gNetData->sanityCheckTime = -1;
    gNetData->lastKeysSent = 0;
    gNetData->incomingCharNum = 0;
    gNetData->outgoingCharNum = gNetData->lastOutgoingCharNum = 0;
    gNetData->outgoingMessage[0] = 0;
    gNetData->incomingMessage[0] = 0;

//#ifdef kBackupData
    gNetData->backupData.packedData1 = gNetData->backupData.packedData2 = 0xffffffff;
//#endif
//#ifdef kBackupData2
    gNetData->backupData2.packedData1 = gNetData->backupData2.packedData2 = 0xffffffff;
//#endif
    for ( i = 0; i < kLatencyQueueLen; i++)
    {
        gNetData->latencyQueue[i].next = -1;
        gNetData->latencyQueue[i].used = false;
    }

    for ( i = 0; i < kMaxNetPlayerNum; i++)
    {
        gNetData->gotMessage[i] = false;
//      gNetData->hasMessageFlag[i] = gNetData->gotMessageFlag[i] = 0;
        gNetData->playerID[i].exists = false;
        gNetData->theseKeys[i] = 0;
        gNetData->playerID[i].color = 0;
        gNetData->playerID[i].race = -1;
        gNetData->pageNum = -1;
        gNetData->lineNum = -1;
        gNetData->whichCheat = -1;
        gNetData->whichShip = -1;
        gNetData->target = false;
    }

    GetNetPreferences( gNetData->playerName, gNetData->gameName,
        &gNetData->protocolFlags, &gNetData->resendDelay,
        &gNetData->registeredSetting, &gNetData->registeredFlags,
        &shortLatency, &gNetData->minutesPlayed, &gNetData->netKills, &gNetData->netLosses,
        &gNetData->netRace, &gNetData->netEnemyColor, &gNetData->netLevel);
    gNetData->opponentIsUnregistered = true;
    gNetData->haveSeenUnregisteredTimeWarning = false;
    gNetLatency = shortLatency;
    gNetData->haveEncounteredSynchError = false;
    gNetData->netSynchBarfCountDown = -1;
    ResetGotMessages( 0x7fffffff);
    ResetSentMessages();
}

void ResetNetData( void)
{
    // assumes you have a netgame you want to restart, but not create

    short   i;

    gNetData->gotEndGameMessage = false;
    gNetData->queueTop = -1;
    gNetData->latencySampleCount = kLatencySampleNum;
    gNetData->latencySample = 0;
    gNetData->preserveSeed = 0;
    gNetData->inSynch = true;
    gNetData->sanityCheckTime = -1;
    gNetData->lastKeysSent = 0;
    gNetData->incomingCharNum = 0;
    gNetData->outgoingCharNum = gNetData->lastOutgoingCharNum = 0;
    gNetData->outgoingMessage[0] = 0;
    gNetData->incomingMessage[0] = 0;

//#ifdef kBackupData
    gNetData->backupData.packedData1 = gNetData->backupData.packedData2 = 0xffffffff;
//#endif
//#ifdef kBackupData2
    gNetData->backupData2.packedData1 = gNetData->backupData2.packedData2 = 0xffffffff;
//#endif
    for ( i = 0; i < kLatencyQueueLen; i++)
    {
        gNetData->latencyQueue[i].next = -1;
        gNetData->latencyQueue[i].used = false;
    }

    for ( i = 0; i < kMaxNetPlayerNum; i++)
    {
        gNetData->gotMessage[i] = false;
        gNetData->theseKeys[i] = 0;
        gNetData->pageNum = -1;
        gNetData->lineNum = -1;
        gNetData->whichCheat = -1;
        gNetData->whichShip = -1;
        gNetData->target = false;
    }

    ResetGotMessages( 0x7fffffff);
    ResetSentMessages();
}

void StopNetworking( void)
{
#ifdef kAllowNetSprocket
    OSStatus    status;
    short       shortLatency = gNetLatency;

    if ( gNetData != nil)
    {
        if ( gAresGlobal->gameRangerInProgress)
        {
            if ( IAmHosting())
            {
                WriteDebugLine((char *)"\pGRHostClosed");
                Wrap_GRHostClosed();
            }

//          Wrap_GRReset();
            gAresGlobal->gameRangerPending = false;

            gAresGlobal->gameRangerInProgress = false;
        }

        if (gNetData->netGame)
        {
            status = Glue_NSpGame_Dispose( gNetData->netGame, kNSpGameFlag_ForceTerminateGame );


        }

        gNetData->netState = kSlacking;
        gNetData->netGame = nil;
    }
    SaveNetPreferences( gNetData->playerName, gNetData->gameName,
        gNetData->protocolFlags, gNetData->resendDelay,
        gNetData->registeredSetting, gNetData->registeredFlags,
        shortLatency, gNetData->minutesPlayed, gNetData->netKills, gNetData->netLosses,
        gNetData->netRace, gNetData->netEnemyColor, gNetData->netLevel);
#endif
}

void DisposeNetworking( void)
{
    if ( gNetData != nil)
    {
        StopNetworking();
        NetSprocketGlueCleanup();
        DisposePtr( (Ptr)gNetData);
    }
}

long GetResendDelay( void)
{
    return( gNetData->resendDelay);
}

void SetResendDelay( long delay)
{
    gNetData->resendDelay = delay;
}

long GetRegisteredSetting( void)
{
    return( gNetData->registeredSetting);
}

void SetRegisteredSetting( long setting)
{
    gNetData->registeredSetting = setting;

    gNetData->registeredFlags &= kLowerBandwidth;

    switch( setting)
    {
        case 2: // the highest setting
            gNetData->registeredFlags |= kRegisterNoChange;
            gNetData->registeredFlags |= kRegisterStandard;
            // no break!

            gNetData->registeredFlags |= kRegisterMenu;
            gNetData->registeredFlags |= kRegisterSelect;
            // no break!

        case 1:
            gNetData->registeredFlags |= kRegisterResend;
            gNetData->registeredFlags |= kRegisterResendRequest;
            break;

        case 0:
            // don't set any flags
            break;
    }
}

Boolean GetBandwidth( void)
{
    if ( gNetData->registeredFlags & kLowerBandwidth) return ( true); // lower bandwidth yes
    else return( false);
}

void SetBandwidth( Boolean setting)
{
    if ( setting)
        gNetData->registeredFlags |= kLowerBandwidth;
    else
        gNetData->registeredFlags &= ~kLowerBandwidth;
}

long GetNumberOfPlayers( void)
{
    return( gNetData->playerNum);
}

unsigned short GetNetMinutesPlayed( void)
{
    return( gNetData->minutesPlayed);
}

void SetNetMinutesPlayed( unsigned short minutesPlayed)
{
    gNetData->minutesPlayed = minutesPlayed;
}

unsigned short GetNetKills( void)
{
    return( gNetData->netKills);
}

void SetNetKills( unsigned short kills)
{
    gNetData->netKills = kills;
}

unsigned short GetNetLosses( void)
{
    return( gNetData->netLosses);
}

void SetNetLosses( unsigned short losses)
{
    gNetData->netLosses = losses;
}

short GetNetRace( void)
{
    return( gNetData->netRace);
}

void SetNetRace( short race)
{
    gNetData->netRace = race;
}

short GetNetEnemyColor( void)
{
    return( gNetData->netEnemyColor);
}

void SetNetEnemyColor( short color)
{
    gNetData->netEnemyColor = color;
}

short GetNetLevel( void)
{
    return( gNetData->netLevel);
}

void SetNetLevel( short level)
{
    gNetData->netLevel = level;
}

Boolean GetOpponentIsUnregistered( void)
{
    return( gNetData->opponentIsUnregistered);
}

void SetOpponentIsUnregistered( Boolean unregistered)
{
    gNetData->opponentIsUnregistered = unregistered;
}

Boolean GetHaveSeenUnregisteredTimeLimitWarning( void)
{
    return( gNetData->haveSeenUnregisteredTimeWarning);
}

void SetHaveSeenUnregisteredTimeLimitWarning( Boolean seenIt)
{
    gNetData->haveSeenUnregisteredTimeWarning = seenIt;
}

Boolean GetAllNetPlayersCheating( void)
{
    Boolean result = true;
    long    i;

    if ( !NetGameIsOn()) return( true);
    for ( i = 0; i < kMaxNetPlayerNum; i++)
    {
        mWriteDebugString("\pCheck Player #");
        WriteDebugLong( i);

        if (!( gAresGlobal->gActiveCheats[i] & kCheatActiveBit))
        {
            result = false;
            mWriteDebugString("\pNOT CHEATING");
        } else
        {
            mWriteDebugString("\pCheating.");
        }
    }

    return( result);
}

Boolean NetGameIsOn( void)
{
    if ( gNetData == nil) return false; // if no NSp, no net game
    return ( gNetData->netGame != nil);
}

void AddPlayerID( long id, long whichPlayer)
{
#ifdef kAllowNetSprocket
    NSpPlayerInfo   *playerInfo;
    OSStatus        status;

    if ( whichPlayer < kMaxNetPlayerNum)
    {
        gNetData->playerID[whichPlayer].exists = true;
        gNetData->playerID[whichPlayer].id = id;
        status = Glue_NSpPlayer_GetInfo( gNetData->netGame, id, &playerInfo);
        if ( status == noErr)
        {
            CopyPString( (unsigned char *)gNetData->playerID[whichPlayer].name,
                (unsigned char *)playerInfo->name);
            Glue_NSpPlayer_ReleaseInfo( gNetData->netGame, playerInfo);
        } else
        {
            mWriteDebugString("\pAdd Player *ERR1*");
        }
        mWriteDebugString("\pAdd Player");
        mWriteDebugString( gNetData->playerID[whichPlayer].name);
//      WriteDebugLong( whichPlayer);
    }
#endif
}

long GetPlayerNumFromID( long id)

{
    long    whichPlayer = 0;

    while ((( gNetData->playerID[whichPlayer].id != id)
            || ( gNetData->playerID[whichPlayer].exists == false))
            && ( whichPlayer < kMaxNetPlayerNum))
        whichPlayer++;

    if ( whichPlayer < kMaxNetPlayerNum)
    {
        return( whichPlayer);
    } else return( -1);
}

long GetPlayerIDFromNum( long num)
{
    if (( num < kMaxNetPlayerNum) && ( num >= 0))
    {
        if ( gNetData->playerID[num].exists)
            return( gNetData->playerID[num].id);
    }
    return( -1);
}

long MyPlayerID( void)
{
#ifdef kAllowNetSprocket
    if ( gNetData->netGame != nil)
    {
        return( Glue_NSpPlayer_GetMyID( gNetData->netGame));
    }
    else return( -1);
#else
    return( -1);
#endif
}

void RemovePlayerID( long id)

{
    long    whichPlayer = GetPlayerNumFromID( id);

    if ( whichPlayer >= 0)
    {
        gNetData->playerID[whichPlayer].exists = false;
        gNetData->playerID[whichPlayer].id = -1;
    }
}

void SetPlayerRace( long whichPlayer, short race)
{
    gNetData->playerID[whichPlayer].race = race;
}

short GetPlayerRace( long whichPlayer)
{
    return( gNetData->playerID[whichPlayer].race);
}

void SetPlayerColor( long whichPlayer, unsigned char color)
{
    gNetData->playerID[whichPlayer].color = color;
}

unsigned char GetPlayerColor( long whichPlayer)
{
    return( gNetData->playerID[whichPlayer].color);
}

StringPtr GetPlayerName( long whichPlayer)
{
    if ( whichPlayer < 0)
        return( gNetData->playerName);
    else
        return( gNetData->playerID[whichPlayer].name);
}

long GetOtherPlayerNum( void)
{
    long    whichPlayer = 0;
#ifdef kAllowNetSprocket

    while (((gNetData->playerID[whichPlayer].exists == false) ||
            ( gNetData->playerID[whichPlayer].id ==
            Glue_NSpPlayer_GetMyID( gNetData->netGame))) &&
            ( whichPlayer < kMaxNetPlayerNum))
    {
        whichPlayer++;
    }
#endif
    if ( whichPlayer < kMaxNetPlayerNum)
        return( whichPlayer);
    else return ( -1);
}

/* GetOtherPlayerConnectionData:
Assumes there are only two players. Returns the lag & throughput of other player.
*/

void GetOtherPlayerConnectionData( unsigned long *roundTripTime, unsigned long *throughPut)
{
#ifdef kAllowNetSprocket
    long    whichPlayer = 0;

    while (((gNetData->playerID[whichPlayer].exists == false) ||
            ( gNetData->playerID[whichPlayer].id ==
            Glue_NSpPlayer_GetMyID( gNetData->netGame))) &&
            ( whichPlayer < kMaxNetPlayerNum))
    {
        whichPlayer++;
    }

    if ( whichPlayer < kMaxNetPlayerNum)
    {
        *roundTripTime = Glue_NSpPlayer_GetRoundTripTime( gNetData->netGame, gNetData->playerID[whichPlayer].id);
        *throughPut = Glue_NSpPlayer_GetThruput( gNetData->netGame, gNetData->playerID[whichPlayer].id);
    } else
    {
        *roundTripTime = -1;
        *throughPut = -1;
    }
#endif
}

/* GetOtherPlayerName:
Assumes there are only two players. name of other player.
*/

void GetOtherPlayerName( StringPtr *s)
{
#ifdef kAllowNetSprocket
    long    whichPlayer = 0;

    while (((gNetData->playerID[whichPlayer].exists == false) ||
            ( gNetData->playerID[whichPlayer].id ==
                Glue_NSpPlayer_GetMyID( gNetData->netGame))) &&
            ( whichPlayer < kMaxNetPlayerNum))
    {
        whichPlayer++;
    }

    if ( whichPlayer < kMaxNetPlayerNum)
    {
        *s = gNetData->playerID[whichPlayer].name;
    } else
    {
        *s = nil;
    }
#endif
}

void SetProtocolListFromFlags( NSpProtocolListReference *theList, unsigned long flags)
{
#ifdef kAllowNetSprocket
    NSpProtocolReference        atRef, ipRef;
    OSStatus                    status = noErr;

    status = Glue_NSpProtocolList_New(NULL, theList);
    if ( status != noErr)
    {
        ShowErrorOfTypeOccurred( eContinueOnlyErr, kErrorStrID, kNSpProtocolListError, status, __FILE__, 3);
    }

    if ( flags & kProtocolAppleTalkFlag)
    {
        atRef = Glue_NSpProtocol_CreateAppleTalk( gNetData->gameName, "\par12", 0, 0);
        status = Glue_NSpProtocolList_Append( *theList, atRef);
        if ( status != noErr)
        {
            ShowErrorOfTypeOccurred( eContinueOnlyErr, kErrorStrID, kNSpProtocolATError, status, __FILE__, 31);
        }
    }

    if ( flags & kProtocolTCPIPFlag)
    {
        ipRef = Glue_NSpProtocol_CreateIP( 26370, 0, 0);
        status = Glue_NSpProtocolList_Append( *theList, ipRef);
        if ( status != noErr)
        {
            ShowErrorOfTypeOccurred( eContinueOnlyErr, kErrorStrID, kNSpProtocolTCPIPError, status, __FILE__, 32);
        }
    }
#endif
}

void GetProtocolFlagsFromList( NSpProtocolListReference theList, unsigned long *flags)
{
#ifdef kAllowNetSprocket
    long                    num = Glue_NSpProtocolList_GetCount( theList), i, j;
    NSpProtocolReference    tRef;
    Ptr                     definitionString = NewPtr( kNSpMaxDefinitionStringLen);
    char                    *c;

    *flags = 0;

    if ( definitionString == nil) return;

    for ( i = 0; i < num; i++)
    {
        tRef = Glue_NSpProtocolList_GetIndexedRef( theList, i);
        c = definitionString;
        for ( j = 0; j < kNSpMaxDefinitionStringLen; j++)
        {
            *c = 0;
            c++;
        }
        Glue_NSpProtocol_ExtractDefinitionString( tRef, definitionString);
        if ( IsIDInDefString( definitionString, (char *)kNSpAppleTalkDefString))
            *flags |= kProtocolAppleTalkFlag;

        if ( IsIDInDefString( definitionString, (char *)kNSpTCPIPDefString))
            *flags |= kProtocolTCPIPFlag;
    }

    DisposePtr( definitionString);
#endif
}

Boolean IsIDInDefString( char *definitionString, char *idString)
{
    char    *dc, ds[kNSpDefStringIDLen + 1], *dsc;
    long    i, j;

    i = 0;
    dc = definitionString;
    do
    {
        ds[0] = kNSpDefStringIDLen;
        dsc = dc;
        for ( j = 1; j <= kNSpDefStringIDLen; j++)
        {
            ds[j] = *dsc;
            dsc++;
        }
        if ( ComparePString( (unsigned char *)ds, (unsigned char *)idString)) return( true);
        i++;
        dc++;
    } while ( i < kNSpMaxDefinitionStringLen);
    return( false);
}

Boolean DoHostGame( void)
{
#ifdef kAllowNetSprocket
    OSStatus                    status = noErr;
    NSpProtocolListReference    theList = NULL;
    Boolean                     OKHit;

    if ( Wrap_GRIsHostCmd()) WriteDebugLine((char *)"\pGR HOST");
    else WriteDebugLine((char *)"\pNO GR HOST");

    ClearNetData();

    SetProtocolListFromFlags( &theList, gNetData->protocolFlags);
    //  Do the host dialog

    OKHit = Wrap_GRNSpDoModalHostDialog(theList, gNetData->gameName, gNetData->playerName,
        gNetData->password, nil);
    InitCursor();
    if (!OKHit)
        return (false);

    //  Now host the game
    status = Glue_NSpGame_Host( &gNetData->netGame, theList, kMaxNetPlayerNum, gNetData->gameName,
                gNetData->password, gNetData->playerName, 0, kNSpClientServer, 0);
/*  if ( status != noErr)
    {
        ShowErrorOfTypeOccurred( eContinueOnlyErr, kErrorStrID, kNSpHostError, status, __FILE__, 4);
        StopNetworking();
        return( false);
    }
*/  GetProtocolFlagsFromList( theList, &gNetData->protocolFlags);
    Glue_NSpProtocolList_Dispose(theList);
    gNetData->hosting = true;
    gNetData->netState = kHosting;

    // let all the players join
//  OKHit = WaitForAllPlayers( kMaxNetPlayerNum);
    if (OKHit == false)
    {
        return (false);
    } else
    {
        gNetData->netState = kStarting;
    }

    mWriteDebugString("\pGRGetPortNumber:");
    WriteDebugLong( Wrap_GRGetPortNumber());

    return (true);
#else
    return( false);
#endif
}

Boolean DoJoinGameModalDialog( void)

{
#ifdef kAllowNetSprocket
    Boolean             OKHit = true;

    ClearNetData();

    gNetData->hosting = false;

    mWriteDebugString("\pNSpDoJoinModal");

    gNetData->address = Wrap_GRNSpDoModalJoinDialog("\par12", "\pSelect Game:",
        gNetData->playerName, gNetData->password, NULL);
    InitCursor();
    if (gNetData->address == NULL)      // The user cancelled
        return (false);
    WriteDebugHexDump( (Ptr)gNetData->address, 16);

    return( true);
#else
    return( false);
#endif
}

Boolean DoJoinGame( void)
{
#ifdef kAllowNetSprocket
    OSStatus            status;
    Boolean             OKHit = true;

    status = Glue_NSpGame_Join(&gNetData->netGame, gNetData->address, gNetData->playerName,
        gNetData->password, 0, NULL, 0, 0);
    mWriteDebugString("\pjoin result:");
    WriteDebugLong( status);
    mWriteDebugString("\pGRGetPortNumber:");
    WriteDebugLong( Wrap_GRGetPortNumber());

    if ( status != noErr)
    {
        switch( status)
        {
            case kNSpTimeoutErr:
            case kNSpConnectFailedErr:
            case kNSpNotAdvertisingErr:
            case -30372://kNSpNotHostAddressErr:
            case kNSpJoinFailedErr:
                return( false);
                break;

            case kNSpInvalidAddressErr:
                ShowErrorAny( eContinueOnlyErr, kErrorStrID, nil, nil, nil, nil,
                    kNSpInvalidAddressError, -1, -1, -1, __FILE__, 2);
                break;

            default:
                ShowErrorOfTypeOccurred( eContinueOnlyErr, kErrorStrID, kNSpJoinError,
                    status, __FILE__, 5);
                break;
        }
//      StopNetworking();
        return( false);
    }

    Wrap_GRNSpReleaseAddressReference( gNetData->address);
    gNetData->netState = kJoining;

    if (OKHit == false)
    {
        gNetData->netState = kSlacking;
    }

    return OKHit;

error:
    return (false);
#else
    return( false);
#endif
}

/*
Boolean WaitForAllPlayers( short maxPlayerNum)
{
    NSpMessageHeader    *theMessage;

    while ( gNetData->playerNum < maxPlayerNum)
    {
        while ((theMessage = NSpMessage_Get(gNetData->netGame)) != NULL)
        {
            if (theMessage->what == kNSpPlayerJoined)
            {
                gNetData->playerNum++;
                AddPlayerID( theMessage->from);
            }
            else if (theMessage->what == kNSpPlayerLeft)
            {
                gNetData->playerNum--;
                RemovePlayerID( theMessage->from);
            }
            else
                SysBeep( 20);

            NSpMessage_Release(gNetData->netGame, theMessage);
        }
    }
    return( true);
}
*/

Boolean WaitForAllStart( void)
{
#ifdef kAllowNetSprocket
    NSpMessageHeader    *theMessage;
    Boolean             startYet = false;
    messageDataType     *theMessageData;

    gAresGlobal->gThisScenarioNumber = -1;

    while (( gAresGlobal->gThisScenarioNumber < 0) || ( gAresGlobal->gPlayerAdmiralNumber < 0))
    {
//      while ((theMessage = Glue_NSpMessage_Get(gNetData->netGame)) != NULL)
        theMessage = Glue_NSpMessage_Get(gNetData->netGame);
        if ( theMessage != nil)
        {
            if (theMessage->what == eStartMessage)
            {
                theMessageData = (messageDataType *)theMessage;
                mWriteDebugString("\pGot Start:");
                WriteDebugHex( theMessageData->data.packedData1, 8);
                WriteDebugHex( theMessageData->data.packedData2, 8);
                gAresGlobal->gThisScenarioNumber = theMessageData->data.packedData1;    //theMessageData->messageData.startMessage.whichChapter;
                gRandomSeed = theMessageData->data.packedData2; //theMessageData->messageData.startMessage.randomSeed;
            }

            Glue_NSpMessage_Release(gNetData->netGame, theMessage);
        }
        if ( (CommandKey()) && ( PeriodKey())) return ( false);
    }
    return( true);
#else
    return( false);
#endif
}

void SendStartMessage(void)
{
#ifdef kAllowNetSprocket
    messageDataType     theMessage;
    OSStatus            status;

    if ( gNetData->netGame != nil)
    {
        Glue_NSpClearMessageHeader(&theMessage.header);
        theMessage.header.to = kNSpAllPlayers;
        theMessage.header.what = eStartMessage;
        theMessage.header.messageLen = sizeof(messageDataType);
//      theMessage.messageData.startMessage.randomSeed = gRandomSeed;
//      theMessage.messageData.startMessage.whichChapter = gAresGlobal->gThisScenarioNumber;
        theMessage.data.packedData1 = gAresGlobal->gThisScenarioNumber;
        theMessage.data.packedData2 = gRandomSeed;
//  #ifdef kBackupData
        theMessage.backupData.packedData1 = theMessage.backupData.packedData2 = 0xffffffff;
//  #endif
//  #ifdef kBackupData2
        theMessage.backupData2.packedData1 = theMessage.backupData2.packedData2 = 0xffffffff;
//  #endif

        status = Glue_NSpMessage_Send( gNetData->netGame, &theMessage.header,
            kNSpSendFlag_Registered);
        if ( status != noErr)
        {
            mWriteDebugString("\p**ERR SEND**");
        }

    }
#endif
}

void SendEndGame(void)
{
#ifdef kAllowNetSprocket
    OSStatus            status;
    NSpMessageHeader    theMessage;

    if ( gNetData->netGame != nil)
    {
        Glue_NSpClearMessageHeader(&theMessage);
        theMessage.to = kNSpAllPlayers;
        theMessage.what = kNSpGameTerminated;
        theMessage.messageLen = sizeof(NSpMessageHeader);

        status = Glue_NSpMessage_Send(gNetData->netGame, &theMessage, kNSpSendFlag_Registered);
    }
#endif
}

Boolean GotAllMessages( void)
{
    Boolean result = true;
    short   i;

    for ( i = 0; i < kMaxNetPlayerNum; i++)
    {
        if ( !(gNetData->gotMessage[i]))
        {
            result = false;
        }
/*      if (( gNetData->hasMessageFlag[i] ^ gNetData->gotMessageFlag[i]))
        {
            result = false;
//          WriteDebugLong( gNetData->gotMessageFlag[i]);
        }
*/  }

    return ( result);
}

void ResetGotMessages( long time)
{
    short   i;
    long    minNetLatency = gNetLatency;

    time %= kMaxNetTime;
    for ( i = 0; i < kMaxNetPlayerNum; i++)
    {
        gNetData->gotMessage[i] = false;
//      gNetData->hasMessageFlag[i] = gNetData->gotMessageFlag[i] = 0;
        gNetData->thisMenuPage[i] = -1;
        gNetData->thisMenuLine[i] = -1;
        gNetData->thisSelectNum[i] = -1;
        gNetData->thisCheat[i] = -1;
        gNetData->preserveSeed = 0;
        gNetData->inSynch = true;
        gNetData->pageNum = -1;
        gNetData->lineNum = -1;
        gNetData->whichCheat = -1;
        gNetData->whichShip = -1;
        gNetData->target = false;
//      gNetData->theseKeys[i] = 0;
    }

    if ( minNetLatency < 3) minNetLatency = 3;
    RemoveExpiredSentMessages( time - ( minNetLatency * 2));
}

long UseNextLatency( void)
{
    long    calc;

//  if (gNetData->latencySampleCount == kLatencySampleNum)
//  {
        calc = gNetData->calcLatency * 6;
        calc /= 100;
        calc -= calc % kMessageLatencyUnit;
        calc += kMessageLatencyUnit;
        if ( calc != gNetLatency)
        {
            gNetLatency = calc;
//          mWriteDebugString("\pUse Next Latency");
//          WriteDebugLong( calc);
        }
//  }
    return( gNetLatency);
}


/* SetNetPlayerData: preserves races, admiralNum, and opponent color
*/
/*void SetNetPlayerData( long whichPlayer, short race, unsigned char color)

{
    gNetData->myRace = myRace;
    gNetData->opponentRace = opponentRace;
    gAresGlobal->gPlayerAdmiralNumber = myAdmiralNumber;
    gNetData->opponentColor = opponentColor;
}

*/
enum
{
    eNoTime = 0,
    ePassiveTime = 1,
    eActiveTime = 2
};

long ProcessPreGameMessages( Handle *text, long *data, long *data2, long *data3,
    long *data4, long *tripTime, long wantTime,
    unsigned char *value1, unsigned char *value2, unsigned char *value3, unsigned char *value4)
{

#ifdef kAllowNetSprocket
    NSpMessageHeader            *theMessage;
    shortMessageType            *shortMessage;
    verbosePreGameMessageType   *verboseMessage;
    NSpPlayerJoinedMessage      *joinMessage;
    messageDataType             *theMessageData;
    long                        result = eNoMessage;
    short                       roundTripTime = eNoTime;
    unsigned long               textMessageChar;

#pragma unused ( tripTime)
    if (gNetData->netGame != nil)
    {
        if ((theMessage = Glue_NSpMessage_Get(gNetData->netGame)) != nil)
        {
            switch( theMessage->what)
            {
/*              case eDummyMessage:
                    if ( gNetData->hosting)
                    {
                        gNetData->latencySampleCount--;
                        gNetData->latencySample +=
                            (NSpGetCurrentTimeStamp( gNetData->netGame) -
                            theMessage->when) * 2;
                        if ( gNetData->latencySampleCount <= 0)
                        {
                            gNetData->latencySample /=
                                (unsigned long)kLatencySampleNum;
                            gNetData->calcLatency = gNetData->latencySample;
                            gNetData->latencySampleCount = kLatencySampleNum;
                            gNetData->latencySample = 0;
                        }
                    }
*/                  break;
                case eStartToPlayMessage:
                    result = theMessage->what;
//                  SysBeep(20);
                    break;

                case ePreGameOpenScenarioMessage:
                {
                    openScenarioPreGameMessageType *openMessage;

                    openMessage = (openScenarioPreGameMessageType *)theMessage;

                    CopyPString( gAresGlobal->otherPlayerScenarioFileName,
                        openMessage->fileName);

                    CopyPString( gAresGlobal->otherPlayerScenarioFileURL,
                        openMessage->url);

                    gAresGlobal->otherPlayerScenarioFileVersion =
                        openMessage->version;

                    gAresGlobal->otherPlayerScenarioFileCheckSum =
                        openMessage->checkSum;

                    result = ePreGameOpenScenarioMessage;
                }
                    break;

                case eCancelMessage:
                case kNSpGameTerminated:
                    result = eCancelMessage;
                    break;

                case eClientReadyMessage:
                case eHostAcceptsMessage:
                    if (( data != nil) && ( data2 != nil) && ( data3 != nil))
                    {
                        result = theMessage->what;
                        verboseMessage = (verbosePreGameMessageType *)theMessage;
                        *data = verboseMessage->data1;
                        *data2 = verboseMessage->data2;
                        *data3 = verboseMessage->data3;
                    }
                    break;

                case eSetLevelMessage:
                    if ( data != nil)
                    {
                        result = eSetLevelMessage;
                        shortMessage = (shortMessageType *)theMessage;
                        *data = shortMessage->shortData;
                    }
                    break;

                case eClientMakeChangesMessage:
                    if ( data != nil)
                    {
                        result = eClientMakeChangesMessage;
                        shortMessage = (shortMessageType *)theMessage;
                        *data = shortMessage->shortData;
                    }
                    break;

                case eSetRaceMessage:
                    if ( data != nil)
                    {
                        result = eSetRaceMessage;
                        shortMessage = (shortMessageType *)theMessage;
                        *data = shortMessage->shortData;
                    }
                    break;

                case eHostIsPlayer2Message:
                    if ( data != nil)
                    {
                        result = eHostIsPlayer2Message;
                        shortMessage = (shortMessageType *)theMessage;
                        *data = shortMessage->shortData;
                    }
                    break;

                case eSetLatencyMessage:
                    if ( data != nil)
                    {
                        result = eSetLatencyMessage;
                        shortMessage = (shortMessageType *)theMessage;
                        *data = shortMessage->shortData;
                    }
                    break;

/*              case eTextMessage:
                    if ( text != nil)
                    {
                        *text = NewHandle( theMessage->messageLen - sizeof(NSpMessageHeader));
                        if ( *text != nil)
                        {
                            result = eTextMessage;
                            HLock( *text);
                            textPtr = (char *)theMessage + sizeof( NSpMessageHeader);
                            BlockMove( textPtr, **text, GetHandleSize( *text));
                            HUnlock( *text);
                        }
                    }
                    break;
*/
                case eStartTextMessage:
                    SysBeep(20);
                    StartIncomingTextMessage();
                    break;

                case eAddTextMessage:
                    SysBeep(20);
                    theMessageData = (messageDataType *)theMessage;
                    AddIncomingTextMessageLong( theMessageData->data.packedData1, 32);
                    break;

                case eEndTextMessage:
                    SysBeep(20);
                    if ( text != nil)
                    {
                        *text = NewHandle( gNetData->incomingCharNum);
                        if ( *text != nil)
                        {
                            result = eTextMessage;
                            HLock( *text);
                            StopIncomingTextMessage( (anyCharType *)**text);
                            HUnlock( *text);
                        }
                    }
                    break;

                case kNSpPlayerJoined:
//                  mWriteDebugString("\pJoined");

                    joinMessage = (NSpPlayerJoinedMessage *)theMessage;

                    result = kNSpPlayerJoined;
                    AddPlayerID( joinMessage->playerInfo.id, gNetData->playerNum);
                    gNetData->playerNum++;
                    break;

                case kNSpPlayerLeft:
//                  mWriteDebugString("\pLeft");
                    result = kNSpPlayerLeft;
                    RemovePlayerID( theMessage->from);
                    gNetData->playerNum--;
                    break;

                case eAdmiralNumberMessage:
                    result = eAdmiralNumberMessage;
                    shortMessage = (shortMessageType *)theMessage;
                    AddPlayerID( theMessage->from, shortMessage->shortData);
                    break;

                case eRoundTripGetReadyMessage:
                    roundTripTime = ePassiveTime;
                    break;

                case eRoundTripReadyMessage:
                    roundTripTime = eActiveTime;
                    break;

                case eStartMessage:
                    theMessageData = (messageDataType *)theMessage;
                    gAresGlobal->gThisScenarioNumber = theMessageData->data.packedData1; //theMessageData->messageData.startMessage.whichChapter;
                    gRandomSeed = theMessageData->data.packedData2; //theMessageData->messageData.startMessage.randomSeed;
                    break;

                case eSetResendDelayMessage:
                    shortMessage = (shortMessageType *)theMessage;
                    result = eSetResendDelayMessage;
//                  SetResendDelay( shortMessage->shortData);
                    if ( data != nil)
                    {
                        *data = shortMessage->shortData;
                    }
                    break;

                case eSetRegisteredStateMessage:
                    shortMessage = (shortMessageType *)theMessage;
                    result = eSetRegisteredStateMessage;
//                  SetRegisteredSetting( shortMessage->shortData);
                    if ( data != nil)
                    {
                        result = eSetRegisteredStateMessage;
                        *data = shortMessage->shortData;
                    }
                    break;

                case eSetBandwidthMessage:
                    shortMessage = (shortMessageType *)theMessage;
                    result = eSetBandwidthMessage;
                    if ( data != nil)
                    {
                        *data = shortMessage->shortData;
                    }
                    break;

                case eRelayPlayerStatsMessage:
                    verboseMessage = (verbosePreGameMessageType *)theMessage;
                    result = eRelayPlayerStatsMessage;
                    if ( data != nil)
                        *data = verboseMessage->data1;
                    if ( data2 != nil)
                        *data2 = verboseMessage->data2;
                    if ( data3 != nil)
                        *data3 = verboseMessage->data3;
                    if ( data4 != nil)
                        *data4 = verboseMessage->data4;
                    break;

                case eDummyMessage:
                    theMessageData = (messageDataType *)theMessage;
                    result = eDummyMessage;
                    if ( data != nil)
                    {
                        *data = theMessageData->data.packedData1;
                    }
                    mWriteDebugString("\p<DUMMY");
                    WriteDebugLong( *data);
                    WriteDebugLong( wantTime);
                    if (( theMessageData->data.packedData1 > wantTime) ||
                        (( theMessageData->data.packedData1 < wantTime) &&
                        ( theMessageData->data.packedData2 == 0xffffffff)))
                    {
                        result = eNoMessage;
                    } else if ((( theMessageData->data.packedData2 & 0xff000000) == 0x80000000) &&
                            ( theMessageData->data.packedData1 == wantTime))
                    {
                        textMessageChar = ( theMessageData->data.packedData2 &
                            0x00ffffff);
                        if ( AddIncomingTextMessageLong( textMessageChar & 0x00ffffff, 24))
                        {
                            if ( text != nil)
                            {
                                *text = NewHandle( gNetData->incomingCharNum);
                                if ( *text != nil)
                                {
    //                              result = eTextMessage;
                                    HLock( *text);
                                    StopIncomingTextMessage( (anyCharType *)**text);
                                    HUnlock( *text);
                                }
                            }
                        }
                    }
                    break;

                case ePreGameCharacterMessage:
                    theMessageData = (messageDataType *)theMessage;
                    result = ePreGameCharacterMessage;
                    if ( data != nil)
                    {
                        *data = theMessageData->data.packedData1;
                    }
                    if ( value1 != nil)
                    {
                        *value1 = theMessageData->data.packedData2;
                    }
                    if ( value2 != nil)
                    {
                        textMessageChar = theMessageData->data.packedData2 & 0x0000ff00;
                        textMessageChar >>= (long)8;
                        *value2 = textMessageChar;
                    }

                    if ( value3 != nil)
                    {
                        textMessageChar = theMessageData->data.packedData2 & 0x00ff0000;
                        textMessageChar >>= (long)16;
                        *value3 = textMessageChar;
                    }

                    if ( value4 != nil)
                    {
                        textMessageChar = theMessageData->data.packedData2 & 0xff000000;
                        textMessageChar >>= (long)24;
                        *value4 = textMessageChar;
                    }
                    break;

                case ePreGamePortraitMessage:
                    theMessageData = (messageDataType *)theMessage;
                    result = ePreGamePortraitMessage;
                    if ( data != nil)
                    {
                        *data = theMessageData->data.packedData1;
                    }
                    if ( value1 != nil)
                    {
                        *value1 = theMessageData->data.packedData2;
                    }
                    if ( value2 != nil)
                    {
                        textMessageChar = theMessageData->data.packedData2 & 0x0000ff00;
                        textMessageChar >>= (long)8;
                        *value2 = textMessageChar;
                    }
                    if ( value3 != nil)
                    {
                        textMessageChar = theMessageData->data.packedData2 & 0x00ff0000;
                        textMessageChar >>= (long)16;
                        *value3 = textMessageChar;
                    }

                    if ( value4 != nil)
                    {
                        textMessageChar = theMessageData->data.packedData2 & 0xff000000;
                        textMessageChar >>= (long)24;
                        *value4 = textMessageChar;
                    }
                    break;

                default:
                    result = theMessage->what;
                    break;

            }
            Glue_NSpMessage_Release(gNetData->netGame, theMessage);
        }
    }
    return( result);
#else
    return( eNoMessage);
#endif
}

void SendPreGameBasicMessage( long whatMessage)
{
#ifdef kAllowNetSprocket
    NSpMessageHeader    theMessage;
    OSStatus            status;

    if ( gNetData->netGame != nil)
    {
        Glue_NSpClearMessageHeader(&theMessage);
        theMessage.to = kNSpAllPlayers;
        theMessage.what = whatMessage;
        theMessage.messageLen = sizeof( NSpMessageHeader);
        status = Glue_NSpMessage_Send( gNetData->netGame, &theMessage,
            kNSpSendFlag_Registered);

        if ( status != noErr)
        {
            mWriteDebugString("\p**SEND ERR**");
        }
    }
#endif
}

void SendPreGameDummyMessage( long time, Boolean registered,
    short useLastSentChar)  // 0 = no, 1 = yes, -1 = send no char)
{
#ifdef kAllowNetSprocket
    messageDataType     theMessage;
    OSStatus            status;
    unsigned long       messageChar;

    if ( gNetData->netGame != nil)
    {
        Glue_NSpClearMessageHeader(&theMessage.header);
        theMessage.header.to = kNSpAllPlayers;
        theMessage.header.what = eDummyMessage;
        theMessage.header.messageLen = sizeof(messageDataType);
        theMessage.data.packedData1 = time;
        if ( useLastSentChar == 1)
        {
            theMessage.data.packedData2 = gNetData->backupData.packedData2;
        } else if ( useLastSentChar == 0)
        {
            if ( gNetData->outgoingCharNum != gNetData->lastOutgoingCharNum)
            {
                messageChar = TickleOutgoingMessage( true);
                messageChar <<= (unsigned long)16;
                messageChar &= 0x00ff0000;
                theMessage.data.packedData2 = messageChar;

                if ( messageChar != 0)
                {
                    messageChar = TickleOutgoingMessage( true);
                    messageChar <<= (unsigned long)8;
                    messageChar &= 0x0000ff00;
                    theMessage.data.packedData2 |= messageChar;
                }

                if ( messageChar != 0)
                {
                    messageChar = TickleOutgoingMessage( true);
                    messageChar &= 0x000000ff;
                    theMessage.data.packedData2 |= messageChar;
                }

                theMessage.data.packedData2 |= 0x80000000;
            } else
            {
                theMessage.data.packedData2 = 0;
            }
            gNetData->backupData.packedData2 = theMessage.data.packedData2;
        } else
        {
            theMessage.data.packedData2 = 0xffffffff;
        }


//  #ifdef kBackupData
        theMessage.backupData.packedData1 = theMessage.backupData.packedData2 = 0xffffffff;
//  #endif
//  #ifdef kBackupData2
        theMessage.backupData2.packedData1 = theMessage.backupData2.packedData2 = 0xffffffff;
//  #endif
        if ( registered)
        {
            status = Glue_NSpMessage_Send( gNetData->netGame, &theMessage.header,
                kNSpSendFlag_Registered);
        } else
        {
            status = Glue_NSpMessage_Send( gNetData->netGame, &theMessage.header,
                kNSpSendFlag_Normal);
        }
        if ( status != noErr)
        {
            mWriteDebugString("\p**SEND ERR**");
        }
    }
#endif
}

void SendPreGameAnyMessage( long time, Boolean registered, long message, unsigned char value1,
    unsigned char value2, unsigned char value3, unsigned char value4, Boolean useLastMessage)
{
#ifdef kAllowNetSprocket
    messageDataType     theMessage;
    OSStatus            status;
    unsigned long       messageChar;

    if ( gNetData->netGame != nil)
    {
        Glue_NSpClearMessageHeader(&theMessage.header);
        theMessage.header.to = kNSpAllPlayers;
        if ( useLastMessage) theMessage.header.what = gNetData->pregamePreviousMessage;
        else theMessage.header.what = message;
        theMessage.header.messageLen = sizeof(messageDataType);
        theMessage.data.packedData1 = time;
        if ( useLastMessage)
        {
            theMessage.data.packedData2 = gNetData->backupData.packedData2;
        } else
        {
            messageChar = value4;
            messageChar <<= (unsigned long)24;
            messageChar &= 0xff000000;
            theMessage.data.packedData2 = messageChar;

            messageChar = value3;
            messageChar <<= (unsigned long)16;
            messageChar &= 0x00ff0000;
            theMessage.data.packedData2 |= messageChar;

            messageChar = value2;
            messageChar <<= (unsigned long)8;
            messageChar &= 0x0000ff00;
            theMessage.data.packedData2 |= messageChar;

            messageChar = value1;
            messageChar &= 0x000000ff;
            theMessage.data.packedData2 |= messageChar;

            gNetData->backupData.packedData2 = theMessage.data.packedData2;
            gNetData->pregamePreviousMessage = message;
        }

//  #ifdef kBackupData
        theMessage.backupData.packedData1 = theMessage.backupData.packedData2 = 0xffffffff;
//  #endif
//  #ifdef kBackupData2
        theMessage.backupData2.packedData1 = theMessage.backupData2.packedData2 = 0xffffffff;
//  #endif
        if ( registered)
        {
            status = Glue_NSpMessage_Send( gNetData->netGame, &theMessage.header,
                kNSpSendFlag_Registered);
        } else
        {
            status = Glue_NSpMessage_Send( gNetData->netGame, &theMessage.header,
                kNSpSendFlag_Normal);
        }
        if ( status != noErr)
        {
            mWriteDebugString("\p**SEND ERR**");
        }
    }
#endif
}

void SendPreGameShortMessage( long message, long shortData)
{
#ifdef kAllowNetSprocket
    shortMessageType    theMessage;
    OSStatus            status;

    if ( gNetData->netGame != nil)
    {
        Glue_NSpClearMessageHeader(&theMessage.header);
        theMessage.header.to = kNSpAllPlayers;
        theMessage.header.what = message;
        theMessage.header.messageLen = sizeof(shortMessageType);

        theMessage.shortData = shortData;

        status = Glue_NSpMessage_Send( gNetData->netGame, &theMessage.header,
            kNSpSendFlag_Registered);

        if ( status != noErr)
        {
            mWriteDebugString("\p**SEND ERR**");
        }
    }
#endif
}

void SendPreGameVerboseMessage( long message, long data1, long data2, long data3, long data4)
{
#ifdef kAllowNetSprocket
    verbosePreGameMessageType   theMessage;
    OSStatus                    status;

    if ( gNetData->netGame != nil)
    {
        Glue_NSpClearMessageHeader(&theMessage.header);
        theMessage.header.to = kNSpAllPlayers;
        theMessage.header.what = message;
        theMessage.header.messageLen = sizeof(verbosePreGameMessageType);

        theMessage.data1 = data1;
        theMessage.data2 = data2;
        theMessage.data3 = data3;
        theMessage.data4 = data4;

        status = Glue_NSpMessage_Send( gNetData->netGame, &theMessage.header,
            kNSpSendFlag_Registered);

        if ( status != noErr)
        {
            mWriteDebugString("\p**SEND ERR**");
        }
    }
#endif
}

void SendPreGameOpenScenarioMessage( long message, StringPtr fileName,
    StringPtr url, unsigned long version, unsigned long checkSum)
{
#ifdef kAllowNetSprocket
    openScenarioPreGameMessageType  theMessage;
    OSStatus                        status;

    if ( fileName == nil) return;
    if ( url == nil) return;

    if ( gNetData->netGame != nil)
    {
        Glue_NSpClearMessageHeader(&theMessage.header);
        theMessage.header.to = kNSpAllPlayers;
        theMessage.header.what = message;
        theMessage.header.messageLen = sizeof(openScenarioPreGameMessageType);

        CopyPString( theMessage.fileName, fileName);
        CopyPString( theMessage.url, url);
        theMessage.version = version;
        theMessage.checkSum = checkSum;

        status = Glue_NSpMessage_Send( gNetData->netGame, &theMessage.header,
            kNSpSendFlag_Registered);

        if ( status != noErr)
        {
            mWriteDebugString("\p**SEND ERR**");
        }
    }
#endif
}

void SendPreGameTextMessage( Ptr sourceText, long length)
{
#ifdef kAllowNetSprocket
//  SendPreGameBasicMessage( eStartTextMessage);
    if ( length > ( kTextMessageLength)) length = kTextMessageLength;
    if ( gNetData->lastOutgoingCharNum + length >= kTextMessageLength )
    {
        BlockMove( sourceText, gNetData->outgoingMessage +
            gNetData->lastOutgoingCharNum,
            kTextMessageLength - gNetData->lastOutgoingCharNum);
        BlockMove( sourceText + kTextMessageLength - gNetData->lastOutgoingCharNum,
            gNetData->outgoingMessage, length -
                (kTextMessageLength - gNetData->lastOutgoingCharNum));
//      gNetData->outgoingCharNum = gNetData->lastOutgoingCharNum;
        gNetData->lastOutgoingCharNum = (length -
                (kTextMessageLength - gNetData->lastOutgoingCharNum));// + 1;
        if ( gNetData->lastOutgoingCharNum >= kTextMessageLength)
            gNetData->lastOutgoingCharNum = 0;
        gNetData->outgoingMessage[gNetData->lastOutgoingCharNum] = 0;
        gNetData->lastOutgoingCharNum++;
        if ( gNetData->lastOutgoingCharNum >= kTextMessageLength)
            gNetData->lastOutgoingCharNum = 0;
        SysBeep(20);
    } else
    {
        BlockMove( sourceText, gNetData->outgoingMessage + gNetData->lastOutgoingCharNum,
            length);
//      gNetData->outgoingCharNum = gNetData->lastOutgoingCharNum;
        gNetData->lastOutgoingCharNum += length;
        gNetData->outgoingMessage[gNetData->lastOutgoingCharNum] = 0;
        gNetData->lastOutgoingCharNum++;
        if ( gNetData->lastOutgoingCharNum >= kTextMessageLength)
            gNetData->lastOutgoingCharNum = 0;
    }
#endif
}

// returns false if other player quit

Boolean ProcessInGameMessages( long time, short *pauseLevel)

{
#ifdef kAllowNetSprocket
    messageDataType     *theMessageData;
    NSpMessageHeader    *theMessage;
    Boolean             endGame = false;
    long                messageTime, admiralNumber;
    packedDataType      *packedData;

    time %= kMaxNetTime;
    while ((gNetData->netGame) && ((theMessage = Glue_NSpMessage_Get(gNetData->netGame)) != nil))
    {
        theMessageData = (messageDataType *)theMessage;

        if ( theMessage->what == eStopPlayingMessage)
        {
            if ( (gAresGlobal->gScenarioWinner &
                kScenarioWinnerTextMask) == kScenarioWinnerNoText)
            {
                gAresGlobal->gScenarioWinner = theMessageData->data.packedData1;
            }
            return( false);
        }
        messageTime = 0;
//      if (( theMessage->what != ePackedMessage) || ( Randomize( 100) < 90))
        {
            if ( theMessage->what == ePackedMessage)
            {
                messageTime = theMessageData->data.packedData1 & kGameTimeMask;
                admiralNumber = (theMessageData->data.packedData2 & kAdmiralMask) >> kAdmiralBitShift;
//          #ifdef kBackupData
                if ( theMessageData->backupData.packedData1 != 0xffffffff)
                    InsertMessageInQueue( &(theMessageData->backupData), time);
//          #endif
//          #ifdef kBackupData2
                if ( theMessageData->backupData2.packedData1 != 0xffffffff)
                    InsertMessageInQueue( &(theMessageData->backupData2),time);
//          #endif
            }
            if (((messageTime > time) && (( messageTime - time) <= kMinCriticalNetTime)) ||
                (( time >= kMaxCriticalNetTime) && ( messageTime <= kMinCriticalNetTime)))
            {
                // the message comes from the future & is to be queued
                if ( theMessage->what == ePackedMessage)
                    if ( !InsertMessageInQueue( &(theMessageData->data), time)) return( false);
            } else
            {
                // the message comes from now or the past & is to be handled
                if ( HandleInGameMessage( theMessageData->header.what, &(theMessageData->data),
                        messageTime == time, pauseLevel))
                {
                    endGame = true;
                }
            }
/*
            if ( messageTime <= time)
            {
                if ( HandleInGameMessage( theMessageData->header.what, &(theMessageData->data),
                        messageTime == time, pauseLevel))
                {
                    endGame = true;
                }
            } else
            {
                if ( theMessage->what == ePackedMessage)
                    if ( !InsertMessageInQueue( &(theMessageData->data))) return( false);
            }
*/      }
        Glue_NSpMessage_Release(gNetData->netGame, theMessage);
    }

    do
    {
        packedData = PeekMessageFromQueue();
        if ( packedData != nil)
        {
            messageTime = packedData->packedData1 & kGameTimeMask;
            admiralNumber = (packedData->packedData2 & kAdmiralMask) >> kAdmiralBitShift;
            if (((messageTime > time) && (( messageTime - time) <= kMinCriticalNetTime)) ||
                (( time >= kMaxCriticalNetTime) && ( messageTime <= kMinCriticalNetTime)))
            {
                packedData = nil;
            } else
            {
                // the message comes from now or the past & is to be handled
                packedData = PopMessageFromQueue();
                if ( HandleInGameMessage( ePackedMessage, packedData, messageTime == time,
                    pauseLevel))
                {
                    endGame = true;
                }
            }
/*          if ( messageTime <= time)
            {
                packedData = PopMessageFromQueue();
                if ( HandleInGameMessage( ePackedMessage, packedData, messageTime == time,
                    pauseLevel))
                {
                    endGame = true;
                }
            } else packedData = nil;
*/      }
    } while ( packedData != nil);
    if ( endGame) StopNetworking();
    return( true);
#else
    return( false);
#endif
}

Boolean HandleInGameMessage( long whatMessage, packedDataType *theMessageData, Boolean rightNow, short *pauseLevel)
{
    unsigned long       admiralNumber = (theMessageData->packedData2 & kAdmiralMask) >> kAdmiralBitShift;
    Boolean             stopNetworking = false, gotTextMessageChar = false;
    unsigned long       textMessageChar = 0;

    switch( whatMessage)
    {
/*      case eStandardMessage:
            if (( !gNetData->gotMessage[admiralNumber]) &&
                ( rightNow))
            {
                gNetData->hasMessageFlag[admiralNumber] =
                    theMessageData->messageData.standardMessage.moreFlags;
                gNetData->gotMessage[admiralNumber] = true;
                gNetData->theseKeys[admiralNumber] =
                    theMessageData->messageData.standardMessage.keyState;
            }
            break;

        case eShortMessage:
            if ( rightNow)
            {
                sameMessage = (sameMessageDataType *)theMessageData;
                gNetData->gotMessage[admiralNumber] = true;
                if ( gNetData->preserveSeed == 0)
                {
                    gNetData->preserveSeed = theMessageData->fromAdmiralNumber &
                        kRandomSeedSynchMask;
                } else
                {
                    if (gNetData->preserveSeed !=
                        (theMessageData->fromAdmiralNumber & kRandomSeedSynchMask))
                    {
                        Str255  mySeed, yourSeed;

                        gNetData->inSynch = false;

                        NumToString( gNetData->preserveSeed, mySeed);
                        NumToString( theMessageData->fromAdmiralNumber & kRandomSeedSynchMask, yourSeed);
                        ShowErrorAny( eContinueOnlyErr, kErrorStrID, mySeed, nil, yourSeed, nil,
                            -1, kNetworkSynchError, -1, -1, __FILE__, gNetData->preserveSeed);
                    }
                }
//              gNetData->theseKeys[theMessageData->fromAdmiralNumber] =
//                  kNoNetKeyChange;
            }
            break;

        case eSelectMessage:
            if (( !(gNetData->gotMessageFlag[admiralNumber] &
                kHasSelectMessageFlag)) && (rightNow))
            {
                gNetData->gotMessageFlag[admiralNumber] |=
                    kHasSelectMessageFlag;
                gNetData->thisSelectNum[admiralNumber] =
                    theMessageData->messageData.selectMessage.whichShip;
                gNetData->thisSelectIsTarget[admiralNumber] =
                    theMessageData->messageData.selectMessage.target;
            }
            break;

        case eMenuCommandMessage:
            if (( !(gNetData->gotMessageFlag[admiralNumber] &
                kHasMenuMessageFlag)) && ( rightNow))
            {
                gNetData->gotMessageFlag[admiralNumber] |=
                    kHasMenuMessageFlag;
                gNetData->thisMenuPage[admiralNumber] =
                    theMessageData->messageData.menuCommandMessage.whichPage;
                gNetData->thisMenuLine[admiralNumber] =
                    theMessageData->messageData.menuCommandMessage.whichLine;
            }
            break;
*/

        case ePackedMessage:
            if ( rightNow)
            {
                if ( !gNetData->gotMessage[admiralNumber])
                {
                    gNetData->gotMessage[admiralNumber] = true;
                    gNetData->theseKeys[admiralNumber] =
                        theMessageData->packedData2 & kKeyStateMask;

                    if (( theMessageData->packedData1 & kWhichPageMask) !=
                        kWhichPageMask)
                    {
                        gNetData->thisMenuPage[admiralNumber] =
                            ( theMessageData->packedData1 & kWhichPageMask) >>
                            kWhichPageBitShift;
                        gNetData->thisMenuLine[admiralNumber] =
                            ( theMessageData->packedData1 & kWhichLineMask) >>
                            kWhichLineBitShift;
                    } else if (( theMessageData->packedData1 & kWhichLineMask) !=
                        kWhichLineMask)
                    {
                        if (( theMessageData->packedData1 & kWhichLineMask) ==
                            kTextMessageCharacterBits)
                        {
                            if ( admiralNumber != gAresGlobal->gPlayerAdmiralNumber)
                            {
                                textMessageChar = ( theMessageData->packedData2 &
                                    kWhichShipMask) >> kWhichShipBitShift;
                                WriteDebugChar('<');
                                WriteDebugChar( textMessageChar);
                                AddIncomingTextMessageCharacter( textMessageChar);
                                if ( textMessageChar == 0)
                                {
                                    HandleInGameTextMessage( (char *)gNetData->incomingMessage,
                                        gNetData->incomingCharNum);
                                    StopIncomingTextMessage( nil);
                                }
                            }
                            gotTextMessageChar = true;
                        } else
                        {
                            textMessageChar =
                                ( theMessageData->packedData1 & kWhichLineMask) >>
                                kWhichLineBitShift;
                            gNetData->thisCheat[admiralNumber] = textMessageChar;
                        }
                    }
                    if ( gNetData->preserveSeed == 0)
                    {
                        gNetData->preserveSeed = theMessageData->packedData1 &
                            kRandomSeedSynchMask;
                    } else
                    {
                        if (gNetData->preserveSeed !=
                            (theMessageData->packedData1 & kRandomSeedSynchMask))
                        {

                            gNetData->inSynch = false;
                        }
                    }

                    if ((( theMessageData->packedData2 & kWhichShipMask) !=
                        kWhichShipMask) && (!gotTextMessageChar))
                    {
                        gNetData->thisSelectNum[admiralNumber] =
                            (theMessageData->packedData2 & kWhichShipMask) >>
                            kWhichShipBitShift;
                        if ( theMessageData->packedData2 & kIsTargetMask)
                            gNetData->thisSelectIsTarget[admiralNumber] = true;
                        else gNetData->thisSelectIsTarget[admiralNumber] = false;
                    }
                }
            }
            break;

        case eResendMessage:
            mWriteDebugString("\p< RESEND");
            ResendSentMessage( theMessageData->packedData1 & kGameTimeMask);
            break;

/*      case eSetLatencyMessage:
            if (( !(gNetData->gotMessageFlag[admiralNumber] & kHasSetLatencyMessageFlag)) &&
                ( rightNow))
            {
                gNetData->gotMessageFlag[admiralNumber] |= kHasSetLatencyMessageFlag;
                gNetData->nextLatency =
                    theMessageData->messageData.setLatencyMessage.latency;
            }
            break;
*/
/*
        case ePreserveSeedMessage:
            mWriteDebugString("\p< PRESERVE");
            if (( !(gNetData->gotMessageFlag[admiralNumber] & kHasPreserveSeedMessageFlag)) &&
                ( rightNow))
            {
                gNetData->gotMessageFlag[admiralNumber] |= kHasPreserveSeedMessageFlag;
                if ( admiralNumber != gAresGlobal->gPlayerAdmiralNumber)
                {
                    gNetData->preserveSeed = gRandomSeed;
                }
            }
            break;
*/
/*
        case eSanityCheckMessage:
            mWriteDebugString("\p< SANITY");
            if (( !(gNetData->gotMessageFlag[admiralNumber] & kHasSanityCheckMessageFlag)) &&
                ( rightNow))
            {
                gNetData->gotMessageFlag[admiralNumber] |= kHasSanityCheckMessageFlag;
                if ( admiralNumber != gAresGlobal->gPlayerAdmiralNumber)
                {
                    if ( gNetData->preserveSeed != theMessageData->messageData.sanityCheckMessage.randomSeed)
                        SysBeep(20);
                }
                gNetData->preserveSeed = -1;
            }
            break;
*/
        case eStartPauseMessage:
//          mWriteDebugString("\p< STARTPAUSE");
            (*pauseLevel)++;
            break;

        case eEndPauseMessage:
            (*pauseLevel)--;
            // |= kGotEndPauseMessageFlag;
//          mWriteDebugString("\p< ENDPAUSE");
            break;

/*      case eTextMessage:
            textPtr = (char *)theMessageData + sizeof( sameMessageDataType);
            HandleInGameTextMessage( textPtr,
                theMessageData->header.messageLen - sizeof( sameMessageDataType));
            break;
*/
        case eStartTextMessage:
            SysBeep(20);
            StartIncomingTextMessage();
            break;

        case eAddTextMessage:
            SysBeep(20);
            AddIncomingTextMessageLong( theMessageData->packedData1, 32);
            break;

        case eEndTextMessage:
            SysBeep(20);
            HandleInGameTextMessage( (char *)gNetData->incomingMessage,
                gNetData->incomingCharNum);
            StopIncomingTextMessage( nil);
            break;

        case eSetResendDelayMessage:
            mWriteDebugString("\p<< DELAY:");
            WriteDebugLong( theMessageData->packedData1);
            SetResendDelay( theMessageData->packedData1);
            break;

        case eSetRegisteredStateMessage:
            mWriteDebugString("\p<< REGISTERED:");
            WriteDebugLong( theMessageData->packedData1);
            SetRegisteredSetting( theMessageData->packedData1);
            break;

        case eSetBandwidthMessage:
            mWriteDebugString("\p<< BAND:");
            WriteDebugLong( theMessageData->packedData1);
            SetBandwidth( theMessageData->packedData1);
            break;

        case kNSpPlayerLeft:
            stopNetworking = true;
            gNetData->playerNum--;
            break;

        case kNSpPlayerJoined:
            gNetData->playerNum++;
            break;

        case kNSpGameTerminated:
            stopNetworking = true;
            break;

        case eBarfOutDebugDataMessage:
            DebugFileSave( kDebugFileName);
            DebugFileCleanup();
            break;
    }
    return( stopNetworking);
}

// given a ptr to
void HandleInGameTextMessage( char *textPtr, long len)
{
    Str255  s;
    char    *c;

    mWriteDebugString("\pGOT TEXT");

    if ( len > 250) len = 250;
    c = (char *)s;
    *c = len;
    c++;
    while (len > 0) {*c = *textPtr; c++; textPtr++; len--;}
//  mWriteDebugString( s);
    if ( gAresGlobal->gActiveCheats[GetOtherPlayerNum()] & kNameObjectBit)
    {
        SetAdmiralBuildAtName( GetOtherPlayerNum(), s);
        gAresGlobal->gActiveCheats[GetOtherPlayerNum()] &= ~kNameObjectBit;
    } else
    {
        if (( gAresGlobal->gOptions & kOptionSpeechAvailable) &&
            ( gAresGlobal->gOptions & kOptionSpeechOn))
            SpeakString( s);
        StartStringMessage( (anyCharType *)s);
    }
}

// given the info received ProcessInGameMessages, this executes them once
// all data is received

void ExecuteInGameData( void)

{
    spaceObjectType *anObject;
    long            i;

    for ( i =0; i < kMaxNetPlayerNum; i++)
    {
        if (( !gNetData->inSynch) && ( !gNetData->haveEncounteredSynchError))
        {
//          SendInGameBasicMessage( 0, eBarfOutDebugDataMessage, true, false);
            gNetData->haveEncounteredSynchError = true;
            ShowErrorAny( eContinueErr, kErrorStrID, nil, nil, nil, nil,
                kNetworkSynchError, -1, -1, -1, __FILE__, gNetData->preserveSeed);
            SysBeep(20);
//          DebugFileSave( kDebugFileName);
//          DebugFileCleanup();
//          StopNetworking();
            gNetData->netSynchBarfCountDown = 60;
        }

        if ( gNetData->netSynchBarfCountDown >= 0)
        {
            gNetData->netSynchBarfCountDown--;
            if ( gNetData->netSynchBarfCountDown < 0)
            {
                DebugFileSave( kDebugFileName);
                DebugFileCleanup();
                StopNetworking();
            }
        }

        anObject = GetAdmiralFlagship( i);
        if ( gNetData->theseKeys[i] != kNoNetKeyChange)
            anObject->keysDown = gNetData->theseKeys[i];

/*      if ((anObject->attributes & kOnAutoPilot) &&
            ( anObject->keysDown & ( kUpKey | kDownKey | kLeftKey | kRightKey)))
        {
            anObject->keysDown |= kAutoPilotKey;
        }
*/
        if ( gNetData->thisSelectNum[i] >= 0)
        {
            SetPlayerSelectShip( gNetData->thisSelectNum[i],
                gNetData->thisSelectIsTarget[i],
                i);
        }

        if ( gNetData->thisMenuPage[i] >= 0)
        {
            MiniComputerExecute( gNetData->thisMenuPage[i],
                gNetData->thisMenuLine[i],
                i);
        }

        if ( gNetData->thisCheat[i] >= 0)
        {
            ExecuteCheat( gNetData->thisCheat[i], i);
        }
    }


}

void SendPrefabMessage( packedDataType *theData)
{
#ifdef kAllowNetSprocket
    messageDataType     theMessage;
    OSStatus            status;
    spaceObjectType     *anObject = GetAdmiralFlagship( gAresGlobal->gPlayerAdmiralNumber);

    if ( gNetData->netGame != nil)
    {
        Glue_NSpClearMessageHeader(&theMessage.header);
        theMessage.header.to = kNSpAllPlayers;

        theMessage.header.what = ePackedMessage;
        theMessage.header.messageLen = sizeof(messageDataType);
        theMessage.data.packedData1 = theData->packedData1;
        theMessage.data.packedData2 = theData->packedData2;
//  #ifdef kBackupData
        theMessage.backupData.packedData1 = theMessage.backupData.packedData2 = 0xffffffff;
//  #endif
//  #ifdef kBackupData2
        theMessage.backupData2.packedData1 = theMessage.backupData2.packedData2 = 0xffffffff;
//  #endif
        if ( gNetData->registeredFlags & kRegisterResend)
        {
//          mWriteDebugString("\p PREF REG");
            status = Glue_NSpMessage_Send( gNetData->netGame, &theMessage.header,
                kNSpSendFlag_Registered);
        }
        {
            status = Glue_NSpMessage_Send( gNetData->netGame, &theMessage.header,
                kNSpSendFlag_Normal);
        }

        if ( status != noErr)
        {
            mWriteDebugString("\p**SEND ERR**");
        }
    }
#endif
}

Boolean SendInGameMessage( long time)
{
#ifdef kAllowNetSprocket
    messageDataType     theMessage;
    OSStatus            status = noErr;
    spaceObjectType     *anObject = GetAdmiralFlagship( gAresGlobal->gPlayerAdmiralNumber);
    Boolean             charSent = false;
    unsigned long       textMessageChar;

//  if ( time > (gAresGlobal->gGameTime + (gNetLatency * 2))) DebugStr("\pTIME PROBLEM!");
    time %= kMaxNetTime;
    if ( gNetData->netGame != nil)
    {
        Glue_NSpClearMessageHeader(&theMessage.header);
        theMessage.header.to = kNSpAllPlayers;

        theMessage.header.what = ePackedMessage;
        theMessage.header.messageLen = sizeof(messageDataType);
        theMessage.data.packedData2 = theMessage.data.packedData1 = 0;
        theMessage.data.packedData2 |= gAresGlobal->gPlayerAdmiralNumber << kAdmiralBitShift;
        if ( anObject != nil)
        {
            theMessage.data.packedData2 |= anObject->keysDown & kKeyStateMask;
            gNetData->lastKeysSent = anObject->keysDown;
            anObject->keysDown = 0;
        }
        theMessage.data.packedData1 |= time & kGameTimeMask;

        if ( gNetData->pageNum >= 0)
        {
            theMessage.data.packedData1 |= (gNetData->pageNum << kWhichPageBitShift) & kWhichPageMask;
            theMessage.data.packedData1 |= (gNetData->lineNum << kWhichLineBitShift) & kWhichLineMask;
        } else if ( gNetData->whichCheat >= 0)
        {
            theMessage.data.packedData1 |= kWhichPageMask;
            theMessage.data.packedData1 |= (gNetData->whichCheat << kWhichLineBitShift) & kWhichLineMask;
        } else if (( gNetData->outgoingCharNum != gNetData->lastOutgoingCharNum) &&
            (gNetData->whichShip < 0))
        {
            theMessage.data.packedData1 |= kWhichPageMask;
            theMessage.data.packedData1 |= kTextMessageCharacterBits;
            textMessageChar = TickleOutgoingMessage( true);
            WriteDebugChar('>');
            WriteDebugChar( textMessageChar);
            theMessage.data.packedData2 |= (textMessageChar << kWhichShipBitShift) & kWhichShipMask;
            charSent = true;
        } else
        {
            theMessage.data.packedData1 |= kWhichPageMask | kWhichLineMask;
        }

        if ( gNetData->whichShip >= 0)
        {
            theMessage.data.packedData2 |= gNetData->whichShip << kWhichShipBitShift;
            if ( gNetData->target) theMessage.data.packedData2 |= kIsTargetMask;
        } else if ( !charSent)
        {
            theMessage.data.packedData2 |= kWhichShipMask;
        }
        textMessageChar = gAresGlobal->gSynchValue;
        textMessageChar <<= kSynchBitShift;
        theMessage.data.packedData1 |= textMessageChar & kRandomSeedSynchMask;//gRandomSeed & kRandomSeedSynchMask;

/*      DebugFileAppendString( "\pSM\t");
        DebugFileAppendLong( gAresGlobal->gGameTime);
        DebugFileAppendString( "\p\t");
        DebugFileAppendLong( gRandomSeed);
        DebugFileAppendString( "\p\t");
        DebugFileAppendLong( theMessage.data.packedData1 & kRandomSeedSynchMask);
        DebugFileAppendString( "\p\r");
*/
        theMessage.backupData2.packedData1 =
            theMessage.backupData2.packedData2 =
            theMessage.backupData.packedData1 =
            theMessage.backupData.packedData2 = 0xffffffff;

        if ( gNetLatency > kMessageLatencyUnit)
        {
            theMessage.backupData2.packedData1 = gNetData->backupData2.packedData1;
            theMessage.backupData2.packedData2 = gNetData->backupData2.packedData2;
            gNetData->backupData2.packedData1 = gNetData->backupData.packedData1;
            gNetData->backupData2.packedData2 = gNetData->backupData.packedData2;
        }

        if ( gNetLatency > 0)
        {
            theMessage.backupData.packedData1 = gNetData->backupData.packedData1;
            theMessage.backupData.packedData2 = gNetData->backupData.packedData2;
            gNetData->backupData.packedData1 = theMessage.data.packedData1;
            gNetData->backupData.packedData2 = theMessage.data.packedData2;
        }

        if ( !InsertMessageInQueue( &(theMessage.data), time)) return( false);
        if ( !StoreSentMessage( &(theMessage.data))) return( false);

        if (( gNetData->registeredFlags & kLowerBandwidth) && ( gNetLatency > kMessageLatencyUnit) &&
             ( !(time & 0x00000001))) return true;

        if ( gNetData->registeredFlags & kRegisterStandard)
        {
//          mWriteDebugString("\p STAND REG");
//          if ( Randomize( 10) < 8)
                status = Glue_NSpMessage_Send( gNetData->netGame, &theMessage.header,
                    kNSpSendFlag_Registered);
        } else
        {
//          if ( Randomize( 10) < 8)
                status = Glue_NSpMessage_Send( gNetData->netGame, &theMessage.header,
                    kNSpSendFlag_Normal);
        }
        if ( status != noErr)
        {
            mWriteDebugString("\p**SEND ERR**");
        }
    }
    return( true);
#else
    return( false);
#endif
}

Boolean SendInGameBasicMessage( long time, long whatMessage, Boolean registered, Boolean toSelf)
{
#ifdef kAllowNetSprocket
    messageDataType     theMessage;
    OSStatus            status;

    time %= kMaxNetTime;

    if ( gNetData->netGame != nil)
    {
        Glue_NSpClearMessageHeader(&theMessage.header);
        theMessage.header.to = kNSpAllPlayers;
        theMessage.header.what = whatMessage;
        theMessage.header.messageLen = sizeof(messageDataType);
        /*
        theMessage.fromAdmiralNumber = gAresGlobal->gPlayerAdmiralNumber;
        theMessage.gameTime = time;
        if ( toSelf)
        {
            if ( !InsertMessageInQueue( &theMessage)) return( false);
        }
        if ( !registered)
        {
            if ( !StoreSentMessage( &theMessage)) return( false);
            status = NSpMessage_Send( gNetData->netGame, &theMessage.header, kNSpSendFlag_Normal);
        } else
        {
            status = NSpMessage_Send( gNetData->netGame, &theMessage.header, kNSpSendFlag_Registered);
        }

//      mWriteDebugString("\p> BASIC");
//      WriteDebugLong( whatMessage);

        if ( status != noErr)
        {
            mWriteDebugString("\p**SEND ERR**");
        }
        */
        theMessage.data.packedData1 = 0;
        theMessage.data.packedData2 = 0;
        theMessage.data.packedData2 |= gAresGlobal->gPlayerAdmiralNumber << kAdmiralBitShift;

//  #ifdef kBackupData
        theMessage.backupData.packedData1 = theMessage.backupData.packedData2 = 0xffffffff;
//  #endif
//  #ifdef kBackupData2
        theMessage.backupData2.packedData1 = theMessage.backupData2.packedData2 = 0xffffffff;
//  #endif

        if ( toSelf)
        {
//          if ( !InsertMessageInQueue( &(theMessage.data))) return( false);
        }
        if ( !registered)
        {
//          if ( !StoreSentMessage( &theMessage)) return( false);
            status = Glue_NSpMessage_Send( gNetData->netGame, &theMessage.header, kNSpSendFlag_Normal);
        } else
        {
//          mWriteDebugString("\pBASIC REG");
            status = Glue_NSpMessage_Send( gNetData->netGame, &theMessage.header,
                kNSpSendFlag_Registered);
        }
    }
    return( true);
#else
    return( false);
#endif
}

Boolean SendInGameMiscLongMessage( long time, long whatMessage, long data,
    Boolean registered, Boolean toSelf)
{
#ifdef kAllowNetSprocket
    messageDataType     theMessage;
    OSStatus            status;

    time %= kMaxNetTime;
    if ( gNetData->netGame != nil)
    {
        Glue_NSpClearMessageHeader(&theMessage.header);
        theMessage.header.to = kNSpAllPlayers;
        theMessage.header.what = whatMessage;
        theMessage.header.messageLen = sizeof(messageDataType);
        /*
        theMessage.fromAdmiralNumber = gAresGlobal->gPlayerAdmiralNumber;
        theMessage.gameTime = time;
        theMessage.messageData.miscLongDataMessage.longData1 = data;
        if ( toSelf)
        {
            if ( !InsertMessageInQueue( &theMessage)) return( false);
        }
        if ( !registered)
        {
            if ( !StoreSentMessage( &theMessage)) return( false);
            status = NSpMessage_Send( gNetData->netGame, &theMessage.header, kNSpSendFlag_Normal);
        } else
        {
            status = NSpMessage_Send( gNetData->netGame, &theMessage.header, kNSpSendFlag_Registered);
        }

//      mWriteDebugString("\p> BASIC");
//      WriteDebugLong( whatMessage);

        if ( status != noErr)
        {
            mWriteDebugString("\p**SEND ERR**");
        }
        */
        theMessage.data.packedData1 = data;
        theMessage.data.packedData2 = 0;
        theMessage.data.packedData2 |= gAresGlobal->gPlayerAdmiralNumber << kAdmiralBitShift;

//  #ifdef kBackupData
        theMessage.backupData.packedData1 = theMessage.backupData.packedData2 = 0xffffffff;
//  #endif
//  #ifdef kBackupData2
        theMessage.backupData2.packedData1 = theMessage.backupData2.packedData2 = 0xffffffff;
//  #endif

        if ( toSelf)
        {
//          if ( !InsertMessageInQueue( &(theMessage.data))) return( false);
        }
        if ( !registered)
        {
//          if ( !StoreSentMessage( &theMessage)) return( false);
            status = Glue_NSpMessage_Send( gNetData->netGame, &theMessage.header, kNSpSendFlag_Normal);
        } else
        {
//          mWriteDebugString("\pMISC REG");
            status = Glue_NSpMessage_Send( gNetData->netGame, &theMessage.header,
                kNSpSendFlag_Registered);
        }

//      mWriteDebugString("\p> BASIC");
//      WriteDebugLong( whatMessage);

        if ( status != noErr)
        {
            mWriteDebugString("\p**SEND ERR**");
        }
    }
    return( true);
#else
    return( false);
#endif
}

Boolean SendSelectMessage( long time, long whichShip, Boolean target)
{
#ifdef kAllowNetSprocket

    time %= kMaxNetTime;
    if ( gNetData->netGame != nil)
    {
/*      NSpClearMessageHeader(&theMessage.header);
        theMessage.header.to = kNSpAllPlayers;
        theMessage.header.what = eSelectMessage;
        theMessage.header.messageLen = sizeof(messageDataType);
        theMessage.fromAdmiralNumber = gAresGlobal->gPlayerAdmiralNumber;
        theMessage.gameTime = time;
        theMessage.messageData.selectMessage.whichShip = whichShip;
        theMessage.messageData.selectMessage.target = target;
    //  mWriteDebugString("\p> SEND Select");
    //  WriteDebugLong(theMessage.messageData.selectMessage.whichShip);
        if ( theMessage.messageData.selectMessage.target)
        {
    //      mWriteDebugString("\pTARGET");
        }
        if ( !InsertMessageInQueue( &theMessage)) return( false);
        if (!StoreSentMessage( &theMessage)) return( false);
        if ( gNetData->registeredFlags & kRegisterSelect)
        {
            status = NSpMessage_Send( gNetData->netGame, &theMessage.header,
                kNSpSendFlag_Registered);
        } else
        {
            status = NSpMessage_Send( gNetData->netGame, &theMessage.header,
                kNSpSendFlag_Normal);
        }

        if ( status != noErr)
        {
            mWriteDebugString("\p**SEND ERR**");
        }
        gNetData->hasMessageFlag[gAresGlobal->gPlayerAdmiralNumber] |= kHasSelectMessageFlag;
*/
        gNetData->whichShip = whichShip;
        gNetData->target = target;
    }

    return( true);
#else
    return( false);
#endif
}

Boolean SendMenuMessage( long time, short whichPage, short whichLine)
{
#ifdef kAllowNetSprocket

    time %= kMaxNetTime;
    if ( gNetData->netGame != nil)
    {
/*      NSpClearMessageHeader(&theMessage.header);
        theMessage.header.to = kNSpAllPlayers;
        theMessage.header.what = eMenuCommandMessage;
        theMessage.header.messageLen = sizeof(messageDataType);
        theMessage.fromAdmiralNumber = gAresGlobal->gPlayerAdmiralNumber;
        theMessage.gameTime = time;
        theMessage.messageData.menuCommandMessage.whichPage = whichPage;
        theMessage.messageData.menuCommandMessage.whichLine = whichLine;
        if ( !InsertMessageInQueue( &theMessage)) return( false);
        if (!StoreSentMessage( &theMessage)) return( false);
        if ( gNetData->registeredFlags & kRegisterMenu)
        {
            status = NSpMessage_Send( gNetData->netGame, &theMessage.header,
                kNSpSendFlag_Registered);
        } else
        {
            status = NSpMessage_Send( gNetData->netGame, &theMessage.header,
                kNSpSendFlag_Normal);
        }

        if ( status != noErr)
        {
            mWriteDebugString("\p**SEND ERR**");
        }
        gNetData->hasMessageFlag[gAresGlobal->gPlayerAdmiralNumber] |= kHasMenuMessageFlag;
*/
        gNetData->pageNum = whichPage;
        gNetData->lineNum = whichLine;
    }
    return( true);
#else
    return( false);
#endif
}

void SendCheatMessage( short whichCheat)
{
     if (( gNetData->pageNum == -1) && ( gNetData->whichCheat < 0)) gNetData->whichCheat = whichCheat;
}

Boolean SendSetLatencyMessage( long time)
{
#ifdef kAllowNetSprocket
#pragma unused ( time)

/*  messageDataType     theMessage;
    OSStatus            status;

    if ( gNetData->netGame != nil)
    {
        if ( gNetData->latencySampleCount == kLatencySampleNum)
        {
//          mWriteDebugString("\p> LATENCY");
            NSpClearMessageHeader(&theMessage.header);
            theMessage.header.to = kNSpAllPlayers;
            theMessage.header.what = eSetLatencyMessage;
            theMessage.header.messageLen = sizeof(messageDataType);
            theMessage.fromAdmiralNumber = gAresGlobal->gPlayerAdmiralNumber;
            theMessage.gameTime = time;
            theMessage.messageData.setLatencyMessage.latency = gNetData->calcLatency;
//          WriteDebugLong( theMessage.messageData.setLatencyMessage.latency);
            if ( !InsertMessageInQueue( &theMessage)) return( false);
            if (!StoreSentMessage( &theMessage)) return( false);
            status = NSpMessage_Send( gNetData->netGame, &theMessage.header, kNSpSendFlag_Normal);

            if ( status != noErr)
            {
                mWriteDebugString("\p**SEND ERR**");
            }
            gNetData->hasMessageFlag[gAresGlobal->gPlayerAdmiralNumber] |= kHasSetLatencyMessageFlag;
        }
    }
*/
    return( true);
#else
    return( false);
#endif
}

Boolean SendSanityCheckMessage( long time)
{
#ifdef kAllowNetSprocket
#pragma unused ( time)
/*  messageDataType     theMessage;
    OSStatus            status;

    if ( gNetData->netGame != nil)
    {
        mWriteDebugString("\p> SANITY");
        NSpClearMessageHeader(&theMessage.header);
        theMessage.header.to = kNSpAllPlayers;
        theMessage.header.what = eSanityCheckMessage;
        theMessage.header.messageLen = sizeof(messageDataType);
        theMessage.fromAdmiralNumber = gAresGlobal->gPlayerAdmiralNumber;
        theMessage.gameTime = time;
        theMessage.messageData.sanityCheckMessage.randomSeed = gRandomSeed;
        if ( !InsertMessageInQueue( &theMessage)) return( false);
        if (!StoreSentMessage( &theMessage)) return( false);
        status = NSpMessage_Send( gNetData->netGame, &theMessage.header,
            kNSpSendFlag_Normal);

        if ( status != noErr)
        {
            mWriteDebugString("\p**SEND ERR**");
        }
        gNetData->hasMessageFlag[gAresGlobal->gPlayerAdmiralNumber] |= kHasSanityCheckMessageFlag;
    }
*/
    return( true);
#else
    return( false);
#endif
}

Boolean SendPreserveSeedMessage( long time)
{
#pragma unused ( time)
#ifdef kAllowNetSprocket
/*  messageDataType     theMessage;
    OSStatus            status;

    if ( gNetData->netGame != nil)
    {
        mWriteDebugString("\p> PRESERVE");
        NSpClearMessageHeader(&theMessage.header);
        theMessage.header.to = kNSpAllPlayers;
        theMessage.header.what = ePreserveSeedMessage;
        theMessage.header.messageLen = sizeof(messageDataType);
        theMessage.fromAdmiralNumber = gAresGlobal->gPlayerAdmiralNumber;
        theMessage.gameTime = time;
        if ( !InsertMessageInQueue( &theMessage)) return( false);
        if (!StoreSentMessage( &theMessage)) return( false);
        status = NSpMessage_Send( gNetData->netGame, &theMessage.header, kNSpSendFlag_Normal);

        if ( status != noErr)
        {
            mWriteDebugString("\p**SEND ERR**");
        }
        gNetData->hasMessageFlag[gAresGlobal->gPlayerAdmiralNumber] |= kHasPreserveSeedMessageFlag;
    }
    gNetData->sanityCheckTime = time;
*/  return( true);
#else
    return( false);
#endif
}

void SendInGameTextMessage( Ptr sourceText, long length)
{
#ifdef kAllowNetSprocket
    if ( !NetGameIsOn()) return;
//  SendInGameBasicMessage( 0, eStartTextMessage, true, false);
    if ( length > ( kTextMessageLength )) length = kTextMessageLength;
    if ( gNetData->lastOutgoingCharNum + length >= kTextMessageLength )
    {
        BlockMove( sourceText, gNetData->outgoingMessage +
            gNetData->lastOutgoingCharNum,
            kTextMessageLength - gNetData->lastOutgoingCharNum);
        BlockMove( sourceText + kTextMessageLength - gNetData->lastOutgoingCharNum,
            gNetData->outgoingMessage, length -
                (kTextMessageLength - gNetData->lastOutgoingCharNum));
        gNetData->lastOutgoingCharNum = (length -
                (kTextMessageLength - gNetData->lastOutgoingCharNum));// + 1;
        if ( gNetData->lastOutgoingCharNum >= kTextMessageLength)
            gNetData->lastOutgoingCharNum = 0;
        gNetData->outgoingMessage[gNetData->lastOutgoingCharNum] = 0;
        gNetData->lastOutgoingCharNum++;
        if ( gNetData->lastOutgoingCharNum >= kTextMessageLength)
            gNetData->lastOutgoingCharNum = 0;
    } else
    {
        BlockMove( sourceText, gNetData->outgoingMessage + gNetData->lastOutgoingCharNum,
            length);
        gNetData->lastOutgoingCharNum += length;
        gNetData->outgoingMessage[gNetData->lastOutgoingCharNum] = 0;
        gNetData->lastOutgoingCharNum++;
        if ( gNetData->lastOutgoingCharNum >= kTextMessageLength)
            gNetData->lastOutgoingCharNum = 0;
    }

#endif
}

Boolean SendInGameShortMessage( long time)
{
#pragma unused ( time)
#ifdef kAllowNetSprocket
/*  sameMessageDataType theMessage;
    OSStatus                status;

    if ( gNetData->netGame != nil)
    {
        NSpClearMessageHeader(&theMessage.header);
        theMessage.header.to = kNSpAllPlayers;
        theMessage.header.what = eShortMessage;
        theMessage.header.messageLen = sizeof(sameMessageDataType);
        theMessage.fromAdmiralNumber = gAresGlobal->gPlayerAdmiralNumber;
        theMessage.fromAdmiralNumber |= gRandomSeed & kRandomSeedSynchMask;
        theMessage.gameTime = time;
        if ( !InsertMessageInQueue( (messageDataType *)&theMessage))
            return( false);

        if (!StoreSentMessage( (messageDataType *)&theMessage))
            return( false);
        if ( gNetData->registeredFlags & kRegisterNoChange)
        {
            status = NSpMessage_Send( gNetData->netGame, &theMessage.header,
                kNSpSendFlag_Registered);
        } else
        {
            status = NSpMessage_Send( gNetData->netGame, &theMessage.header, kNSpSendFlag_Normal);
        }

        if ( status != noErr)
        {
            mWriteDebugString("\p**SEND ERR**");
        }
    }
*/  return( true);
#else
    return( false);
#endif
}

void HostAutoSanityCheck( long time)
{
#pragma unused ( time)
/*  if ( gNetData->preserveSeed < 0)
    {
        SendPreserveSeedMessage( time + gNetLatency);
        gNetData->preserveSeed = gNetLatency * 9;
    } else if ( time == gNetData->sanityCheckTime)
    {
        SendSanityCheckMessage( time + gNetLatency);
    } else
    {
        gNetData->preserveSeed--;
    }
*/
}

void SendResendMessage( long time)
{
#ifdef kAllowNetSprocket
    messageDataType     theMessage;
    OSStatus            status;

    time %= kMaxNetTime;
    if (( gNetData->netGame != nil)/* && ( gNetData->hosting)*/)
    {
        Glue_NSpClearMessageHeader(&theMessage.header);
        theMessage.header.to = kNSpAllPlayers;
        theMessage.header.what = eResendMessage;
        theMessage.header.messageLen = sizeof(messageDataType);

        theMessage.data.packedData1 = time;
        theMessage.data.packedData2 = 0;
        theMessage.data.packedData2 |= gAresGlobal->gPlayerAdmiralNumber << kAdmiralBitShift;

//  #ifdef kBackupData
        theMessage.backupData.packedData1 = theMessage.backupData.packedData2 = 0xffffffff;
//  #endif
//  #ifdef kBackupData2
        theMessage.backupData2.packedData1 = theMessage.backupData2.packedData2 = 0xffffffff;
//  #endif

        if ( gNetData->registeredFlags & kRegisterResendRequest)
        {
//          mWriteDebugString("\p RESEND REG");
            status = Glue_NSpMessage_Send( gNetData->netGame, &theMessage.header,
                kNSpSendFlag_Registered);
        } else
        {
            status = Glue_NSpMessage_Send( gNetData->netGame, &theMessage.header,
                kNSpSendFlag_Normal);
        }

        mWriteDebugString("\p> RESEND >");
        if ( status != noErr)
        {
            mWriteDebugString("\p**SEND ERR**");
        }
    }
#endif
}

Boolean IAmHosting( void)

{
    return( gNetData->hosting);
}

Boolean InsertMessageInQueue( packedDataType *theData, long time)
{
    short   queueNumber = 0, nextQueue = gNetData->queueTop, previousQueue = -1;
    long    relativeTime;   // "corrected" for time wrap-around
    Boolean placeFound = false;

    while (( gNetData->latencyQueue[queueNumber].used == true) &&
        ( queueNumber < kLatencyQueueLen))
    {
        queueNumber++;
    }

    if ( queueNumber >= kLatencyQueueLen)
    {
        DebugStr("\pMESSAGE QUEUE FULL.");
        return( false);
    }

    gNetData->latencyQueue[queueNumber].data.packedData1 = theData->packedData1;
    gNetData->latencyQueue[queueNumber].data.packedData2 = theData->packedData2;
    gNetData->latencyQueue[queueNumber].used = true;
/*  while (( nextQueue != -1) &&
        (( gNetData->latencyQueue[nextQueue].data.packedData1 & kGameTimeMask)
            < (gNetData->latencyQueue[queueNumber].data.packedData1 & kGameTimeMask)))
*/
    time = gNetData->latencyQueue[queueNumber].data.packedData1 & kGameTimeMask;
    while (( nextQueue != -1) && ( !placeFound))
    {
        relativeTime = gNetData->latencyQueue[nextQueue].data.packedData1 & kGameTimeMask;
        if (( time <= kMinCriticalNetTime) && ( relativeTime >= kMaxCriticalNetTime))
            relativeTime -= kMaxNetTime;
        else if (( time >= kMaxCriticalNetTime) && ( relativeTime <= kMinCriticalNetTime))
            relativeTime += kMaxNetTime;
        if ( relativeTime < time)
        {
            previousQueue = nextQueue;
            nextQueue = gNetData->latencyQueue[nextQueue].next;
        } else placeFound = true;
    }
    if (( ( gNetData->latencyQueue[nextQueue].data.packedData1 & kGameTimeMask) ==
        (theData->packedData1 & kGameTimeMask)) &&
        ((gNetData->latencyQueue[nextQueue].data.packedData2 & kAdmiralMask) ==
        ( theData->packedData2 & kAdmiralMask)))
    {
        if ((gNetData->latencyQueue[nextQueue].data.packedData1 !=
                gNetData->latencyQueue[queueNumber].data.packedData1) ||
            (gNetData->latencyQueue[nextQueue].data.packedData2 !=
                gNetData->latencyQueue[queueNumber].data.packedData2))
        {
            ShowErrorAny( eContinueErr, kErrorStrID, nil, nil, nil, nil, kCorruptNetDataError,
                -1, -1, -1, __FILE__, gNetData->latencyQueue[queueNumber].data.packedData1 & kGameTimeMask);
        }
    }

    if ( previousQueue == -1)
    {
        gNetData->latencyQueue[queueNumber].next = gNetData->queueTop;
        gNetData->queueTop = queueNumber;
//      mWriteDebugString("\p<top>");
    } else
    {
        gNetData->latencyQueue[queueNumber].next = gNetData->latencyQueue[previousQueue].next;
        gNetData->latencyQueue[previousQueue].next = queueNumber;
//      mWriteDebugString("\p<after>");
//      WriteDebugLong( gNetData->latencyQueue[previousQueue].message.gameTime);
    }
    return( true);
}

packedDataType *PopMessageFromQueue( void)
{
    long    queueNumber = gNetData->queueTop;

    if ( gNetData->queueTop >= 0)
    {
//      mWriteDebugString("\p- POP");
//      WriteDebugLong( gNetData->latencyQueue[gNetData->queueTop].message.gameTime);

        gNetData->queueTop = gNetData->latencyQueue[gNetData->queueTop].next;
        gNetData->latencyQueue[queueNumber].used = false;
        return( &(gNetData->latencyQueue[queueNumber].data));
    } else return( nil);
}

packedDataType *PeekMessageFromQueue( void)
{
    if ( gNetData->queueTop >= 0)
    {
        return( &(gNetData->latencyQueue[gNetData->queueTop].data));
    } else return ( nil);
}

void DebugMessageQueue( void)
{
    long    queueNumber = gNetData->queueTop;

    if ( gNetData != nil)
    {
        mWriteDebugString("\pMessage Queue:");
        while ( queueNumber != -1)
        {
            WriteDebugLong( gNetData->latencyQueue[queueNumber].data.packedData1 & kGameTimeMask);

            queueNumber = gNetData->latencyQueue[queueNumber].next;
        }
    } else
    {
        mWriteDebugString("\pNo Net Game");
    }
}

Boolean JumpstartLatencyQueue( long fromTime, long byUnit)

{
    long    i;

    for ( i = fromTime; i < (fromTime + gNetLatency + byUnit); i += byUnit)
    {
        if ( !SendInGameMessage( i)) return( false);
    }
    return( true);
}

Boolean StoreSentMessage( packedDataType *theMessageData)

{
    short   i = 0;

    while (( i < kLatencyQueueLen) && ( gNetData->sentMessage[i].packedData1 != 0xffffffff))
    {
        i++;
    }

    if ( i >= kLatencyQueueLen)
    {
//      DebugStr("\pSent Message Heap FULL!");
        return( false);
    }

//  gNetData->sentMessage[i] = *theMessageData;
    gNetData->sentMessage[i].packedData1 = theMessageData->packedData1;
    gNetData->sentMessage[i].packedData2 = theMessageData->packedData2;
    return( true);
}

void ResendSentMessage( long time)

{
    short   i = 0;

    for ( i = 0; i < kLatencyQueueLen; i++)
    {
        if ((( gNetData->sentMessage[i].packedData1 & kGameTimeMask) == time) &&
            ( gNetData->sentMessage[i].packedData1 != 0xffffffff))
        {
//          mWriteDebugString("\p.resending.");
            SendPrefabMessage( &gNetData->sentMessage[i]);
        }
    }

}

void RemoveExpiredSentMessages( long oldTime) // chuck all message <= old time

{
    short   i;
    long    messageTime;

    if ( oldTime < 0)
    {
        for ( i = 0; i < kLatencyQueueLen; i++)
        {
            if ( gNetData->sentMessage[i].packedData1 != 0xffffffff)
            {
                messageTime = gNetData->sentMessage[i].packedData1 & kGameTimeMask;
                if (( messageTime > kMinCriticalNetTime) && ( messageTime < ( kMaxNetTime +
                    oldTime)))
                {
                    gNetData->sentMessage[i].packedData1 = 0xffffffff;
                }
            }
        }
    } else if ( oldTime >= kMaxCriticalNetTime)
    {
        for ( i = 0; i < kLatencyQueueLen; i++)
        {
            if ( gNetData->sentMessage[i].packedData1 != 0xffffffff)
            {
                messageTime = gNetData->sentMessage[i].packedData1 & kGameTimeMask;
                if (( messageTime < kMaxCriticalNetTime) && ( messageTime > ( kMinCriticalNetTime -
                    ( kMaxNetTime - oldTime))))
                {
                    gNetData->sentMessage[i].packedData1 = 0xffffffff;
                } else if ( messageTime < oldTime)
                {
                    gNetData->sentMessage[i].packedData1 = 0xffffffff;
                }
            }
        }
    } else if ( oldTime <= kMinCriticalNetTime)
    {
        for ( i = 0; i < kLatencyQueueLen; i++)
        {
            if ( gNetData->sentMessage[i].packedData1 != 0xffffffff)
            {
                messageTime = gNetData->sentMessage[i].packedData1 & kGameTimeMask;
                if (( messageTime > kMinCriticalNetTime) && ( messageTime < ( kMaxNetTime -
                    ( kMinCriticalNetTime - oldTime))))
                {
                    gNetData->sentMessage[i].packedData1 = 0xffffffff;
                } else if ( messageTime < oldTime)
                {
                    gNetData->sentMessage[i].packedData1 = 0xffffffff;
                }
            }
        }
    } else
    {
        for ( i = 0; i < kLatencyQueueLen; i++)
        {
            if ((( gNetData->sentMessage[i].packedData1 & kGameTimeMask) <= oldTime) &&
                ( gNetData->sentMessage[i].packedData1 != 0xffffffff))
            {
                gNetData->sentMessage[i].packedData1 = 0xffffffff;
            }
        }
    }
}

void ResetSentMessages( void)
{
    short   i;

    for ( i = 0; i < kLatencyQueueLen; i++)
    {
        gNetData->sentMessage[i].packedData1 = 0xffffffff;
    }
}

unsigned char TickleOutgoingMessage( Boolean registered)
{
    unsigned long textOut = 0, charNum = 0;
    unsigned char   result = 0;

#pragma unused ( registered)
/*  if (( gNetData->outgoingMessage[gNetData->outgoingCharNum] != 0) &&
        ( gNetData->outgoingCharNum != gNetData->lastOutgoingCharNum))
    {
        do
        {
            textOut <<= (long)8;
            textOut |= gNetData->outgoingMessage[gNetData->outgoingCharNum];
            gNetData->outgoingCharNum++;
            if ( gNetData->outgoingCharNum >= kTextMessageLength)
                gNetData->outgoingCharNum = 0;
            charNum++;
        } while ((charNum < 4) && ( gNetData->outgoingMessage[gNetData->outgoingCharNum]
            != 0));
        SendInGameMiscLongMessage( 0, eAddTextMessage, textOut, registered, false);
        if ( gNetData->outgoingMessage[gNetData->outgoingCharNum] == 0)
        {
            SendInGameBasicMessage( 0, eEndTextMessage, true, false);
            gNetData->outgoingCharNum++;
            if ( gNetData->outgoingCharNum >= kTextMessageLength)
                gNetData->outgoingCharNum = 0;
        }
    }
*/
    if ( gNetData->outgoingCharNum != gNetData->lastOutgoingCharNum)
    {
        result = gNetData->outgoingMessage[gNetData->outgoingCharNum];
        gNetData->outgoingCharNum++;
        if ( gNetData->outgoingCharNum >= kTextMessageLength)
            gNetData->outgoingCharNum = 0;
    }
    WriteDebugChar( result);
    return( result);
}

void StartIncomingTextMessage( void)
{
//  gNetData->incomingCharNum = 0;
}

Boolean AddIncomingTextMessageLong( unsigned long what, unsigned long bitShift)
{
    Boolean         endFound = false;

    while ((  bitShift > 0) && ( gNetData->incomingCharNum < kTextMessageLength)
        && ( !endFound))
    {
        bitShift -= 8;
/*      gNetData->incomingMessage[gNetData->incomingCharNum] = (what >> bitShift) & 0xff;
        if ( gNetData->incomingMessage[gNetData->incomingCharNum] != 0)
        gNetData->incomingCharNum++;
*/
        AddIncomingTextMessageCharacter( (what >> bitShift) & 0xff);
        if (( (what >> bitShift) & 0xff) == 0) endFound = true;
//      if ( gNetData->incomingCharNum >= kTextMessageLength) gNetData->incomingCharNum = 0;
    }
    return( endFound);
}

void AddIncomingTextMessageCharacter( unsigned char what)
{
    gNetData->incomingMessage[gNetData->incomingCharNum] = what;
//  WriteDebugChar('<');
    if ( what != 0)
    {
        gNetData->incomingCharNum++;
        WriteDebugChar( what);
    } else
    {
        mWriteDebugString("\p-EOM-");
    }

}

void StopIncomingTextMessage( anyCharType *dest)
{
    short   i = 0;

    if ( dest != nil)
    {
        while ( i < gNetData->incomingCharNum)
        {
            *dest++ = gNetData->incomingMessage[i++];
        }
    }
    gNetData->incomingCharNum = 0;
}
