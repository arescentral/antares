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

#include "rezin/MacRoman.hpp"
#include "AresGlobalType.hpp"
#include "AresMain.hpp"
#include "AresPreferences.hpp"
#include "BriefingRenderer.hpp"
#include "ColorTranslation.hpp"
#include "Debug.hpp"
#include "DirectText.hpp"
#include "Error.hpp"
#include "Fakes.hpp"
#include "Instruments.hpp"
#include "InterfaceHandling.hpp"
#include "KeyCodes.hpp"
#include "KeyMapTranslation.hpp"
#include "MessageScreen.hpp"
#include "Music.hpp"
#include "NatePixTable.hpp"
#include "NetSetupScreen.hpp"
#include "OffscreenGWorld.hpp"
#include "Options.hpp"
#include "Picture.hpp"
#include "Races.hpp"
#include "Resource.hpp"
#include "ScenarioMaker.hpp"
#include "ScrollStars.hpp"
#include "SoundFX.hpp"               // for button on/off
#include "StringHandling.hpp"
#include "StringList.hpp"
#include "StringNumerics.hpp"
#include "Transitions.hpp"
#include "VideoDriver.hpp"

using rezin::mac_roman_encoding;
using sfz::BytesPiece;
using sfz::String;
using sfz::StringPiece;
using sfz::scoped_ptr;
using sfz::scoped_array;

