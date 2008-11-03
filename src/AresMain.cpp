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

#include "ConditionalMacros.h"

#if TARGET_OS_WIN32
    #include <QuickTimeVR.h>
    #include "QTUtilities.h"
    #include "QTVRUtilities.h"
    #include <TextUtils.h>
    #include <Script.h>
    #include <string.h>
#endif // TARGET_OS_WIN32

#include "Resources.h"
#include "GXMath.h"
#include "ToolUtils.h"
#include "Traps.h"
#include "Fonts.h"
//#include <profiler.h>
#include <Timer.h>
#include <Palettes.h>

#define kProfiling_On   0

#if kProfiling_On
#include <profiler.h>
#endif

//#include <stdio.h>        // for _DATE_ & _TIME_ macros

#include "Debug.h"
#include "AresDemoScanner.h"
#include "AresGuideMaker.h"
#include "WrapGameRanger.h"

//#include "CopyProtection.h"  // is included in prefs
#include "SetFontByString.h"
#include "VersionString.h"
#include "GDeviceHandling.h"
#include "Error.h"
#include "OffscreenGWorld.h"
#include "SpriteHandling.h"
#include "SpriteCursor.h"
#include "ColorTranslation.h"
#include "Transitions.h"
#include "Rotation.h"
#include "Processor.h"
#include "SpaceObjectHandling.h"
#include "DirectText.h"
#include "MessageScreen.h"
#include "TitleScreen.h"
#include "ScreenLabel.h"
#include "HandleHandling.h"
#include "Options.h"
#include "StringHandling.h"
#include "HideMenubar.h"
#include "TimeLimit.h"
#include "NetSetupScreen.h"
#include "AresCheat.h"
#include "AresAppleEvent.h"
#include "AresGameRanger.h"
#include "AmbrosiaSerial.h"
#include "AresResFile.h"
#include "SoundFX.h"
#include "Randomize.h"
#include "MathSpecial.h"
#include "AresGlobalType.h"
#include "TimeUnit.h"
#include "Motion.h"
#include "PlayerShip.h"
#include "NonPlayerShip.h"
#include "ScrollStars.h"
#include "AresPreferences.h"
#include "AresNetwork.h"
#include "Instruments.h"
#include "InterfaceHandling.h"
#include "PlayerInterface.h"
#include "KeyMapTranslation.h"
#include "KeyCodes.h"
#include "ShotsBeamsExplosions.h"
#include "ScenarioMaker.h"
#include "Admiral.h"
#include "Music.h"
#include "Minicomputer.h"
#include "EnvironmentCheck.h"
#include "AresMoviePlayer.h"
#include "Beam.h"
#include "AresNetworkSprocket.h"
#include "AresExternalFile.h"
#include "NateDraw.h"
#include "RegistrationTool.h"
#include "AresMain.h"


//#define   kTempNet

#define MENU_BAR_ID     500
#define APPLE_MENU_ID   500
#define FILE_MENU_ID    501
#define EDIT_MENU_ID    502

#define ABOUT_ALERT     500

#define ABOUT_ITEM      1

#define QUIT_ITEM       1

#define kSmallScreenMem     2580
#define kMediumScreenMem    3092
#define kLargeScreenMem     3855
#define kBaseMemorySize     2000000//1652000

#define kReplayDataSize     6000L
#define kReplayBufferSize   5000L

#define kMainError      "\pmain"
#define kReplayResType  'NLRP'
#define kReplayResID    600

#define k68KMaxFrameSkip    9

#define kResendRequestFirstTime     600
#define kResendRequestSecondTime    1200

// result 0 = lose, 1 = win, 2 = restart, 3 = quit

#define kLoseGame       0
#define kWinGame            1
#define kRestartGame    2
#define kQuitGame       3
#define kNoGame         -1

//#define   kCanRecordGame

#define kReplayNotAutoPlay  // if this is set, will play back recorded games

#define mAbortNetworkGame       { if ( gAresGlobal->gGameOver == 0) gAresGlobal->gGameOver = 1; result = kQuitGame; StopNetworking();}

#define kTitleTextScrollWidth   450

#define kFractionalLagCorrectTolerance  8

#define kInterfaceResFileName   "\p:Ares Data Folder:Ares Interfaces"
#define kSpriteResFileName      "\p:Ares Data Folder:Ares Sprites"
#define kSoundResFileName       "\p:Ares Data Folder:Ares Sounds"
//#define   kConstantRate

//#define   kTestNumber     50//5

//#define   kUseSmallPlayWindow // for debugging

extern Handle                   gSpaceObjectData;
extern long                     /*gAresGlobal->gPlayerShipNumber, gAresGlobal->gGameTime, gAresGlobal->gGameStartTime,*/ gRandomSeed;
//                              gAresGlobal->gScenarioWinner, gAresGlobal->gPlayerAdmiralNumber;
extern long                     gNatePortLeft, gNatePortTop, /*gAresGlobal->gSoundVolume,*/ gNetLatency;
extern netStatusType            gNetworkStatus;
//extern UnsignedWide               gAresGlobal->gLastTime;
//extern unsigned long          gAresGlobal->gOptions, gAresGlobal->gTheseKeys, gAresGlobal->gLastKeys;
extern scenarioType             *gThisScenario;
//extern Boolean                    EMERGENCYHACKTEST;
extern short                    gSpriteFileRefID, gInterfaceFileRefID;

aresGlobalType  *gAresGlobal = nil;

CWindowPtr      gTheWindow = nil;//, gAresGlobal->gBackWindow = nil;
MenuHandle      gAppleMenu;
long            gLastTick, /*gAresGlobal->gGameOver = 0,*/
                WORLD_WIDTH = 640,
                WORLD_HEIGHT = 480,
                CLIP_LEFT = 128,
                CLIP_TOP = 0,
                CLIP_RIGHT = 608,
                CLIP_BOTTOM = 480,
                gPlayScreenWidth = 480,
                gPlayScreenHeight = 480;
Boolean         hackWarn = true;

//#ifdef kNonPlayableDemo
//long          gAresGlobal->gForceDemoLevel = 1;
//#endif

//Handle            gAresGlobal->gReplayData = nil;
//short         gAresGlobal->gMainResRefNum;

//unsigned long gAresGlobal->gFrameCount = 0;

extern GDHandle theDevice;
//extern Handle gSerialNumber;
//extern Handle gAresGlobal->gPreferencesData;

//#pragma code68020 off

void main( void);
void SetWindowColorTable( WindowPtr);
static pascal Boolean SetColorTableEntry (CTabHandle, short, const RGBColor *);
void Pause( long time);
void DrawOutlinedString( StringPtr string, RGBColor *color);

#if TARGET_OS_MAC
void main( void)
#else
int CALLBACK WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR theCmdLine, int nCmdShow)
#endif TARGET_OS_MAC
{
    Rect                    windowRect, tRect;
    OSErr                   error;
    RGBColor                initialFadeColor;
    Boolean                 skipFading = false;
    Point                   tpoint;
    EventRecord             theEvent;
    CWindowPtr              whichWindow;
    CTabHandle              theClut = nil, tClut = nil;
    PaletteHandle           thePalette = nil, tPalette, originalPalette;
    Size                    freeMemory = 0;
    Str255                  tempString, userName;
    short                   ts1;

//  Debugger();
    ToolBoxInit();

    gAresGlobal = (aresGlobalType *)NewPtr( sizeof( aresGlobalType));
    if ( gAresGlobal == nil) ExitToShell();
    for ( error = 0; error < kMaxPlayerNum; error++)
        gAresGlobal->gActiveCheats[error] = 0;
    gAresGlobal->gKeyMapBuffer = NewPtr( sizeof(KeyMap) * (long)kKeyMapBufferNum);
    if ( gAresGlobal->gKeyMapBuffer == nil) ExitToShell();
    gAresGlobal->gKeyMapBufferTop = 0;
    gAresGlobal->gKeyMapBufferBottom = 0;
    gAresGlobal->gBackWindow = nil;
    gAresGlobal->gForceDemoLevel = 1;
    gAresGlobal->gReplayData = nil;
    gAresGlobal->gFrameCount = 0;
    gAresGlobal->gPreferencesData = nil;
    gAresGlobal->gGameOver = 1;
    gAresGlobal->gAdmiralData = nil;
    gAresGlobal->gDestBalanceData = nil;
    gAresGlobal->gPreferenceRefNum = 0;
    gAresGlobal->gPreferencesData = nil;
    gAresGlobal->gOptions = kDefaultOptions;
    gAresGlobal->gRaceData = nil;
    gAresGlobal->gScrollStarData = nil;
    gAresGlobal->gWarpStars = false;
    gAresGlobal->gLastClipBottom = 0;
    gAresGlobal->gScrollStarNumber = -1;
    gAresGlobal->gGameTime = 0;
    gAresGlobal->gGameStartTime = 0;
    gAresGlobal->gClosestObject = 0;
    gAresGlobal->gFarthestObject = 0;
    gAresGlobal->gCenterScaleH = 0;
    gAresGlobal->gCenterScaleV = 0;
    gAresGlobal->gProximityGrid = nil;
    gAresGlobal->gLastKeys = 0;
    gAresGlobal->gTheseKeys = 0;
    gAresGlobal->gPlayerShipNumber = 0;
    gAresGlobal->gSelectionLabel = -1;
    gAresGlobal->gDestKeyTime = 0;
    gAresGlobal->gZoomMode = 0;
    gAresGlobal->gDestinationLabel = -1;
    gAresGlobal->gAlarmCount = -1;
    gAresGlobal->gSendMessageLabel = -1;
    gAresGlobal->gDemoZoomOverride = false;
    gAresGlobal->gPlayerAdmiralNumber;
    gAresGlobal->gScenarioRotation = 0;
    gAresGlobal->gThisScenarioNumber = -1;
    gAresGlobal->gScenarioRefID = 0;
    gAresGlobal->gScenarioData = nil;
    gAresGlobal->gScenarioInitialData = nil;
    gAresGlobal->gScenarioConditionData = nil;
    gAresGlobal->gScenarioBriefData = nil;
    gAresGlobal->gRadarBlipData = nil;
    gAresGlobal->gScaleList = nil;
    gAresGlobal->gSectorLineData = nil;
    gAresGlobal->gRadarCount = 0;
    gAresGlobal->gRadarSpeed = 30;
    gAresGlobal->gRadarRange = kRadarSize * 50;
    gAresGlobal->gWhichScaleNum = 0;
    gAresGlobal->gLastScale = SCALE_SCALE;
    gAresGlobal->gInstrumentTop = 0;
    gAresGlobal->gRightPanelLeftEdge = 608;
    gAresGlobal->gMouseActive = kMouseOff;
    gAresGlobal->gMessageData = nil;
    gAresGlobal->gStatusString = nil;
    gAresGlobal->gMessageData = nil;
    gAresGlobal->gMessageTimeCount = 0;
    gAresGlobal->gMessageLabelNum = -1;
    gAresGlobal->gStatusLabelNum = -1;
    gAresGlobal->gTrueClipBottom = 0;
//  gAresGlobal->gMiniScreenHandle = nil;
    gAresGlobal->gMiniScreenData.lineData = nil;
    gAresGlobal->gMiniScreenData.objectData = nil;
    gAresGlobal->gMissionStatusStrList = nil;
    gAresGlobal->gScreenLabelData = nil;
    gAresGlobal->gBeamData = nil;
    gAresGlobal->gColorAnimationStep = 0;
    gAresGlobal->gColorAnimationInSpeed = -1;
    gAresGlobal->gColorAnimationOutSpeed = -1;
    gAresGlobal->gColorAnimationTable = nil;
    gAresGlobal->gSaveColorTable = nil;
    gAresGlobal->gLastSoundTime = 0;
    gAresGlobal->gSoundVolume = 0;
    gAresGlobal->gSoundFileRefID = 0;
    gAresGlobal->gLastSelectedBuildPrice = 0;
    gAresGlobal->gAutoPilotOff = true;
    gAresGlobal->isQuitting = false;
    gAresGlobal->aeInited = false;
    gAresGlobal->gameRangerPending = false;
    gAresGlobal->gameRangerInProgress = false;
    gAresGlobal->useGameRanger = false;
    gAresGlobal->returnToMain = false;
    gAresGlobal->levelNum = 31;
    gAresGlobal->keyMask = 0;
    gAresGlobal->haveSeenRTNotice = false;
    gAresGlobal->ambrosia_Is_Registered = false;
    gAresGlobal->user_is_scum = false;
    gAresGlobal->gSerialNumerator = 0;
    gAresGlobal->gSerialDenominator = 0;
    gAresGlobal->okToOpenFile = true;
    gAresGlobal->externalFileRefNum = -1;
    gAresGlobal->externalFileSpec.vRefNum = 0;
    gAresGlobal->externalFileSpec.parID = 0;
    gAresGlobal->externalFileSpec.name[0] = 0;
    gAresGlobal->originalExternalFileSpec.vRefNum = 0;
    gAresGlobal->originalExternalFileSpec.parID = 0;
    gAresGlobal->originalExternalFileSpec.name[0] = 0;
    gAresGlobal->otherPlayerScenarioFileName[0] = 0;
    gAresGlobal->otherPlayerScenarioFileURL[0] = 0;
    gAresGlobal->otherPlayerScenarioFileVersion = 0;
    gAresGlobal->otherPlayerScenarioFileCheckSum = 0;

    gAresGlobal->internetConfigPresent = false;

    gAresGlobal->hotKeyDownTime = -1;
/*  error = ICStart( &gAresGlobal->internetConfig, 'ar12');
    if ( error == noErr)
    {
        error = ICFindConfigFile( gAresGlobal->internetConfig, 0, nil);
        if ( error == noErr)
        {
            gAresGlobal->internetConfigPresent = true;
        }
    }
*/
    DebugWindowInit ( (WindowPtr)gTheWindow);

    BringDebugToFront();

    if ( !EnvironmentCheck()) ExitToShell();
    if ( AAE_Init() != noErr) ExitToShell();
    AGR_Init();

    MenuBarInit();
    gAresGlobal->gMainResRefNum = CurResFile();

    HandleHandlerInit();

    gInterfaceFileRefID = ARF_OpenResFile( kInterfaceResFileName);
    if ( gInterfaceFileRefID < 0)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kInterfacesFileError, kDataFolderError, -1, -1, __FILE__, 1);
    } else
        UseResFile ( gInterfaceFileRefID);

    gAresGlobal->gScenarioRefID = ARF_OpenResFile( kScenarioResFileName);
    if ( gAresGlobal->gScenarioRefID == -1)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kScenariosFileError, kDataFolderError, -1, -1, __FILE__, 1);
    }

    gAresGlobal->gSoundFileRefID = ARF_OpenResFile( kSoundResFileName);
    if ( gAresGlobal->gSoundFileRefID == -1)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kSoundsFileError, kDataFolderError, -1, -1, __FILE__, 3);
    }
    UseResFile( gAresGlobal->gSoundFileRefID);

    gSpriteFileRefID = ARF_OpenResFile( kSpriteResFileName);
    error = ResError();

    if ( error != noErr)
    {
        ShowErrorOfTypeOccurred( eContinueOnlyErr, kErrorStrID, kDataFileResourceError, error, __FILE__, 1);
    }
    if ( gSpriteFileRefID == -1)
    {
        ShowErrorAny( eExitToShellErr, kErrorStrID, nil, nil, nil, nil, kSpritesFileError, kDataFolderError, -1, -1, __FILE__, 8);
    }

