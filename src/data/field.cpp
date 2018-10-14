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

#include "data/field.hpp"

#include <set>

#include "data/level.hpp"

namespace antares {

pn::string path_value::path() const {
    if (_parent && (_parent->_kind != Kind::ROOT)) {
        if (_kind == Kind::KEY) {
            return pn::format("{0}.{1}", _parent->path(), _key);
        } else {
            return pn::format("{0}[{1}]", _parent->path(), _index);
        }
    } else {
        if (_kind == Kind::KEY) {
            return _key.copy();
        } else {
            return pn::format("[{0}]", _index);
        }
    }
}

bool path_value::is_root() const { return _kind == Kind::ROOT; }

pn::string path_value::prefix() const {
    if (_kind == Kind::ROOT) {
        return "";
    } else if (_parent->_kind == Kind::ROOT) {
        if (_kind == Kind::KEY) {
            return pn::format("{0}: ", _key);
        } else {
            return pn::format("[{0}]: ", _index);
        }
    } else {
        if (_kind == Kind::KEY) {
            return pn::format("{0}.{1}: ", _parent->path(), _key);
        } else {
            return pn::format("{0}[{1}]: ", _parent->path(), _index);
        }
    }
}

pn::value_cref path_value::array_get(pn::array_cref a, int64_t index) {
    if ((0 <= index) && (index < a.size())) {
        return a[index];
    } else {
        return pn::value_cref{&pn_null};
    }
}

DEFINE_FIELD_READER(sfz::optional<bool>) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_bool()) {
        return sfz::make_optional(x.value().as_bool());
    } else {
        throw std::runtime_error(pn::format("{0}must be null or bool", x.prefix()).c_str());
    }
}

DEFINE_FIELD_READER(bool) {
    if (x.value().is_bool()) {
        return x.value().as_bool();
    } else {
        throw std::runtime_error(pn::format("{0}must be bool", x.prefix()).c_str());
    }
}

DEFINE_FIELD_READER(sfz::optional<int64_t>) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_int()) {
        return sfz::make_optional(x.value().as_int());
    } else {
        throw std::runtime_error(pn::format("{0}must be null or int", x.prefix()).c_str());
    }
}

DEFINE_FIELD_READER(int64_t) {
    if (x.value().is_int()) {
        return x.value().as_int();
    } else {
        throw std::runtime_error(pn::format("{0}must be int", x.prefix()).c_str());
    }
}

static void check_range(path_value x, int64_t i, Range<int64_t> range) {
    if (i < range.begin) {
        throw std::runtime_error(
                pn::format("{0}must be >= {1} (was {2})", x.prefix(), range.begin, i).c_str());
    } else if (i >= range.end) {
        throw std::runtime_error(
                pn::format("{0}must be < {1} (was {2})", x.prefix(), range.end, i).c_str());
    }
}

int64_t int_field_within(path_value x, Range<int64_t> range) {
    auto i = read_field<int64_t>(x);
    check_range(x, i, range);
    return i;
}

DEFINE_FIELD_READER(double) {
    if (x.value().is_number()) {
        return x.value().as_number();
    } else {
        throw std::runtime_error(pn::format("{0}must be number", x.prefix()).c_str());
    }
}

DEFINE_FIELD_READER(sfz::optional<Fixed>) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_number()) {
        return sfz::make_optional(Fixed::from_float(x.value().as_number()));
    } else {
        throw std::runtime_error(pn::format("{0}must be null or number", x.prefix()).c_str());
    }
}

DEFINE_FIELD_READER(Fixed) {
    if (x.value().is_number()) {
        return Fixed::from_float(x.value().as_number());
    } else {
        throw std::runtime_error(pn::format("{0}must be number", x.prefix()).c_str());
    }
}

DEFINE_FIELD_READER(sfz::optional<pn::string_view>) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_string()) {
        return sfz::make_optional(x.value().as_string());
    } else {
        throw std::runtime_error(pn::format("{0}must be null or string", x.prefix()).c_str());
    }
}

DEFINE_FIELD_READER(sfz::optional<pn::string>) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_string()) {
        return sfz::make_optional(x.value().as_string().copy());
    } else {
        throw std::runtime_error(pn::format("{0}must be null or string", x.prefix()).c_str());
    }
}

