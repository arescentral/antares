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
#include "KeySetupScreen.hpp"
#include "MessageScreen.hpp"
#include "Music.hpp"
#include "NatePixTable.hpp"
#include "NetSetupScreen.hpp"
#include "OffscreenGWorld.hpp"
#include "Options.hpp"
#include "Picture.hpp"
#include "Races.hpp"
#include "ScenarioMaker.hpp"
#include "ScrollStars.hpp"
#include "SoundFX.hpp"               // for button on/off
#include "StringHandling.hpp"
#include "StringNumerics.hpp"
#include "Transitions.hpp"
#include "VideoDriver.hpp"

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

bool IsKeyReserved( KeyMap, bool);
void BlackenOffscreen( void);
void Pause( long);

void DoLoadingInterface(Rect *contentRect, unsigned char* levelName) {
    int                     error;
    unsigned char           color, *strPtr;
    transColorType          *transColor;
    Rect                lRect, clipRect, boundsRect;
    Rect                    tRect;
    retroTextSpecType       retroTextSpec;
    long                    height;

    BlackenWindow();

    error = OpenInterface( kLoadingScreenID);
    if ( error == kNoError)
    {
        DrawEntireInterface();

        GetAnyInterfaceItemContentBounds( GetAnyInterfaceItemPtr( 0), contentRect); // item 0 = loading rect
        CloseInterface();

// it is assumed that we're "recovering" from a fade-out
        AutoFadeFrom( 10, false);

        DrawInRealWorld();
        mSetDirectFont( kTitleFontNum);
        mGetTranslateColorShade( PALE_GREEN, LIGHT, color, transColor);
        lRect.left = 0;
        lRect.top = 0;
        lRect.right = WORLD_WIDTH;
        lRect.bottom = WORLD_HEIGHT;
//      MoveTo( contentRect->left + (( contentRect->right - contentRect->left) / 2) - (stringWidth / 2),
//              contentRect->top);

        strPtr = levelName + 1;
        retroTextSpec.textLength = *levelName;
        retroTextSpec.text.reset(new std::string);
        for (int i = 0; i < retroTextSpec.textLength; ++i) {
            (*retroTextSpec.text)[i] = strPtr[i];
        }

        retroTextSpec.thisPosition = retroTextSpec.linePosition = retroTextSpec.lineCount = 0;
        retroTextSpec.tabSize =220;
        mGetTranslateColorShade( PALE_GREEN, VERY_LIGHT, retroTextSpec.color, transColor);
        mGetTranslateColorShade( SKY_BLUE, DARKEST, retroTextSpec.backColor, transColor);
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
    unsigned char   color;
    long            width, height, temp;
    transColorType  *transColor;
    Rect        clipRect;
    Rect            tRect;
    RGBColor        fadeColor = {0, 0, 0};
    Str255          string;

    Button();  // Hack to get it to update.
    if ( total < 0)
    {
        DrawInOffWorld();
        DefaultColors();
        PaintRect(*contentRect);
        GetIndString( string, 2004, 33);

        mSetDirectFont( kButtonFontNum);
        mGetDirectStringDimensions(string, width, height);

        clipRect = *contentRect;
        tRect = Rect(0, 0, width, height);
        tRect.center_in(*contentRect);

        mGetTranslateColorShade( kLoadingScreenColor, LIGHTER, color, transColor);
        MoveTo( tRect.left, tRect.top + mDirectFontAscent());
        DrawDirectTextStringClipped( string, color, gOffWorld, clipRect, 0, 0);


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
        PaintRect(tRect);

        tRect = Rect(contentRect->left + temp, contentRect->top, contentRect->right, contentRect->bottom);
        SetTranslateColorShadeFore( kLoadingScreenColor, DARK);
        PaintRect(tRect);
        NormalizeColors();
        DrawInRealWorld();
        NormalizeColors();
        tRect = Rect(contentRect->left, contentRect->top, contentRect->right, contentRect->bottom);
        CopyOffWorldToRealWorld(tRect);
        if ( tRect.left >= tRect.right - 2) AutoFadeTo( 10, &fadeColor, false);
    }
}

PlayAgainResult DoPlayAgain(bool allowResume, bool allowSkip) {
    int                     error = kNoError;
    Rect                    tRect;
    interfaceItemType       *item;
    bool                 done = false;
    Point                   where;
    short                   whichItem;
    PlayAgainResult         result = PLAY_AGAIN_QUIT;
    EventRecord             theEvent;
    char                    whichChar;

//  BlackenWindow();

    FlushEvents(everyEvent, 0);
    if ( allowSkip) error = OpenInterface( 5017);
    else if ( allowResume) error = OpenInterface( kPlayAgainResumeID);
    else error = OpenInterface( kPlayAgainID);
    if ( error == kNoError)
    {
        tRect = Rect(CLIP_LEFT, CLIP_TOP, CLIP_RIGHT, CLIP_BOTTOM);
        CenterAllItemsInRect( &tRect);
        item = GetAnyInterfaceItemPtr( kPlayAgainBox);
        DrawInOffWorld();
//      LongRectToRect( &(item->bounds), &tRect);
        GetAnyInterfaceItemGraphicBounds( item, &tRect);
        DefaultColors();
        PaintRect(tRect);
        DrawInRealWorld();

        if ( globals()->gOptions & kOptionNetworkOn)
        {
            SetStatusOfAnyInterfaceItem( kPlayAgainOKButton, kDimmed, false);
        }
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
                    case kPlayAgainOKButton:
                        done = true;
                        result = PLAY_AGAIN_RESTART;
                        break;

                    case kPlayAgainNoButton:
                        done = true;
                        result = PLAY_AGAIN_QUIT;
                        break;

                    case kPlayAgainResumeButton:
                        done = true;
                        result = PLAY_AGAIN_RESUME;
                        break;

                    case 5:
                        done = true;
                        result = PLAY_AGAIN_SKIP;
                        break;
                }

            }
        }
        DrawInOffWorld();
        GetAnyInterfaceItemGraphicBounds( item, &tRect);
        DefaultColors();
        PaintRect(tRect);
        DrawInRealWorld();
        CloseInterface();
    }
    return result;
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
        }
    }
#endif NETSPROCKET_AVAILABLE
}

void DoHelpScreen( void)

