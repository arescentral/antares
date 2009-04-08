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

// Player Interface.c

// Tells Interface Handling what to do, and actually does the work.

#include "PlayerInterface.hpp"

#include <QDOffscreen.h>

#include "AmbrosiaSerial.h"
#include "AresExternalFile.hpp"
#include "AresGlobalType.hpp"
#include "AresMain.hpp"
#include "AresMoviePlayer.hpp"
#include "AresNetwork.hpp"
#include "AresNetworkSprocket.hpp"
#include "AresPreferences.hpp"
#include "BriefingRenderer.hpp"
#include "ColorTranslation.hpp"
#include "ConditionalMacros.h"
//#include "CopyProtection.h"  // included in prefs
#include "Debug.hpp"
#include "DirectText.hpp"
#include "Error.hpp"
#include "GDeviceHandling.hpp"
#include "HandleHandling.hpp"
#include "HideMenubar.hpp"
#include "Instruments.hpp"
#include "InterfaceHandling.hpp"
#include "KeyCodes.hpp"
#include "KeyMapTranslation.hpp"
#include "KeySetupScreen.hpp"
#include "MessageScreen.hpp"
#include "Music.hpp"
#include "NatePixTable.hpp"
#include "NetSetupScreen.hpp"
#include "OffscreenGWorld.hpp"
#include "Options.hpp"
#include "Races.hpp"
#include "RegistrationTool.h"
#include "Resources.h"
#include "ScenarioMaker.hpp"
#include "ScrollStars.hpp"
#include "SoundFX.hpp"               // for button on/off
#include "StringHandling.hpp"
#include "StringNumerics.hpp"
#include "TitleScreen.hpp"           // for CenterRectInRect
#include "Transitions.hpp"
#include "WrapGameRanger.hpp"
#include "WinAresGlue.hpp"

#define kThisVersion    0x00000201  // last was 200; last was 104

#define kContentStringID            2002
#define kContentNetWaitNum          1
#define kContentNetDeclineNum       2
#define kContentNetAcceptNum        3

#define kMainScreenResID            5000

#define kMainPlayButton             0
#define kMainNetworkButton          1
#define kMainOptionsButton          2
#define kMainQuitButton             3
#define kMainAboutButton            4
#define kMainDemoButton             5
#define kMainTrainButton            6
#define kMainDemoTimeOutTime        1800 // 30 secs till demo

#define kStartNetworkGameID         5001

#define kNetSetupHostButton         1
#define kNetSetupJoinButton         0
#define kNetSetupCancelButton       2
//#define   kNetSetupNameButton         3

/*#define   kNetHostID                  5002
#define kHostInviteButton           0
#define kHostStatusRect             1
#define kHostBeginButton            2
#define kHostCancelButton           3
#define kHostAvailableList          4
#define kHostInGameList             5
*/
#define kNetHostID                  5003
#define kHostCancelButton           0
#define kHostDeclineButton          1
#define kHostAcceptButton           2
#define kHostHostNameRect           3

#define kClientWaitID               5004
#define kClientWaitCancelButton     0
#define kClientWaitStrID            2004
#define kClientWaitHostDeclinedStrNum   4
#define kClientWaitHostAcceptedStrNum   5
#define kClientWaitHostWaitingStrNum    6
#define kClientWaitStatusRect       1

#define kHostPollTime               600     // in ticks = 10 seconds

#define kOptionsScreenID            5007
#define kOptionGameMusicButton      0
#define kOptionIdleMusicButton      1
#define kOptionQuickDrawButton      2
#define kOptionSoundUpButton        2//3
#define kOptionSoundDownButton      3//4
#define kOptionSpeechOnButton       4
#define kOptionNoScaleUpButton      5
#define kOptionCancelButton         5//6
#define kOptionDoneButton           6//7
#define kOptionKeyControlButton     7//8
#define kOptionScreenEasyButton     9
#define kOptionScreenMediumButtun   10
#define kOptionScreenHardButton     11
#define kOptionVolumeBox            13//19
#define kOptionVolumeColor          PALE_PURPLE

#define kKeyScreenID                5006
#define kKeyCancelButton            19//18
#define kKeyDoneButton              20//19
#define kKeyOptionButton            21//20
#define kKeySubstituteCheckbox      22
#define kKeyIllustrationBox         31
#define kKeyIllustrationPictID      520

#define kMissionBriefingScreenID    6000
#define kMissionDoneButton          0
#define kMissionPreviousButton      1
#define kMissionNextButton          2
#define kMissionMapRect             7
#define kMissionDataWidth               200
#define kMissionDataHeight          120
#define kMissionDataVBuffer         40
#define kMissionDataTopBuffer       30
#define kMissionDataBottomBuffer    15
#define kMissionDataHBuffer         41
#define kMissionDataHiliteColor     GOLD
#define kMissionLineHJog            10
#define kMissionNextTimeOutTime     120
#define kMissionStarMapPictID       8000
#define kMissionBriefPointOffset    2
#define kMissionStarMapBriefNum     0
#define kMissionBlankBriefNum       1
#define kMissionStarPointWidth      16
#define kMissionStarPointHeight     12

#define kLoadingScreenID            6001
#define kLoadingScreenColor         PALE_GREEN

#define kDebriefShipResID           800
#define kDebriefShipShapeNum        0
#define kDebriefZMin                1
#define kDebriefZMax                10000
#define kDebriefZScaleMultiple      12209 // = (zmax^2) / (maxscale - minscale)
#define kDebriefVanishH             635
#define kDebriefVanishV             300
#define kDebriefShipV               -60
#define kDebriefShipHMultiple       119760 // = (zmax^2) / (kDebriefVanishH + width of sprite)
#define kDebriefShipVMultiple       1666667 // = (zmax^2) / (kDebriefVanishH + width of sprite)
#define kDebriefTimeOutTime         360
#define kDebriefCopyVChunkSize      20
#define kDebriefCopyVChunkTime      1

#define kSummaryTextID              6000
#define kSummaryKeyStringID         6000
#define kYourMinStringNum           1
#define kYourSecStringNum           2
#define kParMinStringNum            3
#define kParSecStringNum            4
#define kYourLossStringNum          5
#define kParLossStringNum           6
#define kYourKillStringNum          7
#define kParKillStringNum           8
#define kYourScoreStringNum         9
#define kParScoreStringNum          10

#define kTimePoints                 50
#define kLossesPoints               30
#define kKillsPoints                20

#define kScoreTableHeight           120

#define kFirstShipDataPictID        1001
#define kShipDataTextID             6001
#define kShipDataKeyStringID        6001
#define kShipDataNameID             6002
#define kWeaponDataTextID           6003
#define kShipOrObjectStringNum      1
#define kShipTypeStringNum          2
#define kMassStringNum              3
#define kShieldStringNum            4
#define kHasLightStringNum          5
#define kMaxSpeedStringNum          6
#define kThrustStringNum            7
#define kTurnStringNum              8
#define kWeaponNumberStringNum      9
#define kWeaponNameStringNum        10
#define kWeaponGuidedStringNum      11
#define kWeaponRangeStringNum       12
#define kWeaponDamageStringNum      13
#define kWeaponAutoTargetStringNum  14
#define kShipDataShipStringNum      1
#define kShipDataObjectStringNum    2
#define kShipDataDashStringNum      3
#define kShipDataYesStringNum       4
#define kShipDataNoStringNum        5
#define kShipDataPulseStringNum     6
#define kShipDataBeamStringNum      7
#define kShipDataSpecialStringNum   8

#define kShipDataWidth              240

#define kAboutAresID                5010
#define kAboutAresOKButton          0

#define kPlayAgainID                5008
#define kPlayAgainResumeID          5009
#define kPlayAgainOKButton          0
#define kPlayAgainNoButton          1
#define kPlayAgainBox               3
#define kPlayAgainResumeButton      4

#define kNetSettingsID              5014
#define kNetSettingsBox             12
#define kNetSettingsOKButton        0
#define kNetSettingsCancelButton    1
#define kNetSettingsFirstRegisteredRadio    2
#define kNetSettingsLastRegisteredRadio     4
#define kNetSettingsNeverRadio      2
#define kNetSettingsResendRadio     3
#define kNetSettingsAlwaysRadio     4

#define kNetSettingsFirstDelayRadio 5
#define kNetSettingsLastDelayRadio  7
#define kNetSettings1SecondRadio    5
#define kNetSettings2SecondsRadio   6
#define kNetSettings4SecondsRadio   7

#define kNetSettingsLowerBandwidthCheck 8

#define kSelectLevelID              5011
#define kSelectLevelOKButton        0
#define kSelectLevelCancelButton    1
#define kSelectLevelPreviousButton  2
#define kSelectLevelNextButton      3
#define kSelectLevelNameBox         4

/*
#define kNetLevelID                 5013
#define kNetLevelSendTextButton     5
#define kNetLevelEnterTextBox       6
#define kNetLevelCommBox            7
#define kNetLevelDecreaseLagButton  8
#define kNetLevelIncreaseLagButton  9
#define kNetLevelLagBox             10
#define kNextRaceButton             11
#define kPreviousRaceButton         12
#define kHostIsPlayer2CheckBox      13
#define kNetLevelRaceBox            14
#define kNetLevelColorBox           15
#define kColorBoxSpacing            3
*/
#define kNetLevelID                 5020

#define kNetLevelSendTextButton     2
#define kNetLevelEnterTextBox       3
#define kNetLevelCommBox            4
#define kNetLevelTabBox             9

#define kNetLevelItemNum            14

#define kNetLevelSettingTabID       5023
#define kNetLevelSettingTabNum      7
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

#define kNetLevelPlayerTabID        5022
#define kNetLevelPlayerTabNum       6
#define kPreviousRaceButton         ( kNetLevelItemNum + 0)
#define kNextRaceButton             ( kNetLevelItemNum + 1)
#define kNetLevelRaceBox            ( kNetLevelItemNum + 2)
#define kNetLevelColorBox           ( kNetLevelItemNum + 3)

#define kNetLevelLevelTabID         5021
#define kNetLevelLevelTabNum        5
#define kPreviousNetLevelButton     ( kNetLevelItemNum + 0)
#define kHostIsPlayer2CheckBox      ( kNetLevelItemNum + 1)
#define kNextNetLevelButton         ( kNetLevelItemNum + 2)
#define kNetLevelBox                ( kNetLevelItemNum + 3)

#define kColorBoxSpacing            3

#define kCommLinePad                5

#define kHelpScreenID               5012
#define kNOFHelpScreenID            5015
#define kHelpScreenTextID           6002
#define kNOFHelpScreenTextID        6005
#define kHelpScreenKeyStringID      6003
#define kHelpScreenBox              1
#define kHelpScreenDoneButton       0

#define kDebriefTextWidth           300

#define kScrollTextWidth            640//540
#define kScrollTextHeight           200
#define kScrollTextSpeed            4
#define kScrollMovieHeight          140



//4
#define kScrollTextLineBuffer       2
#define kScrollTextDelimiter1       '#'
#define kScrollTextDelimiter2       '+'
#define kReturnChar                 0x0d
#define kScrollTextMovieChar        'M'
#define kScrollTextBackgroundChar   'B'

#define mPlayScreenSound            PlayVolumeSound( kComputerBeep3, kMediumLowVolume, kShortPersistence, kMustPlaySound)

#define mDoubleDigitize( mstring) if ( mstring[0]==1) { mstring[0]=2; mstring[2]=mstring[1]; mstring[1]='0';}

extern aresGlobalType           *gAresGlobal;
extern netStatusType            gNetworkStatus;
extern Handle                   gHostEntity, gClientEntity;
extern netAresEntity            gMyNetEntity;
//extern unsigned long          gAresGlobal->gOptions;
//extern KeyMap                 gAresGlobal->gKeyControl[];
extern PixMapHandle             thePixMapHandle;
extern long                     gNatePortLeft, gNatePortTop, CLIP_LEFT, CLIP_RIGHT, CLIP_TOP,
                                CLIP_BOTTOM, /*gAresGlobal->gTrueClipBottom, gAresGlobal->gSoundVolume,*/
                                gNetLatency, /*gAresGlobal->gThisScenarioNumber,*/ gRandomSeed;
extern  GWorldPtr               gOffWorld, gRealWorld, gSaveWorld;
extern CWindowPtr               gTheWindow/*, gAresGlobal->gBackWindow*/;       // we need the window for copying to the real world, a hack
extern directTextType           *gDirectText;
extern Handle                   gDirectTextData;
extern long                     gWhichDirectText, WORLD_WIDTH, WORLD_HEIGHT;
//                              gAresGlobal->gPlayerAdmiralNumber;
extern  Handle                  gColorTranslateTable, gBaseObjectData, gObjectActionData/*, gAresGlobal->gPreferencesData*/;
extern  GDHandle                theDevice;

Boolean IsKeyReserved( KeyMap, Boolean);
void BlackenOffscreen( void);
void Pause( long);

Boolean stickyCheat = false;

mainScreenResultType DoMainScreenInterface( long *demoLevel)

{
    Point                   where;
    int                     error;
    short                   whichItem;
    Boolean                 done = FALSE, readyToCheat = false, timeout = false;
    EventRecord             theEvent;
    char                    whichChar;
    mainScreenResultType    result = kNullResult;
    long                    startDemoTime = TickCount();
    CWindowPtr              whichWindow;


    *demoLevel = -1;
    FlushEvents(everyEvent, 0);
/*  if ( gAresGlobal->gameRangerPending)
    {
                        result = kMainNetwork;
                        done = true;
                        Wrap_GRGetWaitingCmd();
                        return result;
    } else if ( gAresGlobal->isQuitting)
    {
                        result = kMainQuit;
                        done = true;
                        return result;
    }
*/
    BlackenOffscreen();
    error = OpenInterface( kMainScreenResID);
    if ( error == kNoError)
    {
        if ( !(gAresGlobal->gOptions & kOptionNetworkAvailable))// NetSprocketPresent())
        {
            SetStatusOfAnyInterfaceItem( kMainNetworkButton, kDimmed, FALSE);
        }

        if ( gAresGlobal->gOptions & kOptionNoSinglePlayer)
        {
            SetStatusOfAnyInterfaceItem( kMainPlayButton, kDimmed, false);
        }

        DrawEntireInterface();
        AutoFadeFrom( 30, FALSE);
        while ( !done)
        {

            if ( !gAresGlobal->haveSeenRTNotice)
            {
                Ambrosia_Update_Registered();

                if ( !Ambrosia_Is_Registered())
                {
                    SetMBarState( true, theDevice);
                    RT_DisplayNotice( false);
                    SetMBarState( false, theDevice);
                }
                gAresGlobal->haveSeenRTNotice = true;
            }

            InterfaceIdle();
            Ares_WaitNextEvent (everyEvent, &theEvent, 3, nil);
            gAresGlobal->returnToMain = false;

            whichItem = -1;
            switch ( theEvent.what )
            {
                case nullEvent:
                    InterfaceIdle();
                    if ( gAresGlobal->gOptions & kOptionInBackground)
                    {
                        startDemoTime = TickCount();
                    } else if (AutoShowHideMenubar( theEvent.where, theDevice))
                    {
                        startDemoTime = TickCount();
                    }

                    if (( TickCount() - startDemoTime) > kMainDemoTimeOutTime)
                    {
                        whichItem = kMainDemoButton;
                        timeout = true;
                    }
                    if ( gAresGlobal->isQuitting)
                    {
                        result = kMainQuit;
                        done = true;
                    }
                    if ( gAresGlobal->gameRangerPending)
                    {
                        result = kMainNetwork;
                        done = true;

                        Wrap_GRGetWaitingCmd();


                    }
                    break;

                case osEvt:
//                  HandleOSEvent( &theEvent);
                    startDemoTime = TickCount();
                    break;

                case mouseDown:
                    startDemoTime = TickCount();
                    where = theEvent.where;
                    GlobalToLocal( &where);

                    if ( HandleMouseDown( &theEvent))
                    {
                        result = kMainQuit;
                        done = true;
                    }

                    whichItem = InterfaceMouseDown( where);
                    break;
                case mouseUp:
                    break;
                case keyDown:
                case autoKey:
                    if ( HandleKeyDown( &theEvent))
                    {
                        result = kMainQuit;
                        done = true;
                    }

                    startDemoTime = TickCount();
                    whichChar = theEvent.message & charCodeMask;

                    if (( whichChar >= '0') && ( whichChar <= '9'))
                    {
                        *demoLevel = whichChar - '0';
                        result = kMainDemo;
                        done = true;
                    }
                    if ( whichChar == 'f')
                    {
                        readyToCheat = true;
                        PlayVolumeSound( kCloakOn, kMaxSoundVolume, kLongPersistence, kMustPlaySound);
                    } else if (( whichChar == '-') && ( readyToCheat))
                    {
                        SetStatusOfAnyInterfaceItem( kMainPlayButton, kActive, true);
                        DrawAnyInterfaceItemOffToOn( GetAnyInterfaceItemPtr( kMainPlayButton));
                        SetStatusOfAnyInterfaceItem( kMainOptionsButton, kActive, true);
                        DrawAnyInterfaceItemOffToOn( GetAnyInterfaceItemPtr( kMainOptionsButton));
                        PlayVolumeSound( kCloakOff, kMaxSoundVolume, kLongPersistence, kMustPlaySound);
                        stickyCheat = true;
                    } else readyToCheat = false;

                    whichItem = InterfaceKeyDown( theEvent.message);
                    break;

                case updateEvt:
                    startDemoTime = TickCount();
                    whichWindow = ( CWindowPtr)theEvent.message;

                    if ( whichWindow == gTheWindow)
                    {
                        BeginUpdate( (WindowPtr)whichWindow);
                            MacSetPort( (WindowPtr)gTheWindow);
                            CopyOffWorldToRealWorld((WindowPtr)gTheWindow, &(gTheWindow->portRect));
                        EndUpdate( (WindowPtr)whichWindow);
                        break;
                        EndUpdate( (WindowPtr)whichWindow);
                    } else if ( whichWindow == gAresGlobal->gBackWindow)
                    {
                        BeginUpdate( (WindowPtr)whichWindow);
                            MacSetPort( (WindowPtr)gAresGlobal->gBackWindow);
                            MacFillRect(  &(gAresGlobal->gBackWindow->portRect), (Pattern *)&qd.black);
                        EndUpdate( (WindowPtr)whichWindow);
                    } else
                    {
                        BeginUpdate( (WindowPtr)whichWindow);
                        EndUpdate( (WindowPtr)whichWindow);
                    }
                    MacSetPort( (WindowPtr)gTheWindow);
                    break;

            }
            switch ( whichItem)
            {
                case kMainQuitButton:
                    done = TRUE;
                    result = kMainQuit;
                    break;

                case kMainDemoButton:
                    done = TRUE;
                    if ( timeout)
                        result = kMainTimeoutDemo;
                    else
                        result = kMainDemo;
                    break;

                case kMainTrainButton:
                    done = true;
                    result = kMainTrain;
                    break;

                case kMainPlayButton:
                    result = kMainPlay;
                    done = TRUE;
                    break;

                case kMainNetworkButton:
                    result = kMainNetwork;
                    done = TRUE;
                    break;

                case kMainAboutButton:
//                  result = kMainAbout;
//                  done = true;

                    CloseInterface();
//                  DoAboutAresInterface();
                    DoScrollText( (WindowPtr)gTheWindow, 6500, 2/*kScrollTextSpeed*/,
                        540, kTitleFontNum/*kComputerFontNum*/, -1);
                    OpenInterface( kMainScreenResID);
                    if ( !(gAresGlobal->gOptions & kOptionNetworkAvailable))// NetSprocketPresent())
                    {
                        SetStatusOfAnyInterfaceItem( kMainNetworkButton, kDimmed, FALSE);
                    }
                    if ( gAresGlobal->gOptions & kOptionNoSinglePlayer)
                    {
                        SetStatusOfAnyInterfaceItem( kMainPlayButton, kDimmed, false);
                    }
                    DrawEntireInterface();
                    startDemoTime = TickCount();

                    break;

                case kMainOptionsButton:
//                  #ifndef kNonPlayableDemo
                    CloseInterface();
                    DoOptionsInterface();

                    OpenInterface( kMainScreenResID);
                    if ( !(gAresGlobal->gOptions & kOptionNetworkAvailable))// NetSprocketPresent())
                    {
                        SetStatusOfAnyInterfaceItem( kMainNetworkButton, kDimmed, FALSE);
                    }
                    if ( gAresGlobal->gOptions & kOptionNoSinglePlayer)
                    {
                        SetStatusOfAnyInterfaceItem( kMainPlayButton, kDimmed, false);
                    }

                    DrawEntireInterface();
//                  #endif
                    startDemoTime = TickCount();
                    break;

            }


        }
        CloseInterface();
    }
    return ( result);
}

void DoAboutAresInterface( void)

{
    Point                   where;
    int                     error;
    short                   whichItem;
    Boolean                 done = FALSE;
    EventRecord             theEvent;
    char                    whichChar;
    CWindowPtr              whichWindow;

    FlushEvents(everyEvent, 0);
    BlackenWindow();

    error = OpenInterface( kAboutAresID);
    if ( error == kNoError)
    {
        DrawEntireInterface();

        while ( !done)
        {

            InterfaceIdle();

            Ares_WaitNextEvent (everyEvent, &theEvent, 3, nil);
            {
                whichItem = -1;
                switch ( theEvent.what )
                {
                    case nullEvent:
                        InterfaceIdle();
                        if ( gAresGlobal->gOptions & kOptionInBackground)
                        {
                        } else if (AutoShowHideMenubar( theEvent.where, theDevice))
                        {
                        }
                        if ( gAresGlobal->returnToMain)
                        {
                            done = true;
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
                                MacSetPort( (WindowPtr)gTheWindow);
                                CopyOffWorldToRealWorld((WindowPtr)gTheWindow, &(gTheWindow->portRect));
                            EndUpdate( (WindowPtr)whichWindow);
                            break;
                            EndUpdate( (WindowPtr)whichWindow);
                        } else if ( whichWindow == gAresGlobal->gBackWindow)
                        {
                            BeginUpdate( (WindowPtr)whichWindow);
                                MacSetPort( (WindowPtr)gAresGlobal->gBackWindow);
                                MacFillRect(  &(gAresGlobal->gBackWindow->portRect), (Pattern *)&qd.black);
                            EndUpdate( (WindowPtr)whichWindow);
                        } else
                        {
                            BeginUpdate( (WindowPtr)whichWindow);
                            EndUpdate( (WindowPtr)whichWindow);
                        }
                        MacSetPort( (WindowPtr)gTheWindow);

                        break;


                    case mouseDown:
                        where = theEvent.where;
                        GlobalToLocal( &where);
                        HandleMouseDown( &theEvent);
                        whichItem = InterfaceMouseDown( where);
                        break;
                    case mouseUp:
                        break;
                    case keyDown:
                    case autoKey:
                        whichChar = theEvent.message & charCodeMask;
                        whichItem = InterfaceKeyDown( theEvent.message);
                        break;
                }
                switch ( whichItem)
                {
                    case kAboutAresOKButton:
                        done = TRUE;
                        break;
                }

            }
        }
        CloseInterface();
    }
}

void DoLoadingInterface( Rect *contentRect, StringPtr levelName)

{
    int                     error;
    unsigned char           color, *strPtr;
    transColorType          *transColor;
    longRect                lRect, clipRect, boundsRect;
    PixMapHandle            offMap = GetGWorldPixMap( gOffWorld);
    Rect                    tRect;
    retroTextSpecType       retroTextSpec;
    long                    height, waitTime;

    BlackenWindow();

    error = OpenInterface( kLoadingScreenID);
    if ( error == kNoError)
    {
        DrawEntireInterface();

        GetAnyInterfaceItemContentBounds( GetAnyInterfaceItemPtr( 0), contentRect); // item 0 = loading rect
        CloseInterface();

// it is assumed that we're "recovering" from a fade-out
        AutoFadeFrom( 10, FALSE);

        DrawInRealWorld();
        mSetDirectFont( kTitleFontNum)
        mGetTranslateColorShade( PALE_GREEN, LIGHT, color, transColor)
        lRect.left = 0;
        lRect.top = 0;
        lRect.right = WORLD_WIDTH;
        lRect.bottom = WORLD_HEIGHT;
//      MoveTo( contentRect->left + (( contentRect->right - contentRect->left) / 2) - (stringWidth / 2),
//              contentRect->top);

        strPtr = levelName + 1;
        retroTextSpec.textLength = *levelName;
        retroTextSpec.text = (char **)&strPtr;

        retroTextSpec.thisPosition = retroTextSpec.linePosition = retroTextSpec.lineCount = 0;
        retroTextSpec.tabSize =220;
        mGetTranslateColorShade( PALE_GREEN, VERY_LIGHT, retroTextSpec.color, transColor)
        mGetTranslateColorShade( SKY_BLUE, DARKEST, retroTextSpec.backColor, transColor)
        retroTextSpec.backColor = 0xff;
        retroTextSpec.originalColor = retroTextSpec.nextColor = retroTextSpec.color;
        retroTextSpec.originalBackColor = retroTextSpec.nextBackColor = retroTextSpec.backColor;

        retroTextSpec.topBuffer = 2;
        retroTextSpec.bottomBuffer = 0;
        height = DetermineDirectTextHeightInWidth( &retroTextSpec, kSmallScreenWidth);

        boundsRect.left = (WORLD_WIDTH / 2) - ( retroTextSpec.autoWidth / 2);
        boundsRect.right = boundsRect.left + retroTextSpec.autoWidth;
        boundsRect.top = (contentRect->top / 2) - ( retroTextSpec.autoHeight / 2);
        boundsRect.bottom = boundsRect.top + retroTextSpec.autoHeight;
        retroTextSpec.xpos = boundsRect.left;
        retroTextSpec.ypos = boundsRect.top + mDirectFontAscent;

        clipRect.left = 0;
        clipRect.right = clipRect.left + WORLD_WIDTH;
        clipRect.top = 0;
        clipRect.bottom = clipRect.top + WORLD_HEIGHT;
//      DrawDirectTextInRect( &retroTextSpec, &boundsRect, &clipRect, *offMap, 0,0);
        while ( retroTextSpec.thisPosition < retroTextSpec.textLength)
        {
            PlayVolumeSound(  kTeletype, kMediumLowVolume, kShortPersistence, kLowPrioritySound);
            DrawRetroTextCharInRect( &retroTextSpec, 3, &boundsRect, &clipRect, *thePixMapHandle, gNatePortLeft,
                gNatePortTop);

            waitTime = TickCount();
            while (( TickCount() - waitTime) < 3) {
                // DO NOTHING
            };
        }


//      DrawDirectTextStringClipped( levelName, color, *offMap, &lRect, 0, 0);
        LongRectToRect( &boundsRect, &tRect);
//      CopyOffWorldToRealWorld((WindowPtr)gTheWindow, &tRect);
    }
}

