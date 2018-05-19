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
    auto i = optional_int(x);
    if (i.has_value()) {
        check_ranges(x, *i, ranges);
    }
    return i;
}

int64_t required_int(path_value x, const std::initializer_list<int64_t>& ranges) {
    auto i = required_int(x);
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
    throw std::runtime_error(pn::format("{0}: must be one of {1}", x.path(), keys).c_str());
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
    sfz::optional<pn::string_view> s = optional_string(x);
    if (s.has_value()) {
        return sfz::make_optional(parse_ticks(x, *s));
    } else {
        return sfz::nullopt;
    }
}

ticks required_ticks(path_value x) { return parse_ticks(x, required_string(x)); }

sfz::optional<secs> optional_secs(path_value x) {
    sfz::optional<pn::string_view> s = optional_string(x);
    if (s.has_value()) {
        return sfz::make_optional(parse_secs(x, *s));
    } else {
        return sfz::nullopt;
    }
}

sfz::optional<Handle<Admiral>> optional_admiral(path_value x) {
    sfz::optional<int64_t> i = optional_int(x);
    if (i.has_value()) {
        return sfz::make_optional(Handle<Admiral>(*i));
    } else {
        return sfz::nullopt;
    }
}

Handle<Admiral> required_admiral(path_value x) { return Handle<Admiral>(required_int(x)); }

NamedHandle<const BaseObject> required_base(path_value x) {
    return NamedHandle<const BaseObject>(required_string(x));
}

sfz::optional<Handle<const Initial>> optional_initial(path_value x) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_int()) {
        return sfz::make_optional(Handle<const Initial>(x.value().as_int()));
    } else if (x.value().as_string() == "player") {
        return sfz::make_optional(Handle<const Initial>(-2));
    } else {
        throw std::runtime_error(
                pn::format("{0}: must be null, int, or \"player\"", x.path()).c_str());
    }
}

Handle<const Initial> required_initial(path_value x) {
    if (x.value().is_int()) {
        return Handle<const Initial>(x.value().as_int());
    } else if (x.value().as_string() == "player") {
        return Handle<const Initial>(-2);
    } else {
        throw std::runtime_error(pn::format("{0}: must be int, or \"player\"", x.path()).c_str());
    }
}

sfz::optional<NamedHandle<const Level>> optional_level(path_value x) {
    sfz::optional<pn::string_view> s = optional_string(x);
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
    return NamedHandle<const Race>(required_string(x));
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
    throw std::runtime_error(pn::format("{0}: must be one of {1}", x.path(), keys).c_str());
}

Owner required_owner(path_value x) {
    return required_enum<Owner>(
            x, {{"any", Owner::ANY}, {"same", Owner::SAME}, {"different", Owner::DIFFERENT}});
}

template <typename T, T (*F)(path_value)>
std::pair<T, T> range(path_value x) {
    struct Pair {
        T begin, end;
    };
    Pair p = required_struct<Pair>(x, {{"begin", {&Pair::begin, F}}, {"end", {&Pair::end, F}}});
    return {p.begin, p.end};
}

sfz::optional<Range<int64_t>> optional_int_range(path_value x) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_int()) {
        int64_t begin = x.value().as_int();
        return sfz::make_optional(Range<int64_t>{begin, begin + 1});
    } else if (x.value().is_map()) {
        auto r = range<int64_t, required_int>(x);
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
        auto r = range<int64_t, required_int>(x);
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
        auto r = range<Fixed, required_fixed>(x);
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
        auto r = range<Fixed, required_fixed>(x);
        return {r.first, r.second};
    } else {
        throw std::runtime_error(pn::format("{0}: must be float or map", x.path()).c_str());
    }
}

sfz::optional<Range<ticks>> optional_ticks_range(path_value x) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_string()) {
        ticks begin = required_ticks(x);
        return sfz::make_optional(Range<ticks>{begin, begin + ticks{1}});
    } else if (x.value().is_map()) {
        auto r = range<ticks, required_ticks>(x);
        return sfz::make_optional(Range<ticks>{r.first, r.second});
    } else {
        throw std::runtime_error(pn::format("{0}: must be null, string or map", x.path()).c_str());
    }
}

Range<ticks> required_ticks_range(path_value x) {
    if (x.value().is_int()) {
        ticks begin = required_ticks(x);
        return Range<ticks>{begin, begin + ticks{1}};
    } else if (x.value().is_map()) {
        auto r = range<ticks, required_ticks>(x);
        return {r.first, r.second};
    } else {
        throw std::runtime_error(pn::format("{0}: must be string or map", x.path()).c_str());
    }
}

HandleList<const Condition> optional_condition_range(path_value x) {
    auto range = optional_int_range(x);
    if (range.has_value()) {
        return HandleList<const Condition>(range->begin, range->end);
    } else {
        return {-1, -1};
    }
}