{
    int                     error = kNoError;
    Rect                    tRect, textRect;
    interfaceItemType       *item;
    bool                 done = false;
    Point                   where;
    short                   whichItem;
    EventRecord             theEvent;
    char                    whichChar;
    Rect                clipRect, boundsRect;
    long                    height;
    retroTextSpecType       retroTextSpec;
    transColorType          *transColor;

    FlushEvents(everyEvent, 0);
    if ( globals()->gOptions & kOptionSubstituteFKeys)
        error = OpenInterface( kNOFHelpScreenID);
    else
        error = OpenInterface( kHelpScreenID);
    if ( error == kNoError)
    {
        tRect = Rect(CLIP_LEFT, CLIP_TOP, CLIP_RIGHT, globals()->gTrueClipBottom);
        CenterAllItemsInRect( &tRect);
        item = GetAnyInterfaceItemPtr( kHelpScreenBox);
        DrawInOffWorld();
        GetAnyInterfaceItemGraphicBounds( item, &tRect);
        textRect = item->bounds;
        DefaultColors();
        CopyOffWorldToSaveWorld(tRect);
        PaintRect(tRect);
        DrawInRealWorld();

        DrawAllItemsOfKind( kPictureRect, false, false, false);
        DrawAllItemsOfKind( kLabeledRect, true, false, false);
        DrawAllItemsOfKind( kPlainRect, true, false, false);
        DrawAllItemsOfKind( kCheckboxButton, true, false, false);
        DrawAllItemsOfKind( kRadioButton, true, false, false);
        DrawAllItemsOfKind( kPlainButton, true, false, false);
        DrawAllItemsOfKind( kTextRect, true, false, false);

        if ( globals()->gOptions & kOptionSubstituteFKeys) {
            retroTextSpec.text.reset(
                    new std::string(Resource::get_data('TEXT', kNOFHelpScreenTextID)));
        } else {
            retroTextSpec.text.reset(
                    new std::string(Resource::get_data('TEXT', kHelpScreenTextID)));
        }

        if (retroTextSpec.text.get() != nil) {
            Replace_KeyCode_Strings_With_Actual_Key_Names(
                    retroTextSpec.text.get(), kKeyMapNameID, 4);

            retroTextSpec.textLength = retroTextSpec.text->size();

            mSetDirectFont( kComputerFontNum);
            retroTextSpec.thisPosition = retroTextSpec.linePosition = retroTextSpec.lineCount = 0;
            retroTextSpec.tabSize = 220;
            mGetTranslateColorShade( RED, VERY_LIGHT, retroTextSpec.color, transColor);
            mGetTranslateColorShade( RED, VERY_DARK, retroTextSpec.backColor, transColor);
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
            retroTextSpec.ypos = boundsRect.top + mDirectFontAscent();

            clipRect.left = 0;
            clipRect.right = clipRect.left + WORLD_WIDTH;
            clipRect.top = 0;
            clipRect.bottom = clipRect.top + WORLD_HEIGHT;
            clipRect = textRect;

            DrawDirectTextInRect(&retroTextSpec, clipRect, clipRect, gOffWorld, 0, 0);
            CopyOffWorldToRealWorld(tRect);

            retroTextSpec.text.reset();
        }

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
                    case kHelpScreenDoneButton:
                        done = true;
                        break;

                }

            }
        }
        DrawInOffWorld();
        GetAnyInterfaceItemGraphicBounds( item, &tRect);
        DefaultColors();
        PaintRect(tRect);
        CopySaveWorldToOffWorld(tRect);
        DrawInRealWorld();
        CloseInterface();
    }
}

void StartPauseIndicator(unsigned char* pauseString, unsigned char hue) {
    unsigned char   color;
    long            width, height, count;
    Rect            tRect, stringRect;
    transColorType  *transColor;
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

    mGetTranslateColorShade( GREEN, DARKER, color, transColor);
    for ( count = clipRect.top + 2; count < clipRect.bottom; count += 2)
    {
        DrawNateLine( gOffWorld, clipRect, clipRect.left, count, clipRect.right - 1,
                    count, 0, 0, color);
    }

    mGetTranslateColorShade( GREEN, LIGHTER, color, transColor);
    DrawNateVBracket( gOffWorld, clipRect, clipRect, 0, 0,color);
    MoveTo( stringRect.left, stringRect.top + mDirectFontAscent());
    DrawDirectTextStringClipped( pauseString, color, gOffWorld, clipRect, 0, 0);

    DrawInRealWorld();
    DefaultColors();
    tRect = clipRect;
    CopyOffWorldToRealWorld(tRect);
}

void StopPauseIndicator(unsigned char* pauseString) {
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
    PaintRect(stringRect);

    DrawInRealWorld();
    DefaultColors();
}


void DrawInterfaceOneAtATime( void)

{
        DrawAllItemsOfKind( kPictureRect, false, true, true);
        DrawAllItemsOfKind( kLabeledRect, true, false, false);
        DrawAllItemsOfKind( kTabBox, true, false, false);
        DrawAllItemsOfKind( kTabBoxButton, true, false, false);
        DrawAllItemsOfKind( kPlainRect, true, false, false);
        DrawAllItemsOfKind( kCheckboxButton, true, false, false);
        DrawAllItemsOfKind( kRadioButton, true, false, false);
        DrawAllItemsOfKind( kPlainButton, true, false, false);
        DrawAllItemsOfKind( kTextRect, true, false, false);
}

void DoOptionsInterface( void)

