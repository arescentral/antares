// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2015-2017 The Antares Authors
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

#ifndef ANTARES_DATA_ACTION_HPP_
#define ANTARES_DATA_ACTION_HPP_

#include <stdint.h>
#include <memory>
#include <pn/string>
#include <sfz/sfz.hpp>
#include <vector>

#include "data/cash.hpp"
#include "data/counter.hpp"
#include "data/enums.hpp"
#include "data/handle.hpp"
#include "data/object-ref.hpp"
#include "data/range.hpp"
#include "data/tags.hpp"
#include "drawing/color.hpp"
#include "math/fixed.hpp"
#include "math/geometry.hpp"
#include "math/units.hpp"

namespace antares {

class SpaceObject;
union Action;
union Level;
struct Initial;
struct Condition;
class path_value;

enum class ActionType {
    AGE,
    ASSUME,
    CAPTURE,
    CAP_SPEED,
    CHECK,
    CLOAK,
    CONDITION,
    CREATE,
    DELAY,
    DESTROY,
    DISABLE,
    ENERGIZE,
    EQUIP,
    FIRE,
    FLASH,
    GROUP,
    HEAL,
    HOLD,
    KEY,
    LAND,
    MESSAGE,
    MORPH,
    MOVE,
    OCCUPY,
    PAY,
    PLAY,
    PUSH,
    REMOVE,
    REVEAL,
    SCORE,
    SELECT,
    SLOW,
    SPARK,
    SPEED,
    SPIN,
    STOP,
    TARGET,
    THRUST,
    WARP,
    WIN,
    ZOOM,
};

enum class Within { CIRCLE, SQUARE };

struct ActionBase {
    ActionType type;

    sfz::optional<bool> reflexive;  // does it apply to object executing verb?

    struct Filter {
        struct Attributes {
            uint32_t bits = 0;
        };
        Attributes           attributes;
        Tags                 tags;
        sfz::optional<Owner> owner;
    } filter;

    struct Override {
        sfz::optional<ObjectRef> subject;
        sfz::optional<ObjectRef> direct;
    } override_;
};

struct AgeAction : public ActionBase {
    sfz::optional<bool> relative;  // if true, add value to age; if false, set age to value
    Range<ticks>        value;     // age range
};

struct AssumeAction : public ActionBase {
    int64_t which;  // which initial to become
                    // Note: player 1’s score 0 is added to this number
};

struct CapSpeedAction : public ActionBase {
    sfz::optional<Fixed> value;  // if absent set to base type’s default
};

struct CaptureAction : public ActionBase {
    sfz::optional<Handle<Admiral>>
            player;  // if present, set focus object’s owner to `*player`
                     // if absent and reflexive, set focus object’s owner to direct object’s
                     // if absent and non-reflexive, set focus object’s owner to subject object’s
};

struct CheckAction : public ActionBase {};

struct CloakAction : public ActionBase {};

struct ConditionAction : public ActionBase {
    std::vector<Handle<const Condition>> enable;
    std::vector<Handle<const Condition>> disable;
};

struct CreateAction : public ActionBase {
    NamedHandle<const BaseObject> base;                // what type
    sfz::optional<Range<int64_t>> count;               // # to make randomly
    sfz::optional<bool>           relative_velocity;   // is velocity relative to creator?
    sfz::optional<bool>           relative_direction;  // determines initial heading
    sfz::optional<int64_t>        distance;  // create at this distance in random direction
    Within                        within = Within::CIRCLE;
    sfz::optional<bool>           inherit;  // if false, gets creator as target
                                            // if true, gets creator’s target as target
    sfz::optional<bool> legacy_random;      // if true, consume a random number from
                                            // subject object even if not necessary
};

struct DelayAction : public ActionBase {
    ticks duration;
};

struct DestroyAction : public ActionBase {};

struct DisableAction : public ActionBase {
    Range<ticks> duration;
};

struct EnergizeAction : public ActionBase {
    int64_t value;
};

struct EquipAction : public ActionBase {
    Weapon                        which;
    NamedHandle<const BaseObject> base;
};

struct FireAction : public ActionBase {
    Weapon which;
};

struct FlashAction : public ActionBase {
    ticks    duration;  // length of flash
    RgbColor color;     // color of flash
};

struct GroupAction : public ActionBase {
    std::vector<Action> of;
};

struct HealAction : public ActionBase {
    int64_t value;
};

struct HoldAction : public ActionBase {};

struct KeyAction : public ActionBase {
    enum class Key {
        UP            = 0,
        DOWN          = 1,
        LEFT          = 2,
        RIGHT         = 3,
        PULSE         = 4,
        BEAM          = 5,
        SPECIAL       = 6,
        WARP          = 7,
        SELECT_FRIEND = 8,
        SELECT_FOE    = 9,
        SELECT_BASE   = 10,
        TARGET        = 11,
        ORDER         = 12,
        ZOOM_IN       = 13,
        ZOOM_OUT      = 14,
        COMP_UP       = 15,
        COMP_DOWN     = 16,
        COMP_ACCEPT   = 17,
        COMP_BACK     = 18,

        COMP_MESSAGE  = 26,
        COMP_SPECIAL  = 27,
        COMP_BUILD    = 28,
        ZOOM_SHORTCUT = 29,
        SEND_MESSAGE  = 30,
        MOUSE         = 31,
    };

