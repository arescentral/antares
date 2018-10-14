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

#include "ui/screens/briefing.hpp"

#include <pn/file>

#include "config/gamepad.hpp"
#include "config/keys.hpp"
#include "data/base-object.hpp"
#include "data/level.hpp"
#include "data/plugin.hpp"
#include "data/resource.hpp"
#include "drawing/briefing.hpp"
#include "drawing/color.hpp"
#include "drawing/interface.hpp"
#include "drawing/shapes.hpp"
#include "drawing/text.hpp"
#include "game/instruments.hpp"
#include "game/level.hpp"
#include "game/sys.hpp"
#include "math/random.hpp"
#include "ui/card.hpp"
#include "ui/interface-handling.hpp"
#include "ui/screens/object-data.hpp"
#include "video/driver.hpp"

using std::make_pair;
using std::pair;
using std::vector;

namespace antares {

namespace {

enum BriefingPoint {
    STAR_MAP         = -2,
    BLANK_SYSTEM_MAP = -1,
};

}  // namespace

static const Hue kMissionDataHiliteColor = Hue::GOLD;

static const int32_t kMissionDataWidth        = 200;
static const int32_t kMissionDataVBuffer      = 40;
static const int32_t kMissionDataTopBuffer    = 30;
static const int32_t kMissionDataBottomBuffer = 15;
static const int32_t kMissionDataHBuffer      = 41;
static const int32_t kMissionLineHJog         = 10;

static vector<inlinePictType> populate_inline_picts(
        Rect rect, pn::string_view text, InterfaceStyle style) {
    StyledText interface_text(interface_font(style));
    interface_text.set_interface_text(text);
    interface_text.wrap_to(rect.width(), kInterfaceTextHBuffer, kInterfaceTextVBuffer);
    std::vector<inlinePictType> result;
    for (const inlinePictType& pict : interface_text.inline_picts()) {
        result.emplace_back();
        result.back().bounds  = pict.bounds;
        result.back().object  = pict.object;
        result.back().picture = pict.picture.copy();
        result.back().bounds.offset(rect.left, rect.top);
    }
    return result;
}

static BoxRect update_mission_brief_point(
        int32_t whichBriefPoint, const Level& level, const coordPointType& corner, int32_t scale,
        const Rect& bounds, vector<inlinePictType>* inlinePict, Rect* highlight_rect,
        vector<pair<Point, Point>>* lines, pn::string_ref text) {
    if (whichBriefPoint < 0) {
        // No longer handled here.
        return BoxRect{{}};
    }

    BoxRectData data;
    data.hue   = Hue::GOLD;
    data.style = InterfaceStyle::LARGE;

    auto info = BriefPoint_Data_Get(whichBriefPoint, level, corner, scale, 32, bounds);
    text      = std::move(info.content);
    data.label.emplace(std::move(info.header));
    Rect hiliteBounds = info.highlight;
    hiliteBounds.offset(bounds.left, bounds.top);
    *inlinePict     = populate_inline_picts(data.bounds, text, data.style);
    *highlight_rect = Rect{};

    int16_t textHeight = GetInterfaceTextHeightFromWidth(text, data.style, kMissionDataWidth);

    if (hiliteBounds.empty()) {
        data.bounds = {Point{(bounds.width() / 2) - (kMissionDataWidth / 2) + bounds.left,
                             (bounds.height() / 2) - (textHeight / 2) + bounds.top},
                       Size{kMissionDataWidth, textHeight}};
    } else {
        if ((hiliteBounds.left + (hiliteBounds.right - hiliteBounds.left) / 2) >
            (bounds.left + (bounds.right - bounds.left) / 2)) {
            data.bounds.right = hiliteBounds.left - kMissionDataHBuffer;
            data.bounds.left  = data.bounds.right - kMissionDataWidth;
        } else {
            data.bounds.left  = hiliteBounds.right + kMissionDataHBuffer;
            data.bounds.right = data.bounds.left + kMissionDataWidth;
        }

        data.bounds.top =
                hiliteBounds.top + (hiliteBounds.bottom - hiliteBounds.top) / 2 - textHeight / 2;
        data.bounds.bottom = data.bounds.top + textHeight;
        if (data.bounds.top < (bounds.top + kMissionDataTopBuffer)) {
            data.bounds.top    = bounds.top + kMissionDataTopBuffer;
            data.bounds.bottom = data.bounds.top + textHeight;
        }
        if (data.bounds.bottom > (bounds.bottom - kMissionDataBottomBuffer)) {
            data.bounds.bottom = bounds.bottom - kMissionDataBottomBuffer;
            data.bounds.top    = data.bounds.bottom - textHeight;
        }

        if (data.bounds.left < (bounds.left + kMissionDataVBuffer)) {
            data.bounds.left  = bounds.left + kMissionDataVBuffer;
            data.bounds.right = data.bounds.left + kMissionDataWidth;
        }
        if (data.bounds.right > (bounds.right - kMissionDataVBuffer)) {
            data.bounds.right = bounds.right - kMissionDataVBuffer;
            data.bounds.left  = data.bounds.right - kMissionDataWidth;
        }
    }

    BoxRect box_rect{std::move(data)};
    lines->clear();
    if (!hiliteBounds.empty()) {
        hiliteBounds.right++;
        hiliteBounds.bottom++;
        *highlight_rect = hiliteBounds;
        Rect newRect    = box_rect.outer_bounds();
        if (data.bounds.right < hiliteBounds.left) {
            Point p1(hiliteBounds.left, hiliteBounds.top);
            Point p2(newRect.right + kMissionLineHJog, hiliteBounds.top);
            Point p3(newRect.right + kMissionLineHJog, newRect.top);
            Point p4(newRect.right + 2, newRect.top);
            lines->push_back(make_pair(p1, p2));
            lines->push_back(make_pair(p2, p3));
            lines->push_back(make_pair(p3, p4));

            Point p5(hiliteBounds.left, hiliteBounds.bottom - 1);
            Point p6(newRect.right + kMissionLineHJog, hiliteBounds.bottom - 1);
            Point p7(newRect.right + kMissionLineHJog, newRect.bottom - 1);
            Point p8(newRect.right + 2, newRect.bottom - 1);
            lines->push_back(make_pair(p5, p6));
            lines->push_back(make_pair(p6, p7));
            lines->push_back(make_pair(p7, p8));
        } else {
            Point p1(hiliteBounds.right, hiliteBounds.top);
            Point p2(newRect.left - kMissionLineHJog, hiliteBounds.top);
            Point p3(newRect.left - kMissionLineHJog, newRect.top);
            Point p4(newRect.left - 3, newRect.top);
            lines->push_back(make_pair(p1, p2));
            lines->push_back(make_pair(p2, p3));
            lines->push_back(make_pair(p3, p4));

            Point p5(hiliteBounds.right, hiliteBounds.bottom - 1);
            Point p6(newRect.left - kMissionLineHJog, hiliteBounds.bottom - 1);
            Point p7(newRect.left - kMissionLineHJog, newRect.bottom - 1);
            Point p8(newRect.left - 3, newRect.bottom - 1);
            lines->push_back(make_pair(p5, p6));
            lines->push_back(make_pair(p6, p7));
            lines->push_back(make_pair(p7, p8));
        }
    }
    return box_rect;
}

BriefingScreen::BriefingScreen(const Level& level, bool* cancelled)
        : InterfaceScreen("briefing", {0, 0, 640, 480}),
          _level(level),
          _cancelled(cancelled),
          _briefing_point(_level.base.starmap.has_value() ? STAR_MAP : BLANK_SYSTEM_MAP),
          _briefing_point_start(_briefing_point),
          _briefing_point_end(_level.base.briefings.size()),
          _data_item{{}} {
    build_star_map();
    for (int i = 0; i < 500; ++i) {
        Star star;
        star.shade      = Randomize(kVisibleShadeNum);
        star.location.h = _bounds.left + Randomize(_bounds.width());
        star.location.v = _bounds.top + Randomize(_bounds.height());
        _system_stars.push_back(star);
    }

    button(DONE)->bind({[this] { stack()->pop(this); }});

    button(PREVIOUS)->bind({
            [this] {
                if (_briefing_point > _briefing_point_start) {
                    --_briefing_point;
                }
                build_brief_point();
            },
            [this] { return _briefing_point > _briefing_point_start; },
    });

    button(NEXT)->bind({
            [this] {
                if (_briefing_point < _briefing_point_end - 1) {
                    ++_briefing_point;
                }
                build_brief_point();
            },
            [this] { return _briefing_point < _briefing_point_end - 1; },
    });
}

BriefingScreen::~BriefingScreen() {}

void BriefingScreen::become_front() {
    if (_briefing_point_end == 0) {
        stack()->pop(this);
    } else {
        InterfaceScreen::become_front();
    }
}

void BriefingScreen::overlay() const {
    switch (_briefing_point) {
        case STAR_MAP: {
            Point off       = offset();
            Rect  star_rect = _star_rect;
            star_rect.offset(off.h, off.v);
            const Point star   = star_rect.center();
            RgbColor    gold   = GetRGBTranslateColorShade(Hue::GOLD, LIGHTEST);
            Rect        bounds = _bounds;
            bounds.offset(off.h, off.v);
            plug.starmap.draw_cropped(bounds, Rect(Point(0, 2), bounds.size()));
            Rects rects;
            draw_vbracket(rects, star_rect, gold);
            rects.fill({star.h, bounds.top, star.h + 1, star_rect.top + 1}, gold);
            rects.fill({star.h, star_rect.bottom, star.h + 1, bounds.bottom + 1}, gold);
            rects.fill({bounds.left, star.v, star_rect.left + 1, star.v + 1}, gold);
            rects.fill({star_rect.right - 1, star.v, bounds.right + 1, star.v + 1}, gold);
        } break;

        case BLANK_SYSTEM_MAP: draw_system_map(); break;

        default: draw_brief_point(); break;
    }
}

void BriefingScreen::mouse_down(const MouseDownEvent& event) {
    Point off   = offset();
    Point where = event.where();
    where.offset(-off.h, -off.v);
    for (const auto& pict : _inline_pict) {
        if (pict.bounds.contains(where) && pict.object) {
            stack()->push(new ObjectDataScreen(
                    event.where(), *pict.object, ObjectDataScreen::MOUSE, event.button(),
                    Key::NONE, Gamepad::Button::NONE));
            return;
        }
    }
    InterfaceScreen::mouse_down(event);
}

void BriefingScreen::key_down(const KeyDownEvent& event) {
    switch (event.key()) {
        case Key::ESCAPE: {
            *_cancelled = true;
            stack()->pop(this);
        }
            return;
        case Key::K1: return show_object_data(0, event);
        case Key::K2: return show_object_data(1, event);
        case Key::K3: return show_object_data(2, event);
        case Key::K4: return show_object_data(3, event);
        case Key::K5: return show_object_data(4, event);
        case Key::K6: return show_object_data(5, event);
        case Key::K7: return show_object_data(6, event);
        case Key::K8: return show_object_data(7, event);
        case Key::K9: return show_object_data(8, event);
        case Key::K0: return show_object_data(9, event);
        default: { return InterfaceScreen::key_down(event); }
    }
}

void BriefingScreen::gamepad_button_down(const GamepadButtonDownEvent& event) {
    switch (event.button) {
        case Gamepad::Button::B: {
            *_cancelled = true;
            stack()->pop(this);
        }
            return;
        case Gamepad::Button::UP: return show_object_data(0, event);
        case Gamepad::Button::DOWN: return show_object_data(1, event);
        default: { return InterfaceScreen::gamepad_button_down(event); }
    }
}

void BriefingScreen::build_star_map() {
    Rect pix_bounds = plug.starmap.size().as_rect();
    pix_bounds.offset(0, 2);
    pix_bounds.bottom -= 3;
    _bounds = pix_bounds;
    _bounds.center_in(widget(MAP_RECT)->inner_bounds());

    if (_level.base.starmap.has_value()) {
        _star_rect = *_level.base.starmap;

        // Move `_star_rect` so that it is inside of `pix_bounds`.
        if (_star_rect.left < pix_bounds.left) {
            _star_rect.offset(pix_bounds.left - _star_rect.left, 0);
        } else if (_star_rect.right > pix_bounds.right) {
            _star_rect.offset(pix_bounds.right - _star_rect.right, 0);
        }
        if (_star_rect.top < pix_bounds.top) {
            _star_rect.offset(0, pix_bounds.top - _star_rect.top);
        } else if (_star_rect.bottom > pix_bounds.bottom) {
            _star_rect.offset(0, pix_bounds.bottom - _star_rect.bottom);
        }
        _star_rect.offset(_bounds.left, _bounds.top);
    }
}

void BriefingScreen::build_brief_point() {
    if (_briefing_point >= 0) {
        coordPointType corner;
        int32_t        scale;
        Rect           map_rect = widget(MAP_RECT)->inner_bounds();
        GetLevelFullScaleAndCorner(0, &corner, &scale, &map_rect);

        vector<inlinePictType> inline_pict;

        _data_item = update_mission_brief_point(
                _briefing_point, _level, corner, scale, map_rect, &inline_pict, &_highlight_rect,
                &_highlight_lines, _text);
        swap(inline_pict, _inline_pict);
    }
}

void BriefingScreen::draw_system_map() const {
    Point off = offset();
    {
        Points points;
        for (int i = 0; i < _system_stars.size(); ++i) {
            const Star& star       = _system_stars[i];
            RgbColor    star_color = GetRGBTranslateColorShade(Hue::GRAY, star.shade + DARKEST);
            Point       location   = star.location;
            location.offset(off.h, off.v);
            points.draw(location, star_color);
        }
    }

    coordPointType corner;
    int32_t        scale;
    Rect           pix_bounds = _bounds.size().as_rect();
    GetLevelFullScaleAndCorner(0, &corner, &scale, &pix_bounds);
    Rect bounds = _bounds;
    bounds.offset(off.h, off.v);
    draw_arbitrary_sector_lines(corner, scale, 16, bounds);
    draw_briefing_objects(bounds.origin(), 32, pix_bounds, corner, scale);
}

void BriefingScreen::draw_brief_point() const {
    draw_system_map();

    Point off = offset();
    if (!_highlight_rect.empty()) {
        Rect highlight_rect = _highlight_rect;
        highlight_rect.offset(off.h, off.v);
        Rects          rects;
        const RgbColor very_light = GetRGBTranslateColorShade(kMissionDataHiliteColor, LIGHTEST);
        rects.fill(
                {highlight_rect.left, highlight_rect.top, highlight_rect.right,
                 highlight_rect.top + 1},
                very_light);
        rects.fill(
                {highlight_rect.right - 1, highlight_rect.top, highlight_rect.right,
                 highlight_rect.bottom},
                very_light);
        rects.fill(
                {highlight_rect.left, highlight_rect.bottom - 1, highlight_rect.right,
                 highlight_rect.bottom},
                very_light);
        rects.fill(
                {highlight_rect.left, highlight_rect.top, highlight_rect.left + 1,
                 highlight_rect.bottom},
                very_light);

        const RgbColor medium = GetRGBTranslateColorShade(kMissionDataHiliteColor, MEDIUM);
        for (size_t i = 0; i < _highlight_lines.size(); ++i) {
            using std::swap;
            Point p1 = _highlight_lines[i].first;
            p1.offset(off.h, off.v);
            Point p2 = _highlight_lines[i].second;
            p2.offset(off.h, off.v);
            if (p1.h > p2.h) {
                swap(p1.h, p2.h);
            }
            if (p1.v > p2.v) {
                swap(p1.v, p2.v);
            }
            rects.fill({p1.h, p1.v, p2.h + 1, p2.v + 1}, medium);
        }
    }

    Rect bounds = _data_item.outer_bounds();
    bounds.offset(off.h, off.v);
    Rects().fill(bounds, RgbColor::black());
    _data_item.draw(off, KEYBOARD_MOUSE);
    bounds = _data_item.inner_bounds();
    bounds.offset(off.h, off.v);
    draw_text_in_rect(bounds, _text, _data_item.style(), _data_item.hue());
}

void BriefingScreen::show_object_data(int index, const KeyDownEvent& event) {
    show_object_data(index, ObjectDataScreen::KEY, 0, event.key(), Gamepad::Button::NONE);
}

void BriefingScreen::show_object_data(int index, const GamepadButtonDownEvent& event) {
    show_object_data(index, ObjectDataScreen::GAMEPAD, 0, Key::NONE, event.button);
}

void BriefingScreen::show_object_data(
        int index, ObjectDataScreen::Trigger trigger, int mouse, Key key,
        Gamepad::Button gamepad) {
    if (index < _inline_pict.size()) {
        auto obj = _inline_pict[index].object;
        if (obj) {
            const Point origin = _inline_pict[index].bounds.center();
            stack()->push(new ObjectDataScreen(origin, *obj, trigger, mouse, key, gamepad));
            return;
        }
    }
}

}  // namespace antares
