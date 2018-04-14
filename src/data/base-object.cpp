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
                                                  "hard_base"};

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
    if (x.value().is_map()) {
        Fixed px = required_fixed(x.get("x"));
        Fixed py = required_fixed(x.get("y"));
        return {px, py};
    } else {
        throw std::runtime_error(pn::format("{0}: must be map", x.path()).c_str());
    }
}

std::vector<fixedPointType> optional_fixed_point_array(path_value x) {
    if (x.value().is_null()) {
        return {};
    } else if (x.value().is_array()) {
        pn::array_cref              a = x.value().as_array();
        std::vector<fixedPointType> result;
        for (int i = 0; i < a.size(); ++i) {
            result.emplace_back(required_fixed_point(x.get(i)));
        }
        return result;
    } else {
        throw std::runtime_error(pn::format("{0}: must be null or array", x.path()).c_str());
    }
}

sfz::optional<BaseObject::Weapon> optional_weapon(path_value x) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_map()) {
        BaseObject::Weapon w;
        w.base      = required_base(x.get("base"));
        w.positions = optional_fixed_point_array(x.get("positions"));
        return sfz::make_optional(std::move(w));
    } else {
        throw std::runtime_error(pn::format("{0}: must be null or map", x.path()).c_str());
    }
}

static sfz::optional<int16_t> optional_layer(path_value x) {
    sfz::optional<int64_t> i = optional_int(x, {1, 4});
    if (i.has_value()) {
        return sfz::make_optional<int16_t>(*i);
    }
    return sfz::nullopt;
}

static sfz::optional<int32_t> optional_scale(path_value x) {
    sfz::optional<Fixed> f = optional_fixed(x);
    if (f.has_value()) {
        return sfz::make_optional<int32_t>(f->val() << 4);
    }
    return sfz::nullopt;
}

BaseObject::Rotation optional_rotation_frame(path_value x) {
    using Rotation = BaseObject::Rotation;
    return optional_struct<Rotation>(
                   x,
                   {
                           {"sprite", {&Rotation::sprite, required_string_copy}},
                           {"layer", {&Rotation::layer, optional_layer, 0}},
                           {"scale", {&Rotation::scale, optional_scale, 4096}},
                           {"frames", {&Rotation::frames, required_int_range}},
                           {"turn_rate", {&Rotation::turn_rate, optional_fixed, Fixed::zero()}},
                   })
            .value_or(Rotation{});
}

BaseObject::Animation optional_animation_frame(path_value x) {
    using Animation = BaseObject::Animation;
    return optional_struct<Animation>(
                   x,
                   {
                           {"sprite", {&Animation::sprite, required_string_copy}},
                           {"layer", {&Animation::layer, optional_layer, 0}},
                           {"scale", {&Animation::scale, optional_scale, 4096}},
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
                   })
            .value_or(Animation{});
}