{
    Point                   where;
    int                     error;
    short                   whichItem;
    bool                 done = false, cancel = false;
    EventRecord             theEvent;
    char                    whichChar;
    scoped_ptr<Preferences> tempPrefs(new Preferences(*globals()->gPreferencesData));
    Preferences*    prefsData = globals()->gPreferencesData.get();
    Rect                    volumeRect;

    BlackenWindow();

    FlushEvents(everyEvent, 0);

    error = OpenInterface( kOptionsScreenID);
    SetOptionCheckboxes( prefsData->options);
    if ( !(globals()->gOptions & kOptionSpeechAvailable)) SetStatusOfAnyInterfaceItem( kOptionSpeechOnButton, kDimmed, false);
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
                    case kOptionCancelButton:
                        done = true;
                        cancel = true;
                        break;

                    case kOptionDoneButton:
                        done = true;
                        break;

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
                            DrawOptionVolumeLevel( &volumeRect, prefsData->volume);
                            globals()->gSoundVolume = prefsData->volume;
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
                            DrawOptionVolumeLevel( &volumeRect, prefsData->volume);
                            globals()->gSoundVolume = prefsData->volume;
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
                }

            }
        }
        if ( !cancel)
        {
            SaveAllPreferences(); // sets globals()->gOptions
        } else
        {
            *globals()->gPreferencesData = *tempPrefs;
            if ( globals()->gSoundVolume != prefsData->volume)
            {
                globals()->gSoundVolume = prefsData->volume;
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

    tRect = Rect(notchBounds.left, notchBounds.top,
          notchBounds.left + notchWidth - 2,
        notchBounds.bottom);

    shade = 2;
    for ( count = 0; count < level; count++)
    {
        SetTranslateColorShadeFore( kOptionVolumeColor, shade);
        graphicRect = tRect;
        graphicRect.inset(2, 6);
        graphicRect.center_in(tRect);
        PaintRect(graphicRect);
        tRect.left += notchWidth;
        tRect.right = tRect.left + notchWidth - 2;
        shade += 2;
    }

    NormalizeColors();
    for ( count = level; count < ( kMaxVolumePreference); count++)
    {
        graphicRect = tRect;
        graphicRect.inset(2, 6);
        graphicRect.center_in(tRect);
        PaintRect(graphicRect);
        tRect.left += notchWidth;
        tRect.right = tRect.left + notchWidth - 2;
    }

    DrawInRealWorld();
    NormalizeColors();
    CopyOffWorldToRealWorld(*bounds);
}

bool DoKeyInterface( void)

{
    Point                   where;
    int                     error;
    short                   whichItem, i, whichKeyButton = -1, keyNum = 0, currentKey = 0, checkKey;
    bool                 done = false, result = true, cancel = false;
    EventRecord             theEvent;
    KeyMap                  keyMap;
    Preferences     *prefsData = nil;
    unsigned long           options = globals()->gOptions;

    BlackenOffscreen();

    FlushEvents(everyEvent, 0);
    error = OpenInterface( kKeyScreenID);
    if ( error == kNoError)
    {
        for ( i = 0; i < kKeyControlNum; i++)
        {
            SetButtonKeyNum( i,
                GetKeyNumFromKeyMap( globals()->gKeyControl[i]));
        }

        prefsData = globals()->gPreferencesData.get();

        SwitchAnyRadioOrCheckbox( kKeySubstituteCheckbox,
            ((options & kOptionSubstituteFKeys) ? (true):(false)));

        DrawInterfaceOneAtATime();
        while (!done) {
            if (( AnyEvent()) && ( !( globals()->gOptions & kOptionInBackground)))
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
                    SetStatusOfAnyInterfaceItem( checkKey, kIH_Hilite, true);
                    Pause( 12);
                    SetStatusOfAnyInterfaceItem( checkKey, kActive, true);
                    Pause( 12);
                    SetStatusOfAnyInterfaceItem( checkKey, kIH_Hilite, true);
                    Pause( 12);
                    while ( AnyEvent()) {
                        // DO NOTHING
                    };
                    SetStatusOfAnyInterfaceItem( checkKey, kActive, true);
                }

                if (( keyNum > 0) && (whichKeyButton >= 0) && (whichKeyButton < kKeyControlNum))
                {
                    mPlayScreenSound;
                    SetButtonKeyNum( whichKeyButton, keyNum);
                    SetStatusOfAnyInterfaceItem( whichKeyButton, kIH_Hilite, true);
                    do
                    {
                        GetKeys( keyMap);
                        currentKey = GetKeyNumFromKeyMap( keyMap);
                    } while ( currentKey > 0);

                    SetStatusOfAnyInterfaceItem( whichKeyButton, kActive, true);
                    whichKeyButton++;
                    if ( whichKeyButton >= kKeyControlNum) whichKeyButton = 0;
                    SetStatusOfAnyInterfaceItem( whichKeyButton, kIH_Hilite, true);
                    DrawKeyControlPicture( whichKeyButton);
                }
                keyNum = currentKey = 0;
            }
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
//                      whichChar = theEvent.message & charCodeMask;
//                      whichItem = InterfaceKeyDown( theEvent.message);
                        break;
                }

                if (( whichItem >= 0) &&  (whichItem < kKeyControlNum))
                {
                    if (( whichKeyButton >= 0) &&  (whichKeyButton < kKeyControlNum))
                        SetStatusOfAnyInterfaceItem( whichKeyButton, kActive, true);
                    SetStatusOfAnyInterfaceItem( whichItem, kIH_Hilite, true);
                    whichKeyButton = whichItem;
                    DrawKeyControlPicture( whichKeyButton);
                }

                switch ( whichItem)
                {
                    case kKeyCancelButton:
                        cancel = true;
                        done = true;
                        break;
                    case kKeyDoneButton:
                        done = true;
                        break;
                    case kKeyOptionButton:
                        result = false;
                        done = true;
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
                            SetStatusOfAnyInterfaceItem( checkKey, kIH_Hilite, true);
                            Pause( 12);
                            SetStatusOfAnyInterfaceItem( checkKey, kActive, true);
                            Pause( 12);
                            SetStatusOfAnyInterfaceItem( checkKey, kIH_Hilite, true);
                            Pause( 12);
                            while ( AnyEvent()) {
                                // DO NOTHING
                            };
                            if ( checkKey != whichKeyButton)
                                SetStatusOfAnyInterfaceItem( checkKey, kActive, true);
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
                GetKeyMapFromKeyNum( GetButtonKeyNum( i), globals()->gKeyControl[i]);
            }
            prefsData->options = ( prefsData->options & ~kOptionSubstituteFKeys) |
                                ( options & kOptionSubstituteFKeys);
            SaveKeyControlPreferences();
            SaveAllPreferences();
        }
        CloseInterface();
    }
    WriteDebugLine("\pRESULT:");
    WriteDebugLong( result);
    return( result);
}

//
// BothCommandAndQ:
//  returns true if both the command and q keys are set by player. If this is
//  true, then command-q for quit during a game should be disabled.
//

bool BothCommandAndQ( void)
{
    bool command = false, q = false;
    short   b;

    for ( b = 0; b < kKeyExtendedControlNum; b++)
    {
        if ( mQKey( globals()->gKeyControl[b])) q = true;
        if ( mCommandKey( globals()->gKeyControl[b])) command = true;
    }

    if (( q) && ( command)) return ( true);
    else return( false);
}

bool IsKeyReserved( KeyMap keyMap, bool alternateFKey)
{
#pragma unused( alternateFKey)

    // not related to fkey
    if ( mPauseKey( keyMap)) return( true);
    if ( mEnterTextKey( keyMap)) return( true);
    if ( mRestartResumeKey( keyMap)) return( true);
    return( false);
}

