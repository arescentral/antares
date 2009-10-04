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

#include "AresMain.hpp"

#include <Timer.h>
#include <Quickdraw.h>
//#include <stdio.h>        // for _DATE_ & _TIME_ macros

#define kProfiling_On   0
#if kProfiling_On
#include <profiler.h>
#endif

#include "Admiral.hpp"
#include "AmbrosiaSerial.h"
#include "AresAppleEvent.hpp"
#include "AresCheat.hpp"
#include "AresDemoScanner.hpp"
#include "AresExternalFile.hpp"
#include "AresGameRanger.hpp"
#include "AresGlobalType.hpp"
#include "AresGuideMaker.hpp"
#include "AresMoviePlayer.hpp"
#include "AresNetworkSprocket.hpp"
#include "AresPreferences.hpp"
#include "AresResFile.hpp"

#include "Beam.hpp"

#include "ColorTable.hpp"
#include "ColorTranslation.hpp"

#include "Debug.hpp"
#include "DirectText.hpp"

#include "EnvironmentCheck.hpp"
#include "Error.hpp"

#include "FakeDrawing.hpp"
#include "Fakes.hpp"

#include "GXMath.h"

#include "InputSource.hpp"
#include "Instruments.hpp"
#include "InterfaceHandling.hpp"

#include "KeyCodes.hpp"
#include "KeyMapTranslation.hpp"

#include "MathSpecial.hpp"
#include "MessageScreen.hpp"
#include "Minicomputer.hpp"
#include "Motion.hpp"
#include "Music.hpp"

#include "NateDraw.hpp"
#include "NetSetupScreen.hpp"
#include "NonPlayerShip.hpp"

#include "OffscreenGWorld.hpp"
#include "Options.hpp"

#include "PlayerInterface.hpp"
#include "PlayerShip.hpp"

#include "Randomize.hpp"
#include "RegistrationTool.h"
#include "Resources.h"
#include "Rotation.hpp"

#include "ScenarioMaker.hpp"
#include "ScreenLabel.hpp"
#include "ScrollStars.hpp"
#include "ShotsBeamsExplosions.hpp"
#include "SoundFX.hpp"
#include "SpaceObjectHandling.hpp"
#include "SpriteCursor.hpp"
#include "SpriteHandling.hpp"
#include "StringHandling.hpp"

#include "TimeLimit.hpp"
#include "TimeUnit.hpp"
#include "TitleScreen.hpp"
#include "ToolUtils.h"
#include "Transitions.hpp"
#include "Traps.h"

#include "VersionString.hpp"
#include "VideoDriver.hpp"

#include "WrapGameRanger.hpp"

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

enum GameResult {
    NO_GAME = -1,
    LOSE_GAME = 0,
    WIN_GAME = 1,
    RESTART_GAME = 2,
    QUIT_GAME = 3,
};

//#define   kCanRecordGame

#define kReplayNotAutoPlay  // if this is set, will play back recorded games

#define mAbortNetworkGame       { if ( globals()->gGameOver == 0) globals()->gGameOver = 1; result = kQuitGame; StopNetworking();}

#define kTitleTextScrollWidth   450

#define kFractionalLagCorrectTolerance  8

#define kInterfaceResFileName   "\p:Ares Data Folder:Ares Interfaces"
#define kSpriteResFileName      "\p:Ares Data Folder:Ares Sprites"
#define kSoundResFileName       "\p:Ares Data Folder:Ares Sounds"

//#define   kTestNumber     50//5

//#define   kUseSmallPlayWindow // for debugging

extern TypedHandle<spaceObjectType> gSpaceObjectData;
extern int32_t gRandomSeed;
extern long gNatePortLeft, gNatePortTop, gNetLatency;
extern scenarioType *gThisScenario;
extern short gSpriteFileRefID, gInterfaceFileRefID;
extern PixMap* gActiveWorld;
extern scoped_ptr<Window> fakeWindow;

CWindowPtr      gTheWindow = nil;//, globals()->gBackWindow = nil;
MenuHandle      gAppleMenu;
long            gLastTick, // globals()->gGameOver = 0,
                WORLD_WIDTH = 640,
                WORLD_HEIGHT = 480,
                CLIP_LEFT = 128,
                CLIP_TOP = 0,
                CLIP_RIGHT = 608,
                CLIP_BOTTOM = 480,
                gPlayScreenWidth = 480,
                gPlayScreenHeight = 480;
Boolean         hackWarn = true;

unsigned long kNoKeys = 0;

int GetDemoScenario();
void FakeInit(int argc, char* const* argv);
void Pause( long time);
void MainLoop();
GameResult PlayTheGame(long *seconds);

int main(int argc, char* const* argv) {
    FakeInit(argc, argv);
    return 0;
}

