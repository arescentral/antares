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

#include "NetSetupScreen.h"

#include <QDOffscreen.h>

#include "AmbrosiaSerial.h"
#include "AresExternalFile.h"
#include "AresGlobalType.h"
#include "AresMain.h"
#include "AresNetworkSprocket.h"
#include "AresResFile.h"
#include "ColorTranslation.h"
#include "ConditionalMacros.h"
#include "Debug.h"
#include "DirectText.h"
#include "Error.h"
#include "EZSprite.h"
#include "HandleHandling.h"
#include "HideMenubar.h"
#include "Icl8ToCicn.h"
#include "IconSuiteFromAlias.h"
#include "InterfaceHandling.h"
#include "MessageScreen.h" // for retroText drawing
#include "OffscreenGWorld.h"
#include "Options.h"
#include "PlayerInterface.h"
#include "Races.h"
#include "Resources.h"
#include "ScenarioMaker.h"
#include "SingleDataFile.h" // to determine whether or not to allow changing of mission or race
#include "SmartFile.h"
#include "StringHandling.h"
#include "StringNumerics.h"
#include "TitleScreen.h" // for CenterRectInRect

#define kHasCustomIcon      0x0400

#define kNetLevelID                 5020

#define kNetLevelOKButton           0
#define kNetLevelCancelButton       1
//#define   kNetLevelSendTextButton     2
#define kNetLevelChangesCheck       10
//#define   kNetLevelEnterTextBox       4
//#define   kNetLevelCommBox            5
#define kNetLevelTabBox             14//10
#define kNetLevelHelpBox            15//11

#define kNetLevelItemNum            20//16//15

#define kNetLevelSettingTabID       5023
#define kNetLevelSettingTabNum      13//8
#define kNetLevelRegisterNever      ( kNetLevelItemNum + 0)
#define kNetLevelRegisterResend     ( kNetLevelItemNum + 1)
#define kNetLevelRegisterAlways     ( kNetLevelItemNum + 2)
#define kNetLevelResend1Second      ( kNetLevelItemNum + 3)
#define kNetLevelResend2Seconds     ( kNetLevelItemNum + 4)
#define kNetLevelResend4Seconds     ( kNetLevelItemNum + 5)
#define kNetLevelIncreaseLagButton  ( kNetLevelItemNum + 6)
#define kNetLevelDecreaseLagButton  ( kNetLevelItemNum + 7)
#define kNetLevelLowerBandwidth     ( kNetLevelItemNum + 8)
#define kNetLevelLagBox             ( kNetLevelItemNum + 9)
#define kNetSettingsFirstRegisteredRadio    kNetLevelRegisterNever
#define kNetSettingsLastRegisteredRadio     kNetLevelRegisterAlways
#define kNetSettingsFirstDelayRadio         kNetLevelResend1Second
#define kNetSettingsLastDelayRadio          kNetLevelResend4Seconds

#define kNetLevelPlayerTabID        5022
#define kNetLevelPlayerTabNum       12//7
#define kPreviousRaceButton         ( kNetLevelItemNum + 0)
#define kNextRaceButton             ( kNetLevelItemNum + 1)
#define kNetLevelRaceBox            ( kNetLevelItemNum + 2)
#define kNetLevelColorBox           ( kNetLevelItemNum + 3)
#define kHostIsPlayer2CheckBox      ( kNetLevelItemNum + 4)

#define kNetLevelLevelTabID         5021
#define kNetLevelLevelTabNum        11//6
#define kPreviousNetLevelButton     ( kNetLevelItemNum + 0)
#define kNextNetLevelButton         ( kNetLevelItemNum + 1)
#define kOpenNetLevelButton         ( kNetLevelItemNum + 2)
#define kNetLevelBox                ( kNetLevelItemNum + 3)
#define kWhyDisabledNetLevelText    ( kNetLevelItemNum + 4)

#define kFirstPlayerItemGroup       2
#define kPlayerItemGroupSize        4
#define kPlayerGroupPortraitNum     0
#define kPlayerGroupShipNum         1
#define kPlayerGroupDataNum         2
#define kPlayerGroupTextNum         3

#define kMaxVisibleChar             120

#define kNetLagGraphBox             kNetLevelLagBox//( kNetLevelItemNum + 12)
#define kNetLagMaxValue             96//48

#define kColorBoxSpacing            3
#define kColorBoxHSpacing           0

#define kCommLinePad                5

#define kTimeSampleNum              64

#define kMyCommColor                PALE_GREEN
#define kOpponentCommColor          ORANGE

#define kNetLevelHelpStrID          2008
#define kNetLevelLevelStrNum        18//13
#define kNetLevelPlayerStrNum       23//16
#define kNetLevelSettingsStrNum     28//21

#define kPortraitSize               32
#define kPortraitMargin             0//2

#define kCharBufferSize         512

#define kDeleteChar             0x08
#define kBackspaceChar          28

#define kPlayerDataTextID                       6006
#define kPlayerDataKeyStringID              6006
#define kPlayerNumberKeyStringNum           1
#define kPlayerNameKeyStringNum             2
#define kPlayerSpeciesKeyStringNum          3
#define kPlayerGamesPlayedKeyStringNum  4
#define kPlayerMinutesPlayedKeyStringNum    9
#define kPlayerKillsKeyStringNum            10
#define kPlayerLossesKeyStringNum           11

#define kPortraitFileName               "\pYour Ares Net Portrait"

#define mIncreaseBufferCounter( mbuf) (mbuf)++; if ( mbuf >= kCharBufferSize) mbuf = 0

extern aresGlobalType           *gAresGlobal;
extern long                     gNatePortLeft, gNatePortTop, CLIP_LEFT, CLIP_RIGHT, CLIP_TOP,
                                CLIP_BOTTOM, /*gAresGlobal->gTrueClipBottom,*/
                                gNetLatency, /*gAresGlobal->gThisScenarioNumber,*/ gRandomSeed;
extern GWorldPtr                gOffWorld, gRealWorld, gSaveWorld;
extern CWindowPtr               gTheWindow/*, gAresGlobal->gBackWindow*/;       // we need the window for copying to the real world, a hack
extern directTextType           *gDirectText;
extern Handle                   gDirectTextData, gColorTranslateTable, gBaseObjectData;
extern long                     gWhichDirectText, WORLD_WIDTH, WORLD_HEIGHT;
                                /*gAresGlobal->gPlayerAdmiralNumber;*/
//extern unsigned long          gAresGlobal->gOptions;
extern  GDHandle                theDevice;

typedef struct
{
    retroTextSpecType   retroTextSpec;
    long                    currentLineTop;
    long                    whichTab;
    long                    thisChapter;
    long                    myNum;
    long                    opponentNum;
    long                    currentLatency;
    long                    recommendedLatency;
    long                    sampleNum;
    long                    lastSampleTime;
    long                    sampleLag[kTimeSampleNum];
    short                   myRace;
    short                   opponentRace;
    short                   currentRegistered;
    short                   currentDelay;
    short                   currentDelayValue;
    unsigned char       opponentColor;
    char                    outgoingChar[kCharBufferSize];
    char                    incomingChar[kCharBufferSize];
    long                    outgoingCharTop;
    long                    outgoingCharBottom;
    long                    incomingCharTop;
    long                    incomingCharBottom;
    Ptr                 myPortraitData;
    Ptr                 opponentPortraitData;
    long                    inPortX;
    long                    inPortY;
    long                    outPortX;
    long                    outPortY;
    short                   opponentMinutesPlayed;
    short                   opponentKills;
    short                   opponentLosses;
    Str255              visibleString[kMaxNetPlayerNum];
    Boolean             opponentHasArrived;
    Boolean             currentBandwidth;
    Boolean             hostIsPlayer2;
    Boolean             imReady;
    Boolean             opponentReady;
    Boolean             canMakeChanges;
    Boolean             clientCanMakeChanges;
    Boolean             scenarioLoaded;
    Str255              whyScenarioNotLoaded;
} netSetupType;

#if NETSPROCKET_AVAILABLE

void SetNetLevelCanMakeChanges( netSetupType *, Boolean);
void ActivateLevelTab( netSetupType *, Boolean);
void HandleLevelTabHit( short, Point, netSetupType *);
void SetNetLevelNum( netSetupType *);
void ActivatePlayerTab( netSetupType *, Boolean);
void HandlePlayerTabHit( short, Point, netSetupType *);
void SetNetHostIsPlayer2( netSetupType *);
void SetCurrentNetRace( netSetupType *);
void ActivateSettingTab( netSetupType *, Boolean);
long GetDelayValueFromDelayRadio( short);
void SetNetLag( netSetupType *);
void SetNetRegistered( netSetupType *, short);
void SetNetDelay( netSetupType *, short );
void HandleSettingTabHit( short, Point, netSetupType *);
void SetNetLowerBandwidth( netSetupType *);
void ShowNetLevelName( long);
void ShowWhyNetLevelNotLoaded( netSetupType *setup, StringPtr why);
void NetLevelDrawLagGraph( netSetupType *);
long NetLevelGetLagMean( netSetupType *);
long NetLevelGetLagStandardDeviation( netSetupType *, long);
long NetLevelGetLagHighestBelow( netSetupType *, long);
unsigned char NetLevelSetColor( netSetupType *, unsigned char, unsigned char, Point, Boolean, Boolean);
void ShowNetTextMessage( netSetupType *, Boolean);
void SendNetTextMessage( netSetupType *);
void AddCharToVisibleString( netSetupType *, long, anyCharType, Boolean);
Boolean GetPortraitHRun( long *, long *, unsigned char *, unsigned char *, unsigned char *);
void AddPortraitHRun( long *, long *, unsigned char, unsigned char, unsigned char *);
short GetSpriteResIDFromRace( short, long *);
void UpdateSampleShip( netSetupType *, long);
void UpdatePlayerData( netSetupType *, long);
void DrawPortraitByPlayerNum( netSetupType *, long);
void OpenOtherScenarioFile( netSetupType *setup);
void LaunchURL( StringPtr s);

