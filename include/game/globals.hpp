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
#include "data/handle.hpp"
#include "data/scenario.hpp"
#include "drawing/color.hpp"
#include "game/starfield.hpp"
#include "math/random.hpp"
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
    int16_t         top;
    int32_t         thisValue;
    uint8_t         color;
    bool         automatic;      // if it's automatic, it is redrawn automatically
};

struct miniScreenLineType;
struct miniComputerDataType {
    std::unique_ptr<miniScreenLineType[]> lineData;
    std::unique_ptr<SpaceObject[]> objectData;
    int32_t                 selectLine;
    int32_t                 pollTime;
    int32_t                 buildTimeBarValue;
    int32_t                 currentScreen;
    int32_t                 clickLine;
};

struct hotKeyType {
    Handle<SpaceObject>     object;
    int32_t                 objectID;
};

struct Admiral;
struct beamType;
struct Destination;
struct proximityUnitType;
struct scrollStarType;
class InputSource;
class StringList;

struct GlobalState {
    int64_t  time;    // Current game time.
    Random   random;  // Global random number generator.

    std::unique_ptr<Admiral[]>  admirals;  // All admirals (whether active or not).
    Handle<Admiral>             admiral;   // Local player.

    std::unique_ptr<SpaceObject[]>  objects;  // All space objects (whether active or not).
    Handle<SpaceObject>             ship;     // Local player's flagship.
    Handle<SpaceObject>             root;     // Head of LL of active objs, in creation time order.

    std::unique_ptr<beamType[]>     beams;         // Auxiliary info for kIsBeam objects.
    std::unique_ptr<Destination[]>  destinations;  // Auxiliary info for kIsDestination objects.

    bool             game_over;     // True if an admiral won or lost the level.
    int64_t          game_over_at;  // The time to stop the game (ignored unless game_over).
    Handle<Admiral>  victor;        // The winner (or none).
    int              next_level;    // Next level (or -1 for none).
    int16_t          victory_text;  // Text resource to show in debriefing.
};

extern GlobalState g;

struct aresGlobalType {
    aresGlobalType();
    ~aresGlobalType();

    uint32_t        gSynchValue;
    std::unique_ptr<InputSource> gInputSource;
    uint64_t        gLastTime;
    Handle<SpaceObject> gClosestObject;
    Handle<SpaceObject> gFarthestObject;
    int32_t         gCenterScaleH;
    int32_t         gCenterScaleV;
    Handle<Label>   gSelectionLabel;
    ZoomType        gZoomMode;
    ZoomType        gPreviousZoomMode;

    int32_t         gRadarCount;            // = 0;
    bool            radar_is_functioning;
    int32_t         gWhichScaleNum;         // = 0;
    int32_t         gLastScale;             // = SCALE_SCALE;
    int32_t         gInstrumentTop;         // = 0;
    barIndicatorType    gBarIndicator[ kBarIndicatorNum];
    miniComputerDataType    gMiniScreenData;
    std::unique_ptr<StringList>          gMissionStatusStrList;
    smartSoundHandle    gSound[kSoundNum];
    smartSoundChannel   gChannel[kMaxChannelNum];
    int32_t         gLastSoundTime;         // = 0
    std::unique_ptr<StringList>        gAresCheatStrings;

    KeyMap          gLastMessageKeyMap;
    bool         gAutoPilotOff;          // hack for turning off auto in netgame
    uint32_t        keyMask;
    scenarioInfoType    scenarioFileInfo;   // x-ares; for factory +
                                            // 3rd party files
    int32_t         scenarioNum;

    hotKeyType      hotKey[kHotKeyNum];
    int32_t         hotKeyDownTime;
    int32_t         lastHotKey;

    Handle<SpaceObject>     lastSelectedObject;
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