void UpdateLoadingInterface( long value, long total, Rect *contentRect)

{
    unsigned char   *getwidchar, *getwidwid, color;
    long            width, height, strlen, temp;
    transColorType  *transColor;
    longRect        clipRect;
    Rect            tRect;
    RGBColor        fadeColor = {0, 0, 0};
    PixMapHandle    offMap = GetGWorldPixMap( gOffWorld);
    Str255          string;

    if ( total < 0)
    {
        DrawInOffWorld();
        DefaultColors();
        PaintRect( contentRect);
        GetIndString( string, 2004, 33);

        mSetDirectFont( kButtonFontNum);
        mGetDirectStringDimensions( string, width, height, strlen, getwidchar, getwidwid)

        mCopyAnyRect( clipRect, *contentRect);
        MacSetRect( &tRect, 0, 0, width, height);
        CenterRectInRect( &tRect, contentRect);

        mGetTranslateColorShade( kLoadingScreenColor, LIGHTER, color, transColor)
        MoveTo( tRect.left, tRect.top + mDirectFontAscent);
        DrawDirectTextStringClipped( string, color, *offMap, &clipRect, 0, 0);


        DrawInRealWorld();
        DefaultColors();
        CopyOffWorldToRealWorld((WindowPtr)gTheWindow, contentRect);
    } else
    {
        width = contentRect->right - contentRect->left;
        DrawInOffWorld();

        temp = (value * width);
        temp /= total;

        MacSetRect( &tRect, contentRect->left, contentRect->top, contentRect->left + temp, contentRect->bottom);
        SetTranslateColorShadeFore( kLoadingScreenColor, LIGHT);
        PaintRect( &tRect);

        MacSetRect( &tRect, contentRect->left + temp, contentRect->top, contentRect->right, contentRect->bottom);
        SetTranslateColorShadeFore( kLoadingScreenColor, DARK);
        PaintRect( &tRect);
        NormalizeColors();
        DrawInRealWorld();
        NormalizeColors();
        MacSetRect( &tRect, contentRect->left, contentRect->top, contentRect->right, contentRect->bottom);
        CopyOffWorldToRealWorld((WindowPtr)gTheWindow, &tRect);
        if ( tRect.left >= tRect.right - 2) AutoFadeTo( 10, &fadeColor, FALSE);
    }
}

short DoPlayAgain( Boolean allowResume, Boolean allowSkip) // return 0 = quit, 1 = restart, 2 = resume, 3 = skip

{
    int                     error = kNoError;
    Rect                    tRect;
    interfaceItemType       *item;
    Boolean                 done = false;
    Point                   where;
    short                   whichItem, result = 0;
    EventRecord             theEvent;
    char                    whichChar;
    CWindowPtr              whichWindow;

//  BlackenWindow();

    FlushEvents(everyEvent, 0);
    if ( allowSkip) error = OpenInterface( 5017);
    else if ( allowResume) error = OpenInterface( kPlayAgainResumeID);
    else error = OpenInterface( kPlayAgainID);
    if ( error == kNoError)
    {
        MacSetRect( &tRect, CLIP_LEFT, CLIP_TOP, CLIP_RIGHT, CLIP_BOTTOM);
        CenterAllItemsInRect( &tRect);
        item = GetAnyInterfaceItemPtr( kPlayAgainBox);
        DrawInOffWorld();
//      LongRectToRect( &(item->bounds), &tRect);
        GetAnyInterfaceItemGraphicBounds( item, &tRect);
        DefaultColors();
        PaintRect( &tRect);
        DrawInRealWorld();

        if ( gAresGlobal->gOptions & kOptionNetworkOn)
        {
            SetStatusOfAnyInterfaceItem( kPlayAgainOKButton, kDimmed, false);
        }
        DrawAllItemsOfKind( kPictureRect, FALSE, false, false);
        DrawAllItemsOfKind( kLabeledRect, TRUE, FALSE, FALSE);
        DrawAllItemsOfKind( kPlainRect, TRUE, FALSE, FALSE);
        DrawAllItemsOfKind( kCheckboxButton, TRUE, FALSE, FALSE);
        DrawAllItemsOfKind( kRadioButton, TRUE, FALSE, FALSE);
        DrawAllItemsOfKind( kPlainButton, TRUE, FALSE, FALSE);
        DrawAllItemsOfKind( kTextRect, TRUE, FALSE, FALSE);
        while ( AnyRealKeyDown()) {
            // DO NOTHING
        };

        while ( !done)
        {

            InterfaceIdle();

            Ares_WaitNextEvent (everyEvent, &theEvent, 3, nil);
            {
                whichItem = -1;
                switch ( theEvent.what )
                {
                    case nullEvent:
                        InterfaceIdle();
                        if ( gAresGlobal->gOptions & kOptionInBackground)
                        {
                        } else if (AutoShowHideMenubar( theEvent.where, theDevice))
                        {
                        }
                        if ( gAresGlobal->returnToMain)
                        {
                            done = true;
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
                                MacSetPort( (WindowPtr)gTheWindow);
                                CopyOffWorldToRealWorld((WindowPtr)gTheWindow, &(gTheWindow->portRect));
                            EndUpdate( (WindowPtr)whichWindow);
                            break;
                            EndUpdate( (WindowPtr)whichWindow);
                        } else if ( whichWindow == gAresGlobal->gBackWindow)
                        {
                            BeginUpdate( (WindowPtr)whichWindow);
                                MacSetPort( (WindowPtr)gAresGlobal->gBackWindow);
                                MacFillRect(  &(gAresGlobal->gBackWindow->portRect), (Pattern *)&qd.black);
                            EndUpdate( (WindowPtr)whichWindow);
                        } else
                        {
                            BeginUpdate( (WindowPtr)whichWindow);
                            EndUpdate( (WindowPtr)whichWindow);
                        }
                        MacSetPort( (WindowPtr)gTheWindow);

                        break;


                    case mouseDown:
                        where = theEvent.where;
                        GlobalToLocal( &where);
                        HandleMouseDown( &theEvent);
                        whichItem = InterfaceMouseDown( where);
                        break;
                    case mouseUp:
                        break;
                    case keyDown:
                    case autoKey:
                        whichChar = theEvent.message & charCodeMask;

                        whichItem = InterfaceKeyDown( theEvent.message);
                        break;
                }
                switch ( whichItem)
                {
                    case kPlayAgainOKButton:
                        done = TRUE;
                        result = 1;
                        break;

                    case kPlayAgainNoButton:
                        done = true;
                        result = 0;
                        break;

                    case kPlayAgainResumeButton:
                        done = true;
                        result = 2;
                        break;

                    case 5:
                        done = true;
                        result = 3;
                        break;
                }

            }
        }
        DrawInOffWorld();
        GetAnyInterfaceItemGraphicBounds( item, &tRect);
        DefaultColors();
        PaintRect( &tRect);
        DrawInRealWorld();
        CloseInterface();
    }
    return ( result);
}

void DoNetSettings( void)

{
#if NETSPROCKET_AVAILABLE
    int                     error = kNoError;
    Rect                    tRect;
    interfaceItemType       *item;
    Boolean                 done = false, cancel = false, currentBandwidth = GetBandwidth();
    Point                   where;
    short                   whichItem, result = 0, currentRegistered = 0,
                            currentDelay = 0, i;
    EventRecord             theEvent;
    char                    whichChar;
    CWindowPtr              whichWindow;

//  BlackenWindow();

    FlushEvents(everyEvent, 0);
    OpenInterface( kNetSettingsID);
    if ( error == kNoError)
    {
        MacSetRect( &tRect, CLIP_LEFT, CLIP_TOP, CLIP_RIGHT, CLIP_BOTTOM);
        CenterAllItemsInRect( &tRect);
        item = GetAnyInterfaceItemPtr( kNetSettingsBox);
        DrawInOffWorld();
        GetAnyInterfaceItemGraphicBounds( item, &tRect);
        DefaultColors();
        CopyOffWorldToSaveWorld( &tRect);
        PaintRect( &tRect);
        DrawInRealWorld();

        currentRegistered = GetRegisteredSetting();
        currentDelay = GetResendDelay();
        if ( currentDelay <= 60) currentDelay = 0;
        else if ( currentDelay <= 120) currentDelay = 1;
        else if ( currentDelay <= 240) currentDelay = 2;
        else currentDelay = 3;
        for ( i = kNetSettingsFirstRegisteredRadio;
                i <= kNetSettingsLastRegisteredRadio; i++)
        {
            if (( i - kNetSettingsFirstRegisteredRadio) == currentRegistered)
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
            if (( i - kNetSettingsFirstDelayRadio) == currentDelay)
            {
                SwitchAnyRadioOrCheckbox( i, true);
            } else
            {
                SwitchAnyRadioOrCheckbox( i, false);
            }
        }

        SwitchAnyRadioOrCheckbox( kNetSettingsLowerBandwidthCheck, currentBandwidth);

        DrawAllItemsOfKind( kPictureRect, FALSE, false, false);
        DrawAllItemsOfKind( kLabeledRect, TRUE, FALSE, FALSE);
        DrawAllItemsOfKind( kPlainRect, TRUE, FALSE, FALSE);
        DrawAllItemsOfKind( kCheckboxButton, TRUE, FALSE, FALSE);
        DrawAllItemsOfKind( kRadioButton, TRUE, FALSE, FALSE);
        DrawAllItemsOfKind( kPlainButton, TRUE, FALSE, FALSE);
        DrawAllItemsOfKind( kTextRect, TRUE, FALSE, FALSE);
        while ( AnyRealKeyDown()) {
            // DO NOTHING
        };

        while ( !done)
        {

            InterfaceIdle();

            Ares_WaitNextEvent (everyEvent, &theEvent, 3, nil);
            {
                whichItem = -1;
                switch ( theEvent.what )
                {
                    case nullEvent:
                        InterfaceIdle();
                        if ( gAresGlobal->gOptions & kOptionInBackground)
                        {
                        } else if (AutoShowHideMenubar( theEvent.where, theDevice))
                        {
                        }
                        if ( gAresGlobal->returnToMain)
                        {
                            cancel = true;
                            done = true;
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
                                MacSetPort( (WindowPtr)gTheWindow);
                                CopyOffWorldToRealWorld((WindowPtr)gTheWindow, &(gTheWindow->portRect));
                            EndUpdate( (WindowPtr)whichWindow);
                            break;
                            EndUpdate( (WindowPtr)whichWindow);
                        } else if ( whichWindow == gAresGlobal->gBackWindow)
                        {
                            BeginUpdate( (WindowPtr)whichWindow);
                                MacSetPort( (WindowPtr)gAresGlobal->gBackWindow);
                                MacFillRect(  &(gAresGlobal->gBackWindow->portRect), (Pattern *)&qd.black);
                            EndUpdate( (WindowPtr)whichWindow);
                        } else
                        {
                            BeginUpdate( (WindowPtr)whichWindow);
                            EndUpdate( (WindowPtr)whichWindow);
                        }
                        MacSetPort( (WindowPtr)gTheWindow);

                        break;


                    case mouseDown:
                        where = theEvent.where;
                        GlobalToLocal( &where);
                        HandleMouseDown( &theEvent);
                        whichItem = InterfaceMouseDown( where);
                        break;
                    case mouseUp:
                        break;
                    case keyDown:
                    case autoKey:
                        whichChar = theEvent.message & charCodeMask;

                        whichItem = InterfaceKeyDown( theEvent.message);
                        break;
                }
                switch ( whichItem)
                {
                    case kNetSettingsOKButton:
                        done = TRUE;
                        break;

                    case kNetSettingsCancelButton:
                        done = true;
                        cancel = true;
                        break;

                    case kNetSettingsLowerBandwidthCheck:
                        currentBandwidth = !currentBandwidth;
                        break;

                    default:
                        if (( whichItem >= kNetSettingsFirstRegisteredRadio) &&
                            ( whichItem <= kNetSettingsLastRegisteredRadio))
                        {
                            SwitchAnyRadioOrCheckbox( currentRegistered +
                                kNetSettingsFirstRegisteredRadio, false);
                            RefreshInterfaceItem( currentRegistered +
                                kNetSettingsFirstRegisteredRadio);
                            currentRegistered = whichItem - kNetSettingsFirstRegisteredRadio;
                            SwitchAnyRadioOrCheckbox( currentRegistered +
                                kNetSettingsFirstRegisteredRadio, true);
                        } else if (( whichItem >= kNetSettingsFirstDelayRadio) &&
                            ( whichItem <= kNetSettingsLastDelayRadio))
                        {
                            SwitchAnyRadioOrCheckbox( currentDelay +
                                kNetSettingsFirstDelayRadio, false);
                            RefreshInterfaceItem( currentDelay +
                                kNetSettingsFirstDelayRadio);
                            currentDelay = whichItem - kNetSettingsFirstDelayRadio;
                            SwitchAnyRadioOrCheckbox( currentDelay +
                                kNetSettingsFirstDelayRadio, true);
                        }
                        break;
                }

            }
        }
        DrawInOffWorld();
        GetAnyInterfaceItemGraphicBounds( item, &tRect);
        DefaultColors();
        PaintRect( &tRect);
        CopySaveWorldToOffWorld( &tRect);
        DrawInRealWorld();
        CloseInterface();
    }

    if ( !cancel)
    {
        SetRegisteredSetting( currentRegistered);
        SetBandwidth( currentBandwidth);
        SendInGameMiscLongMessage( 0, eSetRegisteredStateMessage, currentRegistered, true, false);
        SendInGameMiscLongMessage( 0, eSetBandwidthMessage, currentBandwidth, true, false);
        switch( currentDelay)
        {
            case 0:
                SetResendDelay( 60);
                SendInGameMiscLongMessage( 0, eSetResendDelayMessage, 60, true, false);
                break;

            case 1:
                SetResendDelay( 120);
                SendInGameMiscLongMessage( 0, eSetResendDelayMessage, 120, true, false);
                break;

            case 2:
                SetResendDelay( 240);
                SendInGameMiscLongMessage( 0, eSetResendDelayMessage, 240, true, false);
                break;
    /*
            case 3:
                SetResendDelay( 360);
                SendInGameMiscLongMessage( 0, eSetResendDelayMessage, 360, true, false);
                break;
    */
        }
    }
#endif NETSPROCKET_AVAILABLE
}

void DoHelpScreen( void)

{
    int                     error = kNoError;
    Rect                    tRect, textRect;
    interfaceItemType       *item;
    Boolean                 done = false;
    Point                   where;
    short                   whichItem;
    EventRecord             theEvent;
    char                    whichChar;
    CWindowPtr              whichWindow;
    longRect                clipRect, boundsRect;
    long                    height, score = 0;
    retroTextSpecType       retroTextSpec;
    transColorType          *transColor;
    PixMapHandle            offMap;

    FlushEvents(everyEvent, 0);
    if ( gAresGlobal->gOptions & kOptionSubstituteFKeys)
        error = OpenInterface( kNOFHelpScreenID);
    else
        error = OpenInterface( kHelpScreenID);
    if ( error == kNoError)
    {
        MacSetRect( &tRect, CLIP_LEFT, CLIP_TOP, CLIP_RIGHT, gAresGlobal->gTrueClipBottom);
        CenterAllItemsInRect( &tRect);
        item = GetAnyInterfaceItemPtr( kHelpScreenBox);
        DrawInOffWorld();
        GetAnyInterfaceItemGraphicBounds( item, &tRect);
        LongRectToRect( &item->bounds, &textRect);
        DefaultColors();
        CopyOffWorldToSaveWorld( &tRect);
        PaintRect( &tRect);
        DrawInRealWorld();

        DrawAllItemsOfKind( kPictureRect, FALSE, false, false);
        DrawAllItemsOfKind( kLabeledRect, TRUE, FALSE, FALSE);
        DrawAllItemsOfKind( kPlainRect, TRUE, FALSE, FALSE);
        DrawAllItemsOfKind( kCheckboxButton, TRUE, FALSE, FALSE);
        DrawAllItemsOfKind( kRadioButton, TRUE, FALSE, FALSE);
        DrawAllItemsOfKind( kPlainButton, TRUE, FALSE, FALSE);
        DrawAllItemsOfKind( kTextRect, TRUE, FALSE, FALSE);

        if ( gAresGlobal->gOptions & kOptionSubstituteFKeys)
            retroTextSpec.text = GetResource( 'TEXT', kNOFHelpScreenTextID);
        else
            retroTextSpec.text = GetResource( 'TEXT', kHelpScreenTextID);
        if ( retroTextSpec.text != nil)
        {
            DetachResource( retroTextSpec.text);

/*          for ( l = 0; l < kKeyControlNum; l++)
            {
                GetKeyNumName( numString, GetKeyNumFromKeyMap( gAresGlobal->gKeyControl[l]));
                while ( numString[0] < 4)
                {
                    numString[0]++;
                    numString[numString[0]] = ' ';
                }
                GetIndString( tempString, kHelpScreenKeyStringID, l + 1);
                while ( Munger( retroTextSpec.text, 0, (tempString + 1), *tempString, numString + 1, *numString) > 0)
                 {
                    // DO NOTHING
                 };
            }
*/
            Replace_KeyCode_Strings_With_Actual_Key_Names( retroTextSpec.text, kKeyMapNameID,
                4);

            retroTextSpec.textLength = GetHandleSize( retroTextSpec.text);

            mSetDirectFont( kComputerFontNum)
            retroTextSpec.thisPosition = retroTextSpec.linePosition = retroTextSpec.lineCount = 0;
            retroTextSpec.tabSize = 220;
            mGetTranslateColorShade( RED, VERY_LIGHT, retroTextSpec.color, transColor)
            mGetTranslateColorShade( RED, VERY_DARK, retroTextSpec.backColor, transColor)
            retroTextSpec.originalColor = retroTextSpec.nextColor = retroTextSpec.color;
            retroTextSpec.originalBackColor = retroTextSpec.nextBackColor = retroTextSpec.backColor;

            retroTextSpec.topBuffer = 1;
            retroTextSpec.bottomBuffer = 1;

            height = DetermineDirectTextHeightInWidth( &retroTextSpec, textRect.right - textRect.left);

            boundsRect.left = textRect.left + (((textRect.right - textRect.left) / 2) -
                                ( retroTextSpec.autoWidth / 2));
            boundsRect.right = boundsRect.left + retroTextSpec.autoWidth;
            boundsRect.top = textRect.top + (((textRect.bottom - textRect.top) / 2) -
                                ( retroTextSpec.autoHeight / 2));
            boundsRect.bottom = boundsRect.top + retroTextSpec.autoHeight;
            retroTextSpec.xpos = boundsRect.left;
            retroTextSpec.ypos = boundsRect.top + mDirectFontAscent;

            clipRect.left = 0;
            clipRect.right = clipRect.left + WORLD_WIDTH;
            clipRect.top = 0;
            clipRect.bottom = clipRect.top + WORLD_HEIGHT;
            RectToLongRect( &textRect, &clipRect);

            offMap = GetGWorldPixMap( gOffWorld);
            DrawDirectTextInRect( &retroTextSpec, &clipRect, &clipRect, *offMap, 0, 0);
            CopyOffWorldToRealWorld((WindowPtr)gTheWindow, &tRect);

/*          while ( retroTextSpec.thisPosition < retroTextSpec.textLength)
            {
                PlayVolumeSound(  kTeletype, kMediumLowVolume, kShortPersistence, kLowPrioritySound);
                DrawRetroTextCharInRect( &retroTextSpec, -1, &boundsRect, &clipRect, *thePixMapHandle, gNatePortLeft,
                    gNatePortTop);

                waitTime = TickCount();
                while (( TickCount() - waitTime) < 3);
            }
*/          DisposeHandle( retroTextSpec.text);
        }

        while ( AnyRealKeyDown()) {
            // DO NOTHING
        };

        while ( !done)
        {

            InterfaceIdle();

            Ares_WaitNextEvent (everyEvent, &theEvent, 3, nil);
            {
                whichItem = -1;
                switch ( theEvent.what )
                {
                    case nullEvent:
                        InterfaceIdle();
                        if ( gAresGlobal->gOptions & kOptionInBackground)
                        {
                        } else if (AutoShowHideMenubar( theEvent.where, theDevice))
                        {
                        }
                        break;
                        if ( gAresGlobal->returnToMain)
                        {
                            done = true;
                        }
                    case osEvt:
//                      HandleOSEvent( &theEvent);
                        break;
                    case updateEvt:
                        whichWindow = ( CWindowPtr)theEvent.message;

                        if ( whichWindow == gTheWindow)
                        {
                            BeginUpdate( (WindowPtr)whichWindow);
                                MacSetPort( (WindowPtr)gTheWindow);
                                CopyOffWorldToRealWorld((WindowPtr)gTheWindow, &(gTheWindow->portRect));
                            EndUpdate( (WindowPtr)whichWindow);
                            break;
                            EndUpdate( (WindowPtr)whichWindow);
                        } else if ( whichWindow == gAresGlobal->gBackWindow)
                        {
                            BeginUpdate( (WindowPtr)whichWindow);
                                MacSetPort( (WindowPtr)gAresGlobal->gBackWindow);
                                MacFillRect(  &(gAresGlobal->gBackWindow->portRect), (Pattern *)&qd.black);
                            EndUpdate( (WindowPtr)whichWindow);
                        } else
                        {
                            BeginUpdate( (WindowPtr)whichWindow);
                            EndUpdate( (WindowPtr)whichWindow);
                        }
                        MacSetPort( (WindowPtr)gTheWindow);

                        break;


                    case mouseDown:
                        where = theEvent.where;
                        GlobalToLocal( &where);
                        HandleMouseDown( &theEvent);
                        whichItem = InterfaceMouseDown( where);
                        break;
                    case mouseUp:
                        break;
                    case keyDown:
                    case autoKey:
                        whichChar = theEvent.message & charCodeMask;

                        whichItem = InterfaceKeyDown( theEvent.message);
                        break;
                }
                switch ( whichItem)
                {
                    case kHelpScreenDoneButton:
                        done = TRUE;
                        break;

                }

            }
        }
        DrawInOffWorld();
        GetAnyInterfaceItemGraphicBounds( item, &tRect);
        DefaultColors();
        PaintRect( &tRect);
        CopySaveWorldToOffWorld( &tRect);
        DrawInRealWorld();
        CloseInterface();
    }
}

void StartPauseIndicator(StringPtr pauseString, unsigned char hue)
{
    unsigned char   *getwidchar, *getwidwid, color;
    long            width, height, strlen, count;
    Rect            tRect, stringRect;
    PixMapHandle    offMap = GetGWorldPixMap( gOffWorld), saveMap = GetGWorldPixMap( gSaveWorld);
    transColorType  *transColor;
    longRect        clipRect;

#pragma unused( hue)
    mSetDirectFont( kTitleFontNum);
    mGetDirectStringDimensions( pauseString, width, height, strlen, getwidchar, getwidwid)
    MacSetRect( &tRect, CLIP_LEFT, CLIP_TOP, CLIP_RIGHT, gAresGlobal->gTrueClipBottom);

    MacSetRect( &stringRect, 0, 0, width, height);
    CenterRectInRect( &stringRect, &tRect);
    mCopyAnyRect( tRect, stringRect);
    tRect.left -= 4;
    tRect.top -= 4;
    tRect.right += 4;
    tRect.bottom += 4;

    DrawInSaveWorld();
    DefaultColors();
    CopyRealWorldToSaveWorld( (WindowPtr)gTheWindow, &tRect);
    DrawInOffWorld();
    DefaultColors();
    CopySaveWorldToOffWorld( &tRect);
    DrawInOffWorld();

    mCopyAnyRect( clipRect, tRect);

    mGetTranslateColorShade( GREEN, DARKER, color, transColor)
//  DrawNateRectClipped( *offMap, &clipRect, &clipRect, 0, 0,color);
    for ( count = clipRect.top + 2; count < clipRect.bottom; count += 2)
    {
        DrawNateLine( *offMap, &clipRect, clipRect.left, count, clipRect.right - 1,
                    count, 0, 0, color);
    }

    mGetTranslateColorShade( GREEN, LIGHTER, color, transColor)
    DrawNateVBracket( *offMap, &clipRect, &clipRect, 0, 0,color);
    MoveTo( stringRect.left, stringRect.top + mDirectFontAscent);
    DrawDirectTextStringClipped( pauseString, color, *offMap, &clipRect, 0, 0);

    DrawInRealWorld();
    DefaultColors();
    mCopyAnyRect( tRect, clipRect);
    CopyOffWorldToRealWorld((WindowPtr)gTheWindow, &tRect);
}

void StopPauseIndicator( StringPtr pauseString)
{
    unsigned char   *getwidchar, *getwidwid;
    long            width, height, strlen;
    Rect            tRect, stringRect;
    PixMapHandle    offMap = GetGWorldPixMap( gOffWorld), saveMap = GetGWorldPixMap( gSaveWorld);

    mSetDirectFont( kTitleFontNum);
    mGetDirectStringDimensions( pauseString, width, height, strlen, getwidchar, getwidwid)
    MacSetRect( &tRect, CLIP_LEFT, CLIP_TOP, CLIP_RIGHT, gAresGlobal->gTrueClipBottom);

    MacSetRect( &stringRect, 0, 0, width, height);
    CenterRectInRect( &stringRect, &tRect);
    stringRect.left -= 4;
    stringRect.top -= 4;
    stringRect.right += 4;
    stringRect.bottom += 4;

    DrawInOffWorld();
    DefaultColors();
    CopySaveWorldToOffWorld( &stringRect);
    DrawInRealWorld();
    CopyOffWorldToRealWorld( (WindowPtr)gTheWindow, &stringRect);
    DrawInOffWorld();
    DefaultColors();
    PaintRect( &stringRect);

    DrawInRealWorld();
    DefaultColors();
}