//  AresNetworkInit();
    //error = InitPreferences();

//  gRandomSeed = 61769;
//  qd.randSeed = gRandomSeed;

    error = InitPreferences();
    if ( error == kNoError)
    {

#if NETSPROCKET_AVAILABLE
        if ( InitNetworking() == kNoError)
        {
            gAresGlobal->gOptions |= kOptionNetworkAvailable;
        } else
#endif NETSPROCKET_AVAILABLE
        {
            gAresGlobal->gOptions &= ~kOptionNetworkAvailable;
        }

        GetDateTime( (unsigned long *)&qd.randSeed);
        GetDateTime( (unsigned long *)&gRandomSeed);


//          WriteDebugLine((char *)"\p>Net");

        error  = RT_Open( true, VERSION_2_CODES);
        if ( error != noErr)
        {
            ShowErrorOfTypeOccurred( eQuitErr, kErrorStrID, 77, error, __FILE__, 0);
        }

        if ( OptionKey())
        {
            gAresGlobal->gOptions &= ~(kOptionScreenMedium | kOptionScreenLarge | kOptionScreenSmall);
            gAresGlobal->gOptions |= kOptionScreenSmall;//kOptionScreenLarge;
        }

        if ( gAresGlobal->gOptions & kOptionScreenMedium)
        {
            WORLD_WIDTH = kMediumScreenWidth;
            WORLD_HEIGHT = kMediumScreenHeight;
            CLIP_LEFT = kLeftPanelWidth;
            CLIP_TOP = 0;
            CLIP_RIGHT = kMediumScreenWidth - kRightPanelWidth;
            CLIP_BOTTOM = kMediumScreenHeight;
            gPlayScreenWidth = kMediumScreenWidth - (kLeftPanelWidth + kRightPanelWidth);
            gPlayScreenHeight = kMediumScreenHeight;
        } else if ( gAresGlobal->gOptions & kOptionScreenLarge)
        {
            WORLD_WIDTH = kLargeScreenWidth;
            WORLD_HEIGHT = kLargeScreenHeight;
            CLIP_LEFT = kLeftPanelWidth;
            CLIP_TOP = 0;
            CLIP_RIGHT = kLargeScreenWidth - kRightPanelWidth;
            CLIP_BOTTOM = kLargeScreenHeight;
            gPlayScreenWidth = kLargeScreenWidth - (kLeftPanelWidth + kRightPanelWidth);
            gPlayScreenHeight = kLargeScreenHeight;
        } else
        {
            WORLD_WIDTH = kSmallScreenWidth;
            WORLD_HEIGHT = kSmallScreenHeight;
            CLIP_LEFT = kLeftPanelWidth;
            CLIP_TOP = 0;
            CLIP_RIGHT = kSmallScreenWidth - kRightPanelWidth;
            CLIP_BOTTOM = kSmallScreenHeight;
            gPlayScreenWidth = kSmallScreenWidth - (kLeftPanelWidth + kRightPanelWidth);
            gPlayScreenHeight = kSmallScreenHeight;
        }

        // returns true if device of desired size available
//      Debugger();
//      error = ChooseTheDevice( 8, TRUE);

        MacSetRect( &tRect, 0, 0, 640, 480);
        error = UserChooseTheDevice( 8, TRUE, &tRect);

        #ifndef kUseSmallPlayWindow
            WORLD_WIDTH = tRect.right - tRect.left;
            WORLD_HEIGHT = tRect.bottom - tRect.top;
        #else
            WORLD_WIDTH = 640;
            WORLD_HEIGHT = 480;
        #endif

        if (( WORLD_WIDTH > kLargeScreenWidth) || ( WORLD_HEIGHT > kLargeScreenHeight))
        {
            MacSetRect( &tRect, 0, 0, kLargeScreenWidth, kLargeScreenHeight);
            CenterRectInDevice( theDevice, &tRect);
            WORLD_WIDTH = tRect.right - tRect.left;
            WORLD_HEIGHT = tRect.bottom - tRect.top;
        }


        freeMemory = CompactMem( maxSize) - kBaseMemorySize;
        freeMemory /= 3;
        if ( freeMemory < (WORLD_WIDTH * WORLD_HEIGHT))
        {
            WORLD_HEIGHT = 3L * freeMemory;
            WORLD_HEIGHT /= 4;
            WORLD_HEIGHT = lsqrt( WORLD_HEIGHT);
            WORLD_WIDTH = WORLD_HEIGHT * 4;
            WORLD_WIDTH /= 3;
            WORLD_WIDTH -= WORLD_WIDTH % 8;
            if (( WORLD_WIDTH > kLargeScreenWidth) || ( WORLD_HEIGHT > kLargeScreenHeight))
            {
                WORLD_WIDTH = kLargeScreenWidth;
                WORLD_HEIGHT = kLargeScreenHeight;
            }
            if (( WORLD_WIDTH < kSmallScreenWidth) || ( WORLD_HEIGHT > kSmallScreenHeight))
            {
                // ERROR put in error handing & exit here -- perhaps you can calc it?
            }
            MacSetRect( &tRect, 0, 0, WORLD_WIDTH, WORLD_HEIGHT);
            CenterRectInDevice( theDevice, &tRect);
            WORLD_WIDTH = tRect.right - tRect.left;
            WORLD_HEIGHT = tRect.bottom - tRect.top;
//          NumToString( WORLD_WIDTH, tempString);
//          ParamText( tempString, nil, nil, nil);
//          Alert( 802, nil);
        } else // we had enough memory to fill the screen (up to large screen size)
        {
//          NumToString( freeMemory, tempString);
//          ParamText( tempString, nil, nil, nil);
//          Alert( 802, nil);
        }

            CLIP_LEFT = kLeftPanelWidth;
            CLIP_TOP = 0;
            CLIP_RIGHT = WORLD_WIDTH - kRightPanelWidth;
            CLIP_BOTTOM = WORLD_HEIGHT;
            gPlayScreenWidth = WORLD_WIDTH - (kLeftPanelWidth + kRightPanelWidth);
            gPlayScreenHeight = WORLD_HEIGHT;

        {
            short   oldResFile = CurResFile();

            UseResFile( gAresGlobal->gMainResRefNum);

            theClut = GetCTable( 256);

            UseResFile( oldResFile);
        }

        if ( theClut == nil)
        {
            ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, RESOURCE_ERROR, -1, -1, -1, __FILE__, 500);
        }

        thePalette = NewPalette( 256, theClut, pmExplicit + pmTolerant, 0);
        SetPalette( (WindowPtr)-1, thePalette, false);

