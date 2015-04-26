// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2012 The Antares Authors
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
#include "data/picture.hpp"
#include "drawing/briefing.hpp"
#include "drawing/color.hpp"
#include "drawing/interface.hpp"
#include "drawing/shapes.hpp"
#include "game/instruments.hpp"
#include "game/scenario-maker.hpp"
#include "math/random.hpp"
#include "ui/card.hpp"
#include "ui/interface-handling.hpp"
#include "ui/screens/object-data.hpp"
#include "video/driver.hpp"

using sfz::Exception;
using sfz::String;
using sfz::format;
using std::vector;

namespace antares {

namespace {

enum BriefingPoint {
    STAR_MAP = 0,
    BLANK_SYSTEM_MAP,
    BRIEFING_POINT_COUNT,
};

const int kStarMapPictId = 8000;
const int kMissionStarPointWidth = 16;
const int kMissionStarPointHeight = 12;
const int32_t kMissionDataHiliteColor = GOLD;

LabeledRect data_item(const InterfaceItem& map_rect) {
    Rect bounds(0, 0, 200, 200);
    bounds.center_in(map_rect.bounds());
    return LabeledRect(0, bounds, {4000, 1}, GOLD, kLarge);
}

}  // namespace

BriefingScreen::BriefingScreen(const Scenario* scenario, bool* cancelled)
        : InterfaceScreen("briefing", world, true),
          _scenario(scenario),
          _cancelled(cancelled),
          _briefing_point(0),
          _briefing_point_count(_scenario->brief_point_size() + 2),
          _data_item(data_item(item(MAP_RECT))) {
    build_star_map();
    build_system_map();
}

BriefingScreen::~BriefingScreen() { }

void BriefingScreen::become_front() {
    if (_briefing_point_count <= 2) {
        stack()->pop(this);
    } else {
        InterfaceScreen::become_front();
    }
}

void BriefingScreen::overlay() const {
    switch (_briefing_point) {
      case STAR_MAP:
        {
            const Point star = _star_rect.center();
            RgbColor gold = GetRGBTranslateColorShade(GOLD, VERY_LIGHT);
            _star_map->draw_cropped(_bounds, Point(0, 2));
            draw_vbracket(_star_rect, gold);
            VideoDriver::driver()->draw_line(
                    Point(star.h, _bounds.top), Point(star.h, _star_rect.top), gold);
            VideoDriver::driver()->draw_line(
                    Point(star.h, _star_rect.bottom), Point(star.h, _bounds.bottom), gold);
            VideoDriver::driver()->draw_line(
                    Point(_bounds.left, star.v), Point(_star_rect.left, star.v), gold);
            VideoDriver::driver()->draw_line(
                    Point(_star_rect.right - 1, star.v), Point(_bounds.right, star.v), gold);
        }
        break;

      case BLANK_SYSTEM_MAP:
        draw_system_map();
        break;

      default:
        draw_brief_point();
        break;
    }
}

void BriefingScreen::mouse_down(const MouseDownEvent& event) {
    for (size_t i = 0; i < _inline_pict.size(); ++i) {
        if (_inline_pict[i].bounds.contains(event.where())) {
            const int pict_id = _inline_pict[i].id;
            for (auto obj: BaseObject::all()) {
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
        default: {
            return InterfaceScreen::key_down(event);
        }
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
        default: {
            return InterfaceScreen::gamepad_button_down(event);
        }
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
      case DONE:
        stack()->pop(this);
        break;

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

      default:
        throw Exception(format("Got unknown button {0}.", button.id));
    }
}

void BriefingScreen::build_star_map() {
    Picture pict(kStarMapPictId);
    _star_map = VideoDriver::driver()->new_sprite(format("/pictures/{0}.png", kStarMapPictId), pict);
    Rect pix_bounds = pict.size().as_rect();
    pix_bounds.offset(0, 2);
    pix_bounds.bottom -= 3;
    _bounds = pix_bounds;
    _bounds.center_in(item(MAP_RECT).bounds());

    _star_rect = Rect(_scenario->star_map_point(), Size(0, 0));
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

void BriefingScreen::build_system_map() {
    ArrayPixMap pix(_bounds.width(), _bounds.height());
    Rect pix_bounds = pix.size().as_rect();
    pix.fill(RgbColor::kClear);

    // Draw 500 randomized stars.
    for (int i = 0; i < 500; ++i) {
        Star star;
        star.shade = Randomize(kVisibleShadeNum);
        star.location.h = _bounds.left + Randomize(_bounds.width());
        star.location.v = _bounds.top + Randomize(_bounds.height());
        _system_stars.push_back(star);
    }

    coordPointType corner;
    int32_t scale;
    GetScenarioFullScaleAndCorner(_scenario, 0, &corner, &scale, &pix_bounds);
    Briefing_Objects_Render(&pix, 32, &pix_bounds, &corner, scale);
}

void BriefingScreen::build_brief_point() {
    if (_briefing_point >= BRIEFING_POINT_COUNT) {
        coordPointType corner;
        int32_t scale;
        Rect map_rect = item(MAP_RECT).bounds();
        GetScenarioFullScaleAndCorner(_scenario, 0, &corner, &scale, &map_rect);

        vector<inlinePictType> inline_pict;

        update_mission_brief_point(
                &_data_item, _briefing_point, _scenario, &corner, scale, &map_rect, inline_pict,
                _highlight_rect, _highlight_lines, _text);
        swap(inline_pict, _inline_pict);
    }
}

void BriefingScreen::draw_system_map() const {
    for (int i = 0; i < _system_stars.size(); ++i) {
        const Star& star = _system_stars[i];
        RgbColor star_color = GetRGBTranslateColorShade(GRAY, star.shade + DARKEST);
        VideoDriver::driver()->draw_point(star.location, star_color);
    }

    coordPointType corner;
    int32_t scale;
    Rect pix_bounds = _bounds.size().as_rect();
    GetScenarioFullScaleAndCorner(_scenario, 0, &corner, &scale, &pix_bounds);
    draw_arbitrary_sector_lines(corner, scale, 16, _bounds);
    draw_briefing_objects(_bounds.origin(), 32, pix_bounds, corner, scale);
}

void BriefingScreen::draw_brief_point() const {
    draw_system_map();

    if (!_highlight_rect.empty()) {
        const RgbColor very_light = GetRGBTranslateColorShade(kMissionDataHiliteColor, VERY_LIGHT);
        VideoDriver::driver()->draw_line(
                Point(_highlight_rect.left, _highlight_rect.top),
                Point(_highlight_rect.right - 1, _highlight_rect.top),
                very_light);
        VideoDriver::driver()->draw_line(
                Point(_highlight_rect.right - 1, _highlight_rect.top),
                Point(_highlight_rect.right - 1, _highlight_rect.bottom - 1),
                very_light);
        VideoDriver::driver()->draw_line(
                Point(_highlight_rect.right - 1, _highlight_rect.bottom - 1),
                Point(_highlight_rect.left, _highlight_rect.bottom - 1),
                very_light);
        VideoDriver::driver()->draw_line(
                Point(_highlight_rect.left, _highlight_rect.bottom - 1),
                Point(_highlight_rect.left, _highlight_rect.top),
                very_light);

        const RgbColor medium = GetRGBTranslateColorShade(kMissionDataHiliteColor, MEDIUM);
        for (size_t i = 0; i < _highlight_lines.size(); ++i) {
            Point p1 = _highlight_lines[i].first;
            Point p2 = _highlight_lines[i].second;
            VideoDriver::driver()->draw_line(p1, p2, medium);
        }
    }

    Rect bounds;
    GetAnyInterfaceItemGraphicBounds(_data_item, &bounds);
    VideoDriver::driver()->fill_rect(bounds, RgbColor::kBlack);
    draw_interface_item(_data_item, KEYBOARD_MOUSE);
    vector<inlinePictType> unused;
    draw_text_in_rect(_data_item.bounds(), _text, _data_item.style, _data_item.hue, unused);
}

void BriefingScreen::show_object_data(int index, const KeyDownEvent& event) {
    show_object_data(index, ObjectDataScreen::KEY, event.key());
}

void BriefingScreen::show_object_data(int index, const GamepadButtonDownEvent& event) {
    show_object_data(index, ObjectDataScreen::GAMEPAD, event.button);
}

void BriefingScreen::show_object_data(int index, ObjectDataScreen::Trigger trigger, int which) {
    if (index < _inline_pict.size()) {
        const int pict_id = _inline_pict[index].id;
        const Point origin = _inline_pict[index].bounds.center();
        for (auto obj: BaseObject::all()) {
            if (obj->pictPortraitResID == pict_id) {
                stack()->push(new ObjectDataScreen(origin, obj, trigger, which));
                return;
            }
        }
    }
}

}  // namespace antares