void DrawInterfaceOneAtATime( void)

{
        DrawAllItemsOfKind( kPictureRect, FALSE, TRUE, TRUE);
        DrawAllItemsOfKind( kLabeledRect, TRUE, FALSE, FALSE);
        DrawAllItemsOfKind( kTabBox, TRUE, FALSE, FALSE);
        DrawAllItemsOfKind( kTabBoxButton, TRUE, FALSE, FALSE);
        DrawAllItemsOfKind( kPlainRect, TRUE, FALSE, FALSE);
        DrawAllItemsOfKind( kCheckboxButton, TRUE, FALSE, FALSE);
        DrawAllItemsOfKind( kRadioButton, TRUE, FALSE, FALSE);
        DrawAllItemsOfKind( kPlainButton, TRUE, FALSE, FALSE);
        DrawAllItemsOfKind( kTextRect, TRUE, FALSE, FALSE);
}

void DoOptionsInterface( void)

{
    Point                   where;
    int                     error;
    short                   whichItem;
    Boolean                 done = FALSE, cancel = FALSE;
    EventRecord             theEvent;
    char                    whichChar;
    CWindowPtr              whichWindow;
    Handle                  tempPrefs = gAresGlobal->gPreferencesData;
    OSErr                   err;
    preferencesDataType     *prefsData = nil;
    Rect                    volumeRect;

    BlackenWindow();

    FlushEvents(everyEvent, 0);
    err = HandToHand( &tempPrefs);
    if (( tempPrefs == nil) || ( err != noErr))
    {
        return;
    }

    MoveHHi( tempPrefs);
    HLock( tempPrefs);
    prefsData = (preferencesDataType *)*gAresGlobal->gPreferencesData;

    error = OpenInterface( kOptionsScreenID);
    SetOptionCheckboxes( prefsData->options);
    if ( !(gAresGlobal->gOptions & kOptionSpeechAvailable)) SetStatusOfAnyInterfaceItem( kOptionSpeechOnButton, kDimmed, false);
    GetAnyInterfaceItemContentBounds( GetAnyInterfaceItemPtr( kOptionVolumeBox), &volumeRect);
    if ( prefsData->volume == 0)
    {
        SetStatusOfAnyInterfaceItem( kOptionSoundDownButton, kDimmed, false);
    } else if ( prefsData->volume >= ( kMaxVolumePreference))
    {
        SetStatusOfAnyInterfaceItem( kOptionSoundUpButton, kDimmed, false);
    }

    if ( error == kNoError)
    {
//      DrawEntireInterface();
        DrawInterfaceOneAtATime();
        DrawOptionVolumeLevel( &volumeRect, prefsData->volume);
        while ( !done)
        {
            InterfaceIdle();
            Ares_WaitNextEvent (everyEvent, &theEvent, 3, nil);
            {
                whichItem = -1;
                switch ( theEvent.what )
                {
                    case nullEvent:
                        InterfaceIdle();
                        if ( gAresGlobal->gOptions & kOptionInBackground)
                        {
                        } else if (AutoShowHideMenubar( theEvent.where, theDevice))
                        {
                        }
                        if ( gAresGlobal->returnToMain)
                        {
                            done = true;
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
                                MacSetPort( (WindowPtr)gTheWindow);
                                CopyOffWorldToRealWorld((WindowPtr)gTheWindow, &(gTheWindow->portRect));
                            EndUpdate( (WindowPtr)whichWindow);
                            break;
                            EndUpdate( (WindowPtr)whichWindow);
                        } else if ( whichWindow == gAresGlobal->gBackWindow)
                        {
                            BeginUpdate( (WindowPtr)whichWindow);
                                MacSetPort( (WindowPtr)gAresGlobal->gBackWindow);
                                MacFillRect(  &(gAresGlobal->gBackWindow->portRect), (Pattern *)&qd.black);
                            EndUpdate( (WindowPtr)whichWindow);
                        } else
                        {
                            BeginUpdate( (WindowPtr)whichWindow);
                            EndUpdate( (WindowPtr)whichWindow);
                        }
                        MacSetPort( (WindowPtr)gTheWindow);

                        break;

                    case mouseDown:
                        where = theEvent.where;
                        GlobalToLocal( &where);
                        HandleMouseDown( &theEvent);
                        whichItem = InterfaceMouseDown( where);
                        break;
                    case mouseUp:
                        break;
                    case keyDown:
                    case autoKey:
                        whichChar = theEvent.message & charCodeMask;
                        whichItem = InterfaceKeyDown( theEvent.message);
                        break;
                }
                switch ( whichItem)
                {
                    case kOptionCancelButton:
                        done = TRUE;
                        cancel = TRUE;
                        break;

                    case kOptionDoneButton:
                        done = TRUE;

                        // if we've changed screen size
/*                      if ( (prefsData->options & ( kOptionScreenSmall | kOptionScreenMedium |
                                kOptionScreenLarge)) != ( gAresGlobal->gOptions & ( kOptionScreenSmall | kOptionScreenMedium |
                                kOptionScreenLarge)))
                        {
                            whichItem = ResizeWindowDialog( 2500);

                            // if cancel was hit then revert screen size options
                            if ( whichItem < 0)
                            {
                                prefsData->options &= ~( kOptionScreenSmall | kOptionScreenMedium |
                                                kOptionScreenLarge);
                                prefsData->options |= gAresGlobal->gOptions & ( kOptionScreenSmall | kOptionScreenMedium |
                                                kOptionScreenLarge);
                            }
                        }

*/                      break;

                    case kOptionSoundUpButton:
                        if ( prefsData->volume < kMaxVolumePreference)
                        {
                            if ( prefsData->volume == 0)
                            {
                                SetStatusOfAnyInterfaceItem( kOptionSoundDownButton, kActive, true);
                            }
                            prefsData->volume++;
                            if ( prefsData->volume >= (kMaxVolumePreference))
                            {
                                SetStatusOfAnyInterfaceItem( kOptionSoundUpButton, kDimmed, true);
                            }
                            DrawOptionVolumeLevel( &volumeRect, (long)prefsData->volume);
                            gAresGlobal->gSoundVolume = prefsData->volume;
                            if ( prefsData->options & kOptionMusicIdle)
                            {
                                SetSongVolume( kMaxMusicVolume);
                            }
                        }
                        PlayVolumeSound( kComputerBeep2, kMaxSoundVolume, kLongPersistence, kMustPlaySound);
                        break;

                    case kOptionSoundDownButton:
                        if ( prefsData->volume > 0)
                        {
                            if ( prefsData->volume == (kMaxVolumePreference))
                            {
                                SetStatusOfAnyInterfaceItem( kOptionSoundUpButton, kActive, true);
                            }
                            prefsData->volume--;
                            if ( prefsData->volume <= 0)
                            {
                                SetStatusOfAnyInterfaceItem( kOptionSoundDownButton, kDimmed, true);
                            }
                            DrawOptionVolumeLevel( &volumeRect, (long)prefsData->volume);
                            gAresGlobal->gSoundVolume = prefsData->volume;
                            if ( prefsData->options & kOptionMusicIdle)
                            {
                                SetSongVolume( kMaxMusicVolume);
                            }
                        }
                        PlayVolumeSound( kComputerBeep2, kMaxSoundVolume, kLongPersistence, kMustPlaySound);
                        break;

                    case kOptionKeyControlButton:
//                      #ifndef kNonPlayableDemo
                        CloseInterface();

                        done = Key_Setup_Screen_Do();//DoKeyInterface();

                        OpenInterface( kOptionsScreenID);

                        BlackenOffscreen();
                        if ( !done)
                        {
                            SetOptionCheckboxes( prefsData->options);
                            DrawInterfaceOneAtATime();
                            DrawOptionVolumeLevel( &volumeRect, prefsData->volume);
                        }
//                      #endif
                        break;

                    case kOptionGameMusicButton:
                        prefsData->options ^= kOptionMusicPlay;
                        break;

                    case kOptionIdleMusicButton:
                        prefsData->options ^= kOptionMusicIdle;
                        if ( prefsData->options & kOptionMusicIdle)
                        {
                            LoadSong( kTitleSongID);
                            SetSongVolume( kMaxMusicVolume);
                            PlaySong();
                        } else
                        {
                            StopAndUnloadSong();
                        }
                        break;

                    case kOptionSpeechOnButton:
                        prefsData->options ^= kOptionSpeechOn;
                        break;


/*                  case kOptionQuickDrawButton:
                        prefsData->options ^= kOptionQDOnly;
                        break;
*/
                }

            }
        }
        if ( !cancel)
        {
            SaveAllPreferences(); // sets gAresGlobal->gOptions
        } else
        {
            BlockMove( *tempPrefs, *gAresGlobal->gPreferencesData, sizeof( preferencesDataType));
            if ( gAresGlobal->gSoundVolume != prefsData->volume)
            {
                gAresGlobal->gSoundVolume = prefsData->volume;
                if ( prefsData->options & kOptionMusicIdle)
                {
                    SetSongVolume( kMaxMusicVolume);
                }
            }
            if ( prefsData->options & kOptionMusicIdle)
            {
                if ( !SongIsPlaying())
                {
                    LoadSong( kTitleSongID);
                    SetSongVolume( kMaxMusicVolume);
                    PlaySong();
                }
            } else if ( SongIsPlaying())
            {
                StopAndUnloadSong();
            }

        }
        CloseInterface();
    }
    HUnlock( tempPrefs);
    DisposeHandle( tempPrefs);
}

void SetOptionCheckboxes( unsigned long options)

{
    SwitchAnyRadioOrCheckbox( kOptionGameMusicButton, ((options & kOptionMusicPlay) ? (true):(false)));
    SwitchAnyRadioOrCheckbox( kOptionIdleMusicButton, ((options & kOptionMusicIdle) ? (true):(false)));
    SwitchAnyRadioOrCheckbox( kOptionSpeechOnButton, ((options & kOptionSpeechOn) ? (true):(false)));
//  SwitchAnyRadioOrCheckbox( kOptionQuickDrawButton, options & kOptionQDOnly);
//  SwitchAnyRadioOrCheckbox( kOptionRowSkipButton, options & kOptionRowSkip);
//  SwitchAnyRadioOrCheckbox( kOptionBlackgroundButton, options & kOptionBlackground);
//  SwitchAnyRadioOrCheckbox( kOptionNoScaleUpButton, options & kOptionNoScaleUp);
//  SwitchAnyRadioOrCheckbox( kOptionScreenSmallButton, options & kOptionScreenSmall);
//  SwitchAnyRadioOrCheckbox( kOptionScreenMediumButton, options & kOptionScreenMedium);
//  SwitchAnyRadioOrCheckbox( kOptionScreenLargeButton, options & kOptionScreenLarge);
}

void DrawOptionVolumeLevel( Rect *bounds, long level)
{
    long        notchWidth = (( bounds->right - bounds->left) / ( kMaxVolumePreference)),
                notchHeight = (( bounds->bottom - bounds->top) - 4), count, shade;

    Rect        notchBounds, tRect, graphicRect;

    notchBounds.left = bounds->left + ((( bounds->right - bounds->left) / 2) -
        (( notchWidth * (kMaxVolumePreference))/ 2));
    notchBounds.top = bounds->top + 2;
    notchBounds.bottom = notchBounds.top + notchHeight;
    notchBounds.right = notchBounds.left + (( notchWidth * (kMaxVolumePreference))/ 2);

    DrawInOffWorld();

    MacSetRect( &tRect, notchBounds.left, notchBounds.top,
          notchBounds.left + notchWidth - 2,
        notchBounds.bottom);

    shade = 2;
    for ( count = 0; count < level; count++)
    {
        SetTranslateColorShadeFore( kOptionVolumeColor, (unsigned char)shade);
        graphicRect = tRect;
        MacInsetRect( &graphicRect, 2, 6);
        CenterRectInRect( &graphicRect, &tRect);
        PaintRect( &graphicRect);
        tRect.left += notchWidth;
        tRect.right = tRect.left + notchWidth - 2;
        shade += 2;
    }

    NormalizeColors();
    for ( count = level; count < ( kMaxVolumePreference); count++)
    {
        graphicRect = tRect;
        MacInsetRect( &graphicRect, 2, 6);
        CenterRectInRect( &graphicRect, &tRect);
        PaintRect( &graphicRect);
        tRect.left += notchWidth;
        tRect.right = tRect.left + notchWidth - 2;
    }

    DrawInRealWorld();
    NormalizeColors();
    CopyOffWorldToRealWorld((WindowPtr)gTheWindow, bounds);
}

Boolean DoKeyInterface( void)

{
    Point                   where;
    int                     error;
    short                   whichItem, i, whichKeyButton = -1, keyNum = 0, currentKey = 0, checkKey;
    Boolean                 done = FALSE, result = TRUE, cancel = FALSE, keyUse = false;
    EventRecord             theEvent;
    KeyMap                  keyMap;
    CWindowPtr              whichWindow;
    preferencesDataType     *prefsData = nil;
    unsigned long           options = gAresGlobal->gOptions;

    BlackenOffscreen();

    FlushEvents(everyEvent, 0);
    error = OpenInterface( kKeyScreenID);
    if ( error == kNoError)
    {
        for ( i = 0; i < kKeyControlNum; i++)
        {
            SetButtonKeyNum( i,
                GetKeyNumFromKeyMap( gAresGlobal->gKeyControl[i]));
        }

        prefsData = (preferencesDataType *)*gAresGlobal->gPreferencesData;

        SwitchAnyRadioOrCheckbox( kKeySubstituteCheckbox,
            ((options & kOptionSubstituteFKeys) ? (true):(false)));

        DrawInterfaceOneAtATime();
        while ( !done)
        {
            InterfaceIdle();
            if (( AnyEvent()) && ( !( gAresGlobal->gOptions & kOptionInBackground)))
            {
                GetKeys( keyMap);
                keyNum = GetKeyNumFromKeyMap( keyMap);
                if ( currentKey > 0) keyNum = currentKey;

                // make sure it's not a reserved key
                if ( IsKeyReserved( keyMap, ((options & kOptionSubstituteFKeys) ?
                    ( true):(false))))
                {
                    PlayVolumeSound( kWarningTone, kMediumLowVolume, kShortPersistence, kMustPlaySound);
                    keyNum = -1;
                }

                // make sure it's not a key that's already in use
                checkKey = 0;
                while (( checkKey < kKeyControlNum) &&
                    ((checkKey == whichKeyButton) || ( GetButtonKeyNum( checkKey) != keyNum)))
                    checkKey++;
                if ( checkKey < kKeyControlNum)
                {
                    PlayVolumeSound( kWarningTone, kMediumLowVolume, kShortPersistence, kMustPlaySound);
                    keyNum = -1;
                    SetStatusOfAnyInterfaceItem( checkKey, kIH_Hilite, TRUE);
                    Pause( 12);
                    SetStatusOfAnyInterfaceItem( checkKey, kActive, TRUE);
                    Pause( 12);
                    SetStatusOfAnyInterfaceItem( checkKey, kIH_Hilite, TRUE);
                    Pause( 12);
                    while ( AnyEvent()) {
                        // DO NOTHING
                    };
                    SetStatusOfAnyInterfaceItem( checkKey, kActive, TRUE);
                }

                if (( keyNum > 0) && (whichKeyButton >= 0) && (whichKeyButton < kKeyControlNum))
                {
                    mPlayScreenSound;
                    SetButtonKeyNum( whichKeyButton, keyNum);
                    SetStatusOfAnyInterfaceItem( whichKeyButton, kIH_Hilite, TRUE);
                    do
                    {
                        GetKeys( keyMap);
                        currentKey = GetKeyNumFromKeyMap( keyMap);
                    } while ( currentKey > 0);

                    SetStatusOfAnyInterfaceItem( whichKeyButton, kActive, TRUE);
                    whichKeyButton++;
                    if ( whichKeyButton >= kKeyControlNum) whichKeyButton = 0;
                    SetStatusOfAnyInterfaceItem( whichKeyButton, kIH_Hilite, TRUE);
                    DrawKeyControlPicture( whichKeyButton);
                }
                keyNum = currentKey = 0;
            }
            Ares_WaitNextEvent (everyEvent, &theEvent, 3, nil);
            {
                whichItem = -1;
                switch ( theEvent.what )
                {
                    case nullEvent:
                        InterfaceIdle();
                        if ( gAresGlobal->gOptions & kOptionInBackground)
                        {
                        } else if (AutoShowHideMenubar( theEvent.where, theDevice))
                        {
                        }
                        if ( gAresGlobal->returnToMain)
                        {
                            done = true;
                            result = false;
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
                                MacSetPort( (WindowPtr)gTheWindow);
                                CopyOffWorldToRealWorld((WindowPtr)gTheWindow, &(gTheWindow->portRect));
                            EndUpdate( (WindowPtr)whichWindow);
                            break;
                            EndUpdate( (WindowPtr)whichWindow);
                        } else if ( whichWindow == gAresGlobal->gBackWindow)
                        {
                            BeginUpdate( (WindowPtr)whichWindow);
                                MacSetPort( (WindowPtr)gAresGlobal->gBackWindow);
                                MacFillRect(  &(gAresGlobal->gBackWindow->portRect), (Pattern *)&qd.black);
                            EndUpdate( (WindowPtr)whichWindow);
                        } else
                        {
                            BeginUpdate( (WindowPtr)whichWindow);
                            EndUpdate( (WindowPtr)whichWindow);
                        }
                        MacSetPort( (WindowPtr)gTheWindow);

                        break;

                    case mouseDown:
                        where = theEvent.where;
                        GlobalToLocal( &where);
                        HandleMouseDown( &theEvent);
                        whichItem = InterfaceMouseDown( where);
                        break;
                    case mouseUp:
                        break;
                    case keyDown:
                    case autoKey:
//                      whichChar = theEvent.message & charCodeMask;
//                      whichItem = InterfaceKeyDown( theEvent.message);
                        break;
                }

                if (( whichItem >= 0) &&  (whichItem < kKeyControlNum))
                {
                    if (( whichKeyButton >= 0) &&  (whichKeyButton < kKeyControlNum))
                        SetStatusOfAnyInterfaceItem( whichKeyButton, kActive, TRUE);
                    SetStatusOfAnyInterfaceItem( whichItem, kIH_Hilite, TRUE);
                    whichKeyButton = whichItem;
                    DrawKeyControlPicture( whichKeyButton);
                }

                switch ( whichItem)
                {
                    case kKeyCancelButton:
                        cancel = TRUE;
                        done = TRUE;
                        break;
                    case kKeyDoneButton:
                        done = TRUE;
                        break;
                    case kKeyOptionButton:
                        result = FALSE;
                        done = TRUE;
                        break;

                    case kKeySubstituteCheckbox:
                        options ^= kOptionSubstituteFKeys;

                        // make sure we're not using a key that is now reserved
                        checkKey = -1;
                        for ( i = 0; (i < kKeyControlNum) && ( checkKey == -1); i++)
                        {
                            GetKeyMapFromKeyNum( GetButtonKeyNum( i), keyMap);
                            if ( IsKeyReserved( keyMap,
                                ((options & kOptionSubstituteFKeys) ?
                                ( true):(false))))
                            {
                                checkKey = i;
                            }
                        }

                        if ( checkKey != -1)
                        {
                            PlayVolumeSound( kWarningTone, kMediumLowVolume, kShortPersistence, kMustPlaySound);
                            keyNum = -1;
                            SetStatusOfAnyInterfaceItem( checkKey, kIH_Hilite, TRUE);
                            Pause( 12);
                            SetStatusOfAnyInterfaceItem( checkKey, kActive, TRUE);
                            Pause( 12);
                            SetStatusOfAnyInterfaceItem( checkKey, kIH_Hilite, TRUE);
                            Pause( 12);
                            while ( AnyEvent()) {
                                // DO NOTHING
                            };
                            if ( checkKey != whichKeyButton)
                                SetStatusOfAnyInterfaceItem( checkKey, kActive, TRUE);
                            options ^= kOptionSubstituteFKeys;
                            SwitchAnyRadioOrCheckbox( kKeySubstituteCheckbox,
                                ((options & kOptionSubstituteFKeys) ? (true):(false)));
                            RefreshInterfaceItem( kKeySubstituteCheckbox);
                        }

                        break;
                }

            }
        }
        if ( !cancel)
        {
            for ( i = 0; i < kKeyControlNum; i++)
            {
                GetKeyMapFromKeyNum( GetButtonKeyNum( i), gAresGlobal->gKeyControl[i]);
            }
            prefsData->options = ( prefsData->options & ~kOptionSubstituteFKeys) |
                                ( options & kOptionSubstituteFKeys);
            SaveKeyControlPreferences();
            SaveAllPreferences();
        }
        CloseInterface();
    }
    WriteDebugLine((char *)"\pRESULT:");
    WriteDebugLong( result);
    return( result);
}

//
// BothCommandAndQ:
//  returns true if both the command and q keys are set by player. If this is
//  true, then command-q for quit during a game should be disabled.
//

Boolean BothCommandAndQ( void)
{
    Boolean command = false, q = false;
    short   b;

    for ( b = 0; b < kKeyExtendedControlNum; b++)
    {
        if ( mQKey( gAresGlobal->gKeyControl[b])) q = true;
        if ( mCommandKey( gAresGlobal->gKeyControl[b])) command = true;
    }

    if (( q) && ( command)) return ( true);
    else return( false);
}

Boolean IsKeyReserved( KeyMap keyMap, Boolean alternateFKey)
{
#pragma unused( alternateFKey)

/*  if ( alternateFKey)
    {
        if ( mNOFHelpKey( keyMap)) return( true);
        if ( mNOFVolumeDownKey( keyMap)) return( true);
        if ( mNOFVolumeUpKey( keyMap)) return( true);
        if ( mNOFNetSettingsKey( keyMap)) return( true);
        if ( mNOFFastMotionKey( keyMap)) return( true);
        if ( mNOFScale221Key( keyMap)) return( true);
        if ( mNOFScale121Key( keyMap)) return( true);
        if ( mNOFScale122Key( keyMap)) return( true);
        if ( mNOFScale124Key( keyMap)) return( true);
        if ( mNOFScaleHostileKey( keyMap)) return( true);
        if ( mNOFScaleObjectKey( keyMap)) return( true);
        if ( mNOFScaleAllKey( keyMap)) return( true);
    } else
    {
        if ( mHelpKey( keyMap)) return( true);
        if ( mVolumeDownKey( keyMap)) return( true);
        if ( mVolumeUpKey( keyMap)) return( true);
        if ( mActionMusicKey( keyMap)) return( true);
        if ( mNetSettingsKey( keyMap)) return( true);
        if ( mTransferKey( keyMap)) return( true);
        if ( mSlowMotionKey( keyMap)) return( true);
        if ( mFastMotionKey( keyMap)) return( true);
        if ( mScale221Key( keyMap)) return( true);
        if ( mScale121Key( keyMap)) return( true);
        if ( mScale122Key( keyMap)) return( true);
        if ( mScale124Key( keyMap)) return( true);
        if ( mScaleHostileKey( keyMap)) return( true);
        if ( mScaleObjectKey( keyMap)) return( true);
        if ( mScaleAllKey( keyMap)) return( true);
    }
*/
    // not related to fkey
//  if ( mDeleteKey( keyMap)) return( true);
    if ( mPauseKey( keyMap)) return( true);
    if ( mEnterTextKey( keyMap)) return( true);
//  if ( mQKey( keyMap)) return( true);
    if ( mRestartResumeKey( keyMap)) return( true);
    return( false);
}

void DrawKeyControlPicture( long whichKey)
{
    Rect    tRect, newRect;
    PicHandle       thePict = nil;

    GetAnyInterfaceItemContentBounds( GetAnyInterfaceItemPtr( kKeyIllustrationBox), &tRect);

    DrawInOffWorld();

    thePict = (PicHandle)HHGetResource( 'PICT', kKeyIllustrationPictID);
    if ( thePict != nil)
    {
        HLockHi( (Handle)thePict);
        newRect = (**thePict).picFrame;
        CenterRectInRect( &newRect, &tRect);
        DrawPicture( thePict, &newRect);
        ReleaseResource( (Handle)thePict);
        thePict = nil;
    }
    if ( whichKey >= kSelectFriendKeyNum)
    {
        if ( whichKey < kCompUpKeyNum)
        {
            whichKey = kSelectFriendKeyNum;
        } else
        {
            whichKey = kSelectFriendKeyNum + 1;
        }
    }
    thePict = (PicHandle)HHGetResource( 'PICT', kKeyIllustrationPictID + 1 + whichKey);
    if ( thePict != nil)
    {
        HLockHi( (Handle)thePict);
        newRect = (**thePict).picFrame;
        CenterRectInRect( &newRect, &tRect);
        DrawPicture( thePict, &newRect);
        ReleaseResource( (Handle)thePict);
    }
    DrawInRealWorld();
    CopyOffWorldToRealWorld((WindowPtr)gTheWindow, &tRect);

}


netResultType StartNetworkGameSetup( void)

