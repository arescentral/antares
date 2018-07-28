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

sfz::optional<bool> optional_bool(path_value x) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_bool()) {
        return sfz::make_optional(x.value().as_bool());
    } else {
        throw std::runtime_error(pn::format("{0}: must be null or bool", x.path()).c_str());
    }
}

bool required_bool(path_value x) {
    if (x.value().is_bool()) {
        return x.value().as_bool();
    } else {
        throw std::runtime_error(pn::format("{0}: must be bool", x.path()).c_str());
    }
}

sfz::optional<int64_t> optional_int(path_value x) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_int()) {
        return sfz::make_optional(x.value().as_int());
    } else {
        throw std::runtime_error(pn::format("{0}: must be null or int", x.path()).c_str());
    }
}

int64_t required_int(path_value x) {
    if (x.value().is_int()) {
        return x.value().as_int();
    } else {
        throw std::runtime_error(pn::format("{0}: must be int", x.path()).c_str());
    }
}

// 0 {} => false
// -1 {0} => false
// 0 {0} => true
// 1 {0} => true
// 0 {1, 4} => false
// 2 {1, 4} => true
// 4 {1, 4} => false
static void check_ranges(path_value x, int64_t i, const std::initializer_list<int64_t>& ranges) {
    bool ok = false;
    for (int64_t x : ranges) {
        if (i >= x) {
            ok = !ok;
        } else {
            break;
        }
    }
    if (!ok) {
        pn::string err      = pn::format("{0}: must satisfy", x.path());
        bool       is_first = true;
        bool       is_begin = true;
        for (int64_t x : ranges) {
            if (is_first) {
                err += pn::format(" ({0} <= x", x);
            } else if (is_begin) {
                err += pn::format(" or ({0} <= x", x);
            } else {
                err += pn::format(" < {0})", x);
            }
            is_begin = !is_begin;
            is_first = false;
        }
        if (!is_begin) {
            err += ")";
        }
        throw std::runtime_error(err.c_str());
    }
}

sfz::optional<int64_t> optional_int(path_value x, const std::initializer_list<int64_t>& ranges) {
    auto i = read_field<sfz::optional<int64_t>>(x);
    if (i.has_value()) {
        check_ranges(x, *i, ranges);
    }
    return i;
}

int64_t required_int(path_value x, const std::initializer_list<int64_t>& ranges) {
    auto i = read_field<int64_t>(x);
    check_ranges(x, i, ranges);
    return i;
}

double required_double(path_value x) {
    if (x.value().is_number()) {
        return x.value().as_number();
    } else {
        throw std::runtime_error(pn::format("{0}: must be number", x.path()).c_str());
    }
}

sfz::optional<Fixed> optional_fixed(path_value x) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_number()) {
        return sfz::make_optional(Fixed::from_float(x.value().as_number()));
    } else {
        throw std::runtime_error(pn::format("{0}: must be null or number", x.path()).c_str());
    }
}

Fixed required_fixed(path_value x) {
    if (x.value().is_number()) {
        return Fixed::from_float(x.value().as_number());
    } else {
        throw std::runtime_error(pn::format("{0}: must be number", x.path()).c_str());
    }
}

sfz::optional<pn::string_view> optional_string(path_value x) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_string()) {
        return sfz::make_optional(x.value().as_string());
    } else {
        throw std::runtime_error(pn::format("{0}: must be null or string", x.path()).c_str());
    }
}

sfz::optional<pn::string> optional_string_copy(path_value x) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_string()) {
        return sfz::make_optional(x.value().as_string().copy());
    } else {
        throw std::runtime_error(pn::format("{0}: must be null or string", x.path()).c_str());
    }
}

pn::string_view required_string(path_value x) {
    if (x.value().is_string()) {
        return x.value().as_string();
    } else {
        throw std::runtime_error(pn::format("{0}: must be string", x.path()).c_str());
    }
}

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
        std::throw_with_nested(std::runtime_error(x.path().c_str()));
    }
}

static secs parse_secs(path_value x, pn::string_view s) {
    ticks t  = parse_ticks(x, s);
    secs  ss = std::chrono::duration_cast<secs>(t);
    if (t != ss) {
        throw std::runtime_error(
                pn::format("{0}: {1} is not an even number of seconds", x.path(), s).c_str());
    }
    return ss;
}