DEFINE_FIELD_READER(pn::string_view) {
    if (x.value().is_string()) {
        return x.value().as_string();
    } else {
        throw std::runtime_error(pn::format("{0}must be string", x.prefix()).c_str());
    }
}

DEFINE_FIELD_READER(pn::string) { return read_field<pn::string_view>(x).copy(); }

static ticks parse_ticks(path_value x, pn::string_view s) {
    try {
        enum { START, START_COMPONENT, COMPONENT_DONE, INT, DECIMAL, FLOAT } state = START;
        pn::string_view::size_type begin                                           = 0;
        int64_t                    ll;
        double                     d;
        pn_error_code_t            err;
        ticks                      result{0};
        int                        sign = 1;

        for (pn::string_view::size_type i = 0; i < s.size(); ++i) {
            char ch = s.data()[i];
            switch (state) {
                case START:
                    if (isdigit(ch)) {
                        state = INT;
                        begin = i;
                    } else if (ch == '-') {
                        sign  = -1;
                        state = START_COMPONENT;
                    } else if (ch == '+') {
                        state = START_COMPONENT;
                    } else {
                        throw std::runtime_error(pn::format("expected digit ({0})", i).c_str());
                    }
                    break;

                case START_COMPONENT:
                case COMPONENT_DONE:
                    if (isdigit(ch)) {
                        state = INT;
                        begin = i;
                    } else {
                        throw std::runtime_error(pn::format("expected digit ({0})", i).c_str());
                    }
                    break;

                case DECIMAL:
                    if (isdigit(ch)) {
                        state = FLOAT;
                    } else {
                        throw std::runtime_error(pn::format("expected digit ({0})", i).c_str());
                    }
                    break;

                case INT:
                    if (ch == '.') {
                        state = DECIMAL;
                        break;
                    } else if (isdigit(ch)) {
                        break;
                    }

                    if (!pn::strtoll(s.substr(begin, i - begin), &ll, &err)) {
                        throw std::runtime_error(
                                pn::format("{0} ({1}:{2})", pn_strerror(err), begin, i).c_str());
                    }
                    switch (ch) {
                        case 'h': result += ticks{ll * 216000}; break;
                        case 'm': result += ticks{ll * 3600}; break;
                        case 's': result += ticks{ll * 60}; break;
                        case 't': result += ticks{ll * 1}; break;
                        default:
                            throw std::runtime_error(pn::format("expected unit ({0})", i).c_str());
                    }
                    state = COMPONENT_DONE;
                    break;

                case FLOAT:
                    if (isdigit(ch)) {
                        break;
                    }

                    if (!pn::strtod(s.substr(begin, i - begin), &d, &err)) {
                        throw std::runtime_error(
                                pn::format("{0} ({1}:{2})", pn_strerror(err), begin, i).c_str());
                    }
                    switch (ch) {
                        case 'h': result += ticks{int64_t(d * 216000)}; break;
                        case 'm': result += ticks{int64_t(d * 3600)}; break;
                        case 's': result += ticks{int64_t(d * 60)}; break;
                        case 't': result += ticks{int64_t(d * 1)}; break;
                        default:
                            throw std::runtime_error(pn::format("expected unit ({0})", i).c_str());
                    }
                    state = COMPONENT_DONE;
                    break;
            }
        }

        switch (state) {
            case COMPONENT_DONE: return sign * result;

            case START:
            case START_COMPONENT:
            case DECIMAL:
            case INT:
            case FLOAT: throw std::runtime_error("unexpected end of duration");
        }
    } catch (...) {
        if (x.is_root()) {
            throw;
        }
        std::throw_with_nested(std::runtime_error(x.path().c_str()));
    }
}

static secs parse_secs(path_value x, pn::string_view s) {
    ticks t  = parse_ticks(x, s);
    secs  ss = std::chrono::duration_cast<secs>(t);
    if (t != ss) {
        throw std::runtime_error(
                pn::format("{0}{1} is not an even number of seconds", x.prefix(), s).c_str());
    }
    return ss;
}

DEFINE_FIELD_READER(sfz::optional<ticks>) {
    auto s = read_field<sfz::optional<pn::string_view>>(x);
    if (s.has_value()) {
        return sfz::make_optional(parse_ticks(x, *s));
    } else {
        return sfz::nullopt;
    }
}