HandleList<const Initial> required_initial_range(path_value x) {
    auto range = required_int_range(x);
    return HandleList<const Initial>(range.begin, range.end);
}

static int32_t required_int32(path_value x) {
    return required_int(x, {-0x80000000ll, 0x80000000ll});
}

sfz::optional<Point> optional_point(path_value x) {
    return optional_struct<Point>(
            x, {{"x", {&Point::h, required_int32}}, {"y", {&Point::v, required_int32}}});
}

Point required_point(path_value x) {
    return required_struct<Point>(
            x, {{"x", {&Point::h, required_int32}}, {"y", {&Point::v, required_int32}}});
}

sfz::optional<Rect> optional_rect(path_value x) {
    return optional_struct<Rect>(
            x, {
                       {"left", {&Rect::left, required_int32}},
                       {"top", {&Rect::top, required_int32}},
                       {"right", {&Rect::right, required_int32}},
                       {"bottom", {&Rect::bottom, required_int32}},
               });
}

Rect required_rect(path_value x) {
    return required_struct<Rect>(
            x, {
                       {"left", {&Rect::left, required_int32}},
                       {"top", {&Rect::top, required_int32}},
                       {"right", {&Rect::right, required_int32}},
                       {"bottom", {&Rect::bottom, required_int32}},
               });
}

sfz::optional<RgbColor> optional_color(path_value x) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_map()) {
        return sfz::make_optional<RgbColor>(required_color(x));
    } else {
        throw std::runtime_error(pn::format("{0}: must be null or map", x.path()).c_str());
    }
}

static uint8_t                required_uint8(path_value x) { return required_int(x, {0, 256}); }
static sfz::optional<uint8_t> optional_uint8(path_value x) {
    auto i = optional_int(x, {0, 256});
    return i.has_value() ? sfz::make_optional<uint8_t>(*i) : sfz::nullopt;
}

RgbColor required_color(path_value x) {
    return required_struct<RgbColor>(
            x, {{"r", {&RgbColor::red, required_uint8}},
                {"g", {&RgbColor::green, required_uint8}},
                {"b", {&RgbColor::blue, required_uint8}},
                {"a", {&RgbColor::alpha, optional_uint8, 255}}});
}