{
#if NETSPROCKET_AVAILABLE
    Point                   where;
    int                     error;
    short                   whichItem = -1;
    Boolean                 done = FALSE;
    EventRecord             theEvent;
    char                    whichChar;
    netResultType           result = kCancel;
    CWindowPtr              whichWindow;

    if ( gAresGlobal->gameRangerPending)
    {
        if ( Wrap_GRIsHostCmd())
        {
            whichItem = kNetSetupHostButton;
        } else if ( Wrap_GRIsJoinCmd())
        {
            whichItem = kNetSetupJoinButton;
        }
    }

    FlushEvents(everyEvent, 0);
    if ( whichItem == -1)
    {
        error = OpenInterface( kStartNetworkGameID);
        if ( error == kNoError)
        {
            DrawInterfaceOneAtATime();
            MacSetPort( (WindowPtr)gTheWindow);
            InvalRect( &(gTheWindow->portRect));
            while ( !done)
            {
                InterfaceIdle();
                Ares_WaitNextEvent (everyEvent, &theEvent, 3, nil);
                {
                    whichItem = -1;
                    switch ( theEvent.what )
                    {
                        case nullEvent:
                            InterfaceIdle();
                            if ( gAresGlobal->gOptions & kOptionInBackground)
                            {
                            } else if (AutoShowHideMenubar( theEvent.where, theDevice))
                            {

                            }
                            if ( gAresGlobal->returnToMain)
                            {
                                done = true;
                                result = kCancel;
                            }
                            break;
                        case osEvt:
//                          HandleOSEvent( &theEvent);
                            break;
                        case updateEvt:
                            whichWindow = ( CWindowPtr)theEvent.message;

                            if ( whichWindow == gTheWindow)
                            {
                                BeginUpdate( (WindowPtr)whichWindow);
                                    MacSetPort( (WindowPtr)gTheWindow);
                                    CopyOffWorldToRealWorld((WindowPtr)gTheWindow, &(gTheWindow->portRect));
                                EndUpdate( (WindowPtr)whichWindow);
                                break;
                                EndUpdate( (WindowPtr)whichWindow);
                            } else if ( whichWindow == gAresGlobal->gBackWindow)
                            {
                                BeginUpdate( (WindowPtr)whichWindow);
                                    MacSetPort( (WindowPtr)gAresGlobal->gBackWindow);
                                    MacFillRect(  &(gAresGlobal->gBackWindow->portRect), (Pattern *)&qd.black);
                                EndUpdate( (WindowPtr)whichWindow);
                            } else
                            {
                                BeginUpdate( (WindowPtr)whichWindow);
                                EndUpdate( (WindowPtr)whichWindow);
                            }
                            MacSetPort( (WindowPtr)gTheWindow);
                            break;
                        case mouseDown:
                            where = theEvent.where;
                            GlobalToLocal( &where);
                            HandleMouseDown( &theEvent);
                            whichItem = InterfaceMouseDown( where);
                            break;
                        case mouseUp:
                            break;
                        case keyDown:
                        case autoKey:
                            whichChar = theEvent.message & charCodeMask;
                            whichItem = InterfaceKeyDown( theEvent.message);
                            break;

                    }

                    switch ( whichItem)
                    {
                        case kNetSetupCancelButton:
                            done = TRUE;
                            result = kCancel;
                            break;

                        case kNetSetupHostButton:
//                          BlackenWindow();
//                          if ( DoHostGame()) result = kHost;
                            done = TRUE;
                            break;

                        case kNetSetupJoinButton:
//                          BlackenWindow();
//                          if ( DoJoinGameModalDialog()) result = kClient;
                            done = TRUE;
                            break;
                    }

                }
            }
            CloseInterface();
        }
    }
    switch( whichItem)
    {
        case kNetSetupHostButton:
            BlackenWindow();
            if ( DoHostGame()) result = kHost;
            break;

        case kNetSetupJoinButton:
            BlackenWindow();
            if ( DoJoinGameModalDialog()) result = kClient;
            break;
    }
    return ( result);
#else
    return( kCancel);
#endif NETSPROCKET_AVAILABLE
}

void DrawStringInInterfaceItem( long whichItem, StringPtr string)
{
    PixMapHandle        offMap = GetGWorldPixMap( gOffWorld);
    Rect                tRect;
    interfaceItemType   *anItem;

    DrawInOffWorld();
    DefaultColors();
    GetAnyInterfaceItemContentBounds( GetAnyInterfaceItemPtr( whichItem), &tRect);

    PaintRect( &tRect);
    anItem = GetAnyInterfaceItemPtr( whichItem);
    if ( string != nil)
    {
        DrawInterfaceTextInRect( &tRect, string + 1, string[0],
                                anItem->style, anItem->color, *offMap,
                                0, 0, nil);
    }
    DrawInRealWorld();
    CopyOffWorldToRealWorld((WindowPtr)gTheWindow, &tRect);
}

netResultType ClientWaitInterface( void)

{
#if NETSPROCKET_AVAILABLE
    Point                   where;
    int                     error;
    short                   whichItem;
    Boolean                 done = FALSE;
    EventRecord             theEvent;
    char                    whichChar;
    netResultType           result = kCancel;
    long                    theMessage, roundTripTime, version, serialNumerator,
                            serialDenominator;
    Str255                  s;
    CWindowPtr              whichWindow;

    if ( gAresGlobal->gameRangerPending)
    {
        gAresGlobal->gameRangerPending = false;
        gAresGlobal->gameRangerInProgress = true;

    }

    FlushEvents(everyEvent, 0);
    error = OpenInterface( kClientWaitID);
    if ( error == kNoError)
    {
        SetStatusOfAnyInterfaceItem( kClientWaitCancelButton, kDimmed, false);
        DrawInterfaceOneAtATime();
        GetIndString( s, kClientWaitStrID, 18);
//      CopyPString(s, "\pLooking for host (cannont cancel).");
//      DebugStr(s);
        DrawStringInInterfaceItem( kClientWaitStatusRect, s);
        if ( DoJoinGame())
        {
            GetIndString( s, kClientWaitStrID, kClientWaitHostWaitingStrNum);
            DrawStringInInterfaceItem( kClientWaitStatusRect, s);
        } else
        {
            GetIndString( s, kClientWaitStrID, 19);
            DrawStringInInterfaceItem( kClientWaitStatusRect, s);
        }

        SetStatusOfAnyInterfaceItem( kClientWaitCancelButton, kActive, true);

        while ( !done)
        {
            InterfaceIdle();
            Ares_WaitNextEvent (everyEvent, &theEvent, 0, nil);
            {
                whichItem = -1;
                switch ( theEvent.what )
                {
                    case nullEvent:
                        InterfaceIdle();
                        if ( gAresGlobal->gOptions & kOptionInBackground)
                        {
                        } else if (AutoShowHideMenubar( theEvent.where, theDevice))
                        {
                        }
                        if ( gAresGlobal->returnToMain)
                        {
                            whichItem = kClientWaitCancelButton;
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
                                MacSetPort( (WindowPtr)gTheWindow);
                                CopyOffWorldToRealWorld((WindowPtr)gTheWindow, &(gTheWindow->portRect));
                            EndUpdate( (WindowPtr)whichWindow);
                            break;
                            EndUpdate( (WindowPtr)whichWindow);
                        } else if ( whichWindow == gAresGlobal->gBackWindow)
                        {
                            BeginUpdate( (WindowPtr)whichWindow);
                                MacSetPort( (WindowPtr)gAresGlobal->gBackWindow);
                                MacFillRect(  &(gAresGlobal->gBackWindow->portRect), (Pattern *)&qd.black);
                            EndUpdate( (WindowPtr)whichWindow);
                        } else
                        {
                            BeginUpdate( (WindowPtr)whichWindow);
                            EndUpdate( (WindowPtr)whichWindow);
                        }
                        MacSetPort( (WindowPtr)gTheWindow);
                        break;

                    case mouseDown:
                        where = theEvent.where;
                        GlobalToLocal( &where);
                        HandleMouseDown( &theEvent);
                        whichItem = InterfaceMouseDown( where);
                        break;
                    case mouseUp:
                        break;
                    case keyDown:
                    case autoKey:
                        whichChar = theEvent.message & charCodeMask;
                        whichItem = InterfaceKeyDown( theEvent.message);
                        break;
                }

                switch ( whichItem)
                {
                    case kClientWaitCancelButton:
                        done = TRUE;
                        result = kCancel;
                        StopNetworking();
                        break;
                }
                do
                {
                    theMessage = ProcessPreGameMessages( nil, &version, &serialNumerator,
                        &serialDenominator, nil,
                        &roundTripTime, 0, nil, nil, nil, nil);

                    switch( theMessage)
                    {
                        case eHostAcceptsMessage:
                            if (( version == kThisVersion)
#ifdef kUsePublicCopyProtection
                                && (
                                    (
                                        ( serialNumerator != gAresGlobal->gSerialNumerator)
                                        ||
                                        ( serialDenominator !=
                                            gAresGlobal->gSerialDenominator)
                                        )
                                        ||
                                        (
                                            ( serialNumerator == 0)
                                            &&
                                            ( serialDenominator == 0)
                                        )
                                    )
#endif kUsePublicCopyProtection
                                )
                            {
                                GetIndString( s, kClientWaitStrID, kClientWaitHostAcceptedStrNum);
                                DrawStringInInterfaceItem( kClientWaitStatusRect, s);
                                done = true;
                                result = kClient;
                                SendPreGameVerboseMessage( eClientReadyMessage,
                                    kThisVersion, gAresGlobal->gSerialNumerator,
                                    gAresGlobal->gSerialDenominator, 0);
                                if ( ( serialNumerator == 0) &&
                                            ( serialDenominator == 0))
                                    SetOpponentIsUnregistered( true);
                                else
                                    SetOpponentIsUnregistered( false);
                            } else
                            {
                                SendPreGameVerboseMessage( eClientReadyMessage,
                                    kThisVersion, gAresGlobal->gSerialNumerator,
                                    gAresGlobal->gSerialDenominator, 0);

                                if ( version < kThisVersion)
                                    ShowErrorAny( eContinueOnlyErr, kErrorStrID,
                                        nil, nil, nil, nil, kOlderVersionError, -1, -1, -1, __FILE__, 0);
                                else if ( version > kThisVersion)
                                    ShowErrorAny( eContinueOnlyErr, kErrorStrID,
                                        nil, nil, nil, nil, kNewerVersionError, -1, -1, -1, __FILE__, 0);
                                else
                                    ShowErrorAny( eContinueOnlyErr, kErrorStrID,
                                        nil, nil, nil, nil, kSameSerialNumberError, -1, -1, -1, __FILE__, 0);

                            }
                            break;

                        case eHostDeclinesMessage:
                            GetIndString( s, kClientWaitStrID, kClientWaitHostDeclinedStrNum);
                            DrawStringInInterfaceItem( kClientWaitStatusRect, s);
                            StopNetworking();
                            break;

                    }
                } while ( theMessage != eNoMessage);

            }
        }
        CloseInterface();
    }
    return ( result);
#else
    return ( kCancel);
#endif NETSPROCKET_AVAILABLE
}

netResultType HostAcceptClientInterface( void)

{
#if NETSPROCKET_AVAILABLE
    Point                   where;
    int                     error;
    short                   whichItem;
    Boolean                 done = FALSE;
    EventRecord             theEvent;
    char                    whichChar;
    netResultType           result = kCancel;
    long                    theMessage, roundTripTime, version, serialNumerator,
                            serialDenominator;
    Str31                   s;
    StringPtr               name;
    CWindowPtr              whichWindow;

    if ( gAresGlobal->gameRangerPending)
    {
        gAresGlobal->gameRangerPending = false;
        gAresGlobal->gameRangerInProgress = true;

        Wrap_GRHostReady();
    }


    FlushEvents(everyEvent, 0);
    error = OpenInterface( kNetHostID);
    if ( error == kNoError)
    {
        if ( GetNumberOfPlayers() == 2)
        {
            SetStatusOfAnyInterfaceItem( kHostAcceptButton, kActive, false);
            SetStatusOfAnyInterfaceItem( kHostDeclineButton, kActive, false);
        } else
        {
            SetStatusOfAnyInterfaceItem( kHostAcceptButton, kDimmed, false);
            SetStatusOfAnyInterfaceItem( kHostDeclineButton, kDimmed, false);
        }
        DrawInterfaceOneAtATime();
        while ( !done)
        {
            InterfaceIdle();
            Ares_WaitNextEvent (everyEvent, &theEvent, 0, nil);
            {
                whichItem = -1;
                switch ( theEvent.what )
                {
                    case nullEvent:
                        InterfaceIdle();
                        if ( gAresGlobal->gOptions & kOptionInBackground)
                        {
                        } else if (AutoShowHideMenubar( theEvent.where, theDevice))
                        {
                        }
                        if ( gAresGlobal->returnToMain)
                        {
                            whichItem = kHostCancelButton;
                        }
                        break;
                    case osEvt:
//                      HandleOSEvent( &theEvent);
                        break;
                    case mouseDown:
                        where = theEvent.where;
                        GlobalToLocal( &where);
                        HandleMouseDown( &theEvent);
                        whichItem = InterfaceMouseDown( where);
                        break;
                    case mouseUp:
                        break;
                    case keyDown:
                    case autoKey:
                        whichChar = theEvent.message & charCodeMask;
                        whichItem = InterfaceKeyDown( theEvent.message);
                        break;

                    case updateEvt:
                        whichWindow = ( CWindowPtr)theEvent.message;

                        if ( whichWindow == gTheWindow)
                        {
                            BeginUpdate( (WindowPtr)whichWindow);
                                MacSetPort( (WindowPtr)gTheWindow);
                                CopyOffWorldToRealWorld((WindowPtr)gTheWindow, &(gTheWindow->portRect));
                            EndUpdate( (WindowPtr)whichWindow);
                            break;
                            EndUpdate( (WindowPtr)whichWindow);
                        } else if ( whichWindow == gAresGlobal->gBackWindow)
                        {
                            BeginUpdate( (WindowPtr)whichWindow);
                                MacSetPort( (WindowPtr)gAresGlobal->gBackWindow);
                                MacFillRect(  &(gAresGlobal->gBackWindow->portRect), (Pattern *)&qd.black);
                            EndUpdate( (WindowPtr)whichWindow);
                        } else
                        {
                            BeginUpdate( (WindowPtr)whichWindow);
                            EndUpdate( (WindowPtr)whichWindow);
                        }
                        MacSetPort( (WindowPtr)gTheWindow);
                        break;
                }

                switch ( whichItem)
                {
                    case kHostCancelButton:
                        done = TRUE;
                        result = kCancel;
                        StopNetworking();
                        break;

                    case kHostAcceptButton:
                        SendPreGameVerboseMessage( eHostAcceptsMessage, kThisVersion,
                            gAresGlobal->gSerialNumerator,
                            gAresGlobal->gSerialDenominator,
                            0);
                        break;

                    case kHostDeclineButton:
                        SendPreGameBasicMessage( eHostDeclinesMessage);
                        break;


                }
            }
//          SendPreGameBasicMessage( eRoundTripGetReadyMessage);
            do
            {
                theMessage = ProcessPreGameMessages( nil, &version, &serialNumerator,
                    &serialDenominator, nil, &roundTripTime, 0,
                    nil, nil, nil, nil);
/*              if ( roundTripTime != -1)
                {
                    mWriteDebugString("\pRound Trip!");
                    WriteDebugLong( roundTripTime);
                }
*/              switch( theMessage)
                {
                    case kNSpPlayerJoined:
                    case kNSpPlayerLeft:
                        if ( GetNumberOfPlayers() == 2)
                        {
                            SetStatusOfAnyInterfaceItem( kHostAcceptButton, kActive, true);
                            SetStatusOfAnyInterfaceItem( kHostDeclineButton, kActive, true);
                            GetOtherPlayerName( &name);
                            DrawStringInInterfaceItem( kHostHostNameRect, name);
                            PlayVolumeSound( kComputerBeep2, kMediumLowVolume, kShortPersistence, kMustPlaySound);
                        } else
                        {
                            SetStatusOfAnyInterfaceItem( kHostAcceptButton, kDimmed, true);
                            SetStatusOfAnyInterfaceItem( kHostDeclineButton, kDimmed, true);
                            s[0] = 0;
                            DrawStringInInterfaceItem( kHostHostNameRect, s);
                        }
                        break;

                    case eClientReadyMessage:
                        if ( version < kThisVersion)
                        {
                            ShowErrorAny( eContinueOnlyErr, kErrorStrID,
                                nil, nil, nil, nil, kOlderVersionError, -1, -1, -1, __FILE__, 0);
                        } else if ( version > kThisVersion)
                        {
                            ShowErrorAny( eContinueOnlyErr, kErrorStrID,
                                nil, nil, nil, nil, kNewerVersionError, -1, -1, -1, __FILE__, 0);
                        } else if (( serialNumerator == gAresGlobal->gSerialNumerator)
                            && ( serialDenominator == gAresGlobal->gSerialDenominator)
                            && (( serialNumerator != 0) || ( serialDenominator != 0)))
                        {
                            ShowErrorAny( eContinueOnlyErr, kErrorStrID,
                                nil, nil, nil, nil, kSameSerialNumberError, -1, -1, -1, __FILE__, 0);
                        } else
                        {
                            if ( ( serialNumerator == 0) &&
                                        ( serialDenominator == 0))
                                SetOpponentIsUnregistered( true);
                            else
                                SetOpponentIsUnregistered( false);
                            done = true;
                            result = kHost;
                        }
                        break;
                }
            } while ( theMessage != eNoMessage);
        }
        CloseInterface();
    }
    return ( result);
#else
    return( kCancel);
#endif
}