DEFINE_FIELD_READER(ticks) { return parse_ticks(x, read_field<pn::string_view>(x)); }

DEFINE_FIELD_READER(sfz::optional<secs>) {
    auto s = read_field<sfz::optional<pn::string_view>>(x);
    if (s.has_value()) {
        return sfz::make_optional(parse_secs(x, *s));
    } else {
        return sfz::nullopt;
    }
}

DEFINE_FIELD_READER(Tags) {
    if (x.value().is_null()) {
        return {};
    } else if (x.value().is_map()) {
        pn::map_cref m = x.value().as_map();
        Tags         result;
        for (const auto& kv : m) {
            auto v = read_field<sfz::optional<bool>>(x.get(kv.key()));
            if (v.has_value()) {
                result.tags[kv.key().copy()] = *v;
            }
        }
        return result;
    } else {
        throw std::runtime_error(pn::format("{0}must be null or map", x.prefix()).c_str());
    }
}

DEFINE_FIELD_READER(sfz::optional<Handle<Admiral>>) {
    auto i = read_field<sfz::optional<int64_t>>(x);
    if (i.has_value()) {
        return sfz::make_optional(Handle<Admiral>(*i));
    } else {
        return sfz::nullopt;
    }
}

DEFINE_FIELD_READER(Handle<Admiral>) { return Handle<Admiral>(read_field<int64_t>(x)); }

DEFINE_FIELD_READER(sfz::optional<NamedHandle<const BaseObject>>) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_int()) {
        return sfz::make_optional(NamedHandle<const BaseObject>(x.value().as_string()));
    } else {
        throw std::runtime_error(pn::format("{0}must be null or int", x.prefix()).c_str());
    }
}

DEFINE_FIELD_READER(NamedHandle<const BaseObject>) {
    return NamedHandle<const BaseObject>(read_field<pn::string_view>(x));
}

DEFINE_FIELD_READER(sfz::optional<Handle<const Initial>>) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_int()) {
        return sfz::make_optional(Handle<const Initial>(x.value().as_int()));
    } else {
        throw std::runtime_error(pn::format("{0}must be null or int", x.prefix()).c_str());
    }
}

DEFINE_FIELD_READER(Handle<const Initial>) {
    if (x.value().is_int()) {
        return Handle<const Initial>(x.value().as_int());
    } else {
        throw std::runtime_error(pn::format("{0}must be int", x.prefix()).c_str());
    }
}

DEFINE_FIELD_READER(sfz::optional<Handle<const Condition>>) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_int()) {
        return sfz::make_optional(Handle<const Condition>(x.value().as_int()));
    } else {
        throw std::runtime_error(pn::format("{0}must be null or int", x.prefix()).c_str());
    }
}

DEFINE_FIELD_READER(Handle<const Condition>) {
    return Handle<const Condition>(read_field<int64_t>(x));
}

DEFINE_FIELD_READER(sfz::optional<NamedHandle<const Level>>) {
    auto s = read_field<sfz::optional<pn::string_view>>(x);
    if (s.has_value()) {
        return sfz::make_optional(NamedHandle<const Level>(*s));
    } else {
        return sfz::nullopt;
    }
}

DEFINE_FIELD_READER(sfz::optional<Owner>) {
    return optional_enum<Owner>(
            x, {{"any", Owner::ANY}, {"same", Owner::SAME}, {"different", Owner::DIFFERENT}});
}

DEFINE_FIELD_READER(NamedHandle<const Race>) {
    return NamedHandle<const Race>(read_field<pn::string_view>(x));
}

DEFINE_FIELD_READER(Owner) {
    return required_enum<Owner>(
            x, {{"any", Owner::ANY}, {"same", Owner::SAME}, {"different", Owner::DIFFERENT}});
}

template <typename T>
std::pair<T, T> range(path_value x) {
    struct Pair {
        T begin, end;
    };
    Pair p = required_struct<Pair>(x, {{"begin", &Pair::begin}, {"end", &Pair::end}});
    return {p.begin, p.end};
}

