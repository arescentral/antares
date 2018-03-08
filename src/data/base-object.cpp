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

// Space Object Handling >> MUST BE INITED _AFTER_ SCENARIOMAKER << (uses Ares Scenarios file)

#include "data/base-object.hpp"

#include "data/level.hpp"

namespace antares {

static bool read_weapons(pn::file_view in, BaseObject* object) {
    int32_t pulse, beam, special;
    if (!in.read(
                &pulse, &beam, &special, &object->pulse.positionNum, &object->beam.positionNum,
                &object->special.positionNum)) {
        return false;
    }
    object->pulse.base   = Handle<BaseObject>(pulse);
    object->beam.base    = Handle<BaseObject>(beam);
    object->special.base = Handle<BaseObject>(special);

    for (auto weapon : {&object->pulse, &object->beam, &object->special}) {
        for (size_t i = 0; i < kMaxWeaponPosition; ++i) {
            if (!read_from(in, &weapon->position[i])) {
                return false;
            }
        }
    }
    return true;
}

bool read_from(pn::file_view in, BaseObject* object) {
    uint8_t section[32];

    if (!in.read(&object->attributes)) {
        return false;
    }
    if (object->attributes & kIsSelfAnimated) {
        object->attributes &= ~(kShapeFromDirection | kIsVector);
    } else if (object->attributes & kShapeFromDirection) {
        object->attributes &= ~kIsVector;
    }

    int32_t initial_age, age_range, initial_velocity, initial_velocity_range;
    if (!(in.read(&object->baseClass, pn::pad(4), &object->price) &&
          read_from(in, &object->offenseValue) && in.read(&object->destinationClass) &&
          read_from(in, &object->maxVelocity) && read_from(in, &object->warpSpeed) &&
          in.read(&object->warpOutDistance) &&
          in.read(&initial_velocity, &initial_velocity_range) && read_from(in, &object->mass) &&
          read_from(in, &object->maxThrust) &&
          in.read(&object->health, &object->damage, &object->energy, &initial_age, &age_range))) {
        return false;
    }
    object->initial_velocity = {
            Fixed::from_val(initial_velocity),
            Fixed::from_val(initial_velocity + std::max(0, initial_velocity_range))};
    if (initial_age >= 0) {
        object->initial_age =
                Range<ticks>{ticks(initial_age), ticks(initial_age + std::max(0, age_range))};
    } else {
        object->initial_age = {ticks(-1), ticks(-1)};
    }

    if (object->attributes & kNeutralDeath) {
        object->occupy_count = age_range;
    } else {
        object->occupy_count = -1;
    }

    int32_t initial_direction, initial_direction_range;
    if (!in.read(
                &object->naturalScale, &object->pixLayer, &object->pixResID, &object->tinySize,
                &object->shieldColor, pn::pad(1), &initial_direction, &initial_direction_range)) {
        return false;
    }
    object->initial_direction = {initial_direction,
                                 initial_direction + std::max(0, initial_direction_range)};

    if ((object->shieldColor != 0xFF) && (object->shieldColor != 0)) {
        object->shieldColor = GetTranslateColorShade(static_cast<Hue>(object->shieldColor), 15);
    }

    if (!(read_weapons(in, object) && read_from(in, &object->friendDefecit) &&
          read_from(in, &object->dangerThreshold) &&
          in.read(&object->specialDirection, &object->arriveActionDistance) &&
          in.read(pn::pad(48)) && (fread(section, 1, 32, in.c_obj()) == 32) &&
          in.read(&object->buildFlags) && in.read(&object->orderFlags))) {
        return false;
    }

    static const char hex[]      = "0123456789abcdef";
    int               level_tag  = (object->buildFlags & kLevelKeyTag) >> kLevelKeyTagShift;
    int               engage_tag = (object->buildFlags & kEngageKeyTag) >> kEngageKeyTagShift;
    int               order_tag  = (object->orderFlags & kOrderKeyTag) >> kOrderKeyTagShift;
    object->levelKeyTag          = level_tag ? pn::rune(hex[level_tag]).copy() : "";
    object->engageKeyTag         = engage_tag ? pn::rune(hex[engage_tag]).copy() : "";
    object->orderKeyTag          = order_tag ? pn::rune(hex[order_tag]).copy() : "";

    uint32_t build_time;
    if (!(read_from(in, &object->buildRatio) &&
          in.read(&build_time, &object->skillNum, &object->skillDen, &object->skillNumAdj,
                  &object->skillDenAdj, &object->pictPortraitResID, pn::pad(10)))) {
        return false;
    }
    object->buildTime = 3 * ticks(build_time / 10);

    pn::file sub = pn::data_view{section, 32}.open();
    if (object->attributes & kShapeFromDirection) {
        read_from(sub, &object->frame.rotation);
    } else if (object->attributes & kIsSelfAnimated) {
        read_from(sub, &object->frame.animation);
    } else if (object->attributes & kIsVector) {
        read_from(sub, &object->frame.vector);
        if (object->frame.vector.color > 16) {
            object->frame.vector.color = object->frame.vector.color;
        } else {
            object->frame.vector.color = 0;
        }
    } else {
        read_from(sub, &object->frame.weapon);
    }
    return true;
}

bool read_from(pn::file_view in, objectFrameType::Rotation* rotation) {
    return in.read(&rotation->shapeOffset, &rotation->rotRes) &&
           read_from(in, &rotation->maxTurnRate) && read_from(in, &rotation->turnAcceleration);
}

bool read_from(pn::file_view in, objectFrameType::Animation* animation) {
    int32_t first_shape, last_shape, direction, direction_range, frame_shape, frame_shape_range;
    if (!(in.read(&first_shape, &last_shape, &direction, &direction_range) &&
          read_from(in, &animation->frameSpeed) && read_from(in, &animation->frameSpeedRange) &&
          in.read(&frame_shape, &frame_shape_range))) {
        return false;
    }
    animation->firstShape      = Fixed::from_long(first_shape);
    animation->lastShape       = Fixed::from_long(last_shape);
    animation->frameShape      = Fixed::from_long(frame_shape);
    animation->frameShapeRange = Fixed::from_long(frame_shape_range);
    if ((direction == 0) && (direction_range == 0)) {
        animation->direction = AnimationDirection::NONE;
    } else if ((direction == +1) && (direction_range == 0)) {
        animation->direction = AnimationDirection::PLUS;
    } else if ((direction == -1) && (direction_range == 0)) {
        animation->direction = AnimationDirection::MINUS;
    } else if ((direction == -1) && (direction_range == -1)) {
        animation->direction = AnimationDirection::RANDOM;
    } else {
        animation->direction = AnimationDirection::NONE;
    }
    return true;
}

bool read_from(pn::file_view in, objectFrameType::Vector* vector) {
    return in.read(&vector->color, &vector->kind, &vector->accuracy, &vector->range);
}

bool read_from(pn::file_view in, objectFrameType::Weapon* weapon) {
    int32_t fire_time;
    if (!(in.read(&weapon->usage, &weapon->energyCost, &fire_time, &weapon->ammo,
                  &weapon->range) &&
          read_from(in, &weapon->inverseSpeed) && in.read(&weapon->restockCost))) {
        return false;
    }
    weapon->fireTime = ticks(fire_time);
    return true;
}

BaseObject base_object(pn::value_cref x0) {
    if (!x0.is_map()) {
        throw std::runtime_error("must be map");
    }

    path_value x{x0};
    BaseObject o;
    if (!read_from(x.get("bin").value().as_data().open(), &o)) {
        throw std::runtime_error("read failure");
    }

    o.name       = required_string(x.get("long_name")).copy();
    o.short_name = required_string(x.get("short_name")).copy();

    o.race = optional_race(x.get("race")).value_or(Race::none());

    o.destroy = optional_action_array(x.get("on_destroy"))
                        .value_or(std::vector<std::unique_ptr<const Action>>{});
    o.expire = optional_action_array(x.get("on_expire"))
                       .value_or(std::vector<std::unique_ptr<const Action>>{});
    o.create = optional_action_array(x.get("on_create"))
                       .value_or(std::vector<std::unique_ptr<const Action>>{});
    o.collide = optional_action_array(x.get("on_collide"))
                        .value_or(std::vector<std::unique_ptr<const Action>>{});
    o.activate = optional_action_array(x.get("on_activate"))
                         .value_or(std::vector<std::unique_ptr<const Action>>{});
    o.arrive = optional_action_array(x.get("on_arrive"))
                       .value_or(std::vector<std::unique_ptr<const Action>>{});

    o.destroyDontDie  = optional_bool(x.get("destroy_dont_die")).value_or(false);
    o.expireDontDie   = optional_bool(x.get("expire_dont_die")).value_or(false);
    o.activate_period = optional_ticks_range(x.get("activate_period"))
                                .value_or(Range<ticks>{ticks(0), ticks(0)});

    return o;
}

}  // namespace antares