/*
netResultType HostNetworkGameSetup( void)

{
    Point                   where;
    int                     error, addressesFound = 0;
    short                   whichItem, count;
    Boolean                 done = FALSE;
    EventRecord             theEvent;
    char                    whichChar;
    netResultType           result = kCancel;
    Handle                  addressList;
    AddrBlock               *anAddress;
    netClientEntity         *aClient;
    netSetupDataType        netData, *receivedData = nil;
    long                    lastPoll = 0;
    Str255                  s;

    // clear the screen

    BlackenWindow();

    // get the interface in memory

    error = OpenInterface( kNetHostID);
    SetInterfaceListCallback( kHostAvailableList, GetClientListLength, GetClientListName,
                        IsThisClientHilited);
    SetInterfaceListCallback( kHostInGameList, GetInGameListLength, GetInGameListName,
                        IsThisInGameHilited);

    // create & clear the list of incoming address from name binding search

    addressList = NewHandle( sizeof( AddrBlock) * (long)kMaxAvailableClient);
    if ( addressList == nil)
        error = MEMORY_ERROR;
    else
    {
        MoveHHi( addressList);
        HLock( addressList);
    }

    // clear the list of client entities

    aClient = ( netClientEntity *)*gClientEntity;
    for ( count = 0; count < kMaxAvailableClient; count++)
    {
        aClient->status = kDoesNotExist;
        aClient->entity.address.aNode = aClient->entity.address.aNet = aClient->entity.address.aSocket = 0;
        aClient->selected = FALSE;
        aClient++;
    }
    aClient = ( netClientEntity *)*gClientEntity;

    // if all is OK, proceed

    if ( error == kNoError)
    {

        // draw the entire interface

        DrawEntireInterface();

        // repeat until done

        while ( !done)
        {
            // first clear the address list and the client list's stillThere flags

            anAddress = ( AddrBlock *)*addressList;
            aClient = ( netClientEntity *)*gClientEntity;
            for ( count = 0; count < kMaxAvailableClient; count++)
            {
                anAddress->aNode = anAddress->aNet = anAddress->aSocket = 0;
                aClient->stillThere = FALSE;
                anAddress++;
                aClient++;
            }

            SetStatusOfAnyInterfaceItem( kHostInviteButton, kDimmed, TRUE);
            SetStatusOfAnyInterfaceItem( kHostCancelButton, kDimmed, TRUE);
            SetStatusOfAnyInterfaceItem( kHostBeginButton, kDimmed, TRUE);

            // get all addresses from name binding protocol (NBP)

            WriteDebugLine( (char *)"\pPolling");
            addressesFound = NatewerkGetName( (AddrBlock *)*addressList, kAresNetObjectName,
                            kAresNetTypeName, kMaxAvailableClient);
            WriteDebugLine( (char *)"\pPolled");
            lastPoll = TickCount();

            // step through list of address we got

            anAddress = ( AddrBlock *)*addressList;
            while ( addressesFound > 0)
            {
                WriteDebugLine( ( char *)"\pICU!");
                // if we did in fact find this address (should not be needed)

                if ( anAddress->aSocket != 0)
                {

                    // if the client exists and we have the same address, set stillThere flag

                    aClient = GetClientAddressMatch( anAddress);
                    if ( aClient != nil)
                        aClient->stillThere = TRUE;

                    // if this address is not yet listed, add it

                    else
                    {

                        // search for free client slot in list

                        count = 0;
                        aClient = ( netClientEntity *)*gClientEntity;

                        while (( count < kMaxAvailableClient) && ( aClient->status != kDoesNotExist))
                        {
                            count++;
                            aClient++;
                        }

                        // if we have a free client, then add address

                        if ( count < kMaxAvailableClient)
                        {
                            WriteDebugLine((char *)"\pAdd Client");
                            // we don't know the player's name yet, so we can't list it

                            aClient->status = kHasNoName;
                            aClient->entity.address = *anAddress;
                            aClient->stillThere = TRUE;

                            // send a What Is Your Name? Message

                            netData.type = kWhatIsYourName;
                            netData.address = gMyNetEntity.address;
                            CopyPString( (char *)&(netData.name), (char *)&(gMyNetEntity.name));

                            error = NatewerkSendData( aClient->entity.address,
                                                    gMyNetEntity.address.aSocket, (Ptr)&netData,
                                                    (short)sizeof( netSetupDataType));
                            if ( error) WriteDebugLine( (char *)"\pName? Err");
                        } else WriteDebugLine((char *)"\pClient overflow"); // this probably won't ever happen
                    }
                } else WriteDebugLine(( char *)"\pIllegal Address");    // this should never happen
                addressesFound--;
                anAddress++;
            }   // end of walk through address list

            // now remove any clients who aren't still there

            aClient = ( netClientEntity *)*gClientEntity;

            for ( count = 0; count < kMaxAvailableClient; count++)
            {
                if (( aClient->status != kDoesNotExist) && ( !(aClient->stillThere)))
                    aClient->status = kDoesNotExist;
                aClient++;
            }

            RefreshInterfaceItem( kHostAvailableList);
            SetStatusOfAnyInterfaceItem( kHostCancelButton, kDimmed, TRUE);
            if ( GetSelectClient() != nil) SetStatusOfAnyInterfaceItem( kHostInviteButton, kDimmed, TRUE);
            if ( GetInGameListLength() > 0) SetStatusOfAnyInterfaceItem( kHostBeginButton, kDimmed, TRUE);

            while (( !done) && (( TickCount() - lastPoll) < kHostPollTime))
            {
                // process any messages we may have received

                while ( GetTopUsedQueue() != nil)
                {
                    receivedData = ( netSetupDataType *)GetTopUsedQueue();

                    switch( receivedData->type)
                    {

                        // we received a invitation receipt

                        case kIReceivedInvite:

                            // which client sent it?

                            aClient = GetClientAddressMatch( &(receivedData->address));

                            if ( aClient != nil)
                            {
                                if ( aClient->status == kInvitedNoResponse)
                                {
                                    aClient->status = kResponding;
                                    RefreshInterfaceItem( kHostAvailableList);
                                } else WriteDebugLine( (char *)"\p<Rcvd Err1");
                            } else WriteDebugLine( (char *)"\p<Rcvd Err2");

                            break;

                        // we received an acception

                        case kIAcceptInvite:

                            // which client sent it?

                            aClient = GetClientAddressMatch( &(receivedData->address));

                            if ( aClient != nil)
                            {
                                if ( aClient->status == kResponding)
                                {
                                    aClient->status = kInGame;
                                    RefreshInterfaceItem( kHostInGameList);
                                    SetStatusOfAnyInterfaceItem( kHostBeginButton, kDimmed, TRUE);

                                    GetIndString( s, kContentStringID, kContentNetAcceptNum);
                                    DrawStringInInterfaceContent( kHostStatusRect, s);
                                }
                                else WriteDebugLine( (char *)"\p<Accpt Err1");
                            } else WriteDebugLine( (char *)"\p<Accpt Err2");

                            break;

                        // we received an declention

                        case kIDeclineInvite:

                            // which client sent it?

                            aClient = GetClientAddressMatch( &(receivedData->address));

                            if ( aClient != nil)
                            {
                                if ( aClient->status == kResponding)
                                {
                                    aClient->status = kNotInterested;
                                    GetIndString( s, kContentStringID, kContentNetDeclineNum);
                                    DrawStringInInterfaceContent( kHostStatusRect, s);
                                }
                                else WriteDebugLine( (char *)"\p<Dcln Err1");
                            } else WriteDebugLine( (char *)"\p<Dcln Err2");

                            break;

                        // we received a name

                        case kMyNameIs:
                            // which client sent it?

                            aClient = GetClientAddressMatch( &(receivedData->address));

                            if ( aClient != nil)
                            {
                                if ( aClient->status == kHasNoName)
                                {
                                    aClient->status = kAvailable;
                                    CopyPString( (char *)aClient->entity.name, (char *)receivedData->name);
                                    RefreshInterfaceItem( kHostAvailableList);
                                } else WriteDebugLine( (char *)"\p<Mynm Err1");
                            } else WriteDebugLine( (char *)"\p<Mynm Err2");

                            break;

                        default:

                            WriteDebugLine((char *)"Unexpctd Msg");
                            break;
                    }
                    PopTopUsedQueue();
                }

                InterfaceIdle();
                if (Ares_WaitNextEvent (everyEvent, &theEvent, 0, nil))
                {
                    whichItem = -1;
                    switch ( theEvent.what )
                    {
                        case nullEvent:
                            InterfaceIdle();
                            break;
                    case osEvt:
//                      HandleOSEvent( &theEvent);
                        break;
                        case mouseDown:
                            where = theEvent.where;
                            GlobalToLocal( &where);
                            whichItem = InterfaceMouseDown( where);
                            break;
                        case mouseUp:
                            break;
                        case keyDown:
                        case autoKey:
                            whichChar = theEvent.message & charCodeMask;
                            whichItem = InterfaceKeyDown( theEvent.message);
                            break;
                    }

                    switch ( whichItem)
                    {

                        case kHostAvailableList:

                            SetStatusOfAnyInterfaceItem( kHostInviteButton,
                                    ((GetSelectClient() != nil) ? ( kActive):( kDimmed)), TRUE);
                            break;

                        // user hit cancel button

                        case kHostBeginButton:

                            done = TRUE;
                            result = kHost;
                            gNetworkStatus = hostWorking;
                            // walk through client list and send everyone who is included a cancel msg

                            aClient = ( netClientEntity *)*gClientEntity;

                            for ( count = 0; count < kMaxAvailableClient; count++)
                            {
                                if ( aClient->status == kInGame)
                                {
                                    netData.type = kWeAreStarting;
                                    netData.address = gMyNetEntity.address;
                                    CopyPString( (char *)&(netData.name), (char *)&(gMyNetEntity.name));

                                    error = NatewerkSendData( aClient->entity.address,
                                                        gMyNetEntity.address.aSocket, (Ptr)&netData,
                                                        (short)sizeof( netSetupDataType));

                                    if ( error) WriteDebugLine( (char *)"\p>Cncl Err");
                                }
                                aClient++;
                            }

                            result = HostBeginGame();

                            break;

                        case kHostCancelButton:
                            done = TRUE;
                            result = kCancel;

                            // walk through client list and send everyone who is included a cancel msg

                            aClient = ( netClientEntity *)*gClientEntity;

                            for ( count = 0; count < kMaxAvailableClient; count++)
                            {
                                if (( aClient->status == kInGame) || ( aClient->status == kResponding))
                                {
                                    netData.type = kWeAreCanceling;
                                    netData.address = gMyNetEntity.address;
                                    CopyPString( (char *)&(netData.name), (char *)&(gMyNetEntity.name));

                                    error = NatewerkSendData( aClient->entity.address,
                                                        gMyNetEntity.address.aSocket, (Ptr)&netData,
                                                        (short)sizeof( netSetupDataType));

                                    if ( error) WriteDebugLine( (char *)"\p>Cncl Err");
                                }
                                aClient++;
                            }

                            break;

                            // user hit invite button

                            case kHostInviteButton:

                            // which client is selected?

                            aClient = GetSelectClient();

                            if ( aClient != nil)
                            {
                                aClient->status = kInvitedNoResponse;

                                netData.type = kYouAreInvited;
                                netData.address = gMyNetEntity.address;
                                CopyPString( (char *)&(netData.name), (char *)&(gMyNetEntity.name));

                                error = NatewerkSendData( aClient->entity.address,
                                                    gMyNetEntity.address.aSocket, (Ptr)&netData,
                                                    (short)sizeof( netSetupDataType));
                                if ( !error)
                                {
                                    GetIndString( s, kContentStringID, kContentNetWaitNum);
                                    DrawStringInInterfaceContent( kHostStatusRect, s);
                                } else
                                    WriteDebugLine( (char *)"\p>Invt Err");
                            }

                            break;



                    }

                }
            } // while ( ( TickCount() - lastPoll) < kHostPollTime)
        } // while ( !done)
        CloseInterface();
        DisposeHandle( addressList);
    }
    WriteDebugLine((char *)"\pHost Int OK");
    return ( result);
}

netResultType ClientNetworkGameSetup( void)

{
    Point                   where;
    int                     error;
    short                   whichItem, count;
    Boolean                 done = FALSE, responded = FALSE;    // responded is scratch
    EventRecord             theEvent;
    char                    whichChar;
    netResultType           result = kCancel;
    netClientEntity         *hostChoice = nil, *aHost, *proposingHost = nil;
    netSetupDataType        netData, *receivedData = nil;

    BlackenWindow();

    error = OpenInterface( kNetClientID);
    if ( error == kNoError)
    {
        DrawEntireInterface();

        // first clear the host list

        aHost = ( netClientEntity *)*gClientEntity;
        for ( count = 0; count < kMaxAvailableHost; count++)
        {
            aHost->status = kDoesNotExist;
            aHost->entity.address.aNode = aHost->entity.address.aNet = aHost->entity.address.aSocket = 0;
            aHost++;
        }

        // make our presence known by posting Ares entity name (players name n/a)

        if ( error == kNoError)
            error = NatewerkPostName( gMyNetEntity.address.aSocket, kAresNetObjectName,
                                kAresNetTypeName);

        if ( error) WriteDebugLine( (char *)"\pPost Err");
        else WriteDebugLine( (char *)"\pPost OK");

        while ( !done)
        {
            // process the incoming message queue

            while ( GetTopUsedQueue() != nil)
            {
                receivedData = ( netSetupDataType *)GetTopUsedQueue();

                switch ( receivedData->type)
                {

                    // we received a what is your name? messge

                    case kWhatIsYourName:

                        if ( hostChoice == nil)
                        {
                            netData.type = kMyNameIs;
                            netData.address = gMyNetEntity.address;
                            CopyPString( (char *)&(netData.name), (char *)&(gMyNetEntity.name));

                            error = NatewerkSendData( receivedData->address,
                                                gMyNetEntity.address.aSocket, (Ptr)&netData,
                                                (short)sizeof( netSetupDataType));

                            if ( error) WriteDebugLine( (char *)"\pSndNm Err");
                        }

                        break;

                    // we received a cancel message before we accepted/declined

                    case kWeAreCanceling:

                        // step through our host list
                        aHost = ( netClientEntity *)*gClientEntity;
                        for ( count = 0; count < kMaxAvailableHost; count++)
                        {
                            // if we got cancel message from this host

                            if (( aHost->status != kDoesNotExist) &&
                                ( mSameAddress( &(aHost->entity.address), &(receivedData->address))))
                            {

                            // then this host goes away

                                aHost->status = kDoesNotExist;
                                aHost->entity.address.aNode = aHost->entity.address.aNet =
                                                        aHost->entity.address.aSocket = 0;

                                // if this host is the one we currently see as inviting us
                                if ( aHost == proposingHost)
                                {
                                    // then it is no longer the proposing host

                                    proposingHost = nil;
                                    SetStatusOfAnyInterfaceItem( kClientAcceptButton, kDimmed, TRUE);
                                    SetStatusOfAnyInterfaceItem( kClientDeclineButton, kDimmed, TRUE);
                                    RefreshInterfaceItem( kClientHostNameRect);
                                }
                            }
                            aHost++;
                        }

                        break;

                    // we received an invitation

                    case kYouAreInvited:

                        //  1st send receipt of invitation

                        netData.type = kIReceivedInvite;
                        netData.address = gMyNetEntity.address;
                        CopyPString( (char *)&(netData.name), (char *)&(gMyNetEntity.name));

                        error = NatewerkSendData( receivedData->address,
                                            gMyNetEntity.address.aSocket, (Ptr)&netData,
                                            (short)sizeof( netSetupDataType));

                        if ( error) WriteDebugLine( (char *)"\p<Invte Err0");

                        // 2nd make sure we haven't been invited by this guy before

                        aHost = ( netClientEntity *)*gClientEntity;
                        responded = FALSE;

                        for ( count = 0; count < kMaxAvailableHost; count++)
                        {
                            if ( mSameAddress( &(aHost->entity.address), &(receivedData->address)))
                            {
                                if ( aHost->status == kNotInterested)
                                {
                                    netData.type = kIDeclineInvite;
                                    netData.address = gMyNetEntity.address;
                                    CopyPString( (char *)&(netData.name), (char *)&(gMyNetEntity.name));
                                    error = NatewerkSendData( receivedData->address,
                                                        gMyNetEntity.address.aSocket, (Ptr)&netData,
                                                        (short)sizeof( netSetupDataType));
                                    if ( error) WriteDebugLine( (char *)"\p<Invte Err1");
                                }
                                responded = TRUE;
                            }
                            aHost++;
                        }

                        // find a free host

                        if ( !responded)
                        {
                            aHost = ( netClientEntity *)*gClientEntity;
                            count = 0;
                            while (( count < kMaxAvailableHost) && ( aHost->status != kDoesNotExist))
                            {
                                count++;
                                aHost++;
                            }

                            // if we still have room in our host list

                            if ( count != kMaxAvailableHost)
                            {
                                aHost->entity.address = receivedData->address;
                                CopyPString( (char *)&(aHost->entity.name), (char *)&(receivedData->name));
                                aHost->status = kAvailable;
                            } else WriteDebugLine( (char *)"\p<Invte Err2");
                        }

                        break;

                    default:

                        WriteDebugLine( (char *)"\p<Msg Err");

                        break;
                }
                PopTopUsedQueue();
            }

            aHost = ( netClientEntity *)*gClientEntity;
            count = 0;
            while (( count < kMaxAvailableHost) && ( aHost->status != kAvailable))
            {
                count++;
                aHost++;
            }

            if (( count < kMaxAvailableHost) && ( proposingHost != aHost))
            {
                if ( proposingHost == nil)
                {
                    SetStatusOfAnyInterfaceItem( kClientAcceptButton, kDimmed, TRUE);
                    SetStatusOfAnyInterfaceItem( kClientDeclineButton, kDimmed, TRUE);
                }
                proposingHost = aHost;
                DrawStringInInterfaceContent( kClientHostNameRect, proposingHost->entity.name);
            }

            InterfaceIdle();

            if (Ares_WaitNextEvent (everyEvent, &theEvent, 0, nil))
            {
                whichItem = -1;
                switch ( theEvent.what )
                {
                    case nullEvent:
                        InterfaceIdle();
                        break;
                    case osEvt:
//                      HandleOSEvent( &theEvent);
                        break;
                    case mouseDown:
                        where = theEvent.where;
                        GlobalToLocal( &where);
                        whichItem = InterfaceMouseDown( where);
                        break;
                    case mouseUp:
                        break;
                    case keyDown:
                    case autoKey:
                        whichChar = theEvent.message & charCodeMask;
                        whichItem = InterfaceKeyDown( theEvent.message);
                        break;
                }
                switch ( whichItem)
                {
                    case kClientCancelButton:

                        aHost = ( netClientEntity *)*gClientEntity;
                        for ( count = 0; count < kMaxAvailableHost; count++)
                        {
                            if ( aHost->status != kDoesNotExist)
                            {
                                netData.type = kIDeclineInvite;
                                netData.address = gMyNetEntity.address;
                                CopyPString( (char *)&(netData.name), (char *)&(gMyNetEntity.name));
                                error = NatewerkSendData( receivedData->address,
                                                    gMyNetEntity.address.aSocket, (Ptr)&netData,
                                                    (short)sizeof( netSetupDataType));
                                if ( error) WriteDebugLine( (char *)"\p<Invte Err1");
                            }
                            aHost++;
                        }
                        done = TRUE;
                        result = kCancel;
                        break;

                    case kClientAcceptButton:

                        if ( proposingHost != nil)
                        {
                            aHost = ( netClientEntity *)*gClientEntity;
                            for ( count = 0; count < kMaxAvailableHost; count++)
                            {
                                if (( aHost != proposingHost) && (aHost->status != kDoesNotExist))
                                {
                                    netData.type = kIDeclineInvite;
                                    netData.address = gMyNetEntity.address;
                                    CopyPString( (char *)&(netData.name), (char *)&(gMyNetEntity.name));
                                    error = NatewerkSendData( receivedData->address,
                                                        gMyNetEntity.address.aSocket, (Ptr)&netData,
                                                        (short)sizeof( netSetupDataType));
                                    if ( error) WriteDebugLine( (char *)"\p<Invte Err1");
                                }
                                aHost++;
                            }

                            netData.type = kIAcceptInvite;
                            netData.address = gMyNetEntity.address;
                            CopyPString( (char *)&(netData.name), (char *)&(gMyNetEntity.name));

                            error = NatewerkSendData( receivedData->address,
                                                gMyNetEntity.address.aSocket, (Ptr)&netData,
                                                (short)sizeof( netSetupDataType));

                            if ( error) WriteDebugLine( (char *)"\p>Acpt Err");
                            done = TRUE;

                            CloseInterface();
                            result = ClientNetworkWaitForHost();
                        }
                        break;

                    case kClientDeclineButton:
                        if ( proposingHost != nil)
                        {
                            netData.type = kIDeclineInvite;
                            netData.address = gMyNetEntity.address;
                            CopyPString( (char *)&(netData.name), (char *)&(gMyNetEntity.name));

                            error = NatewerkSendData( receivedData->address,
                                                gMyNetEntity.address.aSocket, (Ptr)&netData,
                                                (short)sizeof( netSetupDataType));

                            if ( error) WriteDebugLine( (char *)"\p>Dcln Err");
                            proposingHost->status = kNotInterested;
                            proposingHost = nil;
                            SetStatusOfAnyInterfaceItem( kClientAcceptButton, kDimmed, TRUE);
                            SetStatusOfAnyInterfaceItem( kClientDeclineButton, kDimmed, TRUE);
                            RefreshInterfaceItem( kClientHostNameRect);
                        }
                        break;

                }

            }
        }
        CloseInterface();
    }
    NatewerkRemoveName( kAresNetObjectName, kAresNetTypeName);
    return ( result);
}

netResultType ClientNetworkWaitForHost( void)

{
    Point                   where;
    int                     error;
    short                   whichItem, count;
    Boolean                 done = FALSE, responded = FALSE;    // responded is scratch
    EventRecord             theEvent;
    char                    whichChar;
    netResultType           result = kCancel;
    netClientEntity         *hostChoice = nil, *aHost, *proposingHost = nil;
    netSetupDataType        netData, *receivedData = nil;

    BlackenWindow();

    error = OpenInterface( kClientWaitID);
    if ( error == kNoError)
    {
        DrawEntireInterface();

        while ( !done)
        {
            // process the incoming message queue

            while ( GetTopUsedQueue() != nil)
            {
                receivedData = ( netSetupDataType *)GetTopUsedQueue();

                switch ( receivedData->type)
                {
                    case kWeAreCanceling:

                        done = TRUE;
                        result = kCancel;
                        break;

                    case kWeAreStarting:

                        gNetworkStatus = clientWorking;
                        done = TRUE;

                        PopTopUsedQueue();
                        result = ClientBeginGame();
                        break;

                    default:

                        WriteDebugLine( (char *)"\p<Msg Err");

                        break;
                }
                PopTopUsedQueue();
            }

            InterfaceIdle();

            if (Ares_WaitNextEvent (everyEvent, &theEvent, 0, nil))
            {
                whichItem = -1;
                switch ( theEvent.what )
                {
                    case nullEvent:
                        InterfaceIdle();
                        break;
                    case osEvt:
//                      HandleOSEvent( &theEvent);
                        break;
                    case mouseDown:
                        where = theEvent.where;
                        GlobalToLocal( &where);
                        whichItem = InterfaceMouseDown( where);
                        break;
                    case mouseUp:
                        break;
                    case keyDown:
                    case autoKey:
                        whichChar = theEvent.message & charCodeMask;
                        whichItem = InterfaceKeyDown( theEvent.message);
                        break;
                }
                switch ( whichItem)
                {

                }

            }
        }
        CloseInterface();
    }
    return ( result);
}
*/
/*
netResultType HostBeginGame( void)

{
    AresNetworkHostBegin();
    return ( kHost);
}
*/
/*
netResultType ClientBeginGame( void)

{
    AresNetworkClientBegin();
    return ( kClient);
}
*/

void BlackenWindow( void)

{
    Rect    tRect;

    MacSetRect( &tRect, 0, 0, WORLD_WIDTH, WORLD_HEIGHT);
    DrawInSaveWorld();
    DefaultColors();
    PaintRect( &tRect);
    DrawInOffWorld();
    DefaultColors();
    PaintRect( &tRect);
    DrawInRealWorld();
    DefaultColors();
    PaintRect( &tRect);
}

void BlackenOffscreen( void)

{
    Rect    tRect;

    MacSetRect( &tRect, 0, 0, WORLD_WIDTH, WORLD_HEIGHT);
    DrawInSaveWorld();
    DefaultColors();
    PaintRect( &tRect);
    DrawInOffWorld();
    DefaultColors();
    PaintRect( &tRect);
    DrawInRealWorld();
    DefaultColors();
}

//
//      Here's the set up for multiple textEdit items:
//
//      InterfaceTextEditItemInit( 9);
//      InterfaceTextEditSetText( 9, ( anyCharType *)"\pThis is a larger test string.");
//      InterfaceTextEditDeactivate( 9);
//      InterfaceTextEditItemInit( 8);
//      InterfaceTextEditSetText( 8, ( anyCharType *)"\pThis is a test string.");
//      InterfaceTextEditSelectAll( 8);
//      InterfaceTextEditActivate( 8);
//

/*
short GetClientListLength( void)
{
    netClientEntity     *aClient;
    short               i, result = 0;

    aClient = ( netClientEntity *)*gClientEntity;
    for ( i = 0; i < kMaxAvailableClient; i++)
    {
        if ( aClient->status == kAvailable) result++;
        aClient++;
    }
    WriteDebugLong( (long)result);
    return ( result);
}
*/
/*
void GetClientListName( short which, anyCharType *s)

{
    netClientEntity     *aClient;
    short               availCount = 0, count = 0;

    aClient = ( netClientEntity *)*gClientEntity + (long)which;
    while (( count < kMaxAvailableClient) && ( availCount < which))
    {
        if ( aClient->status == kAvailable) availCount++;
        aClient++;
        count++;
    }
    if ( availCount == which)
        CopyPString( (char *)s, (char *)aClient->entity.name);
    else *s = 0;
}
*/
/*
Boolean IsThisClientHilited( short which, Boolean set)

{
    netClientEntity     *aClient;
    short               i, listNum = 0;

    if ( !set)
    {
        aClient = ( netClientEntity *)*gClientEntity + (long)which;
        return ( aClient->selected);
    } else
    {
        aClient = ( netClientEntity *)*gClientEntity;
        for ( i = 0; i < kMaxAvailableClient; i++)
        {
            aClient->selected = FALSE;
            if ( aClient->status == kAvailable)
            {
                if ( listNum == which)
                    aClient->selected = TRUE;
                RefreshInterfaceListEntry( kHostAvailableList, listNum);
                listNum++;
            }
            aClient++;
        }
        return ( TRUE);
    }
}
*/
/*
netClientEntity *GetSelectClient( void)

{
    netClientEntity     *aClient = nil;
    short               i;

    aClient = ( netClientEntity *)*gClientEntity;
    for ( i = 0; i < kMaxAvailableClient; i++)
    {
        if (( aClient->status == kAvailable) && ( aClient->selected))
            return( aClient);
        aClient++;
    }
    return( nil);
}
*/
/*
short GetInGameListLength( void)
{
    netClientEntity     *aClient;
    short               i, result = 0;

    aClient = ( netClientEntity *)*gClientEntity;
    for ( i = 0; i < kMaxAvailableClient; i++)
    {
        if ( aClient->status == kInGame) result++;
        aClient++;
    }
    WriteDebugLong( (long)result);
    return ( result);
}
*/

/*
void GetInGameListName( short which, anyCharType *s)

{
    netClientEntity     *aClient;
    short               availCount = 0, count = 0;

    aClient = ( netClientEntity *)*gClientEntity + (long)which;
    while (( count < kMaxAvailableClient) && ( availCount < which))
    {
        if ( aClient->status == kInGame) availCount++;
        aClient++;
        count++;
    }
    if ( availCount == which)
        CopyPString( (char *)s, (char *)aClient->entity.name);
    else *s = 0;
}
*/

Boolean IsThisInGameHilited( short which, Boolean set)

{
#pragma unused( which, set)
    return ( FALSE);
}

long DoSelectLevelInterface( long startChapter)

{
    Point                   where;
    int                     error, x;
    short                   whichItem;
    Boolean                 done = FALSE, enteringCheat = false;
    EventRecord             theEvent;
    char                    whichChar;
    interfaceItemType       *anItem;
    Handle                  textData = nil;
    Rect                        totalRect;
    PixMapHandle            saveMap, offMap;
    CWindowPtr              whichWindow;
    Str255                  chapterName, cheatString;
    long                    thisLevel = GetScenarioNumberFromChapterNumber(
                                        startChapter),
                            thisChapter = startChapter,
                            lastChapter = startChapter;

    cheatString[0] = 0;

    if ( ThisChapterIsNetworkable( thisChapter)) thisChapter = 1;
    if ( ThisChapterIsNetworkable( thisChapter))
    {
        gAresGlobal->externalFileSpec.name[0] = 0;
        EF_OpenExternalFile();
        ShowErrorAny( eContinueOnlyErr, kErrorStrID,
            nil, nil, nil, nil, 80, -1, -1, -1, __FILE__, 0);
        startChapter = GetStartingLevelPreference();
        thisLevel = GetScenarioNumberFromChapterNumber( startChapter);
    }

    BlackenWindow();
    FlushEvents(everyEvent, 0);
    error = OpenInterface( kSelectLevelID);
    saveMap = GetGWorldPixMap( gSaveWorld);
    offMap = GetGWorldPixMap( gOffWorld);
    MacSetRect( &totalRect, 0, 0, WORLD_WIDTH, WORLD_HEIGHT);
    if ( error == kNoError)
    {
//      DrawEntireInterface();
        HideCursor();
        if ( GetScenarioPlayerNum( thisLevel) <= 0) SetStatusOfAnyInterfaceItem( kSelectLevelOKButton, kDimmed, false);
        if ( thisChapter >= GetStartingLevelPreference())
        {
            SetStatusOfAnyInterfaceItem( kSelectLevelNextButton, kDimmed, false);
        }
        if ( thisChapter <= 1)
        {
            SetStatusOfAnyInterfaceItem( kSelectLevelPreviousButton, kDimmed, false);
        }
        DrawInterfaceOneAtATime();

        anItem = GetAnyInterfaceItemPtr( kSelectLevelNameBox);

        GetScenarioName( thisLevel, chapterName);
        DrawLevelNameInBox( chapterName, kTitleFontNum, -1, kSelectLevelNameBox);

// it is assumed that we're "recovering" from a fade-out
        AutoFadeFrom( 60, FALSE);
        MacShowCursor();
        while ( !done)
        {
            InterfaceIdle();

            Ares_WaitNextEvent (everyEvent, &theEvent, 3, nil);
            {
                whichItem = -1;
                switch ( theEvent.what )
                {
                    case updateEvt:
                        whichWindow = ( CWindowPtr)theEvent.message;

                        if ( whichWindow == gTheWindow)
                        {
                            BeginUpdate( (WindowPtr)whichWindow);
                                MacSetPort( (WindowPtr)gTheWindow);
                                CopyOffWorldToRealWorld((WindowPtr)gTheWindow, &(gTheWindow->portRect));
                            EndUpdate( (WindowPtr)whichWindow);
                            break;
                            EndUpdate( (WindowPtr)whichWindow);
                        } else if ( whichWindow == gAresGlobal->gBackWindow)
                        {
                            BeginUpdate( (WindowPtr)whichWindow);
                                MacSetPort( (WindowPtr)gAresGlobal->gBackWindow);
                                MacFillRect(  &(gAresGlobal->gBackWindow->portRect), (Pattern *)&qd.black);
                            EndUpdate( (WindowPtr)whichWindow);
                        } else
                        {
                            BeginUpdate( (WindowPtr)whichWindow);
                            EndUpdate( (WindowPtr)whichWindow);
                        }
                        MacSetPort( (WindowPtr)gTheWindow);

                        break;

                    case nullEvent:
                        InterfaceIdle();
                        if ( gAresGlobal->gOptions & kOptionInBackground)
                        {
                        } else if (AutoShowHideMenubar( theEvent.where, theDevice))
                        {
                        }
                        if ( gAresGlobal->returnToMain)
                        {
                            whichItem = kSelectLevelCancelButton;
                        }

                        break;
                    case osEvt:
//                      HandleOSEvent( &theEvent);
                        break;

                    case mouseDown:
                        where = theEvent.where;
                        GlobalToLocal( &where);
                        HandleMouseDown( &theEvent);
                        whichItem = InterfaceMouseDown( where);
                        break;
                    case mouseUp:
                        break;
                    case keyDown:
                    case autoKey:
                        whichChar = theEvent.message & charCodeMask;
                        whichItem = InterfaceKeyDown( theEvent.message);
/*                      if ( whichChar == '|')
                        {
                            x = gAresGlobal->levelNum;
                            if ( x > GetScenarioNumber())
                                x = GetScenarioNumber();
                            SaveStartingLevelPreferences( x);
                         }
*/
                        if ( whichChar == '*')
                        {
                            enteringCheat = true;
                            cheatString[0] = 0;
                            PlayVolumeSound( kCloakOn, kMaxSoundVolume, kLongPersistence, kMustPlaySound);
                        } else if ( enteringCheat)
                        {
                            cheatString[0] += 1;
                            cheatString[cheatString[0]] = whichChar;
                            if ( cheatString[0] >= 2)
                            {
                                enteringCheat = false;
                                x = StringToLong( cheatString);
                                if (( x < GetScenarioNumber()) && ( x >= 1))
                                {
                                    if ( ThisChapterIsNetworkable( x))
                                    {
                                        PlayVolumeSound( kWarningTone, kMaxSoundVolume, kLongPersistence, kMustPlaySound);
                                    } else
                                    {
                                        PlayVolumeSound( kCloakOn, kMaxSoundVolume, kLongPersistence, kMustPlaySound);
                                        SaveStartingLevelPreferences( x);
                                        thisChapter = x;
                                    }
                                } else
                                {
                                    PlayVolumeSound( kWarningTone, kMaxSoundVolume, kLongPersistence, kMustPlaySound);
                                }
                            }
                        }
                        break;
                }
                switch ( whichItem)
                {
                    case kSelectLevelNextButton:
                        if ( thisChapter < GetStartingLevelPreference())
                        {
                            thisChapter++;
                        }
                        break;

                    case kSelectLevelPreviousButton:
                        if ( thisChapter > 1)
                        {
                            thisChapter--;
/*                          thisLevel = GetScenarioNumberFromChapterNumber( thisChapter);
                            GetScenarioName( thisLevel, chapterName);
                            DrawLevelNameInBox( chapterName, kTitleFontNum, -1, kSelectLevelNameBox);

                            if ( GetScenarioPlayerNum( thisLevel) <= 0)
                                SetStatusOfAnyInterfaceItem( kSelectLevelOKButton, kDimmed, true);
                            else
                                SetStatusOfAnyInterfaceItem( kSelectLevelOKButton, kActive, true);
*/                      }
                        break;

                    case kSelectLevelOKButton:
                        done = TRUE;
                        break;

                    case kSelectLevelCancelButton:
                        thisLevel = -1;
                        done = true;
                        break;

                }
                if (( thisChapter != lastChapter) && ( thisLevel >= 0))
                {
                    thisLevel = GetScenarioNumberFromChapterNumber( thisChapter);
                    GetScenarioName( thisLevel, chapterName);
                    DrawLevelNameInBox( chapterName, kTitleFontNum, -1, kSelectLevelNameBox);

                    if ( thisChapter >= GetStartingLevelPreference())
                    {
                        SetStatusOfAnyInterfaceItem( kSelectLevelNextButton, kDimmed, true);
                    } else
                    {
                        SetStatusOfAnyInterfaceItem( kSelectLevelNextButton, kActive, true);
                    }

                    if ( thisChapter <= 1)
                    {
                        SetStatusOfAnyInterfaceItem( kSelectLevelPreviousButton, kDimmed, true);
                    } else
                    {
                        SetStatusOfAnyInterfaceItem( kSelectLevelPreviousButton, kActive, true);
                    }


                    if (( GetScenarioPlayerNum( thisLevel) <= 0) || ( ThisChapterIsNetworkable( thisChapter)))
                        SetStatusOfAnyInterfaceItem( kSelectLevelOKButton, kDimmed, true);
                    else
                        SetStatusOfAnyInterfaceItem( kSelectLevelOKButton, kActive, true);
                    lastChapter = thisChapter;
                }
            }

        }
        CloseInterface();
    }
    return ( thisLevel);
}

