// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2017 The Antares Authors
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
#include "data/level.hpp"
#include "data/string-list.hpp"
#include "drawing/color.hpp"
#include "game/starfield.hpp"
#include "math/random.hpp"
#include "math/units.hpp"
#include "sound/fx.hpp"
#include "video/transitions.hpp"

namespace antares {

const size_t kBarIndicatorNum = 5;

const int32_t kLeftPanelWidth   = 128;
const int32_t kRightPanelWidth  = 32;
const int32_t kSmallScreenWidth = 640;
const int32_t kRadarSize        = 110;

enum ZoomType {
    kTimesTwoZoom        = 0,
    kActualSizeZoom      = 1,
    kHalfSizeZoom        = 2,
    kQuarterSizeZoom     = 3,
    kEighthSizeZoom      = 4,
    kNearestFoeZoom      = 5,
    kNearestAnythingZoom = 6,
    kSmallestZoom        = 7,
};

struct miniScreenLineType;
struct miniComputerDataType {
    std::unique_ptr<miniScreenLineType[]> lineData;
    int32_t                               selectLine;
    int32_t                               currentScreen;
    int32_t                               clickLine;
};

struct hotKeyType {
    Handle<SpaceObject> object;
    int32_t             objectID;
};

class Admiral;
struct Vector;
struct Destination;
struct proximityUnitType;
struct scrollStarType;
class Sprite;
class InputSource;
class StringList;

struct GlobalState {
    uint32_t   sync;    // Indicates when net games are desynchronized.
    game_ticks time;    // Current game time.
    Random     random;  // Global random number generator.

    Handle<Level> level;
    int32_t       angle;

    std::unique_ptr<Admiral[]> admirals;  // All admirals (whether active or not).
    Handle<Admiral>            admiral;   // Local player.

    std::unique_ptr<SpaceObject[]> objects;  // All space objects (whether active or not).
    Handle<SpaceObject>            ship;     // Local player's flagship.
    Handle<SpaceObject>            root;     // Head of LL of active objs, in creation time order.

    std::unique_ptr<Vector[]>      vectors;       // Auxiliary info for kIsVector objects.
    std::unique_ptr<Destination[]> destinations;  // Auxiliary info for kIsDestination objects.
    std::unique_ptr<Sprite[]>      sprites;       // Auxiliary info for objects with sprites.

    bool            game_over;     // True if an admiral won or lost the level.
    game_ticks      game_over_at;  // The time to stop the game (ignored unless game_over).
    Handle<Admiral> victor;        // The winner (or none).
    int             next_level;    // Next level (or -1 for none).
    int16_t         victory_text;  // Text resource to show in debriefing.

    ticks                    radar_count;  // Counts down to a radar pulse every 5/6 seconds.
    std::unique_ptr<Point[]> radar_blips;  // Screen locations of radar blips.
    bool                     radar_on;     // Maybe false if player ship is offline.

    std::unique_ptr<Label[]> labels;
    Handle<Label>            control_label;  // Local player's current control object.
    Handle<Label>            target_label;   // Local player's current target object.
    Handle<Label>            message_label;  // Destroyed, captured, lost messages.
    Handle<Label>            status_label;   // Autopilot, zoom, low shields messages.
    Handle<Label>            send_label;     // Message local player is currently entering.

    int32_t bottom_border;  // When a message is being displayed.

    uint32_t key_mask;  // Determines input to reject.

    miniComputerDataType mini;

    ZoomType            zoom;
    Handle<SpaceObject> closest;   // Nearest object or hostile, depending on zoom.
    Handle<SpaceObject> farthest;  // Farthest object (sufficient for zoom-to-all).
};

extern GlobalState& g;  // head
extern GlobalState  head;
extern GlobalState  tail;

struct aresGlobalType {
    aresGlobalType();
    ~aresGlobalType();

    hotKeyType hotKey[kHotKeyNum];

    Handle<SpaceObject> lastSelectedObject;
    int32_t             lastSelectedObjectID;

    game_ticks next_klaxon;

    Starfield   starfield;
    Transitions transitions;
};

aresGlobalType* globals();
void            init_globals();

Rect world();
Rect play_screen();
Rect viewport();

}  // namespace antares

#endif  // ANTARES_GAME_GLOBALS_HPP_