long DoTabbedNetLevelInterface( void)
{
    Point                       where;
    int                     error;
    short                       whichItem, helpItem = -1;
    Boolean                 done = FALSE, cancel = FALSE, gotDummy = false,
                                    messageToResend = false;
    EventRecord             theEvent;
    char                        whichChar;
    CWindowPtr              whichWindow;
    long                        i, tabItemNum = 0, message, newLevel,
                                    lastFlashTime = 0, dummyRequestTime = 0,
                                    waitTime = 60, longData2, longData3, longData4;
    interfaceItemType       *anItem;
    Rect                        tRect;
    scenarioType            *scenario;
    netSetupType            setup;
    unsigned char           beginButtonRealColor, val1, val2, val3, val4, runColor, runLen;
    Str255                  tstring, whyDisabledString;
    CIconHandle             tempIcon;

    mWriteDebugString("\pEnter NetSetup");
    EraseOffWorld();
    EraseSaveWorld();
    gAresGlobal->gThisScenarioNumber = gRandomSeed = -1;


    if ( !Ambrosia_Is_Registered())
    {
        SetNetRace( GetRaceNumFromID( 100));
        SetNetLevel( GetFirstNetworkScenario());
    }

    setup.whichTab                      = kNetLevelLevelTabNum;
    setup.thisChapter                   = GetNetLevel();//0;
    setup.myNum                         = 0;
    setup.opponentNum                   = 1;
    setup.myRace                        = GetNetRace();
    setup.currentLatency                = gNetLatency;
    setup.recommendedLatency        = 3;
    setup.currentRegistered         = GetRegisteredSetting();
    setup.currentDelayValue         = GetResendDelay();
    setup.opponentColor             = GetNetEnemyColor();//1;
    setup.currentBandwidth          = GetBandwidth();
    setup.hostIsPlayer2             = false;
    setup.imReady                       = false;
    setup.opponentReady             = false;
    setup.canMakeChanges                = true;
    setup.clientCanMakeChanges      = false;
    setup.opponentHasArrived        = false;
    setup.scenarioLoaded            = true;
    setup.sampleNum                 = 0;
    setup.retroTextSpec.text        = nil;
    setup.outgoingCharTop           = 0;
    setup.outgoingCharBottom        = 0;
    setup.incomingCharTop           = 0;
    setup.incomingCharBottom        = 0;
    setup.inPortX                       = 0;
    setup.inPortY                       = 0;
    setup.outPortX                      = 0;
    setup.outPortY                      = 0;
    setup.opponentMinutesPlayed = 0;
    setup.opponentKills             = 0;
    setup.opponentLosses                = 0;
    setup.whyScenarioNotLoaded[0]       = 0;

    setup.myPortraitData            = NewPtr( kPortraitSize * kPortraitSize);
    if ( setup.myPortraitData == nil) MyDebugString("\pCan't allocate space for your portrait.");
    ClearPortrait( (unsigned char *)setup.myPortraitData);
    setup.opponentPortraitData      = NewPtr( kPortraitSize * kPortraitSize);
    if ( setup.opponentPortraitData == nil) DebugStr("\pCan't allcoate space for opponent portrait.");
    ClearPortrait( (unsigned char *)setup.opponentPortraitData);

    for ( i = 0; i < kMaxNetPlayerNum; i++)
    {
        setup.visibleString[i][0] = 0;
    }
    for ( i = 0; i < kTimeSampleNum; i++)
        setup.sampleLag[i] = 0;

    FlushEvents(everyEvent, 0);

    error = OpenInterface( kNetLevelID);

    if ( error == kNoError)
    {
//      GetAnyInterfaceItemContentBounds( GetAnyInterfaceItemPtr( kNetLevelCommBox), &tRect);
//      setup.currentLineTop = tRect.top;
//      InterfaceTextEditItemInit( kNetLevelEnterTextBox);
//      InterfaceTextEditSetText( kNetLevelEnterTextBox, ( anyCharType *)"\p");
//      InterfaceTextEditSelectAll( kNetLevelEnterTextBox);
//      InterfaceTextEditActivate( kNetLevelEnterTextBox);

        if ( !IAmHosting())
        {
            setup.myNum = 1;
            setup.opponentNum = 0;
            SetStatusOfAnyInterfaceItem( kNetLevelChangesCheck, kDimmed, false);
        } else
        {

//          SendPreGameShortMessage( eAdmiralNumberMessage, setup.myNum);
        }
        SwitchAnyRadioOrCheckbox( kNetLevelChangesCheck,
            setup.clientCanMakeChanges);

        AddPlayerID( MyPlayerID(), setup.myNum);

        if ( !ThisChapterIsNetworkable( setup.thisChapter))
        {
            setup.thisChapter = GetFirstNetworkScenario();
        }
        scenario = GetScenarioPtrFromChapter( setup.thisChapter);

        if ( scenario != nil)
        {
            if ( !IsRaceLegal( setup.myRace, setup.myNum, scenario))
            {
                setup.myRace =
                    GetRaceNumFromID( scenario->player[setup.myNum].playerRace);
            }
        }
        setup.opponentRace = GetRaceNumFromID( scenario->player[setup.opponentNum].playerRace);
        if ( setup.currentDelayValue <= 60) setup.currentDelay = 0;
        else if ( setup.currentDelayValue <= 120) setup.currentDelay = 1;
        else if ( setup.currentDelayValue <= 240) setup.currentDelay = 2;
        else setup.currentDelay = 3;

        tabItemNum = AppendInterface( kNetLevelLevelTabID, kNetLevelTabBox,
            true);
        ActivateLevelTab( &setup, false);
        DrawInterfaceOneAtATime();
        DrawStringInInterfaceItem( kNetLevelHelpBox, nil);
        if (( !Ambrosia_Is_Registered()) || ( GetOpponentIsUnregistered()))
        {
            GetIndString( tstring, 2010, 1);
            DrawStringInInterfaceItem( kWhyDisabledNetLevelText, tstring);
        }
        ShowNetLevelName( setup.thisChapter);

        anItem = GetAnyInterfaceItemPtr( kNetLevelOKButton);
        beginButtonRealColor = anItem->color;

/*      if ( IAmHosting())
        {
            SendPreGameShortMessage( eSetLatencyMessage, setup.currentLatency);
            SendPreGameShortMessage( eSetRegisteredStateMessage, GetRegisteredSetting());
            SendPreGameShortMessage( eSetResendDelayMessage, GetResendDelay());
            SendPreGameShortMessage( eSetBandwidthMessage, GetBandwidth());
            SendPreGameShortMessage( eClientMakeChangesMessage, setup.clientCanMakeChanges);
            SendPreGameShortMessage( eSetRaceMessage, setup.myRace);
            SendPreGameShortMessage( eSetLevelMessage, setup.thisChapter);
//          SendPreGameDummyMessage( setup.sampleNum, setup.currentRegistered > 1, -1);
            messageToResend = true;
            SendPreGameAnyMessage( setup.sampleNum, setup.currentRegistered > 1,
                 eDummyMessage, 0, 0, 0, 0, false);
        } else
        {
            SendPreGameShortMessage( eSetRaceMessage, setup.myRace);
        }
        SendPreGameVerboseMessage( eRelayPlayerStatsMessage, GetNetMinutesPlayed(),
            GetNetKills(), GetNetLosses(), 0);
*/
        SendPreGameBasicMessage( eReadyForNetSetupMessage);

//          SendPreGameAnyMessage( setup.sampleNum, setup.currentRegistered > 1,
//               eDummyMessage, 0, 0, false);

        GetAnyInterfaceItemContentBounds( GetAnyInterfaceItemPtr( kFirstPlayerItemGroup +
            (setup.myNum * kPlayerItemGroupSize) + kPlayerGroupPortraitNum), &tRect);
        tRect.right = tRect.left + 32;
        tRect.bottom = tRect.top + 32;
        if ( !PlotCustomIcon( &tRect))
        {
            tempIcon = GetCIcon( 501);
            if ( tempIcon == nil) MyDebugString("\pCan't find your portrait.");
            else
            {
                DrawInOffWorld();
                DefaultColors();
                PlotCIcon( &tRect, tempIcon);
                DrawInRealWorld();
                DefaultColors();
                DisposeCIcon( tempIcon);
            }
        }
        ConvertOffscreenRectToPortraitData( tRect.left, tRect.top, (unsigned char *)setup.myPortraitData);
        DisplayPortraitData( tRect.left, tRect.top, (unsigned char *)setup.myPortraitData, kMyCommColor);
        UpdateSampleShip( &setup, 0);
        UpdateSampleShip( &setup, 1);
        UpdatePlayerData( &setup, 0);
        UpdatePlayerData( &setup, 1);

/*      while ( GetPortraitHRun(  &setup.outPortX, &setup.outPortY, &runColor, &runLen,
            (unsigned char *)setup.myPortraitData))
        {
            AddPortraitHRun( &setup.inPortX, &setup.inPortY, runColor, runLen,
                (unsigned char *)setup.opponentPortraitData);
        }
        GetAnyInterfaceItemContentBounds( GetAnyInterfaceItemPtr( kFirstPlayerItemGroup +
            (setup.opponentNum * kPlayerItemGroupSize) + kPlayerGroupPortraitNum), &tRect);
        DisplayPortraitData( tRect.left, tRect.top, (unsigned char *)setup.opponentPortraitData);
        setup.inPortX                   = 0;
        setup.inPortY                   = 0;
        setup.outPortX                  = 0;
        setup.outPortY                  = 0;
*/
        dummyRequestTime = setup.lastSampleTime = TickCount();
        while ( !done)
        {
            if (( setup.imReady) && ( setup.opponentReady)) done = true;
            if ( setup.opponentReady)
            {
                if ( (TickCount() - lastFlashTime) > 12)
                {
                    anItem = GetAnyInterfaceItemPtr( kNetLevelOKButton);
                    if ( anItem->color != beginButtonRealColor)
                    {
                        anItem->color = beginButtonRealColor;
                        SetStatusOfAnyInterfaceItem( kNetLevelOKButton, kActive, true);
                    } else
                    {
                        anItem->color = GOLD;
                        SetStatusOfAnyInterfaceItem( kNetLevelOKButton, kIH_Hilite, true);
                    }
                    lastFlashTime = TickCount();
                }
            }

            InterfaceIdle();
            Ares_WaitNextEvent (everyEvent, &theEvent, 3, nil);
            {
                whichItem = -1;
                switch ( theEvent.what )
                {
                    case nullEvent:
                        if ( gAresGlobal->returnToMain)
                        {
                            whichItem = kNetLevelCancelButton;
                            break;
                        }
                        if ( gAresGlobal->gOptions & kOptionInBackground)
                            whichItem = -1;
                        else
                        {
                            where = theEvent.where;
                            GlobalToLocal( &where);
                            whichItem = PtInInterfaceItem( where);
                        }
                        if ( whichItem >= 0)
                        {
                            if ( whichItem >= kNetLevelItemNum)
                            {
                                switch( setup.whichTab)
                                {
                                    case kNetLevelLevelTabNum:
                                        whichItem = whichItem - kNetLevelItemNum +
                                            kNetLevelLevelStrNum;
                                        break;

                                    case kNetLevelPlayerTabNum:
                                        whichItem = whichItem - kNetLevelItemNum +
                                            kNetLevelPlayerStrNum;
                                        break;

                                    case kNetLevelSettingTabNum:
                                        whichItem = whichItem - kNetLevelItemNum +
                                            kNetLevelSettingsStrNum;
                                        break;
                                }
                            } else whichItem += 1;

                            if ( whichItem != helpItem)
                            {
                                GetIndString( tstring, kNetLevelHelpStrID, whichItem);
                                DrawStringInInterfaceItem( kNetLevelHelpBox, tstring);
                                helpItem = whichItem;
                            }
                            whichItem = -1;
                        } else if ( whichItem != helpItem)
                        {
                            DrawStringInInterfaceItem( kNetLevelHelpBox, nil);
                            helpItem = whichItem;
                        }

                        if ( setup.incomingCharBottom != setup.incomingCharTop)
                        {
                            PlayVolumeSound( kComputerBeep3, kMediumLowVolume, kShortPersistence, kMustPlaySound);
                            AddCharToVisibleString( &setup, setup.opponentNum,
                                setup.incomingChar[setup.incomingCharBottom], true);
                            mIncreaseBufferCounter(setup.incomingCharBottom);
                        }

                        if (( (TickCount() - dummyRequestTime) > waitTime) && ( messageToResend))
                        {
/*                          setup.sampleLag[setup.sampleNum % kTimeSampleNum] = TickCount() -
                                setup.lastSampleTime;
                            setup.sampleNum++;
                            setup.sampleLag[setup.sampleNum % kTimeSampleNum] = TickCount() -
                                setup.lastSampleTime;
                            NetLevelDrawLagGraph( &setup);
*/
                            if ( setup.currentRegistered > 0) waitTime = 600;

                            if (( !IAmHosting()) && ( setup.sampleNum > 0))
                            {
//                              SendPreGameDummyMessage( setup.sampleNum - 1,
//                                  setup.currentRegistered > 0, 1);
                                SendPreGameAnyMessage( setup.sampleNum - 1,
                                    setup.currentRegistered > 0, eDummyMessage,
                                    0, 0, 0, 0, true);
                            } else
                            {
//                              SendPreGameDummyMessage( setup.sampleNum,
//                                  setup.currentRegistered > 0, 1);
                                SendPreGameAnyMessage( setup.sampleNum,
                                    setup.currentRegistered > 0, eDummyMessage,
                                    0, 0, 0, 0, true);
                            }
                            dummyRequestTime = TickCount();
                        }
//                      TickleOutgoingMessage( true);
                        InterfaceIdle();
                        gotDummy = false;
                        do
                        {
                            message = ProcessPreGameMessages( &(setup.retroTextSpec.text),
                                &newLevel, &longData2, &longData3, &longData4, nil,
                                setup.sampleNum, &val1, &val2, &val3, &val4);
                            if ( setup.retroTextSpec.text != nil) ShowNetTextMessage( &setup, false);
                            switch( message)
                            {
                                case eNoMessage:
                                    break;

                                case eStartToLoadLevelMessage:
                                    setup.opponentReady = true;
                                    break;

                                case ePreGameOpenScenarioMessage:
                                    OpenOtherScenarioFile( &setup);
                                    break;

                                case ePreGameGotScenarioMessage:
                                    if ( setup.opponentHasArrived)
                                    {
                                        SetStatusOfAnyInterfaceItem(
                                            kNetLevelOKButton, kActive, true);
                                        setup.scenarioLoaded = true;
                                    }
                                    DrawStringInInterfaceItem(
                                        kWhyDisabledNetLevelText, nil);
                                    SetNetLevelNum( &setup);
                                    SendPreGameShortMessage( eSetLevelMessage, setup.thisChapter);
                                    SetCurrentNetRace( &setup);
                                    SendPreGameShortMessage( eSetRaceMessage, setup.myRace);
                                    break;

                                case ePreGameNoScenarioMessage:
                                    GetIndString( whyDisabledString, 2010, 2);
                                    CopyPString( gAresGlobal->otherPlayerScenarioFileName,
                                        gAresGlobal->externalFileSpec.name);
                                    ShowWhyNetLevelNotLoaded( &setup,
                                        whyDisabledString);
                                    break;

                                case ePreGameOldVersionScenarioMessage:
                                    GetIndString( whyDisabledString, 2010, 3);
                                    CopyPString( gAresGlobal->otherPlayerScenarioFileName,
                                        gAresGlobal->externalFileSpec.name);
                                    ShowWhyNetLevelNotLoaded( &setup,
                                        whyDisabledString);
                                    break;

                                case ePreGameNewVersionScenarioMessage:
                                    if ( gAresGlobal->internetConfigPresent)
                                    {
                                        GetIndString( whyDisabledString, 2010, 7);
                                    } else
                                    {
                                        GetIndString( whyDisabledString, 2010, 4);
                                        ConcatenatePString( whyDisabledString,
                                            gAresGlobal->scenarioFileInfo.downloadURLString);
                                        ConcatenatePString( whyDisabledString, "\p.");
                                    }
                                    CopyPString( gAresGlobal->otherPlayerScenarioFileName,
                                        gAresGlobal->externalFileSpec.name);
                                    ShowWhyNetLevelNotLoaded( &setup,
                                        whyDisabledString);
                                    break;

                                case ePreGameWrongCheckSumScenarioMessage:
                                    if ( gAresGlobal->internetConfigPresent)
                                    {
                                        GetIndString( whyDisabledString, 2010, 8);
                                    } else
                                    {
                                        GetIndString( whyDisabledString, 2010, 5);
                                        ConcatenatePString( whyDisabledString,
                                            gAresGlobal->scenarioFileInfo.downloadURLString);
                                        ConcatenatePString( whyDisabledString, "\p.");
                                    }
                                    CopyPString( gAresGlobal->otherPlayerScenarioFileName,
                                        gAresGlobal->externalFileSpec.name);
                                    ShowWhyNetLevelNotLoaded( &setup,
                                        whyDisabledString);
                                    break;

                                case eCancelMessage:
                                case kNSpPlayerLeft:
                                    done = true;
                                    cancel = true;

                                    StopNetworking();

                                    break;

                                case eReadyForNetSetupMessage:
                                    if ( !setup.opponentHasArrived)
                                    {
                                        if ( IAmHosting())
                                        {
                                            SendPreGameShortMessage( eAdmiralNumberMessage, setup.myNum);
                                            SendPreGameShortMessage( eSetLatencyMessage, setup.currentLatency);
                                            SendPreGameShortMessage( eSetRegisteredStateMessage, GetRegisteredSetting());
                                            SendPreGameShortMessage( eSetResendDelayMessage, GetResendDelay());
                                            SendPreGameShortMessage( eSetBandwidthMessage, GetBandwidth());
                                            SendPreGameShortMessage( eClientMakeChangesMessage, setup.clientCanMakeChanges);
                                            SendPreGameShortMessage( eSetRaceMessage, setup.myRace);
                                            SendPreGameShortMessage( eSetLevelMessage, setup.thisChapter);
                                            messageToResend = true;
                                            SendPreGameAnyMessage( setup.sampleNum, setup.currentRegistered > 1,
                                                 eDummyMessage, 0, 0, 0, 0, false);
                                        } else
                                        {
                                            SendPreGameShortMessage( eSetRaceMessage, setup.myRace);
                                        }
                                        SendPreGameVerboseMessage( eRelayPlayerStatsMessage, GetNetMinutesPlayed(),
                                            GetNetKills(), GetNetLosses(), 0);
                                        setup.opponentHasArrived = true;

                                        switch( setup.whichTab)
                                        {
                                            case kNetLevelLevelTabNum:
                                                ActivateLevelTab( &setup, true);
                                                break;

                                            case kNetLevelPlayerTabNum:
                                                ActivatePlayerTab( &setup, true);
                                                break;

                                            case kNetLevelSettingTabNum:
                                                ActivateSettingTab( &setup, true);
                                                break;
                                        }
                                    }
                                    break;

                                case eAdmiralNumberMessage:
                                    UpdatePlayerData( &setup, setup.opponentNum);
                                    break;

                                case eSetLevelMessage:
                                    setup.thisChapter = newLevel;
                                    SetNetLevelNum( &setup);
                                    SetCurrentNetRace( &setup);
                                    SendPreGameShortMessage( eSetRaceMessage, setup.myRace);
                                    break;

                                case eSetLatencyMessage:
                                    setup.currentLatency = newLevel;
                                    SetNetLag( &setup);
                                    break;

                                case eHostIsPlayer2Message:
                                    setup.hostIsPlayer2 = newLevel;
                                    SetNetHostIsPlayer2( &setup);
                                    break;

                                case eSetRaceMessage:
                                    setup.opponentRace = newLevel;
                                    SetCurrentNetRace( &setup);
                                    break;

                                case eSetRegisteredStateMessage:
                                    SetNetRegistered( &setup, newLevel);
                                    break;

                                case eSetResendDelayMessage:
                                    SetNetDelay( &setup, newLevel);
                                    break;

                                case eSetBandwidthMessage:
                                    setup.currentBandwidth = newLevel;
                                    SetNetLowerBandwidth( &setup);
                                    break;

                                case eTextMessage:
//                                  ShowNetTextMessage( &setup, false);
                                    break;

                                case eRelayPlayerStatsMessage:
                                    setup.opponentMinutesPlayed = newLevel;
                                    setup.opponentKills = longData2;
                                    setup.opponentLosses = longData3;
                                    UpdatePlayerData( &setup, setup.opponentNum);
                                    break;

                                case eDummyMessage: case ePreGameCharacterMessage:
                                case ePreGamePortraitMessage:
                                    if (( newLevel == setup.sampleNum) && ( !gotDummy))
                                    {
                                        setup.sampleLag[setup.sampleNum % kTimeSampleNum] = TickCount() -
                                            setup.lastSampleTime;
                                        if ( IAmHosting())
                                        {
                                            setup.sampleNum++;
                                            setup.sampleLag[setup.sampleNum % kTimeSampleNum] = TickCount() -
                                                setup.lastSampleTime;
                                            dummyRequestTime = setup.lastSampleTime = TickCount();
                                            NetLevelDrawLagGraph( &setup);
                                        }
                                        setup.sampleLag[setup.sampleNum % kTimeSampleNum] = TickCount() -
                                            setup.lastSampleTime;
                                        dummyRequestTime = setup.lastSampleTime = TickCount();
                                        NetLevelDrawLagGraph( &setup);
                                        gotDummy = true;

                                        if ( message == ePreGameCharacterMessage)
                                        {
                                            setup.incomingChar[setup.incomingCharTop] =
                                                val1;
                                            mIncreaseBufferCounter(setup.incomingCharTop);
                                            if ( val2 != 0)
                                            {
                                                setup.incomingChar[setup.incomingCharTop] =
                                                    val2;
                                                mIncreaseBufferCounter(setup.incomingCharTop);
                                                if ( val3 != 0)
                                                {
                                                    setup.incomingChar[setup.incomingCharTop] =
                                                        val3;
                                                    mIncreaseBufferCounter(setup.incomingCharTop);

                                                    if ( val4 != 0)
                                                    {
                                                        setup.incomingChar[setup.incomingCharTop] =
                                                            val4;
                                                        mIncreaseBufferCounter(setup.incomingCharTop);
                                                    }
                                                }
                                            }
                                        } else if ( message == ePreGamePortraitMessage)
                                        {
                                            mWriteDebugString("\p<PORT");
                                            WriteDebugLong( newLevel);
                                            WriteDebugLong( val1 + val2);
                                            runColor = val1 >> 4;
                                            runColor &= 0x0f;
                                            runLen = val1 & 0x0f;
                                            AddPortraitHRun( &setup.inPortX,
                                                &setup.inPortY, runColor, runLen,
                                                (unsigned char *)setup.opponentPortraitData);
                                            if ( val2 != 0)
                                            {
                                                runColor = val2 >> 4;
                                                runColor &= 0x0f;
                                                runLen = val2 & 0x0f;
                                                AddPortraitHRun( &setup.inPortX,
                                                    &setup.inPortY, runColor, runLen,
                                                    (unsigned char *)setup.opponentPortraitData);

                                                if ( val3 != 0)
                                                {
                                                    runColor = val3 >> 4;
                                                    runColor &= 0x0f;
                                                    runLen = val3 & 0x0f;
                                                    AddPortraitHRun( &setup.inPortX,
                                                        &setup.inPortY, runColor, runLen,
                                                        (unsigned char *)setup.opponentPortraitData);

                                                    if ( val4 != 0)
                                                    {
                                                        runColor = val4 >> 4;
                                                        runColor &= 0x0f;
                                                        runLen = val4 & 0x0f;
                                                        AddPortraitHRun( &setup.inPortX,
                                                            &setup.inPortY, runColor, runLen,
                                                            (unsigned char *)setup.opponentPortraitData);
                                                    }
                                                }
                                            }
                                            GetAnyInterfaceItemContentBounds(
                                                GetAnyInterfaceItemPtr( kFirstPlayerItemGroup +
                                                    (setup.opponentNum * kPlayerItemGroupSize) +
                                                    kPlayerGroupPortraitNum),
                                                &tRect);
                                            DisplayPortraitData( tRect.left, tRect.top,
                                                (unsigned char *)setup.opponentPortraitData,
                                                kOpponentCommColor);

                                        }
                                    } else if ( newLevel < setup.sampleNum)
                                    {
//                                      SendPreGameDummyMessage( newLevel,
//                                          setup.currentRegistered > 0, -1);
                                    }
                                    break;

                                case eClientMakeChangesMessage:
                                    if (( !IAmHosting()) && ( !setup.imReady))
                                    {
                                        setup.clientCanMakeChanges = newLevel;
                                        SetNetLevelCanMakeChanges( &setup, newLevel);
                                        if ( setup.clientCanMakeChanges)
                                        {
                                            SwitchAnyRadioOrCheckbox( kNetLevelChangesCheck,
                                                true);
                                        } else
                                        {
                                            SwitchAnyRadioOrCheckbox( kNetLevelChangesCheck,
                                                false);
                                        }
                                        RefreshInterfaceItem( kNetLevelChangesCheck);
                                    }
                                    break;
                            }
                        } while (( message != eNoMessage) && ( NetGameIsOn())
                            && ( !gotDummy) &&
                            (!(( setup.imReady) && ( setup.opponentReady))));
                        if ( gotDummy)
                        {
                            waitTime = 60;
//                          SendPreGameDummyMessage( setup.sampleNum,
//                              setup.currentRegistered > 1, 0);
                            if ( setup.outgoingCharBottom !=
                                setup.outgoingCharTop)
                            {
                                val1 = val2 = val3 = val4 = 0;
                                val1 = setup.outgoingChar[setup.outgoingCharBottom];
                                mIncreaseBufferCounter( setup.outgoingCharBottom);
                                if( setup.outgoingCharBottom != setup.outgoingCharTop)
                                {
                                    val2 = setup.outgoingChar[setup.outgoingCharBottom];
                                    mIncreaseBufferCounter( setup.outgoingCharBottom);

                                    if( setup.outgoingCharBottom != setup.outgoingCharTop)
                                    {
                                        val3 = setup.outgoingChar[setup.outgoingCharBottom];
                                        mIncreaseBufferCounter( setup.outgoingCharBottom);

                                        if( setup.outgoingCharBottom != setup.outgoingCharTop)
                                        {
                                            val4 = setup.outgoingChar[setup.outgoingCharBottom];
                                            mIncreaseBufferCounter( setup.outgoingCharBottom);
                                        }
                                    }
                                }
                                messageToResend = true;
                                SendPreGameAnyMessage( setup.sampleNum,
                                    setup.currentRegistered > 1,
                                    ePreGameCharacterMessage,
                                    val1, val2, val3, val4, false);
//                                  AddCharToVisibleString( &setup, setup.myNum, setup.outgoingChar[setup.outgoingCharBottom]);
                            } else if ( GetPortraitHRun( &setup.outPortX, &setup.outPortY,
                                &runColor, &runLen,
                                (unsigned char *)setup.myPortraitData))
                            {
                                val1 = val2 = val3 = val4 = 0;
                                val1 = runColor << 4;
                                val1 &= 0xf0;
                                val1 |= runLen;
                                if ( GetPortraitHRun( &setup.outPortX, &setup.outPortY,
                                        &runColor, &runLen,
                                        (unsigned char *)setup.myPortraitData))
                                {
                                    val2 = runColor << 4;
                                    val2 &= 0xf0;
                                    val2 |= runLen;

                                    if ( GetPortraitHRun( &setup.outPortX, &setup.outPortY,
                                            &runColor, &runLen,
                                            (unsigned char *)setup.myPortraitData))
                                    {
                                        val3 = runColor << 4;
                                        val3 &= 0xf0;
                                        val3 |= runLen;

                                        if ( GetPortraitHRun( &setup.outPortX, &setup.outPortY,
                                                &runColor, &runLen,
                                                (unsigned char *)setup.myPortraitData))
                                        {
                                            val4 = runColor << 4;
                                            val4 &= 0xf0;
                                            val4 |= runLen;
                                        }
                                    }
                                }

                                messageToResend = true;
                                SendPreGameAnyMessage( setup.sampleNum,
                                    setup.currentRegistered > 1,
                                    ePreGamePortraitMessage,
                                    val1, val2, val3, val4, false);

                            } else
                            {
                                messageToResend = true;
                                SendPreGameAnyMessage( setup.sampleNum,
                                    setup.currentRegistered > 1, eDummyMessage,
                                    0, 0, 0, 0, false);
                            }
                            if ( !IAmHosting())
                            {
                                setup.sampleNum++;
                                setup.sampleLag[setup.sampleNum % kTimeSampleNum] = TickCount() -
                                    setup.lastSampleTime;
                                dummyRequestTime = setup.lastSampleTime = TickCount();
                                NetLevelDrawLagGraph( &setup);
                            }
                        }

                        if ( gAresGlobal->gOptions & kOptionInBackground)
                        {
                        } else
                        {
                            if (AutoShowHideMenubar( theEvent.where, theDevice))
                            {
                            }
                        }
                        break;
                    case osEvt:
//                      HandleOSEvent( &theEvent);
                        break;
                    case updateEvt:
                        whichWindow = ( CWindowPtr)theEvent.message;

                        if ( whichWindow == gTheWindow)
                        {
                            BeginUpdate( (WindowPtr)whichWindow);
                                SetPort( (WindowPtr)gTheWindow);
                                CopyOffWorldToRealWorld((WindowPtr)gTheWindow, &(gTheWindow->portRect));
                            EndUpdate( (WindowPtr)whichWindow);
                            break;
                            EndUpdate( (WindowPtr)whichWindow);
                        } else if ( whichWindow == gAresGlobal->gBackWindow)
                        {
                            BeginUpdate( (WindowPtr)whichWindow);
                                SetPort( (WindowPtr)gAresGlobal->gBackWindow);
                                FillRect(  &(gAresGlobal->gBackWindow->portRect), (Pattern *)&qd.black);
                            EndUpdate( (WindowPtr)whichWindow);
                        } else
                        {
                            BeginUpdate( (WindowPtr)whichWindow);
                            EndUpdate( (WindowPtr)whichWindow);
                        }
                        SetPort( (WindowPtr)gTheWindow);

                        break;

                    case mouseDown:
                        where = theEvent.where;
                        GlobalToLocal( &where);
                        HandleMouseDown( &theEvent);
                        whichItem = InterfaceMouseDown( where);

                        switch( setup.whichTab)
                        {
                            case kNetLevelLevelTabNum:
                                HandleLevelTabHit( whichItem, where, &setup);
                                break;

                            case kNetLevelPlayerTabNum:
                                HandlePlayerTabHit( whichItem, where, &setup);
                                break;

                            case kNetLevelSettingTabNum:
                                HandleSettingTabHit( whichItem, where,
                                    &setup);
                                break;
                        }
                        break;
                    case mouseUp:
                        break;
                    case keyDown:
                    case autoKey:
                        whichChar = theEvent.message & charCodeMask;
                        whichItem = InterfaceKeyDown( theEvent.message);
                        where.h = where.v = 0xffff;

                        switch( setup.whichTab)
                        {
                            case kNetLevelLevelTabNum:
                                HandleLevelTabHit( whichItem, where, &setup);
                                break;

                            case kNetLevelPlayerTabNum:
                                HandlePlayerTabHit( whichItem, where, &setup);
                                break;

                            case kNetLevelSettingTabNum:
                                HandleSettingTabHit( whichItem, where,
                                    &setup);
                                break;
                        }
                        if ( whichItem < 0)
                        {
                            setup.outgoingChar[setup.outgoingCharTop] = whichChar;
                            AddCharToVisibleString( &setup, setup.myNum, whichChar, true);
                            mIncreaseBufferCounter(setup.outgoingCharTop);
                            PlayVolumeSound(  kTeletype, kMediumLowVolume, kShortPersistence, kLowPrioritySound);
                        }
                        break;
                }
                switch ( whichItem)
                {
                    case kNetLevelCancelButton:
                        done = TRUE;
                        cancel = TRUE;
                        SendPreGameBasicMessage( eCancelMessage);
                        StopNetworking();
                        break;

                    case kNetLevelOKButton:
                        setup.imReady = true;
                        setup.canMakeChanges = false;
                        SendPreGameBasicMessage( eStartToLoadLevelMessage);
                        SetStatusOfAnyInterfaceItem( kNetLevelOKButton, kDimmed, true);
                        SetNetLevelCanMakeChanges( &setup, false);
                        break;

/*                  case kNetLevelSendTextButton:
                        SendNetTextMessage( &setup);
                        break;
*/
                    case kNetLevelChangesCheck:
                        if ( GetAnyRadioOrCheckboxOn( kNetLevelChangesCheck))
                        {
                            setup.clientCanMakeChanges = true;
                            SendPreGameShortMessage( eClientMakeChangesMessage, true);
                        } else
                        {
                            setup.clientCanMakeChanges = false;
                            SendPreGameShortMessage( eClientMakeChangesMessage, false);
                        }
                        break;

                    case kNetLevelLevelTabNum:
                    case kNetLevelPlayerTabNum:
                    case kNetLevelSettingTabNum:
                        switch( setup.whichTab)
                        {
                            case kNetLevelLevelTabNum:
                                break;

                            case kNetLevelPlayerTabNum:
                                break;

                            case kNetLevelSettingTabNum:
                                break;
                        }

                        ShortenInterface( tabItemNum);
                        SwitchAnyRadioOrCheckbox( setup.whichTab, false);
                        DrawAnyInterfaceItemOffToOn( GetAnyInterfaceItemPtr( setup.whichTab));

                        setup.whichTab = whichItem;
                        switch( setup.whichTab)
                        {
                            case kNetLevelLevelTabNum:
                                tabItemNum = AppendInterface( kNetLevelLevelTabID,
                                    kNetLevelTabBox, true);
                                ActivateLevelTab( &setup, false);
                                DrawInterfaceRange( kNetLevelItemNum,
                                    kNetLevelItemNum + tabItemNum, kNetLevelTabBox);
                                if ( setup.scenarioLoaded)
                                    ShowNetLevelName( setup.thisChapter);
                                else ShowWhyNetLevelNotLoaded( &setup, nil);
                                if (( !Ambrosia_Is_Registered()) || ( GetOpponentIsUnregistered()))
                                {
                                    GetIndString( tstring, 2010, 1);
                                    DrawStringInInterfaceItem( kWhyDisabledNetLevelText, tstring);
                                }
                                break;

                            case kNetLevelPlayerTabNum:
                                tabItemNum = AppendInterface( kNetLevelPlayerTabID,
                                    kNetLevelTabBox, true);
                                scenario = GetScenarioPtrFromChapter( setup.thisChapter);
                                ActivatePlayerTab( &setup, false);
                                DrawInterfaceRange( kNetLevelItemNum,
                                    kNetLevelItemNum + tabItemNum, kNetLevelTabBox);
                                NetLevelShowRaces( setup.myRace, setup.opponentRace);
                                where.h = where.v = 0;
                                setup.opponentColor = NetLevelSetColor( &setup,
                                    setup.opponentColor,
                                    GetApparentColorFromRace( setup.myRace), where,
                                    setup.myRace == setup.opponentRace, false);
                                UpdateSampleShip( &setup, setup.opponentNum);
                                break;

                            case kNetLevelSettingTabNum:
                                tabItemNum = AppendInterface(
                                    kNetLevelSettingTabID, kNetLevelTabBox,
                                        false);
                                ActivateSettingTab( &setup, false);
                                DrawInterfaceRange( kNetLevelItemNum,
                                    kNetLevelItemNum + tabItemNum, kNetLevelTabBox);
                                //NetLevelShowLag( setup.currentLatency / kMessageLatencyUnit, setup.recommendedLatency / kMessageLatencyUnit);
                                break;
                        }
                }

            }

        }
        if ( !cancel)
        {
            SetRegisteredSetting( setup.currentRegistered);
            SetBandwidth( setup.currentBandwidth);
            SetResendDelay( setup.currentDelayValue);
            SetPlayerRace( setup.myNum, setup.myRace);
            SetPlayerColor( setup.myNum, 0);
            SetPlayerRace( setup.opponentNum, setup.opponentRace);
            SetPlayerColor( setup.opponentNum, setup.opponentColor);
            gAresGlobal->gPlayerAdmiralNumber = setup.myNum;
            gNetLatency = setup.currentLatency;
            SetHaveSeenUnregisteredTimeLimitWarning( false);

        } else
        {
            setup.thisChapter = -1;
            gAresGlobal->externalFileSpec.name[0] = 0;
            EF_OpenExternalFile();
        }
        CloseInterface();
    }
    DisposePtr( setup.myPortraitData);
    DisposePtr( setup.opponentPortraitData);
    mWriteDebugString("\pLEVEL:");
    WriteDebugLong( setup.thisChapter);
    return( setup.thisChapter);
}

