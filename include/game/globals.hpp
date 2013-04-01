// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2012 The Antares Authors
//
// This file is part of Antares, a tactical space combat game.
//
// Antares is free software: you can redistribute it and/or modify it
// under the terms of the Lesser GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Antares is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with Antares.  If not, see http://www.gnu.org/licenses/

#ifndef ANTARES_GAME_GLOBALS_HPP_
#define ANTARES_GAME_GLOBALS_HPP_

#include <queue>
#include <sfz/sfz.hpp>

#include "config/keys.hpp"
#include "data/scenario.hpp"
#include "drawing/color.hpp"
#include "game/starfield.hpp"
#include "sound/fx.hpp"
#include "video/transitions.hpp"

namespace antares {

const size_t kKeyControlNum                = 19;
const size_t kKeyExtendedControlNum     = 44;
const size_t kBarIndicatorNum              = 5;
const size_t kKeyMapBufferNum              = 256;
const size_t kHotKeyNum                    = 10;

const int32_t kLeftPanelWidth       = 128;
const int32_t kRightPanelWidth      = 32;
const int32_t kSmallScreenWidth     = 640;
const int32_t kRadarSize                = 110;

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
    int32_t         thisValue;
    unsigned char   color;
    bool         automatic;      // if it's automatic, it is redrawn automatically
};

struct miniScreenLineType;
struct miniComputerDataType {
    std::unique_ptr<miniScreenLineType[]> lineData;
    std::unique_ptr<spaceObjectType[]> objectData;
    int32_t                 selectLine;
    int32_t                 pollTime;
    int32_t                 buildTimeBarValue;
    int32_t                 currentScreen;
    int32_t                 clickLine;
};

struct hotKeyType {
    int32_t     objectNum;
    int32_t     objectID;
};

struct admiralType;
struct beamType;
struct destBalanceType;
struct proximityUnitType;
struct scrollStarType;
class InputSource;
class StringList;

struct ScenarioWinnerType {
    int8_t next;
    int16_t text;
    int8_t player;
};

struct aresGlobalType {
    aresGlobalType();
    ~aresGlobalType();

    uint32_t        gActiveCheats[kMaxPlayerNum];
    uint32_t        gSynchValue;
    std::unique_ptr<InputSource> gInputSource;
    int32_t         gGameOver;
    std::unique_ptr<admiralType[]>       gAdmiralData;
    int64_t         gGameTime;
    uint64_t        gLastTime;
    int32_t         gClosestObject;
    int32_t         gFarthestObject;
    int32_t         gCenterScaleH;
    int32_t         gCenterScaleV;
    int32_t         gPlayerShipNumber;
    int32_t         gSelectionLabel;
    ZoomType        gZoomMode;
    int32_t         gPlayerAdmiralNumber;
    ScenarioWinnerType gScenarioWinner;

    int32_t         gRadarCount;            // = 0;
    int32_t         gRadarSpeed;            // = 30;
    int32_t         gRadarRange;            // kRadarSize * 50;
    bool            radar_is_functioning;
    int32_t         gWhichScaleNum;         // = 0;
    int32_t         gLastScale;             // = SCALE_SCALE;
    int32_t         gInstrumentTop;         // = 0;
    barIndicatorType    gBarIndicator[ kBarIndicatorNum];
    miniComputerDataType    gMiniScreenData;
    std::unique_ptr<StringList>          gMissionStatusStrList;
    std::unique_ptr<beamType[]>          gBeamData;
    smartSoundHandle    gSound[kSoundNum];
    smartSoundChannel   gChannel[kMaxChannelNum];
    int32_t         gLastSoundTime;         // = 0
    std::unique_ptr<StringList>        gAresCheatStrings;
    std::unique_ptr<StringList>         key_names;
    std::unique_ptr<StringList>         key_long_names;

    KeyMap*         gKeyMapBuffer;          // = NewPtr( sizeof (KeyMap) * (int32_t)kKeyMapBufferNum;
    int32_t         gKeyMapBufferTop;       // = 0;
    int32_t         gKeyMapBufferBottom;    // = 0;
    KeyMap          gLastMessageKeyMap;
    uint32_t        gSerialNumerator;
    uint32_t        gSerialDenominator;
    bool         gAutoPilotOff;          // hack for turning off auto in netgame
    int32_t         levelNum;
    uint32_t        keyMask;
    scenarioInfoType    scenarioFileInfo;   // x-ares; for factory +
                                            // 3rd party files
    int32_t         maxScenarioBrief;
    int32_t         maxScenarioCondition;
    int32_t         maxScenarioInitial;
    int32_t         maxBaseObject;
    int32_t         maxObjectAction;
    int32_t         scenarioNum;

    hotKeyType      hotKey[kHotKeyNum];
    int32_t         hotKeyDownTime;
    int32_t         lastHotKey;

    int32_t         lastSelectedObject;
    int32_t         lastSelectedObjectID;
    bool         destKeyUsedForSelection;
    bool         hotKey_target;

    Starfield starfield;
    Transitions transitions;
};

aresGlobalType* globals();
void init_globals();

extern Rect world;
extern Rect play_screen;
extern Rect viewport;

}  // namespace antares

#endif // ANTARES_GAME_GLOBALS_HPP_