void DrawLevelNameInBox( StringPtr name, long fontNum, short descriptionTextID,
    long itemNum)
{
    longRect                clipRect;
    Rect                    tRect;
    unsigned char           *strPtr;
    PixMapHandle            offMap;
    retroTextSpecType       retroTextSpec;
    transColorType          *transColor;
    interfaceItemType       *anItem;
    long                    height, descriptionLength = 0;
    Handle                  textData = nil;

    offMap = GetGWorldPixMap( gOffWorld);
    anItem = GetAnyInterfaceItemPtr( itemNum);
    strPtr = name + 1;

    if ( descriptionTextID > 0)
    {
        textData = GetResource( 'TEXT', descriptionTextID);
        if ( textData != nil)
        {
            DetachResource( textData);
            MoveHHi( textData);
            HLock( textData);
            descriptionLength = GetHandleSize( textData);
        }
    }

    descriptionLength += (long)name[0];
    retroTextSpec.textLength = descriptionLength;
    retroTextSpec.text = NewHandle( descriptionLength);
    if ( retroTextSpec.text == nil)
    {
        if ( textData != nil) DisposeHandle( textData);
        return;
    }

    MoveHHi( retroTextSpec.text);
    HLock( retroTextSpec.text);
    BlockMove( name + 1, *retroTextSpec.text, (long)name[0]);
    if ( textData != nil)
    {
        BlockMove( *textData, *retroTextSpec.text + (long)name[0], GetHandleSize( textData));
        DisposeHandle( textData);
    }

    retroTextSpec.thisPosition = retroTextSpec.linePosition = retroTextSpec.lineCount = 0;
    retroTextSpec.tabSize =220;
    mGetTranslateColorShade( AQUA, VERY_LIGHT, retroTextSpec.color, transColor)
    mGetTranslateColorShade( AQUA, DARKEST, retroTextSpec.backColor, transColor)
    retroTextSpec.backColor = 0xff;
    retroTextSpec.originalColor = retroTextSpec.nextColor = retroTextSpec.color;
    retroTextSpec.originalBackColor = retroTextSpec.nextBackColor = retroTextSpec.backColor;
    retroTextSpec.topBuffer = 2;
    retroTextSpec.bottomBuffer = 0;

    mSetDirectFont( fontNum)
    height = DetermineDirectTextHeightInWidth( &retroTextSpec, anItem->bounds.right - anItem->bounds.left);

    retroTextSpec.xpos = anItem->bounds.left;
    retroTextSpec.ypos = anItem->bounds.left + mDirectFontAscent;

//  clipRect.left = 0;
//  clipRect.right = clipRect.left + WORLD_WIDTH;
//  clipRect.top = 0;
//  clipRect.bottom = clipRect.top + WORLD_HEIGHT;
    clipRect = anItem->bounds;
    LongRectToRect( &anItem->bounds, &tRect);
    DrawInOffWorld();
    DefaultColors();
    PaintRect( &tRect);
    DrawInRealWorld();
    DrawDirectTextInRect( &retroTextSpec, &anItem->bounds, &clipRect, *offMap, 0,0);
    CopyOffWorldToRealWorld((WindowPtr)gTheWindow, &tRect);
    DisposeHandle( retroTextSpec.text);
}

Boolean DoMissionInterface( long whichScenario)

{
    Point                   where, dataPoint;
    int                     error, x, y;
    short                   whichItem, i;
    Boolean                 done = FALSE, cancel = FALSE;
    EventRecord             theEvent;
    char                    whichChar;
    interfaceItemType       dataItem;
    Handle                  textData = nil;
    long                    length, scale, mustFit = 0, whichBriefPoint = 0, nextStartTime = TickCount(),
                            thisMissionWaitTime = 0;
    Rect                    tRect, mapRect, totalRect;
    coordPointType          corner;
    PixMapHandle            saveMap, offMap;
    CWindowPtr              whichWindow;
    inlinePictType          inlinePict[kMaxInlinePictNum];

    FlushEvents(everyEvent, 0);
    if ( GetBriefPointNumber( whichScenario) < 1) return true;

    error = OpenInterface( kMissionBriefingScreenID);
//  HHCheckAllHandles();
    saveMap = GetGWorldPixMap( gSaveWorld);
    offMap = GetGWorldPixMap( gOffWorld);
    MacSetRect( &totalRect, 0, 0, WORLD_WIDTH, WORLD_HEIGHT);


    if ( error == kNoError)
    {
//      DrawEntireInterface();
        HideCursor();
        SetStatusOfAnyInterfaceItem( kMissionPreviousButton, kDimmed, FALSE);
        DrawInterfaceOneAtATime();
        CopyOffWorldToSaveWorld( &totalRect);
// it is assumed that we're "recovering" from a fade-out
        AutoFadeFrom( 60, FALSE);

        GetAnyInterfaceItemContentBounds( GetAnyInterfaceItemPtr( kMissionMapRect), &mapRect);

        DrawInSaveWorld();

        for ( length = 0; length < 500; length++)
        {
            SetTranslateColorShadeFore( GRAY, Randomize( kVisibleShadeNum) + DARKEST);
            x = Randomize( mapRect.right - mapRect.left) + mapRect.left;
            y = Randomize( mapRect.bottom - mapRect.top) + mapRect.top;
            MoveTo( x, y);
            MacLineTo( x, y);
        }
        NormalizeColors();
        DrawInRealWorld();

        CopySaveWorldToOffWorld( &totalRect);
        CopyOffWorldToRealWorld((WindowPtr)gTheWindow, &totalRect);

        GetScenarioFullScaleAndCorner( whichScenario, 0, &corner, &scale, &mapRect);
        DrawArbitrarySectorLines( &corner, scale, 16, &mapRect, saveMap,  0, 0);
//      HHCheckAllHandles();
        Briefing_Objects_Render( whichScenario, saveMap, 32,
            &mapRect, 0, 0, &corner,
            scale);
//      HHCheckAllHandles();

//      HHCheckAllHandles();
//      GetBriefPointBounds( whichBriefPoint, &corner, scale, 0, &tRect);

        tRect.left = ( mapRect.right - mapRect.left) / 2 - ( kMissionDataWidth / 2) + mapRect.left;
        tRect.right = tRect.left + kMissionDataWidth;
        tRect.top = ( mapRect.bottom - mapRect.top) / 2 - ( kMissionDataWidth / 2) + mapRect.top;
        tRect.bottom = tRect.top + kMissionDataWidth;

        RectToLongRect( &tRect, &(dataItem.bounds));
        dataItem.color = GOLD;
        dataItem.kind = kLabeledRect;
        dataItem.style = kLarge;
        dataItem.item.labeledRect.label.stringID = 4000;
        dataItem.item.labeledRect.label.stringNumber = 1;
//      HHCheckAllHandles();
        MacShowCursor();
//      HHCheckAllHandles();
        thisMissionWaitTime = UpdateMissionBriefPoint( &dataItem, whichBriefPoint, whichScenario, &corner, scale, mustFit,
                                &mapRect, &tRect, inlinePict) * 2;
        thisMissionWaitTime += thisMissionWaitTime / 2;


/////////////////////////////////////
// Display free memory
/*      DrawInRealWorld();
        NumToString( CompactMem( maxSize), s);
        MoveTo( 10, 50);
        SetTranslateColorFore( WHITE);
        DrawString( s);
        SetTranslateColorFore( BLACK);
*/
//
/////////////////////////////////////
        nextStartTime = TickCount();

        while ( !done)
        {
            InterfaceIdle();
//          if (Ares_WaitNextEvent (everyEvent, &theEvent, 0, nil))
            Ares_WaitNextEvent (everyEvent, &theEvent, 3, nil);
            {
                whichItem = -1;
                switch ( theEvent.what )
                {
                    case updateEvt:
                        whichWindow = ( CWindowPtr)theEvent.message;

                        if ( whichWindow == gTheWindow)
                        {
                            BeginUpdate( (WindowPtr)whichWindow);
                                MacSetPort( (WindowPtr)gTheWindow);
                                CopyOffWorldToRealWorld((WindowPtr)gTheWindow, &(gTheWindow->portRect));
/////////////////////////////////////
// Display free memory
/*      DrawInRealWorld();
        NumToString( CompactMem( maxSize), s);
        MoveTo( 10, 50);
        SetTranslateColorFore( WHITE);
        DrawString( s);
        SetTranslateColorFore( BLACK);
*/
//
/////////////////////////////////////
                            EndUpdate( (WindowPtr)whichWindow);
                            break;
                            EndUpdate( (WindowPtr)whichWindow);
                        } else if ( whichWindow == gAresGlobal->gBackWindow)
                        {
                            BeginUpdate( (WindowPtr)whichWindow);
                                MacSetPort( (WindowPtr)gAresGlobal->gBackWindow);
                                MacFillRect(  &(gAresGlobal->gBackWindow->portRect), (Pattern *)&qd.black);
                            EndUpdate( (WindowPtr)whichWindow);
                        } else
                        {
                            BeginUpdate( (WindowPtr)whichWindow);
                            EndUpdate( (WindowPtr)whichWindow);
                        }
                        MacSetPort( (WindowPtr)gTheWindow);

                        break;

                    case nullEvent:
                        InterfaceIdle();
                        if ( gAresGlobal->gOptions & kOptionInBackground)
                        {
                        } else if (AutoShowHideMenubar( theEvent.where, theDevice))
                        {
                        }
                        if (( gAresGlobal->gOptions & (kOptionAutoPlay | kOptionReplay)) && (!( gAresGlobal->gOptions & kOptionInBackground))
                            && ( (TickCount() - nextStartTime) > thisMissionWaitTime))
                        {
                            if ( whichBriefPoint < ( GetBriefPointNumber( whichScenario) - 1))
                            {
                                whichItem = kMissionNextButton;
                            } else
                            {
                                whichItem = kMissionDoneButton;
                            }
                            nextStartTime = TickCount();
                        }

                        if ( gAresGlobal->returnToMain)
                        {
                            done = true;
                            cancel = true;
                        }
                        break;
                    case osEvt:
//                      HandleOSEvent( &theEvent);
                        break;

                    case mouseDown:
                        where = theEvent.where;
                        GlobalToLocal( &where);
                        HandleMouseDown( &theEvent);
                        for ( i = 0; i < kMaxInlinePictNum; i++)
                        {
                            if (( inlinePict[i].id >= 0) &&
                                ( MacPtInRect( where, &(inlinePict[i].bounds))))
                            {
                                ShowObjectData( where, inlinePict[i].id, &mapRect);
                            }
                        }
                        whichItem = InterfaceMouseDown( where);
                        break;
                    case mouseUp:
                        break;
                    case keyDown:
                    case autoKey:
                        if ( AnyCancelKeys())
                        {
                            done = true;
                            cancel = true;
                        }

                        whichChar = theEvent.message & charCodeMask;
                        if (( whichChar >= '1') && ( whichChar <= '9'))
                        {
                            i = whichChar - '1';
                            if ( inlinePict[i].id >= 0)
                            {
                                dataPoint.h = inlinePict[i].bounds.left +
                                        (( inlinePict[i].bounds.right -
                                        inlinePict[i].bounds.left) / 2);
                                dataPoint.v = inlinePict[i].bounds.top +
                                        (( inlinePict[i].bounds.bottom -
                                        inlinePict[i].bounds.top) / 2);
                                ShowObjectData( dataPoint, inlinePict[i].id, &mapRect);
                            }
                        }
                        whichItem = InterfaceKeyDown( theEvent.message);

                        break;
                }
                switch ( whichItem)
                {
                    case kMissionNextButton:
                        if ( whichBriefPoint <
                            (( GetBriefPointNumber( whichScenario) + kMissionBriefPointOffset) - 1))
                        {
                            whichBriefPoint++;
                            thisMissionWaitTime = UpdateMissionBriefPoint( &dataItem, whichBriefPoint, whichScenario,
                                    &corner, scale, mustFit, &mapRect, &tRect, inlinePict) * 2;
                            thisMissionWaitTime += thisMissionWaitTime / 2;
                            if ( whichBriefPoint >=
                                (( GetBriefPointNumber( whichScenario) + kMissionBriefPointOffset) - 1))
                            {
                                SetStatusOfAnyInterfaceItem( kMissionNextButton, kDimmed, true);
                                DrawAnyInterfaceItemSaveToOffToOn( GetAnyInterfaceItemPtr( kMissionNextButton));
                            }
                            if ( whichBriefPoint == 1)
                            {
                                SetStatusOfAnyInterfaceItem( kMissionPreviousButton, kActive, true);
                                DrawAnyInterfaceItemSaveToOffToOn( GetAnyInterfaceItemPtr( kMissionPreviousButton));
                            }
                        }
                        break;

                    case kMissionPreviousButton:
                        if ( whichBriefPoint > 0)
                        {
                            whichBriefPoint--;
                            thisMissionWaitTime = UpdateMissionBriefPoint( &dataItem, whichBriefPoint,
                                    whichScenario, &corner, scale, mustFit, &mapRect, &tRect,
                                    inlinePict) * 2;
                            thisMissionWaitTime += thisMissionWaitTime / 2;
                            if ( whichBriefPoint == 0)
                            {
                                SetStatusOfAnyInterfaceItem( kMissionPreviousButton, kDimmed, true);
                                DrawAnyInterfaceItemSaveToOffToOn( GetAnyInterfaceItemPtr( kMissionPreviousButton));
                            }
                            if ( whichBriefPoint ==
                                (( GetBriefPointNumber( whichScenario) + kMissionBriefPointOffset) - 2))
                            {
                                SetStatusOfAnyInterfaceItem( kMissionNextButton, kActive, true);
                                DrawAnyInterfaceItemSaveToOffToOn( GetAnyInterfaceItemPtr( kMissionNextButton));
                            }
                        }
                        break;

                    case kMissionDoneButton:
                        done = TRUE;
                        break;

                }

            }
        }
        CloseInterface();
    }
    if ( cancel) return( false);
    else return( true);
}

long UpdateMissionBriefPoint( interfaceItemType *dataItem, long whichBriefPoint, long whichScenario,
                    coordPointType *corner, long scale, long mustFit, Rect *bounds,
                    Rect *usedRect, inlinePictType *inlinePict)

{
    Rect            oldRect, newRect, hiliteBounds;
    Handle          textData;
    long            length, headerID, headerNumber, contentID, textlength = 0,
                    i;
    short           textHeight = 0;
    Boolean         bailOut = false;
    PixMapHandle    offMap = GetGWorldPixMap( gOffWorld);
    PicHandle       thePict = nil;
    Point           starPoint;
    transColorType  *transColor;
    unsigned char   color;
    longRect        longClipRect, starRect;
    inlinePictType  *thisInlinePict;

#pragma unused( mustFit)

    HideCursor();
    GetAnyInterfaceItemGraphicBounds( dataItem, &oldRect);
    CopySaveWorldToOffWorld( usedRect);

    DrawInOffWorld();
//  HHCheckAllHandles();

    if ( whichBriefPoint >=kMissionBriefPointOffset)
    {
        whichBriefPoint -= kMissionBriefPointOffset;

        BriefPoint_Data_Get( whichBriefPoint, whichScenario, &headerID, &headerNumber, &contentID,
                                 &hiliteBounds, corner, scale, 16, 32, bounds);

        textData = HHGetResource( 'TEXT', contentID);
        if ( textData != nil)
        {
            HLockHi( textData);

            textlength = length = GetHandleSize( textData);
            textHeight = GetInterfaceTextHeightFromWidth( (anyCharType *)*textData, length,
                            dataItem->style, kMissionDataWidth);
        }
        if ( hiliteBounds.left == hiliteBounds.right)
        {
            dataItem->bounds.left = ( bounds->right - bounds->left) / 2 - ( kMissionDataWidth / 2) + bounds->left;
            dataItem->bounds.right = dataItem->bounds.left + kMissionDataWidth;
            dataItem->bounds.top = ( bounds->bottom - bounds->top) / 2 - ( textHeight / 2) + bounds->top;
            dataItem->bounds.bottom = dataItem->bounds.top + textHeight;
        } else
        {
            if ( ( hiliteBounds.left + ( hiliteBounds.right - hiliteBounds.left) / 2) >
                ( bounds->left + (bounds->right - bounds->left) / 2))
            {
                dataItem->bounds.right = hiliteBounds.left - kMissionDataHBuffer;
                dataItem->bounds.left = dataItem->bounds.right - kMissionDataWidth;
            } else
            {
                dataItem->bounds.left = hiliteBounds.right + kMissionDataHBuffer;
                dataItem->bounds.right = dataItem->bounds.left + kMissionDataWidth;
            }

            dataItem->bounds.top = hiliteBounds.top + ( hiliteBounds.bottom - hiliteBounds.top) / 2 -
                                    textHeight / 2;
            dataItem->bounds.bottom = dataItem->bounds.top + textHeight;
            if ( dataItem->bounds.top < ( bounds->top + kMissionDataTopBuffer))
            {
                dataItem->bounds.top = bounds->top + kMissionDataTopBuffer;
                dataItem->bounds.bottom = dataItem->bounds.top + textHeight;
            }
            if ( dataItem->bounds.bottom > ( bounds->bottom - kMissionDataBottomBuffer))
            {
                dataItem->bounds.bottom = bounds->bottom - kMissionDataBottomBuffer;
                dataItem->bounds.top = dataItem->bounds.bottom - textHeight;
            }

            if ( dataItem->bounds.left < ( bounds->left + kMissionDataVBuffer))
            {
                dataItem->bounds.left = bounds->left + kMissionDataVBuffer;
                dataItem->bounds.right = dataItem->bounds.left + kMissionDataWidth;
            }
            if ( dataItem->bounds.right > ( bounds->right - kMissionDataVBuffer))
            {
                dataItem->bounds.right = bounds->right - kMissionDataVBuffer;
                dataItem->bounds.left = dataItem->bounds.right - kMissionDataWidth;
            }

            SetTranslateColorShadeFore( kMissionDataHiliteColor, VERY_LIGHT);
            hiliteBounds.right++;
            hiliteBounds.bottom++;
            MacFrameRect( &hiliteBounds);
            SetTranslateColorShadeFore( kMissionDataHiliteColor, MEDIUM);
            GetAnyInterfaceItemGraphicBounds( dataItem, &newRect);
            if ( dataItem->bounds.right < hiliteBounds.left)
            {
                MoveTo( hiliteBounds.left, hiliteBounds.top);
                MacLineTo( newRect.right + kMissionLineHJog, hiliteBounds.top);
                MacLineTo( newRect.right + kMissionLineHJog, newRect.top);
                MacLineTo( newRect.right + 1, newRect.top);
                MoveTo( hiliteBounds.left, hiliteBounds.bottom - 1);
                MacLineTo( newRect.right + kMissionLineHJog, hiliteBounds.bottom - 1);
                MacLineTo( newRect.right + kMissionLineHJog, newRect.bottom - 1);
                MacLineTo( newRect.right + 1, newRect.bottom - 1);
            } else
            {
                MoveTo( hiliteBounds.right, hiliteBounds.top);
                MacLineTo( newRect.left - kMissionLineHJog, hiliteBounds.top);
                MacLineTo( newRect.left - kMissionLineHJog, newRect.top);
                MacLineTo( newRect.left - 2, newRect.top);
                MoveTo( hiliteBounds.right, hiliteBounds.bottom - 1);
                MacLineTo( newRect.left - kMissionLineHJog, hiliteBounds.bottom - 1);
                MacLineTo( newRect.left - kMissionLineHJog, newRect.bottom - 1);
                MacLineTo( newRect.left - 2, newRect.bottom - 1);
            }

        }
        dataItem->item.labeledRect.label.stringID = headerID;
        dataItem->item.labeledRect.label.stringNumber = headerNumber;
        GetAnyInterfaceItemGraphicBounds( dataItem, &newRect);
        SetTranslateColorFore( BLACK);
        PaintRect( &newRect);
        DrawAnyInterfaceItem( dataItem, *offMap, 0, 0);

        DrawInOffWorld();
        if ( textData != nil)
        {
            LongRectToRect( &(dataItem->bounds), &newRect);
            DrawInterfaceTextInRect( &newRect, (anyCharType *)*textData, length,
                            dataItem->style, dataItem->color, *offMap, 0, 0, inlinePict);
            ReleaseResource( textData);
        }

        DrawInRealWorld();
        NormalizeColors();

        GetAnyInterfaceItemGraphicBounds( dataItem, &newRect);
        BiggestRect( &newRect, &oldRect);
        BiggestRect( &newRect, &hiliteBounds);
        oldRect = *usedRect;
        BiggestRect( &oldRect, &newRect);
        CopyOffWorldToRealWorld((WindowPtr)gTheWindow, &oldRect);
        *usedRect = newRect;
    } else // it's a special briefpoint!
    {
        if ( inlinePict != nil)
        {
            thisInlinePict = inlinePict;
            for ( i = 0; i < kMaxInlinePictNum; i++)
            {
                thisInlinePict->id = -1;
                thisInlinePict->bounds.left = thisInlinePict->bounds.right = -1;
                thisInlinePict++;
            }
        }

        if ( whichBriefPoint == kMissionStarMapBriefNum)
        {
            thePict = (PicHandle)HHGetResource( 'PICT', kMissionStarMapPictID);
            if ( thePict != nil)
            {
                HLockHi( (Handle)thePict);
                newRect = (**thePict).picFrame;
                CenterRectInRect( &newRect, bounds);
                DrawPicture( thePict, &newRect);
                ReleaseResource( (Handle)thePict);
                GetScenarioStarMapPoint( whichScenario, &starPoint);
                starPoint.h += bounds->left;
                starPoint.v += bounds->top;

                mGetTranslateColorShade( GOLD, VERY_LIGHT, color, transColor)
                RectToLongRect( bounds, &longClipRect);
                starRect.left = starPoint.h - kMissionStarPointWidth;
                starRect.top = starPoint.v - kMissionStarPointHeight;
                starRect.right = starPoint.h + kMissionStarPointWidth;
                starRect.bottom = starPoint.v + kMissionStarPointHeight;

                DrawNateVBracket( *offMap, &starRect, &longClipRect, 0, 0,color);
                DrawNateLine( *offMap, &longClipRect, starPoint.h,
                            starPoint.v + kMissionStarPointHeight,
                            starPoint.h,
                            bounds->bottom, 0, 0, color);
                DrawNateLine( *offMap, &longClipRect, starPoint.h,
                            starPoint.v - kMissionStarPointHeight,
                            starPoint.h,
                            bounds->top, 0, 0, color);
                DrawNateLine( *offMap, &longClipRect, starPoint.h - kMissionStarPointWidth,
                            starPoint.v,
                            bounds->left,
                            starPoint.v, 0, 0, color);
                DrawNateLine( *offMap, &longClipRect, starPoint.h + kMissionStarPointWidth,
                            starPoint.v,
                            bounds->right,
                            starPoint.v, 0, 0, color);

                oldRect = *usedRect;
                BiggestRect( &oldRect, &newRect);
                DrawInRealWorld();
                NormalizeColors();
                CopyOffWorldToRealWorld((WindowPtr)gTheWindow, &oldRect);
                *usedRect = newRect;
                textlength = 100;
            }
        } else if ( whichBriefPoint == kMissionBlankBriefNum)
        {
            oldRect = *bounds;
            DrawInRealWorld();
            NormalizeColors();
            CopyOffWorldToRealWorld((WindowPtr)gTheWindow, &oldRect);
            *usedRect = oldRect;
            textlength = 50;
        }
    }

    MacShowCursor();

    return( textlength);
}

