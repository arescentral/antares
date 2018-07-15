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

static uint8_t required_uint8(path_value x) { return required_int(x, {0, 0x100}); }

static fixedPointType required_fixed_point(path_value x) {
    return required_struct<fixedPointType>(
            x, {
                       {"x", {&fixedPointType::h, required_fixed}},
                       {"y", {&fixedPointType::v, required_fixed}},
               });
}

static uint32_t distance(path_value x) {
    double d = required_double(x);
    d        = floor(pow(d, 2));
    return d;
}

static sfz::optional<BaseObject::Weapon> optional_weapon(path_value x) {
    return optional_struct<BaseObject::Weapon>(
            x, {
                       {"base", {&BaseObject::Weapon::base, required_base}},
                       {"positions",
                        {&BaseObject::Weapon::positions,
                         optional_array<fixedPointType, required_fixed_point>}},
               });
}

static int16_t required_layer(path_value x) { return required_int(x, {1, 4}); }

static int32_t required_scale(path_value x) { return required_fixed(x).val() << 4; }

static sfz::optional<BaseObject::Rotation> optional_rotation_frame(path_value x) {
    using Rotation = BaseObject::Rotation;
    return optional_struct<Rotation>(
            x, {
                       {"sprite", {&Rotation::sprite, required_string_copy}},
                       {"layer", {&Rotation::layer, required_layer}},
                       {"scale", {&Rotation::scale, required_scale}},
                       {"frames", {&Rotation::frames, required_int_range}},
               });
}

static BaseObject::Animation::Direction required_animation_direction(path_value x) {
    using Direction = BaseObject::Animation::Direction;
    return required_enum<Direction>(
            x, {{"0", Direction::NONE},
                {"+", Direction::PLUS},
                {"-", Direction::MINUS},
                {"?", Direction::RANDOM}});
}

static sfz::optional<BaseObject::Animation> optional_animation_frame(path_value x) {
    using Animation = BaseObject::Animation;
    return optional_struct<Animation>(
            x, {
                       {"sprite", {&Animation::sprite, required_string_copy}},
                       {"layer", {&Animation::layer, required_layer}},
                       {"scale", {&Animation::scale, required_scale}},
                       {"frames", {&Animation::frames, required_fixed_range}},
                       {"direction", {&Animation::direction, required_animation_direction}},
                       {"speed", {&Animation::speed, required_fixed}},
                       {"first", {&Animation::first, required_fixed_range}},
               });
}

static BaseObject::Ray::To required_ray_to(path_value x) {
    return required_enum<BaseObject::Ray::To>(
            x, {{"object", BaseObject::Ray::To::OBJECT}, {"coord", BaseObject::Ray::To::COORD}});
}

static sfz::optional<BaseObject::Ray> optional_ray_frame(path_value x) {
    using Ray = BaseObject::Ray;
    return optional_struct<Ray>(
            x, {
                       {"hue", {&Ray::hue, optional_hue}},
                       {"to", {&Ray::to, required_ray_to}},
                       {"lightning", {&Ray::lightning, required_bool}},
                       {"accuracy", {&Ray::accuracy, required_int32}},
                       {"range", {&Ray::range, required_int32}},
               });
}

static sfz::optional<BaseObject::Bolt> optional_bolt_frame(path_value x) {
    using Bolt = BaseObject::Bolt;
    return optional_struct<Bolt>(x, {{"color", {&Bolt::color, required_color}}});
}

static BaseObject::Device::Usage optional_usage(path_value x) {
    using Usage = BaseObject::Device::Usage;
    return optional_struct<Usage>(
                   x, {{"attacking", {&Usage::attacking, optional_bool, false}},
                       {"defense", {&Usage::defense, optional_bool, false}},
                       {"transportation", {&Usage::transportation, optional_bool, false}}})
            .value_or(Usage{});
}

