// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2018 The Antares Authors
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

#ifndef ANTARES_DATA_FIELD_HPP_
#define ANTARES_DATA_FIELD_HPP_

#include <pn/fwd>
#include <pn/string>
#include <pn/value>
#include <sfz/sfz.hpp>

#include "data/enums.hpp"
#include "data/handle.hpp"
#include "drawing/color.hpp"
#include "math/fixed.hpp"
#include "math/geometry.hpp"
#include "math/units.hpp"

namespace antares {

struct Level;
struct Level_Initial;
struct Level_Condition;
struct Race;

template <typename T>
struct Range {
    T begin, end;

    T range() const { return end - begin; }

    static constexpr Range empty() { return {-1, -1}; }
};

class path_value {
    enum class Kind { ROOT, KEY, INDEX };

  public:
    path_value(pn::value_cref x) : _kind{Kind::ROOT}, _value{x} {}

    pn::value_cref value() const { return _value; }

    path_value get(pn::string_view key) const {
        return path_value{this, Kind::KEY, key, 0, _value.as_map().get(key)};
    }
    path_value get(int64_t index) const {
        return path_value{this, Kind::INDEX, pn::string_view{}, index,
                          array_get(_value.as_array(), index)};
    }

    pn::string path() const;
    pn::string prefix() const;

  private:
    static pn::value_cref array_get(pn::array_cref a, int64_t index);

    path_value(
            const path_value* parent, Kind kind, pn::string_view key, int64_t index,
            pn::value_cref value)
            : _parent{parent}, _kind{kind}, _key{key}, _index{index}, _value{value} {}

    const path_value* _parent = nullptr;
    Kind              _kind;
    pn::string_view   _key;
    int64_t           _index;
    pn::value_cref    _value;
};

sfz::optional<bool> optional_bool(path_value x);
bool                required_bool(path_value x);

sfz::optional<int64_t> optional_int(path_value x);
int64_t                required_int(path_value x);
sfz::optional<int64_t> optional_int(path_value x, const std::initializer_list<int64_t>& ranges);
int64_t                required_int(path_value x, const std::initializer_list<int64_t>& ranges);

double required_double(path_value x);

sfz::optional<Fixed> optional_fixed(path_value x);
Fixed                required_fixed(path_value x);

sfz::optional<pn::string_view> optional_string(path_value x);
sfz::optional<pn::string>      optional_string_copy(path_value x);
pn::string_view                required_string(path_value x);
pn::string                     required_string_copy(path_value x);

sfz::optional<ticks> optional_ticks(path_value x);
ticks                required_ticks(path_value x);
sfz::optional<secs>  optional_secs(path_value x);

sfz::optional<Handle<Admiral>>             optional_admiral(path_value x);
Handle<Admiral>                            required_admiral(path_value x);
NamedHandle<const BaseObject>              required_base(path_value x);
sfz::optional<Handle<const Level_Initial>> optional_initial(path_value x);
Handle<const Level_Initial>                required_initial(path_value x);
sfz::optional<NamedHandle<const Level>>    optional_level(path_value x);
sfz::optional<Owner>                       optional_owner(path_value x);
Owner                                      required_owner(path_value x);
NamedHandle<const Race>                    required_race(path_value x);

sfz::optional<Range<int64_t>>     optional_int_range(path_value x);
Range<int64_t>                    required_int_range(path_value x);
sfz::optional<Range<Fixed>>       optional_fixed_range(path_value x);
Range<Fixed>                      required_fixed_range(path_value x);
sfz::optional<Range<ticks>>       optional_ticks_range(path_value x);
Range<ticks>                      required_ticks_range(path_value x);
HandleList<const Level_Condition> optional_condition_range(path_value x);
HandleList<const Level_Initial>   required_initial_range(path_value x);

sfz::optional<Point> optional_point(path_value x);
Point                required_point(path_value x);
Rect                 required_rect(path_value x);

sfz::optional<RgbColor> optional_color(path_value x);
RgbColor                required_color(path_value x);

AnimationDirection        required_animation_direction(path_value x);
sfz::optional<Hue>        optional_hue(path_value x);
Hue                       required_hue(path_value x);
InterfaceStyle            required_interface_style(path_value x);
KillKind                  required_kill_kind(path_value x);
sfz::optional<MoveOrigin> optional_origin(path_value x);
int                       required_key(path_value x);
ConditionOp               required_condition_op(path_value x);
IconShape                 required_icon_shape(path_value x);
LevelType                 required_level_type(path_value x);
PlayerType                required_player_type(path_value x);
PushKind                  required_push_kind(path_value x);
Screen                    required_screen(path_value x);
SubjectValue              required_subject_value(path_value x);
Weapon                    required_weapon(path_value x);
Zoom                      required_zoom(path_value x);

uint32_t optional_object_attributes(path_value x);
uint32_t optional_keys(path_value x);

std::vector<pn::string> required_string_array(path_value x);
std::vector<pn::string> optional_string_array(path_value x);
std::vector<int>        optional_int_array(path_value x);

template <typename T>
struct field {
    std::function<void(T* t, path_value x)> set;

    constexpr field(std::nullptr_t) : set([](T*, path_value) {}) {}

    template <typename F, typename U>
    constexpr field(F(U::*field), F (*reader)(path_value x))
            : set([field, reader](T* t, path_value x) { (t->*field) = reader(x); }) {}

    template <typename F, typename U, typename D>
    constexpr field(F(U::*field), sfz::optional<F> (*reader)(path_value x), const D& default_value)
            : set([field, reader, default_value](T* t, path_value x) {
                  (t->*field) = reader(x).value_or(default_value);
              }) {}

    template <typename U>
    constexpr field(
            pn::string(U::*field), sfz::optional<pn::string_view> (*reader)(path_value x),
            pn::string_view default_value)
            : set([field, reader, default_value](T* t, path_value x) {
                  (t->*field) = reader(x).value_or(default_value).copy();
              }) {}
};

uint32_t optional_flags(path_value x, const std::map<pn::string_view, int>& flags);

template <typename T>
T required_struct(path_value x, const std::map<pn::string_view, field<T>>& fields) {
    if (x.value().is_map()) {
        T t;
        for (const auto& kv : fields) {
            path_value v = x.get(kv.first);
            kv.second.set(&t, v);
        }
        for (const auto& kv : x.value().as_map()) {
            pn::string_view k = kv.key();
            path_value      v = x.get(k);
            if (fields.find(k) == fields.end()) {
                throw std::runtime_error(pn::format("{0}unknown field", v.prefix()).c_str());
            }
        }
        return t;
    } else {
        throw std::runtime_error(pn::format("{0}must be map", x.prefix()).c_str());
    }
}

template <typename T>
sfz::optional<T> optional_struct(path_value x, const std::map<pn::string_view, field<T>>& fields) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_map()) {
        return sfz::make_optional(required_struct(x, fields));
    } else {
        throw std::runtime_error(pn::format("{0}must be null or map", x.prefix()).c_str());
    }
}

}  // namespace antares

#endif  // ANTARES_DATA_FIELD_HPP_