void SetNetLevelCanMakeChanges( netSetupType *setup, Boolean canMakeChanges)
{
    interfaceItemType   *tabBox = GetAnyInterfaceItemPtr( kNetLevelTabBox);
    Rect                    tabBounds;

    GetAnyInterfaceItemContentBounds( tabBox, &tabBounds);
    setup->canMakeChanges = canMakeChanges;
    switch( setup->whichTab)
    {
        case kNetLevelLevelTabNum:
            ActivateLevelTab( setup, true);
            break;

        case kNetLevelPlayerTabNum:
            ActivatePlayerTab( setup, true);
            break;

        case kNetLevelSettingTabNum:
            ActivateSettingTab( setup, true);
            break;
    }
//  CopyOffWorldToRealWorld( (WindowPtr)gTheWindow, &tabBounds);
}

void ActivateLevelTab( netSetupType *setup, Boolean drawLive)
{
    Str255          whyDisabledString;

if (( !Ambrosia_Is_Registered()) || ( GetOpponentIsUnregistered()))
{
        SetStatusOfAnyInterfaceItem( kNextNetLevelButton, kDimmed, drawLive);
        SetStatusOfAnyInterfaceItem( kPreviousNetLevelButton, kDimmed, drawLive);
        SetStatusOfAnyInterfaceItem( kOpenNetLevelButton, kDimmed, drawLive);
        if ( drawLive)
        {
            GetIndString( whyDisabledString, 2010, 1);
            DrawStringInInterfaceItem( kWhyDisabledNetLevelText, whyDisabledString);
        }
} else
{
    if (( !setup->canMakeChanges) || ( !setup->opponentHasArrived) ||
        (!setup->scenarioLoaded))
    {
        SetStatusOfAnyInterfaceItem( kNextNetLevelButton, kDimmed, drawLive);
        SetStatusOfAnyInterfaceItem( kPreviousNetLevelButton, kDimmed, drawLive);
        if (( setup->canMakeChanges) && ( setup->opponentHasArrived))
            SetStatusOfAnyInterfaceItem( kOpenNetLevelButton, kActive, true);
        else
            SetStatusOfAnyInterfaceItem( kOpenNetLevelButton, kDimmed, true);
    } else
    {
        if ( GetNextNetworkScenario( setup->thisChapter) <= setup->thisChapter)
        {
            SetStatusOfAnyInterfaceItem( kNextNetLevelButton, kDimmed, drawLive);
        } else
        {
            SetStatusOfAnyInterfaceItem( kNextNetLevelButton, kActive, drawLive);
        }
        if ( GetPreviousNetworkScenario( setup->thisChapter) >= setup->thisChapter)
        {
            SetStatusOfAnyInterfaceItem( kPreviousNetLevelButton, kDimmed, drawLive);
        } else
        {
            SetStatusOfAnyInterfaceItem( kPreviousNetLevelButton, kActive, drawLive);
        }
        if (( setup->canMakeChanges) && ( setup->opponentHasArrived))
            SetStatusOfAnyInterfaceItem( kOpenNetLevelButton, kActive, true);
        else
            SetStatusOfAnyInterfaceItem( kOpenNetLevelButton, kDimmed, true);
    }

    if (( GetScenarioPlayerNum(
        GetScenarioNumberFromChapterNumber( setup->thisChapter)) <= 0) ||
        ( !setup->opponentHasArrived) || (setup->imReady) ||
        (!setup->scenarioLoaded))
        SetStatusOfAnyInterfaceItem( kNetLevelOKButton, kDimmed, drawLive);
    else
        SetStatusOfAnyInterfaceItem( kNetLevelOKButton, kActive, drawLive);

        if ( drawLive)
        {
            GetIndString( whyDisabledString, 2010, 1);
            DrawStringInInterfaceItem( kWhyDisabledNetLevelText, nil);
        }
}

}

