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

#include "data/field.hpp"

namespace antares {

static int32_t required_int32(path_value x) {
    return required_int(x, {-0x80000000ll, 0x80000000ll});
}
DEFAULT_READER(int32_t, required_int32);

static uint8_t required_uint8(path_value x) { return required_int(x, {0, 0x100}); }
DEFAULT_READER(uint8_t, required_uint8);

static fixedPointType required_fixed_point(path_value x) {
    return required_struct<fixedPointType>(
            x, {{"x", &fixedPointType::h}, {"y", &fixedPointType::v}});
}
DEFAULT_READER(fixedPointType, required_fixed_point);

static uint32_t distance(path_value x) {
    double d = required_double(x);
    d        = floor(pow(d, 2));
    return d;
}

static sfz::optional<BaseObject::Weapon> optional_weapon(path_value x) {
    return optional_struct<BaseObject::Weapon>(
            x,
            {{"base", &BaseObject::Weapon::base}, {"positions", &BaseObject::Weapon::positions}});
}
DEFAULT_READER(sfz::optional<BaseObject::Weapon>, optional_weapon);

static int16_t required_layer(path_value x) { return required_int(x, {1, 4}); }
static int32_t required_scale(path_value x) { return required_fixed(x).val() << 4; }

static sfz::optional<BaseObject::Rotation> optional_rotation_frame(path_value x) {
    using Rotation = BaseObject::Rotation;
    return optional_struct<Rotation>(
            x, {{"sprite", &Rotation::sprite},
                {"layer", {&Rotation::layer, required_layer}},
                {"scale", {&Rotation::scale, required_scale}},
                {"frames", &Rotation::frames}});
}
DEFAULT_READER(sfz::optional<BaseObject::Rotation>, optional_rotation_frame);

static BaseObject::Animation::Direction required_animation_direction(path_value x) {
    using Direction = BaseObject::Animation::Direction;
    return required_enum<Direction>(
            x, {{"0", Direction::NONE},
                {"+", Direction::PLUS},
                {"-", Direction::MINUS},
                {"?", Direction::RANDOM}});
}
DEFAULT_READER(BaseObject::Animation::Direction, required_animation_direction);

static sfz::optional<BaseObject::Animation> optional_animation_frame(path_value x) {
    using Animation = BaseObject::Animation;
    return optional_struct<Animation>(
            x, {{"sprite", &Animation::sprite},
                {"layer", {&Animation::layer, required_layer}},
                {"scale", {&Animation::scale, required_scale}},
                {"frames", &Animation::frames},
                {"direction", &Animation::direction},
                {"speed", &Animation::speed},
                {"first", &Animation::first}});
}
DEFAULT_READER(sfz::optional<BaseObject::Animation>, optional_animation_frame);

static BaseObject::Ray::To required_ray_to(path_value x) {
    return required_enum<BaseObject::Ray::To>(
            x, {{"object", BaseObject::Ray::To::OBJECT}, {"coord", BaseObject::Ray::To::COORD}});
}
DEFAULT_READER(BaseObject::Ray::To, required_ray_to);

static sfz::optional<BaseObject::Ray> optional_ray_frame(path_value x) {
    using Ray = BaseObject::Ray;
    return optional_struct<Ray>(
            x, {
                       {"hue", &Ray::hue},
                       {"to", &Ray::to},
                       {"lightning", &Ray::lightning},
                       {"accuracy", &Ray::accuracy},
                       {"range", &Ray::range},
               });
}
DEFAULT_READER(sfz::optional<BaseObject::Ray>, optional_ray_frame);

static sfz::optional<BaseObject::Bolt> optional_bolt_frame(path_value x) {
    using Bolt = BaseObject::Bolt;
    return optional_struct<Bolt>(x, {{"color", &Bolt::color}});
}
DEFAULT_READER(sfz::optional<BaseObject::Bolt>, optional_bolt_frame);

static BaseObject::Device::Usage optional_usage(path_value x) {
    using Usage = BaseObject::Device::Usage;
    return optional_struct<Usage>(
                   x, {{"attacking", {&Usage::attacking, optional_bool, false}},
                       {"defense", {&Usage::defense, optional_bool, false}},
                       {"transportation", {&Usage::transportation, optional_bool, false}}})
            .value_or(Usage{});
}
DEFAULT_READER(BaseObject::Device::Usage, optional_usage);

static BaseObject::Device::Direction required_device_direction(path_value x) {
    return required_enum<BaseObject::Device::Direction>(
            x, {{"fore", BaseObject::Device::Direction::FORE},
                {"omni", BaseObject::Device::Direction::OMNI}});
}
DEFAULT_READER(BaseObject::Device::Direction, required_device_direction);

static sfz::optional<BaseObject::Device> optional_device_frame(path_value x) {
    using Device = BaseObject::Device;
    return optional_struct<Device>(
            x, {{"usage", &Device::usage},
                {"direction", &Device::direction},
                {"energy_cost", &Device::energyCost},
                {"fire_time", &Device::fireTime},
                {"ammo", &Device::ammo},
                {"range", &Device::range},
                {"inverse_speed", &Device::inverseSpeed},
                {"restock_cost", &Device::restockCost}});
}
DEFAULT_READER(sfz::optional<BaseObject::Device>, optional_device_frame);

static BaseObject::Icon::Shape required_icon_shape(path_value x) {
    using Shape = BaseObject::Icon::Shape;
    return required_enum<Shape>(
            x, {{"square", Shape::SQUARE},
                {"triangle", Shape::TRIANGLE},
                {"diamond", Shape::DIAMOND},
                {"plus", Shape::PLUS}});
}
DEFAULT_READER(BaseObject::Icon::Shape, required_icon_shape);

static sfz::optional<BaseObject::Icon> optional_icon(path_value x) {
    return optional_struct<BaseObject::Icon>(
            x, {{"shape", &BaseObject::Icon::shape}, {"size", &BaseObject::Icon::size}});
}
DEFAULT_READER(sfz::optional<BaseObject::Icon>, optional_icon);

static BaseObject::Targeting required_targeting(path_value x) {
    return required_struct<BaseObject::Targeting>(
            x, {{"base", &BaseObject::Targeting::base},
                {"hide", &BaseObject::Targeting::hide},
                {"radar", &BaseObject::Targeting::radar},
                {"order", &BaseObject::Targeting::order},
                {"select", &BaseObject::Targeting::select},
                {"lock", &BaseObject::Targeting::lock}});
}
DEFAULT_READER(BaseObject::Targeting, required_targeting);

static BaseObject::Loadout optional_loadout(path_value x) {
    return optional_struct<BaseObject::Loadout>(
                   x, {{"pulse", &BaseObject::Loadout::pulse},
                       {"beam", &BaseObject::Loadout::beam},
                       {"special", &BaseObject::Loadout::special}})
            .value_or(BaseObject::Loadout{});
}
DEFAULT_READER(BaseObject::Loadout, optional_loadout);

static BaseObject set_attributes(BaseObject o) {
    if (((o.rotation.has_value() + o.animation.has_value() + o.ray.has_value() +
          o.bolt.has_value() + o.device.has_value()) != 1)) {
        throw std::runtime_error(
                "must have single rotation, animation, ray, bolt, or device block");
    } else if (o.rotation.has_value()) {
        o.attributes |= kShapeFromDirection;
    } else if (o.animation.has_value()) {
        o.attributes |= kIsSelfAnimated;
        if (!o.expire.after.animation) {
            o.attributes |= kAnimationCycle;
        }
    } else if (o.ray.has_value() || o.bolt.has_value()) {
        o.attributes |= kIsVector;
    } else if (o.device.has_value()) {
        if (o.device->direction == BaseObject::Device::Direction::OMNI) {
            o.attributes |= kAutoTarget;
        }
    }

    if (o.target.base) {
        o.attributes |= kIsDestination;
    }
    if (o.target.hide) {
        o.attributes |= kHideEffect;
    }
    if (o.target.radar) {
        o.attributes |= kAppearOnRadar;
    }
    if (o.target.order) {
        o.attributes |= kCanAcceptDestination;
    }
    if (o.target.select) {
        o.attributes |= kCanBeDestination;
    }
    if (o.target.lock) {
        o.attributes |= kStaticDestination;
    }
    if (o.autotarget) {
        o.attributes |= kAutoTarget;
    }

    if (o.turn_rate > Fixed::zero()) {
        o.attributes |= (kCanTurn | kHasDirectionGoal);
    }
    if (o.destroy.neutralize) {
        o.attributes |= kNeutralDeath;
    }
    if (o.destroy.release_energy) {
        o.attributes |= kReleaseEnergyOnDeath;
    }
    if (o.collide.as.subject) {
        o.attributes |= kCanCollide;
    }
    if (o.collide.as.object) {
        o.attributes |= kCanBeHit;
    }
    if (o.collide.solid) {
        o.attributes |= kOccupiesSpace;
    }
    if (o.collide.edge) {
        o.attributes |= kDoesBounce;
    }

    if (o.ai.combat.hated) {
        o.attributes |= kHated;
    }
    if (o.ai.combat.guided) {
        o.attributes |= kIsGuided;
    }
    if (o.ai.combat.engages) {
        o.attributes |= kCanEngage;
    }
    if (o.ai.combat.engaged) {
        o.attributes |= kCanBeEngaged;
    }
    if (o.ai.combat.evades) {
        o.attributes |= kCanEvade;
    }
    if (o.ai.combat.evaded) {
        o.attributes |= kCanBeEvaded;
    }

    if ((o.attributes & kIsDestination) && !o.ai.build.legacy_non_builder) {
        o.attributes |= kCanAcceptBuild;
    }

    if (o.ai.target.prefer.base.has_value()) {
        if (*o.ai.target.prefer.base) {
            o.orderFlags |= kSoftTargetIsBase;
        } else {
            o.orderFlags |= kSoftTargetIsNotBase;
        }
    }
    if (o.ai.target.prefer.local.has_value()) {
        if (*o.ai.target.prefer.local) {
            o.orderFlags |= kSoftTargetIsLocal;
        } else {
            o.orderFlags |= kSoftTargetIsRemote;
        }
    }
    if (o.ai.target.prefer.owner == Owner::DIFFERENT) {
        o.orderFlags |= kSoftTargetIsFoe;
    } else if (o.ai.target.prefer.owner == Owner::SAME) {
        o.orderFlags |= kSoftTargetIsFriend;
    }
    if (!o.ai.target.prefer.tags.empty()) {
        o.orderFlags |= kSoftTargetMatchesTags;
    }

    if (o.ai.target.force.base.has_value()) {
        if (*o.ai.target.force.base) {
            o.orderFlags |= kHardTargetIsBase;
        } else {
            o.orderFlags |= kHardTargetIsNotBase;
        }
    }
    if (o.ai.target.force.local.has_value()) {
        if (*o.ai.target.force.local) {
            o.orderFlags |= kHardTargetIsLocal;
        } else {
            o.orderFlags |= kHardTargetIsRemote;
        }
    }
    if (o.ai.target.force.owner == Owner::DIFFERENT) {
        o.orderFlags |= kHardTargetIsFoe;
    } else if (o.ai.target.force.owner == Owner::SAME) {
        o.orderFlags |= kHardTargetIsFriend;
    }
    if (!o.ai.target.force.tags.empty()) {
        o.orderFlags |= kHardTargetMatchesTags;
    }

    return o;
}

static BaseObject::Destroy optional_destroy(path_value x) {
    return optional_struct<BaseObject::Destroy>(
                   x, {{"die", &BaseObject::Destroy::die},
                       {"neutralize", &BaseObject::Destroy::neutralize},
                       {"release_energy", &BaseObject::Destroy::release_energy},
                       {"action", &BaseObject::Destroy::action}})
            .value_or(BaseObject::Destroy{});
}
DEFAULT_READER(BaseObject::Destroy, optional_destroy);

static BaseObject::Expire::After optional_expire_after(path_value x) {
    return optional_struct<BaseObject::Expire::After>(
                   x, {{"age",
                        {&BaseObject::Expire::After::age, optional_ticks_range,
                         Range<ticks>{ticks(-1), ticks(-1)}}},
                       {"animation", &BaseObject::Expire::After::animation}})
            .value_or(BaseObject::Expire::After{});
}
DEFAULT_READER(BaseObject::Expire::After, optional_expire_after);

static BaseObject::Expire optional_expire(path_value x) {
    return optional_struct<BaseObject::Expire>(
                   x, {{"after", &BaseObject::Expire::after},
                       {"die", &BaseObject::Expire::die},
                       {"action", &BaseObject::Expire::action}})
            .value_or(BaseObject::Expire{});
}
DEFAULT_READER(BaseObject::Expire, optional_expire);

static BaseObject::Create optional_create(path_value x) {
    return optional_struct<BaseObject::Create>(x, {{"action", &BaseObject::Create::action}})
            .value_or(BaseObject::Create{});
}
DEFAULT_READER(BaseObject::Create, optional_create);

static BaseObject::Collide::As optional_collide_as(path_value x) {
    return optional_struct<BaseObject::Collide::As>(
                   x,
                   {
                           {"subject", &BaseObject::Collide::As::subject},
                           {"object", &BaseObject::Collide::As::object},
                   })
            .value_or(BaseObject::Collide::As{});
}
DEFAULT_READER(BaseObject::Collide::As, optional_collide_as);

static BaseObject::Collide optional_collide(path_value x) {
    return optional_struct<BaseObject::Collide>(
                   x, {{"as", &BaseObject::Collide::as},
                       {"damage", &BaseObject::Collide::damage},
                       {"solid", &BaseObject::Collide::solid},
                       {"edge", &BaseObject::Collide::edge},
                       {"action", &BaseObject::Collide::action}})
            .value_or(BaseObject::Collide{});
}
DEFAULT_READER(BaseObject::Collide, optional_collide);

static BaseObject::Activate optional_activate(path_value x) {
    return optional_struct<BaseObject::Activate>(
                   x, {{"period",
                        {&BaseObject::Activate::period, optional_ticks_range,
                         Range<ticks>{ticks(0), ticks(0)}}},
                       {"action", &BaseObject::Activate::action}})
            .value_or(BaseObject::Activate{});
}
DEFAULT_READER(BaseObject::Activate, optional_activate);

static BaseObject::Arrive optional_arrive(path_value x) {
    return optional_struct<BaseObject::Arrive>(
                   x, {{"distance", {&BaseObject::Arrive::distance, distance}},
                       {"action", &BaseObject::Arrive::action}})
            .value_or(BaseObject::Arrive{});
}
DEFAULT_READER(BaseObject::Arrive, optional_arrive);

static BaseObject::AI::Combat::Skill optional_ai_combat_skill(path_value x) {
    using Skill = BaseObject::AI::Combat::Skill;
    return optional_struct<Skill>(x, {{"num", &Skill::num}, {"den", &Skill::den}})
            .value_or(Skill{});
}
DEFAULT_READER(BaseObject::AI::Combat::Skill, optional_ai_combat_skill);

static BaseObject::AI::Combat optional_ai_combat(path_value x) {
    using Combat = BaseObject::AI::Combat;
    return optional_struct<Combat>(
                   x, {{"hated", &Combat::hated},
                       {"guided", &Combat::guided},
                       {"engages", &Combat::engages},
                       {"engages_if", {&Combat::engages_if, optional_tags}},
                       {"engaged", &Combat::engaged},
                       {"engaged_if", {&Combat::engaged_if, optional_tags}},
                       {"evades", &Combat::evades},
                       {"evaded", &Combat::evaded},
                       {"skill", &Combat::skill}})
            .value_or(Combat{});
}
DEFAULT_READER(BaseObject::AI::Combat, optional_ai_combat);

static BaseObject::AI::Target::Filter optional_ai_target_filter(path_value x) {
    using Filter = BaseObject::AI::Target::Filter;
    return optional_struct<Filter>(
                   x, {{"base", &Filter::base},
                       {"local", &Filter::local},
                       {"owner", &Filter::owner},
                       {"tags", {&Filter::tags, optional_tags}}})
            .value_or(Filter{});
}
DEFAULT_READER(BaseObject::AI::Target::Filter, optional_ai_target_filter);

static BaseObject::AI::Target optional_ai_target(path_value x) {
    using Target = BaseObject::AI::Target;
    return optional_struct<Target>(x, {{"prefer", &Target::prefer}, {"force", &Target::force}})
            .value_or(Target{});
}
DEFAULT_READER(BaseObject::AI::Target, optional_ai_target);

static BaseObject::AI::Escort optional_ai_escort(path_value x) {
    using Escort = BaseObject::AI::Escort;
    return optional_struct<Escort>(
                   x, {{"class", &Escort::class_},
                       {"power", &Escort::power},
                       {"need", &Escort::need}})
            .value_or(Escort{});
}
DEFAULT_READER(BaseObject::AI::Escort, optional_ai_escort);

static BaseObject::AI::Build optional_ai_build(path_value x) {
    using Build = BaseObject::AI::Build;
    return optional_struct<Build>(
                   x, {{"ratio", &Build::ratio},
                       {"needs_escort", &Build::needs_escort},
                       {"legacy_non_builder", &Build::legacy_non_builder}})
            .value_or(Build{});
}
DEFAULT_READER(BaseObject::AI::Build, optional_ai_build);

static BaseObject::AI optional_ai(path_value x) {
    return optional_struct<BaseObject::AI>(
                   x, {{"combat", &BaseObject::AI::combat},
                       {"target", &BaseObject::AI::target},
                       {"escort", &BaseObject::AI::escort},
                       {"build", &BaseObject::AI::build}})
            .value_or(BaseObject::AI{});
}
DEFAULT_READER(BaseObject::AI, optional_ai);

BaseObject base_object(pn::value_cref x0) {
    return set_attributes(required_struct<BaseObject>(
            path_value{x0}, {{"long_name", &BaseObject::name},
                             {"short_name", &BaseObject::short_name},

                             {"notes", nullptr},
                             {"class", nullptr},
                             {"race", nullptr},

                             {"portrait", &BaseObject::portrait},

                             {"price", &BaseObject::price},
                             {"warp_out_distance", {&BaseObject::warpOutDistance, distance}},
                             {"health", &BaseObject::health},
                             {"energy", &BaseObject::energy},
                             {"occupy_count", &BaseObject::occupy_count},

                             {"max_velocity", &BaseObject::maxVelocity},
                             {"warp_speed", &BaseObject::warpSpeed},
                             {"mass", &BaseObject::mass},
                             {"turn_rate", &BaseObject::turn_rate},
                             {"thrust", &BaseObject::thrust},

                             {"build_time", &BaseObject::buildTime},

                             {"shield_color", &BaseObject::shieldColor},

                             {"initial_velocity", &BaseObject::initial_velocity},
                             {"initial_direction", &BaseObject::initial_direction},
                             {"autotarget", &BaseObject::autotarget},

                             {"destroy", &BaseObject::destroy},
                             {"expire", &BaseObject::expire},
                             {"create", &BaseObject::create},
                             {"collide", &BaseObject::collide},
                             {"activate", &BaseObject::activate},
                             {"arrive", &BaseObject::arrive},

                             {"target", &BaseObject::target},
                             {"icon", &BaseObject::icon},
                             {"weapons", &BaseObject::weapons},

                             {"rotation", &BaseObject::rotation},
                             {"animation", &BaseObject::animation},
                             {"ray", &BaseObject::ray},
                             {"bolt", &BaseObject::bolt},
                             {"device", &BaseObject::device},

                             {"tags", {&BaseObject::tags, optional_tags}},
                             {"ai", &BaseObject::ai}}));
}

}  // namespace antares
