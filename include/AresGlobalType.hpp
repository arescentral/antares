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

#ifndef ANTARES_ARES_GLOBAL_TYPE_HPP_
#define ANTARES_ARES_GLOBAL_TYPE_HPP_

// Ares Global Type.h

#include <Base.h>
#include <Files.h>
#include <Quickdraw.h>
#include <Resources.h>

#include "AnyChar.hpp"
#include "Handle.hpp"
#include "ICAPI.h"
#include "NateDraw.hpp"
#include "ScenarioData.hpp"
#include "SoundFX.hpp"

#pragma options align=mac68k

#define kDefaultOptions             (kOptionScreenSmall | kOptionBlackground | kOptionMusicIdle)
#define kMaxPlayerNum               4 // kMaxScenarioPlayerNum in Scenario.h

#define kKeyControlNum          19
#define kKeyExtendedControlNum  44//44//34

#define kRadarSize          110
#define kBarIndicatorNum    5
#define kMouseOff           0
#define kMouseTurningOff    1
#define kMouseActive        2

#define kKeyMapBufferNum    256

#define kHotKeyNum              10

enum ZoomType {
    kTimesTwoZoom           = 0,
    kActualSizeZoom         = 1,
    kHalfSizeZoom           = 2,
    kQuarterSizeZoom        = 3,
    kEighthSizeZoom         = 4,
    kNearestFoeZoom         = 5,
    kNearestAnythingZoom    = 6,
    kSmallestZoom           = 7,
};

struct barIndicatorType {
    short           top;
    long            thisValue;
    long            lastValue;
    unsigned char   color;
    Boolean         automatic;      // if it's automatic, it is redrawn automatically
};

struct miniScreenLineType;
struct miniComputerDataType {
    TypedHandle<miniScreenLineType> lineData;
    TypedHandle<spaceObjectType> objectData;
    long                    selectLine;
    long                    pollTime;
    long                    buildTimeBarValue;
    long                    currentScreen;
    long                    clickLine;
};

struct hotKeyType {
    long        objectNum;
    long        objectID;
};

struct admiralType;
struct beamType;
struct briefPointType;
struct destBalanceType;
struct longMessageType;
struct preferencesDataType;
struct proximityUnitType;
struct raceType;
struct scenarioType;
struct scenarioConditionType;
struct scenarioInitialType;
struct screenLabelType;
struct scrollStarType;
class StringList;