void HandleLevelTabHit( short whichItem, Point where, netSetupType *setup)
{
    scenarioType        *scenario = nil;
    interfaceItemType   *item;
    Rect                tRect;

    item = GetAnyInterfaceItemPtr( kNetLevelBox);
    GetAnyInterfaceItemGraphicBounds( item, &tRect);

    if ( MacPtInRect( where, &tRect))
    {
        if (( gAresGlobal->otherPlayerScenarioFileURL[0] > 0) &&
            ( !setup->scenarioLoaded))
            LaunchURL( gAresGlobal->otherPlayerScenarioFileURL);

    } else switch( whichItem)
    {
        case kNextNetLevelButton:
            if ( GetNextNetworkScenario( setup->thisChapter) > setup->thisChapter)
            {
                setup->thisChapter = GetNextNetworkScenario( setup->thisChapter);
                SetNetLevelNum( setup);
            }

            SendPreGameShortMessage( eSetRaceMessage, setup->myRace);
            SendPreGameShortMessage( eSetLevelMessage, setup->thisChapter);
            break;

        case kPreviousNetLevelButton:
            if ( GetPreviousNetworkScenario( setup->thisChapter) < setup->thisChapter)
            {
                setup->thisChapter = GetPreviousNetworkScenario( setup->thisChapter);
                SetNetLevelNum( setup);
            }

            SendPreGameShortMessage( eSetRaceMessage, setup->myRace);
            SendPreGameShortMessage( eSetLevelMessage, setup->thisChapter);
            break;

        case kOpenNetLevelButton:
        {
            FSSpec  destFile;

            if ( SmartFile_SelectFile( &destFile, 301) == noErr)
            {
                BlockMove( &destFile, &gAresGlobal->externalFileSpec,
                    sizeof( FSSpec));

                if ( EF_OpenExternalFile() == noErr)
                {
                    if ( !ThisChapterIsNetworkable( setup->thisChapter))
                    {
                        setup->thisChapter = GetFirstNetworkScenario();
                        if ( setup->thisChapter < 0)
                        {
                            ShowErrorAny( eContinueOnlyErr, -1,
                                "\pThat scenario file contains no networkable",
                                "\p scenarios. Reverting to built-in scenarios.",
                                nil, nil, -1, -1, -1, -1, __FILE__, 1);
                            gAresGlobal->externalFileSpec.name[0] = 0;
                            EF_OpenExternalFile();
                            setup->thisChapter = GetFirstNetworkScenario();
                        }
                    }
                    scenario = GetScenarioPtrFromChapter( setup->thisChapter);

                    setup->scenarioLoaded = false;
                    SetStatusOfAnyInterfaceItem( kNetLevelOKButton,
                        kDimmed, true);
                    SendPreGameOpenScenarioMessage(
                        ePreGameOpenScenarioMessage,
                        gAresGlobal->externalFileSpec.name,
                        gAresGlobal->scenarioFileInfo.downloadURLString,
                        gAresGlobal->scenarioFileInfo.version,
                        gAresGlobal->scenarioFileInfo.checkSum);


/*                  if ( !IsRaceLegal( setup->myRace, setup->myNum, scenario))
                    {
                        setup->myRace =
                            GetRaceNumFromID(
                                scenario->player[setup->myNum].playerRace);
                    }
*/
//                  SetNetLevelNum( setup);
//                  SendPreGameShortMessage( eSetLevelMessage, setup->thisChapter);

//                  SetCurrentNetRace( setup);
//                  SendPreGameShortMessage( eSetRaceMessage, setup->myRace);
                }

            }
        }
            break;
    }
}

void SetNetLevelNum( netSetupType *setup)
{
    Str255          chapterName;
    scenarioType    *scenario;

    if ( setup->whichTab == kNetLevelLevelTabNum)
    {
        GetScenarioName( GetScenarioNumberFromChapterNumber( setup->thisChapter),
            chapterName);
        if ( setup->scenarioLoaded)
        {
            DrawLevelNameInBox( chapterName, kButtonFontNum,
                GetScenarioPrologueID(
                    GetScenarioNumberFromChapterNumber(
                        setup->thisChapter)), kNetLevelBox);
        } else
        {
            ShowWhyNetLevelNotLoaded( setup, nil);
        }

        if (( !Ambrosia_Is_Registered()) || ( GetOpponentIsUnregistered()))
        {
            SetStatusOfAnyInterfaceItem( kNextNetLevelButton, kDimmed, true);
            SetStatusOfAnyInterfaceItem( kPreviousNetLevelButton, kDimmed, true);
            SetStatusOfAnyInterfaceItem( kOpenNetLevelButton, kDimmed, true);
        } else if (( !setup->canMakeChanges) || ( !setup->opponentHasArrived) ||
            ( !setup->scenarioLoaded))
        {
            SetStatusOfAnyInterfaceItem( kNextNetLevelButton, kDimmed, true);
            SetStatusOfAnyInterfaceItem( kPreviousNetLevelButton, kDimmed, true);
            if (( setup->canMakeChanges) && ( setup->opponentHasArrived))
                SetStatusOfAnyInterfaceItem( kOpenNetLevelButton, kActive, true);
            else
                SetStatusOfAnyInterfaceItem( kOpenNetLevelButton, kDimmed, true);
        } else
        {
            if ( GetNextNetworkScenario( setup->thisChapter) <= setup->thisChapter)
            {
                SetStatusOfAnyInterfaceItem( kNextNetLevelButton, kDimmed, true);
            } else
            {
                SetStatusOfAnyInterfaceItem( kNextNetLevelButton, kActive, true);
            }
            if ( GetPreviousNetworkScenario( setup->thisChapter) >= setup->thisChapter)
            {
                SetStatusOfAnyInterfaceItem( kPreviousNetLevelButton, kDimmed, true);
            } else
            {
                SetStatusOfAnyInterfaceItem( kPreviousNetLevelButton, kActive, true);
            }
            if (( setup->canMakeChanges) && ( setup->opponentHasArrived))
                SetStatusOfAnyInterfaceItem( kOpenNetLevelButton, kActive, true);
            else
                SetStatusOfAnyInterfaceItem( kOpenNetLevelButton, kDimmed, true);
        }
        if (( GetScenarioPlayerNum(
                GetScenarioNumberFromChapterNumber( setup->thisChapter)) <= 0)
                || ( !setup->opponentHasArrived) || ( setup->imReady) ||
                ( !setup->scenarioLoaded))
            SetStatusOfAnyInterfaceItem( kNetLevelOKButton, kDimmed, true);
        else
            SetStatusOfAnyInterfaceItem( kNetLevelOKButton, kActive, true);
    }

    scenario = GetScenarioPtrFromChapter( setup->thisChapter);
    if ( scenario != nil)
    {
        setup->myRace = GetNetRace();
        if ( !IsRaceLegal( setup->myRace, setup->myNum, scenario))
        {
            setup->myRace =
                GetRaceNumFromID( scenario->player[setup->myNum].playerRace);
        }
    }

}

void ActivatePlayerTab( netSetupType *setup, Boolean drawLive)
{
    scenarioType    *scenario = GetScenarioPtrFromChapter( setup->thisChapter);

    if (( !setup->canMakeChanges) || ( !setup->opponentHasArrived)
        || ( !setup->scenarioLoaded))
    {
if ( Ambrosia_Is_Registered())
{
        SetStatusOfAnyInterfaceItem( kNextRaceButton, kDimmed, drawLive);
        SetStatusOfAnyInterfaceItem( kPreviousRaceButton, kDimmed, drawLive);
        SetStatusOfAnyInterfaceItem( kHostIsPlayer2CheckBox, kDimmed, drawLive);
}
    } else
    {
        SetStatusOfAnyInterfaceItem( kHostIsPlayer2CheckBox, kActive, drawLive);
if ( Ambrosia_Is_Registered())
{
        if ( GetNextLegalRace( setup->myRace, setup->myNum, scenario) >= 0)
        {
            SetStatusOfAnyInterfaceItem( kNextRaceButton, kActive, drawLive);
        } else
        {
            SetStatusOfAnyInterfaceItem( kNextRaceButton, kDimmed, drawLive);
        }

        if ( GetPreviousLegalRace( setup->myRace, setup->myNum, scenario) >= 0)
        {
            SetStatusOfAnyInterfaceItem( kPreviousRaceButton, kActive, drawLive);
        } else
        {
            SetStatusOfAnyInterfaceItem( kPreviousRaceButton, kDimmed, drawLive);
        }
}
    }
    if ( !setup->hostIsPlayer2)
    {
        SwitchAnyRadioOrCheckbox( kHostIsPlayer2CheckBox, false);
    } else
    {
        SwitchAnyRadioOrCheckbox( kHostIsPlayer2CheckBox, true);
    }
if ( !Ambrosia_Is_Registered())
{
        SetStatusOfAnyInterfaceItem( kNextRaceButton, kDimmed, drawLive);
        SetStatusOfAnyInterfaceItem( kPreviousRaceButton, kDimmed, drawLive);
        SetStatusOfAnyInterfaceItem( kHostIsPlayer2CheckBox, kDimmed, drawLive);
}
}

void HandlePlayerTabHit( short whichItem, Point where, netSetupType *setup)
{
    scenarioType    *scenario = nil;
    Rect            colorRect;

    GetAnyInterfaceItemContentBounds( GetAnyInterfaceItemPtr( kNetLevelColorBox),
        &colorRect);
    if ( PtInRect( where, &colorRect))
    {
        setup->opponentColor = NetLevelSetColor( setup, setup->opponentColor,
            GetApparentColorFromRace( setup->myRace),
            where, setup->myRace == setup->opponentRace, true);
        UpdateSampleShip( setup, setup->opponentNum);
    }
    switch( whichItem)
    {
        case kNextRaceButton:
            scenario = GetScenarioPtrFromChapter( setup->thisChapter);
            if ( scenario != nil)
            {
                if ( GetNextLegalRace( setup->myRace, setup->myNum, scenario) >= 0)
                {
                    setup->myRace = GetNextLegalRace( setup->myRace, setup->myNum, scenario);
                    SetNetRace( setup->myRace);
                    SetCurrentNetRace( setup);
                    SendPreGameShortMessage( eSetRaceMessage, setup->myRace);
                }
            }
            break;

        case kPreviousRaceButton:
            scenario = GetScenarioPtrFromChapter( setup->thisChapter);
            if ( scenario != nil)
            {
                if ( GetPreviousLegalRace( setup->myRace, setup->myNum, scenario) >= 0)
                {
                    setup->myRace = GetPreviousLegalRace( setup->myRace, setup->myNum, scenario);
                    SetNetRace( setup->myRace);
                    SetCurrentNetRace( setup);
                    SendPreGameShortMessage( eSetRaceMessage, setup->myRace);
                }
            }
            break;

        case kHostIsPlayer2CheckBox:
            if ( GetAnyRadioOrCheckboxOn( kHostIsPlayer2CheckBox))
            {
                setup->hostIsPlayer2 = true;
                SendPreGameShortMessage( eHostIsPlayer2Message, true);
            } else
            {
                setup->hostIsPlayer2 = false;
                SendPreGameShortMessage( eHostIsPlayer2Message, false);
            }
            SetNetHostIsPlayer2( setup);
            break;
    }
}