//      theDevice = GetMainDevice();
//      error = true;
//      SetColorDepth( theDevice, 8);
        if ( !error) // really if device was not available
        {
            WORLD_WIDTH = kSmallScreenWidth;
            WORLD_HEIGHT = kSmallScreenHeight;
            CLIP_LEFT = kLeftPanelWidth;
            CLIP_TOP = 0;
            CLIP_RIGHT = kSmallScreenWidth - kRightPanelWidth;
            CLIP_BOTTOM = kSmallScreenHeight;
            gPlayScreenWidth = kSmallScreenWidth - (kLeftPanelWidth + kRightPanelWidth);
            gPlayScreenHeight = kSmallScreenHeight;
            gAresGlobal->gOptions &= ~( kOptionScreenSmall | kOptionScreenMedium |
                            kOptionScreenLarge);
            gAresGlobal->gOptions |= kOptionScreenSmall;
            error = ChooseTheDevice( 8, TRUE);
        }
        if ( error) // really if device was available
        {
            MacSetRect( &windowRect, 0, 0, WORLD_WIDTH, WORLD_HEIGHT);
            GetDeviceRect( theDevice, &tRect);
            tpoint.h = tpoint.v = 0;
            ShieldCursor( &tRect, tpoint);

            if ( theDevice == GetMainDevice())
            {

                // FROM ADG:TOOL CHEST 2/95: HIDEMENUBAR CODE SNIPPET
                /*************************************************/
                /* Set the global MBarHeight to 0 to prevent any */
                /*  other apps from writing to the menu bar.     */
                /*************************************************/

/*              MacSetRect( &mBarRect, tRect.left, tRect.top, tRect.right, tRect.top + oldMBarHeight);

                #ifdef kHideMenuBar
                grayRgn = LMGetGrayRgn();
                LMSetMBarHeight( 0);

                // from TotMGPG p 127-129
//              oldGrayRgn = NewRgn();
//              CopyRgn( grayRgn, oldGrayRgn);

                mBarRgn = NewRgn();
                if ( mBarRgn == nil)
                        ShowErrorRecover( OFFSCREEN_GRAPHICS_ERROR, kMainError, 2);
                RectRgn( mBarRgn, &mBarRect);
                UnionRgn( grayRgn, mBarRgn, grayRgn);
                #endif
*/
//              InitHideMenubar();
//              SetMBarState( false);
            }

            InitHideMenubar();
            SetMBarState( false, theDevice);

            InitSpriteCursor();
            CenterRectInDevice( theDevice, &windowRect);

            gAresGlobal->gBackWindow = nil;
            gAresGlobal->gBackWindow = (CWindowPtr)NewCWindow (nil, &tRect, "\p", false, plainDBox,
                        (WindowPtr)-1, false, 701);
            SetWindowColorTable( (WindowPtr)gAresGlobal->gBackWindow);
            InitTransitions();

            initialFadeColor.red = initialFadeColor.green = initialFadeColor.blue = 0;
            MacSetPort( (WindowPtr)gAresGlobal->gBackWindow);
            RGBBackColor( &initialFadeColor);
            BackPat( &qd.black);

            initialFadeColor.red = initialFadeColor.green = initialFadeColor.blue = 0;
//          initialFadeColor.green = 65000;
            RGBForeColor( &initialFadeColor);
            skipFading = AutoFadeTo( 30, &initialFadeColor, true);

            MacShowCursor();

            do
            {
                Ares_WaitNextEvent (everyEvent, &theEvent, 3, nil);
                switch ( theEvent.what)
                {
                    case updateEvt:
                        whichWindow = ( CWindowPtr)theEvent.message;
                        BeginUpdate( (WindowPtr)whichWindow);
                        EndUpdate( (WindowPtr)whichWindow);
                        break;
                }
            } while ( theEvent.what != nullEvent);

//          WaitForAnyEvent();

            HideCursor();

            MacShowWindow( (WindowPtr)gAresGlobal->gBackWindow);

            RGBBackColor( &initialFadeColor);
//          BackPat( &qd.black);

//          WaitForAnyEvent();

            MacSetPort( (WindowPtr)gAresGlobal->gBackWindow);

            PaintRect( &(gAresGlobal->gBackWindow->portRect));

//          WaitForAnyEvent();

            RestoreDeviceClut( theDevice);

//          WaitForAnyEvent();

            ActivatePalette( (WindowPtr)gAresGlobal->gBackWindow);
            SetWindowPaletteFromClut( gAresGlobal->gBackWindow, theClut);

            ResetTransitions();

//          WaitForAnyEvent();

            skipFading = AutoFadeFrom( 1, true);


            skipFading = AutoFadeTo( 1, &initialFadeColor, true);

            gTheWindow = (CWindowPtr)NewCWindow (nil, &windowRect, "\p", TRUE, plainDBox, //documentProc,//
                        (WindowPtr)-1, true, 700);

            SetWindowColorTable( (WindowPtr)gTheWindow);
            initialFadeColor.red = initialFadeColor.green = initialFadeColor.blue = 0;
            MacSetPort( (WindowPtr)gTheWindow);
            RGBBackColor( &initialFadeColor);
//          BackPat( &qd.black);

            MacShowWindow ( (WindowPtr)gTheWindow);
            RGBBackColor( &initialFadeColor);
//          BackPat( &qd.black);

//          WriteDebugLine((char *)"\p>Debug Win");

            MacSetPort ( (WindowPtr)gTheWindow);
            MacSetRect( &windowRect, 0, 0, WORLD_WIDTH, WORLD_HEIGHT);
            MacFillRect( &tRect, (Pattern *)&qd.black);

            BringDebugToFront();
            skipFading = AutoFadeFrom(1, true);
            SetWindowPaletteFromClut( gTheWindow, theClut);

            ShieldCursorInDevice();

            do
            {
                Ares_WaitNextEvent (everyEvent, &theEvent, 3, nil);
                switch ( theEvent.what)
                {
                    case updateEvt:
                        whichWindow = ( CWindowPtr)theEvent.message;
                        BeginUpdate( (WindowPtr)whichWindow);
                        if ( whichWindow == gAresGlobal->gBackWindow)
                        {
                            MacSetPort( (WindowPtr)gAresGlobal->gBackWindow);
                            MacFillRect(  &(gAresGlobal->gBackWindow->portRect), (Pattern *)&qd.black);
                        } else if ( whichWindow == gTheWindow)
                        {
                            MacSetPort( (WindowPtr)gTheWindow);
                            MacFillRect(  &(gTheWindow->portRect), (Pattern *)&qd.black);
                        }
                        EndUpdate( (WindowPtr)whichWindow);
                        break;
                }
            } while ( theEvent.what != nullEvent);
            MacSetPort ( (WindowPtr)gTheWindow);
            MacShowCursor();

            error = CreateOffscreenWorld( &(gTheWindow->portRect), theClut);
            if ( error == kNoError)
            {
                WriteDebugLine((char *)"\p>Offworld");
                WriteDebugLine((char *)"\pGDPMapBounds");
                WriteDebugLong( (*(*theDevice)->gdPMap)->bounds.left);
                WriteDebugLine((char *)"\pGDRect");
                WriteDebugLong( (*theDevice)->gdRect.left);
                WriteDebugLine((char *)"\pgNatePortLeft");
                WriteDebugLong( gNatePortLeft);
                WriteDebugLine((char *)"\pPortRect");
                WriteDebugLong( gTheWindow->portRect.left);

                error = MusicInit();
                    WriteDebugLine((char *)"\p>Music");
                if ( OpenSoundFile() == kNoError)
                {
                    mWriteDebugString("\p>Sound File");
                    InitMoviePlayer();

                    WriteDebugLine((char *)"\p>Movie");
                    error = RotationInit();
                    if ( error == kNoError)
                    {
                        WriteDebugLine((char *)"\p>Rot");

                        NormalizeColors();
                        DrawInRealWorld();
//                      AutoFadeFrom( 30);
                        ColorTranslatorInit( theClut);
//                      InitTransitions();
//                      ResetTransitions();
                        error = InterfaceHandlingInit();
                        if ( error == kNoError)
                        {
                            WriteDebugLine((char *)"\p>Interface");

                            if ( gAresGlobal->originalExternalFileSpec.name[0] > 0)
                            {
                                gAresGlobal->externalFileSpec =
                                    gAresGlobal->originalExternalFileSpec;

                                EF_OpenExternalFile();
                            }

                            if ( gAresGlobal->gOptions & kOptionMusicIdle)
                            {
                                LoadSong( kTitleSongID);
                                SetSongVolume( kMaxMusicVolume);
                                PlaySong();
                            }
                            MacSetPort( (WindowPtr)gTheWindow);
                            if ( !skipFading)
                            {
                                skipFading = CustomPictFade( 20, 20, 2000, 2000, (WindowPtr)gTheWindow);
                                if ( !skipFading)
                                {
                                    skipFading = CustomPictFade( 20, 20, 2001, 2000, (WindowPtr)gTheWindow);
                                }
                            }

                            BlackTitleScreen();

                            if ( !skipFading) PlayMovieByName("\p:Ares Data Folder:Title", (WindowPtr)gTheWindow,
                                false, theDevice);

//                          AutoFadeTo( 1, &initialFadeColor, FALSE);
//                          DrawTitleScreen();
//                          AutoFadeFrom( 90, FALSE);
                            MacSetPort( (WindowPtr)gTheWindow);

                            skipFading = StartCustomPictFade( 20, 20, 502, 2001,
                                (WindowPtr)gTheWindow, &tPalette, &originalPalette,
                                &tClut, skipFading);


//                          MacShowCursor();

/*                          ColorTest();
                            MacSetRect( &tRect, 0, 0, WORLD_WIDTH, WORLD_HEIGHT);
                            CopyOffWorldToRealWorld( (WindowPtr)gTheWindow, &tRect);

                            WaitForAnyEvent();
*/

            do
            {
                Ares_WaitNextEvent (everyEvent, &theEvent, 3, nil);

                switch ( theEvent.what)
                {
                    case updateEvt:
                        whichWindow = ( CWindowPtr)theEvent.message;
                        BeginUpdate( (WindowPtr)whichWindow);
                        if ( whichWindow == gAresGlobal->gBackWindow)
                        {
                            MacSetPort( (WindowPtr)gAresGlobal->gBackWindow);
                            MacFillRect(  &(gAresGlobal->gBackWindow->portRect), (Pattern *)&qd.black);
                        } else if ( whichWindow == gTheWindow)
                        {
                            MacSetPort( (WindowPtr)gTheWindow);
                            DrawTitleScreen();
                        }
                        EndUpdate( (WindowPtr)whichWindow);
                        break;
                }
            } while ( theEvent.what != nullEvent);

                            MacSetPort( (WindowPtr)gTheWindow);
                            GetVersionString( tempString, gAresGlobal->gMainResRefNum);
                            SetFontByString( "\pgeneva");
                            TextSize( 9);
                            TextFace( 0);
                            initialFadeColor.red = initialFadeColor.blue = initialFadeColor.green = 30000;

                            RGBForeColor( &initialFadeColor);

/*                          MoveTo( ((WORLD_WIDTH - kSmallScreenWidth) / 2) +
                                ( kSmallScreenWidth / 2) - (StringWidth( tempString) / 2),
                                465 + ( WORLD_HEIGHT - kSmallScreenHeight) / 2);
*/
                            MoveTo( ( WORLD_WIDTH - StringWidth( tempString) - 4),
                                478 + ( WORLD_HEIGHT - kSmallScreenHeight) / 2);

/*
                            MoveTo( 370 + ( WORLD_WIDTH - kSmallScreenWidth) / 2,
                                    450 + ( WORLD_HEIGHT - kSmallScreenHeight) / 2);
*/
                            DrawOutlinedString( tempString, &initialFadeColor); //DrawString( tempString);

                            initialFadeColor.red = 65535;
                            initialFadeColor.blue = initialFadeColor.green = 0;
                            RGBForeColor( &initialFadeColor);
                            MoveTo( 4, 12 + ( WORLD_HEIGHT - kSmallScreenHeight) / 2);
                            #ifdef kUseAlphaCopyProtection
                                DrawOutlinedString("\pALPHA COPY PROTECTION IS ON. ",
                                    &initialFadeColor);
                            #endif
                            #ifndef kUsePublicCopyProtection
                                DrawOutlinedString("\pPUBLIC COPY PROTECTION IS OFF. ",
                                    &initialFadeColor);
                            #endif

//                          ColorTest();
                            MoveTo( 35 + ( WORLD_WIDTH - kSmallScreenWidth) / 2,
                                    50 + ( WORLD_HEIGHT - kSmallScreenHeight) / 2);
                            initialFadeColor.red = initialFadeColor.green = initialFadeColor.blue = 65535;

                            RGBForeColor( &initialFadeColor);
                            TextSize( 24);
                            TextFont( 0);
                            TextFace( 0);
//                          DrawEncodedString( "\p\x67\x36\x68\x2D\x0C\x0B\x13\x29\x48\x51\x64\x11\x0B\x27\x29\x0E\x4E\x4D\x46\x12\x57\x3D\x12\x3B\x75\x24\x14\x65\x31\x10\x5B\x30\x4D\x5F\x44\x16\x66\x4C\x4E\x6E"); // Beta Demo -- please do NOT redistribute!
//                          DrawEncodedString( "\p\x75\x43\x59\x42\x55\x2C\x25\x5B\x1D\x17\x25\x53\x0B\x63\x5C\x78\x59\x3F\x42\x65\x58\x6D\x36\x5C\x46\x1E\x63\x67\x6B\x7D\x57\x21\x42\x60\x4F\x26\x5A\x3A\x5E\x42\x35"); // Preview Demo - Please Do Not Redistribute
                            MoveTo( 35 + ( WORLD_WIDTH - kSmallScreenWidth) / 2,
                                        385 + ( WORLD_HEIGHT - kSmallScreenHeight) / 2);
                            TextSize( 12);
                            TextFont( 0);
                            TextFace( 0);
//                          DrawEncodedString( "\p\x7B\x36\x66\x3F\x55\x36\x1C\x5B\x0A\x5F\x67\x12\x1B\x77\x6E"); // version 1.0.0A2
//                          DrawEncodedString( "\p\x66\x46\x68\x34\x5B\x39\x17\x36\x3E\x16\x57\x59\x5E\x1C\x2F\x62\x0D"); // authorized user:
//                          prefsData = (preferencesDataType *)*gAresGlobal->gPreferencesData;
//                          DrawYeOldeEncodedString( (StringPtr)prefsData->serialNumber.name);

                            #ifdef kUsePublicCopyProtection
                                GetIndString( tempString, 700, 3);  // "REGISTERED TO:"
                                RT_GetLicenseeName( (unsigned char (*)[256])userName);
                                ConcatenatePString( tempString, userName);

                                ts1 = StringWidth( tempString);
//                              ts1 += StringWidth( userName);
                                MoveTo( (( WORLD_WIDTH) / 2) - (ts1 / 2),
                                            456 + ( WORLD_HEIGHT - kSmallScreenHeight) / 2);
                                DrawOutlinedString( tempString, &initialFadeColor);
//                              DrawString( gAresGlobal->gUserName);
                            #endif

                            if ( gAresGlobal->externalFileRefNum >= 0)
                            {
                                ts1 = StringWidth(
                                    gAresGlobal->scenarioFileInfo.titleString);

                                MoveTo( (( WORLD_WIDTH) / 2) - (ts1 / 2),
                                            356 + ( WORLD_HEIGHT - kSmallScreenHeight)
                                                / 2);
                                DrawOutlinedString(
                                        gAresGlobal->scenarioFileInfo.titleString,
                                        &initialFadeColor);
                            }

                            SetFontByString( "\pgeneva");
                            TextSize( 10);
                            TextFace( bold);
                            MoveTo( 245 + ( WORLD_WIDTH - kSmallScreenWidth) / 2,
                                    18 + ( WORLD_HEIGHT - kSmallScreenHeight) / 2);
//                          DrawCString( __DATE__);
//                          DrawCString(" ");
//                          DrawCString( __TIME__);
                            error = InitDirectText();
                            if ( error == kNoError)
                            {
                                WriteDebugLine((char *)"\p>DText");

                                error = ScreenLabelInit();
                                if ( error == kNoError)
                                {
                                    WriteDebugLine((char *)"\p>Label");

                                    error = InitMessageScreen();
                                    if ( error == kNoError)
                                    {
                                        WriteDebugLine((char *)"\p>Message");

                                        error = InitScrollStars();
                                        if ( error == kNoError)
                                        {
                                            WriteDebugLine((char *)"\p>ScrollStar");

                                            error = InstrumentInit();
                                            if ( error == kNoError)
                                            {
                                                WriteDebugLine((char *)"\p>Instrument");


                                                SpriteHandlingInit();
                                                AresCheatInit();
                                                error = ScenarioMakerInit();
                                                {
                                                    error = SpaceObjectHandlingInit();  // MUST be after ScenarioMakerInit()
                                                    if ( error == kNoError)
                                                    {
                                                        WriteDebugLine((char *)"\p>SpaceObj");
                                                        error = InitSoundFX();
//                                                      if ( error == kNoError)
                                                        {
                                                            WriteDebugLine((char *)"\p>SoundFX");
                                                            error =InitMotion();
                                                            if ( error == kNoError)
                                                            {
                                                                WriteDebugLine((char *)"\p>Motion");

                                                                error = AdmiralInit();
                                                                if ( error == kNoError)
                                                                {
                                                                    error = InitBeams();
                                                                    if ( error == kNoError)
                                                                    {
//          InitNetworking();
                                                                        TimedWaitForAnyEvent(
                                                                            skipFading?1:1400);
                                                                        EndCustomPictFade(
                                                                            (WindowPtr)gTheWindow,
                                                                            &tPalette, &originalPalette,
                                                                            &tClut,
                                                                            skipFading);
                                                                        MacShowCursor();    // one for the titlescreen
                                                                        MacShowCursor();    // one for the whole deal
//                                                                      ColorTranslatorInit( theClut);

                                                                        gLastTick = TickCount();

//          RandomInit();
                                                                        gAresGlobal->okToOpenFile = true;
                                                                        MainLoop();
//          RandomCleanup();
            CleanupMoviePlayer();
//          DisposeNetworking();
                                                                        CleanupBeams();
                                                                        WriteDebugLine((char *)"\p<Beams");
                                                                    }
                                                                    AdmiralCleanup();
                                                                    WriteDebugLine((char *)"\p<Admiral");
                                                                }

                                                                MotionCleanup();
                                                                WriteDebugLine((char *)"\p<Motion");
                                                            }

                                                            SoundFXCleanup();
                                                            WriteDebugLine((char *)"\p<Sound");
                                                        }
                                                        CleanupSpaceObjectHandling();
                                                        WriteDebugLine((char *)"\p<Obj Handle");
                                                        CleanupSpriteHandling();
                                                        CleanupAresCheat();
                                                        WriteDebugLine((char *)"\p<Sprite");
                                                    }
                                                    ScenarioMakerCleanup();
                                                }
                                                InstrumentCleanup();
                                                WriteDebugLine((char *)"\p<Instrument");
                                            }
                                            CleanupScrollStars();
                                            WriteDebugLine((char *)"\p<Stars");
                                        }
                                        MessageScreenCleanup();
                                        WriteDebugLine((char *)"\p<Message");
                                    }
                                    ScreenLabelCleanup();
                                    WriteDebugLine((char *)"\p<Label");
                                }
                                DirectTextCleanup();
                                WriteDebugLine((char *)"\p<DText");
                            }
                            InterfaceHandlingCleanup();
                            WriteDebugLine((char *)"\p<Interface");
                        }
                        ColorTranslatorCleanup();
                        WriteDebugLine((char *)"\p<Color");
                        RotationCleanup();
                        WriteDebugLine((char *)"\p<Rotation");
                        CleanupTransitions();
                        CleanupSpriteCursor();
                        WriteDebugLine((char *)"\p<Transition");
                        CleanupMoviePlayer();
                    }
                    MusicCleanup();
                    WriteDebugLine((char *)"\p<Music");
                }
                CleanUpOffscreenWorld();
                WriteDebugLine((char *)"\p<GWorld");
            }
            WriteDebugLine((char *)"\p<Network");
            HandleHandlerCleanup();
/*          WriteDebugLine((char *)"\p<WAITING>");
            WaitForAnyEvent();
*/
            CleanUpTheDevice( TRUE);

            DisposeCTable( theClut);

            if ( theDevice == GetMainDevice())
            {
/*              LMSetMBarHeight(oldMBarHeight);
                DiffRgn( grayRgn, mBarRgn, grayRgn);
                DisposeRgn( mBarRgn);
*/
                SetMBarState( true, theDevice);
            }

            DebugWindowCleanup();
            DisposeWindow ( (WindowPtr)gTheWindow);
        } else
        {
            ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, PIX_DEPTH_ERROR, -1, -1, -1, __FILE__, 1);
        }

#if NETSPROCKET_AVAILABLE
        DisposeNetworking();
#endif NETSPROCKET_AVAILABLE
        RT_Close();
        PreferencesCleanup();
    }

    if ( gAresGlobal->internetConfigPresent)
    {
//      ICStop( gAresGlobal->internetConfig);
    }

    FlushEvents(everyEvent, 0);
    DisposePtr( (Ptr)gAresGlobal);
    InitCursor();

}
//#pragma code68020 reset

//#pragma code68020 off
void ToolBoxInit( void)

{
    InitGraf(&qd.thePort);
    InitFonts();
    InitWindows();
    InitMenus();
    TEInit();
    InitDialogs( nil);
    InitCursor();
    SetEventMask( everyEvent);
    MoreMasters();
    MoreMasters();
    MoreMasters();
    MoreMasters();
    MoreMasters();
    MoreMasters();
    MaxApplZone();
    FlushEvents(everyEvent, 0);     // Removes all events
    /*SetGrowZone( &OutOfMemoryError);*/
}
//#pragma code68020 reset


void MenuBarInit( void)
{
    Handle      myMenuBar;

    myMenuBar = GetNewMBar( MENU_BAR_ID);
    SetMenuBar ( myMenuBar);
    gAppleMenu = GetMenuHandle (APPLE_MENU_ID);

    AppendResMenu( gAppleMenu, 'DRVR');
    MacDrawMenuBar();
}

void MainLoop (void)