DEFINE_FIELD_READER(sfz::optional<Range<int64_t>>) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_int()) {
        int64_t begin = x.value().as_int();
        return sfz::make_optional(Range<int64_t>{begin, begin + 1});
    } else if (x.value().is_map()) {
        auto r = range<int64_t>(x);
        return sfz::make_optional(Range<int64_t>{r.first, r.second});
    } else {
        throw std::runtime_error(pn::format("{0}must be null, int, or map", x.prefix()).c_str());
    }
}

DEFINE_FIELD_READER(Range<int64_t>) {
    if (x.value().is_int()) {
        int64_t begin = x.value().as_int();
        return Range<int64_t>{begin, begin + 1};
    } else if (x.value().is_map()) {
        auto r = range<int64_t>(x);
        return {r.first, r.second};
    } else {
        throw std::runtime_error(pn::format("{0}must be int or map", x.prefix()).c_str());
    }
}

DEFINE_FIELD_READER(sfz::optional<Range<Fixed>>) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_float()) {
        Fixed begin = Fixed::from_float(x.value().as_float());
        Fixed end   = Fixed::from_val(begin.val() + 1);
        return sfz::make_optional(Range<Fixed>{begin, end});
    } else if (x.value().is_map()) {
        auto r = range<Fixed>(x);
        return sfz::make_optional(Range<Fixed>{r.first, r.second});
    } else {
        throw std::runtime_error(pn::format("{0}must be float or map", x.prefix()).c_str());
    }
}

DEFINE_FIELD_READER(Range<Fixed>) {
    if (x.value().is_float()) {
        Fixed begin = Fixed::from_float(x.value().as_float());
        Fixed end   = Fixed::from_val(begin.val() + 1);
        return {begin, end};
    } else if (x.value().is_map()) {
        auto r = range<Fixed>(x);
        return {r.first, r.second};
    } else {
        throw std::runtime_error(pn::format("{0}must be float or map", x.prefix()).c_str());
    }
}

DEFINE_FIELD_READER(sfz::optional<Range<ticks>>) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_string()) {
        ticks begin = read_field<ticks>(x);
        return sfz::make_optional(Range<ticks>{begin, begin + ticks{1}});
    } else if (x.value().is_map()) {
        auto r = range<ticks>(x);
        return sfz::make_optional(Range<ticks>{r.first, r.second});
    } else {
        throw std::runtime_error(pn::format("{0}must be null, string or map", x.prefix()).c_str());
    }
}

DEFINE_FIELD_READER(Range<ticks>) {
    if (x.value().is_string()) {
        ticks begin = read_field<ticks>(x);
        return Range<ticks>{begin, begin + ticks{1}};
    } else if (x.value().is_map()) {
        auto r = range<ticks>(x);
        return {r.first, r.second};
    } else {
        throw std::runtime_error(pn::format("{0}must be string or map", x.prefix()).c_str());
    }
}

DEFINE_FIELD_READER(int32_t) { return int_field_within(x, {-0x80000000ll, 0x80000000ll}); }

DEFINE_FIELD_READER(sfz::optional<Point>) {
    return optional_struct<Point>(x, {{"x", &Point::h}, {"y", &Point::v}});
}

DEFINE_FIELD_READER(Point) {
    return required_struct<Point>(x, {{"x", &Point::h}, {"y", &Point::v}});
}

DEFINE_FIELD_READER(sfz::optional<Rect>) {
    return optional_struct<Rect>(
            x, {{"left", &Rect::left},
                {"top", &Rect::top},
                {"right", &Rect::right},
                {"bottom", &Rect::bottom}});
}

DEFINE_FIELD_READER(Rect) {
    return required_struct<Rect>(
            x, {{"left", &Rect::left},
                {"top", &Rect::top},
                {"right", &Rect::right},
                {"bottom", &Rect::bottom}});
}

DEFINE_FIELD_READER(sfz::optional<RgbColor>) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_map() || x.value().is_string()) {
        return sfz::make_optional<RgbColor>(read_field<RgbColor>(x));
    } else {
        throw std::runtime_error(pn::format("{0}must be null or map", x.prefix()).c_str());
    }
}

DEFINE_FIELD_READER(uint8_t) { return int_field_within(x, {0, 256}); }

static bool is_rgb(pn::map_cref m) {
    return (m.size() == 3) && m.has("r") && m.has("g") && m.has("b");
}

static bool is_rgba(pn::map_cref m) {
    return (m.size() == 4) && m.has("r") && m.has("g") && m.has("b") && m.has("a");
}