void SetNetHostIsPlayer2( netSetupType *setup)
{
    scenarioType    *scenario;
    Str255          tempString;
    long                myID, opponentID;

    myID = GetPlayerIDFromNum( setup->myNum);
    opponentID = GetPlayerIDFromNum( setup->opponentNum);
    if (setup->whichTab == kNetLevelPlayerTabNum)
    {
        if ( setup->hostIsPlayer2)
        {
            SwitchAnyRadioOrCheckbox( kHostIsPlayer2CheckBox,
                true);
        } else
        {
            SwitchAnyRadioOrCheckbox( kHostIsPlayer2CheckBox,
                false);
        }
        RefreshInterfaceItem( kHostIsPlayer2CheckBox);
    }
    if ( IAmHosting())
    {
        if ( setup->hostIsPlayer2)
        {
            setup->myNum = 1;
            setup->opponentNum = 0;
        } else
        {
            setup->myNum = 0;
            setup->opponentNum = 1;
        }
    } else
    {
        if ( setup->hostIsPlayer2)
        {
            setup->myNum = 0;
            setup->opponentNum = 1;
        } else
        {
            setup->myNum = 1;
            setup->opponentNum = 0;
        }
    }
//  SendPreGameShortMessage( eAdmiralNumberMessage, setup->myNum);
//  AddPlayerID( MyPlayerID(), setup->myNum);
    AddPlayerID( myID, setup->myNum);
    AddPlayerID( opponentID, setup->opponentNum);
    scenario = GetScenarioPtrFromChapter( setup->thisChapter);
/*  setup->myRace = GetNetRace();
    if ( !(IsRaceLegal( setup->myRace, scenario)))
    {
        setup->myRace =
            GetRaceNumFromID( scenario->player[setup->myNum].playerRace);
    }
    SendPreGameShortMessage( eSetRaceMessage, setup->myRace);
*/  SetCurrentNetRace( setup);
    CopyPString( tempString, setup->visibleString[0]);
    CopyPString( setup->visibleString[0], setup->visibleString[1]);
    CopyPString( setup->visibleString[1], tempString);
    AddCharToVisibleString( setup, 0, 0, false);
    AddCharToVisibleString( setup, 1, 0, false);
    DrawPortraitByPlayerNum( setup, 0);
    DrawPortraitByPlayerNum( setup, 1);
}

void SetCurrentNetRace( netSetupType *setup)
{
    Boolean         tabActive = (setup->whichTab == kNetLevelPlayerTabNum);
    scenarioType    *scenario;
    Point           where;

    where.h = where.v = -1;

    UpdatePlayerData( setup, 0);
    UpdatePlayerData( setup, 1);

    if ( tabActive)
    {
        NetLevelShowRaces( setup->myRace, setup->opponentRace);

        scenario = GetScenarioPtrFromChapter( setup->thisChapter);
if ( (Ambrosia_Is_Registered()) && ( scenario != nil))
{
        if ( GetPreviousLegalRace( setup->myRace, setup->myNum, scenario) >= 0)
        {
            SetStatusOfAnyInterfaceItem( kPreviousRaceButton, kActive, true);
        } else
        {
            SetStatusOfAnyInterfaceItem( kPreviousRaceButton, kDimmed, true);
        }

        if ( GetNextLegalRace( setup->myRace, setup->myNum, scenario) >= 0)
        {
            SetStatusOfAnyInterfaceItem( kNextRaceButton, kActive, true);
        } else
        {
            SetStatusOfAnyInterfaceItem( kNextRaceButton, kDimmed, true);
        }
} else
{
        SetStatusOfAnyInterfaceItem( kNextRaceButton, kDimmed, true);
        SetStatusOfAnyInterfaceItem( kPreviousRaceButton, kDimmed, true);
}
    }
    setup->opponentColor = NetLevelSetColor( setup, setup->opponentColor,
        GetApparentColorFromRace( setup->myRace), where,
        setup->myRace == setup->opponentRace, false);
    UpdateSampleShip( setup, setup->opponentNum);
    UpdateSampleShip( setup, setup->myNum);
}

void ActivateSettingTab( netSetupType *setup, Boolean drawLive)
{
    short i;

    if (( !setup->canMakeChanges) || ( !setup->opponentHasArrived) ||
        ( !setup->scenarioLoaded))
    {
        for ( i = kNetSettingsFirstRegisteredRadio;
                i <= kNetSettingsLastRegisteredRadio; i++)
        {
            SetStatusOfAnyInterfaceItem( i, kDimmed, drawLive);
        }
        for ( i = kNetSettingsFirstDelayRadio;
                i <= kNetSettingsLastDelayRadio; i++)
        {
            SetStatusOfAnyInterfaceItem( i, kDimmed, drawLive);
        }
        SetStatusOfAnyInterfaceItem( kNetLevelLowerBandwidth, kDimmed, drawLive);
        SetStatusOfAnyInterfaceItem( kNetLevelDecreaseLagButton, kDimmed, drawLive);
        SetStatusOfAnyInterfaceItem( kNetLevelIncreaseLagButton, kDimmed, drawLive);
    } else
    {
        for ( i = kNetSettingsFirstRegisteredRadio;
                i <= kNetSettingsLastRegisteredRadio; i++)
        {
            SetStatusOfAnyInterfaceItem( i, kActive, drawLive);
        }
        for ( i = kNetSettingsFirstDelayRadio;
                i <= kNetSettingsLastDelayRadio; i++)
        {
            SetStatusOfAnyInterfaceItem( i, kActive, drawLive);
        }
        SetStatusOfAnyInterfaceItem( kNetLevelLowerBandwidth, kActive, drawLive);
        if  ( setup->currentLatency < kMessageLatencyUnit)
        {
            SetStatusOfAnyInterfaceItem( kNetLevelDecreaseLagButton, kDimmed, drawLive);
        } else
        {
            SetStatusOfAnyInterfaceItem( kNetLevelDecreaseLagButton, kActive, drawLive);
        }
        if  ( setup->currentLatency > ( kMaxMessageLatency - kMessageLatencyUnit))
        {
            SetStatusOfAnyInterfaceItem( kNetLevelIncreaseLagButton, kDimmed, drawLive);
        } else
        {
            SetStatusOfAnyInterfaceItem( kNetLevelIncreaseLagButton, kActive, drawLive);
        }
    }
    for ( i = kNetSettingsFirstRegisteredRadio;
            i <= kNetSettingsLastRegisteredRadio; i++)
    {
        if (( i - kNetSettingsFirstRegisteredRadio) == setup->currentRegistered)
        {
            SwitchAnyRadioOrCheckbox( i, true);
        } else
        {
            SwitchAnyRadioOrCheckbox( i, false);
        }
    }
    for ( i = kNetSettingsFirstDelayRadio;
            i <= kNetSettingsLastDelayRadio; i++)
    {
        if (( i - kNetSettingsFirstDelayRadio) == setup->currentDelay)
        {
            SwitchAnyRadioOrCheckbox( i, true);
        } else
        {
            SwitchAnyRadioOrCheckbox( i, false);
        }
    }

    SwitchAnyRadioOrCheckbox( kNetLevelLowerBandwidth, setup->currentBandwidth);
}

void HandleSettingTabHit( short whichItem, Point where, netSetupType *setup)
{
#pragma unused( where)
    switch( whichItem)
    {
        case kNetLevelDecreaseLagButton:
            if ( setup->currentLatency >= kMessageLatencyUnit)
            {
                setup->currentLatency -= kMessageLatencyUnit;
                SetNetLag( setup);
                SendPreGameShortMessage( eSetLatencyMessage, setup->currentLatency);
            }
            break;

        case kNetLevelIncreaseLagButton:
            if ( setup->currentLatency <=  (kMaxMessageLatency - kMessageLatencyUnit))
            {
                setup->currentLatency += kMessageLatencyUnit;
                SetNetLag( setup);
                SendPreGameShortMessage( eSetLatencyMessage, setup->currentLatency);
            }
            break;

        case kNetLevelLowerBandwidth:
            setup->currentBandwidth = !setup->currentBandwidth;
            SendPreGameShortMessage( eSetBandwidthMessage, setup->currentBandwidth);
            break;

        default:
            if (( whichItem >= kNetSettingsFirstRegisteredRadio) &&
                ( whichItem <= kNetSettingsLastRegisteredRadio))
            {
                SetNetRegistered( setup, whichItem - kNetSettingsFirstRegisteredRadio);
/*              SwitchAnyRadioOrCheckbox( setup->currentRegistered +
                    kNetSettingsFirstRegisteredRadio, false);
                RefreshInterfaceItem( setup->currentRegistered +
                    kNetSettingsFirstRegisteredRadio);
                setup->currentRegistered = whichItem - kNetSettingsFirstRegisteredRadio;
                SwitchAnyRadioOrCheckbox( setup->currentRegistered +
                    kNetSettingsFirstRegisteredRadio, true);
*/
// this must be modified to send registered setting, not registered "position"
                SendPreGameShortMessage( eSetRegisteredStateMessage, setup->currentRegistered);
            } else if (( whichItem >= kNetSettingsFirstDelayRadio) &&
                ( whichItem <= kNetSettingsLastDelayRadio))
            {
                SetNetDelay( setup, GetDelayValueFromDelayRadio( whichItem -
                        kNetSettingsFirstDelayRadio));
/*              SwitchAnyRadioOrCheckbox( setup->currentDelay +
                    kNetSettingsFirstDelayRadio, false);
                RefreshInterfaceItem( setup->currentDelay +
                    kNetSettingsFirstDelayRadio);
                setup->currentDelay = whichItem - kNetSettingsFirstDelayRadio;
                SwitchAnyRadioOrCheckbox( setup->currentDelay +
                    kNetSettingsFirstDelayRadio, true);
*/
// this must be modified to send delay in seconds, not delay "position"
                SendPreGameShortMessage( eSetResendDelayMessage, setup->currentDelayValue);
            }
            break;
    }
}

long GetDelayValueFromDelayRadio( short currentDelay)
{
    switch( currentDelay)
    {
        case 0:
            return( 60);
            break;

        case 1:
            return( 120);
            break;

        case 2:
            return( 240);
            break;

        default:
            return( 60);
            break;
    }
}

void SetNetLowerBandwidth( netSetupType *setup)
{
    if ( setup->whichTab == kNetLevelSettingTabNum)
    {
        SwitchAnyRadioOrCheckbox( kNetLevelLowerBandwidth, setup->currentBandwidth);
        RefreshInterfaceItem( kNetLevelLowerBandwidth);
    }
}

void SetNetLag( netSetupType *setup)
{
    Boolean     tabActive = ( setup->whichTab == kNetLevelSettingTabNum);

    if ( tabActive)
    {
        if (( !setup->canMakeChanges) || ( !setup->opponentHasArrived) ||
            ( !setup->scenarioLoaded))
        {
            SetStatusOfAnyInterfaceItem( kNetLevelDecreaseLagButton, kDimmed, true);
            SetStatusOfAnyInterfaceItem( kNetLevelIncreaseLagButton, kDimmed, true);
        } else
        {
            //NetLevelShowLag( setup->currentLatency / kMessageLatencyUnit, setup->recommendedLatency / kMessageLatencyUnit);
            if  ( setup->currentLatency < kMessageLatencyUnit)
            {
                SetStatusOfAnyInterfaceItem( kNetLevelDecreaseLagButton, kDimmed, true);
            } else
            {
                SetStatusOfAnyInterfaceItem( kNetLevelDecreaseLagButton, kActive, true);
            }
            if  ( setup->currentLatency > ( kMaxMessageLatency - kMessageLatencyUnit))
            {
                SetStatusOfAnyInterfaceItem( kNetLevelIncreaseLagButton, kDimmed, true);
            } else
            {
                SetStatusOfAnyInterfaceItem( kNetLevelIncreaseLagButton, kActive, true);
            }
        }
    }
}

void SetNetRegistered( netSetupType *setup, short setting)
{
    if ( setup->whichTab == kNetLevelSettingTabNum)
    {
        SwitchAnyRadioOrCheckbox( setup->currentRegistered +
            kNetSettingsFirstRegisteredRadio, false);
        RefreshInterfaceItem( setup->currentRegistered +
            kNetSettingsFirstRegisteredRadio);

        setup->currentRegistered = setting;

        SwitchAnyRadioOrCheckbox( setup->currentRegistered +
            kNetSettingsFirstRegisteredRadio, true);
        RefreshInterfaceItem( setup->currentRegistered +
            kNetSettingsFirstRegisteredRadio);
    } else
    {
        setup->currentRegistered = setting;
    }
}

void SetNetDelay( netSetupType *setup, short setting)
{
    long newDelay;

    setup->currentDelayValue = setting;
    if ( setup->currentDelayValue <= 60) newDelay = 0;
    else if ( setup->currentDelayValue <= 120) newDelay = 1;
    else if ( setup->currentDelayValue <= 240) newDelay = 2;
    else newDelay = 3;

    if ( setup->whichTab == kNetLevelSettingTabNum)
    {
        SwitchAnyRadioOrCheckbox( setup->currentDelay +
            kNetSettingsFirstDelayRadio, false);
        RefreshInterfaceItem( setup->currentDelay +
            kNetSettingsFirstDelayRadio);

        setup->currentDelay = newDelay; //whichItem - kNetSettingsFirstDelayRadio;

        SwitchAnyRadioOrCheckbox( setup->currentDelay +
            kNetSettingsFirstDelayRadio, true);
        RefreshInterfaceItem( setup->currentDelay +
            kNetSettingsFirstDelayRadio);
    } else
    {
        setup->currentDelay = newDelay;
    }
}

void ShowNetLevelName( long thisChapter)
{
    Str255  chapterName;

    GetScenarioName( GetScenarioNumberFromChapterNumber( thisChapter), chapterName);
    DrawLevelNameInBox( chapterName, kButtonFontNum,
        GetScenarioPrologueID(GetScenarioNumberFromChapterNumber( thisChapter)), kNetLevelBox);
}

void ShowWhyNetLevelNotLoaded( netSetupType *setup, StringPtr why)
{
    Str255  s;

    if ( why != nil)
    {
        CopyPString( s, "\p-- ");
        ConcatenatePString( s, gAresGlobal->otherPlayerScenarioFileName);
        ConcatenatePString( s, "\p --\r\r");
        ConcatenatePString( s, why);
        CopyPString( setup->whyScenarioNotLoaded, s);
    }

    if ( setup->whichTab != kNetLevelLevelTabNum) return;

    DrawStringInInterfaceItem(
        kNetLevelBox, setup->whyScenarioNotLoaded);
}

void NetLevelShowLag( long lag, long recommended)
{
    unsigned char   *getwidchar, *getwidwid, color;
    long            width, height, strlen;
    Rect            tRect, stringRect, bottomRect;
    PixMapHandle    offMap = GetGWorldPixMap( gOffWorld);
    transColorType  *transColor;
    longRect        clipRect;
    Str255          string, numString;

    GetAnyInterfaceItemContentBounds( GetAnyInterfaceItemPtr( kNetLevelLagBox), &tRect);
    mCopyAnyRect( clipRect, tRect);
    DrawInOffWorld();
    DefaultColors();
    PaintRect( &tRect);

    if ( IAmHosting())
    {
        GetIndString( string, 2004, 10);
        NumToString( recommended, numString);
        ConcatenatePString( (unsigned char *)string, (unsigned char *)numString);
        mSetDirectFont( kButtonSmallFontNum);
        mGetDirectStringDimensions( string, width, height, strlen, getwidchar, getwidwid)
        SetRect( &stringRect, 0, 0, width, height);
        CenterRectInRect( &stringRect, &tRect);
        stringRect.top = tRect.top;
        stringRect.bottom = tRect.top + height;
        mGetTranslateColorShade( AQUA, MEDIUM, color, transColor)
        MoveTo( stringRect.left, stringRect.top + mDirectFontAscent);
        DrawDirectTextStringClipped( string, color, *offMap, &clipRect, 0, 0);

        mCopyAnyRect( bottomRect, tRect);
        bottomRect.top = stringRect.bottom;
    } else
    {
        mCopyAnyRect( bottomRect, tRect);
    }

    NumToString( lag, string);
    mSetDirectFont( kTitleFontNum);
    mGetDirectStringDimensions( string, width, height, strlen, getwidchar, getwidwid)

    SetRect( &stringRect, 0, 0, width, height);
    CenterRectInRect( &stringRect, &bottomRect);

    mGetTranslateColorShade( AQUA, LIGHTER, color, transColor)
    MoveTo( stringRect.left, stringRect.top + mDirectFontAscent);
    DrawDirectTextStringClipped( string, color, *offMap, &clipRect, 0, 0);


    DrawInRealWorld();
    DefaultColors();
    CopyOffWorldToRealWorld((WindowPtr)gTheWindow, &tRect);

}