{
    long                    whichScenario = 0, saveSeed = 0, gameLength, whichDemoLevel, t;
    Boolean                 done = FALSE, jumpLevel = false;
    mainScreenResultType    mainResult;
    netResultType           netResult;
    RGBColor                fadeColor;
    short                   gameResult;
    Str255                  resName, movieName;     // for GetResInfo, when we're jumping to a demo
    ResType                 resType;        // '' ''
    short                   resID;          // '' ''

    if (!(gAresGlobal->gOptions & kOptionHaveSeenIntro))
    {
        DoScrollText( (WindowPtr)gTheWindow, 5600, 4, kTitleTextScrollWidth,
            kTitleFontNum, -1);

        gAresGlobal->gOptions |= kOptionHaveSeenIntro;
        SaveOptionsPreferences();
    }

    while ( !done)
    {
        WriteDebugLine((char *)"\p>MainScreen");
        MacSetPort( (WindowPtr)gTheWindow);
        mainResult = DoMainScreenInterface( &whichDemoLevel);

        switch( mainResult)
        {
            case kMainQuit:
#ifdef kCreateAresDemoData
                MakeDemoDataHack();
#endif
//              DoScrollText( (WindowPtr)gTheWindow, 6501, 4, 540,
//                  kTitleFontNum/*kComputerFontNum*/, -1);
//              MakeAresGuide();

                fadeColor.red = fadeColor.green = fadeColor.blue = 0;
                if ( gAresGlobal->gOptions & kOptionMusicIdle)
                {
                    AutoMusicFadeTo( 60, &fadeColor, FALSE);
                } else
                {
                    AutoFadeTo( 60, &fadeColor, FALSE);
                }
                done = TRUE;
                break;

            case kMainTimeoutDemo:

                if ( Randomize( 4) == 2)
                {
                    DoScrollText( (WindowPtr)gTheWindow, 5600, 4, kTitleTextScrollWidth,
                        kTitleFontNum, -1);
                }

                // NO BREAK! FALL THROUGH TO case kMainDemo:

            case kMainDemo: // identical to main play, except we turn on autoplay bit
//              AutoFadeTo( 60, &fadeColor, FALSE);
//              ShowSuccessAnimation( (WindowPtr)gTheWindow);

                gAresGlobal->gOptions |= kOptionReplay;

                // NO BREAK! FALL THROUGH TO case kMainPlay:

//What you are about to do is totally unsupported. Nothing is final. This is a pre-release development build!
                                  //This is a pre-release development build! This is not the final demo. Elements are subject to change.
            case kMainPlay:
                whichScenario = 0;
                jumpLevel = FALSE;

                if ( (!Ambrosia_Is_Registered()) &&
                    (gAresGlobal->externalFileRefNum > 0))
                {
                    ShowErrorAny( eContinueErr, kErrorStrID,  nil, nil, nil, nil,
                        79, -1, -1, -1, __FILE__, 2);

                    gAresGlobal->externalFileSpec.name[0] = 0;
                    EF_OpenExternalFile();
                }

                if ( !( gAresGlobal->gOptions & kOptionReplay))
                {
                    whichScenario = DoSelectLevelInterface( GetStartingLevelPreference());
                    jumpLevel = TRUE;
                }
//              gAresGlobal->gOptions |= kOptionAutoPlay;

                if ( whichScenario >= 0)
                {
                    #ifdef kCanRecordGame
                    if ( ControlKey())
                    {
                        unsigned long   *randomSeed = nil;

                        if ( gAresGlobal->gOptions & kOptionReplay)
                        {
                            short   refNum = CurResFile();

                            UseResFile( gAresGlobal->gMainResRefNum);
                            if ( gAresGlobal->gReplayData != nil)
                            {
                                DetachResource( gAresGlobal->gReplayData);
                                randomSeed = (unsigned long *)*gAresGlobal->gReplayData;
                                gRandomSeed = *randomSeed;
                            }
                            UseResFile( refNum);
                        } else
                        {
                            SysBeep(20);
                            gAresGlobal->gOptions |= kOptionRecord;
                            if ( gAresGlobal->gReplayData != nil)
                                DisposeHandle( gAresGlobal->gReplayData);
                            gAresGlobal->gReplayData = NewHandle( sizeof( unsigned long) * (long)kReplayDataSize);
                            randomSeed = (unsigned long *)*gAresGlobal->gReplayData;
                            *randomSeed = gRandomSeed;
                        }
                    }
                    #endif

                    if ( gAresGlobal->gOptions & kOptionReplay)
                    {
                        #ifdef kReplayNotAutoPlay

                        short   refNum = CurResFile();
                        unsigned long   *randomSeed = nil;

                        UseResFile( gAresGlobal->gMainResRefNum);

                        if (!(jumpLevel))
                        {

                            gAresGlobal->gReplayData = nil;

                            if ( whichDemoLevel >= 0)// get indexed demo level
                            {
                                gAresGlobal->gReplayData =
                                    GetIndResource( kReplayResType, whichDemoLevel + 1);
                                if ( ResError() != noErr) gAresGlobal->gReplayData = nil;
                                if ( gAresGlobal->gReplayData != nil)
                                {
                                    GetResInfo( gAresGlobal->gReplayData, &resID, &resType, resName);
                                    whichScenario = resID - kReplayResID;
                                }
                            }

                            if ( gAresGlobal->gReplayData == nil)
                            {
                                do
                                {
                                    whichScenario = Randomize( gAresGlobal->levelNum);//Randomize( 30 + 1);
                                    whichScenario = GetScenarioNumberFromChapterNumber( whichScenario);
                                    gAresGlobal->gReplayData = GetResource( kReplayResType, kReplayResID + whichScenario);
                                    if ( ResError() != noErr) gAresGlobal->gReplayData = nil;
                                } while ( gAresGlobal->gReplayData == nil);
                            }                       } else
                        {
                                gAresGlobal->gReplayData = GetResource( kReplayResType, kReplayResID + whichScenario);
                        }

                        if ( gAresGlobal->gReplayData != nil)
                        {
                            DetachResource( gAresGlobal->gReplayData);
                            randomSeed = (unsigned long *)*gAresGlobal->gReplayData;
                            saveSeed = gRandomSeed;
                            gRandomSeed = *randomSeed;
                        }
                        UseResFile( refNum);

                        #else

//                      whichScenario = Randomize( kHackLevelMax);
                        whichScenario = 5;

                        #endif
                    }

                    StopAndUnloadSong();


                    gameResult = kNoGame;

                    do
                    {
                        if (( gameResult == kNoGame) || ( gameResult == kWinGame))
                        {
                            GetScenarioMovieName( whichScenario, movieName);
                            if ( movieName[0] != 0)
                            {
                                PlayMovieByName( movieName, (WindowPtr)gTheWindow,
                                    true, theDevice);
                            }

                            if ( GetScenarioPrologueID( whichScenario) > 0)
                            {
                                DoScrollText( (WindowPtr)gTheWindow,
                                    GetScenarioPrologueID( whichScenario),
                                    4, kTitleTextScrollWidth, kTitleFontNum, 4002);
                            }
                        }

                    gameResult = kNoGame;
                    fadeColor.red = fadeColor.green = fadeColor.blue = 0;
                    if ( gAresGlobal->gOptions & kOptionMusicIdle)
                    {
                        AutoMusicFadeTo( 60, &fadeColor, FALSE);
                        StopAndUnloadSong();
                    } else
                    {
                        AutoFadeTo( 60, &fadeColor, FALSE);
                    }

                        RemoveAllSpaceObjects();
                        gAresGlobal->gGameOver = 0;

                        // mission briefing unfades screen

    //                  if ( gAresGlobal->gOptions & kOptionAutoPlay) whichScenario = 2;

                        if ( gAresGlobal->gOptions & kOptionMusicIdle)
                        {
                            LoadSong( 3000);
                            SetSongVolume( kMaxMusicVolume);
                            PlaySong();
                        }

    //gRandomSeed = 910697788;
                        gameResult = kNoGame;
                        if ( !ConstructScenario( whichScenario))
                            gameResult = kQuitGame;

                        if ( gameResult != kQuitGame)
                        {
                            SetMBarState( false, theDevice);
                            if ( gAresGlobal->gOptions & kOptionMusicIdle)
                            {
                                StopAndUnloadSong();
                            }

                            HideCursor();
                            DrawInstrumentPanel( (WindowPtr) gTheWindow);
                            MacShowCursor();

                            if ( gAresGlobal->gOptions & kOptionMusicPlay)
                            {
                                LoadSong( gThisScenario->songID);
                                SetSongVolume( kMusicVolume);
                                PlaySong();
                            }
                            ResetLastTime( (gThisScenario->startTime & kScenario_StartTimeMask) * kScenarioTimeMultiple);

// *** PLAY THE GAME

                            gameResult = PlayTheGame( &gameLength);

//              DebugFileSave( "\p_Poopy");
//              DebugFileCleanup();


                            if (( !( gAresGlobal->gOptions & (kOptionAutoPlay | kOptionRecord | kOptionReplay))) && ( gameResult == kLoseGame))
                            {
                                if ( (gAresGlobal->gScenarioWinner & kScenarioWinnerTextMask) != kScenarioWinnerNoText)
                                {
                                    DoMissionDebriefingText( (WindowPtr) gTheWindow,
                                        (gAresGlobal->gScenarioWinner & kScenarioWinnerTextMask) >> kScenarioWinnerTextShift,
                                            -1, -1, -1, -1, -1, -1, -1);
                                }
                                if ( DoPlayAgain( false, false))
                                {
                                    gameResult = kRestartGame;
                                    WriteDebugLine((char *)"\pAGAIN!");
                                }  else
                                {
                                    gameResult = kQuitGame;
                                    WriteDebugLine((char *)"\pNOT AGAIN!");
                                    BlackTitleScreen();
                                    AutoFadeFrom( 1, false);
                                }
                            } else if ( gameResult == kWinGame)
                            {
                                if ( (gAresGlobal->gScenarioWinner & kScenarioWinnerTextMask) != kScenarioWinnerNoText)
                                {
                                    DoMissionDebriefingText( (WindowPtr) gTheWindow,
                                        (gAresGlobal->gScenarioWinner & kScenarioWinnerTextMask) >> kScenarioWinnerTextShift,
                                        gameLength, gThisScenario->parTime,
                                        GetAdmiralLoss( 0), gThisScenario->parLosses, GetAdmiralKill( 0), gThisScenario->parKills, 100);
                                }

                                fadeColor.red = fadeColor.green = fadeColor.blue = 0;
                                if ( gAresGlobal->gOptions & kOptionMusicPlay)
                                {
                                    AutoMusicFadeTo( 60, &fadeColor, FALSE);
                                    StopAndUnloadSong();
                                } else
                                {
                                    AutoFadeTo( 60, &fadeColor, FALSE);
                                }

                                if ( gameResult == kWinGame)
                                {

                                    BlackTitleScreen();
                                    AutoFadeFrom( 1, FALSE);

                                    t = 4002; // normal scrolltext song
                                    if (( gAresGlobal->gScenarioWinner & kScenarioWinnerNextMask) ==
                                        kScenarioWinnerNoNext)
                                        t = 4003; // we win but no next level? Play triumph song

                                    if ( (!Ambrosia_Is_Registered()) &&
                                        ( GetChapterNumberFromScenarioNumber( whichScenario) >= 9))
                                    {
                                        gAresGlobal->gScenarioWinner = (gAresGlobal->gScenarioWinner
                                            & ~kScenarioWinnerNextMask) | kScenarioWinnerNoNext;

                                        SaveStartingLevelPreferences( 10);
                                        DoScrollText( (WindowPtr)gTheWindow,
                                            6501,
                                            4, kTitleTextScrollWidth, kTitleFontNum, t);
                                    } else
                                    {

                                        if ( GetScenarioEpilogueID( whichScenario) > 0)
                                            DoScrollText( (WindowPtr)gTheWindow,
                                                GetScenarioEpilogueID( whichScenario),
                                                4, kTitleTextScrollWidth, kTitleFontNum, t);
                                    }
        /*                              DoMissionDebriefing( (WindowPtr) gTheWindow, gameLength, gThisScenario->parTime,
                                        GetAdmiralLoss( 0), gThisScenario->parLosses, GetAdmiralKill( 0), gThisScenario->parKills, 100);
    */

// *********

                                    PlayMovieByName("\p:Ares Data Folder:Next Level", (WindowPtr)gTheWindow,
                                        true, theDevice);
                                    if ( gAresGlobal->gOptions & kOptionMusicIdle)
                                    {
                                        BlackTitleScreen();
                                        StopAndUnloadSong();
                                    }

                                    if ( gAresGlobal->gOptions & kOptionReplay)
                                    {
                                        gRandomSeed = saveSeed;
                                    }

                                    if ( gAresGlobal->gOptions & kOptionRecord)
                                    {
                                        Handle  tres;
                                        short   refNum = CurResFile();

                                        UseResFile( gAresGlobal->gMainResRefNum);
                                        if ( gAresGlobal->gReplayData != nil)
                                        {
                                            tres = GetResource( kReplayResType, kReplayResID + whichScenario);
                                            if ( tres != nil)
                                            {
                                                RemoveResource( tres);
                                                UpdateResFile( gAresGlobal->gMainResRefNum);
                                                DisposeHandle( tres);
                                            }
                                            AddResource( gAresGlobal->gReplayData, kReplayResType, kReplayResID + whichScenario, "\pReplay Data");
                                            ChangedResource( gAresGlobal->gReplayData);
                                            WriteResource( gAresGlobal->gReplayData);
                                            DetachResource( gAresGlobal->gReplayData);
                                            DisposeHandle( gAresGlobal->gReplayData);
                                            gAresGlobal->gReplayData = nil;
                                        }
                                        UseResFile( refNum);
                                    } else if ( gAresGlobal->gOptions & kOptionReplay)
                                    {
                                        DisposeHandle( gAresGlobal->gReplayData);
                                        gAresGlobal->gReplayData = nil;
                                    }

    //                              whichScenario++;
                                    // whichScenario = GetNextScenarioChapter( whichScenario);
                                    if (( gAresGlobal->gScenarioWinner & kScenarioWinnerNextMask) ==
                                        kScenarioWinnerNoNext)
                                        whichScenario = -1;
                                    else
                                    {
                                        whichScenario = GetScenarioNumberFromChapterNumber(
                                            (gAresGlobal->gScenarioWinner &
                                            kScenarioWinnerNextMask) >>
                                            kScenarioWinnerNextShift);
                                    }
                                    if (( !( gAresGlobal->gOptions & (kOptionAutoPlay | kOptionRecord | kOptionReplay))) && ( whichScenario <= GetScenarioNumber()) &&
                                        ( whichScenario >= 0))
                                    {
                                        if (( GetChapterNumberFromScenarioNumber( whichScenario) >= 0) &&
                                            ( GetChapterNumberFromScenarioNumber( whichScenario) <= kHackLevelMax) &&
                                            ( GetChapterNumberFromScenarioNumber( whichScenario) <= GetScenarioNumber()))
                                        {
                                            SaveStartingLevelPreferences( GetChapterNumberFromScenarioNumber( whichScenario));
                                        } else whichScenario = -1;
                                    }
                                } else
                                {
                                    BlackTitleScreen();
                                    AutoFadeFrom( 1, false);
                                }
                            }
                        }
                        if ( gAresGlobal->gOptions & kOptionMusicPlay) StopAndUnloadSong();
                    } while ( (gameResult != kQuitGame) &&
                        ( GetChapterNumberFromScenarioNumber( whichScenario) <= kHackLevelMax)
                        && ( GetChapterNumberFromScenarioNumber( whichScenario) <= GetScenarioNumber())
                        && ( whichScenario >= 0) &&
                        ( !( gAresGlobal->gOptions & (kOptionAutoPlay | kOptionRecord | kOptionReplay))));
                    if ( gAresGlobal->gOptions & kOptionMusicIdle)
                    {
                        LoadSong( kTitleSongID);
                        SetSongVolume( kMaxMusicVolume);
                        PlaySong();
                    }
                }
                gAresGlobal->gOptions &= ~(kOptionAutoPlay | kOptionRecord | kOptionReplay);
                if ( OptionKey()) DebugFileSave( "\p_Poopy");
                DebugFileCleanup();
                break;

            case kMainNetwork:
#if NETSPROCKET_AVAILABLE
                if ( gAresGlobal->gOptions & kOptionNetworkOn)
                    StopNetworking();
                gAresGlobal->gOptions &= ~kOptionNetworkOn;
                netResult = StartNetworkGameSetup();
                if ( gAresGlobal->gOptions & kOptionMusicIdle)
                {
                    StopAndUnloadSong();
                }
                if ( netResult != kCancel)
                {
                    gAresGlobal->gOptions |= kOptionNetworkOn;
                    if ( netResult == kHost)
                    {
                        netResult = HostAcceptClientInterface();
                    } else
                    {
                        netResult = ClientWaitInterface();
                    }
                }

                if ( netResult != kCancel)
                {
                    do
                    {
                        ResetNetData();

                        //whichScenario = DoNetLevelInterface();
                        whichScenario = DoTabbedNetLevelInterface();
                        mWriteDebugString("\pLEVEL:");
                        WriteDebugLong( whichScenario);
                        if ( whichScenario >= 0)
                        {
                            whichScenario = GetScenarioNumberFromChapterNumber( whichScenario);
                            mWriteDebugString("\pSCEN:");
                            WriteDebugLong( whichScenario);
                            StopAndUnloadSong();

    /*                      if ( GetScenarioPrologueID( whichScenario) > 0)
                                DoScrollText( (WindowPtr)gTheWindow, GetScenarioPrologueID( whichScenario), \
                                    4, kTitleFontNum);
    */
                            gameResult = kNoGame;
                            fadeColor.red = fadeColor.green = fadeColor.blue = 0;
                            if ( gAresGlobal->gOptions & kOptionMusicIdle)
                            {
                                AutoMusicFadeTo( 60, &fadeColor, FALSE);
                                StopAndUnloadSong();
                            } else
                            {
                                AutoFadeTo( 60, &fadeColor, FALSE);
                            }

                            RemoveAllSpaceObjects();
                            gAresGlobal->gGameOver = 0;

                            // mission briefing unfades screen

                            if ( gAresGlobal->gOptions & kOptionMusicIdle)
                            {
                                LoadSong( 3000);
                                SetSongVolume( kMaxMusicVolume);
                                PlaySong();
                            }

                            WriteDebugDivider();
                            mWriteDebugString("\pConstructing:");
                            WriteDebugLong( whichScenario);
                            if (ConstructScenario( whichScenario))
                            {
    //                          EMERGENCYHACKTEST = true;

                                SetMBarState( false, theDevice);
                                if ( gAresGlobal->gOptions & kOptionMusicIdle)
                                {
                                    StopAndUnloadSong();
                                }

                                HideCursor();
                                DrawInstrumentPanel( (WindowPtr) gTheWindow);
                                MacShowCursor();
                                if ( gAresGlobal->gOptions & kOptionMusicPlay)
                                {
                                    LoadSong( gThisScenario->songID);
                                    SetSongVolume( kMusicVolume);
                                    PlaySong();
                                }
                                ResetLastTime( (gThisScenario->startTime & kScenario_StartTimeMask) * kScenarioTimeMultiple);

                                if ( gAresGlobal->gameRangerInProgress)
                                {
                                    Wrap_GRGameBegin();
                                }

                                gameResult = PlayTheGame( &gameLength);

                                if ( gAresGlobal->gameRangerInProgress)
                                {
                                    if ( (gAresGlobal->gScenarioWinner & kScenarioWinnerPlayerMask) == gAresGlobal->gPlayerAdmiralNumber)
                                    {
                                        Wrap_GRStatScore( 1);
                                        Wrap_GRStatOtherScore( 0);
                                    } else
                                    {
                                        Wrap_GRStatScore( 0);
                                        Wrap_GRStatOtherScore( 1);
                                    }

                                    Wrap_GRGameEnd();
                                }

                                SetNetMinutesPlayed( GetNetMinutesPlayed() + (( gameLength + 30) / 60));
                                SetNetKills( GetNetKills() + GetAdmiralKill( gAresGlobal->gPlayerAdmiralNumber));
                                SetNetLosses( GetNetLosses() + GetAdmiralLoss( gAresGlobal->gPlayerAdmiralNumber));
                                if ( (gAresGlobal->gScenarioWinner &
                                    kScenarioWinnerTextMask) != kScenarioWinnerNoText)
                                {
//                                  DoMissionDebriefingText( (WindowPtr) gTheWindow,
//                                      (gAresGlobal->gScenarioWinner & kScenarioWinnerTextMask) >> kScenarioWinnerTextShift,
//                                      -1, -1, -1, -1, -1, -1, -1);
                                    DoMissionDebriefingText( (WindowPtr) gTheWindow,
                                        (gAresGlobal->gScenarioWinner & kScenarioWinnerTextMask)
                                        >> kScenarioWinnerTextShift,
                                        gameLength, gThisScenario->parTime,
                                        GetAdmiralLoss( gAresGlobal->gPlayerAdmiralNumber),
                                        gThisScenario->parLosses,
                                        GetAdmiralKill( gAresGlobal->gPlayerAdmiralNumber),
                                        gThisScenario->parKills, 100);

                                }
                                if ( gAresGlobal->gOptions & kOptionMusicPlay)
                                {
                                    StopAndUnloadSong();
                                }

                            }
                        }
                    } while (( whichScenario >= 0) && ( NetGameIsOn()));
                }
                if (( gAresGlobal->gOptions & kOptionMusicIdle) && ( !SongIsPlaying()))
                {
                    LoadSong( kTitleSongID);
                    SetSongVolume( kMaxMusicVolume);
                    PlaySong();
                }

                gAresGlobal->gOptions &= ~kOptionNetworkOn;
                if ( OptionKey()) DebugFileSave( "\p_Poopy");
                DebugFileCleanup();
#endif NETSPROCKET_AVAILABLE
                break;
            case kMainTrain:    // now replay intro
                    DoScrollText( (WindowPtr)gTheWindow, 5600, 4, kTitleTextScrollWidth,
                        kTitleFontNum, -1);
            /*
                whichScenario = GetScenarioNumberFromChapterNumber( 1);
                StopAndUnloadSong();
                if ( GetScenarioPrologueID( whichScenario) > 0)
                    DoScrollText( (WindowPtr)gTheWindow,
                        GetScenarioPrologueID( whichScenario),
                        4, kTitleTextScrollWidth, kTitleFontNum, 4002);

                gameResult = kNoGame;
                fadeColor.red = fadeColor.green = fadeColor.blue = 0;
                if ( gAresGlobal->gOptions & kOptionMusicIdle)
                {
                    AutoMusicFadeTo( 60, &fadeColor, FALSE);
                    StopAndUnloadSong();
                } else
                {
                    AutoFadeTo( 60, &fadeColor, FALSE);
                }

                do
                {
                    RemoveAllSpaceObjects();
                    gAresGlobal->gGameOver = 0;

                    // mission briefing unfades screen

                    if ( gAresGlobal->gOptions & kOptionMusicIdle)
                    {
                        LoadSong( 3000);
                        SetSongVolume( kMaxMusicVolume);
                        PlaySong();
                    }

                    ConstructScenario( whichScenario);

                    SetMBarState( false, theDevice);
                    if ( gAresGlobal->gOptions & kOptionMusicIdle)
                    {
                        StopAndUnloadSong();
                    }

                    HideCursor();
                    DrawInstrumentPanel( (WindowPtr) gTheWindow);
                    MacShowCursor();

                    if ( gAresGlobal->gOptions & kOptionMusicPlay)
                    {
                        LoadSong( gThisScenario->songID);
                        SetSongVolume( kMusicVolume);
                        PlaySong();
                    }
                    ResetLastTime( (gThisScenario->startTime & kScenario_StartTimeMask) * kScenarioTimeMultiple);
                    gameResult = PlayTheGame( &gameLength);

                    if (( !( gAresGlobal->gOptions & (kOptionAutoPlay | kOptionRecord | kOptionReplay))) && ( gameResult == kLoseGame))
                    {
                        if ( (gAresGlobal->gScenarioWinner & kScenarioWinnerTextMask) != kScenarioWinnerNoText)
                        {
                            DoMissionDebriefingText( (WindowPtr) gTheWindow,
                                (gAresGlobal->gScenarioWinner & kScenarioWinnerTextMask) >> kScenarioWinnerTextShift,
                                    -1, -1, -1, -1, -1, -1, -1);
                        }
                        if ( DoPlayAgain( false, false))
                        {
                            gameResult = kRestartGame;
                            WriteDebugLine((char *)"\pAGAIN!");
                        }  else
                        {
                            gameResult = kQuitGame;
                            WriteDebugLine((char *)"\pNOT AGAIN!");
                            BlackTitleScreen();
                            AutoFadeFrom( 1, false);
                        }
                    } else if ( gameResult == kWinGame)
                    {
                        fadeColor.red = fadeColor.green = fadeColor.blue = 0;
                        if ( gAresGlobal->gOptions & kOptionMusicPlay)
                        {
                            AutoMusicFadeTo( 60, &fadeColor, FALSE);
                            StopAndUnloadSong();
                        } else
                        {
                            AutoFadeTo( 60, &fadeColor, FALSE);
                        }

                        if ( gameResult == kWinGame)
                        {

                            BlackTitleScreen();
                            AutoFadeFrom( 1, FALSE);

                            if ( GetScenarioEpilogueID( whichScenario) > 0)
                                DoScrollText( (WindowPtr)gTheWindow,
                                    GetScenarioEpilogueID( whichScenario),
                                    4, kTitleTextScrollWidth, kTitleFontNum, 4002);

                        if ( gAresGlobal->gOptions & kOptionMusicIdle)
                            {
                                BlackTitleScreen();
                                StopAndUnloadSong();
                            }

                            if ( gAresGlobal->gOptions & kOptionReplay)
                            {
                                gRandomSeed = saveSeed;
                            }
                        }
                    }
                    if ( gAresGlobal->gOptions & kOptionMusicPlay) StopAndUnloadSong();
                } while ( (gameResult != kQuitGame) && ( gameResult != kWinGame) &&
                    ( GetChapterNumberFromScenarioNumber( whichScenario) <= kHackLevelMax)
                    && ( GetChapterNumberFromScenarioNumber( whichScenario) <= GetScenarioNumber())
                    && ( whichScenario >= 0) &&
                    ( !( gAresGlobal->gOptions & (kOptionAutoPlay | kOptionRecord | kOptionReplay))));
                if ( gAresGlobal->gOptions & kOptionMusicIdle)
                {
                    LoadSong( kTitleSongID);
                    SetSongVolume( kMaxMusicVolume);
                    PlaySong();
                }
*/              break;

            case kMainAbout:
//              if ( ShiftKey())
//                  DoMissionDebriefing( (WindowPtr) gTheWindow, 371, 123, 456, 789, 101, 112, 131);
                break;

        }
    }


/*  for ( hackCount = 0; hackCount < kTestNumber; hackCount++)
    {
        gAresGlobal->gGameTime = 0;
        RemoveAllSpaceObjects();
        DrawInstrumentPanel();
        MacSetRect( &bounds, 0, 0, WORLD_WIDTH, WORLD_HEIGHT);
        CopyOffWorldToRealWorld( (WindowPtr) gTheWindow, &bounds);
        gAresGlobal->gGameOver = -1;

        MakeOneScenario();

        ResetLastTime();
        HideCursor();
        PlayTheGame();
        MacShowCursor();
    }

    done = TRUE;
*/
}

