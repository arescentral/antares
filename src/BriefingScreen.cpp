// Ares, a tactical space combat game.
// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#include "BriefingScreen.hpp"

#include "BriefingRenderer.hpp"
#include "CardStack.hpp"
#include "ColorTranslation.hpp"
#include "Error.hpp"
#include "Instruments.hpp"
#include "OffscreenGWorld.hpp"
#include "Options.hpp"
#include "PlayerInterface.hpp"
#include "PlayerInterfaceDrawing.hpp"
#include "Randomize.hpp"
#include "ScenarioMaker.hpp"
#include "VideoDriver.hpp"

namespace antares {

extern PixMap* gRealWorld;
extern PixMap* gOffWorld;
extern PixMap* gSaveWorld;

namespace {

const int kBriefingScreenResId = 6000;

}  // namespace

BriefingScreen::BriefingScreen(int scenario, bool* cancelled)
        : InterfaceScreen(kBriefingScreenResId, gRealWorld->bounds(), true),
          _scenario(scenario),
          _cancelled(cancelled),
          _briefing_point(0),
          _briefing_point_count(GetBriefPointNumber(_scenario) + 2) {
    DrawInSaveWorld();

    Rect map_rect = item(MAP_RECT).bounds;

    // Draw 500 randomized stars.
    for (int i = 0; i < 500; ++i) {
        RgbColor star_color;
        GetRGBTranslateColorShade(&star_color, GRAY, Randomize(kVisibleShadeNum) + DARKEST);
        const int x = map_rect.left + Randomize(map_rect.width());
        const int y = map_rect.top + Randomize(map_rect.height());
        gSaveWorld->set(x, y, star_color);
    }
    gOffWorld->view(map_rect).copy(gSaveWorld->view(map_rect));
    gRealWorld->view(map_rect).copy(gSaveWorld->view(map_rect));

    _used_rect = Rect (0, 0, 200, 200);
    _used_rect.center_in(map_rect);

    _data_item.bounds = _used_rect;
    _data_item.color = GOLD;
    _data_item.kind = kLabeledRect;
    _data_item.style = kLarge;
    _data_item.item.labeledRect.label.stringID = 4000;
    _data_item.item.labeledRect.label.stringNumber = 1;

    coordPointType corner;
    long scale;
    GetScenarioFullScaleAndCorner(_scenario, 0, &corner, &scale, &map_rect);
    DrawArbitrarySectorLines(&corner, scale, 16, &map_rect, gSaveWorld, 0, 0);
    Briefing_Objects_Render(_scenario, gSaveWorld, 32, &map_rect, 0, 0, &corner, scale);
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

void BriefingScreen::key_down(const KeyDownEvent& event) {
    if (event.key() == 0x35) {
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
        draw();
        break;

      case NEXT:
        if (_briefing_point < _briefing_point_count - 1) {
            ++_briefing_point;
        }
        adjust_interface();
        draw();
        break;

      default:
        fail("Got unknown button %d.", button);
    }
}

void BriefingScreen::draw() const {
    InterfaceScreen::draw();

    coordPointType corner;
    long scale;
    Rect map_rect = item(MAP_RECT).bounds;
    GetScenarioFullScaleAndCorner(_scenario, 0, &corner, &scale, &map_rect);

    inlinePictType inline_pict[kMaxInlinePictNum];

    UpdateMissionBriefPoint(
            &_data_item, _briefing_point, _scenario, &corner, scale, 0, &map_rect, &_used_rect,
            inline_pict);
}

}  // namespace antares
