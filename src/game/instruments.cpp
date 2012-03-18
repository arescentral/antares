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

#include "game/instruments.hpp"

#include <algorithm>

#include "data/picture.hpp"
#include "data/space-object.hpp"
#include "drawing/color.hpp"
#include "drawing/offscreen-gworld.hpp"
#include "drawing/shapes.hpp"
#include "game/admiral.hpp"
#include "game/cursor.hpp"
#include "game/globals.hpp"
#include "game/labels.hpp"
#include "game/minicomputer.hpp"
#include "game/motion.hpp"
#include "game/player-ship.hpp"
#include "game/space-object.hpp"
#include "math/macros.hpp"
#include "math/random.hpp"
#include "math/rotation.hpp"
#include "math/special.hpp"
#include "math/units.hpp"
#include "video/driver.hpp"

using sfz::format;
using sfz::range;
using sfz::scoped_array;
using sfz::scoped_ptr;
using std::min;
using std::max;

namespace antares {

const int32_t kRadarBlipNum     = 50;
const uint8_t kRadarColor     = GREEN;

const int32_t kRadarLeft        = 6;
const int32_t kRadarTop         = 6;
const int32_t kRadarRight       = 116;
const int32_t kRadarBottom      = 116;
const int32_t kRadarCenter      = 55;
const int32_t kRadarColorSteps  = 14;

const size_t kScaleListNum      = 64;
const int32_t kScaleListShift   = 6;
const int16_t kInstLeftPictID   = 501;
const int16_t kInstRightPictID  = 512;
const int32_t kSiteDistance     = 200;
const int32_t kSiteSize         = 16;

const int32_t kCursorBoundsSize = 16;

const int32_t kBarIndicatorHeight   = 98;
const int32_t kBarIndicatorWidth    = 9;
const int32_t kBarIndicatorLeft     = 6;

enum {
    kShieldBar      = 0,
    kEnergyBar      = 1,
    kBatteryBar     = 2,
    kFineMoneyBar   = 3,
    kGrossMoneyBar  = 4,
};

const size_t kMaxSectorLine         = 32;
const int32_t kMinGraphicSectorSize = 90;

const int32_t kGrossMoneyLeft       = 11;
const int32_t kGrossMoneyTop        = 48;
const int32_t kGrossMoneyHBuffer    = 2;
const int32_t kGrossMoneyVBuffer    = 4;
const int32_t kGrossMoneyBarWidth   = 10;
const int32_t kGrossMoneyBarHeight  = 5;
const int32_t kGrossMoneyBarNum     = 7;
const int32_t kGrossMoneyBarValue   = 5120000;
const uint8_t kGrossMoneyColor      = YELLOW;

const int32_t kFineMoneyLeft        = 25;
const int32_t kFineMoneyTop         = 48;
const int32_t kFineMoneyHBuffer     = 1;
const int32_t kFineMoneyVBuffer     = 1;
const int32_t kFineMoneyBarWidth    = 2;
const int32_t kFineMoneyBarHeight   = 4;
const int32_t kFineMoneyBarNum      = 100;
const int32_t kFineMoneyBarMod      = 5120000;
const int32_t kFineMoneyBarValue    = 51200;
const int32_t kFineMoneyColor       = PALE_GREEN;
const int32_t kFineMoneyNeedColor   = ORANGE;
const uint8_t kFineMoneyUseColor    = SKY_BLUE;

const int32_t kMaxMoneyValue        = kGrossMoneyBarValue * 7;

Rect mini_build_time_rect() {
    Rect result(play_screen.right + 10, 8, play_screen.right + 22, 37);
    result.offset(0, globals()->gInstrumentTop);
    return result;
}

const int32_t kMinimumAutoScale     = 2;

const int32_t kMouseSleepTime       = 58;

const int32_t kSectorLineBrightness = DARKER;

coordPointType          gLastGlobalCorner;

namespace {

scoped_ptr<Sprite> left_instrument_sprite;
scoped_ptr<Sprite> right_instrument_sprite;
scoped_array<Point> gRadarBlipData;
scoped_array<int32_t> gScaleList;
scoped_array<int32_t> gSectorLineData;
bool should_draw_sector_lines = false;

struct SiteData {
    Point a, b, c;
};
SiteData site_data;
bool should_draw_site = false;

template <typename T>
T clamp(T value, T min, T max) {
    if (value < min) {
        return min;
    } else if (value > max) {
        return max;
    } else {
        return value;
    }
}

}  // namespace

static void draw_bar_indicator(int16_t, int32_t, int32_t);
static void draw_money();
static void draw_build_time_bar(int32_t value);

void InstrumentInit() {
    globals()->gInstrumentTop = (world.height() / 2) - ( kPanelHeight / 2);

    gRadarBlipData.reset(new Point[kRadarBlipNum]);
    gScaleList.reset(new int32_t[kScaleListNum]);
    gSectorLineData.reset(new int32_t[kMaxSectorLine * 4]);
    ResetInstruments();

    // Initialize and crop left and right instrument picts.
    {
        Picture pict(kInstLeftPictID);
        ArrayPixMap pix_map(128, min(world.height(), pict.size().height));
        Rect from(Point(0, 0), pix_map.size());
        Rect to(Point(0, 0), pix_map.size());
        if (pict.size().height > world.height()) {
            from.offset(0, (pict.size().height - world.height()) / 2);
        }
        pix_map.view(to).copy(pict.view(from));
        left_instrument_sprite.reset(VideoDriver::driver()->new_sprite(
                    format("/pictures/{0}.png", kInstLeftPictID), pix_map));
    }
    {
        Picture pict(kInstRightPictID);
        ArrayPixMap pix_map(32, min(world.height(), pict.size().height));
        Rect from(Point(0, 0), pix_map.size());
        Rect to(Point(0, 0), pix_map.size());
        if (pict.size().height > world.height()) {
            from.offset(0, (pict.size().height - world.height()) / 2);
        }
        pix_map.view(to).copy(pict.view(from));
        right_instrument_sprite.reset(VideoDriver::driver()->new_sprite(
                    format("/pictures/{0}.png", kInstRightPictID), pix_map));
    }

    MiniScreenInit();
}

void InstrumentCleanup() {
    gRadarBlipData.reset();
    MiniScreenCleanup();
}

void ResetInstruments() {
    int32_t         *l, i;
    Point           *lp;

    globals()->gRadarCount = 0;
    globals()->gRadarSpeed = 30;
    globals()->gRadarRange = kRadarSize * 50;
    globals()->gLastScale = gAbsoluteScale = SCALE_SCALE;
    globals()->gWhichScaleNum = 0;
    gLastGlobalCorner.h = gLastGlobalCorner.v = 0;
    globals()->gMouseActive = false;
    globals()->gMouseTimeout = 0;
    l = gScaleList.get();
    for (i = 0; i < kScaleListNum; i++) {
        *l = SCALE_SCALE;
        l++;
    }

    for ( i = 0; i < kBarIndicatorNum; i++)
    {
        globals()->gBarIndicator[i].lastValue = globals()->gBarIndicator[i].thisValue = -1;
    }
    // the shield bar
    globals()->gBarIndicator[kShieldBar].top = 359 + globals()->gInstrumentTop;
    globals()->gBarIndicator[kShieldBar].color = SKY_BLUE;

    globals()->gBarIndicator[kEnergyBar].top = 231 + globals()->gInstrumentTop;
    globals()->gBarIndicator[kEnergyBar].color = GOLD;

    globals()->gBarIndicator[kBatteryBar].top = 103 + globals()->gInstrumentTop;
    globals()->gBarIndicator[kBatteryBar].color = SALMON;

    lp = gRadarBlipData.get();
    for ( i = 0; i < kRadarBlipNum; i++)
    {
        lp->h = -1;
        lp++;
    }

    l = gSectorLineData.get();
    SFZ_FOREACH(int count, range(kMaxSectorLine), {
        *l = -1;
        l++;
        *l = -1;
        l++;
        *l = -1;
        l++;
        *l = -1;
        l++;
    });
}

void UpdateRadar(int32_t unitsDone) {
    bool radar_is_functioning;
    if (gScrollStarObject == NULL) {
        radar_is_functioning = false;
    } else if (gScrollStarObject->offlineTime <= 0) {
        radar_is_functioning = true;
    } else {
        radar_is_functioning = (Randomize(gScrollStarObject->offlineTime) < 5);
    }

    if (unitsDone < 0) {
        unitsDone = 0;
    }
    globals()->gRadarCount -= unitsDone;
    if (globals()->gMouseActive) {
        globals()->gMouseTimeout += unitsDone;
    }

    RgbColor color;
    if (globals()->gRadarCount <= 0) {
        color = GetRGBTranslateColorShade(kRadarColor, VERY_DARK);
    } else {
        color = GetRGBTranslateColorShade(kRadarColor, ((kRadarColorSteps * globals()->gRadarCount) / globals()->gRadarSpeed) + 1);
    }

    if (radar_is_functioning) {
        for (int rcount = 0; rcount < kRadarBlipNum; rcount++) {
            Point* lp = gRadarBlipData.get() + rcount;
            if (lp->h >= 0) {
                gRealWorld->set(lp->h, lp->v, color);
            }
        }
    }

    if ((gScrollStarObject == NULL) || !gScrollStarObject->active) {
        return;
    }

    Rect bounds(kRadarLeft, kRadarTop, kRadarRight, kRadarBottom);
    bounds.offset(0, globals()->gInstrumentTop);
    bounds.inset(1, 1);

    const RgbColor very_light = GetRGBTranslateColorShade(kRadarColor, VERY_LIGHT);
    const RgbColor darkest = GetRGBTranslateColorShade(kRadarColor, DARKEST);
    const RgbColor very_dark = GetRGBTranslateColorShade(kRadarColor, VERY_DARK);

    if (radar_is_functioning) {
        if (globals()->gRadarCount <= 0) {
            Rect radar = bounds;
            FrameRect(gOffWorld, radar, very_light);
            radar.inset(1, 1);
            gOffWorld->view(radar).fill(darkest);

            int32_t dx = gScrollStarObject->location.h - gGlobalCorner.h;
            dx = dx * kRadarSize / globals()->gRadarRange;
            Rect view_range(-dx, -dx, dx, dx);
            view_range.center_in(bounds);
            view_range.offset(1, 1);
            view_range.clip_to(radar);
            gOffWorld->view(view_range).fill(very_dark);

            for (int i = 0; i < kRadarBlipNum; ++i) {
                Point* lp = gRadarBlipData.get() + i;
                lp->h = -1;
            }

            Point* lp = gRadarBlipData.get();
            Point* end = lp + kRadarBlipNum;
            globals()->gRadarCount = globals()->gRadarSpeed;

            const int32_t rrange = globals()->gRadarRange >> 1L;
            for (int oCount = 0; oCount < kMaxSpaceObject; oCount++) {
                spaceObjectType *anObject = gSpaceObjectData.get() + oCount;
                if (!anObject->active || (anObject == gScrollStarObject)) {
                    continue;
                }
                int x = anObject->location.h - gScrollStarObject->location.h;
                int y = anObject->location.v - gScrollStarObject->location.v;
                if ((x < -rrange) || (x >= rrange) || (y < -rrange) || (y >= rrange)) {
                    continue;
                }
                Point p(x * kRadarSize / globals()->gRadarRange,
                        y * kRadarSize / globals()->gRadarRange);
                p.offset(kRadarCenter + kRadarLeft,
                        kRadarCenter + kRadarTop + globals()->gInstrumentTop);
                if (!radar.contains(p)) {
                    continue;
                }
                *lp = p;
                ++lp;
                if (lp == end) {
                    break;
                }
            }
        }
    } else {
        gRealWorld->view(bounds).fill(darkest);
    }

    uint32_t bestScale = MIN_SCALE;
    switch (globals()->gZoomMode) {
      case kNearestFoeZoom:
      case kNearestAnythingZoom:
        {
            spaceObjectType* anObject = gSpaceObjectData.get() + globals()->gClosestObject;
            uint64_t hugeDistance = anObject->distanceFromPlayer;
            if (hugeDistance == 0) { // if this is true, then we haven't calced its distance
                uint64_t x_distance = ABS<int32_t>(gScrollStarObject->location.h - anObject->location.h);
                uint64_t y_distance = ABS<int32_t>(gScrollStarObject->location.v - anObject->location.v);

                hugeDistance = y_distance * y_distance + x_distance * x_distance;
            }
            bestScale = wsqrt(hugeDistance);
            if (bestScale == 0) bestScale = 1;
            bestScale = globals()->gCenterScaleV / bestScale;
            if (bestScale < SCALE_SCALE) bestScale = (bestScale >> 2L) + (bestScale >> 1L);
            bestScale = clamp<uint32_t>(bestScale, kMinimumAutoScale, SCALE_SCALE);
        }
        break;

      case kActualSizeZoom:
        bestScale = SCALE_SCALE;
        break;

      case kEighthSizeZoom:
        bestScale = kOneEighthScale;
        break;

      case kQuarterSizeZoom:
        bestScale = kOneQuarterScale;
        break;

      case kHalfSizeZoom:
        bestScale = kOneHalfScale;
        break;

      case kTimesTwoZoom:
        bestScale = kTimesTwoScale;
        break;

      case kSmallestZoom:
        {
            spaceObjectType* anObject = gSpaceObjectData.get() + globals()->gFarthestObject;
            uint64_t tempWide = anObject->distanceFromPlayer;
            bestScale = wsqrt(tempWide);
            if (bestScale == 0) bestScale = 1;
            bestScale = globals()->gCenterScaleV / bestScale;
            if (bestScale < SCALE_SCALE) bestScale = (bestScale >> 2L) + (bestScale >> 1L);
            bestScale = clamp<uint32_t>(bestScale, kMinimumAutoScale, SCALE_SCALE);
        }
        break;
    }

    int32_t* scaleval;
    for (int x = 0; x < unitsDone; x++) {
        scaleval = gScaleList.get() + globals()->gWhichScaleNum;
        *scaleval = bestScale;
        globals()->gWhichScaleNum++;
        if (globals()->gWhichScaleNum == kScaleListNum) {
            globals()->gWhichScaleNum = 0;
        }
    }

    scaleval = gScaleList.get();
    int absolute_scale = 0;
    for (int oCount = 0; oCount < kScaleListNum; oCount++) {
        absolute_scale += *scaleval++;
    }
    absolute_scale >>= kScaleListShift;

    gAbsoluteScale = absolute_scale;
}

// SHOW ME THE MONEY
static void draw_money() {
    const admiralType* const admiral = mGetAdmiralPtr(globals()->gPlayerAdmiralNumber);
    const int cash = clamp(admiral->cash, 0, kMaxMoneyValue - 1);
    globals()->gBarIndicator[kFineMoneyBar].thisValue
        = (cash % kFineMoneyBarMod) / kFineMoneyBarValue;
    const int price = MiniComputerGetPriceOfCurrentSelection() / kFineMoneyBarValue;

    if ((globals()->gBarIndicator[kFineMoneyBar].thisValue !=
                globals()->gBarIndicator[kFineMoneyBar].lastValue) ||
            (price != globals()->gLastSelectedBuildPrice)) {
        Rect box(0, 0, kFineMoneyBarWidth, kFineMoneyBarHeight - 1);
        box.offset(kFineMoneyLeft + kFineMoneyHBuffer + play_screen.right,
                kFineMoneyTop + globals()->gInstrumentTop + kFineMoneyVBuffer);

        // First section of the money bar: when we can afford the current selection, displays the
        // money which will remain after it is purchased.  When we cannot, displays the money we
        // currently have.
        int first_threshold;
        RgbColor first_color_major;
        RgbColor first_color_minor;

        // Second section of the money bar: when we can afford the current selection, displays the
        // amount which will be deducted after it is purchased.  When we cannot, displays the
        // amount of additional money which we need to amass before it can be purchased.
        int second_threshold;
        RgbColor second_color_major;
        RgbColor second_color_minor;

        // Third section: money we don't have and don't need for the current selection.
        RgbColor third_color = GetRGBTranslateColorShade(kFineMoneyColor, VERY_DARK);

        if (globals()->gBarIndicator[kFineMoneyBar].thisValue < price) {
            first_color_major = GetRGBTranslateColorShade(kFineMoneyColor, VERY_LIGHT);
            first_color_minor = GetRGBTranslateColorShade(kFineMoneyColor, LIGHT);
            second_color_major = GetRGBTranslateColorShade(kFineMoneyNeedColor, MEDIUM);
            second_color_minor = GetRGBTranslateColorShade(kFineMoneyNeedColor, DARK);
            first_threshold = globals()->gBarIndicator[kFineMoneyBar].thisValue;
            second_threshold = price;
        } else {
            first_color_major = GetRGBTranslateColorShade(kFineMoneyColor, VERY_LIGHT);
            first_color_minor = GetRGBTranslateColorShade(kFineMoneyColor, LIGHT);
            second_color_major = GetRGBTranslateColorShade(kFineMoneyUseColor, VERY_LIGHT);
            second_color_minor = GetRGBTranslateColorShade(kFineMoneyUseColor, LIGHT);
            first_threshold = globals()->gBarIndicator[kFineMoneyBar].thisValue - price;
            second_threshold = globals()->gBarIndicator[kFineMoneyBar].thisValue;
        }

        for (int i = 0; i < kFineMoneyBarNum; ++i) {
            if (i < first_threshold) {
                if ((i % 5) != 0) {
                    VideoDriver::driver()->fill_rect(box, first_color_minor);
                } else {
                    VideoDriver::driver()->fill_rect(box, first_color_major);
                }
            } else if (i < second_threshold) {
                if ((i % 5) != 0) {
                    VideoDriver::driver()->fill_rect(box, second_color_minor);
                } else {
                    VideoDriver::driver()->fill_rect(box, second_color_major);
                }
            } else {
                VideoDriver::driver()->fill_rect(box, third_color);
            }
            box.offset(0, kFineMoneyBarHeight);
        }
        globals()->gBarIndicator[kFineMoneyBar].thisValue = second_threshold;
    }

    globals()->gLastSelectedBuildPrice = price;

    barIndicatorType* gross = globals()->gBarIndicator + kGrossMoneyBar;
    gross->thisValue = (admiral->cash / kGrossMoneyBarValue);
    if (gross->thisValue != gross->lastValue) {
        Rect box(0, 0, kGrossMoneyBarWidth, kGrossMoneyBarHeight - 1);
        box.offset(play_screen.right + kGrossMoneyLeft + kGrossMoneyHBuffer,
                kGrossMoneyTop + globals()->gInstrumentTop + kGrossMoneyVBuffer);

        const RgbColor light = GetRGBTranslateColorShade(kGrossMoneyColor, VERY_LIGHT);
        const RgbColor dark = GetRGBTranslateColorShade(kGrossMoneyColor, VERY_DARK);
        for (int i = 0; i < kGrossMoneyBarNum; ++i) {
            if (i < gross->thisValue) {
                VideoDriver::driver()->fill_rect(box, light);
            } else {
                VideoDriver::driver()->fill_rect(box, dark);
            }
            box.offset(0, kGrossMoneyBarHeight);
        }
    }
}

void DrawInstrumentPanel() {
    globals()->gZoomMode = kNearestFoeZoom;

    gRealWorld->fill(RgbColor::kClear);
    gOffWorld->fill(RgbColor::kClear);

    MakeMiniScreenFromIndString(1);
    DrawMiniScreen();
    ResetInstruments();
    ClearMiniObjectData();
    ShowWholeMiniScreen();
    UpdatePlayerAmmo(-1, -1, -1);
    UpdateRadar(100);
}

void draw_instruments() {
    Rect left_rect(world.left, world.top, viewport.left, world.bottom);
    Rect right_rect(viewport.right, world.top, world.right, world.bottom);

    if (world.height() > 768) {
        left_rect.inset(0, (world.height() - 768) / 2);
        right_rect.inset(0, (world.height() - 768) / 2);
    }

    left_instrument_sprite->draw(left_rect.left, left_rect.top);
    right_instrument_sprite->draw(right_rect.left, right_rect.top);

    scoped_ptr<Sprite> left_instruments(VideoDriver::driver()->new_sprite(
                "/x/left_instruments", gRealWorld->view(left_rect)));
    left_instruments->draw(left_rect);

    baseObjectType* base = gScrollStarObject->baseType;
    draw_bar_indicator(kShieldBar, gScrollStarObject->health, base->health);
    draw_bar_indicator(kEnergyBar, gScrollStarObject->energy, base->energy);
    draw_bar_indicator(kBatteryBar, gScrollStarObject->battery, base->energy * 5);
    draw_build_time_bar(globals()->gMiniScreenData.buildTimeBarValue);
    draw_money();
}

void EraseSite() {
    globals()->old_cursor_coord = globals()->cursor_coord;
}

void update_site() {
    if (gScrollStarObject == NULL) {
        should_draw_site = false;
    } else if (gScrollStarObject->offlineTime <= 0) {
        should_draw_site = true;
    } else {
        should_draw_site = (Randomize(gScrollStarObject->offlineTime) < 5);
    }

    if (should_draw_site && gScrollStarObject->active && (gScrollStarObject->sprite != NULL)) {
        int count;
        Fixed fa, fb, fc;
        GetRotPoint(&fa, &fb, gScrollStarObject->direction);

        fc = mLongToFixed(-kSiteDistance);
        fa = mMultiplyFixed(fc, fa);
        fb = mMultiplyFixed(fc, fb);

        Point a(mFixedToLong(fa), mFixedToLong(fb));
        a.offset(gScrollStarObject->sprite->where.h, gScrollStarObject->sprite->where.v);
        site_data.a = a;

        count = gScrollStarObject->direction;
        mAddAngle(count, 30);
        GetRotPoint(&fa, &fb, count);
        fc = mLongToFixed(kSiteSize);
        fa = mMultiplyFixed(fc, fa);
        fb = mMultiplyFixed(fc, fb);

        Point b(a.h + mFixedToLong(fa), a.v + mFixedToLong(fb));
        site_data.b = b;

        count = gScrollStarObject->direction;
        mAddAngle(count, -30);
        GetRotPoint(&fa, &fb, count);
        fc = mLongToFixed(kSiteSize);
        fa = mMultiplyFixed(fc, fa);
        fb = mMultiplyFixed(fc, fb);

        Point c(a.h + mFixedToLong(fa), a.v + mFixedToLong(fb));
        site_data.c = c;
    } else {
        should_draw_site = false;
    }

    // do the cursor, too
    Point cursor_coord = VideoDriver::driver()->get_mouse();
    MoveSpriteCursor(cursor_coord);
    HideSpriteCursor();
    if (cursor_coord.h < viewport.left) {
        ShowSpriteCursor();
    } else if (cursor_coord.h > (viewport.right - kCursorBoundsSize - 1)) {
        cursor_coord.h = viewport.right - kCursorBoundsSize - 1;
    }
    if (cursor_coord.v < (viewport.top + kCursorBoundsSize)) {
        cursor_coord.v = viewport.top + kCursorBoundsSize;
    } else if (cursor_coord.v > (play_screen.bottom - kCursorBoundsSize - 1)) {
        cursor_coord.v = play_screen.bottom - kCursorBoundsSize - 1;
    }

    globals()->cursor_coord = cursor_coord;
    if ((cursor_coord != globals()->old_cursor_coord) && (!SpriteCursorVisible())) {
        globals()->gMouseActive = true;
        globals()->gMouseTimeout = 0;
    } else if (globals()->gMouseTimeout > kMouseSleepTime) {
        globals()->gMouseActive = false;
    }
}

void draw_site() {
    if (should_draw_site) {
        Stencil stencil(VideoDriver::driver());
        Rect clip = viewport;
        VideoDriver::driver()->fill_rect(clip, RgbColor::kWhite);
        stencil.apply();

        const RgbColor light = GetRGBTranslateColorShade(PALE_GREEN, MEDIUM);
        const RgbColor dark = GetRGBTranslateColorShade(PALE_GREEN, DARKER + kSlightlyDarkerColor);
        VideoDriver::driver()->draw_line(site_data.a, site_data.b, light);
        VideoDriver::driver()->draw_line(site_data.a, site_data.c, light);
        VideoDriver::driver()->draw_line(site_data.b, site_data.c, dark);
    }
}

void update_sector_lines() {
    should_draw_sector_lines = false;
    if (gScrollStarObject != NULL) {
        if (gScrollStarObject->offlineTime <= 0) {
            should_draw_sector_lines = true;
        } else if (Randomize(gScrollStarObject->offlineTime) < 5) {
            should_draw_sector_lines = true;
        }
    }

    if ((globals()->gLastScale < kBlipThreshhold) != (gAbsoluteScale < kBlipThreshhold)) {
        PlayVolumeSound(kComputerBeep4, kMediumVolume, kMediumPersistence, kLowPrioritySound);
    }

    globals()->gLastScale = gAbsoluteScale;
    gLastGlobalCorner = gGlobalCorner;
}

void draw_sector_lines() {
    int32_t         *l;
    uint32_t        size, level, x, h, division;
    RgbColor        color;

    Rect clipRect = viewport;

    size = kSubSectorSize / 4;
    level = 1;
    do {
        level *= 2;
        size *= 4;
        h = (size * globals()->gLastScale) >> SHIFT_SCALE;
    } while (h < kMinGraphicSectorSize);
    level /= 2;
    level *= level;

    x = size - (gLastGlobalCorner.h & (size - 1));
    division = ((gLastGlobalCorner.h + x) >> kSubSectorShift) & 0x0000000f;
    x = ((x * globals()->gLastScale) >> SHIFT_SCALE) + viewport.left;

    l = gSectorLineData.get();
    if (should_draw_sector_lines) {
        while ((x < implicit_cast<uint32_t>(viewport.right)) && (h > 0)) {
            RgbColor color;
            if (!division) {
                color = GetRGBTranslateColorShade(GREEN, kSectorLineBrightness);
            } else if (!(division & 0x3)) {
                color = GetRGBTranslateColorShade(SKY_BLUE, kSectorLineBrightness);
            } else {
                color = GetRGBTranslateColorShade(BLUE, kSectorLineBrightness);
            }

            VideoDriver::driver()->draw_line(Point(x, viewport.top), Point(x, viewport.bottom), color);
            *l = x;
            l += 2;
            division += level;
            division &= 0x0000000f;
            x += h;
        }
    }

    x = size - (gLastGlobalCorner.v & (size - 1));
    division = ((gLastGlobalCorner.v + x) >> kSubSectorShift) & 0x0000000f;
    x = ((x * globals()->gLastScale) >> SHIFT_SCALE) + viewport.top;

    l = gSectorLineData.get() + (kMaxSectorLine * 2);
    if (should_draw_sector_lines) {
        while ((x < implicit_cast<uint32_t>(viewport.bottom)) && (h > 0)) {
            RgbColor color;
            if (!division) {
                color = GetRGBTranslateColorShade(GREEN, kSectorLineBrightness);
            } else if (!(division & 0x3)) {
                color = GetRGBTranslateColorShade(SKY_BLUE, kSectorLineBrightness);
            } else {
                color = GetRGBTranslateColorShade(BLUE, kSectorLineBrightness);
            }

            VideoDriver::driver()->draw_line(Point(viewport.left, x), Point(viewport.right, x), color);
            *l = x;
            l += 2;

            division += level;
            division &= 0x0000000f;

            x += h;
        }
    }
}

void InstrumentsHandleClick() {
    const Point where = globals()->cursor_coord;
    PlayerShipHandleClick(where);
    MiniComputerHandleClick(where);
    if (!SpriteCursorVisible()) {
        globals()->gMouseActive = true;
        globals()->gMouseTimeout = 0;
    }
}

void InstrumentsHandleDoubleClick() {
    const Point where = globals()->cursor_coord;
    PlayerShipHandleClick(where);
    MiniComputerHandleDoubleClick(where);
    if (!SpriteCursorVisible()) {
        globals()->gMouseActive = true;
        globals()->gMouseTimeout = 0;
    }
}

void InstrumentsHandleMouseUp() {
    const Point where = globals()->cursor_coord;
    MiniComputerHandleMouseUp(where);
}

void InstrumentsHandleMouseStillDown() {
    const Point where = globals()->cursor_coord;
    MiniComputerHandleMouseStillDown(where);
}

void DrawArbitrarySectorLines(coordPointType *corner, int32_t scale, int32_t minSectorSize,
        Rect *bounds, PixMap* pixBase) {
    uint32_t        size, level, x, h, division;
    Rect        clipRect;
    RgbColor        color;

    clipRect.left = bounds->left;
    clipRect.right = bounds->right;
    clipRect.top = bounds->top;
    clipRect.bottom = bounds->bottom;

    size = kSubSectorSize >> 2L;
    level = 1;
    do
    {
        level <<= 1L;
        size <<= 2L;
        h = size;
        h *= scale;
        h >>= SHIFT_SCALE;
    } while (h < implicit_cast<uint32_t>(minSectorSize));
    level >>= 1L;
    level *= level;

    x = corner->h;
    x &= size - 1;
    x = size - x;

    division = corner->h + x;
    division >>= kSubSectorShift;
    division &= 0x0000000f;

    x *= scale;
    x >>= SHIFT_SCALE;
    x += bounds->left;

    while ((x < implicit_cast<uint32_t>(bounds->right)) && (h > 0)) {
        if ( !division)
        {
            color = GetRGBTranslateColorShade(GREEN, DARKER);
        } else if ( !(division & 0x3))
        {
            color = GetRGBTranslateColorShade(SKY_BLUE, DARKER);
        } else
        {
            color = GetRGBTranslateColorShade(BLUE, DARKER);
        }

        DrawNateLine( pixBase, clipRect, x, bounds->top, x, bounds->bottom, color);
        division += level;
        division &= 0x0000000f;
        x += h;
    }

    x = corner->v;
    x &= size - 1;
    x = size - x;

    division = corner->v + x;
    division >>= kSubSectorShift;
    division &= 0x0000000f;

    x *= scale;
    x >>= SHIFT_SCALE;
    x += bounds->top;

    while ((x < implicit_cast<uint32_t>(bounds->bottom)) && (h > 0)) {
        if ( !division)
        {
            color = GetRGBTranslateColorShade(GREEN, DARKER);
        } else if ( !(division & 0x3))
        {
            color = GetRGBTranslateColorShade(SKY_BLUE, DARKER);
        } else
        {
            color = GetRGBTranslateColorShade(BLUE, DARKER);
        }

        DrawNateLine( pixBase, clipRect, bounds->left, x, bounds->right, x, color);

        division += level;
        division &= 0x0000000f;

        x += h;
    }
}

void GetArbitrarySingleSectorBounds(coordPointType *corner, coordPointType *location,
        int32_t scale, int32_t minSectorSize, Rect *bounds, Rect *destRect) {
    uint32_t    size, level, x, h, division, scaledLoc;
    Rect        clipRect;

    clipRect.left = bounds->left;
    clipRect.right = bounds->right;
    clipRect.top = bounds->top;
    clipRect.bottom = bounds->bottom;

    destRect->left = bounds->left;
    destRect->right = bounds->right;
    destRect->top = bounds->top;
    destRect->bottom = bounds->bottom;

    size = kSubSectorSize >> 2L;
    level = 1;
    do
    {
        level <<= 1L;
        size <<= 2L;
        h = size;
        h *= scale;
        h >>= SHIFT_SCALE;
    } while (h < implicit_cast<uint32_t>(minSectorSize));
    level >>= 1L;
    level *= level;

    x = corner->h;
    x &= size - 1;
    x = size - x;

    division = corner->h + x;
    division >>= kSubSectorShift;
    division &= 0x0000000f;

    x *= scale;
    x >>= SHIFT_SCALE;
    x += bounds->left;

    scaledLoc = location->h - corner->h;
    scaledLoc *= scale;
    scaledLoc >>= SHIFT_SCALE;
    scaledLoc += bounds->left;

    while ((x < implicit_cast<uint32_t>(bounds->right)) && (h > 0)) {
        division += level;
        division &= 0x0000000f;

        if ((x < scaledLoc) && (x > implicit_cast<uint32_t>(destRect->left))) {
            destRect->left = x;
        }
        if ((x > scaledLoc) && (x < implicit_cast<uint32_t>(destRect->right))) {
            destRect->right = x;
        }

        x += h;
    }

    x = corner->v;
    x &= size - 1;
    x = size - x;

    division = corner->v + x;
    division >>= kSubSectorShift;
    division &= 0x0000000f;

    x *= scale;
    x >>= SHIFT_SCALE;
    x += bounds->top;

    scaledLoc = location->v - corner->v;
    scaledLoc *= scale;
    scaledLoc >>= SHIFT_SCALE;
    scaledLoc += bounds->top;

    while ((x < implicit_cast<uint32_t>(bounds->bottom)) && (h > 0)) {
        division += level;
        division &= 0x0000000f;

        if ((x < scaledLoc) && (x > implicit_cast<uint32_t>(destRect->top))) {
            destRect->top = x;
        }
        if ((x > scaledLoc) && (x < implicit_cast<uint32_t>(destRect->bottom))) {
            destRect->bottom = x;
        }

        x += h;
    }
}

static void draw_bar_indicator(int16_t which, int32_t value, int32_t max) {
    if (value > max) {
        value = max;
    }

    int32_t graphicValue;
    globals()->gBarIndicator[which].lastValue = globals()->gBarIndicator[which].thisValue;
    if (max > 0) {
        graphicValue = (kBarIndicatorHeight * value) / max;
        if (graphicValue < 0) {
            graphicValue = 0;
        } else if (graphicValue > kBarIndicatorHeight) {
            graphicValue = kBarIndicatorHeight;
        }
    } else {
        graphicValue = 0;
    }

    RgbColor        color, lightColor, darkColor;
    Rect            rrect;

    int8_t hue = globals()->gBarIndicator[which].color;
    Rect bar(0, 0, kBarIndicatorWidth, kBarIndicatorHeight);
    bar.offset(
            kBarIndicatorLeft + play_screen.right,
            globals()->gBarIndicator[which].top);
    if (graphicValue < kBarIndicatorHeight) {
        Rect top_bar = bar;
        top_bar.bottom = top_bar.bottom - graphicValue;
        const RgbColor fill_color = GetRGBTranslateColorShade(hue, DARK);
        const RgbColor light_color = GetRGBTranslateColorShade(hue, MEDIUM);
        const RgbColor dark_color = GetRGBTranslateColorShade(hue, DARKER);
        draw_shaded_rect(top_bar, fill_color, light_color, dark_color);
    }

    if (graphicValue > 0) {
        Rect bottom_bar = bar;
        bottom_bar.top = bottom_bar.bottom - graphicValue;
        const RgbColor fill_color = GetRGBTranslateColorShade(hue, LIGHTER);
        const RgbColor light_color = GetRGBTranslateColorShade(hue, VERY_LIGHT);
        const RgbColor dark_color = GetRGBTranslateColorShade(hue, MEDIUM);
        draw_shaded_rect(bottom_bar, fill_color, light_color, dark_color);
    }

    globals()->gBarIndicator[which].thisValue = value;
}

void draw_build_time_bar(int32_t value) {
    if (value < 0) {
        return;
    }
    value = kMiniBuildTimeHeight - value;

    const Rect clip = mini_build_time_rect();

    {
        const RgbColor color = GetRGBTranslateColorShade(PALE_PURPLE, MEDIUM);
        draw_vbracket(clip, color);
    }

    Rect bar = clip;
    bar.inset(2, 2);

    {
        const RgbColor color = GetRGBTranslateColorShade(PALE_PURPLE, DARK);
        VideoDriver::driver()->fill_rect(bar, color);
    }

    if (value > 0) {
        bar.top += value;
        const RgbColor color = GetRGBTranslateColorShade(PALE_PURPLE, LIGHT);
        VideoDriver::driver()->fill_rect(bar, color);
    }
}

}  // namespace antares
