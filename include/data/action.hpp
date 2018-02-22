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
#include <pn/string>
#include <memory>
#include <vector>

#include "data/handle.hpp"
#include "math/fixed.hpp"
#include "math/geometry.hpp"
#include "math/units.hpp"

namespace antares {

class SpaceObject;
struct Level_Initial;

//
// Action:
//  Defines any action that an object can take.  Conditions that can cause an action to execute
//  are:
//  destroy, expire, create, collide, activate, or message.
//

struct Action {
    bool reflexive;  // does it apply to object executing verb?

    uint32_t inclusiveFilter;  // if it has ALL these attributes, OK -- for non-reflective verbs
    uint32_t exclusiveFilter;  // don't execute if it has ANY of these
    uint8_t  levelKeyTag;

    enum class Owner { ANY = 0, SAME = 1, DIFFERENT = -1 };
    Owner owner;
    ticks delay;

    Handle<Level_Initial> initialSubjectOverride;
    Handle<Level_Initial> initialDirectOverride;

    virtual ~Action() = default;
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset) const = 0;

    virtual Handle<BaseObject>  created_base() const;
    virtual std::pair<int, int> sound_range() const;
    virtual bool                alters_owner() const;
    virtual bool                check_conditions() const;

    static const size_t byte_size = 48;
};
bool read_from(pn::file_view in, std::unique_ptr<const Action>* action);

std::vector<std::unique_ptr<const Action>> read_actions(int begin, int end);

struct NoAction : public Action {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset) const;
};

struct AgeAction : public Action {
    bool                    relative;  // if true, add value to age; if false, set age to value
    std::pair<ticks, ticks> value;     // age range

    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset) const;
};

struct AssumeAction : public Action {
    int32_t which;  // which initial to become
                    // Note: player 1’s score 0 is added to this number

    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset) const;
};

struct CapSpeedAction : public Action {
    Fixed value;  // if >= 0, set to value; if < 0, set to base type’s default

    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset) const;
};

struct CaptureAction : public Action {
    bool relative;  // if true and reflexive, set subject’s owner to object’s
                    // if true and non-reflexive, set object’s owner to subject’s
                    // if false, set focus’s owner to `player`
    Handle<Admiral> player;

    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset) const;
    virtual bool alters_owner() const;
};

struct CloakAction : public Action {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset) const;
};

struct ConditionAction : public Action {
    bool                        enabled;
    std::pair<int32_t, int32_t> which;

    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset) const;
};

struct CreateAction : public Action {
    Handle<BaseObject> base;                        // what type
    int32_t            count_minimum      = 1;      // # to make min
    int32_t            count_range        = 0;      // # to make range
    bool               relative_velocity  = false;  // is velocity relative to creator?
    bool               relative_direction = false;  // determines initial heading
    int32_t            distance           = 0;      // create at this distance in random direction
    bool               inherit            = false;  // if false, gets creator as target
                                                    // if true, gets creator’s target as target

    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset) const;
    virtual Handle<BaseObject> created_base() const;
};

struct DisableAction : public Action {
    std::pair<Fixed, Fixed> value;

    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset) const;
};

struct EnergizeAction : public Action {
    int32_t value;

    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset) const;
};

struct EquipAction : public Action {
    enum class Which { PULSE, BEAM, SPECIAL };
    Which              which;
    Handle<BaseObject> base;

    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset) const;
    virtual Handle<BaseObject> created_base() const;
};

struct FireAction : public Action {
    enum class Which { PULSE, BEAM, SPECIAL };
    Which which;

    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset) const;
};

struct FlashAction : public Action {
    int32_t length;  // length of color flash
    uint8_t hue;     // hue of flash
    uint8_t shade;   // brightness of flash

    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset) const;
};

struct HealAction : public Action {
    int32_t value;

    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset) const;
};

struct HoldPositionAction : public Action {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset) const;
};

struct KeyAction : public Action {
    uint32_t disable = 0;  // keys to disable
    uint32_t enable  = 0;  // keys to enable

    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset) const;
};

