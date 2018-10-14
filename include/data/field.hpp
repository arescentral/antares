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
#include "data/range.hpp"
#include "data/tags.hpp"
#include "drawing/color.hpp"
#include "math/fixed.hpp"
#include "math/geometry.hpp"
#include "math/units.hpp"

namespace antares {

union Level;
struct Initial;
struct Condition;
struct Race;

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

    bool       is_root() const;
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

template <typename T>
struct field_reader;

template <typename T>
T read_field(path_value x) {
    return field_reader<T>::read(x);
}

#define DECLARE_FIELD_READER(T)      \
    template <>                      \
    struct field_reader<T> {         \
        static T read(path_value x); \
    }

#define DEFINE_FIELD_READER(T) T field_reader<T>::read(path_value x)

#define FIELD_READER(T)      \
    DECLARE_FIELD_READER(T); \
    DEFINE_FIELD_READER(T)

DECLARE_FIELD_READER(bool);
DECLARE_FIELD_READER(sfz::optional<bool>);
DECLARE_FIELD_READER(int64_t);
DECLARE_FIELD_READER(sfz::optional<int64_t>);
DECLARE_FIELD_READER(uint8_t);
DECLARE_FIELD_READER(int32_t);

int64_t int_field_within(path_value x, Range<int64_t> range);

DECLARE_FIELD_READER(double);
DECLARE_FIELD_READER(Fixed);
DECLARE_FIELD_READER(sfz::optional<Fixed>);

DECLARE_FIELD_READER(pn::string_view);
DECLARE_FIELD_READER(sfz::optional<pn::string_view>);
DECLARE_FIELD_READER(pn::string);
DECLARE_FIELD_READER(sfz::optional<pn::string>);

DECLARE_FIELD_READER(sfz::optional<ticks>);
DECLARE_FIELD_READER(ticks);
DECLARE_FIELD_READER(sfz::optional<secs>);

DECLARE_FIELD_READER(Tags);

DECLARE_FIELD_READER(sfz::optional<Handle<Admiral>>);
DECLARE_FIELD_READER(Handle<Admiral>);
DECLARE_FIELD_READER(sfz::optional<NamedHandle<const BaseObject>>);
DECLARE_FIELD_READER(NamedHandle<const BaseObject>);
DECLARE_FIELD_READER(sfz::optional<Handle<const Initial>>);
DECLARE_FIELD_READER(Handle<const Initial>);
DECLARE_FIELD_READER(sfz::optional<Handle<const Condition>>);
DECLARE_FIELD_READER(Handle<const Condition>);
DECLARE_FIELD_READER(sfz::optional<NamedHandle<const Level>>);
DECLARE_FIELD_READER(sfz::optional<Owner>);
DECLARE_FIELD_READER(Owner);
DECLARE_FIELD_READER(NamedHandle<const Race>);

DECLARE_FIELD_READER(sfz::optional<Range<int64_t>>);
DECLARE_FIELD_READER(Range<int64_t>);
DECLARE_FIELD_READER(sfz::optional<Range<Fixed>>);
DECLARE_FIELD_READER(Range<Fixed>);
DECLARE_FIELD_READER(sfz::optional<Range<ticks>>);
DECLARE_FIELD_READER(Range<ticks>);

DECLARE_FIELD_READER(sfz::optional<Point>);
DECLARE_FIELD_READER(Point);
DECLARE_FIELD_READER(sfz::optional<Rect>);
DECLARE_FIELD_READER(Rect);

DECLARE_FIELD_READER(sfz::optional<RgbColor>);
DECLARE_FIELD_READER(RgbColor);

DECLARE_FIELD_READER(sfz::optional<Hue>);
DECLARE_FIELD_READER(Hue);
DECLARE_FIELD_READER(Screen);
DECLARE_FIELD_READER(Zoom);

template <typename T, int N>
sfz::optional<T> optional_enum(path_value x, const std::pair<pn::string_view, T> (&values)[N]) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_string()) {
        pn::string_view s = x.value().as_string();
        for (auto kv : values) {
            if (s == kv.first) {
                sfz::optional<T> t;
                t.emplace(kv.second);
                return t;
            }
        }
    }

    pn::array keys;
    for (auto kv : values) {
        keys.push_back(kv.first.copy());
    }
    throw std::runtime_error(pn::format("{0}must be one of {1}", x.prefix(), keys).c_str());
}

template <typename T, int N>
T required_enum(path_value x, const std::pair<pn::string_view, T> (&values)[N]) {
    if (x.value().is_string()) {
        for (auto kv : values) {
            pn::string_view s = x.value().as_string();
            if (s == kv.first) {
                return kv.second;
            }
        }
    }

    pn::array keys;
    for (auto kv : values) {
        keys.push_back(kv.first.copy());
    }
    throw std::runtime_error(pn::format("{0}must be one of {1}", x.prefix(), keys).c_str());
}

template <typename T>
T required_object_type(path_value x, T (*get_type)(path_value x)) {
    if (!x.value().is_map()) {
        throw std::runtime_error(pn::format("{0}must be map", x.prefix()).c_str());
    }
    return get_type(x.get("type"));
}

template <typename T>
struct field {
    std::function<void(T* t, path_value x)> set;

    constexpr field(std::nullptr_t) : set([](T*, path_value) {}) {}

    template <typename F, typename U>
    constexpr field(F(U::*field))
            : set([field](T* t, path_value x) { (t->*field) = read_field<F>(x); }) {}
};

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

template <typename T, T (*F)(path_value x)>
static std::vector<T> required_array(path_value x) {
    if (x.value().is_array()) {
        pn::array_cref a = x.value().as_array();
        std::vector<T> result;
        for (int i = 0; i < a.size(); ++i) {
            result.emplace_back(F(x.get(i)));
        }
        return result;
    } else {
        throw std::runtime_error(pn::format("{0}must be array", x.prefix()).c_str());
    }
}

template <typename T, T (*F)(path_value x)>
static std::vector<T> optional_array(path_value x) {
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
        throw std::runtime_error(pn::format("{0}must be null or array", x.prefix()).c_str());
    }
}

template <typename T>
struct field_reader<std::vector<T>> {
    static std::vector<T> read(path_value x) {
        return optional_array<T, field_reader<T>::read>(x);
    }
};

}  // namespace antares

#endif  // ANTARES_DATA_FIELD_HPP_