void NetLevelShowRaces( short myRace, short opponentRace)
{
    unsigned char   *getwidchar, *getwidwid, color;
    long            width, height, strlen, spacingHeight;
    Rect            tRect, stringRect, bottomRect;
    PixMapHandle    offMap = GetGWorldPixMap( gOffWorld);
    transColorType  *transColor;
    longRect        clipRect;
    Str255          string, raceString;

    GetAnyInterfaceItemContentBounds( GetAnyInterfaceItemPtr( kNetLevelRaceBox), &tRect);
    mCopyAnyRect( clipRect, tRect);
    DrawInOffWorld();
    DefaultColors();
    PaintRect( &tRect);

    GetIndString( string, 2004, 16);    // you:
    GetRaceString( raceString, kRaceAdjective, myRace);
    ConcatenatePString( (unsigned char *)string, (unsigned char *)raceString);

    mSetDirectFont( kButtonSmallFontNum);
    mGetDirectStringDimensions( string, width, height, strlen, getwidchar, getwidwid)
    SetRect( &stringRect, 0, 0, width, height);
    CenterRectInRect( &stringRect, &tRect);
    spacingHeight = ((tRect.bottom - tRect.top) - (height * 2)) / 3;
    stringRect.top = tRect.top + spacingHeight;
    stringRect.bottom = stringRect.top + height;
    mGetTranslateColorShade( GREEN, LIGHTER, color, transColor)
    MoveTo( stringRect.left, stringRect.top + mDirectFontAscent);
    DrawDirectTextStringClipped( string, color, *offMap, &clipRect, 0, 0);

    mCopyAnyRect( bottomRect, tRect);
    bottomRect.top = stringRect.bottom;

    GetIndString( string, 2004, 17);    // opponent:
    GetRaceString( raceString, kRaceAdjective, opponentRace);
    ConcatenatePString( (unsigned char *)string, (unsigned char *)raceString);

    mSetDirectFont( kButtonSmallFontNum);
    mGetDirectStringDimensions( string, width, height, strlen, getwidchar, getwidwid)

    SetRect( &stringRect, 0, 0, width, height);
    CenterRectInRect( &stringRect, &bottomRect);
    stringRect.top = tRect.top + spacingHeight * 2 + height;
    stringRect.bottom = stringRect.top + height;
    mGetTranslateColorShade( RED, LIGHTER, color, transColor)
    MoveTo( stringRect.left, stringRect.top + mDirectFontAscent);
    DrawDirectTextStringClipped( string, color, *offMap, &clipRect, 0, 0);

    DrawInRealWorld();
    DefaultColors();
    CopyOffWorldToRealWorld((WindowPtr)gTheWindow, &tRect);

}

// NetLevelSetColor: sets & shows admiral color; if color = 0xff, uses where
// as click.

unsigned char NetLevelSetColor( netSetupType *setup,
    unsigned char color, unsigned char myApparentColor,
    Point where, Boolean allowColors, Boolean handleClick)
{
    short   legalColor[COLOR_NUM] = {   1,  // no colorization
                                        1,  // orange
                                        0,  // bright yellow
                                        0,  // dark blue
                                        0,  // slime green
                                        0,  // dark purple
                                        1,  // bruise (a little dark)
                                        1,  // salmon
                                        1,  // gold
                                        1,  // cyan
                                        1,  // hot pink
                                        1,  // pale green
                                        1,  // pruple
                                        0,  // light blue
                                        1,  // tan
                                        1   // red
                                    }; // 0 = do not show, 1 = show & use, -1 = show no use
    short   colorNum = 0, swatchWidth, thisPosition, i, leftBuffer, spriteID = -1, whichShape;
    Rect    boundsRect, tRect, swatchRect[COLOR_NUM];
    Boolean tabActive = ( setup->whichTab == kNetLevelPlayerTabNum);
    long    race, count, scale;
    baseObjectType  *base = nil;
    unsigned char   originalColor;

    color = GetNetEnemyColor();
    race = GetRaceIDFromNum( setup->opponentRace);
    mGetBaseObjectFromClassRace( base, count, 200, race)
    if ( base != nil) spriteID = base->pixResID;
    if ( spriteID < 0)
    {
        spriteID = 499;
        whichShape = 0;
    } else if ( base->attributes & kShapeFromDirection)
    {
        whichShape = 135 / base->frame.rotation.rotRes;
    } else whichShape = 0;

    if ( allowColors)
    {
        if ( myApparentColor != 0)
            legalColor[myApparentColor] = -1;
    } else
    {
        for ( i = 1; i < COLOR_NUM; i++)
        {
            if ( legalColor[i] == 1)
                legalColor[i] = -1;
        }
    }

    for( i = 0; i < COLOR_NUM; i++)
    {
        if ( legalColor[i] != 0) colorNum++;
    }

    if ( tabActive)
    {
        GetAnyInterfaceItemContentBounds( GetAnyInterfaceItemPtr( kNetLevelColorBox), &boundsRect);
        DrawInOffWorld();
        DefaultColors();
        PaintRect( &boundsRect);

        swatchWidth = (( boundsRect.right - boundsRect.left) / ( colorNum + 1)) * kColorBoxHSpacing;
        swatchWidth = (( boundsRect.right - boundsRect.left) - swatchWidth) / (colorNum);

        leftBuffer = colorNum * (swatchWidth + kColorBoxHSpacing);
        leftBuffer = boundsRect.left + (((boundsRect.right - boundsRect.left) / 2) -
            (leftBuffer / 2));
        thisPosition = 0;

        for ( i = 0; i < COLOR_NUM; i++)
        {
            if ( legalColor[i] != 0)
            {
                swatchRect[i].left = leftBuffer + thisPosition * (swatchWidth +
                    kColorBoxHSpacing) + kColorBoxHSpacing;
                swatchRect[i].top = boundsRect.top + kColorBoxSpacing;
                swatchRect[i].right = swatchRect[i].left + swatchWidth;
                swatchRect[i].bottom = boundsRect.bottom - kColorBoxSpacing;

                thisPosition++;
            } else
            {
                SetRect( &swatchRect[i], 0, 0, -1, -1);
            }
        }

        if ( handleClick)
        {
            i = 0;
            while (( !PtInRect( where, &swatchRect[i])) && ( i < COLOR_NUM)) i++;
            if (( i < COLOR_NUM) && ( legalColor[i] == 1))
            {
                color = i;
                SetNetEnemyColor( color);
                PlayVolumeSound( kComputerBeep1, kMediumLoudVolume, kShortPersistence, kMustPlaySound);
            }
        }
    }

//  if ( legalColor[color] <= 0) color = 0xff;
    if ( legalColor[color] <= 0)
    {
        originalColor = color;
        do
        {
            color++;
            if ( color >= COLOR_NUM) color = 1;
        } while ((color != 0xff) && (color != 0) && (legalColor[color] <= 0) &&
            ( color != originalColor));
        if ( color == originalColor) color = 0xff;

    }
    if ( color == 0xff) color = 0;

    if ( tabActive)
    {
        for ( i = 0; i < COLOR_NUM; i++)
        {
            scale = kOneHalfScale;
            if ( i == color) scale = SCALE_SCALE;
            if ( legalColor[i] != 0)
            {
                if ( i == 0)
                {
                    SetTranslateColorShadeFore( i, LIGHT);
//                  FrameRect( &swatchRect[i]);
//                  MoveTo( swatchRect[i].right - 1, swatchRect[i].top);
//                  LineTo( swatchRect[i].left, swatchRect[i].bottom - 1);
                    EZDrawSpriteOffByID( spriteID, whichShape, scale, 0,
                        &swatchRect[i]);
                } else
                {
                    if ( legalColor[i] == -1)
                    {
//                      SetTranslateColorShadeFore( i, VERY_DARK);
                        DefaultColors();
//                      PaintRect( &swatchRect[i]);
                        EZDrawSpriteOffByID( spriteID, whichShape, scale, i,
                            &swatchRect[i]);
                        PenMode( patOr);
                        PenPat( &qd.gray);
                        PaintRect( &swatchRect[i]);
                        PenNormal();

                    } else
                    {
                        SetTranslateColorShadeFore( i, LIGHT);
                        EZDrawSpriteOffByID( spriteID, whichShape, scale, i,
                            &swatchRect[i]);
                    }
//                  PaintRect( &swatchRect[i]);
                }

                if ( i == color)
                {
                    tRect = swatchRect[i];
                    InsetRect( &tRect, -2, -2);
                    SetTranslateColorFore( 0x00);
//                  FrameRect( &tRect);
                }

                thisPosition++;
            }
        }

        NormalizeColors();
        DrawInRealWorld();
        NormalizeColors();
        CopyOffWorldToRealWorld((WindowPtr)gTheWindow, &boundsRect);
    }

    return( color);
}

void NetLevelDrawLagGraph( netSetupType *setup)
{
    Rect            graphBounds, lagRect, recRect, tRect;
    longRect        clipRect;
    long            i, x, y, whichSample, mean, sd, width, height, strlen;
    Str255          s;
    transColorType  *transColor;
    unsigned char   color, *getwidchar, *getwidwid;
    PixMapHandle    offMap = GetGWorldPixMap( gOffWorld);

    if ( setup->whichTab == kNetLevelSettingTabNum)
    {
        GetAnyInterfaceItemContentBounds( GetAnyInterfaceItemPtr( kNetLagGraphBox), &graphBounds);
        DrawInOffWorld();
        NormalizeColors();
        PaintRect( &graphBounds);

        mSetDirectFont( kButtonSmallFontNum)

        mCopyAnyRect( clipRect, graphBounds);

        graphBounds.top += 2;
        graphBounds.bottom -= 2;

        whichSample = setup->sampleNum % kTimeSampleNum;
        whichSample--;
        if ( whichSample < 0) whichSample = kTimeSampleNum - 2;
        SetTranslateColorShadeFore( AQUA, LIGHT);
        for ( i = kTimeSampleNum - 2; i >= 0; i--)
        {
            x = i * ( graphBounds.right - graphBounds.left);
            x /= (kTimeSampleNum - 1);
            x += graphBounds.left;
            y = ( kNetLagMaxValue - (setup->sampleLag[whichSample] / 1)) *
                ( graphBounds.bottom - graphBounds.top);
            y /= kNetLagMaxValue;
            y += graphBounds.top;
            if ( y < graphBounds.top) y = graphBounds.top;
            if ( y > graphBounds.bottom - 1) y = graphBounds.bottom - 1;

            if ( i < (kTimeSampleNum - 2)) LineTo( x, y);
            else MoveTo( x, y);

            whichSample--;
            if ( whichSample < 0) whichSample = kTimeSampleNum - 2;
        }

        mean = NetLevelGetLagMean( setup);
        y = ( kNetLagMaxValue - (setup->currentLatency * 2)) *
            ( graphBounds.bottom - graphBounds.top);
        y /= kNetLagMaxValue;
        y += graphBounds.top;
        if ( y < graphBounds.top) y = graphBounds.top;
        if ( y > graphBounds.bottom - 1) y = graphBounds.bottom - 1;
        SetTranslateColorShadeFore( YELLOW, LIGHT);
        MoveTo( graphBounds.left, y);
        LineTo( graphBounds.right - 1, y);

        NumToString(  setup->currentLatency / 3, s);

        mGetDirectStringDimensions( s, width, height, strlen, getwidchar, getwidwid)
        lagRect.left = graphBounds.left + 2;
        lagRect.right = lagRect.left + width + 3;
        lagRect.top = y - height - 2,
        lagRect.bottom = lagRect.top + height + 2;
        if ( lagRect.top <= ( graphBounds.top + 1))
        {
            OffsetRect( &lagRect, 0, graphBounds.top - lagRect.top + 2);
        }
        if ( lagRect.bottom >= ( graphBounds.bottom - 1))
        {
            OffsetRect( &lagRect, 0, graphBounds.bottom - lagRect.bottom - 2);
        }
        SetTranslateColorShadeFore( YELLOW, DARKER);
        PaintRect( &lagRect);

        MoveTo( lagRect.left + 2, lagRect.top + mDirectFontAscent + 1);
        mGetTranslateColorShade( YELLOW, LIGHT, color, transColor)
        DrawDirectTextStringClipped( s, color, *offMap, &clipRect, 0, 0);

        sd = NetLevelGetLagStandardDeviation( setup, mean);
        sd += mean;
        sd = NetLevelGetLagHighestBelow( setup, sd);
        sd += 6 - (sd % 6);
        y = ( kNetLagMaxValue - ((sd) / 1)) *
            ( graphBounds.bottom - graphBounds.top);
        y /= kNetLagMaxValue;
        y += graphBounds.top;
        if ( y < graphBounds.top) y = graphBounds.top;
        if ( y > graphBounds.bottom - 1) y = graphBounds.bottom - 1;
        SetTranslateColorShadeFore( GREEN, LIGHT);
        MoveTo( graphBounds.left, y);
        LineTo( graphBounds.right - 1, y);

        NumToString( (sd) / 6, s);

        mGetDirectStringDimensions( s, width, height, strlen, getwidchar, getwidwid)
        recRect.left = graphBounds.left + 2;
        recRect.right = recRect.left + width + 3;
        recRect.top = y + 1,
        recRect.bottom = recRect.top + height + 2;
        if ( recRect.top <= (graphBounds.top + 1))
        {
            OffsetRect( &recRect, 0, graphBounds.top - recRect.top + 1);
        }
        if ( recRect.bottom >= ( graphBounds.bottom - 1))
        {
            OffsetRect( &recRect, 0, graphBounds.bottom - recRect.bottom - 2);
        }
        SectRect( &lagRect, &recRect, &tRect);
        if ( tRect.bottom > tRect.top)
        {
            OffsetRect( &recRect, lagRect.right - lagRect.left + 4, 0);
        }
        SetTranslateColorShadeFore( GREEN, DARKER);
        PaintRect( &recRect);

        MoveTo( recRect.left + 2, recRect.top + mDirectFontAscent + 1);
        mGetTranslateColorShade( GREEN, LIGHT, color, transColor)
        DrawDirectTextStringClipped( s, color, *offMap, &clipRect, 0, 0);

        graphBounds.top -= 2;
        graphBounds.bottom += 2;
        SetTranslateColorShadeFore( AQUA, MEDIUM);
        FrameRect( &graphBounds);

        NormalizeColors();
        DrawInRealWorld();
        NormalizeColors();
        CopyOffWorldToRealWorld((WindowPtr)gTheWindow, &graphBounds);
    }
}

long NetLevelGetLagHighestBelow( netSetupType *setup, long belowWhat)
{
    long    result = 0, i;

    for ( i = 0; i < kTimeSampleNum; i++)
    {
        if ( i != ( setup->sampleNum % kTimeSampleNum))
        {
            if ( ( setup->sampleLag[i]> 0) && ( setup->sampleLag[i] <= belowWhat))
                result = setup->sampleLag[i];
        }
    }
    return( result);
}

long NetLevelGetLagMean( netSetupType *setup)
{
    float   f = 0;
    long    i;

    for ( i = 0; i < kTimeSampleNum; i++)
    {
        if ( i != ( setup->sampleNum % kTimeSampleNum))
            f += setup->sampleLag[i];
    }

    f /= (float)(kTimeSampleNum - 1);
    f += .5;
    return((long)f);
}

long NetLevelGetLagStandardDeviation( netSetupType *setup, long mean)
{
    float   f = 0;
    long    i, l;

    for ( i = 0; i < kTimeSampleNum; i++)
    {
        if ( i != ( setup->sampleNum % kTimeSampleNum))
        {
            l = setup->sampleLag[i] - mean;
            l *= l;
            f += l;
        }
    }
    f /= (float)(kTimeSampleNum-1);
    f += .5;
    l = f;
    l = lsqrt( l);
    return ( l);
}

