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

#include "config/gamepad.hpp"
#include "config/keys.hpp"
#include "data/base-object.hpp"
#include "data/picture.hpp"
#include "data/resource.hpp"
#include "data/string-list.hpp"
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

using sfz::Exception;
using sfz::String;
using sfz::StringSlice;
using sfz::format;
using std::make_pair;
using std::pair;
using std::vector;
namespace utf8 = sfz::utf8;

namespace antares {

namespace {

enum BriefingPoint {
    STAR_MAP = 0,
    BLANK_SYSTEM_MAP,
    BRIEFING_POINT_COUNT,
};

}  // namespace

static const int     kStarMapPictId          = 8000;
static const int     kMissionStarPointWidth  = 16;
static const int     kMissionStarPointHeight = 12;
static const int32_t kMissionDataHiliteColor = GOLD;

static const int32_t kMissionBriefPointOffset = 2;
static const int32_t kMissionDataWidth        = 200;
static const int32_t kMissionDataVBuffer      = 40;
static const int32_t kMissionDataTopBuffer    = 30;
static const int32_t kMissionDataBottomBuffer = 15;
static const int32_t kMissionDataHBuffer      = 41;
static const int32_t kMissionLineHJog         = 10;

static LabeledRect data_item(const InterfaceItem& map_rect) {
    Rect bounds(0, 0, 200, 200);
    bounds.center_in(map_rect.bounds());
    return LabeledRect(0, bounds, {4000, 1}, GOLD, kLarge);
}

static const Font* interface_font(interfaceStyleType style) {
    if (style == kSmall) {
        return sys.fonts.small_button;
    } else {
        return sys.fonts.button;
    }
}

static void populate_inline_picts(
        Rect rect, StringSlice text, interfaceStyleType style,
        vector<inlinePictType>& inline_pict) {
    StyledText interface_text(interface_font(style));
    interface_text.set_interface_text(text);
    interface_text.wrap_to(rect.width(), kInterfaceTextHBuffer, kInterfaceTextVBuffer);
    inline_pict = interface_text.inline_picts();
    for (int i = 0; i < inline_pict.size(); ++i) {
        inline_pict[i].bounds.offset(rect.left, rect.top);
    }
}

static void update_mission_brief_point(
        LabeledRect* dataItem, int32_t whichBriefPoint, const Level* level, coordPointType* corner,
        int32_t scale, Rect* bounds, vector<inlinePictType>& inlinePict, Rect& highlight_rect,
        vector<pair<Point, Point>>& lines, String& text) {
    if (whichBriefPoint < kMissionBriefPointOffset) {
        // No longer handled here.
        return;
    }

    whichBriefPoint -= kMissionBriefPointOffset;

    Rect    hiliteBounds;
    int32_t headerID, headerNumber, contentID;
    BriefPoint_Data_Get(
            whichBriefPoint, level, &headerID, &headerNumber, &contentID, &hiliteBounds, corner,
            scale, 16, 32, bounds);
    hiliteBounds.offset(bounds->left, bounds->top);

    // TODO(sfiera): catch exception.
    Resource rsrc("text", "txt", contentID);
    text.assign(utf8::decode(rsrc.data()));
    int16_t textHeight = GetInterfaceTextHeightFromWidth(text, dataItem->style, kMissionDataWidth);
    if (hiliteBounds.left == hiliteBounds.right) {
        dataItem->bounds().left =
                (bounds->right - bounds->left) / 2 - (kMissionDataWidth / 2) + bounds->left;
        dataItem->bounds().right = dataItem->bounds().left + kMissionDataWidth;
        dataItem->bounds().top =
                (bounds->bottom - bounds->top) / 2 - (textHeight / 2) + bounds->top;
        dataItem->bounds().bottom = dataItem->bounds().top + textHeight;
        highlight_rect            = Rect();
    } else {
        if ((hiliteBounds.left + (hiliteBounds.right - hiliteBounds.left) / 2) >
            (bounds->left + (bounds->right - bounds->left) / 2)) {
            dataItem->bounds().right = hiliteBounds.left - kMissionDataHBuffer;
            dataItem->bounds().left  = dataItem->bounds().right - kMissionDataWidth;
        } else {
            dataItem->bounds().left  = hiliteBounds.right + kMissionDataHBuffer;
            dataItem->bounds().right = dataItem->bounds().left + kMissionDataWidth;
        }

        dataItem->bounds().top =
                hiliteBounds.top + (hiliteBounds.bottom - hiliteBounds.top) / 2 - textHeight / 2;
        dataItem->bounds().bottom = dataItem->bounds().top + textHeight;
        if (dataItem->bounds().top < (bounds->top + kMissionDataTopBuffer)) {
            dataItem->bounds().top    = bounds->top + kMissionDataTopBuffer;
            dataItem->bounds().bottom = dataItem->bounds().top + textHeight;
        }
        if (dataItem->bounds().bottom > (bounds->bottom - kMissionDataBottomBuffer)) {
            dataItem->bounds().bottom = bounds->bottom - kMissionDataBottomBuffer;
            dataItem->bounds().top    = dataItem->bounds().bottom - textHeight;
        }

        if (dataItem->bounds().left < (bounds->left + kMissionDataVBuffer)) {
            dataItem->bounds().left  = bounds->left + kMissionDataVBuffer;
            dataItem->bounds().right = dataItem->bounds().left + kMissionDataWidth;
        }
        if (dataItem->bounds().right > (bounds->right - kMissionDataVBuffer)) {
            dataItem->bounds().right = bounds->right - kMissionDataVBuffer;
            dataItem->bounds().left  = dataItem->bounds().right - kMissionDataWidth;
        }

        hiliteBounds.right++;
        hiliteBounds.bottom++;
        highlight_rect = hiliteBounds;
        Rect newRect;
        GetAnyInterfaceItemGraphicBounds(*dataItem, &newRect);
        lines.clear();
        if (dataItem->bounds().right < hiliteBounds.left) {
            Point p1(hiliteBounds.left, hiliteBounds.top);
            Point p2(newRect.right + kMissionLineHJog, hiliteBounds.top);
            Point p3(newRect.right + kMissionLineHJog, newRect.top);
            Point p4(newRect.right + 2, newRect.top);
            lines.push_back(make_pair(p1, p2));
            lines.push_back(make_pair(p2, p3));
            lines.push_back(make_pair(p3, p4));

            Point p5(hiliteBounds.left, hiliteBounds.bottom - 1);
            Point p6(newRect.right + kMissionLineHJog, hiliteBounds.bottom - 1);
            Point p7(newRect.right + kMissionLineHJog, newRect.bottom - 1);
            Point p8(newRect.right + 2, newRect.bottom - 1);
            lines.push_back(make_pair(p5, p6));
            lines.push_back(make_pair(p6, p7));
            lines.push_back(make_pair(p7, p8));
        } else {
            Point p1(hiliteBounds.right, hiliteBounds.top);
            Point p2(newRect.left - kMissionLineHJog, hiliteBounds.top);
            Point p3(newRect.left - kMissionLineHJog, newRect.top);
            Point p4(newRect.left - 3, newRect.top);
            lines.push_back(make_pair(p1, p2));
            lines.push_back(make_pair(p2, p3));
            lines.push_back(make_pair(p3, p4));

            Point p5(hiliteBounds.right, hiliteBounds.bottom - 1);
            Point p6(newRect.left - kMissionLineHJog, hiliteBounds.bottom - 1);
            Point p7(newRect.left - kMissionLineHJog, newRect.bottom - 1);
            Point p8(newRect.left - 3, newRect.bottom - 1);
            lines.push_back(make_pair(p5, p6));
            lines.push_back(make_pair(p6, p7));
            lines.push_back(make_pair(p7, p8));
        }
    }
    dataItem->label.assign(StringList(headerID).at(headerNumber - 1));
    Rect newRect;
    GetAnyInterfaceItemGraphicBounds(*dataItem, &newRect);
    populate_inline_picts(dataItem->bounds(), text, dataItem->style, inlinePict);
}

BriefingScreen::BriefingScreen(const Level* level, bool* cancelled)
        : InterfaceScreen("briefing", {0, 0, 640, 480}, true),
          _level(level),
          _cancelled(cancelled),
          _briefing_point(0),
          _briefing_point_count(_level->brief_point_size() + 2),
          _data_item(data_item(item(MAP_RECT))) {
    build_star_map();
    for (int i = 0; i < 500; ++i) {
        Star star;
        star.shade      = Randomize(kVisibleShadeNum);
        star.location.h = _bounds.left + Randomize(_bounds.width());
        star.location.v = _bounds.top + Randomize(_bounds.height());
        _system_stars.push_back(star);
    }
}

BriefingScreen::~BriefingScreen() {}

void BriefingScreen::become_front() {
    if (_briefing_point_count <= 2) {
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
            RgbColor    gold   = GetRGBTranslateColorShade(GOLD, VERY_LIGHT);
            Rect        bounds = _bounds;
            bounds.offset(off.h, off.v);
            _star_map.draw_cropped(bounds, Rect(Point(0, 2), bounds.size()));
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
    for (size_t i = 0; i < _inline_pict.size(); ++i) {
        if (_inline_pict[i].bounds.contains(where)) {
            const int pict_id = _inline_pict[i].id;
            for (auto obj : BaseObject::all()) {
                if (obj->pictPortraitResID == pict_id) {
                    stack()->push(new ObjectDataScreen(
                            event.where(), obj, ObjectDataScreen::MOUSE, event.button()));
                    return;
                }
            }
        }
    }
    InterfaceScreen::mouse_down(event);
}

void BriefingScreen::key_down(const KeyDownEvent& event) {
    switch (event.key()) {
        case Keys::ESCAPE: {
            *_cancelled = true;
            stack()->pop(this);
        }
            return;
        case Keys::K1: return show_object_data(0, event);
        case Keys::K2: return show_object_data(1, event);
        case Keys::K3: return show_object_data(2, event);
        case Keys::K4: return show_object_data(3, event);
        case Keys::K5: return show_object_data(4, event);
        case Keys::K6: return show_object_data(5, event);
        case Keys::K7: return show_object_data(6, event);
        case Keys::K8: return show_object_data(7, event);
        case Keys::K9: return show_object_data(8, event);
        case Keys::K0: return show_object_data(9, event);
        default: { return InterfaceScreen::key_down(event); }
    }
}

void BriefingScreen::gamepad_button_down(const GamepadButtonDownEvent& event) {
    switch (event.button) {
        case Gamepad::B: {
            *_cancelled = true;
            stack()->pop(this);
        }
            return;
        case Gamepad::UP: return show_object_data(0, event);
        case Gamepad::DOWN: return show_object_data(1, event);
        default: { return InterfaceScreen::gamepad_button_down(event); }
    }
}

void BriefingScreen::adjust_interface() {
    if (_briefing_point > 0) {
        dynamic_cast<Button&>(mutable_item(PREVIOUS)).status = kActive;
    } else {
        dynamic_cast<Button&>(mutable_item(PREVIOUS)).status = kDimmed;
    }
    if (_briefing_point < _briefing_point_count - 1) {
        dynamic_cast<Button&>(mutable_item(NEXT)).status = kActive;
    } else {
        dynamic_cast<Button&>(mutable_item(NEXT)).status = kDimmed;
    }
}

void BriefingScreen::handle_button(Button& button) {
    switch (button.id) {
        case DONE: stack()->pop(this); break;

        case PREVIOUS:
            if (_briefing_point > 0) {
                --_briefing_point;
            }
            adjust_interface();
            build_brief_point();
            break;

        case NEXT:
            if (_briefing_point < _briefing_point_count - 1) {
                ++_briefing_point;
            }
            adjust_interface();
            build_brief_point();
            break;

        default: throw Exception(format("Got unknown button {0}.", button.id));
    }
}

void BriefingScreen::build_star_map() {
    _star_map       = Picture(kStarMapPictId).texture();
    Rect pix_bounds = _star_map.size().as_rect();
    pix_bounds.offset(0, 2);
    pix_bounds.bottom -= 3;
    _bounds = pix_bounds;
    _bounds.center_in(item(MAP_RECT).bounds());

    _star_rect = Rect(_level->star_map_point(), Size(0, 0));
    _star_rect.inset(-kMissionStarPointWidth, -kMissionStarPointHeight);

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

void BriefingScreen::build_brief_point() {
    if (_briefing_point >= BRIEFING_POINT_COUNT) {
        coordPointType corner;
        int32_t        scale;
        Rect           map_rect = item(MAP_RECT).bounds();
        GetLevelFullScaleAndCorner(_level, 0, &corner, &scale, &map_rect);

        vector<inlinePictType> inline_pict;

        update_mission_brief_point(
                &_data_item, _briefing_point, _level, &corner, scale, &map_rect, inline_pict,
                _highlight_rect, _highlight_lines, _text);
        swap(inline_pict, _inline_pict);
    }
}

void BriefingScreen::draw_system_map() const {
    Point off = offset();
    {
        Points points;
        for (int i = 0; i < _system_stars.size(); ++i) {
            const Star& star       = _system_stars[i];
            RgbColor    star_color = GetRGBTranslateColorShade(GRAY, star.shade + DARKEST);
            Point       location   = star.location;
            location.offset(off.h, off.v);
            points.draw(location, star_color);
        }
    }

    coordPointType corner;
    int32_t        scale;
    Rect           pix_bounds = _bounds.size().as_rect();
    GetLevelFullScaleAndCorner(_level, 0, &corner, &scale, &pix_bounds);
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
        const RgbColor very_light = GetRGBTranslateColorShade(kMissionDataHiliteColor, VERY_LIGHT);
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

    Rect bounds;
    GetAnyInterfaceItemGraphicBounds(_data_item, &bounds);
    bounds.offset(off.h, off.v);
    Rects().fill(bounds, RgbColor::black());
    draw_interface_item(_data_item, KEYBOARD_MOUSE, off);
    bounds = _data_item.bounds();
    bounds.offset(off.h, off.v);
    draw_text_in_rect(bounds, _text, _data_item.style, _data_item.hue);
}

void BriefingScreen::show_object_data(int index, const KeyDownEvent& event) {
    show_object_data(index, ObjectDataScreen::KEY, event.key());
}

void BriefingScreen::show_object_data(int index, const GamepadButtonDownEvent& event) {
    show_object_data(index, ObjectDataScreen::GAMEPAD, event.button);
}

void BriefingScreen::show_object_data(int index, ObjectDataScreen::Trigger trigger, int which) {
    if (index < _inline_pict.size()) {
        const int   pict_id = _inline_pict[index].id;
        const Point origin  = _inline_pict[index].bounds.center();
        for (auto obj : BaseObject::all()) {
            if (obj->pictPortraitResID == pict_id) {
                stack()->push(new ObjectDataScreen(origin, obj, trigger, which));
                return;
            }
        }
    }
}

}  // namespace antares
