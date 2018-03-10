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

int32_t optional_object_order_flags(path_value x) {
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

        int32_t bit    = 0x00000001;
        int32_t result = 0x00000000;
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

int32_t optional_object_build_flags(path_value x) {
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

        int32_t bit    = 0x00000001;
        int32_t result = 0x00000000;
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
        if (w.positions.empty()) {
            w.positions.push_back({Fixed::zero(), Fixed::zero()});
        }
        return sfz::make_optional(std::move(w));
    } else {
        throw std::runtime_error(pn::format("{0}: must be null or map", x.path()).c_str());
    }
}

objectFrameType::Rotation required_rotation_frame(path_value x) {
    if (x.value().is_map()) {
        objectFrameType::Rotation r;
        r.shapeOffset = optional_int(x.get("offset")).value_or(0);
        r.rotRes      = required_int(x.get("resolution"));
        r.maxTurnRate = optional_fixed(x.get("turn_rate")).value_or(Fixed::zero());
        return r;
    } else {
        throw std::runtime_error(pn::format("{0}: must be map", x.path()).c_str());
    }
}

objectFrameType::Animation required_animation_frame(path_value x) {
    if (x.value().is_null()) {
        objectFrameType::Animation a;
        a.shapes    = Range<Fixed>{Fixed::zero(), Fixed::from_val(1)};
        a.direction = AnimationDirection::NONE;
        a.speed     = Fixed::zero();
        a.first     = Range<Fixed>{Fixed::zero(), Fixed::from_val(1)};
        return a;
    } else if (x.value().is_map()) {
        objectFrameType::Animation a;
        a.shapes = optional_fixed_range(x.get("shape"))
                           .value_or(Range<Fixed>{Fixed::zero(), Fixed::from_val(1)});
        a.direction = optional_animation_direction(x.get("direction"))
                              .value_or(AnimationDirection::NONE);
        a.speed = optional_fixed(x.get("speed")).value_or(Fixed::zero());
        a.first = optional_fixed_range(x.get("first"))
                          .value_or(Range<Fixed>{Fixed::zero(), Fixed::from_val(1)});
        return a;
    } else {
        throw std::runtime_error(pn::format("{0}: must be null or map", x.path()).c_str());
    }
}