short PlayTheGame( long *seconds)   // result 0 = lose, 1 = win, 2 = restart, 3 = quit

{
    unsigned long       decideCycle = 0;
    Str255              string;
    UnsignedWide        lastTime, thisTime, scrapTime, netTime;
    longRect                clipRect;
    long                    unitsToDo = 0, unitsPassed = 0, unitsDone = 0, resendTime = 0,
                            l1, l2, l3, newGameTime, lastclicktime = 0, scratch,
                            netCount = 0, additionalSeconds = 0;
    KeyMap              keyMap, lastKeyMap, lastMessageKeyMap;
    Boolean             playerPaused = FALSE, mouseDown = FALSE,
                            waitingForMessage = false,
                            blinkOn = false, enteringMessage = false,
                            afEntering = false, demoKey = false, newKeyMap = false, commandAndQ = false;
    unsigned long       *theseKeys, turnNum = 0, keyDataSize = 0,
                            scenarioCheckTime = 0, replayDataSize = 0;
    Rect                    playAreaRect;
    short                   result = -1, pauseLevel = 0;
    EventRecord         theEvent;
//  long                hacktc = TickCount(), hacktcsamplesize = 4, hacktcsamplecount = 0;

    DebugFileCleanup();
    DebugFileInit();
    DebugFileAppendString("\p<START DEBUG FILE>\r\r");

    commandAndQ = BothCommandAndQ();

    if ( gAresGlobal->gOptions & kOptionNetworkOn)
    {
#if NETSPROCKET_AVAILABLE
        ResetGotMessages( 0x7fffffff);
        if ( !JumpstartLatencyQueue(gAresGlobal->gGameTime, kDecideEveryCycles))
        {
//          ShowErrorAny( eContinueOnlyErr, -1, "\pEnding the game because ", "\pJumpstart failed.", nil, nil, -1, -1, -1, -1, __FILE__, 1);
            mAbortNetworkGame
            DeclareWinner( -1, -1, 300);
        }
#endif NETSPROCKET_AVAILABLE
    }
    SetSpriteCursorTable( 500);
    ShowSpriteCursor( true);
    ResetHintLine();
    MacSetRect( &playAreaRect, CLIP_LEFT, CLIP_TOP, CLIP_RIGHT, CLIP_BOTTOM);

    gAresGlobal->gLastKeys = gAresGlobal->gTheseKeys = 0;

    HideCursor();
    Microseconds( &lastTime);

    unitsPassed = 0;

    WriteDebugLine((char *)"\pEntr Game");
    WriteDebugLong( gRandomSeed);
    DebugFileAppendString("\p---NEW GAME---");
    if ( gAresGlobal->gOptions & kOptionRecord)
    {
        DebugFileAppendString("\p << RECORD >>");
    } else if ( gAresGlobal->gOptions & kOptionReplay)
    {
        DebugFileAppendString("\p << PLAY >>");
    }
    DebugFileAppendString("\p\r");

    DebugFileAppendString("\pTime\tFile\tLine\t#\r");

    if (( gAresGlobal->gOptions & ( kOptionRecord | kOptionReplay)) && ( gAresGlobal->gReplayData != nil))
    {
        MoveHHi( gAresGlobal->gReplayData);
        HLock( gAresGlobal->gReplayData);
        replayDataSize = (GetHandleSize( gAresGlobal->gReplayData) - (long)3) >> (long)2;
        theseKeys = (unsigned long *)*gAresGlobal->gReplayData;
        theseKeys++;
        if ( gAresGlobal->gOptions & kOptionRecord)
        {
            *theseKeys = 0;
            gAresGlobal->gLastKeys = gAresGlobal->gTheseKeys = 0;
            theseKeys++;
            *theseKeys = 0;
            theseKeys--;
        } else
        {
            turnNum = *theseKeys;
            theseKeys++;
        }
    } else
    {
        if ( gAresGlobal->user_is_scum)
        {
            ShowErrorAny( eQuitErr, -1, "\pCan't continue because an error of type "
                "-2021 occurred; contact Ambrosia Software for a valid "
                "registration code.", "\p", nil, nil, -1, -1, -1, -1, __FILE__, 1);
            ExitToShell();
        }
    }
    netTime.hi = netTime.lo = 0;
//  EMERGENCYHACKTEST = false;

    CheckScenarioConditions( 0);

/*  ExecuteCheat( 1, 0);
    ExecuteCheat( 1, 1);

    ExecuteCheat( 3, 0);
    ExecuteCheat( 3, 1);

    ExecuteCheat( 3, 0);
    ExecuteCheat( 3, 1);

    ExecuteCheat( 6, 0);
    ExecuteCheat( 6, 1);

    ExecuteCheat( 2, 0);
    ExecuteCheat( 2, 1);
*/
#if kProfiling_On
    if (ProfilerInit( collectSummary, bestTimeBase, 256, 8) != noErr)
        DebugStr("\pprofiler init error");
    ProfilerSetStatus( true);
#endif

    while (( gAresGlobal->gGameOver <= 0 ) && ( !gAresGlobal->returnToMain))
    {

        gAresGlobal->gFrameCount = 0;
        gLastTick = TickCount();

        SetLongRect( &clipRect, CLIP_LEFT, CLIP_TOP, CLIP_RIGHT, CLIP_BOTTOM);
        while (gAresGlobal->gGameOver <= 0)
        {
            EraseSpriteCursorSprite();
            EraseSpriteTable();
            EraseAllLabels();
            EraseSectorLines();
            PrepareToMoveScrollStars();
            EraseSite();

            while ( unitsPassed == 0)
            {
//              MyWideAdd( (wide *)&gAresGlobal->gLastTime, (wide *)&netTime);
                netTime.hi = netTime.lo = 0;
                Microseconds( &thisTime);
                scrapTime = thisTime;
                WideSubtract( (wide *)&thisTime, (wide *)&gAresGlobal->gLastTime);
                newGameTime = (thisTime.lo / kTimeUnit) + ((gThisScenario->startTime & kScenario_StartTimeMask) * kScenarioTimeMultiple);
//              newGameTime = gAresGlobal->gGameTime + Randomize( 7) + 1;//Randomize( kDecideEveryCycles);
#ifdef kConstantRate
                newGameTime = gAresGlobal->gGameTime + 1;
#endif
//#ifndef powerc
#ifdef powercxx
                if ( gAresGlobal->gOptions & kOptionNetworkOn)
                {
#endif
                    if ( (newGameTime - gAresGlobal->gGameTime) > k68KMaxFrameSkip)
                    {
                        newGameTime = gAresGlobal->gGameTime + k68KMaxFrameSkip;
                        l1 = newGameTime - ((gThisScenario->startTime & kScenario_StartTimeMask) * kScenarioTimeMultiple);
                        l2 = kTimeUnit;
                        MyWideMul( l1, l2, (wide *)&thisTime);
                        gAresGlobal->gLastTime = scrapTime;
                        WideSubtract( (wide *)&gAresGlobal->gLastTime, (wide *)&thisTime);
                    }
#ifdef powercxx
                }
#endif

//#endif
//              if ( (!(gAresGlobal->gOptions & kOptionReplay)) &&
//                  (mSlowMotionKey( keyMap)))
/*              if ( (mSlowMotionKey( keyMap)))
                {
                    demoKey = true;
                    if ( unitsPassed < 3) Pause( 3);
                    newGameTime = gAresGlobal->gGameTime + 1;
                    l1 = newGameTime - ((gThisScenario->startTime & kScenario_StartTimeMask) * kScenarioTimeMultiple);
                    l2 = kTimeUnit;
                    MyWideMul( l1, l2, (wide *)&thisTime);
                    gAresGlobal->gLastTime = scrapTime;
                    WideSubtract( (wide *)&gAresGlobal->gLastTime, (wide *)&thisTime);
                } else */if (((( gAresGlobal->gOptions & kOptionSubstituteFKeys) ?
                    ( mNOFFastMotionKey( keyMap)):( mFastMotionKey( keyMap)))) &&
                    (!enteringMessage))
                {
                    demoKey = true;
                    newGameTime = gAresGlobal->gGameTime + 12;
                    l1 = newGameTime - ((gThisScenario->startTime & kScenario_StartTimeMask) * kScenarioTimeMultiple);
                    l2 = kTimeUnit;
                    MyWideMul( l1, l2, (wide *)&thisTime);
                    gAresGlobal->gLastTime = scrapTime;
                    WideSubtract( (wide *)&gAresGlobal->gLastTime, (wide *)&thisTime);
                }/* else
                {
                    newGameTime = gAresGlobal->gGameTime + Randomize( 9) + 1;
                    l1 = newGameTime - ((gThisScenario->startTime & kScenario_StartTimeMask) * kScenarioTimeMultiple);
                    l2 = kTimeUnit;
                    MyWideMul( l1, l2, (wide *)&thisTime);
                    gAresGlobal->gLastTime = scrapTime;
                    WideSubtract( (wide *)&gAresGlobal->gLastTime, (wide *)&thisTime);
                }*/
//              if (( newGameTime - gAresGlobal->gGameTime) > 6) newGameTime = gAresGlobal->gGameTime + 6;

                if ( newGameTime >= kMaxGameTime)
                {
                    l1 = kTimeUnit;
                    l2 = newGameTime - kMaxGameTime;
                    MyWideMul( l1, l2, (wide *)&thisTime);
                    gAresGlobal->gLastTime = scrapTime;
                    WideSubtract( (wide *)&gAresGlobal->gLastTime, (wide *)&thisTime);
                    additionalSeconds += ( newGameTime / 60);
                    newGameTime -= kMaxGameTime;
//                  gAresGlobal->gGameTime -= kMaxGameTime;
                }
                if ( gAresGlobal->gGameTime <= newGameTime)
                    unitsDone = unitsPassed = newGameTime - gAresGlobal->gGameTime;
                else
                    unitsDone = unitsPassed = newGameTime - ( gAresGlobal->gGameTime - kMaxGameTime);
/*              if ( gAresGlobal->gOptions & ( kOptionReplay | kOptionRecord))
                {
                    if ( unitsDone > kDecideEveryCycles)
                    {
                        gAresGlobal->gGameTime = newGameTime - kDecideEveryCycles;
                        unitsDone = unitsPassed = kDecideEveryCycles;
                    }
                }
*/
//              WideSubtract( (wide *)&thisTime, (wide *)&lastTime);
//              unitsDone = unitsPassed = thisTime.lo / kTimeUnit;
//              thisTick = TickCount();
//              unitsDone = unitsPassed = thisTick - lastTick;
//              newGameTime = TickCount() - gAresGlobal->gGameStartTime;
//              unitsDone = unitsPassed = newGameTime - gAresGlobal->gGameTime;
            }
//          Microseconds( &lastTime);   // don't activate
//          lastTick = thisTick;

            if ( playerPaused)
            {
                playerPaused = false;
                unitsDone = unitsPassed = 0;
                newGameTime = gAresGlobal->gGameTime;
                l1 = newGameTime - ((gThisScenario->startTime & kScenario_StartTimeMask) * kScenarioTimeMultiple);
                l2 = kTimeUnit;
                MyWideMul( l1, l2, (wide *)&thisTime);
                gAresGlobal->gLastTime = scrapTime;
                WideSubtract( (wide *)&gAresGlobal->gLastTime, (wide *)&thisTime);
            }

            if ( gAresGlobal->gGameOver < 0)
            {
                gAresGlobal->gGameOver += unitsPassed;
                if ( gAresGlobal->gGameOver == 0)
                    gAresGlobal->gGameOver = 1;
            }

            while ( unitsPassed > 0)
            {
                unitsToDo = unitsPassed;
                if ( unitsToDo > kMaxTimePerCycle) unitsToDo = kMaxTimePerCycle;
                if ( (decideCycle + unitsToDo) > kDecideEveryCycles)
                    unitsToDo = kDecideEveryCycles - decideCycle;
                decideCycle += unitsToDo;

                if ( unitsToDo > 0) // executed arbitrarily, but at least once every kDecideEveryCycles
                {

                    MoveScrollStars( unitsToDo);
                    MoveSpaceObjects( (spaceObjectType *)*gSpaceObjectData, kMaxSpaceObject,
                                    unitsToDo);

//                  WriteDebugLine((char *)"\pMove");
//                  WriteDebugLong( decideCycle);
//                  WriteDebugLong( unitsToDo);
                }

                gAresGlobal->gGameTime += unitsToDo;
                if ( gAresGlobal->gGameTime >= kMaxGameTime) gAresGlobal->gGameTime -= kMaxGameTime;
                if ( decideCycle == kDecideEveryCycles) // everything in here gets executed once every kDecideEveryCycles
                {
//                  MoveScrollStars( kDecideEveryCycles);
//                  MoveSpaceObjects( (spaceObjectType *)*gSpaceObjectData, kMaxSpaceObject,
//                                  kDecideEveryCycles);

                    playerPaused = FALSE;


                    NonplayerShipThink( kDecideEveryCycles);
                    AdmiralThink();
                    ExecuteActionQueue( kDecideEveryCycles);

                    if ( gAresGlobal->gOptions & kOptionReplay)
                    {
                        while ( turnNum == 0)
                        {
                            theseKeys++;
                            turnNum = *theseKeys;
                            turnNum++;
                            theseKeys++;
                            keyDataSize += 2;
                            if ( keyDataSize >= replayDataSize)
                            {
//                              ShowErrorAny( eContinueOnlyErr, -1, "\pEnding the game because", "\p there aren't anymore keystrokes.", nil, nil, -1, -1, -1, -1, __FILE__, 31);
                                gAresGlobal->gGameOver = 1;
                            }
                        }

                        if ( !playerPaused) playerPaused =
                            PlayerShipGetKeys( kDecideEveryCycles,
                                *theseKeys, &enteringMessage);
                        else
                            PlayerShipGetKeys( kDecideEveryCycles,
                                *theseKeys, &enteringMessage);

                        turnNum--;
                    } else
                    {
                        if ( !playerPaused) playerPaused =
                            PlayerShipGetKeys( kDecideEveryCycles,
                                0xffffffff, &enteringMessage);
                        else PlayerShipGetKeys( kDecideEveryCycles, 0xffffffff,
                            &enteringMessage);

                        if ( gAresGlobal->gOptions & kOptionRecord)
                        {
                            if ( gAresGlobal->gTheseKeys == gAresGlobal->gLastKeys)
                            {
                                (*theseKeys)++;
                            } else
                            {
                                if ( keyDataSize < kReplayBufferSize)
                                {
//                                  WriteDebugDivider();
//                                  WriteDebugLong( *theseKeys);
//                                  WriteDebugLong( keyDataSize);
                                    theseKeys++;
                                    *theseKeys = gAresGlobal->gLastKeys;
                                    theseKeys++;
                                    *theseKeys = 0;
                                    keyDataSize += 2;
                                } else
                                {
//                                  ShowErrorAny( eContinueOnlyErr, -1, "\pEnding the game because", "\p I can't record any more keystrokes.", nil, nil, -1, -1, -1, -1, __FILE__, 32);
                                    gAresGlobal->gGameOver = 1;
                                }
                            }
                        }
                    }

                    if ( Button())
                    {
                        if ( gAresGlobal->gOptions & kOptionReplay)
                        {
                            result = kQuitGame;
                            gAresGlobal->gGameOver = 1;
                        } else
                        {
                            if ( !mouseDown)
                            {
                                if ( !(gAresGlobal->gOptions & ( kOptionAutoPlay | kOptionReplay | kOptionRecord)))
                                {
                                    if ((( gAresGlobal->gGameTime - lastclicktime)) <= GetDblTime())
                                    {
                                        InstrumentsHandleDoubleClick();
                                        lastclicktime -= GetDblTime();
                                    } else
                                    {
                                        InstrumentsHandleClick();
                                        lastclicktime = gAresGlobal->gGameTime;
                                    }
                                }
                                mouseDown = TRUE;
                            } else
                            {
                                InstrumentsHandleMouseStillDown();
                            }
                        }
                    } else if ( mouseDown)
                    {
                        mouseDown = FALSE;
                        InstrumentsHandleMouseUp();
                    }

                    if ( gAresGlobal->gOptions & kOptionNetworkOn)
                    {
//#ifdef kDemoTimeLimit
if ( (!Ambrosia_Is_Registered()) || ( GetOpponentIsUnregistered()))
{
                        if ( gAresGlobal->gGameTime > kTimeLimitWarning)
                        {
                            if ( gAresGlobal->gGameTime > kTimeLimit)
                            {
                                result = kQuitGame;
                                gAresGlobal->gGameOver = 1;
                                DeclareWinner( -1, -1, 302);
                            } else if ( !GetHaveSeenUnregisteredTimeLimitWarning())
                            {
                                SetHaveSeenUnregisteredTimeLimitWarning( true);
                                StartLongMessage( 6123, 6123);

                            }
                        }
}
//#endif
#if NETSPROCKET_AVAILABLE
                        l2 = -1;
                        waitingForMessage = false;
                        resendTime = kResendRequestFirstTime;
                        l1 = l3 = TickCount();
                        afEntering = enteringMessage;
                        Microseconds( &thisTime);   // don't activate
//                      TickleOutgoingMessage( false);
                        if ( SendInGameMessage( gAresGlobal->gGameTime + gNetLatency))
                        {
                            do
                            {
                                GetKeys( keyMap);
                                HandleTextMessageKeys( keyMap, lastMessageKeyMap, &afEntering);
                                for ( scratch = 0; scratch < 4; scratch++)
                                    lastMessageKeyMap[scratch] = keyMap[scratch];
                                if ( !ProcessInGameMessages( gAresGlobal->gGameTime, &pauseLevel))
                                {
//                                  ShowErrorAny( eContinueOnlyErr, -1, "\pEnding the game because ", "\pProcessInGamesMessages failed.", nil, nil, -1, -1, -1, -1, __FILE__, 2);
//                                  mAbortNetworkGame
                                    if ( gAresGlobal->gGameOver == 0)
                                    {
                                        gAresGlobal->gGameOver = 1;
                                        result = kRestartGame;
                                    }

                                    mWriteDebugString("\pPROC ERR");
                                    if ( (gAresGlobal->gScenarioWinner &
                                        kScenarioWinnerTextMask) == kScenarioWinnerNoText)
                                        DeclareWinner( -1, -1,  6004);
                                }

                                // *** START HANDLING NETWORK PAUSE ***
                                if ((mRestartResumeKey( keyMap)) || ((!commandAndQ) &&
                                    ( mQuitKeys( keyMap))))
                                {
                                    if ( blinkOn)
                                    {
                                        StopPauseIndicator( string);
                                        blinkOn = false;
                                    }
                                    if ( !( gAresGlobal->gOptions & (kOptionAutoPlay | kOptionRecord | kOptionReplay)))
                                    {
                                        if ( gAresGlobal->gOptions & kOptionNetworkOn)
                                        {
                                            if ( !SendInGameBasicMessage( 0, eStartPauseMessage,
                                                true, false))
                                            {
//                                              ShowErrorAny( eContinueOnlyErr, -1, "\pEnding the game because ", "\pStartPause failed.", nil, nil, -1, -1, -1, -1, __FILE__, 3);
                                                mAbortNetworkGame
                                                mWriteDebugString("\pPSE SEND ERR");
                                                DeclareWinner( -1, -1, 300);
                                            }
                                        }

                                        MacShowCursor();
                                        RestoreOriginalColors();
                                        switch ( DoPlayAgain( true,
                                            (gThisScenario->startTime & kScenario_IsTraining_Bit) ? (true):(false)))
                                        {
                                            case 0: // quit
//                                              ShowErrorAny( eContinueOnlyErr, -1, "\pEnding the game because", "\p the user chose to quit.", nil, nil, -1, -1, -1, -1, __FILE__, 33);
                                                result = kQuitGame;
                                                gAresGlobal->gGameOver = 1;
                                                if ( CommandKey())
                                                    gAresGlobal->gScenarioWinner = gAresGlobal->gPlayerAdmiralNumber;
                                                gAresGlobal->gScenarioWinner |= kScenarioWinnerNoNext | kScenarioWinnerNoText;
                                                break;

                                            case 1: // restart
//                                              ShowErrorAny( eContinueOnlyErr, -1, "\pEnding the game because", "\p the user chose to restart.", nil, nil, -1, -1, -1, -1, __FILE__, 34);
                                                result = kRestartGame;
                                                gAresGlobal->gGameOver = 1;
                                                if ( CommandKey())
                                                    gAresGlobal->gScenarioWinner = gAresGlobal->gPlayerAdmiralNumber;
                                                gAresGlobal->gScenarioWinner |= kScenarioWinnerNoNext | kScenarioWinnerNoText;
                                                break;

                                            case 2: // resume
                                                break;

                                            case 3: // skip
                                                result = kWinGame;
                                                gAresGlobal->gGameOver = 1;
                                                gAresGlobal->gScenarioWinner =  gAresGlobal->gPlayerAdmiralNumber |
                                                    (( GetChapterNumberFromScenarioNumber(
                                                        gAresGlobal->gThisScenarioNumber)
                                                            +1) << kScenarioWinnerNextShift) |
                                                    kScenarioWinnerNoText;
                                                break;
                                        }
                                        CopyOffWorldToRealWorld( (WindowPtr)gTheWindow, &playAreaRect);
                                        HideCursor();
                                        playerPaused = true;
                                        if ( gAresGlobal->gOptions & kOptionNetworkOn)
                                        {
                                            if ( !SendInGameBasicMessage( 0, eEndPauseMessage,
                                                true, false))
                                            {
//                                              ShowErrorAny( eContinueOnlyErr, -1, "\pEnding the game because ", "\pEndPause failed.", nil, nil, -1, -1, -1, -1, __FILE__, 4);
                                                mAbortNetworkGame
                                                mWriteDebugString("\pPSE END ERR");
                                                DeclareWinner( -1, -1, 300);
                                            }
                                            mWriteDebugString("\pSent End Pause");
                                        }
                                    } else
                                    {
//                                      ShowErrorAny( eContinueOnlyErr, -1, "\pEnding the game because", "\p this isn't a real game and the user hit ESC or COMMAND-Q.", nil, nil, -1, -1, -1, -1, __FILE__, 35);
                                        result = kQuitGame;
                                        gAresGlobal->gGameOver = 1;
                                        if ( CommandKey())
                                            gAresGlobal->gScenarioWinner = gAresGlobal->gPlayerAdmiralNumber;
                                        gAresGlobal->gScenarioWinner |= kScenarioWinnerNoNext | kScenarioWinnerNoText;
                                    }
                                    l1 = TickCount();
                                }
                                // *** END HANDLING NETWORK PAUSE ***

                                if (( pauseLevel > 0) && ( l2 < 0))
                                {
                                    if ( blinkOn)
                                    {
                                        StopPauseIndicator( string);
                                    }
                                    blinkOn = false;
                                    playerPaused = true;
                                    l2 = TickCount();
                                }// else if  ( pauseLevel <= 0) l2 = -1;

                                /*
                                if (( pauseLevel > 0) && (!wasPaused))
                                {
                                    playerPaused = true;
                                    wasPaused = true;
                                    blinkOn = false;
                                    l2 = TickCount();
                                }
                                if ( (wasPaused) && (pauseLevel <= 0))
                                {
                                    wasPaused = false;
                                }
                                */

                                if ( pauseLevel > 0)
                                {
                                    l1 = l3 = TickCount();
                                    if ( (TickCount() - l2) > 20)
                                    {
                                        if ( blinkOn)
                                        {
//                                          GetIndString( string, 3100, 12);
                                            StopPauseIndicator( string);
                                            blinkOn = false;
                                        } else
                                        {
                                            GetIndString( string, 3100, 12);
                                            RestoreOriginalColors();
                                            StartPauseIndicator( string, Randomize(16));
                                            blinkOn = true;
                                        }
                                        l2 = TickCount();
                                    }
                                }

                                if ( (IAmHosting()) && ( GetNumberOfPlayers() < 2))
                                {
                                    if ( gAresGlobal->gGameOver == 0)
                                    {
//                                      ShowErrorAny( eContinueOnlyErr, -1, "\pEnding the game because", "\p I am the host and there are fewer than 2 players.", nil, nil, -1, -1, -1, -1, __FILE__, 22);
                                        DeclareWinner( gAresGlobal->gPlayerAdmiralNumber, -1, 6004);
                                        gAresGlobal->gGameOver = 1;
                                    }
                                    result = kQuitGame;
                                }

                                if ( !NetGameIsOn())
                                {
                                    if ( gAresGlobal->gGameOver == 0)
                                    {
//                                      ShowErrorAny( eContinueOnlyErr, -1, "\pEnding the game because", "\p there is no network game.", nil, nil, -1, -1, -1, -1, __FILE__, 23);
                                        DeclareWinner( gAresGlobal->gPlayerAdmiralNumber, -1, 6004);
                                        gAresGlobal->gGameOver = 1;
                                    }
                                    result = kQuitGame;
                                } else
                                {
                                    if ((( TickCount() - l3) > 60) && ( pauseLevel <= 0))
                                    {
                                        if ( l2 < 0)
                                        {
                                            l2 = TickCount();
                                            if ( blinkOn)
                                            {
                                                StopPauseIndicator( string);
                                                blinkOn = false;
                                            } else
                                            {
                                                GetIndString( string, 3100, 13);
                                                RestoreOriginalColors();
                                                StartPauseIndicator( string, Randomize(16));
                                                blinkOn = true;
                                            }

                                        }
                                        if (( TickCount() - l2) > 20)
                                        {
                                            if (blinkOn)
                                            {
//                                              GetIndString( string, 3100, 13);
                                                StopPauseIndicator( string);
                                                blinkOn = false;
                                            } else
                                            {
                                                GetIndString( string, 3100, 13);
                                                RestoreOriginalColors();
                                                StartPauseIndicator( string, Randomize(16));
                                                blinkOn = true;
                                            }
                                            l2 = TickCount();
                                        }
                                    }
                                    scratch = TickCount() - l1;
                                    if ( scratch > GetResendDelay())
                                    {
                                        SendResendMessage( gAresGlobal->gGameTime);
                                        l1 = TickCount();

                                        if ( GetRegisteredSetting() > 0)
                                            resendTime = GetResendDelay() * 20;
                                        else
                                            resendTime = GetResendDelay() * 2;
                                    }
                                }
                            } while (((pauseLevel > 0) || ( !GotAllMessages())) &&
                                ( result == -1));
                        } else
                        {
//                          ShowErrorAny( eContinueOnlyErr, -1, "\pEnding the game because", "\p SendInGameMessage failed.", nil, nil, -1, -1, -1, -1, __FILE__, 5);
                            mAbortNetworkGame
                            mWriteDebugString("\pSEND MSG ERR");
                            DeclareWinner( -1, -1, 300);
                        }

                        Microseconds( &netTime);    // don't activate
                        WideSubtract( (wide *)&netTime, (wide *)&thisTime);
                        if ( netTime.lo > kTimeUnit)
                        {
                            netCount++;
                            if ( netCount > kFractionalLagCorrectTolerance)
                            {
                                playerPaused = true;
                                netCount = 0;
                            }
                        } else netCount = 0;

                        if ( blinkOn)
                        {
                            StopPauseIndicator( string);
                            blinkOn = false;
                        }
                        ExecuteInGameData();
                        ResetGotMessages( gAresGlobal->gGameTime);
//                      UseNextLatency();
#endif NETSPROCKET_AVAILABLE
                    }

                    CollideSpaceObjects( (spaceObjectType *)*gSpaceObjectData, kMaxSpaceObject);
                    decideCycle  = 0;
                    scenarioCheckTime++;
                    if ( scenarioCheckTime == 30)
                    {
                        scenarioCheckTime = 0;
                        CheckScenarioConditions( 0);
                    }
//                  WriteDebugLine((char *)"\pDecide");
                }
                unitsPassed -= unitsToDo;
            }

//          CollideSpaceObjects( (spaceObjectType *)*gSpaceObjectData, kMaxSpaceObject);
            newKeyMap = false;
            for ( l1 = 0; l1 < 4; l1++)
            {
                lastKeyMap[l1] = keyMap[l1];
            }
            GetKeys( keyMap);
            for ( l1 = 0; l1 < 4; l1++)
            {
#if TARGET_OS_MAC
                if ( lastKeyMap[l1] != keyMap[l1]) newKeyMap = true;
#else
                if ( lastKeyMap[l1].bigEndianValue != keyMap[l1].bigEndianValue) newKeyMap = true;
#endif TARGET_OS_MAC
            }
            if ( !(gAresGlobal->gOptions & kOptionNetworkOn))
            {
#if NETSPROCKET_AVAILABLE
                afEntering = enteringMessage;
                HandleTextMessageKeys( keyMap, lastMessageKeyMap, &afEntering);
                for ( scratch = 0; scratch < 4; scratch++)
                    lastMessageKeyMap[scratch] = keyMap[scratch];
#endif
            }
            if (
                //(!(gAresGlobal->gOptions & kOptionReplay)) &&
                (mPauseKey( keyMap)))
            {
                mWriteDebugString("\pThis Time:");
                WriteDebugLong( newGameTime);
                WriteDebugLong( gAresGlobal->gGameTime);
#if NETSPROCKET_AVAILABLE
                DebugMessageQueue();
#endif
                RestoreOriginalColors();
                GetIndString( string, 3100, 11);
                if ( gAresGlobal->gOptions & kOptionNetworkOn)
                {
#if NETSPROCKET_AVAILABLE
                    if ( !SendInGameBasicMessage( 0, eStartPauseMessage, true,
                        false))
                    {
//                      ShowErrorAny( eContinueOnlyErr, -1, "\pEnding the game because", "\p StartPause failed.", nil, nil, -1, -1, -1, -1, __FILE__, 6);
                        mAbortNetworkGame
                        DeclareWinner( -1, -1, 300);
                    }
#endif NETSPROCKET_AVAILABLE
                }

                PlayVolumeSound( kComputerBeep4, kMaxSoundVolume, kShortPersistence, kMustPlaySound);
                while ( (mPauseKey( keyMap)) && (!(mReturnKey(keyMap))))
                {

                    l1 = TickCount();
                    StartPauseIndicator( string, Randomize( 16));
                    playerPaused = false;
                    while ( (mPauseKey( keyMap)) && (!(mReturnKey(keyMap)))
                        && ( (TickCount() - l1) < 20))
                    {
                        GetKeys( keyMap);
                    }


                    l1 = TickCount();
                    StopPauseIndicator( string);
                    playerPaused = true;
                    while ( (mPauseKey( keyMap)) && (!(mReturnKey(keyMap)))
                        && ( (TickCount() - l1) < 20))
                    {
                        GetKeys( keyMap);
                        if ( CommandKey())
                        {
                            Ares_WaitNextEvent (everyEvent, &theEvent, 3, nil);
                        }
                    }

                }
                if ( gAresGlobal->gOptions & kOptionNetworkOn)
                {
#if NETSPROCKET_AVAILABLE
                    if ( !SendInGameBasicMessage( 0, eEndPauseMessage, true,
                        false))
                    {
//                      ShowErrorAny( eContinueOnlyErr, -1, "\pEnding the game because", "\p Endpause failed.", nil, nil, -1, -1, -1, -1, __FILE__, 7);
                        mAbortNetworkGame
                        DeclareWinner( -1, -1, 300);
                    }
                    mWriteDebugString("\pSent End Pause");
#endif
                }
            }

            if ((!( gAresGlobal->gOptions & kOptionNetworkOn)) &&
                ( !(gAresGlobal->gOptions & kOptionReplay)) &&
                ((mRestartResumeKey( keyMap)) || ((!commandAndQ) && ( mQuitKeys( keyMap)))))
            {
                if ( !( gAresGlobal->gOptions & (kOptionAutoPlay | kOptionRecord | kOptionReplay)))
                {
                    if ( gAresGlobal->gOptions & kOptionNetworkOn)
                    {
#if NETSPROCKET_AVAILABLE
                        if ( !SendInGameBasicMessage( 0, eStartPauseMessage, true,
                            false))
                        {
//                          ShowErrorAny( eContinueOnlyErr, -1, "\pEnding the game because", "\p Startpause failed.", nil, nil, -1, -1, -1, -1, __FILE__, 8);
                            mAbortNetworkGame
                            DeclareWinner( -1, -1, 300);
                        }
#endif NETSPROCKET_AVAILABLE
                    }

                    RestoreOriginalColors();
                    MacShowCursor();
                    switch ( DoPlayAgain( true,
                                            (gThisScenario->startTime & kScenario_IsTraining_Bit) ? (true):(false)))
                    {
                        case 0: // quit
//                          ShowErrorAny( eContinueOnlyErr, -1, "\pEnding the game because", "\p the user chose to quit.", nil, nil, -1, -1, -1, -1, __FILE__, 35);
                            result = kQuitGame;
                            gAresGlobal->gGameOver = 1;
                            if ( CommandKey())
                                gAresGlobal->gScenarioWinner = gAresGlobal->gPlayerAdmiralNumber;
                            gAresGlobal->gScenarioWinner |= kScenarioWinnerNoNext | kScenarioWinnerNoText;
                            break;

                        case 1: // restart
//                          ShowErrorAny( eContinueOnlyErr, -1, "\pEnding the game because", "\p the user chose to restart.", nil, nil, -1, -1, -1, -1, __FILE__, 36);
                            result = kRestartGame;
                            gAresGlobal->gGameOver = 1;
                            if ( CommandKey())
                                gAresGlobal->gScenarioWinner = gAresGlobal->gPlayerAdmiralNumber;
                            gAresGlobal->gScenarioWinner |= kScenarioWinnerNoNext | kScenarioWinnerNoText;
                            break;

                        case 2: // resume
                            break;

                        case 3: // skip
                            result = kWinGame;
                            gAresGlobal->gGameOver = 1;
                            gAresGlobal->gScenarioWinner =  gAresGlobal->gPlayerAdmiralNumber |
                                (( GetChapterNumberFromScenarioNumber(gAresGlobal->gThisScenarioNumber)+1)
                                    << kScenarioWinnerNextShift) |
                                kScenarioWinnerNoText;
                            break;
                    }
                    CopyOffWorldToRealWorld( (WindowPtr)gTheWindow, &playAreaRect);
                    HideCursor();
                    playerPaused = true;
                    if ( gAresGlobal->gOptions & kOptionNetworkOn)
                    {
#if NETSPROCKET_AVAILABLE
                        if ( !SendInGameBasicMessage( 0, eEndPauseMessage, true,
                            false))
                        {
//                          ShowErrorAny( eContinueOnlyErr, -1, "\pEnding the game because", "\p EndPause failed.", nil, nil, -1, -1, -1, -1, __FILE__, 8);
                            mAbortNetworkGame
                            DeclareWinner( -1, -1, 300);
                        }
                        mWriteDebugString("\pSent End Pause");
#endif NETSPROCKET_AVAILABLE
                    }
                } else
                {
//                  ShowErrorAny( eContinueOnlyErr, -1, "\pEnding the game because", "\p this isn't a real game and the user hit ESC or COMMAND-Q.", nil, nil, -1, -1, -1, -1, __FILE__, 37);
                    result = kQuitGame;
                    gAresGlobal->gGameOver = 1;
                    if ( CommandKey())
                        gAresGlobal->gScenarioWinner = gAresGlobal->gPlayerAdmiralNumber;
                    gAresGlobal->gScenarioWinner |= kScenarioWinnerNoNext | kScenarioWinnerNoText;
                }
            }

            if ((!(gAresGlobal->gOptions & kOptionReplay)) &&
                (((!afEntering)&&( mHelpKey( keyMap)))))

            {
                if ( gAresGlobal->gOptions & kOptionNetworkOn)
                {
#if NETSPROCKET_AVAILABLE
                    if ( !SendInGameBasicMessage( 0, eStartPauseMessage, true,
                        false))
                    {
//                      ShowErrorAny( eContinueOnlyErr, -1, "\pEnding the game because", "\p StartPause failed.", nil, nil, -1, -1, -1, -1, __FILE__, 9);
                        mAbortNetworkGame
                        DeclareWinner( -1, -1, 300);
                    }
#endif NETSPROCKET_AVAILABLE
                }
                RestoreOriginalColors();
                MacShowCursor();
                DoHelpScreen();
                HideCursor();
                CopyOffWorldToRealWorld( (WindowPtr)gTheWindow, &playAreaRect);
                playerPaused = true;
                if ( gAresGlobal->gOptions & kOptionNetworkOn)
                {
#if NETSPROCKET_AVAILABLE
                    if ( !SendInGameBasicMessage( 0, eEndPauseMessage, true,
                        false))
                    {
//                      ShowErrorAny( eContinueOnlyErr, -1, "\pEnding the game because", "\p Endpause failed.", nil, nil, -1, -1, -1, -1, __FILE__, 10);
                        mAbortNetworkGame
                        DeclareWinner( -1, -1, 300);
                    }
                    mWriteDebugString("\pSent End Pause");
#endif NETSPROCKET_AVAILABLE
                }
            }
            if ((gAresGlobal->gOptions & kOptionNetworkOn) &&
#if NETSPROCKET_AVAILABLE
            ( IAmHosting()) &&
#endif NETSPROCKET_AVAILABLE
                    (((!afEntering)&&( mNetSettingsKey( keyMap)))))
            {
                if ( gAresGlobal->gOptions & kOptionNetworkOn)
                {
#if NETSPROCKET_AVAILABLE
                    if ( !SendInGameBasicMessage( 0, eStartPauseMessage, true,
                        false))
                    {
//                      ShowErrorAny( eContinueOnlyErr, -1, "\pEnding the game because", "\p Startpause failed.", nil, nil, -1, -1, -1, -1, __FILE__, 11);
                        mAbortNetworkGame
                        DeclareWinner( -1, -1, 300);
                    }
#endif NETSPROCKET_AVAILABLE
                }
                RestoreOriginalColors();
                MacShowCursor();
#if NETSPROCKET_AVAILABLE
                DoNetSettings();
#endif NETSPROCKET_AVAILABLE
                HideCursor();
                CopyOffWorldToRealWorld( (WindowPtr)gTheWindow, &playAreaRect);
                playerPaused = true;
                if ( gAresGlobal->gOptions & kOptionNetworkOn)
                {
#if NETSPROCKET_AVAILABLE
                    if ( !SendInGameBasicMessage( 0, eEndPauseMessage, true,
                        false))
                    {
//                      ShowErrorAny( eContinueOnlyErr, -1, "\pEnding the game because", "\p EndPause failed.", nil, nil, -1, -1, -1, -1, __FILE__, 12);
                        mAbortNetworkGame
                        DeclareWinner( -1, -1, 300);
                    }
                    mWriteDebugString("\pSent End Pause");
#endif NETSPROCKET_AVAILABLE
                }
            }

            if (((!(gAresGlobal->gOptions & kOptionReplay)) &&
            ((!afEntering) && ( ( mVolumeDownKey( keyMap) && !mVolumeDownKey( lastKeyMap))))))
            {
                if ( gAresGlobal->gSoundVolume > 0) gAresGlobal->gSoundVolume--;
                if ( gAresGlobal->gOptions & kOptionMusicPlay)
                {
                    SetSongVolume( kMusicVolume);
                }
            }

            if (((!(gAresGlobal->gOptions & kOptionReplay)) &&
                ((!afEntering) && ( mVolumeUpKey( keyMap) && !mVolumeUpKey( lastKeyMap)))))
            {
                if ( gAresGlobal->gSoundVolume < kMaxVolumePreference) gAresGlobal->gSoundVolume++;
                if ( gAresGlobal->gOptions & kOptionMusicPlay)
                {
                    SetSongVolume( kMusicVolume);
                }
            }

            if ((!(gAresGlobal->gOptions & kOptionReplay)) && (!afEntering) &&
                ( mActionMusicKey( keyMap)) && ( !mActionMusicKey( lastKeyMap)))
            {
                if ( gAresGlobal->gOptions & kOptionMusicPlay)
                {
                    ToggleSong();
                }
            }

#if TARGET_OS_MAC
            keyMap[3] &= ~0x80; // mask out power key
            keyMap[1] &= ~0x02; // mask out caps lock key
            if ( (gAresGlobal->gOptions & kOptionReplay) && (!demoKey) &&
                (!newKeyMap) &&
                ((keyMap[0] != 0) || ( keyMap[1] != 0) || ( keyMap[2] != 0) ||
                (keyMap[3] != 0)))
#else
            keyMap[3].bigEndianValue &= EndianU32_NtoB(~0x80);  // mask out power key
            keyMap[1].bigEndianValue &= EndianU32_NtoB(~0x02);  // mask out caps lock key
            if ( (gAresGlobal->gOptions & kOptionReplay) && (!demoKey) &&
                (!newKeyMap) &&
                ((keyMap[0].bigEndianValue != 0) || ( keyMap[1].bigEndianValue != 0) ||
                ( keyMap[2].bigEndianValue != 0) ||
                (keyMap[3].bigEndianValue != 0)))
#endif TARGET_OS_MAC
            {
                result = kQuitGame;
                gAresGlobal->gGameOver = 1;
            }
            demoKey = false;

//          if ( gAresGlobal->gGameTime > newGameTime) { WriteDebugLong( newGameTime); gAresGlobal->gGameTime = newGameTime;}

            MiniComputerHandleNull( unitsDone);

            ClipToCurrentLongMessage();
            SetLongRect( &clipRect, CLIP_LEFT, CLIP_TOP, CLIP_RIGHT, CLIP_BOTTOM);
//          hacktc = TickCount();
//          hacktcsamplecount++;
            DrawScrollStars( TRUE);
            DrawCurrentLongMessage( unitsDone);

            DrawSectorLines();
            DrawAllBeams();
            DrawSpriteTableInOffWorld( &clipRect);
            UpdateAllLabelPositions( unitsDone);
            DrawAllLabels();
            DrawSite();
            SetLongRect( &clipRect, 0, 0, WORLD_WIDTH, WORLD_HEIGHT);
            DrawSpriteCursorSprite( &clipRect);
//          DrawCurrentLongMessage();

            if (( gAresGlobal->gOptions & kOptionQDOnly))
            {
                RGBColor    hackcolor = { 32765, 32765, 32765};

                DrawOutlinedString( "\pkOptionQDOnly", &hackcolor); //DrawString( tempString);
                ShowSpriteCursorSprite();
                DrawAllBeams();
                DontShowScrollStars();
                CopyOffWorldToRealWorld( (WindowPtr)gTheWindow, &playAreaRect);
            } else
            {
                ShowSpriteCursorSprite();
                ShowSpriteTable();
                ShowAllLabels();
//              DrawAllBeams();
                ShowAllBeams();
                ShowScrollStars( TRUE);
                ShowSectorLines();
                ShowSite();
                CopyOffWorldToRealWorld( (WindowPtr)gTheWindow, &playAreaRect);

            }
//          if ( hacktcsamplecount > hacktcsamplesize)
//          {
//              hacktcsamplecount = 0;
//              WriteDebugLong( TickCount() - hacktc);
//          }
            //PixMapTest();

            DrawMessageScreen( unitsDone);
            UpdateRadar( unitsDone);
            UpdateBooleanColorAnimation( unitsDone);
//          CheckScenarioConditions( unitsDone);
            gAresGlobal->gFrameCount++;
        }
    }

#if kProfiling_On
    ProfilerSetStatus( false);
    if (ProfilerDump("\pares_profile") != noErr)
        DebugStr("\pprofiler dump error");
    ProfilerTerm();
#endif

    if ( gAresGlobal->gOptions & kOptionNetworkOn)
    {
#if NETSPROCKET_AVAILABLE
        if ( IAmHosting())
        {
//          StopNetworking();
            if ( !SendInGameMiscLongMessage( 0, eStopPlayingMessage,
                gAresGlobal->gScenarioWinner, true, false))
            {
                mAbortNetworkGame
            }
        } else
        {
//          StopNetworking();
            if ( !SendInGameMiscLongMessage( 0, eStopPlayingMessage,
                gAresGlobal->gScenarioWinner, true, false))
            {
                mAbortNetworkGame
            }
        }
#endif NETSPROCKET_AVAILABLE
    }

    if ( gAresGlobal->gOptions & kOptionRecord)
    {
                theseKeys++;
                *theseKeys = gAresGlobal->gLastKeys;
        if ( gAresGlobal->gReplayData != nil)
        {
            HUnlock( gAresGlobal->gReplayData);
            SetHandleSize( gAresGlobal->gReplayData, (keyDataSize + 1L) * sizeof( long));
            if ( MemError() != noErr)
            {
//              ShowErrorAny( eContinueOnlyErr, -1, "\pEnding the game because", "\p I didn't have enough memory to record any more keystrokes.", nil, nil, -1, -1, -1, -1, __FILE__, 38);
                SysBeep(20);
                gAresGlobal->gGameOver = 1;
            }
            HLock( gAresGlobal->gReplayData);
            result = kWinGame;
        }

    }
    if (( gAresGlobal->gOptions & ( kOptionRecord | kOptionReplay)) && ( gAresGlobal->gReplayData != nil))
    {
            HUnlock( gAresGlobal->gReplayData);
    }
    WriteDebugLine((char *)"\p<GameOver");
    WriteDebugLong( keyDataSize);
    MacShowCursor();

    Microseconds( &thisTime);
    WideSubtract( (wide *)&thisTime, (wide *)&gAresGlobal->gLastTime);
    newGameTime = thisTime.lo / 1000000; // divide by a million to get seconds
//  *seconds = newGameTime + additionalSeconds;
    *seconds = newGameTime + additionalSeconds;
//  HHCheckAllHandles();
    RestoreOriginalColors();
    if ( result < 0)
    {
        if ( (gAresGlobal->gScenarioWinner & kScenarioWinnerPlayerMask) == gAresGlobal->gPlayerAdmiralNumber) return ( 1);
        else return ( 0);
    } else return ( result);
}