static BaseObject::Device::Direction required_device_direction(path_value x) {
    return required_enum<BaseObject::Device::Direction>(
            x, {{"fore", BaseObject::Device::Direction::FORE},
                {"omni", BaseObject::Device::Direction::OMNI}});
}

static sfz::optional<BaseObject::Device> optional_device_frame(path_value x) {
    using Device = BaseObject::Device;
    return optional_struct<Device>(
            x, {
                       {"usage", {&Device::usage, optional_usage}},
                       {"direction", {&Device::direction, required_device_direction}},
                       {"energy_cost", {&Device::energyCost, required_int32}},
                       {"fire_time", {&Device::fireTime, required_ticks}},
                       {"ammo", {&Device::ammo, required_int32}},
                       {"range", {&Device::range, required_int32}},
                       {"inverse_speed", {&Device::inverseSpeed, required_fixed}},
                       {"restock_cost", {&Device::restockCost, required_int32}},
               });
}

static BaseObject::Icon::Shape required_icon_shape(path_value x) {
    using Shape = BaseObject::Icon::Shape;
    return required_enum<Shape>(
            x, {{"square", Shape::SQUARE},
                {"triangle", Shape::TRIANGLE},
                {"diamond", Shape::DIAMOND},
                {"plus", Shape::PLUS}});
}

static sfz::optional<BaseObject::Icon> optional_icon(path_value x) {
    return optional_struct<BaseObject::Icon>(
            x, {
                       {"shape", {&BaseObject::Icon::shape, required_icon_shape}},
                       {"size", {&BaseObject::Icon::size, required_int}},
               });
}

static BaseObject::Targeting required_targeting(path_value x) {
    return required_struct<BaseObject::Targeting>(
            x, {
                       {"base", {&BaseObject::Targeting::base, required_bool}},
                       {"hide", {&BaseObject::Targeting::hide, required_bool}},
                       {"radar", {&BaseObject::Targeting::radar, required_bool}},
                       {"order", {&BaseObject::Targeting::order, required_bool}},
                       {"select", {&BaseObject::Targeting::select, required_bool}},
                       {"lock", {&BaseObject::Targeting::lock, required_bool}},
               });
}

static BaseObject::Loadout optional_loadout(path_value x) {
    return optional_struct<BaseObject::Loadout>(
                   x,
                   {
                           {"pulse", {&BaseObject::Loadout::pulse, optional_weapon}},
                           {"beam", {&BaseObject::Loadout::beam, optional_weapon}},
                           {"special", {&BaseObject::Loadout::special, optional_weapon}},
                   })
            .value_or(BaseObject::Loadout{});
}

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
                   x,
                   {
                           {"die", {&BaseObject::Destroy::die, required_bool}},
                           {"neutralize", {&BaseObject::Destroy::neutralize, required_bool}},
                           {"release_energy",
                            {&BaseObject::Destroy::release_energy, required_bool}},
                           {"action",
                            {&BaseObject::Destroy::action, optional_array<Action, action>}},
                   })
            .value_or(BaseObject::Destroy{});
}

static BaseObject::Expire::After optional_expire_after(path_value x) {
    return optional_struct<BaseObject::Expire::After>(
                   x,
                   {
                           {"age",
                            {&BaseObject::Expire::After::age, optional_ticks_range,
                             Range<ticks>{ticks(-1), ticks(-1)}}},
                           {"animation", {&BaseObject::Expire::After::animation, required_bool}},
                   })
            .value_or(BaseObject::Expire::After{});
}

static BaseObject::Expire optional_expire(path_value x) {
    return optional_struct<BaseObject::Expire>(
                   x,
                   {
                           {"after", {&BaseObject::Expire::after, optional_expire_after}},
                           {"die", {&BaseObject::Expire::die, required_bool}},
                           {"action",
                            {&BaseObject::Expire::action, optional_array<Action, action>}},
                   })
            .value_or(BaseObject::Expire{});
}

