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

FIELD_READER(fixedPointType) {
    return required_struct<fixedPointType>(
            x, {{"x", &fixedPointType::h}, {"y", &fixedPointType::v}});
}

FIELD_READER(sfz::optional<BaseObject::Weapon>) {
    return optional_struct<BaseObject::Weapon>(
            x,
            {{"base", &BaseObject::Weapon::base}, {"positions", &BaseObject::Weapon::positions}});
}

FIELD_READER(BaseObject::Layer) {
    return static_cast<BaseObject::Layer>(int_field_within(x, {1, 4}));
}

FIELD_READER(BaseObject::Scale) { return BaseObject::Scale{read_field<Fixed>(x).val() << 4}; }

FIELD_READER(sfz::optional<BaseObject::Rotation>) {
    using Rotation = BaseObject::Rotation;
    return optional_struct<Rotation>(
            x, {{"sprite", &Rotation::sprite},
                {"layer", &Rotation::layer},
                {"scale", &Rotation::scale},
                {"frames", &Rotation::frames}});
}

FIELD_READER(BaseObject::Animation::Direction) {
    using Direction = BaseObject::Animation::Direction;
    return required_enum<Direction>(
            x, {{"+", Direction::PLUS}, {"-", Direction::MINUS}, {"?", Direction::RANDOM}});
}

FIELD_READER(sfz::optional<BaseObject::Animation>) {
    using Animation = BaseObject::Animation;
    return optional_struct<Animation>(
            x, {{"sprite", &Animation::sprite},
                {"layer", &Animation::layer},
                {"scale", &Animation::scale},
                {"frames", &Animation::frames},
                {"direction", &Animation::direction},
                {"speed", &Animation::speed},
                {"first", &Animation::first}});
}

FIELD_READER(BaseObject::Ray::To) {
    return required_enum<BaseObject::Ray::To>(
            x, {{"object", BaseObject::Ray::To::OBJECT}, {"coord", BaseObject::Ray::To::COORD}});
}