Boolean HandleMouseDown( EventRecord *theEvent)

{
    long        menuChoice;
    short       whichPart;
    WindowPtr   whichWindow;
    Point       where;
    Boolean     close = FALSE, done = FALSE;

    whichPart = MacFindWindow (theEvent->where,  &whichWindow);
    switch (whichPart)
    {
        case inMenuBar:
            menuChoice = MenuSelect( theEvent->where);
            done = HandleMenuChoice( menuChoice);
            break;
        case inSysWindow:
            SystemClick( theEvent, whichWindow);
            break;
        case inContent:
            where = theEvent->where;
            GlobalToLocal( &where);
            break;
        case inDrag:
            DragWindow (whichWindow, theEvent->where, &qd.screenBits.bounds);
            break;
        case inGoAway:
            if ( TrackGoAway (whichWindow, theEvent->where))
            {
                ShowHide ( whichWindow, FALSE);
                if ( whichWindow == (WindowPtr)gTheWindow)
                    done = TRUE;
            }
            break;
    }
    return ( done);
}

Boolean HandleMenuChoice( long menuChoice)

{
    int     theMenu;
    int     theItem;
    Boolean done = FALSE;

    if (menuChoice != 0)
    {
        theMenu = HiWord( menuChoice);
        theItem = LoWord( menuChoice);
        switch (theMenu)
        {
            case APPLE_MENU_ID:
                HandleAppleChoice( theItem);
                break;
            case FILE_MENU_ID:
                done = HandleFileChoice( theItem);
                break;
            case EDIT_MENU_ID:
                HandleEditChoice( theItem);
                break;
        }
        HiliteMenu( 0 );
    }
    return ( done);
}