void ShowNetTextMessage( netSetupType *setup, Boolean fromMyself)
{
    PixMapHandle    offMap = GetGWorldPixMap( gOffWorld);

#pragma unused( setup, fromMyself)
// ------------------------
// Begin SendText

/*  GetAnyInterfaceItemContentBounds( GetAnyInterfaceItemPtr( kNetLevelCommBox), &tRect);
    mCopyAnyRect( commBox, tRect)
//  currentLineTop = commBox.top;

    mSetDirectFont( kButtonSmallFontNum)
    setup->retroTextSpec.textLength =
        GetHandleSize( setup->retroTextSpec.text);
    HLock( setup->retroTextSpec.text);
    setup->retroTextSpec.thisPosition = setup->retroTextSpec.linePosition =
        setup->retroTextSpec.lineCount = 0;
    setup->retroTextSpec.tabSize = 100;
    if ( !fromMyself)
    {
        if ( setup->opponentColor == GetApparentColorFromRace( setup->myRace))
        {
            if ( GetApparentColorFromRace( setup->myRace) ==
                GetApparentColorFromRace( setup->opponentRace))
            {
                tempColor = setup->opponentColor + 8;
                tempColor %= 16;
            } else
            {
                tempColor = GetApparentColorFromRace( setup->opponentRace);
            }
        } else
        {
            tempColor = setup->opponentColor;
        }
        mGetTranslateColorShade( tempColor, LIGHT, setup->retroTextSpec.color, transColor)
        mGetTranslateColorShade( tempColor, DARKEST, setup->retroTextSpec.backColor, transColor)
    } else
    {
        mGetTranslateColorShade( GetApparentColorFromRace( setup->myRace), LIGHT, setup->retroTextSpec.color, transColor)
        mGetTranslateColorShade( GetApparentColorFromRace( setup->myRace), DARKEST, setup->retroTextSpec.backColor, transColor)
    }
    setup->retroTextSpec.backColor = 0xff;
    setup->retroTextSpec.originalColor = setup->retroTextSpec.nextColor = setup->retroTextSpec.color;
    setup->retroTextSpec.originalBackColor = setup->retroTextSpec.nextBackColor = setup->retroTextSpec.backColor;

    setup->retroTextSpec.topBuffer = 2;
    setup->retroTextSpec.bottomBuffer = 0;
    height = DetermineDirectTextHeightInWidth( &setup->retroTextSpec, commBox.right - commBox.left);
    height += kCommLinePad;
    if ( (setup->currentLineTop + height > commBox.bottom))
    {
        DrawInOffWorld();
        NormalizeColors();
        LongRectToRect( &commBox, &tRect);
        updateRgn = NewRgn();
        ScrollRect( &tRect, 0, commBox.bottom - (setup->currentLineTop + height), updateRgn);
        DisposeRgn( updateRgn);
        setup->currentLineTop = commBox.bottom - height;
        tRect.top = setup->currentLineTop;
        PaintRect( &tRect);
        tRect.top = commBox.top;
        DrawInRealWorld();
        NormalizeColors();
        CopyOffWorldToRealWorld((WindowPtr)gTheWindow, &tRect);
    }

    mCopyAnyRect( retroBounds, commBox)
    retroBounds.top = setup->currentLineTop;
    retroBounds.bottom = setup->currentLineTop + height;
    mCopyAnyRect( tRect, retroBounds);

    setup->retroTextSpec.xpos = retroBounds.left;
    setup->retroTextSpec.ypos = retroBounds.top + mDirectFontAscent;

    while ( setup->retroTextSpec.thisPosition < setup->retroTextSpec.textLength)
    {
        PlayVolumeSound(  kTeletype, kMediumLowVolume, kShortPersistence, kLowPrioritySound);
        DrawInOffWorld();
        NormalizeColors();
        DrawRetroTextCharInRect( &setup->retroTextSpec, 12, &retroBounds, &retroBounds, *offMap,
            0, 0);
        DrawInRealWorld();
        NormalizeColors();
        CopyOffWorldToRealWorld((WindowPtr)gTheWindow, &tRect);

        waitTime = TickCount();
        while (( TickCount() - waitTime) < 3)
        {
            //DO NOTHING
        };
    }
    HUnlock( setup->retroTextSpec.text);
    DisposeHandle( setup->retroTextSpec.text);
    setup->retroTextSpec.text = nil;
    setup->currentLineTop += height;
*/
// End SendText
// ------------------------
}

void SendNetTextMessage( netSetupType *setup)
{
#pragma unused( setup)
/*  if ( GetInterfaceTextEditLength( kNetLevelEnterTextBox) == 0)
        return;

    setup->retroTextSpec.textLength =
        GetInterfaceTextEditLength( kNetLevelEnterTextBox);
    setup->retroTextSpec.text = NewHandle( setup->retroTextSpec.textLength);
    CopyInterfaceTextEditContents( kNetLevelEnterTextBox,
        (anyCharType *)*(setup->retroTextSpec.text),
        &setup->retroTextSpec.textLength);

    // prepend my name
    GetIndString( tempString, 2004, 21);
    Munger( setup->retroTextSpec.text, 0, nil, 0, tempString + 1, tempString[0]);
    setup->retroTextSpec.textLength += tempString[0];

    myName = GetPlayerName( setup->myNum); // -1 = get this machine's name
    Munger( setup->retroTextSpec.text, 0, nil, 0, myName + 1, myName[0]);
    setup->retroTextSpec.textLength += myName[0];

    GetIndString( tempString, 2004, 20);
    Munger( setup->retroTextSpec.text, 0, nil, 0, tempString + 1, tempString[0]);
    setup->retroTextSpec.textLength += tempString[0];

    SendPreGameTextMessage( *setup->retroTextSpec.text, GetHandleSize( setup->retroTextSpec.text));
    ShowNetTextMessage( setup, true);

    InterfaceTextEditSetText( kNetLevelEnterTextBox, ( anyCharType *)"\p");
    InterfaceTextEditSelectAll( kNetLevelEnterTextBox);
    InterfaceTextEditActivate( kNetLevelEnterTextBox);
*/
}

void AddCharToVisibleString( netSetupType *setup, long whichPlayer,
    anyCharType what, Boolean addIt)
{
    PixMapHandle    offMap = GetGWorldPixMap( gOffWorld);
    transColorType  *transColor;
    longRect        clipRect;
    Rect            tRect;
    unsigned char   *getwidchar, *getwidwid, color, textColor, shade;
    long            width, height, strlen, count, count2,
                    whichItem = kFirstPlayerItemGroup + kPlayerGroupTextNum +
                        (whichPlayer * kPlayerItemGroupSize),
                    leftOffset = 0;
    Str255          totalString, tempString;
    short           shadeNum = 0;

    if ( whichPlayer == setup->myNum)
    {
        textColor = kMyCommColor;
    } else
    {
        textColor = kOpponentCommColor;
    }
    if ( addIt)
    {
        if ( setup->visibleString[whichPlayer][0] > kMaxVisibleChar)
            CutCharsFromAnyCharPString( setup->visibleString[whichPlayer], 0, 1);
        setup->visibleString[whichPlayer][0] += 1;
        setup->visibleString[whichPlayer][setup->visibleString[whichPlayer][0]] =
            what;
    }

    count = 1;
    while ( count <= setup->visibleString[whichPlayer][0])
    {
        if (( setup->visibleString[whichPlayer][count] == kDeleteChar) ||
            ( setup->visibleString[whichPlayer][count] == kBackspaceChar))
        {
            if ( count == 1)
            {
                CutCharsFromAnyCharPString( setup->visibleString[whichPlayer], 0, 1);
            } else
            {
                CutCharsFromAnyCharPString( setup->visibleString[whichPlayer], count - 2, 2);
                if ( count > 2) count -= 2;
                else count = 1;
            }
        } else count++;
    }

    count = 1;
    shadeNum = 0;
    totalString[0] = setup->visibleString[whichPlayer][0];
    while ( count <= setup->visibleString[whichPlayer][0])
    {
        if ( setup->visibleString[whichPlayer][count] == 0x0d)
        {
            totalString[count] = ' ';
            shadeNum++;
        } else
        {
            totalString[count] = setup->visibleString[whichPlayer][count];
        }
        count++;
    }

    GetAnyInterfaceItemContentBounds( GetAnyInterfaceItemPtr( whichItem), &tRect);
    mCopyAnyRect( clipRect, tRect);
    DrawInOffWorld();
    DefaultColors();
    SetTranslateColorShadeFore( textColor, VERY_DARK);
    PaintRect( &tRect);
    DefaultColors();

    tRect.left += 2;
    tRect.right -= 2;
    mCopyAnyRect( clipRect, tRect);
    mSetDirectFont( kButtonSmallFontNum);
    mGetDirectStringDimensions( totalString, width, height, strlen, getwidchar, getwidwid)
    if ( width > (tRect.right - tRect.left))
    {
        leftOffset = width - (tRect.right - tRect.left);
    }
    MoveTo( tRect.left - leftOffset, tRect.top + mDirectFontAscent + 1);
    count = 1;
    shadeNum = VERY_LIGHT - shadeNum * 4;
    while ( count <= setup->visibleString[whichPlayer][0])
    {
        if ( shadeNum < DARKER) shade = DARKER;
        else shade = shadeNum;
        mGetTranslateColorShade( textColor, shade, color, transColor)
        count2 = count;
        tempString[0] = 0;
        while ( (count2 <= setup->visibleString[whichPlayer][0]) &&
            ( setup->visibleString[whichPlayer][count2] != 0x0d))
        {
            tempString[0]++;
            tempString[tempString[0]] = totalString[count2];
            count2++;
        }
        if ( setup->visibleString[whichPlayer][count2] == 0x0d)
        {
            tempString[0]++;
            tempString[tempString[0]] = ' ';//totalString[count2];
            count2++;
        }
        count = count2;
        shadeNum += 4;
        DrawDirectTextStringClipped( tempString, color, *offMap, &clipRect, 0, 0);
    }

    tRect.left -= 2;
    tRect.right += 2;
    DefaultColors();
    DrawInRealWorld();
    DefaultColors();
    CopyOffWorldToRealWorld((WindowPtr)gTheWindow, &tRect);
}

void ConvertOffscreenRectToPortraitData( short left, short top, unsigned char *data)
{
    PixMapHandle    offMap = GetGWorldPixMap( gOffWorld);
    long            rowBytes = (*offMap)->rowBytes & 0x3fff, h, v, debugCount = 0;
    unsigned char   *source, p;


    source = (unsigned char *)(*offMap)->baseAddr + (long)top * rowBytes + (long)left;
    rowBytes -= kPortraitSize;
    v = kPortraitSize;
    while ( v-- > 0)
    {
        h = kPortraitSize;
        while ( h-- > 0)
        {
            if ( debugCount >= (kPortraitSize * kPortraitSize))
            {
//               DebugStr("\pOverwrite.");
                 return;
            }
            p = GetRetroIndex( *source++);// & 0x0f;
            if ( p == 0) p = 1;
            else if ( p == 1) p = 15;
            p = p - 1;
            p &= 0x0f;
            *data++ = p;
            debugCount++;
        }
        source += rowBytes;
    }
}

void DisplayPortraitData( short left, short top, unsigned char *data, unsigned char color)
{
    PixMapHandle    offMap = GetGWorldPixMap( gOffWorld);
    long            rowBytes = (*offMap)->rowBytes & 0x3fff, h, v;
    unsigned char   *dest, *dest2, p, originalP;
    Rect            tRect;

    color <<= 4;
    DrawInOffWorld();
    DefaultColors();
    SetRect( &tRect, left, top, left + (kPortraitSize + kPortraitMargin) * 2,
        top + (kPortraitSize + kPortraitMargin) * 2);
    PaintRect( &tRect);

    dest = (unsigned char *)(*offMap)->baseAddr + (long)(top + kPortraitMargin) * rowBytes + (long)left + kPortraitMargin;
    dest2 = dest + rowBytes;
    rowBytes = (rowBytes * 2) - ( kPortraitSize * 2);
//  rowBytes -= ( kPortraitSize * 2);
    v = kPortraitSize;
    while ( v-- > 0)
    {
        h = kPortraitSize;
        while ( h-- > 0)
        {
            originalP = (*data++);
            p = originalP + color + 1;
            p = GetTranslateIndex( p);
            *dest++ = p;
            *dest++ = p;

            p = originalP;
            p >>= 1;
            p += 8;
            if ( p > 15) p = 15;
            p &= 0x0f;
            p += color + 1;
            *dest2++ = p;
            *dest2++ = p;
        }
        dest += rowBytes;
        dest2 += rowBytes;
    }
    DrawInRealWorld();
    DefaultColors();
    CopyOffWorldToRealWorld((WindowPtr)gTheWindow, &tRect);
}

Boolean GetPortraitHRun( long *x, long *y, unsigned char *originalColor, unsigned char *length,
    unsigned char *data)
{
    unsigned char   *color;

    *length = 0;
    if ( *y >= kPortraitSize) return ( false);

    color = data + (*y * (long)kPortraitSize) + *x;
    *originalColor = *color;

    do
    {
        (*length)++;
        (*x)++;
        color++;
    } while (( *color == *originalColor) && ( *x < kPortraitSize) && ( *length < 15));
    if ( *x >= kPortraitSize)
    {
        *x = 0;
        (*y)++;
    }
    return( true);
}

void AddPortraitHRun( long *x, long *y, unsigned char color, unsigned char length,
    unsigned char *data)

{
    unsigned char   *dest;

    if (( length == 0) || ( *y >= kPortraitSize)) return;
    if ( *x >= kPortraitSize)
    {
//      DebugStr("\pArgument out of bounds");
        *x = 0;
        return;
    }
    dest = data + (*y * (long)kPortraitSize) + *x;

    while ( length != 0)
    {
        if ( *x >= kPortraitSize)
        {
//          DebugStr("\pX out of bounds");
            *x = 0;
            return;
        }

        length--;
        *dest++ = color;
        (*x)++;
    }
    if ( *x >= kPortraitSize)
    {
        *x = 0;
        (*y)++;
    }
}

void ClearPortrait( unsigned char *data)
{
    long    l = kPortraitSize * kPortraitSize;

    while ( l-- > 0)
    {
        *data++ = 0xff;
    }
}

//
// PlotCustomIcon
//  We must create an icon-sized gworld in which to plot the icon, since the
//  PlotIconID routine seems to want to use the 4-bit version with Ares' color
//  table. Since the common photoshop preview icons only come in 8-bit versions,
//  I want to force the use of the 8-bit version if it exists. So, I'm creating
//  a temporary GWorld which uses the System palette, plotting the icon in that,
//  copying the gworlds contents to the true destination, then deleting the
//  GWorld. I am also getting the icl8 resource, and confirming that it belongs
//  to the portrait file before using it.
//
//  Added with Ambrosia release --
//  now tries to use scriptable Finder to get icon; works better; can understand
//  new 8.6 icons.
//

//-16455

