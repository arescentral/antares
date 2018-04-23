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

namespace antares {

static int32_t required_int32(path_value x) {
    return required_int(x, {-0x80000000ll, 0x80000000ll});
}

static sfz::optional<uint8_t> optional_uint8(path_value x) {
    sfz::optional<int64_t> i = optional_int(x, {0, 0x100});
    if (i.has_value()) {
        return sfz::make_optional<uint8_t>(*i);
    }
    return sfz::nullopt;
}

static sfz::optional<int32_t> optional_int32(path_value x) {
    sfz::optional<int64_t> i = optional_int(x, {-0x80000000ll, 0x80000000ll});
    if (i.has_value()) {
        return sfz::make_optional<int32_t>(*i);
    }
    return sfz::nullopt;
}

static sfz::optional<uint32_t> optional_uint32(path_value x) {
    sfz::optional<int64_t> i = optional_int(x, {0, 0x100000000ll});
    if (i.has_value()) {
        return sfz::make_optional<uint32_t>(*i);
    }
    return sfz::nullopt;
}

uint32_t optional_object_order_flags(path_value x) {
    if (x.value().is_null()) {
        return 0;
    } else if (x.value().is_map()) {
        static const pn::string_view flags[32] = {"stronger_than_target",
                                                  "base",
                                                  "not_base",
                                                  "local",
                                                  "remote",
                                                  "only_escort_not_base",
                                                  "friend",
                                                  "foe",

                                                  "bit09",
                                                  "bit10",
                                                  "bit11",
                                                  "bit12",
                                                  "bit13",
                                                  "bit14",
                                                  "bit15",
                                                  "bit16",

                                                  "bit17",
                                                  "bit18",
                                                  "hard_matching_friend",
                                                  "hard_matching_foe",
                                                  "hard_friendly_escort_only",
                                                  "hard_no_friendly_escort",
                                                  "hard_remote",
                                                  "hard_local",

                                                  "hard_foe",
                                                  "hard_friend",
                                                  "hard_not_base",
                                                  "hard_base",

                                                  "matching_foe"};

        uint32_t bit    = 0x00000001;
        uint32_t result = 0x00000000;
        for (pn::string_view flag : flags) {
            if (optional_bool(x.get(flag)).value_or(false)) {
                result |= bit;
            }
            bit <<= 1;
        }
        return result;
    } else {
        throw std::runtime_error(pn::format("{0}: must be null or map", x.path()).c_str());
    }
}

uint32_t optional_object_build_flags(path_value x) {
    if (x.value().is_null()) {
        return 0;
    } else if (x.value().is_map()) {
        static const pn::string_view flags[32] = {"uncaptured_base_exists",
                                                  "sufficient_escorts_exist",
                                                  "this_base_needs_protection",
                                                  "friend_up_trend",
                                                  "friend_down_trend",
                                                  "foe_up_trend",
                                                  "foe_down_trend",
                                                  "matching_foe_exists",

                                                  "bit09",
                                                  "bit10",
                                                  "bit11",
                                                  "bit12",
                                                  "bit13",
                                                  "bit14",
                                                  "bit15",
                                                  "bit16",

                                                  "bit17",
                                                  "bit18",
                                                  "bit19",
                                                  "bit20",
                                                  "bit21",
                                                  "bit22",
                                                  "only_engaged_by",
                                                  "can_only_engage"};

        uint32_t bit    = 0x00000001;
        uint32_t result = 0x00000000;
        for (pn::string_view flag : flags) {
            if (optional_bool(x.get(flag)).value_or(false)) {
                result |= bit;
            }
            bit <<= 1;
        }
        return result;
    } else {
        throw std::runtime_error(pn::format("{0}: must be null or map", x.path()).c_str());
    }
}

fixedPointType required_fixed_point(path_value x) {
    return required_struct<fixedPointType>(
            x, {
                       {"x", {&fixedPointType::h, required_fixed}},
                       {"y", {&fixedPointType::v, required_fixed}},
               });
}

template <typename T, T (*F)(path_value x)>
std::vector<T> optional_array(path_value x) {
    if (x.value().is_null()) {
        return {};
    } else if (x.value().is_array()) {
        pn::array_cref a = x.value().as_array();
        std::vector<T> result;
        for (int i = 0; i < a.size(); ++i) {
            result.emplace_back(F(x.get(i)));
        }
        return result;
    } else {
        throw std::runtime_error(pn::format("{0}: must be null or array", x.path()).c_str());
    }
}

static std::map<pn::string, bool> optional_tags(path_value x) {
    if (x.value().is_null()) {
        return {};
    } else if (x.value().is_map()) {
        pn::map_cref               m = x.value().as_map();
        std::map<pn::string, bool> result;
        for (const auto& kv : m) {
            auto v = optional_bool(x.get(kv.key()));
            if (v.has_value()) {
                result[kv.key().copy()] = *v;
            }
        }
        return result;
    } else {
        throw std::runtime_error(pn::format("{0}: must be null or array", x.path()).c_str());
    }
}

sfz::optional<BaseObject::Weapon> optional_weapon(path_value x) {
    return optional_struct<BaseObject::Weapon>(
            x, {
                       {"base", {&BaseObject::Weapon::base, required_base}},
                       {"positions",
                        {&BaseObject::Weapon::positions,
                         optional_array<fixedPointType, required_fixed_point>}},
               });
}

static int16_t required_layer(path_value x) { return required_int(x, {1, 4}); }

static sfz::optional<int32_t> optional_scale(path_value x) {
    sfz::optional<Fixed> f = optional_fixed(x);
    if (f.has_value()) {
        return sfz::make_optional<int32_t>(f->val() << 4);
    }
    return sfz::nullopt;
}

sfz::optional<BaseObject::Rotation> optional_rotation_frame(path_value x) {
    using Rotation = BaseObject::Rotation;
    return optional_struct<Rotation>(
            x, {
                       {"sprite", {&Rotation::sprite, required_string_copy}},
                       {"layer", {&Rotation::layer, required_layer}},
                       {"scale", {&Rotation::scale, optional_scale, SCALE_SCALE}},
                       {"frames", {&Rotation::frames, required_int_range}},
               });
}

sfz::optional<BaseObject::Animation> optional_animation_frame(path_value x) {
    using Animation = BaseObject::Animation;
    return optional_struct<Animation>(
            x, {
                       {"sprite", {&Animation::sprite, required_string_copy}},
                       {"layer", {&Animation::layer, required_layer}},
                       {"scale", {&Animation::scale, optional_scale, SCALE_SCALE}},
                       {"frames",
                        {&Animation::frames, optional_fixed_range,
                         Range<Fixed>{Fixed::zero(), Fixed::from_val(1)}}},
                       {"direction",
                        {&Animation::direction, optional_animation_direction,
                         AnimationDirection::NONE}},
                       {"speed", {&Animation::speed, optional_fixed, Fixed::zero()}},
                       {"first",
                        {&Animation::first, optional_fixed_range,
                         Range<Fixed>{Fixed::zero(), Fixed::from_val(1)}}},
               });
}

sfz::optional<BaseObject::Vector> optional_vector_frame(path_value x) {
    using Vector = BaseObject::Vector;
    return optional_struct<Vector>(
            x, {
                       {"kind", {&Vector::kind, required_vector_kind}},
                       {"accuracy", {&Vector::accuracy, required_int32}},
                       {"range", {&Vector::range, required_int32}},
                       {"color", {&Vector::color, optional_color, RgbColor::clear()}},
                       {"hue", {&Vector::hue, optional_hue}},
               });
}

uint32_t optional_usage(path_value x) {
    if (x.value().is_null()) {
        return 0;
    } else if (x.value().is_map()) {
        static const pn::string_view flags[3] = {"transportation", "attacking", "defense"};
        uint32_t                     bit      = 0x00000001;
        uint32_t                     result   = 0x00000000;
        for (pn::string_view flag : flags) {
            if (optional_bool(x.get(flag)).value_or(false)) {
                result |= bit;
            }
            bit <<= 1;
        }
        return result;
    } else {
        throw std::runtime_error(pn::format("{0}: must be null or map", x.path()).c_str());
    }
}

sfz::optional<BaseObject::Device> optional_device_frame(path_value x) {
    using Device = BaseObject::Device;
    return optional_struct<Device>(
            x, {
                       {"usage", {&Device::usage, optional_usage}},
                       {"energy_cost", {&Device::energyCost, optional_int32, 0}},
                       {"fire_time", {&Device::fireTime, required_ticks}},
                       {"ammo", {&Device::ammo, optional_int32, -1}},
                       {"range", {&Device::range, required_int32}},
                       {"inverse_speed", {&Device::inverseSpeed, optional_fixed, Fixed::zero()}},
                       {"restock_cost", {&Device::restockCost, optional_int32, -1}},
               });
}

static sfz::optional<BaseObject::Icon> optional_icon(path_value x) {
    return optional_struct<BaseObject::Icon>(
            x, {
                       {"shape", {&BaseObject::Icon::shape, required_icon_shape}},
                       {"size", {&BaseObject::Icon::size, required_int}},
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

BaseObject set_attributes(BaseObject o) {
    if (((o.rotation.has_value() + o.animation.has_value() + o.vector.has_value() +
          o.device.has_value()) != 1)) {
        throw std::runtime_error("must have single rotation, animation, vector, or device block");
    } else if (o.rotation.has_value()) {
        o.attributes |= kShapeFromDirection;
    } else if (o.animation.has_value()) {
        o.attributes |= kIsSelfAnimated;
        if (!o.expire.after.animation) {
            o.attributes |= kAnimationCycle;
        }
    } else if (o.vector.has_value()) {
        o.attributes |= kIsVector;
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
    return o;
}

BaseObject::Destroy optional_destroy(path_value x) {
    return optional_struct<BaseObject::Destroy>(
                   x,
                   {
                           {"dont_die", {&BaseObject::Destroy::dont_die, optional_bool, false}},
                           {"neutralize",
                            {&BaseObject::Destroy::neutralize, optional_bool, false}},
                           {"release_energy",
                            {&BaseObject::Destroy::release_energy, optional_bool, false}},
                           {"action", {&BaseObject::Destroy::action, optional_action_array}},
                   })
            .value_or(BaseObject::Destroy{});
}

BaseObject::Expire::After optional_expire_after(path_value x) {
    return optional_struct<BaseObject::Expire::After>(
                   x,
                   {
                           {"age",
                            {&BaseObject::Expire::After::age, optional_ticks_range,
                             Range<ticks>{ticks(-1), ticks(-1)}}},
                           {"animation",
                            {&BaseObject::Expire::After::animation, optional_bool, false}},
                   })
            .value_or(BaseObject::Expire::After{});
}

BaseObject::Expire optional_expire(path_value x) {
    return optional_struct<BaseObject::Expire>(
                   x,
                   {
                           {"after", {&BaseObject::Expire::after, optional_expire_after}},
                           {"dont_die", {&BaseObject::Expire::dont_die, optional_bool, false}},
                           {"action", {&BaseObject::Expire::action, optional_action_array}},
                   })
            .value_or(BaseObject::Expire{});
}

BaseObject::Create optional_create(path_value x) {
    return optional_struct<BaseObject::Create>(
                   x,
                   {
                           {"action", {&BaseObject::Create::action, optional_action_array}},
                   })
            .value_or(BaseObject::Create{});
}

BaseObject::Collide::As optional_collide_as(path_value x) {
    return optional_struct<BaseObject::Collide::As>(
                   x,
                   {
                           {"subject", {&BaseObject::Collide::As::subject, optional_bool, false}},
                           {"object", {&BaseObject::Collide::As::object, optional_bool, false}},
                   })
            .value_or(BaseObject::Collide::As{});
}

BaseObject::Collide optional_collide(path_value x) {
    return optional_struct<BaseObject::Collide>(
                   x,
                   {
                           {"as", {&BaseObject::Collide::as, optional_collide_as}},
                           {"damage", {&BaseObject::Collide::damage, optional_int32, 0}},
                           {"solid", {&BaseObject::Collide::solid, optional_bool, false}},
                           {"edge", {&BaseObject::Collide::edge, optional_bool, false}},
                           {"action", {&BaseObject::Collide::action, optional_action_array}},
                   })
            .value_or(BaseObject::Collide{});
}

BaseObject::Activate optional_activate(path_value x) {
    return optional_struct<BaseObject::Activate>(
                   x,
                   {
                           {"period",
                            {&BaseObject::Activate::period, optional_ticks_range,
                             Range<ticks>{ticks(0), ticks(0)}}},
                           {"action", {&BaseObject::Activate::action, optional_action_array}},
                   })
            .value_or(BaseObject::Activate{});
}

BaseObject::Arrive optional_arrive(path_value x) {
    return optional_struct<BaseObject::Arrive>(
                   x,
                   {
                           {"distance", {&BaseObject::Arrive::distance, optional_int32, 0}},
                           {"action", {&BaseObject::Arrive::action, optional_action_array}},
                   })
            .value_or(BaseObject::Arrive{});
}

BaseObject base_object(pn::value_cref x0) {
    return set_attributes(required_struct<BaseObject>(
            path_value{x0},
            {
                    {"attributes", {&BaseObject::attributes, optional_object_attributes}},
                    {"build_flags", {&BaseObject::buildFlags, optional_object_build_flags}},
                    {"order_flags", {&BaseObject::orderFlags, optional_object_order_flags}},

                    {"long_name", {&BaseObject::name, required_string_copy}},
                    {"short_name", {&BaseObject::short_name, required_string_copy}},

                    {"notes", nullptr},
                    {"class", nullptr},
                    {"race", nullptr},
                    {"danger_threshold", nullptr},

                    {"portrait", {&BaseObject::portrait, optional_string, ""}},

                    {"price", {&BaseObject::price, optional_int32, 0}},
                    {"destination_class", {&BaseObject::destinationClass, optional_int32, 0}},
                    {"warp_out_distance", {&BaseObject::warpOutDistance, optional_uint32, 0}},
                    {"health", {&BaseObject::health, optional_int32, 0}},
                    {"energy", {&BaseObject::energy, optional_int32, 0}},
                    {"skill_num", {&BaseObject::skillNum, optional_uint8, 0}},
                    {"skill_den", {&BaseObject::skillDen, optional_uint8, 0}},
                    {"occupy_count", {&BaseObject::occupy_count, optional_int32, -1}},

                    {"offense", {&BaseObject::offenseValue, optional_fixed, Fixed::zero()}},
                    {"max_velocity", {&BaseObject::maxVelocity, optional_fixed, Fixed::zero()}},
                    {"warp_speed", {&BaseObject::warpSpeed, optional_fixed, Fixed::zero()}},
                    {"mass", {&BaseObject::mass, optional_fixed, Fixed::zero()}},
                    {"turn_rate", {&BaseObject::turn_rate, optional_fixed, Fixed::zero()}},
                    {"max_thrust", {&BaseObject::maxThrust, optional_fixed, Fixed::zero()}},
                    {"friend_deficit",
                     {&BaseObject::friendDefecit, optional_fixed, Fixed::zero()}},
                    {"build_ratio", {&BaseObject::buildRatio, optional_fixed, Fixed::zero()}},

                    {"build_time", {&BaseObject::buildTime, optional_ticks, ticks(0)}},

                    {"shield_color", {&BaseObject::shieldColor, optional_color}},

                    {"initial_velocity",
                     {&BaseObject::initial_velocity, optional_fixed_range,
                      Range<Fixed>{Fixed::zero(), Fixed::zero()}}},
                    {"initial_direction",
                     {&BaseObject::initial_direction, optional_int_range, Range<int64_t>{0, 0}}},

                    {"destroy", {&BaseObject::destroy, optional_destroy}},
                    {"expire", {&BaseObject::expire, optional_expire}},
                    {"create", {&BaseObject::create, optional_create}},
                    {"collide", {&BaseObject::collide, optional_collide}},
                    {"activate", {&BaseObject::activate, optional_activate}},
                    {"arrive", {&BaseObject::arrive, optional_arrive}},

                    {"icon",
                     {&BaseObject::icon, optional_icon, BaseObject::Icon{IconShape::SQUARE, 0}}},
                    {"weapons", {&BaseObject::weapons, optional_loadout}},

                    {"rotation", {&BaseObject::rotation, optional_rotation_frame}},
                    {"animation", {&BaseObject::animation, optional_animation_frame}},
                    {"vector", {&BaseObject::vector, optional_vector_frame}},
                    {"device", {&BaseObject::device, optional_device_frame}},

                    {"tags", {&BaseObject::tags, optional_tags}},
                    {"engage_tag", {&BaseObject::engageKeyTag, optional_string, ""}},
                    {"order_tag", {&BaseObject::orderKeyTag, optional_string, ""}},
            }));
}

}  // namespace antares
