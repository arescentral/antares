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

#ifndef ANTARES_GAME_SPACE_OBJECT_HPP_
#define ANTARES_GAME_SPACE_OBJECT_HPP_

#include "data/space-object.hpp"

namespace antares {

const int16_t kBaseObjectResID      = 500;
const int16_t kObjectActionResID    = 500;

enum dutyType {
    eNoDuty =           0,
    eEscortDuty =       1,
    eGuardDuty =        2,
    eAssaultDuty =      3,
    eHostileBaseDuty =  4
};

class SpaceObject {
  public:
    static SpaceObject* get(int number);
    static Handle<SpaceObject> none() { return Handle<SpaceObject>(-1); }
    static HandleList<SpaceObject> all() { return HandleList<SpaceObject>(0, kMaxSpaceObject); }

    SpaceObject() = default;
    SpaceObject(
            Handle<BaseObject> type, Random seed, int32_t object_id,
            const coordPointType& initial_location,
            int32_t relative_direction, fixedPointType *relative_velocity,
            Handle<Admiral> new_owner, int16_t spriteIDOverride);

    void change_base_type(Handle<BaseObject> base, int32_t spriteIDOverride, bool relative);
    void set_owner(Handle<Admiral> owner, bool message);
    void set_cloak(bool cloak);
    void alter_occupation(Handle<Admiral> owner, int32_t howMuch, bool message);
    void destroy();
    void free();
    void create_floating_player_body();

    uint32_t                attributes = 0;
    BaseObject*             baseType = nullptr;
    Handle<BaseObject>      base;
    int32_t                 number() const;

    uint32_t                keysDown = 0;

    int32_t                 tinySize = 0;
    RgbColor                tinyColor;

    int32_t                 direction = 0;
    int32_t                 directionGoal = 0;
    Fixed                   turnVelocity = 0;
    Fixed                   turnFraction = 0;

    int32_t                 offlineTime = 0;

    coordPointType          location = {0, 0};
    Point                   collisionGrid;
    Handle<SpaceObject>     nextNearObject;
    Point                   distanceGrid;
    Handle<SpaceObject>     nextFarObject;
    Handle<SpaceObject>     previousObject;
    Handle<SpaceObject>     nextObject;

    int32_t                 runTimeFlags = 0;               // distance from origin to destination
    coordPointType          destinationLocation = {0, 0};   // coords of our destination ( or kNoDestination)
    Handle<SpaceObject>     destObject;                     // target of this object.
    Handle<SpaceObject>     destObjectDest;                 // # of our destination's destination in case it dies
    Handle<Destination>     asDestination;                  // If this object kIsDestination.
    int32_t                 destObjectID = kNoShip;         // ID of our dest object
    int32_t                 destObjectDestID = kNoShip;     // id of our dest's destination

    Fixed                   localFriendStrength = 0;
    Fixed                   localFoeStrength = 0;
    Fixed                   escortStrength = 0;
    Fixed                   remoteFriendStrength = 0;
    Fixed                   remoteFoeStrength = 0;

    Fixed                   bestConsideredTargetValue = 0xffffffff;
    Fixed                   currentTargetValue = 0xffffffff;
    Handle<SpaceObject>     bestConsideredTargetNumber;

    int32_t                 timeFromOrigin = 0;     // time it's been since we left
    fixedPointType          idealLocationCalc = {0, 0};  // calced when we got origin
    coordPointType          originLocation = {0, 0};     // coords of our origin

    fixedPointType          motionFraction = {0, 0};
    fixedPointType          velocity = {0, 0};
    Fixed                   thrust = 0;
    Fixed                   maxVelocity = 0;
    Rect                absoluteBounds;
    Random                  randomSeed;

    struct {
        struct {
            int32_t             thisShape;
            Fixed               frameFraction;
            int32_t             frameDirection;
            Fixed               frameSpeed;
        } animation;
        Handle<Beam>            beam;
    } frame;

    int32_t                 _health = 0;
    int32_t                 _energy = 0;
    int32_t                 _battery = 0;

    int32_t                 health() const { return _health; }
    void                    alter_health(int32_t amount);
    int32_t                 max_health() const { return baseType->health; }

    int32_t                 energy() const { return _energy; }
    void                    alter_energy(int32_t amount);
    int32_t                 max_energy() const { return baseType->energy; }

    int32_t                 battery() const { return _battery; }
    void                    alter_battery(int32_t amount);
    int32_t                 max_battery() const { return 5 * max_energy(); }

    void                    recharge();
    bool                    collect_warp_energy(int32_t amount);
    void                    refund_warp_energy();
    int32_t                 warpEnergyCollected = 0;

    Handle<Admiral>         owner;
    int32_t                 age = -1;
    int32_t                 naturalScale = SCALE_SCALE;
    int32_t                 id = kNoShip;
    int16_t                 rechargeTime = 0;
    int16_t                 active = kObjectAvailable;

    int16_t                 layer = 0;
    Sprite*                 sprite = nullptr;

    uint64_t                distanceFromPlayer = 0;
    uint32_t                closestDistance = kMaximumRelevantDistanceSquared;
    Handle<SpaceObject>     closestObject;
    Handle<SpaceObject>     targetObject;
    int32_t                 targetObjectID = kNoShip;
    int32_t                 targetAngle = 0;
    Handle<SpaceObject>     lastTarget;
    int32_t                 lastTargetDistance = 0;
    int32_t                 longestWeaponRange = 0;
    int32_t                 shortestWeaponRange = 0;
    int32_t                 engageRange = kEngageRange;  // longestWeaponRange or kEngageRange

    kPresenceStateType      presenceState = kNormalPresence;
    union {
        struct {
            int16_t speed;
            int16_t scale;
        } landing;
        struct {
            uint8_t step;
            uint8_t progress;
        } warp_in;
        int32_t warping;
        int32_t warp_out;
    } presence;

    int32_t                 hitState = 0;
    int32_t                 cloakState = 0;
    dutyType                duty = eNoDuty;
    int                     pixResID = -1;

    struct Weapon {
        Handle<BaseObject>      base;
        int32_t                 time = 0;
        int32_t                 ammo = 0;
        int32_t                 position = 0;
        int16_t                 charge = 0;
    };
    Weapon                  pulse;
    Weapon                  beam;
    Weapon                  special;

    int32_t                 periodicTime = 0;

    uint32_t                myPlayerFlag = 0x80000000;
    uint32_t                seenByPlayerFlags = 0xffffffff;
    uint32_t                hostileTowardsFlags = 0;

    uint8_t                 shieldColor = 0;
    uint8_t                 originalColor = 0;
};

void SpaceObjectHandlingInit( void);
void ResetAllSpaceObjects( void);
void RemoveAllSpaceObjects( void);
void CorrectAllBaseObjectColor( void);

Handle<SpaceObject> CreateAnySpaceObject(
        Handle<BaseObject> whichBase, fixedPointType *velocity, coordPointType *location,
        int32_t direction, Handle<Admiral> owner, uint32_t specialAttributes,
        int16_t spriteIDOverride);
int32_t CountObjectsOfBaseType(Handle<BaseObject> whichType, Handle<Admiral> owner);

Handle<BaseObject> mGetBaseObjectFromClassRace(int class_, int race);

sfz::StringSlice get_object_name(Handle<BaseObject> id);
sfz::StringSlice get_object_short_name(Handle<BaseObject> id);

}  // namespace antares

#endif // ANTARES_GAME_SPACE_OBJECT_HPP_