Boolean PlotCustomIcon( Rect *bounds)
{
    short                   refID = -1, homeRes;
    OSErr                   anErr = 0;
    Str255              volName;
    short                   vRefNum;
    long                    dirID;
    FInfo                   fndrInfo;
    Handle              iconSuiteHandle, testIconData, icl8Handle, ICNnHandle;
    IconSelectorValue   iconKind;
    FSSpec              portraitFileSpec;

    anErr = HGetVol( volName, &vRefNum, &dirID);
    if ( anErr != noErr) return( false);

    anErr = HGetFInfo( vRefNum, dirID, kPortraitFileName, &fndrInfo);
    if ( anErr != noErr) return( false);

    if ( (fndrInfo.fdFlags & kHasCustomIcon) == 0) return( false);

#ifdef powerc
    anErr = FSMakeFSSpec( vRefNum, dirID, kPortraitFileName, &portraitFileSpec);
    if ( anErr != noErr) goto PlotCustomIcon_oldWay;

    anErr = GetIconSuiteFromFSSpec( &portraitFileSpec, &iconSuiteHandle);
    if ( anErr != noErr) goto PlotCustomIcon_oldWay;

    DrawInOffWorld();
    DefaultColors();

    testIconData = nil;
    anErr = GetIconFromSuite( &ICNnHandle, iconSuiteHandle, 'ICN#');
    anErr = GetIconFromSuite( &icl8Handle, iconSuiteHandle, 'icl8');
    // handles from GetIconFromSuite don't need to be disposed of;
    // DisposeIconSuite does that
    if (( ICNnHandle != nil) && ( icl8Handle != nil))
    {
//      PlotIconHandle( bounds, atNone, ttNone, testIconData);
        Ploticl8ToCICN( 0, 0, bounds, icl8Handle, ICNnHandle);
    } else goto PlotCustomIcon_oldWay;

    DisposeIconSuite( iconSuiteHandle, true);

    DrawInRealWorld();
    DefaultColors();
    return true;
#endif powerc

PlotCustomIcon_oldWay:
    anErr = noErr;

    refID = ARF_OpenResFile( kPortraitFileName);
    if ( refID == -1)
    {
        return false;
    }

//  UseResFile( refID);

    DrawInOffWorld();
    DefaultColors();

    PaintRect( bounds);

//  anErr = PlotIconID( bounds, atNone, ttNone, -16455);
    // see Ploticl8ToCICN on why I can't just use PlotIconSuite
    if ( !Ploticl8ToCICN( -16455, refID, bounds, nil, nil))
    {
        iconKind = svLarge1Bit;
        testIconData = GetResource( 'icl4', -16455);
        if ( testIconData != nil)
        {
            homeRes = HomeResFile( testIconData);

            if (( ResError() == noErr) && ( homeRes == refID) &&
                ( homeRes != -1))
            {
                iconKind |= svLarge4Bit;
            }
            ReleaseResource( testIconData);
        }

        anErr = GetIconSuite( &iconSuiteHandle, -16455, svAllLargeData);
        if (( anErr != noErr) || ( iconSuiteHandle == nil))
        {
            mWriteDebugString("\pGetIconSuite Error");
            WriteDebugLong( anErr);
            return( false);
        }

        anErr = PlotIconSuite( bounds, atNone, ttNone, iconSuiteHandle);
        if ( anErr != noErr)
        {
            mWriteDebugString("\pPlotIconSuite Error");
            WriteDebugLong( anErr);
            return( false);
        }
        DisposeIconSuite( iconSuiteHandle, true);
    }

    DrawInRealWorld();
    DefaultColors();

    CloseResFile( refID);
    if ( anErr == noErr)
        return( true);
    else return( false);
}

short GetSpriteResIDFromRace( short race, long *whichShape)
{
    baseObjectType  *base = nil;
    long            count;
    short           spriteID = -1;

    race = GetRaceIDFromNum( race);
    mGetBaseObjectFromClassRace( base, count, 200, race)
    if ( base != nil) spriteID = base->pixResID;
    if ( spriteID < 0)
    {
        spriteID = 499;
        *whichShape = 0;
    } else if ( base->attributes & kShapeFromDirection)
    {
        *whichShape = 135 / base->frame.rotation.rotRes;
    } else *whichShape = 0;
    return( spriteID);
}

void UpdateSampleShip( netSetupType *setup, long whichPlayer)
{
    short           spriteID, race;
    long            whichShape;
    unsigned char   color = 0, backColor;
    Rect            tRect;

    if ( whichPlayer == setup->myNum)
    {
        race = setup->myRace;
        backColor = kMyCommColor;
    } else
    {
        race = setup->opponentRace;
        color = setup->opponentColor;
        backColor = kOpponentCommColor;
    }

    spriteID = GetSpriteResIDFromRace( race, &whichShape);
    GetAnyInterfaceItemContentBounds( GetAnyInterfaceItemPtr( kFirstPlayerItemGroup +
        (whichPlayer * kPlayerItemGroupSize) + kPlayerGroupShipNum), &tRect);

    DrawInOffWorld();
    DefaultColors();
    SetTranslateColorShadeFore( backColor, DARKEST);
    PaintRect( &tRect);
    DefaultColors();
    DrawInRealWorld();
    EZDrawSpriteOffByID( spriteID, whichShape, SCALE_SCALE, color, &tRect);
    DrawInRealWorld();
    DefaultColors();
    CopyOffWorldToRealWorld((WindowPtr)gTheWindow, &tRect);
}

void UpdatePlayerData( netSetupType *setup, long whichPlayer)
{
    retroTextSpecType   retroTextSpec;
    Str255              tempString, numString, tString2;
    StringPtr           myName;
    transColorType      *transColor;
    PixMapHandle        offPixMap = GetGWorldPixMap( gOffWorld);
    Rect                bounds, inRect;
    longRect            clipRect, lRect;
    long                height, l;
    short               race, minutesPlayed, kills, losses;
    unsigned char       color;
    Boolean             isRegistered;

    if ( whichPlayer == setup->myNum)
    {
        race = setup->myRace;
        color = kMyCommColor;
        minutesPlayed = GetNetMinutesPlayed();
        kills = GetNetKills();
        losses = GetNetLosses();
        if ( Ambrosia_Is_Registered()) isRegistered = true;
        else isRegistered = false;
    } else
    {
        race = setup->opponentRace;
        color = kOpponentCommColor;
        minutesPlayed = setup->opponentMinutesPlayed;
        kills = setup->opponentKills;
        losses = setup->opponentLosses;
        if ( GetOpponentIsUnregistered()) isRegistered = false;
        else isRegistered = true;
    }

    retroTextSpec.text = GetResource( 'TEXT', kPlayerDataTextID);
    if ( retroTextSpec.text != nil)
    {
        DetachResource( retroTextSpec.text);

        GetAnyInterfaceItemContentBounds( GetAnyInterfaceItemPtr( kFirstPlayerItemGroup +
            (whichPlayer * kPlayerItemGroupSize) + kPlayerGroupDataNum), &bounds);

        // player #
        NumToString( whichPlayer + 1, numString);
        GetIndString( tempString, kPlayerDataKeyStringID, kPlayerNumberKeyStringNum);
        Munger( retroTextSpec.text, 0, (tempString + 1), *tempString, numString + 1, *numString);

        // player name
        myName = GetPlayerName( whichPlayer);
        CopyPString( tString2, myName);
        if ( !isRegistered) ConcatenatePString(tString2, "\p ** UNREGISTERED **");
        GetIndString( tempString, kPlayerDataKeyStringID, kPlayerNameKeyStringNum);
        Munger( retroTextSpec.text, 0, (tempString + 1), *tempString, tString2 + 1, *tString2);

        // player race
        GetRaceString( numString, kRaceAdjective, race);
        GetIndString( tempString, kPlayerDataKeyStringID, kPlayerSpeciesKeyStringNum);
        ConcatenatePString( (unsigned char *)numString, "\p (advantage: ");
        SmallFixedToString( GetRaceAdvantage( race), tString2);
        ConcatenatePString( (unsigned char *)numString, (unsigned char *)tString2);
        ConcatenatePString( (unsigned char *)numString, "\p)");
        Munger( retroTextSpec.text, 0, (tempString + 1), *tempString, numString + 1, *numString);

        // time played
        if ( minutesPlayed < 60)
        {
            CopyPString( numString, "\p00:");
        } else
        {
            l = minutesPlayed / 60;
            if ( l < 10)
                CopyPString( numString, "\p0");
            else numString[0] = 0;
            NumToString( l, tempString);
            ConcatenatePString( numString, tempString);
            ConcatenatePString( numString, "\p:");
            minutesPlayed -= l * 60;
        }
        if ( minutesPlayed < 10)
        {
            ConcatenatePString( numString, "\p0");
        }
        NumToString( minutesPlayed, tempString);
        ConcatenatePString( numString, tempString);
        GetIndString( tempString, kPlayerDataKeyStringID,
            kPlayerMinutesPlayedKeyStringNum);
        Munger( retroTextSpec.text, 0, (tempString + 1), *tempString, numString + 1, *numString);

        // kills
        NumToString( kills, numString);
        GetIndString( tempString, kPlayerDataKeyStringID, kPlayerKillsKeyStringNum);
        Munger( retroTextSpec.text, 0, (tempString + 1), *tempString, numString + 1, *numString);

        // losses
        NumToString( losses, numString);
        GetIndString( tempString, kPlayerDataKeyStringID, kPlayerLossesKeyStringNum);
        Munger( retroTextSpec.text, 0, (tempString + 1), *tempString, numString + 1, *numString);

        retroTextSpec.textLength = GetHandleSize( retroTextSpec.text);

        mSetDirectFont( kButtonSmallFontNum)
        retroTextSpec.thisPosition = retroTextSpec.linePosition = retroTextSpec.lineCount = 0;
        retroTextSpec.tabSize = 100;
        mGetTranslateColorShade( color, MEDIUM, retroTextSpec.color, transColor)
        mGetTranslateColorShade( color, DARKEST, retroTextSpec.backColor, transColor)
        retroTextSpec.originalColor = retroTextSpec.nextColor = retroTextSpec.color;
        retroTextSpec.originalBackColor = retroTextSpec.nextBackColor = retroTextSpec.backColor;
        retroTextSpec.topBuffer = 1;
        retroTextSpec.bottomBuffer = 1;

        mCopyAnyRect( clipRect, bounds)
        mCopyAnyRect( inRect, bounds)
//      InsetRect( &inRect, 2, 2);

        height = DetermineDirectTextHeightInWidth( &retroTextSpec, inRect.right - inRect.left);

        retroTextSpec.xpos = inRect.left;
        retroTextSpec.ypos = inRect.top + mDirectFontAscent;

        RectToLongRect( &inRect, &lRect);

        DrawInOffWorld();
        DefaultColors();
        SetTranslateColorShadeFore( color, DARKEST);
        PaintRect( &bounds);
        DefaultColors();

        DrawDirectTextInRect( &retroTextSpec, &lRect, &clipRect, *offPixMap, 0, 0);

        DrawInRealWorld();
        DefaultColors();
        CopyOffWorldToRealWorld((WindowPtr)gTheWindow, &bounds);

        DisposeHandle( retroTextSpec.text);
    }
}

void DrawPortraitByPlayerNum( netSetupType *setup, long whichPlayer)
{
    unsigned char   *data, color;
    Rect            tRect;

    if ( whichPlayer == setup->myNum)
    {
        data = (unsigned char *)setup->myPortraitData;
        color = kMyCommColor;
    } else
    {
        data = (unsigned char *)setup->opponentPortraitData;
        color = kOpponentCommColor;
    }
    GetAnyInterfaceItemContentBounds( GetAnyInterfaceItemPtr( kFirstPlayerItemGroup +
        (whichPlayer * kPlayerItemGroupSize) + kPlayerGroupPortraitNum), &tRect);
    tRect.right = tRect.left + 32;
    tRect.bottom = tRect.top + 32;
    DisplayPortraitData( tRect.left, tRect.top, data, color);
}

void OpenOtherScenarioFile( netSetupType *setup)
{
    FSSpec  destFile;
    Str255  whyDisabledString, filePath;

    if ( gAresGlobal->otherPlayerScenarioFileName[0] > 0)
    {
        CopyPString( filePath, "\p:Ares Net Scenarios Folder:");
        ConcatenatePString( filePath, gAresGlobal->otherPlayerScenarioFileName);
    } else
    {
        filePath[0] = 0;
    }

    if ( (filePath[0] == 0) || (ARF_OpenResFile_External( filePath,
        &destFile) == noErr))
    {
        if ( filePath[0] > 0)
        {
            BlockMove( &destFile, &gAresGlobal->externalFileSpec,
                sizeof( FSSpec));
        } else
        {
            gAresGlobal->externalFileSpec.name[0] = 0;
        }

        if ( EF_OpenExternalFile() == noErr)
        {
            if ( gAresGlobal->scenarioFileInfo.version <
                gAresGlobal->otherPlayerScenarioFileVersion)
            {
                SetStatusOfAnyInterfaceItem( kNetLevelOKButton,
                    kDimmed, true);
                SendPreGameBasicMessage( ePreGameOldVersionScenarioMessage);
                GetIndString( whyDisabledString, 2010, 7);
                ShowWhyNetLevelNotLoaded( setup,
                    whyDisabledString);
                setup->scenarioLoaded = false;
                return;
            } else if ( gAresGlobal->scenarioFileInfo.version >
                gAresGlobal->otherPlayerScenarioFileVersion)
            {
                SetStatusOfAnyInterfaceItem( kNetLevelOKButton,
                    kDimmed, true);
                SendPreGameBasicMessage( ePreGameNewVersionScenarioMessage);
                if ( gAresGlobal->internetConfigPresent)
                {
                    GetIndString( whyDisabledString, 2010, 7);
                } else
                {
                    GetIndString( whyDisabledString, 2010, 4);
                    ConcatenatePString( whyDisabledString,
                        gAresGlobal->otherPlayerScenarioFileURL);
                    ConcatenatePString( whyDisabledString, "\p.");
                }
                ShowWhyNetLevelNotLoaded( setup,
                    whyDisabledString);
                setup->scenarioLoaded = false;
                return;

            } else if ( gAresGlobal->scenarioFileInfo.checkSum !=
                gAresGlobal->otherPlayerScenarioFileCheckSum)
            {
                SetStatusOfAnyInterfaceItem( kNetLevelOKButton,
                    kDimmed, true);
                SendPreGameBasicMessage( ePreGameWrongCheckSumScenarioMessage);
                if ( gAresGlobal->internetConfigPresent)
                {
                    GetIndString( whyDisabledString, 2010, 8);
                } else
                {
                    GetIndString( whyDisabledString, 2010, 5);
                    ConcatenatePString( whyDisabledString,
                        gAresGlobal->otherPlayerScenarioFileURL);
                    ConcatenatePString( whyDisabledString, "\p.");
                }
                ShowWhyNetLevelNotLoaded( setup,
                    whyDisabledString);
                setup->scenarioLoaded = false;
                return;
            }
            SendPreGameBasicMessage( ePreGameGotScenarioMessage);
            DrawStringInInterfaceItem( kWhyDisabledNetLevelText, nil);
            SetNetLevelNum( setup);
            SetCurrentNetRace( setup);
            SendPreGameShortMessage( eSetRaceMessage, setup->myRace);
            setup->scenarioLoaded = true;
            return;
        }

    }
    SetStatusOfAnyInterfaceItem( kNetLevelOKButton,
        kDimmed, true);
    SendPreGameBasicMessage( ePreGameNoScenarioMessage);
    if ( gAresGlobal->internetConfigPresent)
    {
        GetIndString( whyDisabledString, 2010, 9);
    } else
    {
        GetIndString( whyDisabledString, 2010, 6);
        ConcatenatePString( whyDisabledString,
            gAresGlobal->otherPlayerScenarioFileURL);
        ConcatenatePString( whyDisabledString, "\p.");
    }
    ShowWhyNetLevelNotLoaded( setup,
        whyDisabledString);
    setup->scenarioLoaded = false;
}

void LaunchURL( StringPtr s)
{
    OSStatus err = noErr;
    long startSel;
    long endSel;

    if ( gAresGlobal->internetConfigPresent)
    {
        startSel = 0;
        endSel = s[0];
//      err = ICLaunchURL( gAresGlobal->internetConfig, "\p", (char *) &s[1], s[0],
//          &startSel, &endSel);
    }
}


#endif NETSPROCKET_AVAILABLE
