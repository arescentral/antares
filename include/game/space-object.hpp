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

#ifndef ANTARES_GAME_SPACE_OBJECT_HPP_
#define ANTARES_GAME_SPACE_OBJECT_HPP_

#include "data/base-object.hpp"
#include "game/globals.hpp"
#include "math/units.hpp"

namespace antares {

struct BuildableObject;

const int32_t kMaxSpaceObject = 250;

const ticks kTimeToCheckHome = secs(15);

const int32_t kEnergyPodAmount = 500;  // average (calced) of 500 energy units/pod

const int32_t kWarpAcceleration = 1;  // how fast we warp in & out

const int32_t kNoShip              = -1;
const int32_t kNoDestinationCoord  = 0;
const int32_t kNoDestinationObject = -1;
const int32_t kNoOwner             = -1;

const int16_t kObjectInUse     = 1;
const int16_t kObjectToBeFreed = 2;
const int16_t kObjectAvailable = 0;

const int32_t kHitStateMax      = 128;
const int32_t kCloakOnStateMax  = 254;
const int32_t kCloakOffStateMax = -252;

const int32_t kEngageRange = 1048576;  // range at which to engage closest ship
                                       // about 2 subsectors (512 * 2)^2

extern const NamedHandle<const BaseObject> kWarpInFlare;
extern const NamedHandle<const BaseObject> kWarpOutFlare;
extern const NamedHandle<const BaseObject> kPlayerBody;
extern const NamedHandle<const BaseObject> kEnergyBlob;

enum dutyType {
    eNoDuty          = 0,
    eEscortDuty      = 1,
    eGuardDuty       = 2,
    eAssaultDuty     = 3,
    eHostileBaseDuty = 4
};

// RUNTIME FLAG BITS
enum {
    kHasArrived   = 0x00000001,
    kTargetLocked = 0x00000002,  // if some foe has locked on, you will be visible
    kIsCloaked    = 0x00000004,  // if you are near a naturally shielding object
    kIsHidden     = 0x00000008,  // overrides natural shielding
    kIsTarget     = 0x00000010,  // preserve target lock in case you become invisible
};

enum kPresenceStateType {
    kNormalPresence  = 0,
    kLandingPresence = 1,
    kWarpInPresence  = 3,
    kWarpingPresence = 4,
    kWarpOutPresence = 5
};

class SpaceObject {
  public:
    static SpaceObject* get(int number) {
        if ((0 <= number) && (number < kMaxSpaceObject)) {
            return &g.objects[number];
        }
        return nullptr;
    }

    static Handle<SpaceObject>     none() { return Handle<SpaceObject>(-1); }
    static HandleList<SpaceObject> all() { return HandleList<SpaceObject>(0, kMaxSpaceObject); }

    SpaceObject() = default;
    SpaceObject(
            const BaseObject& type, Random seed, int32_t object_id,
            const coordPointType& initial_location, int32_t relative_direction,
            fixedPointType* relative_velocity, Handle<Admiral> new_owner,
            sfz::optional<pn::string_view> spriteIDOverride);

    void change_base_type(
            const BaseObject& base, sfz::optional<pn::string_view> spriteIDOverride,
            bool relative);
    void set_owner(Handle<Admiral> owner, bool message);
    void set_cloak(bool cloak);
    void alter_occupation(Handle<Admiral> owner, int32_t howMuch, bool message);
    void destroy();
    void free();
    void create_floating_player_body();

    pn::string_view long_name() const;
    pn::string_view short_name() const;
    bool            engages(const SpaceObject& b) const;
    Fixed           turn_rate() const;

    uint32_t          attributes = 0;
    const BaseObject* base       = nullptr;
    int32_t           number() const;

    uint32_t keysDown = 0;

    sfz::optional<BaseObject::Icon> icon;

    int32_t direction     = 0;
    int32_t directionGoal = 0;
    Fixed   turnVelocity  = Fixed::zero();
    Fixed   turnFraction  = Fixed::zero();

    int32_t offlineTime = 0;

    coordPointType      location = {0, 0};
    Point               collisionGrid;
    Handle<SpaceObject> nextNearObject;
    Point               distanceGrid;
    Handle<SpaceObject> nextFarObject;
    Handle<SpaceObject> previousObject;
    Handle<SpaceObject> nextObject;

    int32_t        runTimeFlags        = 0;       // distance from origin to destination
    coordPointType destinationLocation = {0, 0};  // coords of our destination ( or kNoDestination)
    Handle<SpaceObject> destObject;               // target of this object.
    Handle<SpaceObject> destObjectDest;  // # of our destination's destination in case it dies
    Handle<Destination> asDestination;   // If this object kIsDestination.
    int32_t             destObjectID     = kNoShip;  // ID of our dest object
    int32_t             destObjectDestID = kNoShip;  // id of our dest's destination

    Fixed localFriendStrength  = Fixed::zero();
    Fixed localFoeStrength     = Fixed::zero();
    Fixed escortStrength       = Fixed::zero();
    Fixed remoteFriendStrength = Fixed::zero();
    Fixed remoteFoeStrength    = Fixed::zero();