void AresMain() {
    RGBColor                initialFadeColor;
    Boolean                 skipFading = false;
    scoped_ptr<ColorTable>  theClut;

    init_globals();
    WORLD_WIDTH = fakeWindow->portRect.right;
    WORLD_HEIGHT = fakeWindow->portRect.bottom;
    CLIP_RIGHT = WORLD_WIDTH - kRightPanelWidth;
    CLIP_BOTTOM = WORLD_HEIGHT;
    gPlayScreenWidth = CLIP_RIGHT - CLIP_LEFT;
    gPlayScreenHeight = CLIP_BOTTOM - CLIP_TOP;

    globals()->gPreferencesData.reset(new Preferences);

    // Disable networking.
    globals()->gOptions &= ~kOptionNetworkAvailable;

    GetDateTime( reinterpret_cast<unsigned long *>(&gRandomSeed));

    theClut.reset(new ColorTable(256));
    gTheWindow = fakeWindow.get();
    gActiveWorld = &fakeWindow->portBits;
    ColorTranslatorInit(*theClut);

    InitSpriteCursor();
    InitTransitions();

    initialFadeColor.red = initialFadeColor.green = initialFadeColor.blue = 0;
    skipFading = AutoFadeTo(30, &initialFadeColor, true);

    ClearScreen();
    ResetTransitions();

    CreateOffscreenWorld(gTheWindow->portRect, *theClut);
    MusicInit();

    InitMoviePlayer();
    RotationInit();
    NormalizeColors();
    DrawInRealWorld();

    InterfaceHandlingInit();

    if (globals()->gOptions & kOptionMusicIdle) {
        LoadSong( kTitleSongID);
        SetSongVolume( kMaxMusicVolume);
        PlaySong();
    }
    if (!skipFading) {
        skipFading = CustomPictFade(20, 20, 2000, 2000, gTheWindow);
    }
    if (!skipFading) {
        skipFading = CustomPictFade(20, 20, 2001, 2000, gTheWindow);
    }

    StartCustomPictFade(20, 20, 502, 2001, gTheWindow, skipFading);
    InitDirectText();
    ScreenLabelInit();
    InitMessageScreen();
    InitScrollStars();
    InstrumentInit();
    SpriteHandlingInit();
    AresCheatInit();
    ScenarioMakerInit();
    SpaceObjectHandlingInit();  // MUST be after ScenarioMakerInit()
    InitSoundFX();
    InitMotion();
    AdmiralInit();
    InitBeams();
    TimedWaitForAnyEvent(skipFading ? 1 : 1400);
    EndCustomPictFade(gTheWindow, skipFading);

    gLastTick = TickCount();

    globals()->okToOpenFile = true;
    MainLoop();
}