namespace antares {

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

#define kTimePoints                 50
#define kLossesPoints               30
#define kKillsPoints                20

#define kScoreTableHeight           120

#define kFirstShipDataPictID        1001
#define kShipDataTextID             6001
#define kShipDataKeyStringID        6001
#define kShipDataNameID             6002
#define kWeaponDataTextID           6003
#define kShipOrObjectStringNum      0
#define kShipTypeStringNum          1
#define kMassStringNum              2
#define kShieldStringNum            3
#define kHasLightStringNum          4
#define kMaxSpeedStringNum          5
#define kThrustStringNum            6
#define kTurnStringNum              7
#define kWeaponNumberStringNum      8
#define kWeaponNameStringNum        9
#define kWeaponGuidedStringNum      10
#define kWeaponRangeStringNum       11
#define kWeaponDamageStringNum      12
#define kWeaponAutoTargetStringNum  13
#define kShipDataShipStringNum      0
#define kShipDataObjectStringNum    1
#define kShipDataDashStringNum      2
#define kShipDataYesStringNum       3
#define kShipDataNoStringNum        4
#define kShipDataPulseStringNum     5
#define kShipDataBeamStringNum      6
#define kShipDataSpecialStringNum   7

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

#define kScrollTextLineBuffer       2
#define kScrollTextDelimiter1       '#'
#define kScrollTextDelimiter2       '+'
#define kReturnChar                 0x0d
#define kScrollTextBackgroundChar   'B'

#define mPlayScreenSound            PlayVolumeSound( kComputerBeep3, kMediumLowVolume, kShortPersistence, kMustPlaySound)

inline void mDoubleDigitize(unsigned char* mstring) {
    if (mstring[0] == 1) {
        mstring[0] = 2;
        mstring[2] = mstring[1];
        mstring[1] = '0';
    }
}

extern Handle                   gHostEntity, gClientEntity;
extern long                     gNatePortLeft, gNatePortTop, gNetLatency, gRandomSeed,
                                CLIP_LEFT, CLIP_RIGHT, CLIP_TOP, CLIP_BOTTOM;
extern directTextType*          gDirectText;
extern long                     WORLD_WIDTH, WORLD_HEIGHT;
extern scoped_array<baseObjectType> gBaseObjectData;
extern scoped_array<objectActionType> gObjectActionData;
extern PixMap*                  gActiveWorld;
extern PixMap*                  gOffWorld;
extern PixMap*                  gSaveWorld;

void BlackenWindow();

void DoLoadingInterface(Rect *contentRect, unsigned char* levelName) {
    int                     error;
    RgbColor color;
    Rect                lRect, clipRect, boundsRect;
    Rect                    tRect;
    retroTextSpecType       retroTextSpec;
    long                    height;

    BlackenWindow();

    error = OpenInterface( kLoadingScreenID);
    if ( error == kNoError)
    {
        DrawEntireInterface();

        GetAnyInterfaceItemContentBounds(*GetAnyInterfaceItemPtr(0), contentRect); // item 0 = loading rect
        CloseInterface();

// it is assumed that we're "recovering" from a fade-out
        // AutoFadeFrom( 10, false);

        DrawInRealWorld();
        mSetDirectFont( kTitleFontNum);
        GetRGBTranslateColorShade(&color, PALE_GREEN, LIGHT);
        lRect.left = 0;
        lRect.top = 0;
        lRect.right = WORLD_WIDTH;
        lRect.bottom = WORLD_HEIGHT;
//      MoveTo( contentRect->left + (( contentRect->right - contentRect->left) / 2) - (stringWidth / 2),
//              contentRect->top);

        retroTextSpec.textLength = *levelName;
        retroTextSpec.text.reset(new String);
        for (int i = 0; i < retroTextSpec.textLength; ++i) {
            retroTextSpec.text->append(
                    BytesPiece(reinterpret_cast<const uint8_t*>(levelName + 1), *levelName),
                    mac_roman_encoding());
        }

        retroTextSpec.thisPosition = retroTextSpec.linePosition = retroTextSpec.lineCount = 0;
        retroTextSpec.tabSize =220;
        GetRGBTranslateColorShade(&retroTextSpec.color, PALE_GREEN, VERY_LIGHT);
        GetRGBTranslateColorShade(&retroTextSpec.backColor, SKY_BLUE, DARKEST);
        retroTextSpec.backColor = RgbColor::kBlack;
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
        retroTextSpec.ypos = boundsRect.top + mDirectFontAscent();

        clipRect.left = 0;
        clipRect.right = clipRect.left + WORLD_WIDTH;
        clipRect.top = 0;
        clipRect.bottom = clipRect.top + WORLD_HEIGHT;
        while ( retroTextSpec.thisPosition < retroTextSpec.textLength)
        {
            PlayVolumeSound(  kTeletype, kMediumLowVolume, kShortPersistence, kLowPrioritySound);
            DrawRetroTextCharInRect( &retroTextSpec, 3, boundsRect, clipRect, gActiveWorld, gNatePortLeft,
                gNatePortTop);
        }

        retroTextSpec.text.reset();

        tRect = boundsRect;
    }
}

void UpdateLoadingInterface( long value, long total, Rect *contentRect)

{
    RgbColor        color;
    long            width, height, temp;
    Rect        clipRect;
    Rect            tRect;
    Str255          string;

    Button();  // Hack to get it to update.
    if ( total < 0)
    {
        DrawInOffWorld();
        DefaultColors();
        gActiveWorld->view(*contentRect).fill(RgbColor::kBlack);
        GetIndString( string, 2004, 33);

        mSetDirectFont( kButtonFontNum);
        mGetDirectStringDimensions(PStringPiece(string), width, height);

        clipRect = *contentRect;
        tRect = Rect(0, 0, width, height);
        tRect.center_in(*contentRect);

        GetRGBTranslateColorShade(&color, kLoadingScreenColor, LIGHTER);
        MoveTo( tRect.left, tRect.top + mDirectFontAscent());
        DrawDirectTextStringClipped(PStringPiece(string), color, gOffWorld, clipRect, 0, 0);


        DrawInRealWorld();
        DefaultColors();
        CopyOffWorldToRealWorld(*contentRect);
    } else
    {
        width = contentRect->right - contentRect->left;
        DrawInOffWorld();

        temp = (value * width);
        temp /= total;

        tRect = Rect(contentRect->left, contentRect->top, contentRect->left + temp, contentRect->bottom);
        SetTranslateColorShadeFore( kLoadingScreenColor, LIGHT);
        GetRGBTranslateColorShade(&color, kLoadingScreenColor, LIGHT);
        gActiveWorld->view(tRect).fill(color);

        tRect = Rect(contentRect->left + temp, contentRect->top, contentRect->right, contentRect->bottom);
        SetTranslateColorShadeFore( kLoadingScreenColor, DARK);
        GetRGBTranslateColorShade(&color, kLoadingScreenColor, DARK);
        gActiveWorld->view(tRect).fill(color);
        NormalizeColors();
        DrawInRealWorld();
        NormalizeColors();
        tRect = Rect(contentRect->left, contentRect->top, contentRect->right, contentRect->bottom);
        CopyOffWorldToRealWorld(tRect);
        if (tRect.left >= tRect.right - 2) {
            // AutoFadeTo( 10, RgbColor::kBlack, false);
        }
    }
}

void DoNetSettings( void)

{
#ifdef NETSPROCKET_AVAILABLE
    int                     error = kNoError;
    Rect                    tRect;
    interfaceItemType       *item;
    bool                 done = false, cancel = false, currentBandwidth = GetBandwidth();
    Point                   where;
    short                   whichItem, result = 0, currentRegistered = 0,
                            currentDelay = 0, i;
    EventRecord             theEvent;
    char                    whichChar;

//  BlackenWindow();

    FlushEvents(everyEvent, 0);
    OpenInterface( kNetSettingsID);
    if ( error == kNoError)
    {
        tRect = Rect(CLIP_LEFT, CLIP_TOP, CLIP_RIGHT, CLIP_BOTTOM);
        CenterAllItemsInRect( &tRect);
        item = GetAnyInterfaceItemPtr( kNetSettingsBox);
        DrawInOffWorld();
        GetAnyInterfaceItemGraphicBounds( item, &tRect);
        DefaultColors();
        CopyOffWorldToSaveWorld( &tRect);
        gActiveWorld->view( &tRect).fill(RgbColor::kBlack);
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

        DrawAllItemsOfKind( kPictureRect, false, false, false);
        DrawAllItemsOfKind( kLabeledRect, true, false, false);
        DrawAllItemsOfKind( kPlainRect, true, false, false);
        DrawAllItemsOfKind( kCheckboxButton, true, false, false);
        DrawAllItemsOfKind( kRadioButton, true, false, false);
        DrawAllItemsOfKind( kPlainButton, true, false, false);
        DrawAllItemsOfKind( kTextRect, true, false, false);
        while ( AnyRealKeyDown()) {
            // DO NOTHING
        };

        while (!done) {
            WaitNextEvent (everyEvent, &theEvent, 3, nil);
            {
                whichItem = -1;
                switch ( theEvent.what )
                {
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
                    case kNetSettingsOKButton:
                        done = true;
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
        gActiveWorld->view( &tRect).fill(RgbColor::kBlack);
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
        }
    }
#endif NETSPROCKET_AVAILABLE
}

void StartPauseIndicator(const StringPiece& pauseString, unsigned char hue) {
    RgbColor        color;
    long            width, height, count;
    Rect            tRect, stringRect;
    Rect        clipRect;

#pragma unused( hue)
    mSetDirectFont( kTitleFontNum);
    mGetDirectStringDimensions(pauseString, width, height);
    tRect = Rect(CLIP_LEFT, CLIP_TOP, CLIP_RIGHT, globals()->gTrueClipBottom);

    stringRect = Rect(0, 0, width, height);
    stringRect.center_in(tRect);
    tRect = stringRect;
    tRect.left -= 4;
    tRect.top -= 4;
    tRect.right += 4;
    tRect.bottom += 4;

    DrawInSaveWorld();
    DefaultColors();
    CopyRealWorldToSaveWorld(tRect);
    DrawInOffWorld();
    DefaultColors();
    CopySaveWorldToOffWorld(tRect);
    DrawInOffWorld();

    clipRect = tRect;

    GetRGBTranslateColorShade(&color, GREEN, DARKER);
    for ( count = clipRect.top + 2; count < clipRect.bottom; count += 2)
    {
        DrawNateLine( gOffWorld, clipRect, clipRect.left, count, clipRect.right - 1,
                    count, 0, 0, color);
    }

    GetRGBTranslateColorShade(&color, GREEN, LIGHTER);
    DrawNateVBracket( gOffWorld, clipRect, clipRect, 0, 0,color);
    MoveTo( stringRect.left, stringRect.top + mDirectFontAscent());
    DrawDirectTextStringClipped(pauseString, color, gOffWorld, clipRect, 0, 0);

    DrawInRealWorld();
    DefaultColors();
    tRect = clipRect;
    CopyOffWorldToRealWorld(tRect);
}

void StopPauseIndicator(const StringPiece& pauseString) {
    long            width, height;
    Rect            tRect, stringRect;

    mSetDirectFont( kTitleFontNum);
    mGetDirectStringDimensions(pauseString, width, height);
    tRect = Rect(CLIP_LEFT, CLIP_TOP, CLIP_RIGHT, globals()->gTrueClipBottom);

    stringRect = Rect(0, 0, width, height);
    stringRect.center_in(tRect);
    stringRect.left -= 4;
    stringRect.top -= 4;
    stringRect.right += 4;
    stringRect.bottom += 4;

    DrawInOffWorld();
    DefaultColors();
    CopySaveWorldToOffWorld(stringRect);
    DrawInRealWorld();
    CopyOffWorldToRealWorld(stringRect);
    DrawInOffWorld();
    DefaultColors();
    gActiveWorld->view(stringRect).fill(RgbColor::kBlack);

    DrawInRealWorld();
    DefaultColors();
}


//
// BothCommandAndQ:
//  returns true if both the command and q keys are set by player. If this is
//  true, then command-q for quit during a game should be disabled.
//

bool BothCommandAndQ() {
    bool command = false;
    bool q = false;

    for (int i = 0; i < kKeyExtendedControlNum; i++) {
        uint32_t key = globals()->gPreferencesData->key(i);
        q |= (key == Keys::Q);
        command |= (key == Keys::COMMAND);
    }

    return command && q;
}

netResultType StartNetworkGameSetup( void)

{
#ifdef NETSPROCKET_AVAILABLE
    Point                   where;
    int                     error;
    short                   whichItem = -1;
    bool                 done = false;
    EventRecord             theEvent;
    char                    whichChar;
    netResultType           result = kCancel;

    if ( globals()->gameRangerPending)
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
            InvalRect(&gRealWorld->bounds);
            while (!done) {
                WaitNextEvent (everyEvent, &theEvent, 3, nil);
                {
                    whichItem = -1;
                    switch ( theEvent.what )
                    {
                        case osEvt:
//                          HandleOSEvent( &theEvent);
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
                        case kNetSetupCancelButton:
                            done = true;
                            result = kCancel;
                            break;

                        case kNetSetupHostButton:
//                          BlackenWindow();
//                          if ( DoHostGame()) result = kHost;
                            done = true;
                            break;

                        case kNetSetupJoinButton:
//                          BlackenWindow();
//                          if ( DoJoinGameModalDialog()) result = kClient;
                            done = true;
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

netResultType ClientWaitInterface( void)

{
#ifdef NETSPROCKET_AVAILABLE
    Point                   where;
    int                     error;
    short                   whichItem;
    bool                 done = false;
    EventRecord             theEvent;
    char                    whichChar;
    netResultType           result = kCancel;
    long                    theMessage, roundTripTime, version, serialNumerator,
                            serialDenominator;
    Str255                  s;

    if ( globals()->gameRangerPending)
    {
        globals()->gameRangerPending = false;
        globals()->gameRangerInProgress = true;

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

        while (!done) {
            WaitNextEvent (everyEvent, &theEvent, 0, nil);
            {
                whichItem = -1;
                switch ( theEvent.what )
                {
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
                    case kClientWaitCancelButton:
                        done = true;
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
                                        ( serialNumerator != globals()->gSerialNumerator)
                                        ||
                                        ( serialDenominator !=
                                            globals()->gSerialDenominator)
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
                                    kThisVersion, globals()->gSerialNumerator,
                                    globals()->gSerialDenominator, 0);
                                if ( ( serialNumerator == 0) &&
                                            ( serialDenominator == 0))
                                    SetOpponentIsUnregistered( true);
                                else
                                    SetOpponentIsUnregistered( false);
                            } else
                            {
                                SendPreGameVerboseMessage( eClientReadyMessage,
                                    kThisVersion, globals()->gSerialNumerator,
                                    globals()->gSerialDenominator, 0);

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
#ifdef NETSPROCKET_AVAILABLE
    Point                   where;
    int                     error;
    short                   whichItem;
    bool                 done = false;
    EventRecord             theEvent;
    char                    whichChar;
    netResultType           result = kCancel;
    long                    theMessage, roundTripTime, version, serialNumerator,
                            serialDenominator;
    Str31                   s;
    unsigned char*          name;

    if ( globals()->gameRangerPending)
    {
        globals()->gameRangerPending = false;
        globals()->gameRangerInProgress = true;

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
        while (!done) {
            WaitNextEvent (everyEvent, &theEvent, 0, nil);
            {
                whichItem = -1;
                switch ( theEvent.what )
                {
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
                    case kHostCancelButton:
                        done = true;
                        result = kCancel;
                        StopNetworking();
                        break;

                    case kHostAcceptButton:
                        SendPreGameVerboseMessage( eHostAcceptsMessage, kThisVersion,
                            globals()->gSerialNumerator,
                            globals()->gSerialDenominator,
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
                switch( theMessage)
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
                        } else if (( serialNumerator == globals()->gSerialNumerator)
                            && ( serialDenominator == globals()->gSerialDenominator)
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

void BlackenWindow( void)

{
    Rect    tRect;

    tRect = Rect(0, 0, WORLD_WIDTH, WORLD_HEIGHT);
    DrawInSaveWorld();
    DefaultColors();
    gActiveWorld->view(tRect).fill(RgbColor::kBlack);
    DrawInOffWorld();
    DefaultColors();
    gActiveWorld->view(tRect).fill(RgbColor::kBlack);
    DrawInRealWorld();
    DefaultColors();
    gActiveWorld->view(tRect).fill(RgbColor::kBlack);
}

long UpdateMissionBriefPoint( interfaceItemType *dataItem, long whichBriefPoint, long whichScenario,
                    coordPointType *corner, long scale, long mustFit, Rect *bounds,
                    Rect *usedRect, inlinePictType *inlinePict)

{
    Rect            oldRect, newRect, hiliteBounds;
    scoped_ptr<String> textData;
    long            length = 0, headerID, headerNumber, contentID, textlength = 0,
                    i;
    short           textHeight = 0;
    scoped_ptr<Picture> thePict;
    Point           starPoint;
    RgbColor        color;
    Rect        longClipRect, starRect;
    inlinePictType  *thisInlinePict;

#pragma unused( mustFit)

    HideCursor();
    GetAnyInterfaceItemGraphicBounds(*dataItem, &oldRect);
    CopySaveWorldToOffWorld(*usedRect);

    DrawInOffWorld();
//  HHCheckAllHandles();

    if ( whichBriefPoint >=kMissionBriefPointOffset)
    {
        whichBriefPoint -= kMissionBriefPointOffset;

        BriefPoint_Data_Get( whichBriefPoint, whichScenario, &headerID, &headerNumber, &contentID,
                                 &hiliteBounds, corner, scale, 16, 32, bounds);

        // TODO(sfiera): catch exception.
        Resource rsrc('TEXT', contentID);
        textData.reset(new String(rsrc.data(), mac_roman_encoding()));
        if (textData.get() != nil) {
            textlength = length = textData->size();
            textHeight = GetInterfaceTextHeightFromWidth(
                    *textData, dataItem->style, kMissionDataWidth);
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
            FrameRect(gActiveWorld, hiliteBounds);
            SetTranslateColorShadeFore( kMissionDataHiliteColor, MEDIUM);
            GetAnyInterfaceItemGraphicBounds(*dataItem, &newRect);
            if ( dataItem->bounds.right < hiliteBounds.left)
            {
                MoveTo( hiliteBounds.left, hiliteBounds.top);
                MacLineTo(gActiveWorld, newRect.right + kMissionLineHJog, hiliteBounds.top);
                MacLineTo(gActiveWorld, newRect.right + kMissionLineHJog, newRect.top);
                MacLineTo(gActiveWorld, newRect.right + 1, newRect.top);
                MoveTo( hiliteBounds.left, hiliteBounds.bottom - 1);
                MacLineTo(gActiveWorld, newRect.right + kMissionLineHJog, hiliteBounds.bottom - 1);
                MacLineTo(gActiveWorld, newRect.right + kMissionLineHJog, newRect.bottom - 1);
                MacLineTo(gActiveWorld, newRect.right + 1, newRect.bottom - 1);
            } else
            {
                MoveTo( hiliteBounds.right, hiliteBounds.top);
                MacLineTo(gActiveWorld, newRect.left - kMissionLineHJog, hiliteBounds.top);
                MacLineTo(gActiveWorld, newRect.left - kMissionLineHJog, newRect.top);
                MacLineTo(gActiveWorld, newRect.left - 2, newRect.top);
                MoveTo( hiliteBounds.right, hiliteBounds.bottom - 1);
                MacLineTo(gActiveWorld, newRect.left - kMissionLineHJog, hiliteBounds.bottom - 1);
                MacLineTo(gActiveWorld, newRect.left - kMissionLineHJog, newRect.bottom - 1);
                MacLineTo(gActiveWorld, newRect.left - 2, newRect.bottom - 1);
            }

        }
        dataItem->item.labeledRect.label.stringID = headerID;
        dataItem->item.labeledRect.label.stringNumber = headerNumber;
        GetAnyInterfaceItemGraphicBounds(*dataItem, &newRect);
        SetTranslateColorFore(BLACK);
        gActiveWorld->view(newRect).fill(RgbColor::kBlack);
        DrawAnyInterfaceItem(*dataItem, gOffWorld);

        DrawInOffWorld();
        if (textData.get() != nil) {
            newRect = dataItem->bounds;
            DrawInterfaceTextInRect(
                    newRect, *textData, dataItem->style, dataItem->color, gOffWorld, inlinePict);
            textData.reset();
        }

        DrawInRealWorld();
        NormalizeColors();

        GetAnyInterfaceItemGraphicBounds(*dataItem, &newRect);
        newRect.enlarge_to(oldRect);
        newRect.enlarge_to(hiliteBounds);
        oldRect = *usedRect;
        oldRect.enlarge_to(newRect);
        CopyOffWorldToRealWorld(*bounds);
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
            thePict.reset(new Picture(kMissionStarMapPictID));
            if (thePict.get() != nil) {
                newRect = thePict->bounds();
                newRect.center_in(*bounds);
                CopyBits(thePict.get(), gActiveWorld, thePict->bounds(), newRect);
                thePict.reset();
                GetScenarioStarMapPoint( whichScenario, &starPoint);
                starPoint.h += bounds->left;
                starPoint.v += bounds->top;

                GetRGBTranslateColorShade(&color, GOLD, VERY_LIGHT);
                longClipRect = *bounds;
                starRect.left = starPoint.h - kMissionStarPointWidth;
                starRect.top = starPoint.v - kMissionStarPointHeight;
                starRect.right = starPoint.h + kMissionStarPointWidth;
                starRect.bottom = starPoint.v + kMissionStarPointHeight;

                DrawNateVBracket( gOffWorld, starRect, longClipRect, 0, 0,color);
                DrawNateLine( gOffWorld, longClipRect, starPoint.h,
                            starPoint.v + kMissionStarPointHeight,
                            starPoint.h,
                            bounds->bottom, 0, 0, color);
                DrawNateLine( gOffWorld, longClipRect, starPoint.h,
                            starPoint.v - kMissionStarPointHeight,
                            starPoint.h,
                            bounds->top, 0, 0, color);
                DrawNateLine( gOffWorld, longClipRect, starPoint.h - kMissionStarPointWidth,
                            starPoint.v,
                            bounds->left,
                            starPoint.v, 0, 0, color);
                DrawNateLine( gOffWorld, longClipRect, starPoint.h + kMissionStarPointWidth,
                            starPoint.v,
                            bounds->right,
                            starPoint.v, 0, 0, color);

                oldRect = *usedRect;
                oldRect.enlarge_to(*bounds);
                DrawInRealWorld();
                NormalizeColors();
                CopyOffWorldToRealWorld(*bounds);
                *usedRect = newRect;
                textlength = 100;
            }
        } else if ( whichBriefPoint == kMissionBlankBriefNum)
        {
            oldRect = *bounds;
            DrawInRealWorld();
            NormalizeColors();
            CopyOffWorldToRealWorld(*bounds);
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
    Rect        lRect, longClipRect;
    baseObjectType  *baseObject = gBaseObjectData.get();// + (pictID - kFirstShipDataPictID);
    retroTextSpecType   retroTextSpec;
    long            height, waitTime, i;

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

        retroTextSpec.text.reset(new String);
        if ( retroTextSpec.text.get() != nil) {
            CreateObjectDataText(retroTextSpec.text.get(), i);
            retroTextSpec.textLength = retroTextSpec.text->size();

            mSetDirectFont( kButtonFontNum);
            retroTextSpec.thisPosition = retroTextSpec.linePosition = retroTextSpec.lineCount = 0;
            retroTextSpec.tabSize = 100;
            GetRGBTranslateColorShade(&retroTextSpec.color, GREEN, VERY_LIGHT);
            GetRGBTranslateColorShade(&retroTextSpec.backColor, GREEN, DARKEST);
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
                dataRect.offset(clipRect->left - dataRect.left + 1, 0);
            } else if ( dataRect.right > clipRect->right)
            {
                dataRect.offset(clipRect->right - dataRect.right - 1, 0);
            }

            if ( dataRect.top < clipRect->top)
            {
                dataRect.offset(0, clipRect->top - dataRect.top + 1);
            } else if ( dataRect.bottom > clipRect->bottom)
            {
                dataRect.offset(0, clipRect->bottom - dataRect.bottom - 1);
            }
            retroTextSpec.xpos = dataRect.left;
            retroTextSpec.ypos = dataRect.top + mDirectFontAscent();

    //      clipRect.left = dataRect.left;
    //      clipRect.right = dataRect.right;
    //      clipRect.top = dataRect.top;
    //      clipRect.bottom = dataRect.bottom;
            longClipRect = *clipRect;
            lRect = dataRect;
            DrawInRealWorld();
            NormalizeColors();
            dataRect.inset(-8, -4);
            gActiveWorld->view(dataRect).fill(RgbColor::kBlack);
            SetTranslateColorShadeFore( GREEN, VERY_LIGHT);
            FrameRect(gActiveWorld, dataRect);
            NormalizeColors();


            while (( retroTextSpec.thisPosition < retroTextSpec.textLength) && (( Button()) || (AnyRealKeyDown())))
            {
                PlayVolumeSound(  kComputerBeep3, kMediumLowVolume, kShortPersistence, kLowPrioritySound);
                DrawRetroTextCharInRect( &retroTextSpec, 24, lRect, lRect, gActiveWorld, gNatePortLeft,
                    gNatePortTop);

                waitTime = TickCount();
                while (( TickCount() - waitTime) < 3) {
                    // DO NOTHING
                };
            }

            retroTextSpec.text.reset();
        }

        MacShowCursor();
        while (( Button()) || (AnyRealKeyDown())) {
            // DO NOTHING
        };

        CopyOffWorldToRealWorld(dataRect);
    }
}

void CreateObjectDataText(String* text, short id) {
    Resource rsrc('TEXT', kShipDataTextID);
    String data(rsrc.data(), mac_roman_encoding());

    const baseObjectType& baseObject = gBaseObjectData.get()[id];

    StringList keys(kShipDataKeyStringID);
    StringList values(kShipDataNameID);

    // *** Replace place-holders in text with real data, using the fabulous Munger routine
    // an object or a ship?
    if ( baseObject.attributes & kCanThink) {
        const StringPiece& name = values.at(0);
        Munger(&data, 0, keys.at(kShipOrObjectStringNum), name);
    } else {
        const StringPiece& name = values.at(1);
        Munger(&data, 0, keys.at(kShipOrObjectStringNum), name);
    }

    // ship name
    {
        StringList names(5000);
        const StringPiece& name = names.at(id);
        Munger(&data, 0, keys.at(kShipTypeStringNum), name);
    }

    // ship mass
    Munger(&data, 0, keys.at(kMassStringNum), small_fixed(baseObject.mass));

    // ship shields
    Munger(&data, 0, keys.at(kShieldStringNum), baseObject.health);

    // light speed
    Munger(&data, 0, keys.at(kHasLightStringNum), baseObject.warpSpeed);

    // max velocity
    Munger(&data, 0, keys.at(kMaxSpeedStringNum), small_fixed(baseObject.maxVelocity));

    // thrust
    Munger(&data, 0, keys.at(kThrustStringNum), small_fixed(baseObject.maxThrust));

    // par turn
    Munger(&data, 0, keys.at(kTurnStringNum),
            small_fixed(baseObject.frame.rotation.turnAcceleration));

    // now, check for weapons!
    CreateWeaponDataText(&data, baseObject.pulse, values.at(kShipDataPulseStringNum));
    CreateWeaponDataText(&data, baseObject.beam, values.at(kShipDataBeamStringNum));
    CreateWeaponDataText(&data, baseObject.special, values.at(kShipDataSpecialStringNum));

    text->append(data);
}

void CreateWeaponDataText(String* text, long whichWeapon, const StringPiece& weaponName) {
    baseObjectType      *weaponObject, *missileObject;
    long                mostDamage, actionNum;
    objectActionType    *action;
    bool             isGuided = false;

    if (whichWeapon == kNoShip) {
        return;
    }

    weaponObject = gBaseObjectData.get() + whichWeapon;

    // TODO(sfiera): catch exception.
    Resource rsrc('TEXT', kWeaponDataTextID);
    String data(rsrc.data(), mac_roman_encoding());
    // damage; this is tricky--we have to guess by walking through activate actions,
    //  and for all the createObject actions, see which creates the most damaging
    //  object.  We calc this first so we can use isGuided

    mostDamage = 0;
    isGuided = false;
    if ( weaponObject->activateActionNum > 0)
    {
        action = gObjectActionData.get() + weaponObject->activateAction;
        for ( actionNum = 0; actionNum < weaponObject->activateActionNum; actionNum++)
        {
            if (( action->verb == kCreateObject) || ( action->verb == kCreateObjectSetDest))
            {
                missileObject = gBaseObjectData.get() +
                    action->argument.createObject.whichBaseType;
                if ( missileObject->attributes & kIsGuided) isGuided = true;
                if ( missileObject->damage > mostDamage) mostDamage = missileObject->damage;
            }
            action++;
        }
    }

    StringList keys(kShipDataKeyStringID);
    StringList values(kShipDataNameID);

    // weapon name #
    Munger(&data, 0, keys.at(kWeaponNumberStringNum), weaponName);

    // weapon name
    {
        StringList names(5000);
        const StringPiece& name = names.at(whichWeapon);
        Munger(&data, 0, keys.at(kWeaponNameStringNum), name);
    }

    const StringPiece& yes = values.at(kShipDataYesStringNum);
    const StringPiece& no = values.at(kShipDataNoStringNum);
    const StringPiece& dash = values.at(kShipDataDashStringNum);

    // is guided
    if (isGuided) {
        Munger(&data, 0, keys.at(kWeaponGuidedStringNum), yes);
    } else {
        Munger(&data, 0, keys.at(kWeaponGuidedStringNum), no);
    }

    // is autotarget
    if (weaponObject->attributes & kAutoTarget) {
        Munger(&data, 0, keys.at(kWeaponAutoTargetStringNum), yes);
    } else {
        Munger(&data, 0, keys.at(kWeaponAutoTargetStringNum), no);
    }

    // range
    Munger(&data, 0, keys.at(kWeaponRangeStringNum),
            lsqrt(weaponObject->frame.weapon.range));

    if (mostDamage > 0) {
        Munger(&data, 0, keys.at(kWeaponDamageStringNum), mostDamage);
    } else {
        Munger(&data, 0, keys.at(kWeaponDamageStringNum), dash);
    }
    text->append(data);
}

#define kBackground_Height  480
#define kScrollText_Buffer  10

void Replace_KeyCode_Strings_With_Actual_Key_Names(String* text, short resID, size_t padTo) {
    StringList keys(kHelpScreenKeyStringID);
    StringList values(resID);

    for (int i = 0; i < kKeyExtendedControlNum; ++i) {
        const StringPiece& search = keys.at(i);
        String replace(values.at(globals()->gPreferencesData->key(i) - 1));
        if (replace.size() < padTo) {
            replace.resize(padTo, ' ');
        }
        while (Munger(text, 0, search, replace) > 0) {
            // DO NOTHING
        };
    }
}

}  // namespace antares