FIELD_READER(sfz::optional<BaseObject::Ray>) {
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

FIELD_READER(sfz::optional<BaseObject::Bolt>) {
    using Bolt = BaseObject::Bolt;
    return optional_struct<Bolt>(x, {{"color", &Bolt::color}});
}

FIELD_READER(BaseObject::Device::Usage) {
    using Usage = BaseObject::Device::Usage;
    return optional_struct<Usage>(
                   x, {{"attacking", &Usage::attacking},
                       {"defense", &Usage::defense},
                       {"transportation", &Usage::transportation}})
            .value_or(Usage{});
}

FIELD_READER(BaseObject::Device::Direction) {
    return required_enum<BaseObject::Device::Direction>(
            x, {{"fore", BaseObject::Device::Direction::FORE},
                {"omni", BaseObject::Device::Direction::OMNI}});
}

FIELD_READER(InvertableSpeed) {
    auto    speed   = read_field<double>(x);
    double  inverse = 256 / speed;
    int32_t rounded = round(inverse);
    return InvertableSpeed{Fixed::from_val(rounded)};
}

FIELD_READER(sfz::optional<BaseObject::Device>) {
    using Device = BaseObject::Device;
    return optional_struct<Device>(
            x, {{"usage", &Device::usage},
                {"direction", &Device::direction},
                {"energy_cost", &Device::energyCost},
                {"fire_time", &Device::fireTime},
                {"ammo", &Device::ammo},
                {"range", &Device::range},
                {"speed", &Device::speed},
                {"restock_cost", &Device::restockCost}});
}

FIELD_READER(BaseObject::Icon::Shape) {
    using Shape = BaseObject::Icon::Shape;
    return required_enum<Shape>(
            x, {{"square", Shape::SQUARE},
                {"triangle", Shape::TRIANGLE},
                {"diamond", Shape::DIAMOND},
                {"plus", Shape::PLUS}});
}

FIELD_READER(sfz::optional<BaseObject::Icon>) {
    return optional_struct<BaseObject::Icon>(
            x, {{"shape", &BaseObject::Icon::shape}, {"size", &BaseObject::Icon::size}});
}

FIELD_READER(BaseObject::Targeting) {
    return required_struct<BaseObject::Targeting>(
            x, {{"base", &BaseObject::Targeting::base},
                {"hide", &BaseObject::Targeting::hide},
                {"radar", &BaseObject::Targeting::radar},
                {"order", &BaseObject::Targeting::order},
                {"select", &BaseObject::Targeting::select},
                {"lock", &BaseObject::Targeting::lock}});
}

FIELD_READER(BaseObject::Loadout) {
    return optional_struct<BaseObject::Loadout>(
                   x, {{"pulse", &BaseObject::Loadout::pulse},
                       {"beam", &BaseObject::Loadout::beam},
                       {"special", &BaseObject::Loadout::special}})
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
    if (o.collide.as.direct) {
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
    if (o.ai.combat.engages.unconditional.value_or(true)) {
        o.attributes |= kCanEngage;
    }
    if (o.ai.combat.engaged.unconditional.value_or(true)) {
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
    if (!o.ai.target.prefer.tags.tags.empty()) {
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
    if (!o.ai.target.force.tags.tags.empty()) {
        o.orderFlags |= kHardTargetMatchesTags;
    }

    return o;
}

FIELD_READER(BaseObject::Destroy) {
    return optional_struct<BaseObject::Destroy>(
                   x, {{"die", &BaseObject::Destroy::die},
                       {"neutralize", &BaseObject::Destroy::neutralize},
                       {"release_energy", &BaseObject::Destroy::release_energy},
                       {"action", &BaseObject::Destroy::action}})
            .value_or(BaseObject::Destroy{});
}

FIELD_READER(BaseObject::Expire::After) {
    return optional_struct<BaseObject::Expire::After>(
                   x, {{"age", &BaseObject::Expire::After::age},
                       {"animation", &BaseObject::Expire::After::animation}})
            .value_or(BaseObject::Expire::After{});
}

FIELD_READER(BaseObject::Expire) {
    return optional_struct<BaseObject::Expire>(
                   x, {{"after", &BaseObject::Expire::after},
                       {"die", &BaseObject::Expire::die},
                       {"action", &BaseObject::Expire::action}})
            .value_or(BaseObject::Expire{});
}

FIELD_READER(BaseObject::Create) {
    return optional_struct<BaseObject::Create>(x, {{"action", &BaseObject::Create::action}})
            .value_or(BaseObject::Create{});
}

FIELD_READER(BaseObject::Collide::As) {
    return optional_struct<BaseObject::Collide::As>(
                   x,
                   {
                           {"subject", &BaseObject::Collide::As::subject},
                           {"direct", &BaseObject::Collide::As::direct},
                   })
            .value_or(BaseObject::Collide::As{});
}

FIELD_READER(BaseObject::Collide) {
    return optional_struct<BaseObject::Collide>(
                   x, {{"as", &BaseObject::Collide::as},
                       {"damage", &BaseObject::Collide::damage},
                       {"solid", &BaseObject::Collide::solid},
                       {"edge", &BaseObject::Collide::edge},
                       {"action", &BaseObject::Collide::action}})
            .value_or(BaseObject::Collide{});
}

FIELD_READER(BaseObject::Activate) {
    return optional_struct<BaseObject::Activate>(
                   x, {{"period", &BaseObject::Activate::period},
                       {"action", &BaseObject::Activate::action}})
            .value_or(BaseObject::Activate{});
}

FIELD_READER(BaseObject::Arrive) {
    return optional_struct<BaseObject::Arrive>(
                   x, {{"distance", &BaseObject::Arrive::distance},
                       {"action", &BaseObject::Arrive::action}})
            .value_or(BaseObject::Arrive{});
}

FIELD_READER(BaseObject::AI::Combat::Skill) {
    using Skill = BaseObject::AI::Combat::Skill;
    return optional_struct<Skill>(x, {{"num", &Skill::num}, {"den", &Skill::den}})
            .value_or(Skill{});
}

FIELD_READER(BaseObject::AI::Combat::Engage::If) {
    using If = BaseObject::AI::Combat::Engage::If;
    return required_struct<If>(x, {{"tags", &If::tags}});
}

FIELD_READER(BaseObject::AI::Combat::Engage) {
    using Engage = BaseObject::AI::Combat::Engage;
    if (x.value().is_bool()) {
        Engage e;
        e.unconditional = sfz::make_optional(x.value().as_bool());
        return e;
    } else if (x.value().is_map()) {
        return required_struct<Engage>(x, {{"if", &Engage::if_}});
    } else {
        throw std::runtime_error(pn::format("{0}must be bool or map", x.prefix()).c_str());
    }
}

FIELD_READER(BaseObject::AI::Combat) {
    using Combat = BaseObject::AI::Combat;
    return optional_struct<Combat>(
                   x, {{"hated", &Combat::hated},
                       {"guided", &Combat::guided},
                       {"engages", &Combat::engages},
                       {"engaged", &Combat::engaged},
                       {"evades", &Combat::evades},
                       {"evaded", &Combat::evaded},
                       {"skill", &Combat::skill}})
            .value_or(Combat{});
}

FIELD_READER(BaseObject::AI::Target::Filter) {
    using Filter = BaseObject::AI::Target::Filter;
    return optional_struct<Filter>(
                   x, {{"base", &Filter::base},
                       {"local", &Filter::local},
                       {"owner", &Filter::owner},
                       {"tags", &Filter::tags}})
            .value_or(Filter{});
}

FIELD_READER(BaseObject::AI::Target) {
    using Target = BaseObject::AI::Target;
    return optional_struct<Target>(x, {{"prefer", &Target::prefer}, {"force", &Target::force}})
            .value_or(Target{});
}

FIELD_READER(BaseObject::AI::Escort) {
    using Escort = BaseObject::AI::Escort;
    return optional_struct<Escort>(
                   x, {{"class", &Escort::class_},
                       {"power", &Escort::power},
                       {"need", &Escort::need}})
            .value_or(Escort{});
}

FIELD_READER(BaseObject::AI::Build) {
    using Build = BaseObject::AI::Build;
    return optional_struct<Build>(
                   x, {{"ratio", &Build::ratio},
                       {"needs_escort", &Build::needs_escort},
                       {"legacy_non_builder", &Build::legacy_non_builder}})
            .value_or(Build{});
}

FIELD_READER(BaseObject::AI) {
    return optional_struct<BaseObject::AI>(
                   x, {{"combat", &BaseObject::AI::combat},
                       {"target", &BaseObject::AI::target},
                       {"escort", &BaseObject::AI::escort},
                       {"build", &BaseObject::AI::build}})
            .value_or(BaseObject::AI{});
}

BaseObject base_object(pn::value_cref x0) {
    return set_attributes(required_struct<BaseObject>(
            path_value{x0}, {{"long_name", &BaseObject::long_name},
                             {"short_name", &BaseObject::short_name},
                             {"tags", &BaseObject::tags},

                             {"notes", nullptr},
                             {"class", nullptr},
                             {"race", nullptr},

                             {"portrait", &BaseObject::portrait},

                             {"price", &BaseObject::price},
                             {"build_time", &BaseObject::buildTime},
                             {"health", &BaseObject::health},
                             {"energy", &BaseObject::energy},
                             {"shield_color", &BaseObject::shieldColor},
                             {"occupy_count", &BaseObject::occupy_count},

                             {"mass", &BaseObject::mass},
                             {"max_velocity", &BaseObject::maxVelocity},
                             {"thrust", &BaseObject::thrust},
                             {"warp_speed", &BaseObject::warpSpeed},
                             {"warp_out_distance", &BaseObject::warpOutDistance},
                             {"turn_rate", &BaseObject::turn_rate},

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

                             {"ai", &BaseObject::ai}}));
}

}  // namespace antares