objectFrameType::Vector required_vector_frame(path_value x) {
    if (x.value().is_map()) {
        objectFrameType::Vector v;
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

int32_t optional_usage(path_value x) {
    if (x.value().is_null()) {
        return 0;
    } else if (x.value().is_map()) {
        static const pn::string_view flags[3] = {"transportation", "attacking", "defense"};
        int32_t                      bit      = 0x00000001;
        int32_t                      result   = 0x00000000;
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

objectFrameType::Weapon required_device_frame(path_value x) {
    if (x.value().is_map()) {
        objectFrameType::Weapon w;
        w.usage        = optional_usage(x.get("usage"));
        w.energyCost   = optional_int(x.get("energy_cost")).value_or(0);
        w.fireTime     = required_ticks(x.get("fire_time"));
        w.ammo         = optional_int(x.get("ammo")).value_or(-1);
        w.range        = required_int(x.get("range"));
        w.inverseSpeed = optional_fixed(x.get("inverse_speed")).value_or(Fixed::zero());
        w.restockCost  = optional_int(x.get("restock_cost")).value_or(-1);
        return w;
    } else {
        throw std::runtime_error(pn::format("{0}: must be map", x.path()).c_str());
    }
}

BaseObject base_object(pn::value_cref x0) {
    if (!x0.is_map()) {
        throw std::runtime_error("must be map");
    }

    path_value x{x0};
    BaseObject o;
    o.attributes = optional_object_attributes(x.get("attributes"));
    o.buildFlags = optional_object_build_flags(x.get("build_flags"));
    o.orderFlags = optional_object_order_flags(x.get("order_flags"));

    o.name       = required_string(x.get("long_name")).copy();
    o.short_name = required_string(x.get("short_name")).copy();

    o.price             = optional_int(x.get("price")).value_or(0);
    o.destinationClass  = optional_int(x.get("destination_class")).value_or(0);
    o.warpOutDistance   = optional_int(x.get("warp_out_distance")).value_or(0);
    o.health            = optional_int(x.get("health")).value_or(0);
    o.damage            = optional_int(x.get("damage")).value_or(0);
    o.energy            = optional_int(x.get("energy")).value_or(0);
    o.naturalScale      = optional_fixed(x.get("scale")).value_or(Fixed::from_long(1)).val() * 16;
    o.pixLayer          = optional_int(x.get("layer")).value_or(0);
    o.pixResID          = optional_int(x.get("sprite")).value_or(-1);
    o.skillNum          = optional_int(x.get("skill_num")).value_or(0);
    o.skillDen          = optional_int(x.get("skill_den")).value_or(0);
    o.pictPortraitResID = optional_int(x.get("portrait")).value_or(0);
    o.occupy_count      = optional_int(x.get("occupy_count")).value_or(-1);
    o.arriveActionDistance = optional_int(x.get("arrive_action_distance")).value_or(0);

    o.offenseValue  = optional_fixed(x.get("offense")).value_or(Fixed::zero());
    o.maxVelocity   = optional_fixed(x.get("max_velocity")).value_or(Fixed::zero());
    o.warpSpeed     = optional_fixed(x.get("warp_speed")).value_or(Fixed::zero());
    o.mass          = optional_fixed(x.get("mass")).value_or(Fixed::zero());
    o.maxThrust     = optional_fixed(x.get("max_thrust")).value_or(Fixed::zero());
    o.friendDefecit = optional_fixed(x.get("friend_deficit")).value_or(Fixed::zero());
    o.buildRatio    = optional_fixed(x.get("build_ratio")).value_or(Fixed::zero());

    o.buildTime = optional_ticks(x.get("build_time")).value_or(ticks(0));

    o.shieldColor = optional_color(x.get("shield_color"));

    o.initial_velocity = optional_fixed_range(x.get("initial_velocity"))
                                 .value_or(Range<Fixed>{Fixed::zero(), Fixed::zero()});
    o.initial_age = optional_ticks_range(x.get("initial_age"))
                            .value_or(Range<ticks>{ticks(-1), ticks(-1)});
    o.initial_direction =
            optional_int_range(x.get("initial_direction")).value_or(Range<int64_t>{0, 0});

    o.destroy  = optional_action_array(x.get("on_destroy"));
    o.expire   = optional_action_array(x.get("on_expire"));
    o.create   = optional_action_array(x.get("on_create"));
    o.collide  = optional_action_array(x.get("on_collide"));
    o.activate = optional_action_array(x.get("on_activate"));
    o.arrive   = optional_action_array(x.get("on_arrive"));

    auto icon = x.get("icon");
    if (icon.value().is_null()) {
        o.icon = {IconShape::SQUARE, 0};
    } else if (icon.value().is_map()) {
        o.icon.shape = required_icon_shape(icon.get("shape"));
        o.icon.size  = required_int(icon.get("size"));
    } else {
        throw std::runtime_error(pn::format("{0}: must be null or map", icon.path()).c_str());
    }

    auto weapons = x.get("weapons");
    if (weapons.value().is_null()) {
        // no weapons
    } else if (weapons.value().is_map()) {
        o.pulse = optional_weapon(weapons.get("pulse"))
                          .value_or(BaseObject::Weapon{BaseObject::none()});
        o.beam = optional_weapon(weapons.get("beam"))
                         .value_or(BaseObject::Weapon{BaseObject::none()});
        o.special = optional_weapon(weapons.get("special"))
                            .value_or(BaseObject::Weapon{BaseObject::none()});
    } else {
        throw std::runtime_error(pn::format("{0}: must be null or map", weapons.path()).c_str());
    }

    auto frame = x.get("frame");
    if (o.attributes & kShapeFromDirection) {
        o.frame.rotation = required_rotation_frame(frame);
    } else if (o.attributes & kIsSelfAnimated) {
        o.frame.animation = required_animation_frame(frame);
    } else if (o.attributes & kIsVector) {
        o.frame.vector = required_vector_frame(frame);
    } else {
        o.frame.weapon = required_device_frame(frame);
    }

    o.destroyDontDie  = optional_bool(x.get("destroy_dont_die")).value_or(false);
    o.expireDontDie   = optional_bool(x.get("expire_dont_die")).value_or(false);
    o.activate_period = optional_ticks_range(x.get("activate_period"))
                                .value_or(Range<ticks>{ticks(0), ticks(0)});

    o.levelKeyTag  = optional_string(x.get("level_tag")).value_or("").copy();
    o.engageKeyTag = optional_string(x.get("engage_tag")).value_or("").copy();
    o.orderKeyTag  = optional_string(x.get("order_tag")).value_or("").copy();

    return o;
}

}  // namespace antares