void HandleAppleChoice ( int theItem)

{
    Str255      accName;
    int         accNumber;

    switch ( theItem)
    {
        case ABOUT_ITEM:
            NoteAlert( ABOUT_ALERT, nil);
            break;
        default:
            GetMenuItemText ( gAppleMenu, theItem, accName);
#if TARGET_OS_MAC
            accNumber = OpenDeskAcc( accName);
#endif TARGET_OS_MAC
            break;
    }
}

Boolean HandleFileChoice( int theItem)

{
    switch( theItem)
    {
        case QUIT_ITEM:
            return ( TRUE);
            break;
    }
    return ( FALSE);
}

void HandleEditChoice( int theItem)

{
#pragma unused( theItem)
}

Boolean HandleKeyDown( EventRecord *theEvent)

{
    char    whichChar;

    whichChar = theEvent->message & charCodeMask;
    if (( theEvent->modifiers & cmdKey ) != 0)
        return( HandleMenuChoice( MenuKey( whichChar )));
    else return( false);
}

void Pause( long time)

{
    long    starttime = TickCount();

    while (( TickCount() - starttime) < time) { /* DO NOTHING */};
}

void SetWindowColorTable( WindowPtr window)
{

    WCTabHandle     winCTabHandle;                                                  // 2
    AuxWinHandle    auxWinHandle;                                                   // 2

    GetAuxWin (window,&auxWinHandle);                                               // 2
    winCTabHandle = (WCTabHandle) ((**auxWinHandle).awCTable);                      // 2

    HandToHand ((Handle *) &winCTabHandle);                                         // 3
    if (!MemError ( ))                                                              // 3
    {
        RGBColor blackness = { 0, 0, 0 };                                           // 4

        if (SetColorTableEntry ((CTabHandle) winCTabHandle, 0, &blackness))         // 4
        {
            SetWinColor (window,winCTabHandle);                                     // 5
        }
    }
}

