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

#ifndef ANTARES_DATA_BASE_OBJECT_HPP_
#define ANTARES_DATA_BASE_OBJECT_HPP_

#include <pn/string>

#include "data/action.hpp"
#include "data/cash.hpp"
#include "data/distance.hpp"
#include "data/range.hpp"
#include "data/tags.hpp"
#include "drawing/color.hpp"
#include "math/fixed.hpp"
#include "math/random.hpp"

namespace antares {

enum {
    kCanTurn              = 0x00000001,  // we have to worry about its rotation velocity
    kCanBeEngaged         = 0x00000002,  // if it's worth going after
    kHasDirectionGoal     = 0x00000004,  // we must turn it towards its goal
    kIsVector             = 0x00000020,  // a vector shot, no sprite
    kDoesBounce           = 0x00000040,  // when it hits the edge, it bounces
    kIsSelfAnimated       = 0x00000080,  // cycles through animation frames
    kShapeFromDirection   = 0x00000100,  // its appearence is based on its direction
    kIsPlayerShip         = 0x00000200,  // this is the ship we focus on
    kCanBeDestination     = 0x00000400,  // can be selected as a place to go to
    kCanEngage            = 0x00000800,  // can go into engage mode
    kCanEvade             = 0x00001000,  // can go into evade mode
    kCanAcceptMessages    = 0x00002000,  // can accept a message from player
    kCanAcceptBuild       = 0x00004000,  // can accept a build message
    kCanAcceptDestination = 0x00008000,  // can accept destination message
    kAutoTarget     = 0x00010000,  // for creating a material weapon, born at parent's target angle
    kAnimationCycle = 0x00020000,  // repeating animation
    kCanCollide     = 0x00040000,  // can collide with another object
    kCanBeHit       = 0x00080000,  // can be on the receving end of a collision
    kIsDestination  = 0x00100000,  // is a point of interest to which ships can be sent
    kHideEffect     = 0x00200000,  // hides other objects around it
    kReleaseEnergyOnDeath = 0x00400000,  // when destroyed, remaining energy released
    kHated                = 0x00800000,  // when you don't own it, if it's hated, you shoot it
    kOccupiesSpace        = 0x01000000,  // 2 objects cannot occupy same physical space
    kStaticDestination    = 0x02000000,  // destination cannot be altered
    kCanBeEvaded          = 0x04000000,  // it can be veered away from
    kNeutralDeath =
            0x08000000,  // if true, object becomes neutral when "destroyed" but doesn't die
    kIsGuided =
            0x10000000,  // doesn't really think; can't accept orders; if not yours, it is feared
    kAppearOnRadar = 0x20000000,  // shows up on radar
    kOnAutoPilot = 0x80000000,  // if human controlled, this temporarily gives the computer control

    kCanThink         = kCanEngage | kCanEvade | kCanAcceptDestination,  // not just "dumb"
    kConsiderDistance = kCanThink | kCanBeDestination,
    kPotentialTarget  = kCanBeEngaged | kCanBeEvaded,
    kRemoteOrHuman    = kIsPlayerShip,

    // for initial objects only
    // kInitiallyExists should NOT be carried over to real objects!
    kInitiallyHidden = 0x00000020,  // does it exist at first, or is it turned on later?
    kFixedRace       = 0x00000010,  // don't change this object even if owner's race is different
    kInitialAttributesMask = kFixedRace | kInitiallyHidden,
};

enum {
    kUseForTransportation = 0x00000001,  // use when we're going to our destination
    kUseForAttacking      = 0x00000002,  // use when we've got a target our sites
    kUseForDefense        = 0x00000004,  // use when we're running/evading
};

enum {
    kSoftTargetIsBase      = 0x00000002,
    kHardTargetIsBase      = 0x08000000,
    kSoftTargetIsNotBase   = 0x00000004,
    kHardTargetIsNotBase   = 0x04000000,
    kSoftTargetIsLocal     = 0x00000008,
    kHardTargetIsLocal     = 0x00800000,
    kSoftTargetIsRemote    = 0x00000010,
    kHardTargetIsRemote    = 0x00400000,
    kSoftTargetIsFriend    = 0x00000040,
    kHardTargetIsFriend    = 0x02000000,
    kSoftTargetIsFoe       = 0x00000080,
    kHardTargetIsFoe       = 0x01000000,
    kSoftTargetMatchesTags = 0x10000000,
    kHardTargetMatchesTags = 0x00080000,
};

struct InvertableSpeed {
    Fixed inverse;
};

class BaseObject {
  public:
    static BaseObject* get(int number);
    static BaseObject* get(pn::string_view name);

    pn::string                long_name;
    pn::string                short_name;
    sfz::optional<pn::string> portrait;

    uint32_t attributes = 0;  // initial attributes (see flags)
    Cash     price;

    Fixed    maxVelocity;      // maximum speed
    Fixed    warpSpeed;        // multiplier of speed at warp (0 if cannot)
    Distance warpOutDistance;  // distance at which to come out of warp

    Fixed mass;       // how quickly thrust acheives max
    Fixed turn_rate;  // max rate at which object can turn
    Fixed thrust;     // maximum amount of thrust

    int32_t health;  // starting health
    int32_t energy;  // starting energy for material objects

