// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2011 Ares Central
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
// License along with this program.  If not, see
// <http://www.gnu.org/licenses/>.

#include "ui/screens/briefing.hpp"

#include "data/picture.hpp"
#include "drawing/briefing.hpp"
#include "drawing/color.hpp"
#include "drawing/interface.hpp"
#include "drawing/offscreen-gworld.hpp"
#include "game/instruments.hpp"
#include "game/scenario-maker.hpp"
#include "math/random.hpp"
#include "ui/card.hpp"
#include "ui/interface-handling.hpp"
#include "video/driver.hpp"

using sfz::Exception;
using sfz::format;

namespace antares {

namespace {

enum BriefingPoint {
    STAR_MAP = 0,
    BLANK_SYSTEM_MAP,
    BRIEFING_POINT_COUNT,
};

const int kBriefingScreenResId = 6000;
const int kStarMapPictId = 8000;
const int kMissionStarPointWidth = 16;
const int kMissionStarPointHeight = 12;

}  // namespace

BriefingScreen::BriefingScreen(const Scenario* scenario, bool* cancelled)
        : InterfaceScreen(kBriefingScreenResId, world, true),
          _scenario(scenario),
          _cancelled(cancelled),
          _briefing_point(0),
          _briefing_point_count(_scenario->brief_point_size() + 2) {
    Rect map_rect = item(MAP_RECT).bounds;

    _data_item.bounds = Rect(0, 0, 200, 200);
    _data_item.bounds.center_in(map_rect);
    _data_item.color = GOLD;
    _data_item.kind = kLabeledRect;
    _data_item.style = kLarge;
    _data_item.item.labeledRect.label.stringID = 4000;
    _data_item.item.labeledRect.label.stringNumber = 1;

    build_star_map();
    build_system_map();
}

BriefingScreen::~BriefingScreen() { }

void BriefingScreen::become_front() {
    if (_briefing_point_count <= 2) {
        stack()->pop(this);
    } else {
        InterfaceScreen::become_front();
        VideoDriver::driver()->set_game_state(MISSION_INTERFACE);
    }
}

void BriefingScreen::draw() const {
    InterfaceScreen::draw();
    switch (_briefing_point) {
      case STAR_MAP:
        _star_map->draw(_bounds.left, _bounds.top);
        break;

      case BLANK_SYSTEM_MAP:
        _system_map->draw(_bounds.left, _bounds.top);
        break;

      default:
        _system_map->draw(_bounds.left, _bounds.top);
        _brief_point->draw(0, 0);
        break;
    }
}

void BriefingScreen::key_down(const KeyDownEvent& event) {
    if (event.key() == Keys::ESCAPE) {
        *_cancelled = true;
        stack()->pop(this);
        return;
    } else {
        return InterfaceScreen::key_down(event);
    }
}

void BriefingScreen::adjust_interface() {
    if (_briefing_point > 0) {
        mutable_item(PREVIOUS)->set_status(kActive);
    } else {
        mutable_item(PREVIOUS)->set_status(kDimmed);
    }
    if (_briefing_point < _briefing_point_count - 1) {
        mutable_item(NEXT)->set_status(kActive);
    } else {
        mutable_item(NEXT)->set_status(kDimmed);
    }
}

void BriefingScreen::handle_button(int button) {
    switch (button) {
      case DONE:
        VideoDriver::driver()->set_game_state(UNKNOWN);
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
        throw Exception(format("Got unknown button {0}.", button));
    }
}

void BriefingScreen::build_star_map() {
    Picture pict(kStarMapPictId);
    Rect pix_bounds = pict.size().as_rect();
    pix_bounds.offset(0, 2);
    pix_bounds.bottom -= 3;
    _bounds = pix_bounds;
    _bounds.center_in(item(MAP_RECT).bounds);

    ArrayPixMap pix(pix_bounds.width(), pix_bounds.height());
    pix.copy(pict.view(pix_bounds));
    pix_bounds.offset(0, -2);

    const Point star = _scenario->star_map_point();
    Rect star_rect(star.h, star.v, star.h, star.v);
    star_rect.inset(-kMissionStarPointWidth, -kMissionStarPointHeight);
    RgbColor gold = GetRGBTranslateColorShade(GOLD, VERY_LIGHT);

    DrawNateVBracket(&pix, star_rect, pix_bounds, gold);
    DrawNateLine(&pix, pix_bounds, star.h, pix_bounds.top, star.h, star_rect.top, gold);
    DrawNateLine(&pix, pix_bounds, star.h, star_rect.bottom, star.h, pix_bounds.bottom, gold);
    DrawNateLine(&pix, pix_bounds, pix_bounds.left, star.v, star_rect.left, star.v, gold);
    DrawNateLine(&pix, pix_bounds, star_rect.right, star.v, pix_bounds.right, star.v, gold);

    _star_map.reset(VideoDriver::driver()->new_sprite("/x/star_map", pix));
}

void BriefingScreen::build_system_map() {
    ArrayPixMap pix(_bounds.width(), _bounds.height());
    Rect pix_bounds = pix.size().as_rect();

    // Draw 500 randomized stars.
    for (int i = 0; i < 500; ++i) {
        RgbColor star_color;
        star_color = GetRGBTranslateColorShade(GRAY, Randomize(kVisibleShadeNum) + DARKEST);
        const int x = pix_bounds.left + Randomize(pix_bounds.width());
        const int y = pix_bounds.top + Randomize(pix_bounds.height());
        pix.set(x, y, star_color);
    }

    coordPointType corner;
    int32_t scale;
    GetScenarioFullScaleAndCorner(_scenario, 0, &corner, &scale, &pix_bounds);
    DrawArbitrarySectorLines(&corner, scale, 16, &pix_bounds, &pix);
    Briefing_Objects_Render(&pix, 32, &pix_bounds, &corner, scale);

    _system_map.reset(VideoDriver::driver()->new_sprite("/x/system_map", pix));
}

void BriefingScreen::build_brief_point() {
    if (_briefing_point >= BRIEFING_POINT_COUNT) {
        coordPointType corner;
        int32_t scale;
        Rect map_rect = item(MAP_RECT).bounds;
        GetScenarioFullScaleAndCorner(_scenario, 0, &corner, &scale, &map_rect);

        inlinePictType inline_pict[kMaxInlinePictNum];

        ArrayPixMap pix(world.width(), world.height());
        pix.fill(RgbColor::kClear);
        UpdateMissionBriefPoint(&_data_item, _briefing_point, _scenario, &corner, scale,
                &map_rect, inline_pict, &pix);
        _brief_point.reset(VideoDriver::driver()->new_sprite(
                    format("/x/brief_point/{0}", _briefing_point), pix));
    } else {
        _brief_point.reset();
    }
}

}  // namespace antares