    std::vector<Key> disable;  // keys to disable
    std::vector<Key> enable;   // keys to enable
};

struct LandAction : public ActionBase {
    int64_t speed;
};

struct MessageAction : public ActionBase {
    sfz::optional<int64_t>  id;     // identifies the message to a "message" condition
    std::vector<pn::string> pages;  // pages of message bodies to show
};

struct MorphAction : public ActionBase {
    sfz::optional<bool>           keep_ammo;
    NamedHandle<const BaseObject> base;
};

struct MoveAction : public ActionBase {
    enum class Origin {
        LEVEL,    // absolute coordinates, in level’s rotated frame of reference
        SUBJECT,  // relative to subject object
        DIRECT,   // relative to direct object
    };
    sfz::optional<Origin>         origin;
    sfz::optional<coordPointType> to;
    sfz::optional<int64_t>        distance;
    Within                        within;
};

struct OccupyAction : public ActionBase {
    int64_t value;
};

struct PayAction : public ActionBase {
    Cash                           value;   // amount to pay; not affected by earning power
    sfz::optional<Handle<Admiral>> player;  // if not present, pay focus object’s owner.
};

struct PlayAction : public ActionBase {
    struct Priority {
        int level;
    };
    Priority            priority;     // 1-5; takes over a channel playing a lower-priority sound
    ticks               persistence;  // time before a lower-priority sound can take channel
    sfz::optional<bool> absolute;     // plays at same volume, regardless of distance from player
    int64_t             volume;       // 1-255; volume at focus object

    struct Sound {
        pn::string sound;
    };
    sfz::optional<pn::string> sound;  // play this sound if present
    std::vector<Sound>        any;    // pick ID randomly
};

struct PushAction : public ActionBase {
    sfz::optional<Fixed> value;
};

struct RemoveAction : public ActionBase {};

struct RevealAction : public ActionBase {
    std::vector<Handle<const Initial>> initial;
};

struct ScoreAction : public ActionBase {
    RelativeCounter counter;
    int64_t         value;  // amount to change by
};

struct SelectAction : public ActionBase {
    Screen  screen;
    int64_t line;
};

struct SlowAction : public ActionBase {
    Fixed value;
};

struct SparkAction : public ActionBase {
    int64_t count;     // number of sparks to create
    Hue     hue;       // hue of sparks; they start bright and fade with time
    Fixed   velocity;  // sparks fly at at random speed up to this
    ticks   age;       // how long the spark will be visible
};

struct SpeedAction : public ActionBase {
    Fixed               value;
    sfz::optional<bool> relative;
};

struct SpinAction : public ActionBase {
    Range<Fixed> value;
};

struct StopAction : public ActionBase {};

struct TargetAction : public ActionBase {};

struct ThrustAction : public ActionBase {
    Range<Fixed> value;  // range
};

struct WarpAction : public ActionBase {};

struct WinAction : public ActionBase {
    sfz::optional<Handle<Admiral>>          player;  // victor; absent = owner of focus object
    sfz::optional<NamedHandle<const Level>> next;    // next chapter to play; absent = none
    pn::string                              text;    // "debriefing" text
};

struct ZoomAction : public ActionBase {
    Zoom value;
};

union Action {
    using Type = ActionType;

    ActionBase base;
    Type       type() const;

    AgeAction       age;
    AssumeAction    assume;
    CapSpeedAction  cap_speed;
    CaptureAction   capture;
    CheckAction     check;
    CloakAction     cloak;
    ConditionAction condition;
    CreateAction    create;
    DelayAction     delay;
    DestroyAction   destroy;
    DisableAction   disable;
    EnergizeAction  energize;
    EquipAction     equip;
    FireAction      fire;
    FlashAction     flash;
    GroupAction     group;
    HealAction      heal;
    HoldAction      hold;
    KeyAction       key;
    LandAction      land;
    MessageAction   message;
    MorphAction     morph;
    MoveAction      move;
    OccupyAction    occupy;
    PayAction       pay;
    PlayAction      play;
    PushAction      push;
    RemoveAction    remove;
    RevealAction    reveal;
    ScoreAction     score;
    SelectAction    select;
    SlowAction      slow;
    SparkAction     spark;
    SpeedAction     speed;
    SpinAction      spin;
    StopAction      stop;
    TargetAction    target;
    ThrustAction    thrust;
    WarpAction      warp;
    WinAction       win;
    ZoomAction      zoom;

    Action(AgeAction a);
    Action(AssumeAction a);
    Action(CapSpeedAction a);
    Action(CaptureAction a);
    Action(CheckAction a);
    Action(CloakAction a);
    Action(ConditionAction a);
    Action(CreateAction a);
    Action(DelayAction a);
    Action(DestroyAction a);
    Action(DisableAction a);
    Action(EnergizeAction a);
    Action(EquipAction a);
    Action(FireAction a);
    Action(FlashAction a);
    Action(GroupAction a);
    Action(HealAction a);
    Action(HoldAction a);
    Action(KeyAction a);
    Action(LandAction a);
    Action(MessageAction a);
    Action(MorphAction a);
    Action(MoveAction a);
    Action(OccupyAction a);
    Action(TargetAction a);
    Action(PayAction a);
    Action(PlayAction a);
    Action(PushAction a);
    Action(RemoveAction a);
    Action(RevealAction a);
    Action(ScoreAction a);
    Action(SelectAction a);
    Action(SlowAction a);
    Action(SparkAction a);
    Action(SpeedAction a);
    Action(SpinAction a);
    Action(StopAction a);
    Action(ThrustAction a);
    Action(WarpAction a);
    Action(WinAction a);
    Action(ZoomAction a);

    ~Action();
    Action(Action&&);
    Action& operator=(Action&&);
};

template <typename T>
struct field_reader;
template <>
struct field_reader<Action> {
    static Action read(path_value x);
};

}  // namespace antares

#endif  // ANTARES_DATA_ACTION_HPP_