void ShowObjectData( Point where, short pictID, Rect *clipRect)
{
    Rect            dataRect;
    PixMapHandle    offPixMap = GetGWorldPixMap( gOffWorld);
    transColorType  *transColor;
    longRect        lRect, longClipRect;
    baseObjectType  *baseObject = (baseObjectType *)*gBaseObjectData;// + ((long)pictID - kFirstShipDataPictID);
    Str255          tempString, numString;
    retroTextSpecType   retroTextSpec;
    long            height, waitTime, i;
    Handle          weaponText;

    // find object who belongs to this pict id
    i = 0;
    while (( i < kMaxBaseObject) && ( baseObject->pictPortraitResID != pictID))
    {
        i++;
        baseObject++;
    }

    if ( i >= kMaxBaseObject) return;

//  if (( (pictID - kFirstShipDataPictID) >= 0) && (( pictID - kFirstShipDataPictID) < kMaxBaseObject))
    {
        HideCursor();

        retroTextSpec.text = GetResource( 'TEXT', kShipDataTextID);
        if ( retroTextSpec.text != nil)
        {
            DetachResource( retroTextSpec.text);

            // *** Replace place-holders in text with real data, using the fabulous Munger routine
            // an object or a ship?
            if ( baseObject->attributes & kCanThink)
                GetIndString( numString, kShipDataNameID, 1);
            else
                GetIndString( numString, kShipDataNameID, 2);

            GetIndString( tempString, kShipDataKeyStringID, kShipOrObjectStringNum);
            Munger( retroTextSpec.text, 0, (tempString + 1), *tempString, numString + 1, *numString);

            // ship name
//          GetIndString( numString, 5000, pictID - kFirstShipDataPictID + 1);
            GetIndString( numString, 5000, i + 1);
            GetIndString( tempString, kShipDataKeyStringID, kShipTypeStringNum);
            Munger( retroTextSpec.text, 0, (tempString + 1), *tempString, numString + 1, *numString);

            // ship mass
            SmallFixedToString( baseObject->mass, numString);
            GetIndString( tempString, kShipDataKeyStringID, kMassStringNum);
            Munger( retroTextSpec.text, 0, (tempString + 1), *tempString, numString + 1, *numString);

            // ship shields
            NumToString( baseObject->health, numString);
            GetIndString( tempString, kShipDataKeyStringID, kShieldStringNum);
            Munger( retroTextSpec.text, 0, (tempString + 1), *tempString, numString + 1, *numString);

            // light speed
            NumToString( baseObject->warpSpeed, numString);
            GetIndString( tempString, kShipDataKeyStringID, kHasLightStringNum);
            Munger( retroTextSpec.text, 0, (tempString + 1), *tempString, numString + 1, *numString);

            // max velocity
            SmallFixedToString( baseObject->maxVelocity, numString);
            GetIndString( tempString, kShipDataKeyStringID, kMaxSpeedStringNum);
            Munger( retroTextSpec.text, 0, (tempString + 1), *tempString, numString + 1, *numString);

            // thrust
            SmallFixedToString( baseObject->maxThrust, numString);
            GetIndString( tempString, kShipDataKeyStringID, kThrustStringNum);
            Munger( retroTextSpec.text, 0, (tempString + 1), *tempString, numString + 1, *numString);

            // par turn
            SmallFixedToString( baseObject->frame.rotation.turnAcceleration, numString);
            GetIndString( tempString, kShipDataKeyStringID, kTurnStringNum);
            Munger( retroTextSpec.text, 0, (tempString + 1), *tempString, numString + 1, *numString);

            // now, check for weapons!


            GetIndString( numString, kShipDataNameID, kShipDataPulseStringNum);
            weaponText = CreateWeaponDataText( baseObject->pulse, numString);
            if ( weaponText != nil)
            {
                HLock( weaponText);
                HandAndHand( weaponText, retroTextSpec.text);
                HUnlock( weaponText);
                DisposeHandle( weaponText);
            }

            GetIndString( numString, kShipDataNameID, kShipDataBeamStringNum);
            weaponText = CreateWeaponDataText( baseObject->beam, numString);
            if ( weaponText != nil)
            {
                HLock( weaponText);
                HandAndHand( weaponText, retroTextSpec.text);
                HUnlock( weaponText);
                DisposeHandle( weaponText);
            }

            GetIndString( numString, kShipDataNameID, kShipDataSpecialStringNum);
            weaponText = CreateWeaponDataText( baseObject->special, numString);
            if ( weaponText != nil)
            {
                HLock( weaponText);
                HandAndHand( weaponText, retroTextSpec.text);
                HUnlock( weaponText);
                DisposeHandle( weaponText);
            }


            retroTextSpec.textLength = GetHandleSize( retroTextSpec.text);

            mSetDirectFont( kButtonFontNum)
            retroTextSpec.thisPosition = retroTextSpec.linePosition = retroTextSpec.lineCount = 0;
            retroTextSpec.tabSize = 100;
            mGetTranslateColorShade( GREEN, VERY_LIGHT, retroTextSpec.color, transColor)
            mGetTranslateColorShade( GREEN, DARKEST, retroTextSpec.backColor, transColor)
            retroTextSpec.originalColor = retroTextSpec.nextColor = retroTextSpec.color;
            retroTextSpec.originalBackColor = retroTextSpec.nextBackColor = retroTextSpec.backColor;
            retroTextSpec.topBuffer = 1;
            retroTextSpec.bottomBuffer = 1;

            height = DetermineDirectTextHeightInWidth( &retroTextSpec, kShipDataWidth);

            dataRect.left = (where.h) - ( retroTextSpec.autoWidth / 2);
            dataRect.right = dataRect.left + retroTextSpec.autoWidth;
            dataRect.top = (where.v) - ( retroTextSpec.autoHeight / 2);
            dataRect.bottom = dataRect.top + retroTextSpec.autoHeight;

            if ( dataRect.left < clipRect->left)
            {
                MacOffsetRect( &dataRect, clipRect->left - dataRect.left + 1, 0);
            } else if ( dataRect.right > clipRect->right)
            {
                MacOffsetRect( &dataRect, clipRect->right - dataRect.right - 1, 0);
            }

            if ( dataRect.top < clipRect->top)
            {
                MacOffsetRect( &dataRect, 0, clipRect->top - dataRect.top + 1);
            } else if ( dataRect.bottom > clipRect->bottom)
            {
                MacOffsetRect( &dataRect, 0, clipRect->bottom - dataRect.bottom - 1);
            }
            retroTextSpec.xpos = dataRect.left;
            retroTextSpec.ypos = dataRect.top + mDirectFontAscent;

    //      clipRect.left = dataRect.left;
    //      clipRect.right = dataRect.right;
    //      clipRect.top = dataRect.top;
    //      clipRect.bottom = dataRect.bottom;
            RectToLongRect( clipRect, &longClipRect);
            RectToLongRect( &dataRect, &lRect);
            DrawInRealWorld();
            NormalizeColors();
            MacInsetRect( &dataRect, -8, -4);
            PaintRect( &dataRect);
            SetTranslateColorShadeFore( GREEN, VERY_LIGHT);
            MacFrameRect( &dataRect);
            NormalizeColors();

//          DrawDirectTextInRect( &retroTextSpec, &lRect, &lRect, *thePixMapHandle, gNatePortLeft, gNatePortTop);

            while (( retroTextSpec.thisPosition < retroTextSpec.textLength) && (( Button()) || (AnyRealKeyDown())))
            {
                PlayVolumeSound(  kComputerBeep3, kMediumLowVolume, kShortPersistence, kLowPrioritySound);
                DrawRetroTextCharInRect( &retroTextSpec, 24, &lRect, &lRect, *thePixMapHandle, gNatePortLeft,
                    gNatePortTop);

                waitTime = TickCount();
                while (( TickCount() - waitTime) < 3) {
                    // DO NOTHING
                };
            }

            DisposeHandle( retroTextSpec.text);
        }

        MacShowCursor();
        while (( Button()) || (AnyRealKeyDown())) {
            // DO NOTHING
        };

        CopyOffWorldToRealWorld((WindowPtr)gTheWindow, &dataRect);
    }
}

Handle CreateWeaponDataText( long whichWeapon, StringPtr weaponName)
{
    baseObjectType      *weaponObject, *missileObject;
    Handle              weaponText = nil;
    Str255              numString, tempString;
    long                mostDamage, actionNum;
    objectActionType    *action;
    Boolean             isGuided = false;

    if ( whichWeapon != kNoShip)
    {
        weaponObject = (baseObjectType *)*gBaseObjectData + whichWeapon;

        weaponText = GetResource( 'TEXT', kWeaponDataTextID);
        if ( weaponText != nil)
        {
            DetachResource( weaponText);

            // damage; this is tricky--we have to guess by walking through activate actions,
            //  and for all the createObject actions, see which creates the most damaging
            //  object.  We calc this first so we can use isGuided

            mostDamage = 0;
            isGuided = false;
            if ( weaponObject->activateActionNum > 0)
            {
                action = (objectActionType *)*gObjectActionData + weaponObject->activateAction;
                for ( actionNum = 0; actionNum < weaponObject->activateActionNum; actionNum++)
                {
                    if (( action->verb == kCreateObject) || ( action->verb == kCreateObjectSetDest))
                    {
                        missileObject = (baseObjectType *)*gBaseObjectData +
                            action->argument.createObject.whichBaseType;
                        if ( missileObject->attributes & kIsGuided) isGuided = true;
                        if ( missileObject->damage > mostDamage) mostDamage = missileObject->damage;
                    }
                    action++;
                }
            }

            // weapon name #
            GetIndString( tempString, kShipDataKeyStringID, kWeaponNumberStringNum);
            Munger( weaponText, 0, (tempString + 1), *tempString, weaponName + 1, *weaponName);

            // weapon name
            GetIndString( numString, 5000, whichWeapon + 1);
            GetIndString( tempString, kShipDataKeyStringID, kWeaponNameStringNum);
            Munger( weaponText, 0, (tempString + 1), *tempString, numString + 1, *numString);

            // is guided
            if ( isGuided)
                GetIndString( numString, kShipDataNameID, kShipDataYesStringNum);
            else
                GetIndString( numString, kShipDataNameID, kShipDataNoStringNum);
            GetIndString( tempString, kShipDataKeyStringID, kWeaponGuidedStringNum);
            Munger( weaponText, 0, (tempString + 1), *tempString, numString + 1, *numString);

            // is autotarget
            if ( weaponObject->attributes & kAutoTarget)
                GetIndString( numString, kShipDataNameID, kShipDataYesStringNum);
            else
                GetIndString( numString, kShipDataNameID, kShipDataNoStringNum);
            GetIndString( tempString, kShipDataKeyStringID, kWeaponAutoTargetStringNum);
            Munger( weaponText, 0, (tempString + 1), *tempString, numString + 1, *numString);

            // range
            NumToString( lsqrt(weaponObject->frame.weapon.range), numString);
            GetIndString( tempString, kShipDataKeyStringID, kWeaponRangeStringNum);
            Munger( weaponText, 0, (tempString + 1), *tempString, numString + 1, *numString);

            if ( mostDamage > 0)
            {
                NumToString( mostDamage, numString);
                GetIndString( tempString, kShipDataKeyStringID, kWeaponDamageStringNum);
                Munger( weaponText, 0, (tempString + 1), *tempString, numString + 1, *numString);
            } else
            {
                GetIndString( numString, kShipDataNameID, kShipDataDashStringNum);
                GetIndString( tempString, kShipDataKeyStringID, kWeaponDamageStringNum);
                Munger( weaponText, 0, (tempString + 1), *tempString, numString + 1, *numString);
            }

        }
    }
    return ( weaponText);
}
void ShowSuccessAnimation( WindowPtr thePort)

{
    long            lastTime, shipScale, zpoint, hpos, unitsToDo, ztimes, startimes, warpcount,
                    autoTimeStart;
    Point           vanishingPoint, shipPoint;
    longRect        starBounds, spriteBounds;
    Rect            tRect, lastBounds, theseBounds;
    Handle          shipSprite = nil;
    spritePix       aSpritePix;
    char            *pixData;
    unsigned char   color, *getwidchar, *getwidwid; // for getting string width
    PixMapHandle    pixMap = GetGWorldPixMap( gOffWorld),
                    saveMap = GetGWorldPixMap( gSaveWorld);
    Boolean         warp;
    char            hackString[] = "\pMISSION COMPLETE";
    transColorType  *transColor;

    SetLongRect( &starBounds, 0, 0, WORLD_WIDTH, WORLD_HEIGHT);
    LongRectToRect( &starBounds, &tRect);
    DrawInSaveWorld();
    SetTranslateColorFore( BLACK);
    PaintRect( &tRect);
    CopySaveWorldToOffWorld( &tRect);
    DrawInRealWorld();
    CopyOffWorldToRealWorld( thePort, &tRect);

// we assume we're recovering from a fade-out

    AutoFadeFrom( 1, FALSE);

    SetAllSoundsNoKeep();
    RemoveAllUnusedSounds();
    shipSprite = HHGetResource( kPixResType, kDebriefShipResID);
    if ( shipSprite != nil)
    {
        DetachResource( shipSprite);
        MoveHHi( shipSprite);
        HLock( shipSprite);
        RemapNatePixTableColor( shipSprite);

        AddSound( 516);
        pixData = GetNatePixTableNatePixData( shipSprite, kDebriefShipShapeNum);

        aSpritePix.data = &pixData;
        aSpritePix.center.h = GetNatePixTableNatePixHRef( shipSprite, kDebriefShipShapeNum);
        aSpritePix.center.v = GetNatePixTableNatePixVRef( shipSprite, kDebriefShipShapeNum);
        aSpritePix.width = GetNatePixTableNatePixWidth( shipSprite, kDebriefShipShapeNum);
        aSpritePix.height = GetNatePixTableNatePixHeight( shipSprite, kDebriefShipShapeNum);

        vanishingPoint.h = kDebriefVanishH + ( WORLD_WIDTH - kSmallScreenWidth) / 2;
        vanishingPoint.v = kDebriefVanishV + ( WORLD_HEIGHT - kSmallScreenHeight) / 2;
        shipPoint.h = -256;
        shipPoint.v = vanishingPoint.v + kDebriefShipV;

        OptScaleSpritePixInPixMap( &aSpritePix, shipPoint, SCALE_SCALE,
                &spriteBounds, &starBounds, pixMap);

        LongRectToRect( &spriteBounds, &lastBounds);
        HideCursor();

        LongRectToRect( &spriteBounds, &tRect);
//      ChunkCopyPixMapToScreenPixMap( *pixMap, &tRect, *thePixMapHandle);


        Reset3DStars( vanishingPoint, &starBounds);
        lastTime = TickCount();
        zpoint = kDebriefZMax;
        PlayVolumeSound( 516, kMaxSoundVolume, kLongPersistence, kMustPlaySound);
        warp = FALSE;
        ztimes = 1;
        startimes = 1;
        warpcount = 0;

        while ( (!AnyRealKeyDown()) && ( zpoint > kDebriefZMin))
        {
            ChunkCopyPixMapToPixMap( *saveMap, &(lastBounds), *pixMap);

            PrepareToMoveScrollStars();
            do
            {
                unitsToDo = TickCount() - lastTime;
            } while ( unitsToDo < 3);
            lastTime = TickCount();
            warpcount += unitsToDo;
            if ( warpcount > 80)
            {
                warp = TRUE;
                startimes = 8;
                ztimes = 4;
            }

            Move3DStars( vanishingPoint, unitsToDo * startimes, &starBounds);
            Draw3DStars( warp, &starBounds, pixMap);

            shipScale = (1 + zpoint * zpoint) / kDebriefZScaleMultiple;
            hpos = (zpoint * zpoint) / kDebriefShipHMultiple;
            hpos = vanishingPoint.h - hpos;
            shipPoint.h = hpos;
            hpos = (zpoint * zpoint) / kDebriefShipVMultiple;
            hpos = vanishingPoint.v - hpos;
            shipPoint.v = hpos;

            OptScaleSpritePixInPixMap( &aSpritePix, shipPoint, shipScale,
                    &spriteBounds, &starBounds, pixMap);
            LongRectToRect( &spriteBounds, &theseBounds);
            tRect = theseBounds;
            BiggestRect( &tRect, &lastBounds);

            Show3DStars( TRUE, &starBounds, pixMap);

            ChunkCopyPixMapToScreenPixMap( *pixMap, &tRect, *thePixMapHandle);
            lastBounds = theseBounds;
            zpoint -= 30 * unitsToDo * ztimes;
            /*
            if ( zpoint < kDebriefZMin)
            {
                zpoint = kDebriefZMax;
                warp = FALSE;
                ztimes = 1;
                startimes = 1;
                warpcount = 0;
                MacSetRect( &tRect, 0, 0, WORLD_WIDTH, WORLD_HEIGHT);
                DrawInSaveWorld();
                SetTranslateColorFore( BLACK);
                PaintRect( &tRect);
                CopySaveWorldToOffWorld( &tRect);
                DrawInRealWorld();
                CopyOffWorldToRealWorld( thePort, &tRect);
                PlayVolumeSound( 516, kMaxSoundVolume, kLongPersistence, kMustPlaySound);
            }*/

        }

        autoTimeStart = TickCount();

        while (( !AnyRealKeyDown()) && (( TickCount() - autoTimeStart) < kDebriefTimeOutTime))
        {
            mSetDirectFont( kTitleFontNum)

            PrepareToMoveScrollStars();
            do
            {
                unitsToDo = TickCount() - lastTime;
            } while ( unitsToDo < 3);
            lastTime = TickCount();
            Move3DStars( vanishingPoint, unitsToDo * startimes, &starBounds);
            Draw3DStars( warp, &starBounds, pixMap);

            mGetDirectStringDimensions( hackString, hpos, zpoint, ztimes, getwidchar, getwidwid)
            mGetTranslateColorShade( RED, VERY_LIGHT, color, transColor)
            tRect.left = (WORLD_WIDTH / 2) - (hpos / 2);
            tRect.top = (WORLD_HEIGHT / 2) - gDirectText->ascent / 2;
            tRect.right = tRect.left + hpos;
            tRect.bottom = tRect.top + zpoint;

            MoveTo( tRect.left, tRect.top + gDirectText->ascent);
            DrawDirectTextStringClipped( (anyCharType *)hackString, color, *pixMap, &starBounds,
                0, 0);

            ChunkCopyPixMapToScreenPixMap( *pixMap, &tRect, *thePixMapHandle);
            Show3DStars( TRUE, &starBounds, pixMap);
        }
        MacShowCursor();
        DisposeHandle( shipSprite);
    }
}

void DoMissionDebriefing( WindowPtr thePort, Rect *destRect, long yourlength, long parlength, long yourloss, long parloss,
    long yourkill, long parkill, long parScore)
{
    longRect            clipRect, boundsRect, tlRect;
    Rect                tRect;
    long                height, waitTime, score = 0;
    retroTextSpecType   retroTextSpec;
    transColorType      *transColor;
    Str255              tempString, numString;
    PixMapHandle        offMap = GetGWorldPixMap( gOffWorld);

#pragma unused( thePort, parScore)
//  MacSetPort( thePort);
//  BlackenWindow();

    // ** CALCULATE THE SCORE
    //  for time you get a max of 100 points
    //  50 points for par
    //  0-50 for par -> 2 x par
    //  50-100 for par -> 1/2 par
    //

    if ( yourlength < parlength)
    {
        if ( yourlength > ( parlength / 2))
        {
            score += (((kTimePoints * 2) * parlength - (kTimePoints * 2) * yourlength) / parlength) + kTimePoints;
        } else score += kTimePoints * 2;
    } else if ( yourlength > parlength)
    {
        if ( yourlength < ( parlength * 2))
        {
            score += ((kTimePoints * 2) * parlength - kTimePoints * yourlength) / parlength;
        } else score += 0;
    } else score += kTimePoints;

    if ( yourloss < parloss)
    {
        if ( yourloss > ( parloss / 2))
        {
            score += (((kLossesPoints * 2) * parloss - (kLossesPoints * 2) * yourloss) / parloss) + kLossesPoints;
        } else score += kLossesPoints * 2;
    } else if ( yourloss > parloss)
    {
        if ( yourloss < ( parloss * 2))
        {
            score += ((kLossesPoints * 2) * parloss - kLossesPoints * yourloss) / parloss;
        } else score += 0;
    } else score += kLossesPoints;

    if ( yourkill < parkill)
    {
        if ( yourkill > ( parkill / 2))
        {
            score += (((kKillsPoints * 2) * parkill - (kKillsPoints * 2) * yourkill) / parkill);
        } else score += 0;
    } else if ( yourkill > parkill)
    {
        if ( yourkill < ( parkill * 2))
        {
            score += ((kKillsPoints * 2) * parkill - kKillsPoints * yourkill) / parkill + kKillsPoints;
        } else score += kKillsPoints * 2;
    } else score += kKillsPoints;

    retroTextSpec.text = GetResource( 'TEXT', kSummaryTextID);
    if ( retroTextSpec.text != nil)
    {
        DetachResource( retroTextSpec.text);

        // *** Replace place-holders in text with real data, using the fabulous Munger routine
        // your minutes
        NumToString( yourlength / 60, numString);
        GetIndString( tempString, kSummaryKeyStringID, kYourMinStringNum);
        Munger( retroTextSpec.text, 0, (tempString + 1), *tempString, numString + 1, *numString);
        // your seconds
        NumToString( yourlength % 60, numString);
//      WriteDebugLine((char *)numString);
//      WriteDebugLong( yourlength % 60);

        mDoubleDigitize( numString);
        GetIndString( tempString, kSummaryKeyStringID, kYourSecStringNum);
        Munger( retroTextSpec.text, 0, (tempString + 1), *tempString, numString + 1, *numString);
        // par minutes
        if ( parlength > 0)
            NumToString( parlength / 60, numString);
        else GetIndString( numString, 6002, 9); // = N/A
            GetIndString( tempString, kSummaryKeyStringID, kParMinStringNum);
        Munger( retroTextSpec.text, 0, (tempString + 1), *tempString, numString + 1, *numString);
        // par seconds
        if ( parlength > 0)
        {
            NumToString( parlength % 60, tempString);
            mDoubleDigitize( tempString);
            CopyPString( numString, "\p:");
            ConcatenatePString( numString, tempString);
            GetIndString( tempString, kSummaryKeyStringID, kParSecStringNum);
            Munger( retroTextSpec.text, 0, (tempString + 1), *tempString, numString + 1, *numString);
        } else
        {
            GetIndString( tempString, kSummaryKeyStringID, kParSecStringNum);
            Munger( retroTextSpec.text, 0, (tempString + 1), *tempString, numString + 1, 0);
        }

        // your loss
        NumToString( yourloss, numString);
        GetIndString( tempString, kSummaryKeyStringID, kYourLossStringNum);
        Munger( retroTextSpec.text, 0, (tempString + 1), *tempString, numString + 1, *numString);
        // par loss
        if ( parlength > 0)
            NumToString( parloss, numString);
        else GetIndString( numString, 6002, 9); // = N/A
        GetIndString( tempString, kSummaryKeyStringID, kParLossStringNum);
        Munger( retroTextSpec.text, 0, (tempString + 1), *tempString, numString + 1, *numString);
        // your kill
        NumToString( yourkill, numString);
        GetIndString( tempString, kSummaryKeyStringID, kYourKillStringNum);
        Munger( retroTextSpec.text, 0, (tempString + 1), *tempString, numString + 1, *numString);
        // par kill
        if ( parlength > 0)
            NumToString( parkill, numString);
        else GetIndString( numString, 6002, 9); // = N/A
        GetIndString( tempString, kSummaryKeyStringID, kParKillStringNum);
        Munger( retroTextSpec.text, 0, (tempString + 1), *tempString, numString + 1, *numString);
        // your score
        if ( parlength > 0)
            NumToString( score, numString);
        else GetIndString( numString, 6002, 9); // = N/A
        GetIndString( tempString, kSummaryKeyStringID, kYourScoreStringNum);
        Munger( retroTextSpec.text, 0, (tempString + 1), *tempString, numString + 1, *numString);
        // par score
        if ( parlength > 0)
            CopyPString( numString, "\p100");
        else GetIndString( numString, 6002, 9); // = N/A
        GetIndString( tempString, kSummaryKeyStringID, kParScoreStringNum);
        Munger( retroTextSpec.text, 0, (tempString + 1), *tempString, numString + 1, *numString);

        retroTextSpec.textLength = GetHandleSize( retroTextSpec.text);

        mSetDirectFont( kButtonFontNum)
        retroTextSpec.thisPosition = retroTextSpec.linePosition = retroTextSpec.lineCount = 0;
        retroTextSpec.tabSize = 60;
        mGetTranslateColorShade( GOLD, VERY_LIGHT, retroTextSpec.color, transColor)
        mGetTranslateColorShade( GOLD, DARKEST, retroTextSpec.backColor, transColor)
        retroTextSpec.originalColor = retroTextSpec.nextColor = retroTextSpec.color;
        retroTextSpec.originalBackColor = retroTextSpec.nextBackColor = retroTextSpec.backColor;

        retroTextSpec.topBuffer = 2;
        retroTextSpec.bottomBuffer = 0;

        height = DetermineDirectTextHeightInWidth( &retroTextSpec, destRect->right - destRect->left);

        boundsRect.left = destRect->left + (((destRect->right - destRect->left) / 2) -
                            ( retroTextSpec.autoWidth / 2));
        boundsRect.right = boundsRect.left + retroTextSpec.autoWidth;
        boundsRect.top = destRect->top + (((destRect->bottom - destRect->top) / 2) -
                            ( retroTextSpec.autoHeight / 2));
        boundsRect.bottom = boundsRect.top + retroTextSpec.autoHeight;
        retroTextSpec.xpos = boundsRect.left;
        retroTextSpec.ypos = boundsRect.top + mDirectFontAscent;

        clipRect.left = 0;
        clipRect.right = clipRect.left + WORLD_WIDTH;
        clipRect.top = 0;
        clipRect.bottom = clipRect.top + WORLD_HEIGHT;
        RectToLongRect( destRect, &clipRect);
        mCopyAnyRect( tlRect, boundsRect);
        tlRect.left -= 2;
        tlRect.top -= 2;
        tlRect.right += 2;
        tlRect.bottom += 2;
        DrawNateVBracket( *offMap, &tlRect, &clipRect, 0, 0, retroTextSpec.color);
        mCopyAnyRect( tRect, tlRect);
        CopyOffWorldToRealWorld((WindowPtr)gTheWindow, &tRect);
//  DrawDirectTextInRect( &retroTextSpec, &clipRect, *thePixMapHandle, gNatePortLeft, gNatePortTop);

        while ( retroTextSpec.thisPosition < retroTextSpec.textLength)
        {
            PlayVolumeSound(  kTeletype, kMediumLowVolume, kShortPersistence, kLowPrioritySound);
            DrawRetroTextCharInRect( &retroTextSpec, 3, &boundsRect, &clipRect, *thePixMapHandle, gNatePortLeft,
                gNatePortTop);

            waitTime = TickCount();
            while (( TickCount() - waitTime) < 3) {
                // DO NOTHING
            };
        }
        DisposeHandle( retroTextSpec.text);
    }

//  WriteDebugLine((char *)"\plength:");
//  WriteDebugLong( yourlength);
/*  while ( AnyRealKeyDown());
    autoTimeStart = TickCount();

    while (( !AnyRealKeyDown()) && (!(( gAresGlobal->gOptions & (kOptionAutoPlay | kOptionReplay)) && (( TickCount() - autoTimeStart) < kDebriefTimeOutTime))));
*/
}