static pascal Boolean SetColorTableEntry (CTabHandle cth, short value, const RGBColor *rgbP)
{
    ColorSpecPtr    ctTable     = (**cth).ctTable;
    short           ctSize      = (**cth).ctSize;

    while (ctSize > -1)
    {
        if (ctTable->value == value)
        {
            ctTable->rgb = *rgbP;
            CTabChanged (cth);
            return true;
        }

        ++ctTable;
        --ctSize;
    }

    return false;
}

void DrawOutlinedString( StringPtr string, RGBColor *color)
{
    RGBColor    backColor = {0, 0, 0};
    Point       pen;

    GetPen( &pen);

    RGBForeColor( &backColor);
    MoveTo( pen.h - 1, pen.v - 1);
    DrawString( string);
    MoveTo( pen.h, pen.v - 1);
    DrawString( string);
    MoveTo( pen.h + 1, pen.v - 1);
    DrawString( string);
    MoveTo( pen.h - 1, pen.v);
    DrawString( string);
    MoveTo( pen.h + 1, pen.v);
    DrawString( string);
    MoveTo( pen.h - 1, pen.v + 1);
    DrawString( string);
    MoveTo( pen.h, pen.v + 1);
    DrawString( string);
    MoveTo( pen.h + 1, pen.v + 1);
    DrawString( string);

    MoveTo( pen.h, pen.v);

    RGBForeColor( color);
    DrawString( string);
}