sfz::optional<ticks> optional_ticks(path_value x) {
    auto s = read_field<sfz::optional<pn::string_view>>(x);
    if (s.has_value()) {
        return sfz::make_optional(parse_ticks(x, *s));
    } else {
        return sfz::nullopt;
    }
}

ticks required_ticks(path_value x) { return parse_ticks(x, read_field<pn::string_view>(x)); }

sfz::optional<secs> optional_secs(path_value x) {
    auto s = read_field<sfz::optional<pn::string_view>>(x);
    if (s.has_value()) {
        return sfz::make_optional(parse_secs(x, *s));
    } else {
        return sfz::nullopt;
    }
}

Tags optional_tags(path_value x) {
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
        throw std::runtime_error(pn::format("{0}: must be null or map", x.path()).c_str());
    }
}

sfz::optional<Handle<Admiral>> optional_admiral(path_value x) {
    auto i = read_field<sfz::optional<int64_t>>(x);
    if (i.has_value()) {
        return sfz::make_optional(Handle<Admiral>(*i));
    } else {
        return sfz::nullopt;
    }
}

Handle<Admiral> required_admiral(path_value x) { return Handle<Admiral>(read_field<int64_t>(x)); }

NamedHandle<const BaseObject> required_base(path_value x) {
    return NamedHandle<const BaseObject>(read_field<pn::string_view>(x));
}

sfz::optional<Handle<const Initial>> optional_initial(path_value x) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_int()) {
        return sfz::make_optional(Handle<const Initial>(x.value().as_int()));
    } else {
        throw std::runtime_error(pn::format("{0}: must be null or int", x.path()).c_str());
    }
}

Handle<const Initial> required_initial(path_value x) {
    if (x.value().is_int()) {
        return Handle<const Initial>(x.value().as_int());
    } else {
        throw std::runtime_error(pn::format("{0}: must be int", x.path()).c_str());
    }
}

Handle<const Condition> required_condition(path_value x) {
    return Handle<const Condition>(read_field<int64_t>(x));
}

sfz::optional<NamedHandle<const Level>> optional_level(path_value x) {
    auto s = read_field<sfz::optional<pn::string_view>>(x);
    if (s.has_value()) {
        return sfz::make_optional(NamedHandle<const Level>(*s));
    } else {
        return sfz::nullopt;
    }
}

sfz::optional<Owner> optional_owner(path_value x) {
    return optional_enum<Owner>(
            x, {{"any", Owner::ANY}, {"same", Owner::SAME}, {"different", Owner::DIFFERENT}});
}

NamedHandle<const Race> required_race(path_value x) {
    return NamedHandle<const Race>(read_field<pn::string_view>(x));
}

Owner required_owner(path_value x) {
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

sfz::optional<Range<int64_t>> optional_int_range(path_value x) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_int()) {
        int64_t begin = x.value().as_int();
        return sfz::make_optional(Range<int64_t>{begin, begin + 1});
    } else if (x.value().is_map()) {
        auto r = range<int64_t>(x);
        return sfz::make_optional(Range<int64_t>{r.first, r.second});
    } else {
        throw std::runtime_error(pn::format("{0}: must be null, int, or map", x.path()).c_str());
    }
}

Range<int64_t> required_int_range(path_value x) {
    if (x.value().is_int()) {
        int64_t begin = x.value().as_int();
        return Range<int64_t>{begin, begin + 1};
    } else if (x.value().is_map()) {
        auto r = range<int64_t>(x);
        return {r.first, r.second};
    } else {
        throw std::runtime_error(pn::format("{0}: must be int or map", x.path()).c_str());
    }
}

sfz::optional<Range<Fixed>> optional_fixed_range(path_value x) {
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
        throw std::runtime_error(pn::format("{0}: must be float or map", x.path()).c_str());
    }
}

Range<Fixed> required_fixed_range(path_value x) {
    if (x.value().is_float()) {
        Fixed begin = Fixed::from_float(x.value().as_float());
        Fixed end   = Fixed::from_val(begin.val() + 1);
        return {begin, end};
    } else if (x.value().is_map()) {
        auto r = range<Fixed>(x);
        return {r.first, r.second};
    } else {
        throw std::runtime_error(pn::format("{0}: must be float or map", x.path()).c_str());
    }
}