void DoMissionDebriefingText( WindowPtr thePort, long textID, long yourlength, long parlength,
            long yourloss, long parloss, long yourkill, long parkill, long parScore)
{
    Rect                tRect, iRect, scoreRect;
    Handle              textData;
    long                length, autoTimeStart, textlength = 0;
    short               textHeight = 0;
    Boolean             bailOut = false, doScore = (parScore >= 0);
    PixMapHandle        offMap = GetGWorldPixMap( gOffWorld);
    interfaceItemType       dataItem;

    MacSetRect( &tRect, CLIP_LEFT, CLIP_TOP, CLIP_RIGHT, CLIP_BOTTOM);
    MacSetRect( &iRect, 0, 0, kDebriefTextWidth, 1);

    dataItem.style = kLarge;
    textData = HHGetResource( 'TEXT', textID);
    if ( textData != nil)
    {
        HLockHi( textData);

        textlength = length = GetHandleSize( textData);
        textHeight = GetInterfaceTextHeightFromWidth( (anyCharType *)*textData, length,
                        dataItem.style, kDebriefTextWidth);
        if ( doScore) textHeight += kScoreTableHeight;

        iRect.bottom = iRect.top + textHeight;
        CenterRectInRect( &iRect, &tRect);

        RectToLongRect( &iRect, &(dataItem.bounds));
        dataItem.color = GOLD;
        dataItem.kind = kLabeledRect;
        dataItem.style = kLarge;
        dataItem.item.labeledRect.label.stringID = 2001;
        dataItem.item.labeledRect.label.stringNumber = 29;

        DrawInOffWorld();

        GetAnyInterfaceItemGraphicBounds( &dataItem, &tRect);
        SetTranslateColorFore( BLACK);
        PaintRect( &tRect);

        DrawAnyInterfaceItem( &dataItem, *offMap, 0, 0);

        LongRectToRect( &(dataItem.bounds), &tRect);
        DrawInterfaceTextInRect( &tRect, (anyCharType *)*textData, length,
                            dataItem.style, dataItem.color, *offMap, 0, 0, nil);

        ReleaseResource( textData);

        DrawInRealWorld();
        NormalizeColors();

        GetAnyInterfaceItemGraphicBounds( &dataItem, &tRect);
        CopyOffWorldToRealWorld( thePort, &tRect);

        if ( doScore)
        {
            scoreRect.left = dataItem.bounds.left;
            scoreRect.right = dataItem.bounds.right;
            scoreRect.bottom = dataItem.bounds.bottom;
            scoreRect.top = scoreRect.bottom - kScoreTableHeight;

            DoMissionDebriefing( thePort, &scoreRect, yourlength, parlength, yourloss, parloss,
                        yourkill, parkill, parScore);
        }
    }
    while (( AnyRealKeyDown()) || ( Button())) {
        // DO NOTHING
    };
    autoTimeStart = TickCount();
    while (( !AnyRealKeyDown()) && (!(Button())) &&
        (!(( gAresGlobal->gOptions & (kOptionAutoPlay | kOptionReplay)) &&
        (( TickCount() - autoTimeStart) < kDebriefTimeOutTime)))) {
        // DO NOTHING
    };
}

#define kBackground_Height  480
#define kScrollText_Buffer  10

void DoScrollText( WindowPtr thePort, long textID, long scrollSpeed, long scrollWidth,
    long textFontNum, long songID)
{
    longRect            clipRect, boundsRect, scrollRect, textRect;
    long                height, waitTime = TickCount(), l, autoTimeStart, sectionLength, textLength,
                        charNum, pictID, bgVOffset = 0, bgPictID = -1;
    retroTextSpecType   retroTextSpec;
    transColorType      *transColor;
    Str255              movieName;
    Rect                tRect, uRect, vRect, pictRect, pictSourceRect, movieRect;
    PixMapHandle        offMap = GetGWorldPixMap( gOffWorld), saveMap = GetGWorldPixMap( gSaveWorld);
    Handle              textHandle;
    anyCharType         *thisChar = nil, *sectionStart = nil, *nextChar;
    PicHandle           thePict = nil, bgPict = nil;
    Boolean             sectionOver, abort = false, wasPicture = true;
    Movie               theMovie = nil;
    RgnHandle           clipRgn = nil;

    if (( gAresGlobal->gOptions & kOptionMusicIdle) && ( songID >= 0))
    {
        if ( SongIsPlaying())
        {
            StopAndUnloadSong();
        }
        LoadSong( songID);
        SetSongVolume( kMaxMusicVolume);
        PlaySong();
    }

    MacSetPort( thePort);
    HideCursor();

    BlackenWindow();


    textHandle = GetResource( 'TEXT', textID);
    if ( ResError() != noErr) return;//Debugger();
    if ( textHandle != nil)
    {
        DetachResource( textHandle);
        if ( MemError() != noErr) return;//Debugger();
        HLockHi( textHandle);
        if ( MemError() != noErr) return;//Debugger();

        mSetDirectFont( textFontNum)

        boundsRect.left = (WORLD_WIDTH / 2) - ( scrollWidth / 2);
        boundsRect.right = boundsRect.left + scrollWidth;
        boundsRect.top = (WORLD_HEIGHT / 2) - ( kScrollTextHeight / 2);
        boundsRect.bottom = boundsRect.top + kScrollTextHeight;

        textRect.left = boundsRect.left;
        textRect.right = boundsRect.right;
        textRect.top = boundsRect.bottom;
        textRect.bottom = textRect.top + mDirectFontHeight + kScrollTextLineBuffer + 1;

        scrollRect.left = boundsRect.left;
        scrollRect.right = boundsRect.right;
        scrollRect.top = boundsRect.top;
        scrollRect.bottom = textRect.bottom;

        movieRect.left = boundsRect.left;
        movieRect.right = boundsRect.right;
        movieRect.bottom = boundsRect.top;
        movieRect.top = movieRect.bottom - kScrollMovieHeight;

        mCopyAnyRect( tRect, scrollRect);
        clipRgn = NewRgn();
        if ( clipRgn == nil) return;
        OpenRgn();
        FrameRect( &tRect);
        CloseRgn( clipRgn);

        DrawInRealWorld();
        MacSetPort( thePort);

/*      if ( LoadMiniMovie("\p:Ares Data Folder:Title", &theMovie, &movieRect, thePort, false) != noErr)
            Debugger();

        StartMiniMovie( theMovie);
*/
        DrawNateRect( *offMap, &scrollRect, 0, 0, 0xff);
        // Here's the behavior:
        //  a section is started with a '#' followed by a '+'
        //  it must be the first character, or the first character after a return
        //  the first section does not need a '#'
        //  if the # is followed by a number, that # is the picture header for that section
        //  the number must be terminated by a return
        //  if there is no number after # then it must be followed by a return
        //

        // look for beginning of section
        charNum = 0;
        textLength = GetHandleSize( textHandle);
        sectionStart = (anyCharType *)*textHandle;

        // while we still have text to do
        while (( charNum < textLength) && ( !abort))
        {
            sectionLength = 0;
            // if this section begins with a delimiter
            if ( *sectionStart == kScrollTextDelimiter1)
            {
                // then increase sectionStart to check for delimiter 2
                charNum++;
                sectionStart++;
                // if we haven't reached the end of the text
                if ( charNum < textLength)
                {
                    // then check for delimiter 2
                    pictID = 0;
                    if ( *sectionStart == kScrollTextDelimiter2)
                    {
                        sectionStart++;
                        charNum++;
                        // we have delimiter 2, check for pict number
                        if ( *sectionStart != kReturnChar)
                        {
                            if ((*sectionStart >= '0') && ( *sectionStart <= '9'))
                            {
                                while (( *sectionStart != kReturnChar) && ( charNum < textLength))
                                {
                                    pictID *= 10;
                                    pictID += (long)*sectionStart - (long)'0';
                                    sectionStart++;
                                    charNum++;
                                }
                            } else if ( *sectionStart == kScrollTextBackgroundChar)
                            {
                                sectionStart++;
                                charNum++;
                                // we have delimiter 2, check for pict number
                                if ( *sectionStart != kReturnChar)
                                {
                                    if ((*sectionStart >= '0') && ( *sectionStart <= '9'))
                                    {
                                        bgPictID = 0;
                                        while (( *sectionStart != kReturnChar) &&
                                            ( charNum < textLength))
                                        {
                                            bgPictID *= 10;
                                            bgPictID += (long)*sectionStart - (long)'0';
                                            sectionStart++;
                                            charNum++;
                                        }
                                        if ( bgPictID > 0)
                                        {
                                            if ( bgPict != nil)
                                                ReleaseResource( (Handle)bgPict);
                                            bgPict = GetPicture( bgPictID);
                                        }
                                    }
                                }
                            } else if ( *sectionStart == kScrollTextMovieChar)
                            {
                                if ( theMovie != nil) CleanUpMiniMovie( &theMovie);
                                movieName[0] = 0;

                                sectionStart++;
                                charNum++;
                                l = 1;
                                while (( *sectionStart != kReturnChar) && ( charNum < textLength))
                                {
                                    movieName[0]++;
                                    movieName[l] = *sectionStart;
                                    sectionStart++;
                                    charNum++;
                                    l++;
                                }
                                if ( LoadMiniMovie( movieName, &theMovie, &movieRect, thePort, false) != noErr)
                                {
                                    //Debugger();
                                }

                                StartMiniMovie( theMovie);
                            }
                        }
                        sectionStart++;
                        charNum++;
                    }
                }
            }

            // now find end of section; either delimiter or end of text
            thisChar = sectionStart;
            sectionOver = false;
            while (( charNum < textLength) && ( !sectionOver) && ( !abort))
            {
                if (( *thisChar == kScrollTextDelimiter1) && ( charNum < (textLength - 1)))
                {
                    nextChar = thisChar;
                    nextChar++;
                    if ( *nextChar == kScrollTextDelimiter2)
                    {
                        sectionOver = true;
                    } else
                    {
                        charNum++;
                        thisChar++;
                        sectionLength++;
                    }

                } else
                {
                    charNum++;
                    thisChar++;
                    sectionLength++;
                }
            }
//          retroTextSpec.text = NewHandle( 1);
            retroTextSpec.text = nil;
            if ( PtrToHand( sectionStart, &(retroTextSpec.text), sectionLength) == noErr)
            {
                if ( retroTextSpec.text != nil)
                {
                    sectionStart = thisChar;
                    HLockHi( retroTextSpec.text);

                    retroTextSpec.textLength = GetHandleSize( retroTextSpec.text);

                    retroTextSpec.thisPosition = retroTextSpec.linePosition =
                        retroTextSpec.lineCount = 0;
                    retroTextSpec.tabSize = scrollWidth / 2;
                    mGetTranslateColorShade( RED, VERY_LIGHT, retroTextSpec.color, transColor)
            //      mGetTranslateColorShade( RED, DARKEST, retroTextSpec.backColor, transColor)
                    retroTextSpec.backColor = WHITE;//0xff;
                    retroTextSpec.originalColor = retroTextSpec.nextColor = retroTextSpec.color;
                    retroTextSpec.originalBackColor = retroTextSpec.nextBackColor = retroTextSpec.backColor;
                    retroTextSpec.topBuffer = kScrollTextLineBuffer;
                    retroTextSpec.bottomBuffer = 0;

                    height = DetermineDirectTextHeightInWidth( &retroTextSpec,
                        scrollWidth-(kScrollText_Buffer<<1));

                    clipRect.left = 0;
                    clipRect.right = clipRect.left + WORLD_WIDTH;
                    clipRect.top = 0;
                    clipRect.bottom = clipRect.top + WORLD_HEIGHT;

                    if ( pictID != 0)
                    {
                        thePict = (PicHandle)GetResource( 'PICT', pictID);
//                      if ( ResError() != noErr) Debugger();

                        if ( thePict != nil)
                        {
                            HLockHi( (Handle)thePict);
                            wasPicture = true;
                            pictRect.left = ( scrollWidth / 2) -
                                ((((**thePict).picFrame.right - (**thePict).picFrame.left)) / 2) +
                                boundsRect.left;
                            pictRect.right = pictRect.left + (((**thePict).picFrame.right - (**thePict).picFrame.left));
                            pictRect.top = boundsRect.bottom;
//                          pictRect.bottom = pictRect.top + ((**thePict).picFrame.bottom - (**thePict).picFrame.top);
                            pictRect.bottom = pictRect.top + mDirectFontHeight + kScrollTextLineBuffer;

                            pictSourceRect = (**thePict).picFrame;
                            pictSourceRect.left = ( scrollWidth / 2) -
                                ((((**thePict).picFrame.right - (**thePict).picFrame.left)) / 2) +
                                boundsRect.left;
                            pictSourceRect.right = pictRect.left + (((**thePict).picFrame.right - (**thePict).picFrame.left));

                            DrawInSaveWorld();
                                if ( bgPict != nil)
                                {
                                    Rect    bgRect = (**bgPict).picFrame;

                                    OffsetRect( &bgRect, -bgRect.left, -bgRect.top);
                                    OffsetRect( &bgRect, scrollRect.left,
                                        pictSourceRect.top - bgVOffset);
                                    do
                                    {
                                        DrawPicture( bgPict, &bgRect);
                                        MacOffsetRect( &bgRect, 0, kBackground_Height);
                                    } while ( bgRect.top < (**saveMap).bounds.bottom);
                                }
                            DrawPicture( thePict, &pictSourceRect);
                            DrawInRealWorld();

                            if ( bgPict != nil)
                            {
                                pictRect.left = pictSourceRect.left = scrollRect.left;
                                pictRect.right = pictSourceRect.right = scrollRect.right;
                            }
                            pictSourceRect.bottom = pictSourceRect.top + mDirectFontHeight + kScrollTextLineBuffer;

                            while (( pictSourceRect.top < (**thePict).picFrame.bottom) && (!abort))
                            {
                                if ( pictSourceRect.bottom > (**thePict).picFrame.bottom)
                                {
                                    pictRect.bottom -= pictSourceRect.bottom - (**thePict).picFrame.bottom;
                                    pictSourceRect.bottom = (**thePict).picFrame.bottom;
                                }
                                CopyBits( (BitMap *)*saveMap, (BitMap *)*offMap,
                                    &pictSourceRect, &pictRect,
                                    srcCopy, nil);

                                LongRectToRect( &scrollRect, &tRect);
                                uRect = tRect;
                                MacOffsetRect( &uRect, 0, -1);
                                LongRectToRect( &boundsRect, &vRect);


                                for (   l = 0;
                                        ((l < (mDirectFontHeight + kScrollTextLineBuffer)) &&
                                            (!abort) &&
                                            ((pictSourceRect.top+l)<
                                                (**thePict).picFrame.bottom));
                                        l++)
                                {
                                    DrawInOffWorld();

//                                  CopyBits( (BitMap *)*offMap, (BitMap *)*offMap, &tRect, &uRect,
//                                      srcCopy, nil);
                                    ScrollRect( &tRect, 0, -1, clipRgn);
                                    DrawInRealWorld();

//                                  DrawNateLine(  *offMap, &scrollRect, scrollRect.left, pictRect.bottom - 1, scrollRect.right - 1,
//                                      pictRect.bottom - 1, 0, 0, BLACK);
                                    DrawNateLine(  *offMap, &scrollRect, scrollRect.left, scrollRect.bottom - 1, scrollRect.right - 1,
                                        scrollRect.bottom - 1, 0, 0, BLACK);
                                    CopyOffWorldToRealWorld( thePort, &vRect);

                                    bgVOffset++;
                                    if ( bgVOffset >= kBackground_Height) bgVOffset = 0;

                                    if ( theMovie != nil)
                                    {
                                        if ( DoMiniMovieTask( theMovie))
                                        {
                                            CleanUpMiniMovie( &theMovie);
                                        }
                                    }

                                    while (( TickCount() - waitTime) < scrollSpeed) {
                                        // DO NOTHING
                                    };
                                    waitTime = TickCount();
                                    if ( AnyModifierKeyDown())
                                    {
                                        while ( AnyModifierKeyDown())
                                        {
                                            // do nothing
                                        }
                                        while ( AnyEvent())
                                        {
                                            // do nothing
                                        }
                                    }
                                    if ( AnyEvent()) abort = true;
                                }

                                MacOffsetRect( &pictSourceRect, 0, (mDirectFontHeight + kScrollTextLineBuffer));
                            }
                            HUnlock( (Handle)thePict);
                            ReleaseResource( (Handle)thePict);
//                          if ( ResError() != noErr) Debugger();
                        }// else DebugStr("\pNo PICT!");
                    }
                    if  ( wasPicture)
                    {
                        Rect    bgRect = (**bgPict).picFrame;

                        wasPicture = false;
                        DrawInSaveWorld();
                        if ( bgPict != nil)
                        {
                            OffsetRect( &bgRect, -bgRect.left, -bgRect.top);
                            OffsetRect( &bgRect, scrollRect.left, 0);
                            do
                            {
                                DrawPicture( bgPict, &bgRect);
                                MacOffsetRect( &bgRect, 0, kBackground_Height);
                            }  while ( bgRect.top < (**saveMap).bounds.bottom);
                        } else
                        {
                            EraseSaveWorld();
                        }
                        DrawInRealWorld();
                    }
                    while (( retroTextSpec.thisPosition < retroTextSpec.textLength) && (!abort))
                    {
                        Rect    bgRect, stRect;

                        retroTextSpec.xpos = textRect.left + 6;
                        retroTextSpec.ypos = textRect.top + mDirectFontAscent + kScrollTextLineBuffer;

                        DrawInOffWorld();
                        mCopyAnyRect( bgRect, textRect);
                        mCopyAnyRect( stRect, textRect);
                        OffsetRect( &bgRect, 0, -bgRect.top);
                        OffsetRect( &bgRect, 0, bgVOffset);
                        // if source bg pict is partially offscreen
                        if ( bgRect.bottom > (**saveMap).bounds.bottom)
                        {
                            stRect.bottom -= bgRect.bottom - (**saveMap).bounds.bottom;
                            bgRect.bottom = (**saveMap).bounds.bottom;
                            CopyBits( (BitMap *)*saveMap, (BitMap *)*offMap,
                                &bgRect, &stRect,
                                srcCopy, nil);
                            stRect.top = stRect.bottom;
                            stRect.bottom = stRect.top + ((textRect.bottom - textRect.top) -
                                            ( bgRect.bottom - bgRect.top));
                            bgRect.top = 0;
                            bgRect.bottom = bgRect.top + (stRect.bottom - stRect.top);
                            CopyBits( (BitMap *)*saveMap, (BitMap *)*offMap,
                                &bgRect, &stRect,
                                srcCopy, nil);
                        } else // just copy appropriate segment
                        {
                            CopyBits( (BitMap *)*saveMap, (BitMap *)*offMap,
                                &bgRect, &stRect,
                                srcCopy, nil);
                        }
                        DrawInRealWorld();

                        textRect.right -= kScrollText_Buffer;
                        DrawRetroTextCharInRect( &retroTextSpec, -1, &textRect, &textRect, *offMap, 0, 0);
                        textRect.right += kScrollText_Buffer;
                        LongRectToRect( &scrollRect, &tRect);
                        uRect = tRect;
                        MacOffsetRect( &uRect, 0, -1);

                        bgVOffset++;
                        if ( bgVOffset >= kBackground_Height) bgVOffset = 0;

                        LongRectToRect( &boundsRect, &vRect);
                        for (   l = 0;
                                ((l < (mDirectFontHeight + kScrollTextLineBuffer)) &&
                                    (!abort));
                                l++)
                        {
                            DrawInOffWorld();
//                          CopyBits( (BitMap *)*offMap, (BitMap *)*offMap, &tRect, &uRect,
//                              srcCopy, nil);
                            ScrollRect( &tRect, 0, -1, clipRgn);

                            bgVOffset++;
                            if ( bgVOffset >= kBackground_Height) bgVOffset = 0;

                            DrawNateLine(  *offMap, &scrollRect, scrollRect.left, scrollRect.bottom - 1, scrollRect.right - 1,
                                scrollRect.bottom - 1, 0, 0, BLACK);
                            DrawInRealWorld();
                            CopyOffWorldToRealWorld( thePort, &vRect);

                            if ( theMovie != nil)
                            {
                                if ( DoMiniMovieTask( theMovie))
                                {
                                    CleanUpMiniMovie( &theMovie);
                                }
                            }

                            while (( TickCount() - waitTime) < scrollSpeed) {
                                // DO NOTHING
                            };
                            waitTime = TickCount();
                            if ( AnyModifierKeyDown())
                            {
                                while ( AnyModifierKeyDown())
                                {
                                    // do nothing;
                                }
                                while ( AnyEvent())
                                {
                                    // do nothing;
                                }
                            }
                            if ( AnyEvent()) abort = true;
                        }
                    }
                    HUnlock( retroTextSpec.text);
                    if ( MemError() != noErr) return;//Debugger();
                    DisposeHandle( retroTextSpec.text);
                    if ( MemError() != noErr) return;//Debugger();
                    retroTextSpec.text = nil;
                }// else DebugStr("\pNil Handle");
            }// else DebugStr("\pError");
        }

        for (   l = 0;
                ((l < kScrollTextHeight) &&
                    (!abort));
                l++)
        {
            DrawInOffWorld();
            LongRectToRect( &scrollRect, &tRect);
//          CopyBits( (BitMap *)*offMap, (BitMap *)*offMap, &tRect, &uRect,
//              srcCopy, nil);
            ScrollRect( &tRect, 0, -1, clipRgn);
            DrawNateLine(  *offMap, &scrollRect, scrollRect.left, scrollRect.bottom - 1, scrollRect.right - 1,
                scrollRect.bottom - 1, 0, 0, BLACK);
            DrawInRealWorld();
            CopyOffWorldToRealWorld( thePort, &vRect);

            if ( theMovie != nil)
            {
                if ( DoMiniMovieTask( theMovie))
                {
                    CleanUpMiniMovie( &theMovie);
                }
            }

            while (( TickCount() - waitTime) < scrollSpeed) {
                // DO NOTHING
            };
            waitTime = TickCount();
            if ( AnyModifierKeyDown())
            {
                while ( AnyModifierKeyDown())
                                {
                                    // do nothing;
                                }
                while ( AnyEvent())
                                {
                                    // do nothing;
                                }
            }
            if ( AnyEvent()) abort = true;
        }
        HUnlock( textHandle);
        if ( MemError() != noErr) return//Debugger();
        DisposeHandle( textHandle);
        if ( MemError() != noErr) return;//Debugger();
    } else SysBeep( 20);

    while ( AnyRealKeyDown()) {
        // DO NOTHING
    };
    autoTimeStart = TickCount();

    if ( theMovie != nil) CleanUpMiniMovie( &theMovie);
//  while (( !AnyRealKeyDown()) && (!abort) && (!(( gAresGlobal->gOptions & (kOptionAutoPlay | kOptionReplay)) && (( TickCount() - autoTimeStart) < kDebriefTimeOutTime))));
    MacShowCursor();

    if (( SongIsPlaying()) && ( songID >= 0))
    {
        StopAndUnloadSong();
    }
    if ( bgPict != nil)
        ReleaseResource( (Handle)bgPict);
    if ( clipRgn != nil) DisposeRgn( clipRgn);
}

// HANDLING SUSPEND & RESUME

void HandleOSEvent( EventRecord *event)
{
    long    eventType = event->message;

    eventType >>= 24L;
    eventType &= 0xff;

    switch (eventType)
    {
        case mouseMovedMessage:
    //                          DoIdle(event); {mouse-moved same as idle for this app}
            break;

        case suspendResumeMessage:
            if ( event->message & resumeFlag)
            {
                ToggleSong();
                InitCursor();
                if ( event->message & convertClipboardFlag)
                {
                    // we don't care about clipboard
                }
                if ( GetDeviceDepth( theDevice) != 8)
                    SetColorDepth( theDevice, 8);
                SetMBarState( false, theDevice);
                gAresGlobal->gOptions &= ~kOptionInBackground;
                SelectWindow( (WindowPtr)gTheWindow);
                MacSetPort( (WindowPtr)gTheWindow);
                ResumeActiveTextEdit();

                if ( gAresGlobal->useGameRanger)
                {
                    if (( Wrap_GRCheckForAE())/* && (!gAresGlobal->gameRangerPending)*/)
                    {
                        gAresGlobal->gameRangerPending = true;
                        gAresGlobal->returnToMain = true;
                        SysBeep(20);
                    }
                }

                WriteDebugLine((char *)"\pRESUME");
                Ambrosia_Update_Registered();
            } else
            {
                ToggleSong();
                SetMBarState( true, theDevice);
                gAresGlobal->gOptions |= kOptionInBackground;
                SuspendActiveTextEdit();
                WriteDebugLine((char *)"\pSUSPEND");
            }
            break;
    }
}

Boolean Ares_WaitNextEvent( short eventMask, EventRecord *theEvent,
    unsigned long sleep, RgnHandle mouseRgn)
{
    Boolean result = WaitNextEvent( eventMask, theEvent, sleep, mouseRgn);

    if ( gAresGlobal != nil)
    {
        switch( theEvent->what)
        {
            case kHighLevelEvent:
                if ( gAresGlobal->aeInited)
                {
                    AEProcessAppleEvent( theEvent);
                }
                break;

            case osEvt:
                HandleOSEvent( theEvent);
                break;

        }
    }

        if ((!(gAresGlobal->gOptions & kOptionInBackground)) &&
            (!gAresGlobal->gameRangerPending))
{
    if (( Wrap_GRIsWaitingCmd())/* && ( !gAresGlobal->gameRangerPending)*/)
    {
        WriteDebugLine((char *)"\pGRIsWaiting!");
        if ( !gAresGlobal->gameRangerPending)
        {
            gAresGlobal->gameRangerPending = true;
            gAresGlobal->returnToMain = true;
        }
    }
}
//  if ( Wrap_GRIsCmd())
//  {
//      WriteDebugLine((char *)"\pGRCommand!");
//  }

    return result;
}

void Replace_KeyCode_Strings_With_Actual_Key_Names( Handle text, short resID,
    short padTo)
{
    long    l;
    Str255  numString, tempString;

    for ( l = 0; l < kKeyExtendedControlNum; l++)
    {
//      GetKeyNumName( numString, GetKeyNumFromKeyMap( gAresGlobal->gKeyControl[l]));
        GetIndString( numString, resID, GetKeyNumFromKeyMap( gAresGlobal->gKeyControl[l]));
        while ( numString[0] < padTo)
        {
            numString[0]++;
            numString[numString[0]] = ' ';
        }
        GetIndString( tempString, kHelpScreenKeyStringID, l + 1);
        while ( Munger( text, 0, (tempString + 1), *tempString,
            numString + 1, *numString) > 0) {
            // DO NOTHING
        };
    }
}