    sfz::optional<Range<Fixed>> initial_velocity;  // initial random velocity (usually relative)
                                                   // Default: max velocity
    Range<int64_t> initial_direction;              // initial random direction (usually relative)
    bool           autotarget = false;

    int32_t occupy_count;  // size of occupying force

    sfz::optional<RgbColor> shieldColor;  // color on radar (!has_value() = don't draw shields)

    struct Icon {
        enum class Shape {
            SQUARE,
            TRIANGLE,
            DIAMOND,
            PLUS,
        } shape;
        int64_t size;
    };
    sfz::optional<Icon> icon;

    struct Targeting {
        bool base   = false;
        bool hide   = false;
        bool radar  = false;
        bool order  = false;
        bool select = false;
        bool lock   = false;
    } target;

    struct Weapon {
        NamedHandle<const BaseObject> base;
        std::vector<fixedPointType>   positions;  // relative positions (unrotated) of fire points
    };
    struct Loadout {
        sfz::optional<Weapon> pulse;
        sfz::optional<Weapon> beam;
        sfz::optional<Weapon> special;
    } weapons;

    struct Destroy {
        bool                die;
        bool                neutralize;
        bool                release_energy;
        std::vector<Action> action;
    } destroy;

    struct Expire {
        struct After {
            sfz::optional<Range<ticks>> age;  // starting random age
            bool                        animation = false;
        } after;
        bool                die;
        std::vector<Action> action;
    } expire;

    struct Create {
        std::vector<Action> action;
    } create;

    struct Collide {
        struct As {
            bool subject = false;
            bool direct  = false;
        } as;
        bool                solid  = false;
        bool                edge   = false;
        int32_t             damage = 0;
        std::vector<Action> action;
    } collide;

    struct Activate {
        sfz::optional<Range<ticks>> period;
        std::vector<Action>         action;
    } activate;

    struct Arrive {
        Distance            distance;
        std::vector<Action> action;
    } arrive;

    enum class Layer { NONE = 0, BASES = 1, SHIPS = 2, SHOTS = 3 };
    struct Scale {
        int64_t factor;  // sprite scale; 4096 = 100%
    };

    // rotation: for objects whose shapes depend on their direction
    struct Rotation {
        pn::string sprite;  // ID of sprite resource
        Layer      layer;
        Scale      scale;

        Range<int64_t> frames;
    };
    sfz::optional<Rotation> rotation;

    // animation: objects whose appearence does not depend on direction
    struct Animation {
        enum class Direction {
            PLUS   = +1,  // +
            MINUS  = -1,  // -
            RANDOM = 0,   // ?
        };

        pn::string sprite;  // ID of sprite resource
        Layer      layer;
        Scale      scale;

        Range<Fixed> frames;     // range of frames from sprite
        Direction    direction;  // frame sequence
        Fixed        speed;      // speed at which object animates
        Range<Fixed> first;      // starting shape #
    };
    sfz::optional<Animation> animation;

    // ray: point-to-point vector object
    struct Ray {
        sfz::optional<Hue> hue;  // if present, override color and cycle through shades
        enum To {
            OBJECT,
            COORD,
        } to              = To::OBJECT;
        bool    lightning = false;
        int32_t accuracy  = 0;  // for non-normal vector objects, how accurate
        int32_t range     = 0;
    };
    sfz::optional<Ray> ray;

    // bolt: moving vector object
    struct Bolt {
        RgbColor color;
    };
    sfz::optional<Bolt> bolt;

    // weapon: weapon objects have no physical form, and can only be activated
    struct Device {
        struct Usage {
            bool attacking;
            bool defense;
            bool transportation;
        } usage;
        enum Direction {
            FORE,  // should use when foe is in front of bearer
            OMNI,  // should use when foe is anywhere near bearer
        } direction = Direction::FORE;
        int32_t         energyCost;   // cost to fire
        ticks           fireTime;     // time between shots
        int32_t         ammo;         // initial ammo
        Distance        range;        // range (= age * max velocity)
        InvertableSpeed speed;        // for AI: inverse = 1/max velocity
        int32_t         restockCost;  // energy to make new ammo
    };
    sfz::optional<Device> device;

    uint32_t orderFlags = 0;
    Tags     tags;
    ticks    buildTime;

    struct AI {
        struct Combat {
            struct Engage {
                sfz::optional<bool> unconditional;  // if has value, ignore if_.
                struct If {
                    Tags tags;
                } if_;
            };

            bool   hated  = false;
            bool   guided = false;
            Engage engages;
            Engage engaged;
            bool   evades = false;
            bool   evaded = false;
            struct Skill {
                uint8_t num;
                uint8_t den;
            } skill;
        } combat;

        struct Target {
            struct Filter {
                sfz::optional<bool> base;
                sfz::optional<bool> local;
                Owner               owner;
                Tags                tags;
            };
            Filter prefer;
            Filter force;
        } target;

        struct Escort {
            int   class_ = 0;
            Fixed power  = Fixed::zero();
            Fixed need   = Fixed::zero();
        } escort;

        struct Build {
            Fixed ratio              = Fixed::zero();
            bool  needs_escort       = false;
            bool  legacy_non_builder = false;
        } build;
    } ai;
};
BaseObject base_object(pn::value_cref x);

}  // namespace antares

#endif  // ANTARES_DATA_BASE_OBJECT_HPP_