struct KillAction : public Action {
    enum class Kind {
        // Removes the focus without any further fanfare.
        NONE = 0,

        // Removes the subject without any further fanfare.
        // Essentially, this is NONE, but always reflexive.
        EXPIRE = 1,

        // Removes the subject and executes its destroy action.
        DESTROY = 2,
    };
    Kind kind;

    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset) const;
};

struct LandAction : public Action {
    int32_t speed;

    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset) const;
};

struct MessageAction : public Action {
    int16_t                 id;     // identifies the message to a "message" condition
    std::vector<pn::string> pages;  // pages of message bodies to show

    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset) const;
    virtual bool check_conditions() const;
};

struct MorphAction : public Action {
    bool               keep_ammo;
    Handle<BaseObject> base;

    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset) const;
    virtual Handle<BaseObject> created_base() const;
};

struct MoveAction : public Action {
    enum Origin {
        LEVEL,    // absolute coordinates, in level’s rotated frame of reference
        SUBJECT,  // relative to subject
        OBJECT,   // relative to object
        FOCUS,    // relative to focus
    };
    Origin         origin;
    coordPointType to;
    int32_t        distance;

    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset) const;
};

struct OccupyAction : public Action {
    int32_t value;

    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset) const;
};

struct OrderAction : public Action {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset) const;
};

struct PayAction : public Action {
    bool            relative;  // if true, pay focus’s owner; if false, pay `player`
    Fixed           value;     // amount to pay; not affected by earning power
    Handle<Admiral> player;

    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset) const;
};

struct PushAction : public Action {
    enum class Kind {
        STOP,        // set focus’s velocity to 0
        COLLIDE,     // impart velocity from subject like a collision (capped)
        DECELERATE,  // decrease focus’s velocity (capped)
        SET,         // set focus’s velocity to value in subject’s direction
        BOOST,       // add to focus’s velocity in subject’s direction
        CRUISE,      // set focus’s velocity in focus’s direction
    };
    Kind  kind;
    Fixed value;

    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset) const;
};

struct RevealAction : public Action {
    HandleList<Level_Initial> initial;

    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset) const;
};

struct ScoreAction : public Action {
    Handle<Admiral> player;  // which player’s score to change; -1 = owner of focus
    int32_t         which;   // 0-2; each player has three "scores"
    int32_t         value;   // amount to change by

    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset) const;
    virtual bool check_conditions() const;
};

struct SelectAction : public Action {
    int32_t screen;
    int32_t line;

    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset) const;
};

struct SoundAction : public Action {
    uint8_t priority;                // 1-5; takes over a channel playing a lower-priority sound
    ticks   persistence;             // time before a lower-priority sound can take channel
    bool    absolute;                // plays at same volume, regardless of distance from player
    int32_t volume;                  // 1-255; volume at focus
    std::pair<int32_t, int32_t> id;  // pick ID randomly in [first, second)

    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset) const;
    virtual std::pair<int, int> sound_range() const;
};

struct SparkAction : public Action {
    int32_t count;     // number of sparks to create
    uint8_t hue;       // hue of sparks; they start bright and fade with time
    int32_t decay;     // sparks will be visible for 17.05/decay seconds
    Fixed   velocity;  // sparks fly at at random speed up to this

    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset) const;
};

struct SpinAction : public Action {
    std::pair<Fixed, Fixed> value;

    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset) const;
};

struct ThrustAction : public Action {
    bool                    relative;  // if true, set to value; if false, add value
    std::pair<Fixed, Fixed> value;     // range

    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset) const;
};

struct WarpAction : public Action {
    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset) const;
};

struct WinAction : public Action {
    Handle<Admiral> player;  // victor; -1 = owner of focus
    int32_t         next;    // next chapter to play; -1 = none
    pn::string      text;    // "debriefing" text

    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset) const;
};

struct ZoomAction : public Action {
    int32_t value;

    virtual void apply(
            Handle<SpaceObject> subject, Handle<SpaceObject> focus, Handle<SpaceObject> object,
            Point* offset) const;
};

}  // namespace antares

#endif  // ANTARES_DATA_ACTION_HPP_