    Fixed               bestConsideredTargetValue = kFixedNone;
    Fixed               currentTargetValue        = kFixedNone;
    Handle<SpaceObject> bestConsideredTargetNumber;

    ticks          timeFromOrigin    = ticks(0);  // time it's been since we left
    fixedPointType idealLocationCalc = {Fixed::zero(),
                                        Fixed::zero()};  // calced when we got origin
    coordPointType originLocation    = {0, 0};           // coords of our origin

    fixedPointType motionFraction = {Fixed::zero(), Fixed::zero()};
    fixedPointType velocity       = {Fixed::zero(), Fixed::zero()};
    Fixed          thrust         = Fixed::zero();
    Fixed          maxVelocity    = Fixed::zero();
    Rect           absoluteBounds;
    Random         randomSeed;

    struct {
        struct {
            using Direction = BaseObject::Animation::Direction;
            Fixed     thisShape;
            Fixed     frameFraction;
            Direction direction;
            Fixed     speed;
        } animation;
        Handle<Vector> vector;
    } frame;

    int32_t _health  = 0;
    int32_t _energy  = 0;
    int32_t _battery = 0;

    int32_t health() const { return _health; }
    void    alter_health(int32_t amount);
    int32_t max_health() const { return base->health; }

    int32_t energy() const { return _energy; }
    void    alter_energy(int32_t amount);
    int32_t max_energy() const { return base->energy; }

    int32_t battery() const { return _battery; }
    void    alter_battery(int32_t amount);
    int32_t max_battery() const { return 5 * max_energy(); }

    void    recharge();
    bool    collect_warp_energy(int32_t amount);
    void    refund_warp_energy();
    int32_t warpEnergyCollected = 0;

    Handle<Admiral> owner;
    bool            expires      = false;
    ticks           expire_after = ticks(-1);
    int32_t         naturalScale = SCALE_SCALE;
    int32_t         id           = kNoShip;
    ticks           rechargeTime = ticks(0);
    int16_t         active       = kObjectAvailable;

    BaseObject::Layer layer = BaseObject::Layer::NONE;
    Handle<Sprite>    sprite;

    uint64_t            distanceFromPlayer = 0;
    uint32_t            closestDistance    = kMaximumRelevantDistanceSquared;
    Handle<SpaceObject> closestObject;
    Handle<SpaceObject> targetObject;
    int32_t             targetObjectID = kNoShip;
    int32_t             targetAngle    = 0;
    Handle<SpaceObject> lastTarget;
    int32_t             lastTargetDistance  = 0;
    int32_t             longestWeaponRange  = 0;
    int32_t             shortestWeaponRange = 0;
    int32_t             engageRange         = kEngageRange;  // longestWeaponRange or kEngageRange

    kPresenceStateType presenceState = kNormalPresence;
    union {
        struct {
            int16_t speed;
            int16_t scale;
        } landing;
        struct {
            uint8_t step;
            ticks   progress;
        } warp_in;
        Fixed warping;
        Fixed warp_out;
    } presence;

    int32_t  hitState   = 0;
    int32_t  cloakState = 0;
    dutyType duty       = eNoDuty;

    struct PixID {
        pn::string_view name;
        Hue             hue = Hue::GRAY;
    };
    sfz::optional<PixID> pix_id;

    struct Weapon {
        const BaseObject* base;
        game_ticks        time;
        int32_t           ammo     = 0;
        int32_t           position = 0;
        int16_t           charge   = 0;
    };
    Weapon pulse;
    Weapon beam;
    Weapon special;

    ticks periodicTime = ticks(0);

    uint32_t myPlayerFlag        = 0x80000000;
    uint32_t seenByPlayerFlags   = 0xffffffff;
    uint32_t hostileTowardsFlags = 0;

    sfz::optional<RgbColor> shieldColor;
    uint8_t                 originalColor = 0;
};

void SpaceObjectHandlingInit(void);
void ResetAllSpaceObjects(void);
void RemoveAllSpaceObjects(void);

Handle<SpaceObject> CreateAnySpaceObject(
        const BaseObject& whichBase, fixedPointType* velocity, coordPointType* location,
        int32_t direction, Handle<Admiral> owner, uint32_t specialAttributes,
        sfz::optional<pn::string_view> spriteIDOverride);
int32_t CountObjectsOfBaseType(const BaseObject* whichType, Handle<Admiral> owner);

NamedHandle<const BaseObject> get_buildable_object_handle(
        const BuildableObject& o, const NamedHandle<const Race>& race);
const BaseObject* get_buildable_object(
        const BuildableObject& o, const NamedHandle<const Race>& race);

bool tags_match(const BaseObject& o, const Tags& query);

sfz::optional<pn::string_view> sprite_resource(const BaseObject& o);
BaseObject::Layer              sprite_layer(const BaseObject& o);
int32_t                        sprite_scale(const BaseObject& o);
int32_t                        rotation_resolution(const BaseObject& o);

}  // namespace antares

#endif  // ANTARES_GAME_SPACE_OBJECT_HPP_