BaseObject::Vector optional_vector_frame(path_value x) {
    if (x.value().is_null()) {
        return BaseObject::Vector{};
    } else if (x.value().is_map()) {
        BaseObject::Vector v;
        v.kind     = required_vector_kind(x.get("kind"));
        v.accuracy = required_int(x.get("accuracy"));
        v.range    = required_int(x.get("range"));

        sfz::optional<RgbColor> color = optional_color(x.get("color"));
        sfz::optional<Hue>      hue   = optional_hue(x.get("hue"));
        if (v.kind == VectorKind::BOLT) {
            v.visible    = color.has_value();
            v.bolt_color = color.value_or(RgbColor::clear());
            v.beam_hue   = Hue::GRAY;
        } else {
            v.visible    = hue.has_value();
            v.bolt_color = RgbColor::clear();
            v.beam_hue   = hue.value_or(Hue::GRAY);
        }
        return v;
    } else {
        throw std::runtime_error(pn::format("{0}: must be map", x.path()).c_str());
    }
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

BaseObject::Device optional_device_frame(path_value x) {
    using Device = BaseObject::Device;
    return optional_struct<Device>(
                   x,
                   {
                           {"usage", {&Device::usage, optional_usage}},
                           {"energy_cost", {&Device::energyCost, optional_int32, 0}},
                           {"fire_time", {&Device::fireTime, required_ticks}},
                           {"ammo", {&Device::ammo, optional_int32, -1}},
                           {"range", {&Device::range, required_int32}},
                           {"inverse_speed",
                            {&Device::inverseSpeed, optional_fixed, Fixed::zero()}},
                           {"restock_cost", {&Device::restockCost, optional_int32, -1}},
                   })
            .value_or(Device{});
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

BaseObject base_object(pn::value_cref x0) {
    return required_struct<BaseObject>(
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
                    {"damage", {&BaseObject::damage, optional_int32, 0}},
                    {"energy", {&BaseObject::energy, optional_int32, 0}},
                    {"skill_num", {&BaseObject::skillNum, optional_uint8, 0}},
                    {"skill_den", {&BaseObject::skillDen, optional_uint8, 0}},
                    {"occupy_count", {&BaseObject::occupy_count, optional_int32, -1}},
                    {"arrive_action_distance",
                     {&BaseObject::arriveActionDistance, optional_int32, 0}},

                    {"offense", {&BaseObject::offenseValue, optional_fixed, Fixed::zero()}},
                    {"max_velocity", {&BaseObject::maxVelocity, optional_fixed, Fixed::zero()}},
                    {"warp_speed", {&BaseObject::warpSpeed, optional_fixed, Fixed::zero()}},
                    {"mass", {&BaseObject::mass, optional_fixed, Fixed::zero()}},
                    {"max_thrust", {&BaseObject::maxThrust, optional_fixed, Fixed::zero()}},
                    {"friend_deficit",
                     {&BaseObject::friendDefecit, optional_fixed, Fixed::zero()}},
                    {"build_ratio", {&BaseObject::buildRatio, optional_fixed, Fixed::zero()}},

                    {"build_time", {&BaseObject::buildTime, optional_ticks, ticks(0)}},

                    {"shield_color", {&BaseObject::shieldColor, optional_color}},

                    {"initial_velocity",
                     {&BaseObject::initial_velocity, optional_fixed_range,
                      Range<Fixed>{Fixed::zero(), Fixed::zero()}}},
                    {"initial_age",
                     {&BaseObject::initial_age, optional_ticks_range,
                      Range<ticks>{ticks(-1), ticks(-1)}}},
                    {"initial_direction",
                     {&BaseObject::initial_direction, optional_int_range, Range<int64_t>{0, 0}}},

                    {"on_destroy", {&BaseObject::destroy, optional_action_array}},
                    {"on_expire", {&BaseObject::expire, optional_action_array}},
                    {"on_create", {&BaseObject::create, optional_action_array}},
                    {"on_collide", {&BaseObject::collide, optional_action_array}},
                    {"on_activate", {&BaseObject::activate, optional_action_array}},
                    {"on_arrive", {&BaseObject::arrive, optional_action_array}},

                    {"icon",
                     {&BaseObject::icon, optional_icon, BaseObject::Icon{IconShape::SQUARE, 0}}},
                    {"weapons", {&BaseObject::weapons, optional_loadout}},

                    {"rotation", {&BaseObject::rotation, optional_rotation_frame}},
                    {"animation", {&BaseObject::animation, optional_animation_frame}},
                    {"vector", {&BaseObject::vector, optional_vector_frame}},
                    {"device", {&BaseObject::device, optional_device_frame}},

                    {"destroy_dont_die", {&BaseObject::destroyDontDie, optional_bool, false}},
                    {"expire_dont_die", {&BaseObject::expireDontDie, optional_bool, false}},
                    {"activate_period",
                     {&BaseObject::activate_period, optional_ticks_range,
                      Range<ticks>{ticks(0), ticks(0)}}},

                    {"level_tag", {&BaseObject::levelKeyTag, optional_string, ""}},
                    {"engage_tag", {&BaseObject::engageKeyTag, optional_string, ""}},
                    {"order_tag", {&BaseObject::orderKeyTag, optional_string, ""}},
            });
}

}  // namespace antares