AnimationDirection required_animation_direction(path_value x) {
    return required_enum<AnimationDirection>(
            x, {{"0", AnimationDirection::NONE},
                {"+", AnimationDirection::PLUS},
                {"-", AnimationDirection::MINUS},
                {"?", AnimationDirection::RANDOM}});
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

InterfaceStyle required_interface_style(path_value x) {
    return required_enum<InterfaceStyle>(
            x, {{"small", InterfaceStyle::SMALL}, {"large", InterfaceStyle::LARGE}});
}

KillKind required_kill_kind(path_value x) {
    return required_enum<KillKind>(
            x, {{"none", KillKind::NONE},
                {"expire", KillKind::EXPIRE},
                {"destroy", KillKind::DESTROY}});
}

sfz::optional<MoveOrigin> optional_origin(path_value x) {
    return optional_enum<MoveOrigin>(
            x, {{"level", MoveOrigin::LEVEL},
                {"subject", MoveOrigin::SUBJECT},
                {"object", MoveOrigin::OBJECT}});
}

int required_key(path_value x) {
    return required_enum<int>(
            x, {{"up", 0},
                {"down", 1},
                {"left", 2},
                {"right", 3},
                {"fire_1", 4},
                {"fire_2", 5},
                {"fire_s", 6},
                {"warp", 0},
                {"select_friend", 8},
                {"select_foe", 9},
                {"select_base", 10},
                {"target", 11},
                {"order", 12},
                {"zoom_in", 13},
                {"zoom_out", 14},
                {"comp_up", 15},
                {"comp_down", 16},
                {"comp_accept", 17},
                {"comp_back", 18},

                {"comp_message", 26},
                {"comp_special", 27},
                {"comp_build", 28},
                {"zoom_shortcut", 29},
                {"send_message", 30},
                {"mouse", 31}});
}

ConditionOp required_condition_op(path_value x) {
    return required_enum<ConditionOp>(
            x, {{"eq", ConditionOp::EQ},
                {"ne", ConditionOp::NE},
                {"lt", ConditionOp::LT},
                {"gt", ConditionOp::GT},
                {"le", ConditionOp::LE},
                {"ge", ConditionOp::GE}});
}

IconShape required_icon_shape(path_value x) {
    return required_enum<IconShape>(
            x, {{"square", IconShape::SQUARE},
                {"triangle", IconShape::TRIANGLE},
                {"diamond", IconShape::DIAMOND},
                {"plus", IconShape::PLUS}});
}

LevelType required_level_type(path_value x) {
    return required_enum<LevelType>(
            x, {{"solo", LevelType::SOLO}, {"net", LevelType::NET}, {"demo", LevelType::DEMO}});
}

PlayerType required_player_type(path_value x) {
    return required_enum<PlayerType>(x, {{"human", PlayerType::HUMAN}, {"cpu", PlayerType::CPU}});
}

PushKind required_push_kind(path_value x) {
    return required_enum<PushKind>(
            x, {{"stop", PushKind::STOP},
                {"collide", PushKind::COLLIDE},
                {"decelerate", PushKind::DECELERATE},
                {"boost", PushKind::BOOST},
                {"set", PushKind::SET},
                {"cruise", PushKind::CRUISE}});
}

Screen required_screen(path_value x) {
    return required_enum<Screen>(
            x, {{"main", Screen::MAIN},
                {"build", Screen::BUILD},
                {"special", Screen::SPECIAL},
                {"message", Screen::MESSAGE},
                {"status", Screen::STATUS}});
}

SubjectValue required_subject_value(path_value x) {
    return required_enum<SubjectValue>(
            x, {{"control", SubjectValue::CONTROL},
                {"target", SubjectValue::TARGET},
                {"player", SubjectValue::PLAYER}});
}

Weapon required_weapon(path_value x) {
    return required_enum<Weapon>(
            x, {{"pulse", Weapon::PULSE}, {"beam", Weapon::BEAM}, {"special", Weapon::SPECIAL}});
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

uint32_t optional_object_attributes(path_value x) {
    return optional_flags(
            x, {{"can_be_engaged", 1},
                {"does_bounce", 6},
                {"can_be_destination", 10},
                {"can_engage", 11},
                {"can_evade", 12},
                {"can_accept_build", 14},
                {"can_accept_destination", 15},
                {"autotarget", 16},
                {"animation_cycle", 17},
                {"can_collide", 18},
                {"can_be_hit", 19},
                {"is_destination", 20},
                {"hide_effect", 21},
                {"release_energy_on_death", 22},
                {"hated", 23},
                {"occupies_space", 24},
                {"static_destination", 25},
                {"can_be_evaded", 26},
                {"neutral_death", 27},
                {"is_guided", 28},
                {"appear_on_radar", 29}});
}

uint32_t optional_keys(path_value x) {
    if (x.value().is_null()) {
        return 0x00000000;
    } else if (x.value().is_array()) {
        pn::array_cref a      = x.value().as_array();
        uint32_t       result = 0x00000000;
        for (int i = 0; i < a.size(); ++i) {
            int key = required_key(x.get(i));
            result |= (0x1 << key);
        }
        return result;
    } else {
        throw std::runtime_error(pn::format("{0}: must be null or list", x.path()).c_str());
    }
}

std::vector<pn::string> required_string_array(path_value x) {
    if (x.value().is_array()) {
        pn::array_cref          a = x.value().as_array();
        std::vector<pn::string> result;
        for (int i = 0; i < a.size(); ++i) {
            result.emplace_back(required_string(x.get(i)).copy());
        }
        return result;
    } else {
        throw std::runtime_error(pn::format("{0}: must be array", x.path()).c_str());
    }
}

pn::string required_string_copy(path_value x) { return required_string(x).copy(); }

std::vector<pn::string> optional_string_array(path_value x) {
    if (x.value().is_null()) {
        return {};
    } else if (x.value().is_array()) {
        pn::array_cref          a = x.value().as_array();
        std::vector<pn::string> result;
        for (int i = 0; i < a.size(); ++i) {
            result.emplace_back(required_string(x.get(i)).copy());
        }
        return result;
    } else {
        throw std::runtime_error(pn::format("{0}: must be null or array", x.path()).c_str());
    }
}

std::vector<int> optional_int_array(path_value x) {
    if (x.value().is_null()) {
        return {};
    } else if (x.value().is_array()) {
        pn::array_cref   a = x.value().as_array();
        std::vector<int> result;
        for (int i = 0; i < a.size(); ++i) {
            result.emplace_back(required_int(x.get(i)));
        }
        return result;
    } else {
        throw std::runtime_error(pn::format("{0}: must be null or array", x.path()).c_str());
    }
}

uint32_t optional_flags(path_value x, const std::map<pn::string_view, int>& flags) {
    if (x.value().is_null()) {
        return 0;
    } else if (x.value().is_map()) {
        uint32_t result = 0;
        for (auto kv : flags) {
            if (optional_bool(x.get(kv.first)).value_or(false)) {
                result |= 1 << kv.second;
            }
        }
        for (auto kv : x.value().as_map()) {
            if (flags.find(kv.key()) == flags.end()) {
                path_value v = x.get(kv.key());
                throw std::runtime_error(pn::format("{0}unknown flag", v.prefix()).c_str());
            }
        }
        return result;
    } else {
        throw std::runtime_error(pn::format("{0}must be null or map", x.prefix()).c_str());
    }
}

}  // namespace antares

#include <pn/fwd>