static BaseObject::Create optional_create(path_value x) {
    return optional_struct<BaseObject::Create>(
                   x,
                   {
                           {"action",
                            {&BaseObject::Create::action, optional_array<Action, action>}},
                   })
            .value_or(BaseObject::Create{});
}

static BaseObject::Collide::As optional_collide_as(path_value x) {
    return optional_struct<BaseObject::Collide::As>(
                   x,
                   {
                           {"subject", {&BaseObject::Collide::As::subject, required_bool}},
                           {"object", {&BaseObject::Collide::As::object, required_bool}},
                   })
            .value_or(BaseObject::Collide::As{});
}

static BaseObject::Collide optional_collide(path_value x) {
    return optional_struct<BaseObject::Collide>(
                   x,
                   {
                           {"as", {&BaseObject::Collide::as, optional_collide_as}},
                           {"damage", {&BaseObject::Collide::damage, required_int32}},
                           {"solid", {&BaseObject::Collide::solid, required_bool}},
                           {"edge", {&BaseObject::Collide::edge, required_bool}},
                           {"action",
                            {&BaseObject::Collide::action, optional_array<Action, action>}},
                   })
            .value_or(BaseObject::Collide{});
}

static BaseObject::Activate optional_activate(path_value x) {
    return optional_struct<BaseObject::Activate>(
                   x,
                   {
                           {"period",
                            {&BaseObject::Activate::period, optional_ticks_range,
                             Range<ticks>{ticks(0), ticks(0)}}},
                           {"action",
                            {&BaseObject::Activate::action, optional_array<Action, action>}},
                   })
            .value_or(BaseObject::Activate{});
}

static BaseObject::Arrive optional_arrive(path_value x) {
    return optional_struct<BaseObject::Arrive>(
                   x,
                   {
                           {"distance", {&BaseObject::Arrive::distance, distance}},
                           {"action",
                            {&BaseObject::Arrive::action, optional_array<Action, action>}},
                   })
            .value_or(BaseObject::Arrive{});
}

static BaseObject::AI::Combat::Skill optional_ai_combat_skill(path_value x) {
    using Skill = BaseObject::AI::Combat::Skill;
    return optional_struct<Skill>(
                   x,
                   {
                           {"num", {&Skill::num, required_uint8}},
                           {"den", {&Skill::den, required_uint8}},
                   })
            .value_or(Skill{});
}

static BaseObject::AI::Combat optional_ai_combat(path_value x) {
    using Combat = BaseObject::AI::Combat;
    return optional_struct<Combat>(
                   x,
                   {
                           {"hated", {&Combat::hated, required_bool}},
                           {"guided", {&Combat::guided, required_bool}},
                           {"engages", {&Combat::engages, required_bool}},
                           {"engages_if", {&Combat::engages_if, optional_tags}},
                           {"engaged", {&Combat::engaged, required_bool}},
                           {"engaged_if", {&Combat::engaged_if, optional_tags}},
                           {"evades", {&Combat::evades, required_bool}},
                           {"evaded", {&Combat::evaded, required_bool}},
                           {"skill", {&Combat::skill, optional_ai_combat_skill}},
                   })
            .value_or(Combat{});
}

static BaseObject::AI::Target::Filter optional_ai_target_filter(path_value x) {
    using Filter = BaseObject::AI::Target::Filter;
    return optional_struct<Filter>(
                   x,
                   {
                           {"base", {&Filter::base, optional_bool}},
                           {"local", {&Filter::local, optional_bool}},
                           {"owner", {&Filter::owner, required_owner}},
                           {"tags", {&Filter::tags, optional_tags}},
                   })
            .value_or(Filter{});
}

static BaseObject::AI::Target optional_ai_target(path_value x) {
    using Target = BaseObject::AI::Target;
    return optional_struct<Target>(
                   x,
                   {
                           {"prefer", {&Target::prefer, optional_ai_target_filter}},
                           {"force", {&Target::force, optional_ai_target_filter}},
                   })
            .value_or(Target{});
}

