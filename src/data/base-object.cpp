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
    int32_t pulse_count, beam_count, special_count;
    if (!in.read(&pulse, &beam, &special, &pulse_count, &beam_count, &special_count)) {
        return false;
    }
    object->pulse.base   = Handle<BaseObject>(pulse);
    object->beam.base    = Handle<BaseObject>(beam);
    object->special.base = Handle<BaseObject>(special);
    object->pulse.positions.resize(std::max(1, pulse_count));
    object->beam.positions.resize(std::max(1, beam_count));
    object->special.positions.resize(std::max(1, special_count));

    for (auto weapon : {&object->pulse, &object->beam, &object->special}) {
        for (size_t i = 0; i < kMaxWeaponPosition; ++i) {
            fixedPointType  ignored;
            fixedPointType* p = &ignored;
            if (i < weapon->positions.size()) {
                p = &weapon->positions[i];
            }
            if (!read_from(in, p)) {
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

    int32_t initial_direction, initial_direction_range, icon;
    if (!in.read(
                &object->naturalScale, &object->pixLayer, &object->pixResID, &icon,
                &object->shieldColor, pn::pad(1), &initial_direction, &initial_direction_range)) {
        return false;
    }
    object->initial_direction = {initial_direction,
                                 initial_direction + std::max(0, initial_direction_range)};
    if ((0 < icon) && (icon < 0x50)) {
        object->icon.size = icon & 0x0F;
        switch (icon & 0xF0) {
            case 0x00: object->icon.shape = IconShape::SQUARE; break;
            case 0x10: object->icon.shape = IconShape::TRIANGLE; break;
            case 0x20: object->icon.shape = IconShape::DIAMOND; break;
            case 0x30: object->icon.shape = IconShape::PLUS; break;
            case 0x40: object->icon.shape = IconShape::SQUARE; break;
        }
    } else {
        object->icon = {IconShape::SQUARE, 0};
    }

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
    } else {
        read_from(sub, &object->frame.weapon);
    }
    return true;
}

bool read_from(pn::file_view in, objectFrameType::Rotation* rotation) {
    int32_t turn_rate;
    if (!in.read(&rotation->shapeOffset, &rotation->rotRes, &turn_rate, pn::pad(4))) {
        return false;
    }
    rotation->maxTurnRate = Fixed::from_val(turn_rate);
    return true;
}

bool read_from(pn::file_view in, objectFrameType::Animation* animation) {
    int32_t begin, end, dir, dir_range, speed, first, first_range;
    if (!in.read(&begin, &end, &dir, &dir_range, &speed, pn::pad(4), &first, &first_range)) {
        return false;
    }

    // TODO(sfiera): use Fixed::from_long(1) instead of Fixed::from_val(1). This causes
    // arescentral/antares#163.
    animation->shapes = {Fixed::from_long(begin), Fixed::from_long(end) + Fixed::from_val(1)};
    animation->speed  = Fixed::from_val(speed);
    animation->first  = {Fixed::from_long(first), Fixed::from_long(first + first_range)};

    if ((dir == 0) && (dir_range == 0)) {
        animation->direction = AnimationDirection::NONE;
    } else if ((dir == +1) && (dir_range == 0)) {
        animation->direction = AnimationDirection::PLUS;
    } else if ((dir == -1) && (dir_range == 0)) {
        animation->direction = AnimationDirection::MINUS;
    } else if ((dir == -1) && (dir_range == -1)) {
        animation->direction = AnimationDirection::RANDOM;
    } else {
        animation->direction = AnimationDirection::NONE;
    }

    return true;
}

bool read_from(pn::file_view in, objectFrameType::Vector* vector) {
    uint8_t kind, color;
    if (!in.read(&color, &kind, &vector->accuracy, &vector->range)) {
        return false;
    }
    switch (kind) {
        default: vector->kind = VectorKind::BOLT; break;
        case 1: vector->kind = VectorKind::BEAM_TO_OBJECT; break;
        case 2: vector->kind = VectorKind::BEAM_TO_COORD; break;
        case 3: vector->kind = VectorKind::BEAM_TO_OBJECT_LIGHTNING; break;
        case 4: vector->kind = VectorKind::BEAM_TO_COORD_LIGHTNING; break;
    }
    if (color <= 16) {
        vector->visible = 0;
    } else if (vector->kind == VectorKind::BOLT) {
        vector->visible    = true;
        vector->bolt_color = GetRGBTranslateColor(color);
    } else {
        vector->visible  = true;
        vector->beam_hue = static_cast<Hue>(color >> 4);
    }
    return true;
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