void MainLoop() {
    long                    saveSeed = 0, gameLength;
    bool                    done = false;
    short                   resID = 0;

    if (!(globals()->gOptions & kOptionHaveSeenIntro)) {
        DoScrollText( gTheWindow, 5600, 4, kTitleTextScrollWidth,
            kTitleFontNum, -1);

        globals()->gOptions |= kOptionHaveSeenIntro;
        SaveOptionsPreferences();
    }

    while (!done) {
        long whichDemoLevel;
        switch (DoMainScreenInterface(&whichDemoLevel)) {
          case kMainQuit:
            {
                RGBColor fadeColor = { 0, 0, 0 };
                if (globals()->gOptions & kOptionMusicIdle) {
                    AutoMusicFadeTo( 60, &fadeColor, FALSE);
                } else {
                    AutoFadeTo( 60, &fadeColor, FALSE);
                }
                done = true;
            }
            break;

          case kMainTimeoutDemo:
            if (Randomize(4) == 2) {
                DoScrollText( gTheWindow, 5600, 4, kTitleTextScrollWidth,
                        kTitleFontNum, -1);
            }
            // fall through

          case kMainDemo:
            globals()->gOptions |= kOptionReplay;
            // fall through

          case kMainPlay:
            {
                int whichScenario = 0;

                if (globals()->gOptions & kOptionReplay) {
                    if (whichDemoLevel >= 0) {
                        globals()->gInputSource.reset(
                                new ReplayInputSource(whichDemoLevel + 1));
                        whichScenario = resID - kReplayResID;
                    }

                    while (globals()->gInputSource.get() == nil) {
                        whichScenario = GetDemoScenario();
                        globals()->gInputSource.reset(
                                new ReplayInputSource(kReplayResID + whichScenario));
                    }

                    saveSeed = gRandomSeed;
                    gRandomSeed = globals()->gInputSource->random_seed();
                } else {
                    whichScenario = DoSelectLevelInterface(GetStartingLevelPreference());
                    if (whichScenario < 0) {
                        break;
                    }
                }

                StopAndUnloadSong();

                GameResult gameResult = NO_GAME;
                do {
                    if ((gameResult == NO_GAME) || (gameResult == WIN_GAME)) {
                        if (GetScenarioPrologueID(whichScenario) > 0) {
                            DoScrollText( gTheWindow,
                                    GetScenarioPrologueID( whichScenario),
                                    4, kTitleTextScrollWidth, kTitleFontNum, 4002);
                        }
                    }

                    gameResult = NO_GAME;
                    RGBColor fadeColor = { 0, 0, 0 };
                    if (globals()->gOptions & kOptionMusicIdle) {
                        AutoMusicFadeTo(60, &fadeColor, FALSE);
                        StopAndUnloadSong();
                    } else {
                        AutoFadeTo( 60, &fadeColor, FALSE);
                    }

                    RemoveAllSpaceObjects();
                    globals()->gGameOver = 0;

                    if (globals()->gOptions & kOptionMusicIdle) {
                        LoadSong(3000);
                        SetSongVolume( kMaxMusicVolume);
                        PlaySong();
                    }

                    gameResult = NO_GAME;
                    if (!ConstructScenario(whichScenario)) {
                        break;
                    }

                    if (globals()->gOptions & kOptionMusicIdle) {
                        StopAndUnloadSong();
                    }

                    DrawInstrumentPanel(gTheWindow);

                    if (globals()->gOptions & kOptionMusicPlay) {
                        LoadSong(gThisScenario->songID);
                        SetSongVolume(kMusicVolume);
                        PlaySong();
                    }
                    ResetLastTime((gThisScenario->startTime & kScenario_StartTimeMask) * kScenarioTimeMultiple);

                    gameResult = PlayTheGame(&gameLength);

                    switch (gameResult) {
                      case LOSE_GAME:
                        if (!(globals()->gOptions & kOptionReplay)) {
                            if ((globals()->gScenarioWinner & kScenarioWinnerTextMask) !=
                                    kScenarioWinnerNoText) {
                                DoMissionDebriefingText(
                                        (globals()->gScenarioWinner & kScenarioWinnerTextMask) >> kScenarioWinnerTextShift,
                                        -1, -1, -1, -1, -1, -1, -1);
                            }
                            if (DoPlayAgain(false, false)) {
                                gameResult = RESTART_GAME;
                            } else {
                                gameResult = QUIT_GAME;
                                ClearScreen();
                                AutoFadeFrom(1, false);
                            }
                        }
                        break;

                      case WIN_GAME:
                        {
                            if ((globals()->gScenarioWinner & kScenarioWinnerTextMask) !=
                                    kScenarioWinnerNoText) {
                                DoMissionDebriefingText(
                                        (globals()->gScenarioWinner & kScenarioWinnerTextMask) >> kScenarioWinnerTextShift,
                                        gameLength, gThisScenario->parTime,
                                        GetAdmiralLoss(0), gThisScenario->parLosses,
                                        GetAdmiralKill(0), gThisScenario->parKills,
                                        100);
                            }

                            fadeColor.red = fadeColor.green = fadeColor.blue = 0;
                            if (globals()->gOptions & kOptionMusicPlay) {
                                AutoMusicFadeTo( 60, &fadeColor, FALSE);
                                StopAndUnloadSong();
                            } else {
                                AutoFadeTo( 60, &fadeColor, FALSE);
                            }

                            ClearScreen();
                            AutoFadeFrom( 1, FALSE);

                            // normal scrolltext song
                            int scroll_song = 4002;
                            if ((globals()->gScenarioWinner & kScenarioWinnerNextMask) ==
                                    kScenarioWinnerNoNext) {
                                // we win but no next level? Play triumph song
                                scroll_song = 4003;
                            }

                            if (GetScenarioEpilogueID( whichScenario) > 0) {
                                DoScrollText(gTheWindow,
                                        GetScenarioEpilogueID( whichScenario),
                                        4, kTitleTextScrollWidth, kTitleFontNum, scroll_song);
                            }

                            if (globals()->gOptions & kOptionMusicIdle) {
                                ClearScreen();
                                StopAndUnloadSong();
                            }

                            if (globals()->gOptions & kOptionReplay) {
                                gRandomSeed = saveSeed;
                            }
                            globals()->gInputSource.reset();

                            if ((globals()->gScenarioWinner & kScenarioWinnerNextMask) ==
                                    kScenarioWinnerNoNext) {
                                whichScenario = -1;
                            } else {
                                whichScenario = GetScenarioNumberFromChapterNumber(
                                        (globals()->gScenarioWinner & kScenarioWinnerNextMask)
                                        >> kScenarioWinnerNextShift);
                            }

                            if ((!(globals()->gOptions & kOptionReplay))
                                    && (whichScenario <= GetScenarioNumber())
                                    && (whichScenario >= 0)) {
                                int chapter = GetChapterNumberFromScenarioNumber(whichScenario);
                                if ((chapter >= 0) && (chapter <= kHackLevelMax)
                                        && (chapter <= GetScenarioNumber())) {
                                    SaveStartingLevelPreferences(chapter);
                                } else {
                                    whichScenario = -1;
                                }
                            }
                        }
                        break;

                      default:
                        break;
                    }
                    if (globals()->gOptions & kOptionMusicPlay) {
                        StopAndUnloadSong();
                    }
                } while ((gameResult != QUIT_GAME)
                        && (GetChapterNumberFromScenarioNumber(whichScenario) <= kHackLevelMax)
                        && (GetChapterNumberFromScenarioNumber(whichScenario) <= GetScenarioNumber())
                        && (whichScenario >= 0)
                        && (!(globals()->gOptions & kOptionReplay)));

                if ( globals()->gOptions & kOptionMusicIdle) {
                    LoadSong(kTitleSongID);
                    SetSongVolume(kMaxMusicVolume);
                    PlaySong();
                }
                globals()->gOptions &= ~kOptionReplay;
            }
            break;

          case kMainNetwork:
            {
#if NETSPROCKET_AVAILABLE
                if ( globals()->gOptions & kOptionNetworkOn)
                    StopNetworking();
                globals()->gOptions &= ~kOptionNetworkOn;
                netResult = StartNetworkGameSetup();
                if ( globals()->gOptions & kOptionMusicIdle)
                {
                    StopAndUnloadSong();
                }
                if ( netResult != kCancel)
                {
                    globals()->gOptions |= kOptionNetworkOn;
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
                            if ( globals()->gOptions & kOptionMusicIdle)
                            {
                                AutoMusicFadeTo( 60, &fadeColor, FALSE);
                                StopAndUnloadSong();
                            } else
                            {
                                AutoFadeTo( 60, &fadeColor, FALSE);
                            }

                            RemoveAllSpaceObjects();
                            globals()->gGameOver = 0;

                            // mission briefing unfades screen

                            if ( globals()->gOptions & kOptionMusicIdle)
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

                                if ( globals()->gOptions & kOptionMusicIdle)
                                {
                                    StopAndUnloadSong();
                                }

                                HideCursor();
                                DrawInstrumentPanel(  gTheWindow);
                                MacShowCursor();
                                if ( globals()->gOptions & kOptionMusicPlay)
                                {
                                    LoadSong( gThisScenario->songID);
                                    SetSongVolume( kMusicVolume);
                                    PlaySong();
                                }
                                ResetLastTime( (gThisScenario->startTime & kScenario_StartTimeMask) * kScenarioTimeMultiple);

                                if ( globals()->gameRangerInProgress)
                                {
                                    Wrap_GRGameBegin();
                                }

                                gameResult = PlayTheGame( &gameLength);

                                if ( globals()->gameRangerInProgress)
                                {
                                    if ( (globals()->gScenarioWinner & kScenarioWinnerPlayerMask) == globals()->gPlayerAdmiralNumber)
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
                                SetNetKills( GetNetKills() + GetAdmiralKill( globals()->gPlayerAdmiralNumber));
                                SetNetLosses( GetNetLosses() + GetAdmiralLoss( globals()->gPlayerAdmiralNumber));
                                if ( (globals()->gScenarioWinner &
                                            kScenarioWinnerTextMask) != kScenarioWinnerNoText)
                                {
                                    //                                  DoMissionDebriefingText( (WindowPtr) gTheWindow,
                                    //                                      (globals()->gScenarioWinner & kScenarioWinnerTextMask) >> kScenarioWinnerTextShift,
                                    //                                      -1, -1, -1, -1, -1, -1, -1);
                                    DoMissionDebriefingText(  gTheWindow,
                                            (globals()->gScenarioWinner & kScenarioWinnerTextMask)
                                            >> kScenarioWinnerTextShift,
                                            gameLength, gThisScenario->parTime,
                                            GetAdmiralLoss( globals()->gPlayerAdmiralNumber),
                                            gThisScenario->parLosses,
                                            GetAdmiralKill( globals()->gPlayerAdmiralNumber),
                                            gThisScenario->parKills, 100);

                                }
                                if ( globals()->gOptions & kOptionMusicPlay)
                                {
                                    StopAndUnloadSong();
                                }

                            }
                        }
                    } while (( whichScenario >= 0) && ( NetGameIsOn()));
                }
                if (( globals()->gOptions & kOptionMusicIdle) && ( !SongIsPlaying()))
                {
                    LoadSong( kTitleSongID);
                    SetSongVolume( kMaxMusicVolume);
                    PlaySong();
                }

                globals()->gOptions &= ~kOptionNetworkOn;
                if ( OptionKey()) DebugFileSave( "\p_Poopy");
                DebugFileCleanup();
#endif NETSPROCKET_AVAILABLE
            }
            break;

          case kMainTrain:
            DoScrollText( gTheWindow, 5600, 4, kTitleTextScrollWidth,
                    kTitleFontNum, -1);

          case kMainAbout:
          case kMainOptions:
          case kNullResult:
            // These options are already handled by DoMainScreenInterface().
            break;
        }
    }
}