static bool is_hue(pn::string_view s) {
    static const std::set<pn::string_view> kHues = {
            "red",         "orange",   "yellow", "blue", "green", "purple",
            "indigo",      "salmon",   "gold",   "aqua", "pink",  "pale-green",
            "pale-purple", "sky-blue", "tan",    "gray"};
    return kHues.find(s) != kHues.end();
}

static bool is_hue_shade(pn::map_cref m) { return (m.size() == 1) && is_hue(m.begin()->key()); }

DEFINE_FIELD_READER(RgbColor) {
    if (x.value().is_map()) {
        const pn::map_cref m = x.value().as_map();
        if (is_rgba(m)) {
            return rgba(
                    read_field<uint8_t>(m.get("r")), read_field<uint8_t>(m.get("g")),
                    read_field<uint8_t>(m.get("b")), read_field<uint8_t>(m.get("a")));
        } else if (is_rgb(m)) {
            return rgb(
                    read_field<uint8_t>(m.get("r")), read_field<uint8_t>(m.get("g")),
                    read_field<uint8_t>(m.get("b")));
        } else if (is_hue_shade(m)) {
            pn::string_view hue = m.begin()->key();
            return RgbColor::tint(
                    read_field<Hue>(path_value{pn::value{hue.copy()}}),
                    read_field<uint8_t>(m.get(hue)));
        }
    } else if (x.value().is_string()) {
        const pn::string_view s = x.value().as_string();
        if (s == "white") {
            return RgbColor::white();
        } else if (s == "black") {
            return RgbColor::black();
        } else if (s == "clear") {
            return RgbColor::clear();
        }
        // TODO(sfiera): hex colors.
    }
    throw std::runtime_error(pn::format("{0}must be color", x.prefix()).c_str());
}

DEFINE_FIELD_READER(sfz::optional<Hue>) {
    return optional_enum<Hue>(
            x, {{"red", Hue::RED},
                {"orange", Hue::ORANGE},
                {"yellow", Hue::YELLOW},
                {"blue", Hue::BLUE},
                {"green", Hue::GREEN},
                {"purple", Hue::PURPLE},
                {"indigo", Hue::INDIGO},
                {"salmon", Hue::SALMON},
                {"gold", Hue::GOLD},
                {"aqua", Hue::AQUA},
                {"pink", Hue::PINK},
                {"pale-green", Hue::PALE_GREEN},
                {"pale-purple", Hue::PALE_PURPLE},
                {"sky-blue", Hue::SKY_BLUE},
                {"tan", Hue::TAN},
                {"gray", Hue::GRAY}});
}

DEFINE_FIELD_READER(Hue) {
    return required_enum<Hue>(
            x, {{"red", Hue::RED},
                {"orange", Hue::ORANGE},
                {"yellow", Hue::YELLOW},
                {"blue", Hue::BLUE},
                {"green", Hue::GREEN},
                {"purple", Hue::PURPLE},
                {"indigo", Hue::INDIGO},
                {"salmon", Hue::SALMON},
                {"gold", Hue::GOLD},
                {"aqua", Hue::AQUA},
                {"pink", Hue::PINK},
                {"pale-green", Hue::PALE_GREEN},
                {"pale-purple", Hue::PALE_PURPLE},
                {"sky-blue", Hue::SKY_BLUE},
                {"tan", Hue::TAN},
                {"gray", Hue::GRAY}});
}

DEFINE_FIELD_READER(Screen) {
    return required_enum<Screen>(
            x, {{"main", Screen::MAIN},
                {"build", Screen::BUILD},
                {"special", Screen::SPECIAL},
                {"message", Screen::MESSAGE},
                {"status", Screen::STATUS}});
}

DEFINE_FIELD_READER(Zoom) {
    return required_enum<Zoom>(
            x, {{"2:1", Zoom::DOUBLE},
                {"1:1", Zoom::ACTUAL},
                {"1:2", Zoom::HALF},
                {"1:4", Zoom::QUARTER},
                {"1:16", Zoom::SIXTEENTH},
                {"foe", Zoom::FOE},
                {"object", Zoom::OBJECT},
                {"all", Zoom::ALL}});
}

}  // namespace antares

#include <pn/fwd>
