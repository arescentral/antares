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
#include <Quickdraw.h>

#include "AnyChar.hpp"
#include "NateDraw.hpp"
#include "ScenarioData.hpp"
#include "SmartPtr.hpp"
#include "SoundFX.hpp"

namespace antares {

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
    bool         automatic;      // if it's automatic, it is redrawn automatically
};

struct miniScreenLineType;
struct miniComputerDataType {
    scoped_array<miniScreenLineType> lineData;
    scoped_array<spaceObjectType> objectData;
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
struct Preferences;
struct proximityUnitType;
struct raceType;
struct scenarioType;
struct scenarioConditionType;
struct scenarioInitialType;
struct screenLabelType;
struct scrollStarType;
class InputSource;
class MessageData;
class StringList;

struct ScenarioWinnerType {
    int8_t next;
    int16_t text;
    int8_t player;
};

struct aresGlobalType {
    aresGlobalType();
    ~aresGlobalType();

    unsigned long   gActiveCheats[kMaxPlayerNum];
    unsigned long   gSynchValue;
    long            gForceDemoLevel;
    scoped_ptr<InputSource> gInputSource;
    short           gMainResRefNum;
    unsigned long   gFrameCount;
    scoped_ptr<Preferences> gPreferencesData;
    long            gGameOver;
    scoped_array<admiralType>       gAdmiralData;
    scoped_array<destBalanceType>   gDestBalanceData;
    KeyMap          gKeyControl[kKeyExtendedControlNum];
    short           gPreferenceRefNum;
    unsigned long   gOptions;
    scoped_array<raceType>           gRaceData;
    scoped_array<scrollStarType>    gScrollStarData;
    bool         gWarpStars;
    long            gLastClipBottom;
    long            gScrollStarNumber;
    long            gGameTime;
    long            gGameStartTime;
    uint64_t        gLastTime;
    long            gClosestObject;
    long            gFarthestObject;
    long            gCenterScaleH;
    long            gCenterScaleV;
    scoped_array<proximityUnitType> gProximityGrid;
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
    long            gPlayerAdmiralNumber;
    ScenarioWinnerType gScenarioWinner;
    long            gScenarioRotation;  // = 0;
    long            gThisScenarioNumber;// = -1;
    short           gScenarioRefID;     // = 0;
    scoped_array<scenarioType>          gScenarioData;      // = nil;
    scoped_array<scenarioInitialType>    gScenarioInitialData;   // = nil;
    scoped_array<scenarioConditionType>  gScenarioConditionData; // = nil;
    scoped_array<briefPointType>     gScenarioBriefData;     // = nil;
    scoped_array<Point>             gRadarBlipData;         // = nil;
    scoped_array<int32_t>           gScaleList;             // = nil;
    scoped_array<int32_t>           gSectorLineData;        // = nil;
    int32_t         gRadarCount;            // = 0;
    int32_t         gRadarSpeed;            // = 30;
    int32_t         gRadarRange;            // kRadarSize * 50;
    int32_t         gWhichScaleNum;         // = 0;
    int32_t         gLastScale;             // = SCALE_SCALE;
    int32_t         gInstrumentTop;         // = 0;
    int32_t         gRightPanelLeftEdge;    // = 608
    barIndicatorType    gBarIndicator[ kBarIndicatorNum];
    short           gMouseActive;           // = kMouseOff;
    scoped_ptr<MessageData>         gMessageData;
    scoped_array<unsigned char>     gStatusString;          // = nil
    scoped_ptr<longMessageType>     gLongMessageData;       // = nil
    long            gMessageTimeCount;      // = 0;
    long            gMessageLabelNum;       // = -1;
    long            gStatusLabelNum;        // = -1;
    long            gTrueClipBottom;        // = 0;
//  Handle          gMiniScreenHandle;      // = nil;
    miniComputerDataType    gMiniScreenData;
    scoped_ptr<StringList>          gMissionStatusStrList;
    scoped_array<screenLabelType>   gScreenLabelData;       // = nil;
    scoped_array<beamType>          gBeamData;              // = nil;
    long            gColorAnimationStep;    // = 0;
    long            gColorAnimationInSpeed; // = -1;
    long            gColorAnimationOutSpeed;// = -1;
    scoped_ptr<ColorTable>          gColorAnimationTable;   // = nil;
    scoped_ptr<ColorTable>          gSaveColorTable;        // = nil;
    RGBColor        gColorAnimationGoal;
    smartSoundHandle    gSound[kSoundNum];
    smartSoundChannel   gChannel[kMaxChannelNum];
    long            gLastSoundTime;         // = 0
    long            gSoundVolume;           // = 0;
    short           gSoundFileRefID;        // = 0;
    scoped_ptr<StringList>        gAresCheatStrings;
    KeyMap*         gKeyMapBuffer;          // = NewPtr( sizeof (KeyMap) * (long)kKeyMapBufferNum;
    long            gKeyMapBufferTop;       // = 0;
    long            gKeyMapBufferBottom;    // = 0;
    KeyMap          gLastMessageKeyMap;
    unsigned long   gSerialNumerator;
    unsigned long   gSerialDenominator;
    long            gLastSelectedBuildPrice;
    Str255          gUserName;
    bool         gAutoPilotOff;          // hack for turning off auto in netgame
    long            levelNum;
    unsigned long   keyMask;
    scenarioInfoType    scenarioFileInfo;   // x-ares; for factory +
                                            // 3rd party files
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

    hotKeyType      hotKey[kHotKeyNum];
    long            hotKeyDownTime;
    long            lastHotKey;

    long            lastSelectedObject;
    long            lastSelectedObjectID;
    bool         destKeyUsedForSelection;
    bool         hotKey_target;
};

aresGlobalType* globals();
void init_globals();

}  // namespace antares

#endif // ANTARES_ARES_GLOBAL_TYPE_HPP_