void DrawKeyControlPicture( long whichKey)
{
    Rect    tRect, newRect;
    scoped_ptr<Picture> thePict;

    GetAnyInterfaceItemContentBounds( GetAnyInterfaceItemPtr( kKeyIllustrationBox), &tRect);

    DrawInOffWorld();

    thePict.reset(new Picture(kKeyIllustrationPictID));
    if (thePict.get() != nil) {
        newRect = thePict->bounds();
        newRect.center_in(tRect);
        CopyBits(thePict.get(), gActiveWorld, thePict->bounds(), newRect);
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

    thePict.reset(new Picture(kKeyIllustrationPictID + 1 + whichKey));
    if (thePict.get() != nil) {
        newRect = thePict->bounds();
        newRect.center_in(tRect);
        CopyBits(thePict.get(), gActiveWorld, thePict->bounds(), newRect);
    }
    thePict.reset();

    DrawInRealWorld();
    CopyOffWorldToRealWorld(tRect);

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

void DrawStringInInterfaceItem( long whichItem, const unsigned char* string)
{
    Rect                tRect;
    interfaceItemType   *anItem;

    DrawInOffWorld();
    DefaultColors();
    GetAnyInterfaceItemContentBounds( GetAnyInterfaceItemPtr( whichItem), &tRect);

    PaintRect(tRect);
    anItem = GetAnyInterfaceItemPtr( whichItem);
    if ( string != nil)
    {
        DrawInterfaceTextInRect(tRect, string + 1, string[0],
                                anItem->style, anItem->color, gOffWorld,
                                0, 0, nil);
    }
    DrawInRealWorld();
    CopyOffWorldToRealWorld(tRect);
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
    PaintRect(tRect);
    DrawInOffWorld();
    DefaultColors();
    PaintRect(tRect);
    DrawInRealWorld();
    DefaultColors();
    PaintRect(tRect);
}

void BlackenOffscreen( void)

{
    Rect    tRect;

    tRect = Rect(0, 0, WORLD_WIDTH, WORLD_HEIGHT);
    DrawInSaveWorld();
    DefaultColors();
    PaintRect(tRect);
    DrawInOffWorld();
    DefaultColors();
    PaintRect(tRect);
    DrawInRealWorld();
    DefaultColors();
}


bool IsThisInGameHilited( short which, bool set)

{
#pragma unused( which, set)
    return ( false);
}

long DoSelectLevelInterface( long startChapter)

{
    Point                   where;
    int                     error, x;
    short                   whichItem;
    bool                 done = false, enteringCheat = false;
    EventRecord             theEvent;
    char                    whichChar;
    interfaceItemType       *anItem;
    Rect                        totalRect;
    Str255                  chapterName, cheatString;
    long                    thisLevel = GetScenarioNumberFromChapterNumber(
                                        startChapter),
                            thisChapter = startChapter,
                            lastChapter = startChapter;

    cheatString[0] = 0;

    if ( ThisChapterIsNetworkable( thisChapter)) thisChapter = 1;
    // Old code would now revert to factory scenarios now if still networkable.

    BlackenWindow();
    FlushEvents(everyEvent, 0);
    error = OpenInterface( kSelectLevelID);
    totalRect = Rect(0, 0, WORLD_WIDTH, WORLD_HEIGHT);
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
        AutoFadeFrom( 60, false);
        MacShowCursor();
        while (!done) {
            VideoDriver::driver()->set_game_state(SELECT_LEVEL_INTERFACE);
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
                        }
                        break;

                    case kSelectLevelOKButton:
                        done = true;
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

void DrawLevelNameInBox(unsigned char* name, long fontNum, short descriptionTextID,
    long itemNum) {
    Rect                clipRect;
    Rect                    tRect;
    unsigned char           *strPtr;
    retroTextSpecType       retroTextSpec;
    transColorType          *transColor;
    interfaceItemType       *anItem;
    long                    height;
    scoped_ptr<std::string> textData;

    anItem = GetAnyInterfaceItemPtr( itemNum);
    strPtr = name + 1;

    assert(descriptionTextID <= 0);

    retroTextSpec.textLength = name[0];
    retroTextSpec.text.reset(new std::string);
    for (int i = 1; i < name[0] + 1; ++i) {
        *retroTextSpec.text += name[i];
    }

    retroTextSpec.thisPosition = retroTextSpec.linePosition = retroTextSpec.lineCount = 0;
    retroTextSpec.tabSize =220;
    mGetTranslateColorShade( AQUA, VERY_LIGHT, retroTextSpec.color, transColor);
    mGetTranslateColorShade( AQUA, DARKEST, retroTextSpec.backColor, transColor);
    retroTextSpec.backColor = 0xff;
    retroTextSpec.originalColor = retroTextSpec.nextColor = retroTextSpec.color;
    retroTextSpec.originalBackColor = retroTextSpec.nextBackColor = retroTextSpec.backColor;
    retroTextSpec.topBuffer = 2;
    retroTextSpec.bottomBuffer = 0;

    mSetDirectFont( fontNum);
    height = DetermineDirectTextHeightInWidth( &retroTextSpec, anItem->bounds.right - anItem->bounds.left);

    retroTextSpec.xpos = anItem->bounds.left;
    retroTextSpec.ypos = anItem->bounds.left + mDirectFontAscent();

//  clipRect.left = 0;
//  clipRect.right = clipRect.left + WORLD_WIDTH;
//  clipRect.top = 0;
//  clipRect.bottom = clipRect.top + WORLD_HEIGHT;
    clipRect = anItem->bounds;
    tRect = anItem->bounds;
    DrawInOffWorld();
    DefaultColors();
    PaintRect(tRect);
    DrawInRealWorld();
    DrawDirectTextInRect( &retroTextSpec, anItem->bounds, clipRect, gOffWorld, 0,0);
    CopyOffWorldToRealWorld(tRect);
    retroTextSpec.text.reset();
}

bool DoMissionInterface( long whichScenario)

{
    Point                   where, dataPoint;
    int                     error, x, y;
    short                   whichItem, i;
    bool                 done = false, cancel = false;
    EventRecord             theEvent;
    char                    whichChar;
    interfaceItemType       dataItem;
    long                    length, scale, mustFit = 0, whichBriefPoint = 0, nextStartTime = TickCount(),
                            thisMissionWaitTime = 0;
    Rect                    tRect, mapRect, totalRect;
    coordPointType          corner;
    inlinePictType          inlinePict[kMaxInlinePictNum];

    FlushEvents(everyEvent, 0);
    if ( GetBriefPointNumber( whichScenario) < 1) return true;

    error = OpenInterface( kMissionBriefingScreenID);
//  HHCheckAllHandles();
    totalRect = Rect(0, 0, WORLD_WIDTH, WORLD_HEIGHT);


    if ( error == kNoError)
    {
//      DrawEntireInterface();
        HideCursor();
        SetStatusOfAnyInterfaceItem( kMissionPreviousButton, kDimmed, false);
        DrawInterfaceOneAtATime();
        CopyOffWorldToSaveWorld(totalRect);
// it is assumed that we're "recovering" from a fade-out
        AutoFadeFrom( 60, false);

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

        CopySaveWorldToOffWorld(totalRect);
        CopyOffWorldToRealWorld(totalRect);

        GetScenarioFullScaleAndCorner( whichScenario, 0, &corner, &scale, &mapRect);
        DrawArbitrarySectorLines( &corner, scale, 16, &mapRect, gSaveWorld,  0, 0);
//      HHCheckAllHandles();
        Briefing_Objects_Render( whichScenario, gSaveWorld, 32,
            &mapRect, 0, 0, &corner,
            scale);
//      HHCheckAllHandles();

//      HHCheckAllHandles();
//      GetBriefPointBounds( whichBriefPoint, &corner, scale, 0, &tRect);

        tRect.left = ( mapRect.right - mapRect.left) / 2 - ( kMissionDataWidth / 2) + mapRect.left;
        tRect.right = tRect.left + kMissionDataWidth;
        tRect.top = ( mapRect.bottom - mapRect.top) / 2 - ( kMissionDataWidth / 2) + mapRect.top;
        tRect.bottom = tRect.top + kMissionDataWidth;

        dataItem.bounds = tRect;
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

        nextStartTime = TickCount();

        while (!done) {
//          if (Ares_WaitNextEvent (everyEvent, &theEvent, 0, nil))
            VideoDriver::driver()->set_game_state(MISSION_INTERFACE);
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
                        for ( i = 0; i < kMaxInlinePictNum; i++)
                        {
                            if ((inlinePict[i].id >= 0) &&
                                    inlinePict[i].bounds.contains(where)) {
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
                        done = true;
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
    scoped_ptr<std::string> textData;
    long            length = 0, headerID, headerNumber, contentID, textlength = 0,
                    i;
    short           textHeight = 0;
    scoped_ptr<Picture> thePict;
    Point           starPoint;
    transColorType  *transColor;
    unsigned char   color;
    Rect        longClipRect, starRect;
    inlinePictType  *thisInlinePict;

#pragma unused( mustFit)

    HideCursor();
    GetAnyInterfaceItemGraphicBounds( dataItem, &oldRect);
    CopySaveWorldToOffWorld(*usedRect);

    DrawInOffWorld();
//  HHCheckAllHandles();

    if ( whichBriefPoint >=kMissionBriefPointOffset)
    {
        whichBriefPoint -= kMissionBriefPointOffset;

        BriefPoint_Data_Get( whichBriefPoint, whichScenario, &headerID, &headerNumber, &contentID,
                                 &hiliteBounds, corner, scale, 16, 32, bounds);

        // TODO(sfiera): catch exception.
        textData.reset(new std::string(Resource::get_data('TEXT', contentID)));
        if (textData.get() != nil) {
            textlength = length = textData->size();
            textHeight = GetInterfaceTextHeightFromWidth(
                    reinterpret_cast<const unsigned char*>(textData->c_str()), length,
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
            MacFrameRect(hiliteBounds);
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
        PaintRect(newRect);
        DrawAnyInterfaceItem( dataItem, gOffWorld, 0, 0);

        DrawInOffWorld();
        if (textData.get() != nil) {
            newRect = dataItem->bounds;
            DrawInterfaceTextInRect(
                    newRect, reinterpret_cast<const unsigned char*>(textData->c_str()), length,
                    dataItem->style, dataItem->color, gOffWorld, 0, 0, inlinePict);
            textData.reset();
        }

        DrawInRealWorld();
        NormalizeColors();

        GetAnyInterfaceItemGraphicBounds( dataItem, &newRect);
        newRect.enlarge_to(oldRect);
        newRect.enlarge_to(hiliteBounds);
        oldRect = *usedRect;
        oldRect.enlarge_to(newRect);
        CopyOffWorldToRealWorld(oldRect);
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

                mGetTranslateColorShade( GOLD, VERY_LIGHT, color, transColor);
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
                CopyOffWorldToRealWorld(oldRect);
                *usedRect = newRect;
                textlength = 100;
            }
        } else if ( whichBriefPoint == kMissionBlankBriefNum)
        {
            oldRect = *bounds;
            DrawInRealWorld();
            NormalizeColors();
            CopyOffWorldToRealWorld(oldRect);
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
    transColorType  *transColor;
    Rect        lRect, longClipRect;
    baseObjectType  *baseObject = gBaseObjectData.get();// + (pictID - kFirstShipDataPictID);
    Str255          tempString, numString;
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

        retroTextSpec.text.reset(new std::string(Resource::get_data('TEXT', kShipDataTextID)));
        if ( retroTextSpec.text.get() != nil) {
            // *** Replace place-holders in text with real data, using the fabulous Munger routine
            // an object or a ship?
            if ( baseObject->attributes & kCanThink)
                GetIndString( numString, kShipDataNameID, 1);
            else
                GetIndString( numString, kShipDataNameID, 2);

            GetIndString( tempString, kShipDataKeyStringID, kShipOrObjectStringNum);
            Munger(retroTextSpec.text.get(), 0, (tempString + 1), *tempString, numString + 1, *numString);

            // ship name
//          GetIndString( numString, 5000, pictID - kFirstShipDataPictID + 1);
            GetIndString( numString, 5000, i + 1);
            GetIndString( tempString, kShipDataKeyStringID, kShipTypeStringNum);
            Munger(retroTextSpec.text.get(), 0, (tempString + 1), *tempString, numString + 1, *numString);

            // ship mass
            SmallFixedToString( baseObject->mass, numString);
            GetIndString( tempString, kShipDataKeyStringID, kMassStringNum);
            Munger(retroTextSpec.text.get(), 0, (tempString + 1), *tempString, numString + 1, *numString);

            // ship shields
            NumToString( baseObject->health, numString);
            GetIndString( tempString, kShipDataKeyStringID, kShieldStringNum);
            Munger(retroTextSpec.text.get(), 0, (tempString + 1), *tempString, numString + 1, *numString);

            // light speed
            NumToString( baseObject->warpSpeed, numString);
            GetIndString( tempString, kShipDataKeyStringID, kHasLightStringNum);
            Munger(retroTextSpec.text.get(), 0, (tempString + 1), *tempString, numString + 1, *numString);

            // max velocity
            SmallFixedToString( baseObject->maxVelocity, numString);
            GetIndString( tempString, kShipDataKeyStringID, kMaxSpeedStringNum);
            Munger(retroTextSpec.text.get(), 0, (tempString + 1), *tempString, numString + 1, *numString);

            // thrust
            SmallFixedToString( baseObject->maxThrust, numString);
            GetIndString( tempString, kShipDataKeyStringID, kThrustStringNum);
            Munger(retroTextSpec.text.get(), 0, (tempString + 1), *tempString, numString + 1, *numString);

            // par turn
            SmallFixedToString( baseObject->frame.rotation.turnAcceleration, numString);
            GetIndString( tempString, kShipDataKeyStringID, kTurnStringNum);
            Munger(retroTextSpec.text.get(), 0, (tempString + 1), *tempString, numString + 1, *numString);

            // now, check for weapons!


            GetIndString(numString, kShipDataNameID, kShipDataPulseStringNum);
            *retroTextSpec.text += CreateWeaponDataText(baseObject->pulse, numString);

            GetIndString(numString, kShipDataNameID, kShipDataBeamStringNum);
            *retroTextSpec.text += CreateWeaponDataText(baseObject->beam, numString);

            GetIndString(numString, kShipDataNameID, kShipDataSpecialStringNum);
            *retroTextSpec.text += CreateWeaponDataText(baseObject->special, numString);

            retroTextSpec.textLength = retroTextSpec.text->size();

            mSetDirectFont( kButtonFontNum);
            retroTextSpec.thisPosition = retroTextSpec.linePosition = retroTextSpec.lineCount = 0;
            retroTextSpec.tabSize = 100;
            mGetTranslateColorShade( GREEN, VERY_LIGHT, retroTextSpec.color, transColor);
            mGetTranslateColorShade( GREEN, DARKEST, retroTextSpec.backColor, transColor);
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
            PaintRect(dataRect);
            SetTranslateColorShadeFore( GREEN, VERY_LIGHT);
            MacFrameRect(dataRect);
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

std::string CreateWeaponDataText(long whichWeapon, unsigned char* weaponName) {
    baseObjectType      *weaponObject, *missileObject;
    std::string weaponText;
    Str255              numString, tempString;
    long                mostDamage, actionNum;
    objectActionType    *action;
    bool             isGuided = false;

    if ( whichWeapon != kNoShip)
    {
        weaponObject = gBaseObjectData.get() + whichWeapon;

        // TODO(sfiera): catch exception.
        weaponText = Resource::get_data('TEXT', kWeaponDataTextID);
        if (true) {
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

            // weapon name #
            GetIndString( tempString, kShipDataKeyStringID, kWeaponNumberStringNum);
            Munger(&weaponText, 0, (tempString + 1), *tempString, weaponName + 1, *weaponName);

            // weapon name
            GetIndString( numString, 5000, whichWeapon + 1);
            GetIndString( tempString, kShipDataKeyStringID, kWeaponNameStringNum);
            Munger(&weaponText, 0, (tempString + 1), *tempString, numString + 1, *numString);

            // is guided
            if ( isGuided)
                GetIndString( numString, kShipDataNameID, kShipDataYesStringNum);
            else
                GetIndString( numString, kShipDataNameID, kShipDataNoStringNum);
            GetIndString( tempString, kShipDataKeyStringID, kWeaponGuidedStringNum);
            Munger(&weaponText, 0, (tempString + 1), *tempString, numString + 1, *numString);

            // is autotarget
            if ( weaponObject->attributes & kAutoTarget)
                GetIndString( numString, kShipDataNameID, kShipDataYesStringNum);
            else
                GetIndString( numString, kShipDataNameID, kShipDataNoStringNum);
            GetIndString( tempString, kShipDataKeyStringID, kWeaponAutoTargetStringNum);
            Munger(&weaponText, 0, (tempString + 1), *tempString, numString + 1, *numString);

            // range
            NumToString( lsqrt(weaponObject->frame.weapon.range), numString);
            GetIndString( tempString, kShipDataKeyStringID, kWeaponRangeStringNum);
            Munger(&weaponText, 0, (tempString + 1), *tempString, numString + 1, *numString);

            if ( mostDamage > 0)
            {
                NumToString( mostDamage, numString);
                GetIndString( tempString, kShipDataKeyStringID, kWeaponDamageStringNum);
                Munger(&weaponText, 0, (tempString + 1), *tempString, numString + 1, *numString);
            } else
            {
                GetIndString( numString, kShipDataNameID, kShipDataDashStringNum);
                GetIndString( tempString, kShipDataKeyStringID, kWeaponDamageStringNum);
                Munger(&weaponText, 0, (tempString + 1), *tempString, numString + 1, *numString);
            }

        }
    }
    return ( weaponText);
}

void DoMissionDebriefing(Rect *destRect, long yourlength, long parlength, long yourloss, long parloss,
    long yourkill, long parkill)
{
    Rect            clipRect, boundsRect, tlRect;
    Rect                tRect;
    long                height, waitTime, score = 0;
    retroTextSpecType   retroTextSpec;
    transColorType      *transColor;
    Str255              tempString, numString;

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

    retroTextSpec.text.reset(new std::string(Resource::get_data('TEXT', kSummaryTextID)));
    if (retroTextSpec.text.get() != nil) {
        // *** Replace place-holders in text with real data, using the fabulous Munger routine
        // your minutes
        NumToString( yourlength / 60, numString);
        GetIndString( tempString, kSummaryKeyStringID, kYourMinStringNum);
        Munger(retroTextSpec.text.get(), 0, (tempString + 1), *tempString, numString + 1, *numString);
        // your seconds
        NumToString( yourlength % 60, numString);
//      WriteDebugLine((char *)numString);
//      WriteDebugLong( yourlength % 60);

        mDoubleDigitize( numString);
        GetIndString( tempString, kSummaryKeyStringID, kYourSecStringNum);
        Munger(retroTextSpec.text.get(), 0, (tempString + 1), *tempString, numString + 1, *numString);
        // par minutes
        if ( parlength > 0)
            NumToString( parlength / 60, numString);
        else GetIndString( numString, 6002, 9); // = N/A
            GetIndString( tempString, kSummaryKeyStringID, kParMinStringNum);
        Munger(retroTextSpec.text.get(), 0, (tempString + 1), *tempString, numString + 1, *numString);
        // par seconds
        if ( parlength > 0)
        {
            NumToString( parlength % 60, tempString);
            mDoubleDigitize( tempString);
            CopyPString( numString, "\p:");
            ConcatenatePString( numString, tempString);
            GetIndString( tempString, kSummaryKeyStringID, kParSecStringNum);
            Munger(retroTextSpec.text.get(), 0, (tempString + 1), *tempString, numString + 1, *numString);
        } else
        {
            GetIndString( tempString, kSummaryKeyStringID, kParSecStringNum);
            Munger(retroTextSpec.text.get(), 0, (tempString + 1), *tempString, numString + 1, 0);
        }

        // your loss
        NumToString( yourloss, numString);
        GetIndString( tempString, kSummaryKeyStringID, kYourLossStringNum);
        Munger(retroTextSpec.text.get(), 0, (tempString + 1), *tempString, numString + 1, *numString);
        // par loss
        if ( parlength > 0)
            NumToString( parloss, numString);
        else GetIndString( numString, 6002, 9); // = N/A
        GetIndString( tempString, kSummaryKeyStringID, kParLossStringNum);
        Munger(retroTextSpec.text.get(), 0, (tempString + 1), *tempString, numString + 1, *numString);
        // your kill
        NumToString( yourkill, numString);
        GetIndString( tempString, kSummaryKeyStringID, kYourKillStringNum);
        Munger(retroTextSpec.text.get(), 0, (tempString + 1), *tempString, numString + 1, *numString);
        // par kill
        if ( parlength > 0)
            NumToString( parkill, numString);
        else GetIndString( numString, 6002, 9); // = N/A
        GetIndString( tempString, kSummaryKeyStringID, kParKillStringNum);
        Munger(retroTextSpec.text.get(), 0, (tempString + 1), *tempString, numString + 1, *numString);
        // your score
        if ( parlength > 0)
            NumToString( score, numString);
        else GetIndString( numString, 6002, 9); // = N/A
        GetIndString( tempString, kSummaryKeyStringID, kYourScoreStringNum);
        Munger(retroTextSpec.text.get(), 0, (tempString + 1), *tempString, numString + 1, *numString);
        // par score
        if ( parlength > 0)
            CopyPString( numString, "\p100");
        else GetIndString( numString, 6002, 9); // = N/A
        GetIndString( tempString, kSummaryKeyStringID, kParScoreStringNum);
        Munger(retroTextSpec.text.get(), 0, (tempString + 1), *tempString, numString + 1, *numString);

        retroTextSpec.textLength = retroTextSpec.text->size();

        mSetDirectFont( kButtonFontNum);
        retroTextSpec.thisPosition = retroTextSpec.linePosition = retroTextSpec.lineCount = 0;
        retroTextSpec.tabSize = 60;
        mGetTranslateColorShade( GOLD, VERY_LIGHT, retroTextSpec.color, transColor);
        mGetTranslateColorShade( GOLD, DARKEST, retroTextSpec.backColor, transColor);
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
        retroTextSpec.ypos = boundsRect.top + mDirectFontAscent();

        clipRect.left = 0;
        clipRect.right = clipRect.left + WORLD_WIDTH;
        clipRect.top = 0;
        clipRect.bottom = clipRect.top + WORLD_HEIGHT;
        clipRect = *destRect;
        tlRect = boundsRect;
        tlRect.left -= 2;
        tlRect.top -= 2;
        tlRect.right += 2;
        tlRect.bottom += 2;
        DrawNateVBracket( gOffWorld, tlRect, clipRect, 0, 0, retroTextSpec.color);
        tRect = tlRect;
        CopyOffWorldToRealWorld(tRect);

        while ( retroTextSpec.thisPosition < retroTextSpec.textLength)
        {
            PlayVolumeSound(  kTeletype, kMediumLowVolume, kShortPersistence, kLowPrioritySound);
            DrawRetroTextCharInRect( &retroTextSpec, 3, boundsRect, clipRect, gActiveWorld, gNatePortLeft,
                gNatePortTop);

            waitTime = TickCount();
            while (( TickCount() - waitTime) < 3) {
                // DO NOTHING
            };
        }
        retroTextSpec.text.reset();
    }

}

void DoMissionDebriefingText(long textID, long yourlength, long parlength,
            long yourloss, long parloss, long yourkill, long parkill, long parScore)
{
    Rect                tRect, iRect, scoreRect;
    scoped_ptr<std::string> textData;
    long                length, autoTimeStart, textlength = 0;
    short               textHeight = 0;
    bool             doScore = (parScore >= 0);
    interfaceItemType       dataItem;

    tRect = Rect(CLIP_LEFT, CLIP_TOP, CLIP_RIGHT, CLIP_BOTTOM);
    iRect = Rect(0, 0, kDebriefTextWidth, 1);

    dataItem.style = kLarge;
    textData.reset(new std::string(Resource::get_data('TEXT', textID)));
    if (textData.get() != nil) {
        textlength = length = textData->size();
        textHeight = GetInterfaceTextHeightFromWidth(
                reinterpret_cast<const unsigned char*>(textData->c_str()), length, dataItem.style,
                kDebriefTextWidth);
        if ( doScore) textHeight += kScoreTableHeight;

        iRect.bottom = iRect.top + textHeight;
        iRect.center_in(tRect);

        dataItem.bounds = iRect;
        dataItem.color = GOLD;
        dataItem.kind = kLabeledRect;
        dataItem.style = kLarge;
        dataItem.item.labeledRect.label.stringID = 2001;
        dataItem.item.labeledRect.label.stringNumber = 29;

        DrawInOffWorld();

        GetAnyInterfaceItemGraphicBounds( &dataItem, &tRect);
        SetTranslateColorFore( BLACK);
        PaintRect(tRect);

        DrawAnyInterfaceItem( &dataItem, gOffWorld, 0, 0);

        dataItem.bounds = tRect;
        DrawInterfaceTextInRect(
                tRect, reinterpret_cast<const unsigned char*>(textData->c_str()), length,
                dataItem.style, dataItem.color, gOffWorld, 0, 0, nil);

        textData.reset();

        DrawInRealWorld();
        NormalizeColors();

        GetAnyInterfaceItemGraphicBounds( &dataItem, &tRect);
        CopyOffWorldToRealWorld(tRect);

        if ( doScore)
        {
            scoreRect.left = dataItem.bounds.left;
            scoreRect.right = dataItem.bounds.right;
            scoreRect.bottom = dataItem.bounds.bottom;
            scoreRect.top = scoreRect.bottom - kScoreTableHeight;

            DoMissionDebriefing(&scoreRect, yourlength, parlength, yourloss, parloss,
                        yourkill, parkill);
        }
    }
    while (( AnyRealKeyDown()) || ( Button())) {
        // DO NOTHING
    };
    autoTimeStart = TickCount();
    while (( !AnyRealKeyDown()) && (!(Button())) &&
        (!(( globals()->gOptions & (kOptionAutoPlay | kOptionReplay)) &&
        (( TickCount() - autoTimeStart) < kDebriefTimeOutTime)))) {
        // DO NOTHING
    };
}

#define kBackground_Height  480
#define kScrollText_Buffer  10

void DoScrollText(long textID, long scrollSpeed, long scrollWidth,
    long textFontNum, long songID)
{
    Rect            clipRect, boundsRect, scrollRect, textRect;
    long                height, waitTime = TickCount(), l, autoTimeStart, sectionLength, textLength,
                        charNum, pictID = 0, bgVOffset = 0, bgPictID = -1;
    retroTextSpecType   retroTextSpec;
    transColorType      *transColor;
    Rect                tRect, uRect, vRect, pictRect, pictSourceRect;
    scoped_ptr<std::string> textHandle;
    const char*         sectionStart = NULL;
    const char*         thisChar = NULL;
    const char*         nextChar = NULL;
    scoped_ptr<Picture> thePict;
    scoped_ptr<Picture> bgPict;
    bool             sectionOver, abort = false, wasPicture = true;
    Rect                clipRgn;

    if (( globals()->gOptions & kOptionMusicIdle) && ( songID >= 0))
    {
        if ( SongIsPlaying())
        {
            StopAndUnloadSong();
        }
        LoadSong( songID);
        SetSongVolume( kMaxMusicVolume);
        PlaySong();
    }

    HideCursor();

    BlackenWindow();


    textHandle.reset(new std::string(Resource::get_data('TEXT', textID)));
    if (textHandle.get() != nil) {
        mSetDirectFont( textFontNum);

        boundsRect.left = (WORLD_WIDTH / 2) - ( scrollWidth / 2);
        boundsRect.right = boundsRect.left + scrollWidth;
        boundsRect.top = (WORLD_HEIGHT / 2) - ( kScrollTextHeight / 2);
        boundsRect.bottom = boundsRect.top + kScrollTextHeight;

        textRect.left = boundsRect.left;
        textRect.right = boundsRect.right;
        textRect.top = boundsRect.bottom;
        textRect.bottom = textRect.top + mDirectFontHeight() + kScrollTextLineBuffer + 1;

        scrollRect.left = boundsRect.left;
        scrollRect.right = boundsRect.right;
        scrollRect.top = boundsRect.top;
        scrollRect.bottom = textRect.bottom;

        tRect = scrollRect;
        clipRgn = tRect;

        DrawInRealWorld();

        DrawNateRect( gOffWorld, &scrollRect, 0, 0, 0xff);

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
        textLength = textHandle->size();
        sectionStart = textHandle->c_str();

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
                                    pictID += *sectionStart - '0';
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
                                            bgPictID += *sectionStart - '0';
                                            sectionStart++;
                                            charNum++;
                                        }
                                        if ( bgPictID > 0)
                                        {
                                            bgPict.reset(new Picture(bgPictID));
                                        }
                                    }
                                }
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
            retroTextSpec.text.reset(new std::string);
            for (int i = 0; i < sectionLength; ++i) {
                (*retroTextSpec.text)[i] = sectionStart[i];
            }
            if (true) {
                if (retroTextSpec.text.get() != nil) {
                    sectionStart = thisChar;

                    retroTextSpec.textLength = retroTextSpec.text->size();

                    retroTextSpec.thisPosition = retroTextSpec.linePosition =
                        retroTextSpec.lineCount = 0;
                    retroTextSpec.tabSize = scrollWidth / 2;
                    mGetTranslateColorShade( RED, VERY_LIGHT, retroTextSpec.color, transColor);
            //      mGetTranslateColorShade( RED, DARKEST, retroTextSpec.backColor, transColor);
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
                        thePict.reset(new Picture(pictID));
//                      if ( ResError() != noErr) Debugger();

                        if (thePict.get() != nil) {
                            wasPicture = true;
                            pictRect.left = ( scrollWidth / 2) -
                                (((thePict->bounds().right - thePict->bounds().left)) / 2) +
                                boundsRect.left;
                            pictRect.right = pictRect.left + ((thePict->bounds().right - thePict->bounds().left));
                            pictRect.top = boundsRect.bottom;
//                          pictRect.bottom = pictRect.top + ((**thePict).picFrame.bottom - (**thePict).picFrame.top);
                            pictRect.bottom = pictRect.top + mDirectFontHeight() + kScrollTextLineBuffer;

                            pictSourceRect = thePict->bounds();
                            pictSourceRect.left = ( scrollWidth / 2) -
                                (((thePict->bounds().right - thePict->bounds().left)) / 2) +
                                boundsRect.left;
                            pictSourceRect.right = pictRect.left + ((thePict->bounds().right - thePict->bounds().left));

                            DrawInSaveWorld();
                                if (bgPict.get() != nil) {
                                    Rect bgRect = bgPict->bounds();

                                    bgRect.offset(-bgRect.left, -bgRect.top);
                                    bgRect.offset(scrollRect.left, pictSourceRect.top - bgVOffset);
                                    do
                                    {
                                        CopyBits(bgPict.get(), gActiveWorld, bgPict->bounds(), bgRect);
                                        bgRect.offset(0, kBackground_Height);
                                    } while ( bgRect.top < gSaveWorld->bounds().bottom);
                                }
                            CopyBits(thePict.get(), gActiveWorld, thePict->bounds(), pictSourceRect);
                            DrawInRealWorld();

                            if (bgPict.get() != nil) {
                                pictRect.left = pictSourceRect.left = scrollRect.left;
                                pictRect.right = pictSourceRect.right = scrollRect.right;
                            }
                            pictSourceRect.bottom = pictSourceRect.top + mDirectFontHeight() + kScrollTextLineBuffer;

                            while ((pictSourceRect.top < thePict->bounds().bottom) && (!abort)) {
                                if (pictSourceRect.bottom > thePict->bounds().bottom) {
                                    pictRect.bottom -= pictSourceRect.bottom - thePict->bounds().bottom;
                                    pictSourceRect.bottom = thePict->bounds().bottom;
                                }
                                CopyBits(gSaveWorld, gOffWorld, pictSourceRect, pictRect);

                                tRect = scrollRect;
                                uRect = tRect;
                                uRect.offset(0, -1);
                                vRect = boundsRect;

                                for (   l = 0;
                                        ((l < (mDirectFontHeight() + kScrollTextLineBuffer)) &&
                                            (!abort) &&
                                            ((pictSourceRect.top+l)<
                                                thePict->bounds().bottom));
                                        l++)
                                {
                                    DrawInOffWorld();

                                    ScrollRect(tRect, 0, -1, clipRgn);
                                    DrawInRealWorld();

                                    DrawNateLine(gOffWorld, scrollRect, scrollRect.left, scrollRect.bottom - 1, scrollRect.right - 1,
                                        scrollRect.bottom - 1, 0, 0, BLACK);
                                    CopyOffWorldToRealWorld(vRect);

                                    bgVOffset++;
                                    if ( bgVOffset >= kBackground_Height) bgVOffset = 0;

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

                                pictSourceRect.offset(0, (mDirectFontHeight() + kScrollTextLineBuffer));
                            }
                            thePict.reset();
//                          if ( ResError() != noErr) Debugger();
                        }// else DebugStr("\pNo PICT!");
                    }
                    if  ( wasPicture)
                    {
                        Rect bgRect = bgPict->bounds();

                        wasPicture = false;
                        DrawInSaveWorld();
                        if (bgPict.get() != nil) {
                            bgRect.offset(-bgRect.left, -bgRect.top);
                            bgRect.offset(scrollRect.left, 0);
                            do
                            {
                                CopyBits(bgPict.get(), gActiveWorld, bgPict->bounds(), bgRect);
                                bgRect.offset(0, kBackground_Height);
                            }  while ( bgRect.top < gSaveWorld->bounds().bottom);
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
                        retroTextSpec.ypos = textRect.top + mDirectFontAscent() + kScrollTextLineBuffer;

                        DrawInOffWorld();
                        bgRect = textRect;
                        stRect = textRect;
                        bgRect.offset(0, -bgRect.top);
                        bgRect.offset(0, bgVOffset);
                        // if source bg pict is partially offscreen
                        if ( bgRect.bottom > gSaveWorld->bounds().bottom)
                        {
                            stRect.bottom -= bgRect.bottom - gSaveWorld->bounds().bottom;
                            bgRect.bottom = gSaveWorld->bounds().bottom;
                            CopyBits(gSaveWorld, gOffWorld, bgRect, stRect);
                            stRect.top = stRect.bottom;
                            stRect.bottom = stRect.top + ((textRect.bottom - textRect.top) -
                                            ( bgRect.bottom - bgRect.top));
                            bgRect.top = 0;
                            bgRect.bottom = bgRect.top + (stRect.bottom - stRect.top);
                            CopyBits(gSaveWorld, gOffWorld, bgRect, stRect);
                        } else // just copy appropriate segment
                        {
                            CopyBits(gSaveWorld, gOffWorld, bgRect, stRect);
                        }
                        DrawInRealWorld();

                        textRect.right -= kScrollText_Buffer;
                        DrawRetroTextCharInRect( &retroTextSpec, -1, textRect, textRect, gOffWorld, 0, 0);
                        textRect.right += kScrollText_Buffer;
                        tRect = scrollRect;
                        uRect = tRect;
                        uRect.offset(0, -1);

                        bgVOffset++;
                        if ( bgVOffset >= kBackground_Height) bgVOffset = 0;

                        vRect = boundsRect;
                        for (   l = 0;
                                ((l < (mDirectFontHeight() + kScrollTextLineBuffer)) &&
                                    (!abort));
                                l++)
                        {
                            DrawInOffWorld();
                            ScrollRect(tRect, 0, -1, clipRgn);

                            bgVOffset++;
                            if ( bgVOffset >= kBackground_Height) bgVOffset = 0;

                            DrawNateLine(gOffWorld, scrollRect, scrollRect.left, scrollRect.bottom - 1, scrollRect.right - 1,
                                scrollRect.bottom - 1, 0, 0, BLACK);
                            DrawInRealWorld();
                            CopyOffWorldToRealWorld(vRect);

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
                    retroTextSpec.text.reset();
                }
            }
        }

        for (   l = 0;
                ((l < kScrollTextHeight) &&
                    (!abort));
                l++)
        {
            DrawInOffWorld();
            tRect = scrollRect;
            ScrollRect(tRect, 0, -1, clipRgn);
            DrawNateLine(gOffWorld, scrollRect, scrollRect.left, scrollRect.bottom - 1, scrollRect.right - 1,
                scrollRect.bottom - 1, 0, 0, BLACK);
            DrawInRealWorld();
            CopyOffWorldToRealWorld(vRect);

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
        textHandle.reset();
    } else SysBeep( 20);

    while ( AnyRealKeyDown()) {
        // DO NOTHING
    };
    autoTimeStart = TickCount();

//  while (( !AnyRealKeyDown()) && (!abort) && (!(( globals()->gOptions & (kOptionAutoPlay | kOptionReplay)) && (( TickCount() - autoTimeStart) < kDebriefTimeOutTime))));
    MacShowCursor();

    if (( SongIsPlaying()) && ( songID >= 0))
    {
        StopAndUnloadSong();
    }
}

void Replace_KeyCode_Strings_With_Actual_Key_Names(std::string* text, short resID,
    short padTo)
{
    long    l;
    Str255  numString, tempString;

    for ( l = 0; l < kKeyExtendedControlNum; l++)
    {
//      GetKeyNumName( numString, GetKeyNumFromKeyMap( globals()->gKeyControl[l]));
        GetIndString( numString, resID, GetKeyNumFromKeyMap( globals()->gKeyControl[l]));
        while ( numString[0] < padTo)
        {
            numString[0]++;
            numString[numString[0]] = ' ';
        }
        GetIndString( tempString, kHelpScreenKeyStringID, l + 1);
        while ( Munger(text, 0, (tempString + 1), *tempString,
            numString + 1, *numString) > 0) {
            // DO NOTHING
        };
    }
}

}  // namespace antares