sfz::optional<Range<ticks>> optional_ticks_range(path_value x) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_string()) {
        ticks begin = read_field<ticks>(x);
        return sfz::make_optional(Range<ticks>{begin, begin + ticks{1}});
    } else if (x.value().is_map()) {
        auto r = range<ticks>(x);
        return sfz::make_optional(Range<ticks>{r.first, r.second});
    } else {
        throw std::runtime_error(pn::format("{0}: must be null, string or map", x.path()).c_str());
    }
}

Range<ticks> required_ticks_range(path_value x) {
    if (x.value().is_int()) {
        ticks begin = read_field<ticks>(x);
        return Range<ticks>{begin, begin + ticks{1}};
    } else if (x.value().is_map()) {
        auto r = range<ticks>(x);
        return {r.first, r.second};
    } else {
        throw std::runtime_error(pn::format("{0}: must be string or map", x.path()).c_str());
    }
}

static int32_t required_int32(path_value x) {
    return required_int(x, {-0x80000000ll, 0x80000000ll});
}
DEFAULT_READER(int32_t, required_int32);

sfz::optional<Point> optional_point(path_value x) {
    return optional_struct<Point>(x, {{"x", &Point::h}, {"y", &Point::v}});
}

Point required_point(path_value x) {
    return required_struct<Point>(x, {{"x", &Point::h}, {"y", &Point::v}});
}

sfz::optional<Rect> optional_rect(path_value x) {
    return optional_struct<Rect>(
            x, {{"left", &Rect::left},
                {"top", &Rect::top},
                {"right", &Rect::right},
                {"bottom", &Rect::bottom}});
}

Rect required_rect(path_value x) {
    return required_struct<Rect>(
            x, {{"left", &Rect::left},
                {"top", &Rect::top},
                {"right", &Rect::right},
                {"bottom", &Rect::bottom}});
}

sfz::optional<RgbColor> optional_color(path_value x) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_map()) {
        return sfz::make_optional<RgbColor>(read_field<RgbColor>(x));
    } else {
        throw std::runtime_error(pn::format("{0}: must be null or map", x.path()).c_str());
    }
}

static uint8_t required_uint8(path_value x) { return required_int(x, {0, 256}); }
DEFAULT_READER(uint8_t, required_uint8);

static sfz::optional<uint8_t> optional_uint8(path_value x) {
    auto i = optional_int(x, {0, 256});
    return i.has_value() ? sfz::make_optional<uint8_t>(*i) : sfz::nullopt;
}
DEFAULT_READER(sfz::optional<uint8_t>, optional_uint8);

static bool is_rgb(pn::map_cref m) {
    return (m.size() == 3) && m.has("r") && m.has("g") && m.has("b");
}

static bool is_rgba(pn::map_cref m) {
    return (m.size() == 4) && m.has("r") && m.has("g") && m.has("b") && m.has("a");
}

RgbColor required_color(path_value x) {
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

sfz::optional<Hue> optional_hue(path_value x) {
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
                {"pale green", Hue::PALE_GREEN},
                {"pale purple", Hue::PALE_PURPLE},
                {"sky blue", Hue::SKY_BLUE},
                {"tan", Hue::TAN},
                {"gray", Hue::GRAY}});
}

Hue required_hue(path_value x) {
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
                {"pale green", Hue::PALE_GREEN},
                {"pale purple", Hue::PALE_PURPLE},
                {"sky blue", Hue::SKY_BLUE},
                {"tan", Hue::TAN},
                {"gray", Hue::GRAY}});
}

Screen required_screen(path_value x) {
    return required_enum<Screen>(
            x, {{"main", Screen::MAIN},
                {"build", Screen::BUILD},
                {"special", Screen::SPECIAL},
                {"message", Screen::MESSAGE},
                {"status", Screen::STATUS}});
}

Zoom required_zoom(path_value x) {
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

pn::string required_string_copy(path_value x) { return read_field<pn::string_view>(x).copy(); }

}  // namespace antares

#include <pn/fwd>