struct aresGlobalType {
    unsigned long   gActiveCheats[kMaxPlayerNum];
    unsigned long   gSynchValue;
    CWindowPtr      gBackWindow;
    long            gForceDemoLevel;
    TypedHandle<unsigned long> gReplayData;
    short           gMainResRefNum;
    unsigned long   gFrameCount;
    TypedHandle<preferencesDataType>    gPreferencesData;
    long            gGameOver;
    TypedHandle<admiralType>        gAdmiralData;
    TypedHandle<destBalanceType>    gDestBalanceData;
    KeyMap          gKeyControl[kKeyExtendedControlNum];
    short           gPreferenceRefNum;
    unsigned long   gOptions;
    TypedHandle<raceType>           gRaceData;
    TypedHandle<scrollStarType>     gScrollStarData;
    Boolean         gWarpStars;
    long            gLastClipBottom;
    long            gScrollStarNumber;
    long            gGameTime;
    long            gGameStartTime;
    UnsignedWide    gLastTime;
    long            gClosestObject;
    long            gFarthestObject;
    long            gCenterScaleH;
    long            gCenterScaleV;
    TypedHandle<proximityUnitType>  gProximityGrid;
    KeyMap          gLastKeyMap;
    unsigned long   gLastKeys;
    unsigned long   gTheseKeys;
    long            gPlayerShipNumber;
    long            gSelectionLabel;
    long            gDestKeyTime;
    ZoomType        gZoomMode;
    long            gDestinationLabel;
    long            gAlarmCount;
    long            gSendMessageLabel;
    Boolean         gDemoZoomOverride;
    long            gPlayerAdmiralNumber;
    long            gScenarioWinner;    // -1 = noone, 0 = player loses
    long            gScenarioRotation;  // = 0;
    long            gThisScenarioNumber;// = -1;
    short           gScenarioRefID;     // = 0;
    TypedHandle<scenarioType>       gScenarioData;      // = nil;
    TypedHandle<scenarioInitialType>    gScenarioInitialData;   // = nil;
    TypedHandle<scenarioConditionType>  gScenarioConditionData; // = nil;
    TypedHandle<briefPointType>     gScenarioBriefData;     // = nil;
    TypedHandle<longPointType>      gRadarBlipData;         // = nil;
    TypedHandle<long>               gScaleList;             // = nil;
    TypedHandle<long>               gSectorLineData;        // = nil;
    long            gRadarCount;            // = 0;
    long            gRadarSpeed;            // = 30;
    long            gRadarRange;            // kRadarSize * 50;
    long            gWhichScaleNum;         // = 0;
    long            gLastScale;             // = SCALE_SCALE;
    long            gInstrumentTop;         // = 0;
    long            gRightPanelLeftEdge;    // = 608
    barIndicatorType    gBarIndicator[ kBarIndicatorNum];
    short           gMouseActive;           // = kMouseOff;
    TypedHandle<unsigned char>      gMessageData;           // = nil
    TypedHandle<unsigned char>      gStatusString;          // = nil
    TypedHandle<longMessageType>    gLongMessageData;       // = nil
    long            gMessageTimeCount;      // = 0;
    long            gMessageLabelNum;       // = -1;
    long            gStatusLabelNum;        // = -1;
    long            gTrueClipBottom;        // = 0;
//  Handle          gMiniScreenHandle;      // = nil;
    miniComputerDataType    gMiniScreenData;
    TypedHandle<StringList>         gMissionStatusStrList;
    TypedHandle<screenLabelType>    gScreenLabelData;       // = nil;
    TypedHandle<beamType>           gBeamData;              // = nil;
    long            gColorAnimationStep;    // = 0;
    long            gColorAnimationInSpeed; // = -1;
    long            gColorAnimationOutSpeed;// = -1;
    CTabHandle      gColorAnimationTable;   // = nil;
    CTabHandle      gSaveColorTable;        // = nil;
    RGBColor        gColorAnimationGoal;
    smartSoundHandle    gSound[kSoundNum];
    smartSoundChannel   gChannel[kMaxChannelNum];
    long            gLastSoundTime;         // = 0
    long            gSoundVolume;           // = 0;
    short           gSoundFileRefID;        // = 0;
    TypedHandle<StringList>         gAresCheatStrings;
    KeyMap*         gKeyMapBuffer;          // = NewPtr( sizeof (KeyMap) * (long)kKeyMapBufferNum;
    long            gKeyMapBufferTop;       // = 0;
    long            gKeyMapBufferBottom;    // = 0;
    KeyMap          gLastMessageKeyMap;
    unsigned long   gSerialNumerator;
    unsigned long   gSerialDenominator;
    long            gLastSelectedBuildPrice;
    Str255          gUserName;
    Boolean         gAutoPilotOff;          // hack for turning off auto in netgame
    Boolean         isQuitting;             // x-ares; for appleevent handling
    Boolean         returnToMain;           // x-ares;
    // returnToMain is for both getting a quit appleEvent or
    // a GameRanger command; it stops whatever is going on
    // and returns to main menu
    Boolean         aeInited;               // x-ares; for appleevent handling
    Boolean         gameRangerPending;      // x-ares; for gameRanger
    Boolean         gameRangerInProgress;
    Boolean         useGameRanger;
    Boolean         haveSeenRTNotice;
    Boolean         ambrosia_Is_Registered;
    Boolean         user_is_scum;
    long            levelNum;
    unsigned long   keyMask;
    FSSpec          originalExternalFileSpec;
    FSSpec          externalFileSpec;       // x-ares; for 3rd party files
    short           externalFileRefNum;     // x-ares; for 3rd party files
    scenarioInfoType    scenarioFileInfo;   // x-ares; for factory +
                                            // 3rd party files
    Boolean         okToOpenFile;           // x-ares; only ok to open
                                            // before factory is loaded
    long            maxScenarioBrief;
    long            maxScenarioCondition;
    long            maxScenarioInitial;
    long            maxBaseObject;
    long            maxObjectAction;
    long            scenarioNum;

    Str255          otherPlayerScenarioFileName;    // x-ares; 3rd part net files
    Str255          otherPlayerScenarioFileURL;     // ''
    unsigned long   otherPlayerScenarioFileVersion;
    unsigned long   otherPlayerScenarioFileCheckSum;

    Boolean         internetConfigPresent;
    ICInstance      internetConfig;

    hotKeyType      hotKey[kHotKeyNum];
    long            hotKeyDownTime;
    long            lastHotKey;

    long            lastSelectedObject;
    long            lastSelectedObjectID;
    Boolean         destKeyUsedForSelection;
    Boolean         hotKey_target;
};

extern aresGlobalType* gAresGlobal;

#pragma options align=reset

#endif // ANTARES_ARES_GLOBAL_TYPE_HPP_