GameResult PlayTheGame(long *seconds) {
    unsigned long       decideCycle = 0;
    Str255              string;
    uint64_t            lastTime, thisTime, scrapTime = { 0 }, netTime;
    Rect                clipRect;
    long                    unitsToDo = 0, unitsPassed = 0, unitsDone = 0,
                            l1, l2, newGameTime = 0, lastclicktime = 0,
                            additionalSeconds = 0;
    KeyMap              keyMap = { }, lastKeyMap;
    Boolean             playerPaused = FALSE, mouseDown = FALSE,
                            enteringMessage = false,
                            afEntering = false, demoKey = false, newKeyMap = false, commandAndQ = false;
    unsigned long       keyDataSize = 0, scenarioCheckTime = 0;
    Rect                    playAreaRect;
    GameResult          result = NO_GAME;
    EventRecord         theEvent;
//  long                hacktc = TickCount(), hacktcsamplesize = 4, hacktcsamplecount = 0;

    VideoDriver::driver()->set_game_state(PLAY_GAME);

    DebugFileCleanup();
    DebugFileInit();
    DebugFileAppendString("\p<START DEBUG FILE>\r\r");

    commandAndQ = BothCommandAndQ();

    if ( globals()->gOptions & kOptionNetworkOn)
    {
#if NETSPROCKET_AVAILABLE
        ResetGotMessages( 0x7fffffff);
        if ( !JumpstartLatencyQueue(globals()->gGameTime, kDecideEveryCycles))
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

    globals()->gLastKeys = globals()->gTheseKeys = 0;

    HideCursor();
    Microseconds( &lastTime);

    unitsPassed = 0;

    WriteDebugLine("\pEntr Game");
    WriteDebugLong( gRandomSeed);
    DebugFileAppendString("\p---NEW GAME---");

    DebugFileAppendString("\pTime\tFile\tLine\t#\r");

    if ((globals()->gOptions & kOptionReplay)
            && (globals()->gInputSource.get() != nil)) {
        // pass
    } else
    {
        if ( globals()->user_is_scum)
        {
            ShowErrorAny( eQuitErr, -1, "\pCan't continue because an error of type "
                "-2021 occurred; contact Ambrosia Software for a valid "
                "registration code.", "\p", nil, nil, -1, -1, -1, -1, __FILE__, 1);
            ExitToShell();
        }
    }
    netTime = 0;
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

    while (( globals()->gGameOver <= 0 ) && ( !globals()->returnToMain))
    {

        globals()->gFrameCount = 0;
        gLastTick = TickCount();

        SetLongRect( &clipRect, CLIP_LEFT, CLIP_TOP, CLIP_RIGHT, CLIP_BOTTOM);
        while (globals()->gGameOver <= 0)
        {
            EraseSpriteCursorSprite();
            EraseSpriteTable();
            EraseAllLabels();
            EraseSectorLines();
            PrepareToMoveScrollStars();
            EraseSite();

            while ( unitsPassed == 0)
            {
//              MyWideAdd( (wide *)&globals()->gLastTime, (wide *)&netTime);
                netTime = 0;
                Microseconds( &thisTime);
                scrapTime = thisTime;
                thisTime -= globals()->gLastTime;
                newGameTime = (thisTime / kTimeUnit) + ((gThisScenario->startTime & kScenario_StartTimeMask) * kScenarioTimeMultiple);

//#ifndef powerc
#ifdef powercxx
                if ( globals()->gOptions & kOptionNetworkOn)
                {
#endif
                    if ( (newGameTime - globals()->gGameTime) > k68KMaxFrameSkip)
                    {
                        newGameTime = globals()->gGameTime + k68KMaxFrameSkip;
                        l1 = newGameTime - ((gThisScenario->startTime & kScenario_StartTimeMask) * kScenarioTimeMultiple);
                        l2 = kTimeUnit;
                        MyWideMul(l1, l2, reinterpret_cast<int64_t*>(&thisTime));
                        globals()->gLastTime = scrapTime;
                        globals()->gLastTime -= thisTime;
                    }
#ifdef powercxx
                }
#endif

//#endif
//              if ( (!(globals()->gOptions & kOptionReplay)) &&
//                  (mSlowMotionKey( keyMap)))
/*              if ( (mSlowMotionKey( keyMap)))
                {
                    demoKey = true;
                    if ( unitsPassed < 3) Pause( 3);
                    newGameTime = globals()->gGameTime + 1;
                    l1 = newGameTime - ((gThisScenario->startTime & kScenario_StartTimeMask) * kScenarioTimeMultiple);
                    l2 = kTimeUnit;
                    MyWideMul( l1, l2, (wide *)&thisTime);
                    globals()->gLastTime = scrapTime;
                    WideSubtract( (wide *)&globals()->gLastTime, (wide *)&thisTime);
                } else */if (((( globals()->gOptions & kOptionSubstituteFKeys) ?
                    ( mNOFFastMotionKey( keyMap)):( mFastMotionKey( keyMap)))) &&
                    (!enteringMessage))
                {
                    demoKey = true;
                    newGameTime = globals()->gGameTime + 12;
                    l1 = newGameTime - ((gThisScenario->startTime & kScenario_StartTimeMask) * kScenarioTimeMultiple);
                    l2 = kTimeUnit;
                    MyWideMul(l1, l2, reinterpret_cast<int64_t*>(&thisTime));
                    globals()->gLastTime = scrapTime;
                    globals()->gLastTime -= thisTime;
                }/* else
                {
                    newGameTime = globals()->gGameTime + Randomize( 9) + 1;
                    l1 = newGameTime - ((gThisScenario->startTime & kScenario_StartTimeMask) * kScenarioTimeMultiple);
                    l2 = kTimeUnit;
                    MyWideMul( l1, l2, (wide *)&thisTime);
                    globals()->gLastTime = scrapTime;
                    WideSubtract( (wide *)&globals()->gLastTime, (wide *)&thisTime);
                }*/
//              if (( newGameTime - globals()->gGameTime) > 6) newGameTime = globals()->gGameTime + 6;

                if ( newGameTime >= kMaxGameTime)
                {
                    l1 = kTimeUnit;
                    l2 = newGameTime - kMaxGameTime;
                    MyWideMul(l1, l2, reinterpret_cast<int64_t*>(&thisTime));
                    globals()->gLastTime = scrapTime;
                    globals()->gLastTime -= thisTime;
                    additionalSeconds += ( newGameTime / 60);
                    newGameTime -= kMaxGameTime;
//                  globals()->gGameTime -= kMaxGameTime;
                }
                if ( globals()->gGameTime <= newGameTime)
                    unitsDone = unitsPassed = newGameTime - globals()->gGameTime;
                else
                    unitsDone = unitsPassed = newGameTime - ( globals()->gGameTime - kMaxGameTime);
/*              if ( globals()->gOptions & ( kOptionReplay | kOptionRecord))
                {
                    if ( unitsDone > kDecideEveryCycles)
                    {
                        globals()->gGameTime = newGameTime - kDecideEveryCycles;
                        unitsDone = unitsPassed = kDecideEveryCycles;
                    }
                }
*/
//              WideSubtract( (wide *)&thisTime, (wide *)&lastTime);
//              unitsDone = unitsPassed = thisTime.lo / kTimeUnit;
//              thisTick = TickCount();
//              unitsDone = unitsPassed = thisTick - lastTick;
//              newGameTime = TickCount() - globals()->gGameStartTime;
//              unitsDone = unitsPassed = newGameTime - globals()->gGameTime;
            }
//          Microseconds( &lastTime);   // don't activate
//          lastTick = thisTick;

            if ( playerPaused)
            {
                playerPaused = false;
                unitsDone = unitsPassed = 0;
                newGameTime = globals()->gGameTime;
                l1 = newGameTime - ((gThisScenario->startTime & kScenario_StartTimeMask) * kScenarioTimeMultiple);
                l2 = kTimeUnit;
                MyWideMul(l1, l2, reinterpret_cast<int64_t*>(&thisTime));
                globals()->gLastTime = scrapTime;
                globals()->gLastTime -= thisTime;
            }

            if ( globals()->gGameOver < 0)
            {
                globals()->gGameOver += unitsPassed;
                if ( globals()->gGameOver == 0)
                    globals()->gGameOver = 1;
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
                    MoveSpaceObjects(*gSpaceObjectData, kMaxSpaceObject, unitsToDo);

//                  WriteDebugLine("\pMove");
//                  WriteDebugLong( decideCycle);
//                  WriteDebugLong( unitsToDo);
                }

                globals()->gGameTime += unitsToDo;
                if ( globals()->gGameTime >= kMaxGameTime) globals()->gGameTime -= kMaxGameTime;
                if ( decideCycle == kDecideEveryCycles) // everything in here gets executed once every kDecideEveryCycles
                {
//                  MoveScrollStars( kDecideEveryCycles);
//                  MoveSpaceObjects( (spaceObjectType *)*gSpaceObjectData, kMaxSpaceObject,
//                                  kDecideEveryCycles);

                    playerPaused = FALSE;


                    NonplayerShipThink( kDecideEveryCycles);
                    AdmiralThink();
                    ExecuteActionQueue( kDecideEveryCycles);

                    if ( globals()->gOptions & kOptionReplay)
                    {
                        uint32_t keys;
                        if (!globals()->gInputSource->next(&keys)) {
                            globals()->gGameOver = 1;
                        }

                        if ( !playerPaused) {
                            playerPaused = PlayerShipGetKeys(
                                    kDecideEveryCycles, keys, &enteringMessage);
                        } else {
                            PlayerShipGetKeys( kDecideEveryCycles, keys, &enteringMessage);
                        }
                    } else
                    {
                        if ( !playerPaused) playerPaused =
                            PlayerShipGetKeys( kDecideEveryCycles,
                                0xffffffff, &enteringMessage);
                        else PlayerShipGetKeys( kDecideEveryCycles, 0xffffffff,
                            &enteringMessage);
                    }

                    if ( Button())
                    {
                        if ( globals()->gOptions & kOptionReplay)
                        {
                            result = QUIT_GAME;
                            globals()->gGameOver = 1;
                        } else
                        {
                            if ( !mouseDown)
                            {
                                if ( !(globals()->gOptions & ( kOptionAutoPlay | kOptionReplay)))
                                {
                                    if ((( globals()->gGameTime - lastclicktime)) <= GetDblTime())
                                    {
                                        InstrumentsHandleDoubleClick();
                                        lastclicktime -= GetDblTime();
                                    } else
                                    {
                                        InstrumentsHandleClick();
                                        lastclicktime = globals()->gGameTime;
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

                    if ( globals()->gOptions & kOptionNetworkOn)
                    {
//#ifdef kDemoTimeLimit
if ( (!Ambrosia_Is_Registered()) || ( GetOpponentIsUnregistered()))
{
                        if ( globals()->gGameTime > kTimeLimitWarning)
                        {
                            if ( globals()->gGameTime > kTimeLimit)
                            {
                                result = QUIT_GAME;
                                globals()->gGameOver = 1;
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
                        if ( SendInGameMessage( globals()->gGameTime + gNetLatency))
                        {
                            do
                            {
                                GetKeys( keyMap);
                                HandleTextMessageKeys( keyMap, lastMessageKeyMap, &afEntering);
                                for ( scratch = 0; scratch < 4; scratch++)
                                    lastMessageKeyMap[scratch] = keyMap[scratch];
                                if ( !ProcessInGameMessages( globals()->gGameTime, &pauseLevel))
                                {
//                                  ShowErrorAny( eContinueOnlyErr, -1, "\pEnding the game because ", "\pProcessInGamesMessages failed.", nil, nil, -1, -1, -1, -1, __FILE__, 2);
//                                  mAbortNetworkGame
                                    if ( globals()->gGameOver == 0)
                                    {
                                        globals()->gGameOver = 1;
                                        result = RESTART_GAME;
                                    }

                                    mWriteDebugString("\pPROC ERR");
                                    if ( (globals()->gScenarioWinner &
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
                                    if ( !( globals()->gOptions & (kOptionAutoPlay | kOptionReplay)))
                                    {
                                        if ( globals()->gOptions & kOptionNetworkOn)
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
                                                result = QUIT_GAME;
                                                globals()->gGameOver = 1;
                                                if ( CommandKey())
                                                    globals()->gScenarioWinner = globals()->gPlayerAdmiralNumber;
                                                globals()->gScenarioWinner |= kScenarioWinnerNoNext | kScenarioWinnerNoText;
                                                break;

                                            case 1: // restart
//                                              ShowErrorAny( eContinueOnlyErr, -1, "\pEnding the game because", "\p the user chose to restart.", nil, nil, -1, -1, -1, -1, __FILE__, 34);
                                                result = RESTART_GAME;
                                                globals()->gGameOver = 1;
                                                if ( CommandKey())
                                                    globals()->gScenarioWinner = globals()->gPlayerAdmiralNumber;
                                                globals()->gScenarioWinner |= kScenarioWinnerNoNext | kScenarioWinnerNoText;
                                                break;

                                            case 2: // resume
                                                break;

                                            case 3: // skip
                                                result = WIN_GAME;
                                                globals()->gGameOver = 1;
                                                globals()->gScenarioWinner =  globals()->gPlayerAdmiralNumber |
                                                    (( GetChapterNumberFromScenarioNumber(
                                                        globals()->gThisScenarioNumber)
                                                            +1) << kScenarioWinnerNextShift) |
                                                    kScenarioWinnerNoText;
                                                break;
                                        }
                                        CopyOffWorldToRealWorld( gTheWindow, &playAreaRect);
                                        HideCursor();
                                        playerPaused = true;
                                        if ( globals()->gOptions & kOptionNetworkOn)
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
                                        result = QUIT_GAME;
                                        globals()->gGameOver = 1;
                                        if ( CommandKey())
                                            globals()->gScenarioWinner = globals()->gPlayerAdmiralNumber;
                                        globals()->gScenarioWinner |= kScenarioWinnerNoNext | kScenarioWinnerNoText;
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
                                    if ( globals()->gGameOver == 0)
                                    {
//                                      ShowErrorAny( eContinueOnlyErr, -1, "\pEnding the game because", "\p I am the host and there are fewer than 2 players.", nil, nil, -1, -1, -1, -1, __FILE__, 22);
                                        DeclareWinner( globals()->gPlayerAdmiralNumber, -1, 6004);
                                        globals()->gGameOver = 1;
                                    }
                                    result = QUIT_GAME;
                                }

                                if ( !NetGameIsOn())
                                {
                                    if ( globals()->gGameOver == 0)
                                    {
//                                      ShowErrorAny( eContinueOnlyErr, -1, "\pEnding the game because", "\p there is no network game.", nil, nil, -1, -1, -1, -1, __FILE__, 23);
                                        DeclareWinner( globals()->gPlayerAdmiralNumber, -1, 6004);
                                        globals()->gGameOver = 1;
                                    }
                                    result = QUIT_GAME;
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
                                        SendResendMessage( globals()->gGameTime);
                                        l1 = TickCount();

                                        if ( GetRegisteredSetting() > 0)
                                            resendTime = GetResendDelay() * 20;
                                        else
                                            resendTime = GetResendDelay() * 2;
                                    }
                                }
                            } while (((pauseLevel > 0) || ( !GotAllMessages())) &&
                                ( result == NO_GAME));
                        } else
                        {
//                          ShowErrorAny( eContinueOnlyErr, -1, "\pEnding the game because", "\p SendInGameMessage failed.", nil, nil, -1, -1, -1, -1, __FILE__, 5);
                            mAbortNetworkGame
                            mWriteDebugString("\pSEND MSG ERR");
                            DeclareWinner( -1, -1, 300);
                        }

                        Microseconds( &netTime);    // don't activate
                        netTime -= thisTime;
                        if ( netTime > kTimeUnit)
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
                        ResetGotMessages( globals()->gGameTime);
//                      UseNextLatency();
#endif NETSPROCKET_AVAILABLE
                    }

                    CollideSpaceObjects(*gSpaceObjectData, kMaxSpaceObject);
                    decideCycle  = 0;
                    scenarioCheckTime++;
                    if ( scenarioCheckTime == 30)
                    {
                        scenarioCheckTime = 0;
                        CheckScenarioConditions( 0);
                    }
//                  WriteDebugLine("\pDecide");
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
            if ( !(globals()->gOptions & kOptionNetworkOn))
            {
#if NETSPROCKET_AVAILABLE
                afEntering = enteringMessage;
                HandleTextMessageKeys( keyMap, lastMessageKeyMap, &afEntering);
                for ( scratch = 0; scratch < 4; scratch++)
                    lastMessageKeyMap[scratch] = keyMap[scratch];
#endif
            }
            if (
                //(!(globals()->gOptions & kOptionReplay)) &&
                (mPauseKey( keyMap)))
            {
                mWriteDebugString("\pThis Time:");
                WriteDebugLong( newGameTime);
                WriteDebugLong( globals()->gGameTime);
#if NETSPROCKET_AVAILABLE
                DebugMessageQueue();
#endif
                RestoreOriginalColors();
                GetIndString( string, 3100, 11);
                if ( globals()->gOptions & kOptionNetworkOn)
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
                if ( globals()->gOptions & kOptionNetworkOn)
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

            if ((!( globals()->gOptions & kOptionNetworkOn)) &&
                ( !(globals()->gOptions & kOptionReplay)) &&
                ((mRestartResumeKey( keyMap)) || ((!commandAndQ) && ( mQuitKeys( keyMap)))))
            {
                if ( !( globals()->gOptions & (kOptionAutoPlay | kOptionReplay)))
                {
                    if ( globals()->gOptions & kOptionNetworkOn)
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
                            result = QUIT_GAME;
                            globals()->gGameOver = 1;
                            if ( CommandKey())
                                globals()->gScenarioWinner = globals()->gPlayerAdmiralNumber;
                            globals()->gScenarioWinner |= kScenarioWinnerNoNext | kScenarioWinnerNoText;
                            break;

                        case 1: // restart
//                          ShowErrorAny( eContinueOnlyErr, -1, "\pEnding the game because", "\p the user chose to restart.", nil, nil, -1, -1, -1, -1, __FILE__, 36);
                            result = RESTART_GAME;
                            globals()->gGameOver = 1;
                            if ( CommandKey())
                                globals()->gScenarioWinner = globals()->gPlayerAdmiralNumber;
                            globals()->gScenarioWinner |= kScenarioWinnerNoNext | kScenarioWinnerNoText;
                            break;

                        case 2: // resume
                            break;

                        case 3: // skip
                            result = WIN_GAME;
                            globals()->gGameOver = 1;
                            globals()->gScenarioWinner =  globals()->gPlayerAdmiralNumber |
                                (( GetChapterNumberFromScenarioNumber(globals()->gThisScenarioNumber)+1)
                                    << kScenarioWinnerNextShift) |
                                kScenarioWinnerNoText;
                            break;
                    }
                    CopyOffWorldToRealWorld(&playAreaRect);
                    HideCursor();
                    playerPaused = true;
                    if ( globals()->gOptions & kOptionNetworkOn)
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
                    result = QUIT_GAME;
                    globals()->gGameOver = 1;
                    if ( CommandKey())
                        globals()->gScenarioWinner = globals()->gPlayerAdmiralNumber;
                    globals()->gScenarioWinner |= kScenarioWinnerNoNext | kScenarioWinnerNoText;
                }
            }

            if ((!(globals()->gOptions & kOptionReplay)) &&
                (((!afEntering)&&( mHelpKey( keyMap)))))

            {
                if ( globals()->gOptions & kOptionNetworkOn)
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
                CopyOffWorldToRealWorld(&playAreaRect);
                playerPaused = true;
                if ( globals()->gOptions & kOptionNetworkOn)
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
            if ((globals()->gOptions & kOptionNetworkOn) &&
#if NETSPROCKET_AVAILABLE
            ( IAmHosting()) &&
#endif NETSPROCKET_AVAILABLE
                    (((!afEntering)&&( mNetSettingsKey( keyMap)))))
            {
                if ( globals()->gOptions & kOptionNetworkOn)
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
                CopyOffWorldToRealWorld(&playAreaRect);
                playerPaused = true;
                if ( globals()->gOptions & kOptionNetworkOn)
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

            if (((!(globals()->gOptions & kOptionReplay)) &&
            ((!afEntering) && ( ( mVolumeDownKey( keyMap) && !mVolumeDownKey( lastKeyMap))))))
            {
                if ( globals()->gSoundVolume > 0) globals()->gSoundVolume--;
                if ( globals()->gOptions & kOptionMusicPlay)
                {
                    SetSongVolume( kMusicVolume);
                }
            }

            if (((!(globals()->gOptions & kOptionReplay)) &&
                ((!afEntering) && ( mVolumeUpKey( keyMap) && !mVolumeUpKey( lastKeyMap)))))
            {
                if ( globals()->gSoundVolume < kMaxVolumePreference) globals()->gSoundVolume++;
                if ( globals()->gOptions & kOptionMusicPlay)
                {
                    SetSongVolume( kMusicVolume);
                }
            }

            if ((!(globals()->gOptions & kOptionReplay)) && (!afEntering) &&
                ( mActionMusicKey( keyMap)) && ( !mActionMusicKey( lastKeyMap)))
            {
                if ( globals()->gOptions & kOptionMusicPlay)
                {
                    ToggleSong();
                }
            }

#if TARGET_OS_MAC
            keyMap[3] &= ~0x80; // mask out power key
            keyMap[1] &= ~0x02; // mask out caps lock key
            if ( (globals()->gOptions & kOptionReplay) && (!demoKey) &&
                (!newKeyMap) &&
                ((keyMap[0] != 0) || ( keyMap[1] != 0) || ( keyMap[2] != 0) ||
                (keyMap[3] != 0)))
#else
            keyMap[3].bigEndianValue &= EndianU32_NtoB(~0x80);  // mask out power key
            keyMap[1].bigEndianValue &= EndianU32_NtoB(~0x02);  // mask out caps lock key
            if ( (globals()->gOptions & kOptionReplay) && (!demoKey) &&
                (!newKeyMap) &&
                ((keyMap[0].bigEndianValue != 0) || ( keyMap[1].bigEndianValue != 0) ||
                ( keyMap[2].bigEndianValue != 0) ||
                (keyMap[3].bigEndianValue != 0)))
#endif TARGET_OS_MAC
            {
                result = QUIT_GAME;
                globals()->gGameOver = 1;
            }
            demoKey = false;

//          if ( globals()->gGameTime > newGameTime) { WriteDebugLong( newGameTime); globals()->gGameTime = newGameTime;}

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

            if ((globals()->gOptions & kOptionQDOnly)) {
                ShowSpriteCursorSprite();
                DrawAllBeams();
                DontShowScrollStars();
                CopyOffWorldToRealWorld(&playAreaRect);
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
                CopyOffWorldToRealWorld(&playAreaRect);

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
            globals()->gFrameCount++;
            VideoDriver::driver()->main_loop_iteration_complete(globals()->gGameTime);
        }
    }
    exit(0);  // Temporary: exit after finishing demo.

#if kProfiling_On
    ProfilerSetStatus( false);
    if (ProfilerDump("\pares_profile") != noErr)
        DebugStr("\pprofiler dump error");
    ProfilerTerm();
#endif

    if ( globals()->gOptions & kOptionNetworkOn)
    {
#if NETSPROCKET_AVAILABLE
        if ( IAmHosting())
        {
//          StopNetworking();
            if ( !SendInGameMiscLongMessage( 0, eStopPlayingMessage,
                globals()->gScenarioWinner, true, false))
            {
                mAbortNetworkGame
            }
        } else
        {
//          StopNetworking();
            if ( !SendInGameMiscLongMessage( 0, eStopPlayingMessage,
                globals()->gScenarioWinner, true, false))
            {
                mAbortNetworkGame
            }
        }
#endif NETSPROCKET_AVAILABLE
    }

    WriteDebugLine("\p<GameOver");
    WriteDebugLong( keyDataSize);
    MacShowCursor();

    Microseconds( &thisTime);
    thisTime -= globals()->gLastTime;
    newGameTime = thisTime / 1000000; // divide by a million to get seconds
//  *seconds = newGameTime + additionalSeconds;
    *seconds = newGameTime + additionalSeconds;
//  HHCheckAllHandles();
    RestoreOriginalColors();
    if (result == NO_GAME) {
        if ((globals()->gScenarioWinner & kScenarioWinnerPlayerMask) == globals()->gPlayerAdmiralNumber) {
            return WIN_GAME;
        } else {
            return LOSE_GAME;
        }
    } else {
        return result;
    }
}

Boolean HandleMouseDown( EventRecord *theEvent)

{
    short       whichPart;
    WindowPtr   whichWindow;
    Point       where;
    Boolean     done = FALSE;

    whichPart = MacFindWindow (theEvent->where,  &whichWindow);
    switch (whichPart)
    {
        case inSysWindow:
            SystemClick( theEvent, whichWindow);
            break;
        case inContent:
            where = theEvent->where;
            GlobalToLocal( &where);
            break;
        case inGoAway:
            if ( TrackGoAway (whichWindow, theEvent->where))
            {
                ShowHide ( whichWindow, FALSE);
                if ( whichWindow == gTheWindow)
                    done = TRUE;
            }
            break;
    }
    return ( done);
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

    while (( TickCount() - starttime) < time) {
        // DO NOTHING
    }
}
