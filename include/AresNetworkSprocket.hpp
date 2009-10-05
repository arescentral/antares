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

#ifndef ANTARES_ARES_NETWORK_SPROCKET_HPP_
#define ANTARES_ARES_NETWORK_SPROCKET_HPP_

// Ares NetworkSprocket.h

#include <NetSprocket.h>

#include "AnyChar.hpp"

namespace antares {

#define kBackupData
#define kBackupData2

#define kMaxNetPlayerNum        2

#define kMessageLatency         ((long)24)
#define kMaxMessageLatency      ((long)48)

#define kMessageLatencyUnit     ((long)3)       // must be the same as kDecideEveryCycles
#define kMaxCriticalNetTime     ((long)(kMaxNetTime - (kMaxMessageLatency * (long)2)))
#define kMinCriticalNetTime     ((long)(kMaxMessageLatency * (long)2))

#define kHasSelectMessageFlag       0x00000001
#define kHasMenuMessageFlag         0x00000002
#define kHasSetLatencyMessageFlag   0x00000004
#define kHasSanityCheckMessageFlag  0x00000008
#define kHasPreserveSeedMessageFlag 0x00000010

#define kGotStartPauseMessageFlag   0x80000000
#define kGotEndPauseMessageFlag     0x40000000

#define kProtocolAppleTalkFlag  0x00000001
#define kProtocolTCPIPFlag      0x00000002

#define kRegisterNoChange       0x00000001
#define kRegisterStandard       0x00000002
#define kRegisterResendRequest  0x00000004
#define kRegisterResend         0x00000008
#define kRegisterMenu           0x00000010
#define kRegisterSelect         0x00000020
#define kLowerBandwidth         0x00000040

#define kNoNetKeyChange             0xffffffff

#if TARGET_OS_MAC
#define NETSPROCKET_AVAILABLE       0

struct dummyDumType {
    long    banana;
    short   apple;
};

struct sameMessageDataType {
    NSpMessageHeader            header;
    long                        gameTime;
    long                        fromAdmiralNumber;
};

struct packedDataType {
    unsigned long               packedData1;    // sanity/page/line/time
    unsigned long               packedData2;    // from admiralNum/target/whichShip/keysdown
};

struct messageDataType {
    NSpMessageHeader            header;
    packedDataType              data;
#ifdef kBackupData
    packedDataType              backupData;     // last turn's data
#endif
#ifdef kBackupData2
    packedDataType              backupData2;
#endif
};

// pregame message results
enum
{
    eNilMessage                 = -1,       // MUST NEVER BE USED (Apple(R) resrved)
    ePreGameCharacterMessage    = 10001,    // pregame text character
    ePreGamePortraitMessage     = 10002,    // pregame portrait runline
    eClientMakeChangesMessage   = 10003,    // states whether or not client can make changes
    eStartMessage               = 10004,    // generic begin
    eSanityCheckMessage         = 10005,    // for debugging; make sure rndseeds match
    ePreserveSeedMessage        = 10006,    // for debugging; preseve rand seed for sanity check
    eAdmiralNumberMessage       = 10007,    // set admiral #
    eShortMessage               = 10008,    // potentially used for minimal data
    eNoMessage                  = 10009,    // ?
    eResendMessage              = 10010,    // resend old data
    eTextMessage                = 10011,    // text chat message, contains text
    eSetLevelMessage            = 10012,    // determine level to use
    eCancelMessage              = 10013,    // cancel out of setup
    eHostDeclinesMessage        = 10014,    // host refuses client
    eHostAcceptsMessage         = 10015,    // host accepts client
    eClientReadyMessage         = 10016,    // client got host accept
    eRoundTripStartMessage      = 10017,    // unused (start round trip time)
    eRoundTripEndMessage        = 10018,    // unused (end round trip time)
    eRoundTripGetReadyMessage   = 10019,    // unused (prepare to time round trip)
    eRoundTripReadyMessage      = 10020,    // unused (confirm ready to time)
    eRoundTripDoneMessage       = 10021,    // unused (stop round trip timing)
    eStartPauseMessage          = 10022,    // pause activate
    eEndPauseMessage            = 10023,    // pause deactivated
    eSetLatencyMessage          = 10024,    // host sets latency
    eDummyMessage               = 10025,    // sent just for timing
    eHostIsPlayer2Message       = 10026,    // is host player 2?
    eSetRaceMessage             = 10027,    // assign race
    eStartToPlayMessage         = 10028,    // begin playing game, sent after briefing
    eStartToLoadLevelMessage    = 10029,    // load level, sent after net level select
    eSetResendDelayMessage      = 10030,    // set the amount of time to request a resend
    eSetRegisteredStateMessage  = 10031,    // set which types of messages should be registered
    eBarfOutDebugDataMessage    = 10032,    // write out debug data (if any)
    eStartTextMessage           = 10033,    // begin new incoming text message
    eAddTextMessage             = 10034,    // add chars to text message
    eEndTextMessage             = 10035,    // end incoming text message & process
    ePackedMessage              = 10036,    // the new, basic ingame message
    eSetBandwidthMessage        = 10037,
    eStopPlayingMessage         = 10038,    // end game but return to net setup
    eRelayPlayerStatsMessage    = 10039,    // convey player info during net setup
    eReadyForNetSetupMessage    = 10040,    // player is ready to start net setup
    ePreGameOpenScenarioMessage = 10041,    // external scenario file opened
    ePreGameGotScenarioMessage  = 10042,    // yes, I have that scenario file & all is well
    ePreGameNoScenarioMessage   = 10043,    // no, I don't have that scenario file
    ePreGameOldVersionScenarioMessage   = 10044,    // I have that scenario but
                                                    // it's the wrong version
    ePreGameNewVersionScenarioMessage   = 10045,    // I have that scenario but
                                                    // it's the wrong version
    ePreGameWrongCheckSumScenarioMessage = 10046    // I have that scenario but
                                                    // the checksum is different
};


STUB0(NetSprocketPresent, bool(), false);
STUB0(InitNetworking, short(), 0);
STUB0(BeginNetworking, short(), 0);
STUB0(ClearNetData, void());
STUB0(ResetNetData, void());
STUB0(StopNetworking, void());
STUB0(DisposeNetworking, void());
STUB0(GetNumberOfPlayers, long(), 1);
STUB0(GetAllNetPlayersCheating, bool(), false);
STUB0(NetGameIsOn, bool(), false);
STUB0(GetResendDelay, long(), 0);
STUB0(SetResendDelay, void( long));
STUB0(GetRegisteredSetting, long(), 1);
STUB1(SetRegisteredSetting, void( long));
STUB0(GetBandwidth, bool(), false);
STUB0(SetBandwidth, void( bool));
STUB0(GetNetMinutesPlayed, unsigned short(), 0);
STUB1(SetNetMinutesPlayed, void( unsigned short));
STUB0(GetNetKills, unsigned short(), 0);
STUB1(SetNetKills, void( unsigned short));
STUB0(GetNetLosses, unsigned short(), 0);
STUB1(SetNetLosses, void( unsigned short));
STUB0(GetNetRace, short(), 0);
STUB1(SetNetRace, void( short));
STUB0(GetNetEnemyColor, short(), 0);
STUB1(SetNetEnemyColor, void( short));
STUB0(GetNetLevel, short(), 0);
STUB1(SetNetLevel, void( short));
STUB0(GetOpponentIsUnregistered, bool(), false);
STUB1(SetOpponentIsUnregistered, void( bool unregistered));
STUB0(GetHaveSeenUnregisteredTimeLimitWarning, bool(), false);
STUB1(SetHaveSeenUnregisteredTimeLimitWarning, void( bool));
STUB2(AddPlayerID, void( long, long));
STUB1(GetPlayerNumFromID, long( long), 0);
STUB1(GetPlayerIDFromNum, long( long), 0);
STUB0(MyPlayerID, long(), 0);
STUB1(RemovePlayerID, void( long));
STUB2(SetPlayerRace, void( long, short));
STUB1(GetPlayerRace, short( long), 0);
STUB2(SetPlayerColor, void( long, unsigned char));
STUB1(GetPlayerColor, unsigned char( long), 0);
STUB1(GetPlayerName, const char*( long), "Player");
STUB2(GetOtherPlayerConnectionData, void( unsigned long *, unsigned long *));
STUB1(GetOtherPlayerName, void(unsigned char*));
STUB2(SetProtocolListFromFlags, void( NSpProtocolListReference *, unsigned long));
STUB2(GetProtocolFlagsFromList, void( NSpProtocolListReference, unsigned long *));
STUB2(IsIDInDefString, bool( char *, char *), false);
STUB0(DoHostGame, bool(), false);
STUB0(DoJoinGameModalDialog, bool(), false);
STUB0(DoJoinGame, bool(), false);
STUB0(WaitForAllStart, bool(), false);
STUB0(SendStartMessage, void());
STUB0(SendEndGame, void());
STUB0(GotAllMessages, bool(), false);
STUB1(ResetGotMessages, void( long));
STUB0(UseNextLatency, long(), 0);
/*
STUB11(ProcessPreGameMessages,
    long( Handle *, long *, long *, long *, long *, long *, long, unsigned char *, unsigned char *,
    unsigned char *, unsigned char *),
    0);
*/
STUB1(SendPreGameBasicMessage, void( long));
STUB3(SendPreGameDummyMessage, void( long, bool, short));
STUB8(SendPreGameAnyMessage,
    void(long, bool, long, unsigned char, unsigned char, unsigned char, unsigned char, bool));
STUB2(SendPreGameShortMessage, void( long, long));
STUB5(SendPreGameVerboseMessage, void( long, long, long, long, long));
STUB2(SendPreGameTextMessage, void( Ptr, long));
STUB5(SendPreGameOpenScenarioMessage, void( long message, unsigned char* fileName,
    unsigned char* url, unsigned long version, unsigned long checkSum));

STUB2(ProcessInGameMessages, bool( long, short *), false);
STUB4(HandleInGameMessage, bool( long, packedDataType *, bool, short *), false);
STUB2(HandleInGameTextMessage, void( char *, long));
STUB0(ExecuteInGameData, void());
STUB1(SendPrefabMessage, void( packedDataType *));
STUB1(SendInGameMessage, bool( long), false);
STUB4(SendInGameBasicMessage, bool( long, long, bool, bool), false);
STUB5(SendInGameMiscLongMessage, bool( long, long, long, bool, bool), false);
STUB3(SendSelectMessage, bool( long, long, bool), false);
STUB3(SendMenuMessage, bool( long, short, short), false);
STUB1(SendCheatMessage, void( short));
STUB1(SendSetLatencyMessage, bool( long), false);
STUB1(SendSanityCheckMessage, bool( long), false);
STUB1(SendPreserveSeedMessage, bool( long), false);
STUB2(SendInGameTextMessage, void( Ptr, long));
STUB1(SendInGameShortMessage, bool( long), false);
STUB1(HostAutoSanityCheck, void( long));
STUB1(SendResendMessage, void( long));
STUB0(IAmHosting, bool(), false);
STUB2(InsertMessageInQueue, bool( packedDataType *, long), false);
STUB0(PopMessageFromQueue, packedDataType*(), NULL);
STUB0(PeekMessageFromQueue, packedDataType*(), NULL);
STUB0(DebugMessageQueue, void());
STUB2(JumpstartLatencyQueue, bool( long, long), false);
STUB1(StoreSentMessage, bool( packedDataType *), false);
STUB1(ResendSentMessage, void( long));
STUB1(RemoveExpiredSentMessages, void( long));
STUB0(ResetSentMessages, void());
STUB1(TickleOutgoingMessage, unsigned char(bool), '\0');
STUB0(StartIncomingTextMessage, void());
STUB1(StopIncomingTextMessage, void(unsigned char *));

#if 0
bool NetSprocketPresent(void);
short InitNetworking(void);
short BeginNetworking( void);
void ClearNetData( void);
void ResetNetData( void);
void StopNetworking( void);
void DisposeNetworking( void);
long GetNumberOfPlayers( void);
bool GetAllNetPlayersCheating( void);
bool NetGameIsOn( void);
long GetResendDelay( void);
void SetResendDelay( long);
long GetRegisteredSetting( void);
void SetRegisteredSetting( long);
bool GetBandwidth( void);
void SetBandwidth( bool);
unsigned short GetNetMinutesPlayed( void);
void SetNetMinutesPlayed( unsigned short);
unsigned short GetNetKills( void);
void SetNetKills( unsigned short);
unsigned short GetNetLosses( void);
void SetNetLosses( unsigned short);
short GetNetRace( void);
void SetNetRace( short);
short GetNetEnemyColor( void);
void SetNetEnemyColor( short);
short GetNetLevel( void);
void SetNetLevel( short);
bool GetOpponentIsUnregistered( void);
void SetOpponentIsUnregistered( bool unregistered);
bool GetHaveSeenUnregisteredTimeLimitWarning( void);
void SetHaveSeenUnregisteredTimeLimitWarning( bool);
void AddPlayerID( long, long);
long GetPlayerNumFromID( long);
long GetPlayerIDFromNum( long);
long MyPlayerID( void);
void RemovePlayerID( long);
void SetPlayerRace( long, short);
short GetPlayerRace( long);
void SetPlayerColor( long, unsigned char);
unsigned char GetPlayerColor( long);
StringPtr GetPlayerName( long);
void GetOtherPlayerConnectionData( unsigned long *, unsigned long *);
void GetOtherPlayerName( StringPtr *);
void SetProtocolListFromFlags( NSpProtocolListReference *, unsigned long);
void GetProtocolFlagsFromList( NSpProtocolListReference, unsigned long *);
bool IsIDInDefString( char *, char *);
bool DoHostGame(void);
bool DoJoinGameModalDialog( void);
bool DoJoinGame(void);
bool WaitForAllStart( void);
void SendStartMessage( void);
void SendEndGame(void);
bool GotAllMessages( void);
void ResetGotMessages( long);
long UseNextLatency( void);
long ProcessPreGameMessages( Handle *, long *, long *, long *, long *, long *, long, unsigned char *, unsigned char *,
    unsigned char *, unsigned char *);
void SendPreGameBasicMessage( long);
void SendPreGameDummyMessage( long, bool, short);
void SendPreGameAnyMessage( long, bool, long, unsigned char, unsigned char, unsigned char, unsigned char, bool);
void SendPreGameShortMessage( long, long);
void SendPreGameVerboseMessage( long, long, long, long, long);
void SendPreGameTextMessage( Ptr, long);
void SendPreGameOpenScenarioMessage( long message, StringPtr fileName,
    StringPtr url, unsigned long version, unsigned long checkSum);

bool ProcessInGameMessages( long, short *);
bool HandleInGameMessage( long, packedDataType *, bool, short *);
void HandleInGameTextMessage( char *, long);
void ExecuteInGameData( void);
void SendPrefabMessage( packedDataType *);
bool SendInGameMessage( long);
bool SendInGameBasicMessage( long, long, bool, bool);
bool SendInGameMiscLongMessage( long, long, long, bool, bool);
bool SendSelectMessage( long, long, bool);
bool SendMenuMessage( long, short, short);
void SendCheatMessage( short);
bool SendSetLatencyMessage( long);
bool SendSanityCheckMessage( long);
bool SendPreserveSeedMessage( long);
void SendInGameTextMessage( Ptr, long);
bool SendInGameShortMessage( long);
void HostAutoSanityCheck( long);
void SendResendMessage( long);
bool IAmHosting( void);
bool InsertMessageInQueue( packedDataType *, long);
packedDataType *PopMessageFromQueue( void);
packedDataType *PeekMessageFromQueue( void);
void DebugMessageQueue( void);
bool JumpstartLatencyQueue( long, long);
bool StoreSentMessage( packedDataType *);
void ResendSentMessage( long);
void RemoveExpiredSentMessages( long);
void ResetSentMessages( void);
unsigned char TickleOutgoingMessage(bool);
void StartIncomingTextMessage( void);
void StopIncomingTextMessage( anyCharType *);
#endif
#else   // TARGET_OS_MAC
#define NETSPROCKET_AVAILABLE   0
#endif // TARGET_OS_MAC

}  // namespace antares

#endif // ANTARES_ARES_NETWORK_SPROCKET_HPP_