static BaseObject::AI::Escort optional_ai_escort(path_value x) {
    using Escort = BaseObject::AI::Escort;
    return optional_struct<Escort>(
                   x,
                   {
                           {"class", {&Escort::class_, required_int32}},
                           {"power", {&Escort::power, required_fixed}},
                           {"need", {&Escort::need, required_fixed}},
                   })
            .value_or(Escort{});
}

static BaseObject::AI::Build optional_ai_build(path_value x) {
    using Build = BaseObject::AI::Build;
    return optional_struct<Build>(
                   x,
                   {
                           {"ratio", {&Build::ratio, required_fixed}},
                           {"needs_escort", {&Build::needs_escort, required_bool}},
                           {"legacy_non_builder", {&Build::legacy_non_builder, required_bool}},
                   })
            .value_or(Build{});
}

static BaseObject::AI optional_ai(path_value x) {
    return optional_struct<BaseObject::AI>(
                   x,
                   {
                           {"combat", {&BaseObject::AI::combat, optional_ai_combat}},
                           {"target", {&BaseObject::AI::target, optional_ai_target}},
                           {"escort", {&BaseObject::AI::escort, optional_ai_escort}},
                           {"build", {&BaseObject::AI::build, optional_ai_build}},
                   })
            .value_or(BaseObject::AI{});
}

BaseObject base_object(pn::value_cref x0) {
    return set_attributes(required_struct<BaseObject>(
            path_value{x0},
            {
                    {"long_name", {&BaseObject::name, required_string_copy}},
                    {"short_name", {&BaseObject::short_name, required_string_copy}},

                    {"notes", nullptr},
                    {"class", nullptr},
                    {"race", nullptr},

                    {"portrait", {&BaseObject::portrait, optional_string_copy}},

                    {"price", {&BaseObject::price, required_int32}},
                    {"warp_out_distance", {&BaseObject::warpOutDistance, distance}},
                    {"health", {&BaseObject::health, required_int32}},
                    {"energy", {&BaseObject::energy, required_int32}},
                    {"occupy_count", {&BaseObject::occupy_count, required_int32}},

                    {"max_velocity", {&BaseObject::maxVelocity, required_fixed}},
                    {"warp_speed", {&BaseObject::warpSpeed, required_fixed}},
                    {"mass", {&BaseObject::mass, required_fixed}},
                    {"turn_rate", {&BaseObject::turn_rate, required_fixed}},
                    {"thrust", {&BaseObject::thrust, required_fixed}},

                    {"build_time", {&BaseObject::buildTime, required_ticks}},

                    {"shield_color", {&BaseObject::shieldColor, optional_color}},

                    {"initial_velocity", {&BaseObject::initial_velocity, optional_fixed_range}},
                    {"initial_direction", {&BaseObject::initial_direction, required_int_range}},
                    {"autotarget", {&BaseObject::autotarget, required_bool}},

                    {"destroy", {&BaseObject::destroy, optional_destroy}},
                    {"expire", {&BaseObject::expire, optional_expire}},
                    {"create", {&BaseObject::create, optional_create}},
                    {"collide", {&BaseObject::collide, optional_collide}},
                    {"activate", {&BaseObject::activate, optional_activate}},
                    {"arrive", {&BaseObject::arrive, optional_arrive}},

                    {"target", {&BaseObject::target, required_targeting}},
                    {"icon", {&BaseObject::icon, optional_icon}},
                    {"weapons", {&BaseObject::weapons, optional_loadout}},

                    {"rotation", {&BaseObject::rotation, optional_rotation_frame}},
                    {"animation", {&BaseObject::animation, optional_animation_frame}},
                    {"ray", {&BaseObject::ray, optional_ray_frame}},
                    {"bolt", {&BaseObject::bolt, optional_bolt_frame}},
                    {"device", {&BaseObject::device, optional_device_frame}},

                    {"tags", {&BaseObject::tags, optional_tags}},
                    {"ai", {&BaseObject::ai, optional_ai}},
            }));
}

}  // namespace antares
