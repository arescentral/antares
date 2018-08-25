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

#include "game/instruments.hpp"

#include <algorithm>
#include <sfz/sfz.hpp>

#include "data/base-object.hpp"
#include "drawing/color.hpp"
#include "drawing/shapes.hpp"
#include "game/admiral.hpp"
#include "game/cursor.hpp"
#include "game/globals.hpp"
#include "game/minicomputer.hpp"
#include "game/motion.hpp"
#include "game/player-ship.hpp"
#include "game/space-object.hpp"
#include "game/sys.hpp"
#include "lang/defines.hpp"
#include "math/macros.hpp"
#include "math/random.hpp"
#include "math/rotation.hpp"
#include "math/special.hpp"
#include "math/units.hpp"
#include "video/driver.hpp"

using sfz::range;
using std::max;
using std::min;
using std::unique_ptr;

namespace antares {

const int32_t kPanelHeight = 480;

const int32_t kRadarScale   = 50;
const int32_t kRadarRange   = kRadarSize * kRadarScale;
const ticks   kRadarSpeed   = ticks(30);
const int32_t kRadarBlipNum = 50;
const Hue     kRadarColor   = Hue::GREEN;

const int32_t kRadarLeft       = 6;
const int32_t kRadarTop        = 6;
const int32_t kRadarRight      = 116;
const int32_t kRadarBottom     = 116;
const int32_t kRadarCenter     = 55;
const int32_t kRadarColorSteps = 14;

const size_t  kScaleListNum   = 64;
const int32_t kScaleListShift = 6;
const int32_t kSiteDistance   = 200;
const int32_t kSiteSize       = 16;

const int32_t kBarIndicatorHeight = 98;
const int32_t kBarIndicatorWidth  = 9;
const int32_t kBarIndicatorLeft   = 6;

enum {
    kShieldBar     = 0,
    kEnergyBar     = 1,
    kBatteryBar    = 2,
    kFineMoneyBar  = 3,
    kGrossMoneyBar = 4,
};

const int32_t kMinGraphicSectorSize = 90;

const int32_t kGrossMoneyLeft      = 11;
const int32_t kGrossMoneyTop       = 48;
const int32_t kGrossMoneyHBuffer   = 2;
const int32_t kGrossMoneyVBuffer   = 4;
const int32_t kGrossMoneyBarWidth  = 10;
const int32_t kGrossMoneyBarHeight = 5;
const int32_t kGrossMoneyBarNum    = 7;
const Cash    kGrossMoneyBarValue  = Cash{Fixed::from_long(20000)};
const Hue     kGrossMoneyColor     = Hue::YELLOW;

const int32_t kFineMoneyLeft      = 25;
const int32_t kFineMoneyTop       = 48;
const int32_t kFineMoneyHBuffer   = 1;
const int32_t kFineMoneyVBuffer   = 1;
const int32_t kFineMoneyBarWidth  = 2;
const int32_t kFineMoneyBarHeight = 4;
const int32_t kFineMoneyBarNum    = 100;
const Cash    kFineMoneyBarMod    = kGrossMoneyBarValue;
const Cash    kFineMoneyBarValue  = Cash{Fixed::from_long(200)};
const Hue     kFineMoneyColor     = Hue::PALE_GREEN;
const Hue     kFineMoneyNeedColor = Hue::ORANGE;
const Hue     kFineMoneyUseColor  = Hue::SKY_BLUE;

const Cash kMaxMoneyValue{(kGrossMoneyBarValue.amount * kGrossMoneyBarNum) - Fixed::from_val(1)};

Rect mini_build_time_rect() {
    Rect result(play_screen().right + 10, 8, play_screen().right + 22, 37);
    result.offset(0, instrument_top());
    return result;
}

const int32_t kMinimumAutoScale = 2;

const int32_t kSectorLineBrightness = DARKER;

namespace {

struct barIndicatorType {
    int16_t top;
    int32_t thisValue;
    Hue     hue;
};

static ANTARES_GLOBAL coordPointType gLastGlobalCorner;
static ANTARES_GLOBAL unique_ptr<int32_t[]> gScaleList;
static ANTARES_GLOBAL int32_t gWhichScaleNum;
static ANTARES_GLOBAL int32_t gLastScale;
static ANTARES_GLOBAL bool    should_draw_sector_lines = false;
static ANTARES_GLOBAL Rect view_range;
static ANTARES_GLOBAL barIndicatorType gBarIndicator[kBarIndicatorNum];

struct SiteData {
    bool     should_draw;
    Point    a, b, c;
    RgbColor light, dark;
};
static ANTARES_GLOBAL SiteData site;

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
static void draw_build_time_bar();

void InstrumentInit() {
    g.radar_blips.reset(new Point[kRadarBlipNum]);
    gScaleList.reset(new int32_t[kScaleListNum]);
    ResetInstruments();

    site.light = GetRGBTranslateColorShade(Hue::PALE_GREEN, MEDIUM);
    site.dark  = GetRGBTranslateColorShade(Hue::PALE_GREEN, DARKER + kSlightlyDarkerColor);

    MiniScreenInit();
}

int32_t instrument_top() { return (world().height() / 2) - (kPanelHeight / 2); }

void InstrumentCleanup() {
    g.radar_blips.reset();
    MiniScreenCleanup();
}

void ResetInstruments() {
    int32_t *l, i;
    Point*   lp;

    g.radar_count = ticks(0);
    gLastScale = gAbsoluteScale = SCALE_SCALE;
    gWhichScaleNum              = 0;
    gLastGlobalCorner.h = gLastGlobalCorner.v = 0;
    l                                         = gScaleList.get();
    for (i = 0; i < kScaleListNum; i++) {
        *l = SCALE_SCALE;
        l++;
    }

    for (i = 0; i < kBarIndicatorNum; i++) {
        gBarIndicator[i].thisValue = -1;
    }
    // the shield bar
    gBarIndicator[kShieldBar].top = 359;
    gBarIndicator[kShieldBar].hue = Hue::SKY_BLUE;

    gBarIndicator[kEnergyBar].top = 231;
    gBarIndicator[kEnergyBar].hue = Hue::GOLD;

    gBarIndicator[kBatteryBar].top = 103;
    gBarIndicator[kBatteryBar].hue = Hue::SALMON;

    lp = g.radar_blips.get();
    for (i = 0; i < kRadarBlipNum; i++) {
        lp->h = -1;
        lp++;
    }
}

void UpdateRadar(ticks unitsDone) {
    if (!g.ship.get()) {
        g.radar_on = false;
    } else if (g.ship->offlineTime <= 0) {
        g.radar_on = true;
    } else {
        g.radar_on = (Randomize(g.ship->offlineTime) < 5);
    }

    if (unitsDone < ticks(0)) {
        unitsDone = ticks(0);
    }
    g.radar_count -= unitsDone;

    if (!g.ship.get() || !g.ship->active) {
        return;
    }

    Rect bounds(kRadarLeft, kRadarTop, kRadarRight, kRadarBottom);
    bounds.offset(0, instrument_top());
    bounds.inset(1, 1);

    if (g.radar_on) {
        if (g.radar_count <= ticks(0)) {
            Rect radar = bounds;
            radar.inset(1, 1);

            int32_t dx = g.ship->location.h - gGlobalCorner.h;
            dx         = dx / kRadarScale;
            view_range = Rect(-dx, -dx, dx, dx);
            view_range.center_in(bounds);
            view_range.offset(1, 1);
            view_range.clip_to(radar);

            for (int i = 0; i < kRadarBlipNum; ++i) {
                Point* lp = g.radar_blips.get() + i;
                lp->h     = -1;
            }

            Point* lp     = g.radar_blips.get();
            Point* end    = lp + kRadarBlipNum;
            g.radar_count = kRadarSpeed;

            const int32_t rrange = kRadarRange >> 1L;
            for (auto anObject : SpaceObject::all()) {
                if (!anObject->active || (anObject == g.ship)) {
                    continue;
                }
                int x = anObject->location.h - g.ship->location.h;
                int y = anObject->location.v - g.ship->location.v;
                if ((x < -rrange) || (x >= rrange) || (y < -rrange) || (y >= rrange)) {
                    continue;
                }
                Point p(x * kRadarSize / kRadarRange, y * kRadarSize / kRadarRange);
                p.offset(kRadarCenter + kRadarLeft, kRadarCenter + kRadarTop + instrument_top());
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
    }

    uint32_t bestScale = MIN_SCALE;
    switch (g.zoom) {
        case Zoom::FOE:
        case Zoom::OBJECT: {
            auto     anObject     = g.closest;
            uint64_t hugeDistance = anObject->distanceFromPlayer;
            if (hugeDistance == 0) {  // if this is true, then we haven't calced its distance
                uint64_t x_distance = ABS<int32_t>(g.ship->location.h - anObject->location.h);
                uint64_t y_distance = ABS<int32_t>(g.ship->location.v - anObject->location.v);

                hugeDistance = y_distance * y_distance + x_distance * x_distance;
            }
            bestScale = wsqrt(hugeDistance);
            if (bestScale == 0)
                bestScale = 1;
            bestScale = center_scale().height / bestScale;
            if (bestScale < SCALE_SCALE)
                bestScale = (bestScale >> 2L) + (bestScale >> 1L);
            bestScale = clamp<uint32_t>(bestScale, kMinimumAutoScale, SCALE_SCALE);
        } break;

        case Zoom::ACTUAL: bestScale = SCALE_SCALE; break;

        case Zoom::SIXTEENTH: bestScale = kOneEighthScale; break;

        case Zoom::QUARTER: bestScale = kOneQuarterScale; break;

        case Zoom::HALF: bestScale = kOneHalfScale; break;

        case Zoom::DOUBLE: bestScale = kTimesTwoScale; break;

        case Zoom::ALL: {
            auto     anObject = g.farthest;
            uint64_t tempWide = anObject->distanceFromPlayer;
            bestScale         = wsqrt(tempWide);
            if (bestScale == 0)
                bestScale = 1;
            bestScale = center_scale().height / bestScale;
            if (bestScale < SCALE_SCALE)
                bestScale = (bestScale >> 2L) + (bestScale >> 1L);
            bestScale = clamp<uint32_t>(bestScale, kMinimumAutoScale, SCALE_SCALE);
        } break;
    }

    int32_t* scaleval;
    for (ticks x = ticks(0); x < unitsDone; x++) {
        scaleval  = gScaleList.get() + gWhichScaleNum;
        *scaleval = bestScale;
        gWhichScaleNum++;
        if (gWhichScaleNum == kScaleListNum) {
            gWhichScaleNum = 0;
        }
    }

    scaleval           = gScaleList.get();
    int absolute_scale = 0;
    for (int oCount = 0; oCount < kScaleListNum; oCount++) {
        absolute_scale += *scaleval++;
    }
    absolute_scale >>= kScaleListShift;

    gAbsoluteScale = absolute_scale;
}

void draw_radar() {
    Rect bounds(kRadarLeft, kRadarTop, kRadarRight, kRadarBottom);
    bounds.offset(0, instrument_top());
    bounds.inset(1, 1);

    const RgbColor very_light = GetRGBTranslateColorShade(kRadarColor, LIGHTEST);
    const RgbColor darkest    = GetRGBTranslateColorShade(kRadarColor, DARKEST);
    const RgbColor very_dark  = GetRGBTranslateColorShade(kRadarColor, VERY_DARK);
    if (g.radar_on) {
        Rect radar = bounds;
        {
            Rects rects;
            rects.fill(radar, very_light);
            radar.inset(1, 1);
            rects.fill(radar, darkest);
            if ((view_range.width() > 0) && (view_range.height() > 0)) {
                rects.fill(view_range, very_dark);
            }
        }

        RgbColor color;
        if (g.radar_count <= ticks(0)) {
            color = very_dark;
        } else {
            color = GetRGBTranslateColorShade(
                    kRadarColor, ((kRadarColorSteps * g.radar_count) / kRadarSpeed) + 1);
        }

        Points points;
        for (int rcount = 0; rcount < kRadarBlipNum; rcount++) {
            Point* lp = g.radar_blips.get() + rcount;
            if (lp->h >= 0) {
                points.draw(*lp, color);
            }
        }
    } else {
        Rects().fill(bounds, darkest);
    }
}

// SHOW ME THE MONEY
static void draw_money() {
    auto&      admiral = g.admiral;
    const Cash cash    = clamp(admiral->cash(), Cash{Fixed::zero()}, kMaxMoneyValue);
    gBarIndicator[kFineMoneyBar].thisValue =
            mFixedToLong((cash.amount % kFineMoneyBarMod.amount) / kFineMoneyBarValue.amount);
    const int price = mFixedToLong(
            MiniComputerGetPriceOfCurrentSelection().amount / kFineMoneyBarValue.amount);

    Rect box(0, 0, kFineMoneyBarWidth, kFineMoneyBarHeight - 1);
    box.offset(
            kFineMoneyLeft + kFineMoneyHBuffer + play_screen().right,
            kFineMoneyTop + instrument_top() + kFineMoneyVBuffer);

    // First section of the money bar: when we can afford the current selection, displays the
    // money which will remain after it is purchased.  When we cannot, displays the money we
    // currently have.
    int      first_threshold;
    RgbColor first_color_major;
    RgbColor first_color_minor;

    // Second section of the money bar: when we can afford the current selection, displays the
    // amount which will be deducted after it is purchased.  When we cannot, displays the
    // amount of additional money which we need to amass before it can be purchased.
    int      second_threshold;
    RgbColor second_color_major;
    RgbColor second_color_minor;

    // Third section: money we don't have and don't need for the current selection.
    RgbColor third_color = GetRGBTranslateColorShade(kFineMoneyColor, VERY_DARK);

    if (gBarIndicator[kFineMoneyBar].thisValue < price) {
        first_color_major  = GetRGBTranslateColorShade(kFineMoneyColor, LIGHTEST);
        first_color_minor  = GetRGBTranslateColorShade(kFineMoneyColor, LIGHT);
        second_color_major = GetRGBTranslateColorShade(kFineMoneyNeedColor, MEDIUM);
        second_color_minor = GetRGBTranslateColorShade(kFineMoneyNeedColor, DARK);
        first_threshold    = gBarIndicator[kFineMoneyBar].thisValue;
        second_threshold   = price;
    } else {
        first_color_major  = GetRGBTranslateColorShade(kFineMoneyColor, LIGHTEST);
        first_color_minor  = GetRGBTranslateColorShade(kFineMoneyColor, LIGHT);
        second_color_major = GetRGBTranslateColorShade(kFineMoneyUseColor, LIGHTEST);
        second_color_minor = GetRGBTranslateColorShade(kFineMoneyUseColor, LIGHT);
        first_threshold    = gBarIndicator[kFineMoneyBar].thisValue - price;
        second_threshold   = gBarIndicator[kFineMoneyBar].thisValue;
    }

    Rects rects;
    for (int i = 0; i < kFineMoneyBarNum; ++i) {
        if (i < first_threshold) {
            if ((i % 5) != 0) {
                rects.fill(box, first_color_minor);
            } else {
                rects.fill(box, first_color_major);
            }
        } else if (i < second_threshold) {
            if ((i % 5) != 0) {
                rects.fill(box, second_color_minor);
            } else {
                rects.fill(box, second_color_major);
            }
        } else {
            rects.fill(box, third_color);
        }
        box.offset(0, kFineMoneyBarHeight);
    }
    gBarIndicator[kFineMoneyBar].thisValue = second_threshold;

    barIndicatorType* gross = gBarIndicator + kGrossMoneyBar;
    gross->thisValue        = mFixedToLong(admiral->cash().amount / kGrossMoneyBarValue.amount);

    box = Rect(0, 0, kGrossMoneyBarWidth, kGrossMoneyBarHeight - 1);
    box.offset(
            play_screen().right + kGrossMoneyLeft + kGrossMoneyHBuffer,
            kGrossMoneyTop + instrument_top() + kGrossMoneyVBuffer);

    const RgbColor light = GetRGBTranslateColorShade(kGrossMoneyColor, LIGHTEST);
    const RgbColor dark  = GetRGBTranslateColorShade(kGrossMoneyColor, VERY_DARK);
    for (int i = 0; i < kGrossMoneyBarNum; ++i) {
        if (i < gross->thisValue) {
            rects.fill(box, light);
        } else {
            rects.fill(box, dark);
        }
        box.offset(0, kGrossMoneyBarHeight);
    }
}

void set_up_instruments() {
    g.zoom = Zoom::FOE;

    MiniComputerDoCancel();  // i.e., go to main screen
    ResetInstruments();
    UpdateRadar(ticks(100));  // full update
}

void draw_instruments() {
    Rect left_rect(world().left, world().top, viewport().left, world().bottom);
    Rect right_rect(viewport().right, world().top, world().right, world().bottom);

    left_rect.inset(0, (world().height() - 768) / 2);
    right_rect.inset(0, (world().height() - 768) / 2);

    sys.left_instrument_texture.draw(left_rect.left, left_rect.top);
    sys.right_instrument_texture.draw(right_rect.left, right_rect.top);

    if (g.ship.get() && g.ship->active) {
        const SpaceObject::Weapon& pulse   = g.ship->pulse;
        const SpaceObject::Weapon& beam    = g.ship->beam;
        const SpaceObject::Weapon& special = g.ship->special;
        draw_player_ammo(
                (pulse.base && (pulse.base->device->ammo > 0)) ? pulse.ammo : -1,
                (beam.base && (beam.base->device->ammo > 0)) ? beam.ammo : -1,
                (special.base && (special.base->device->ammo > 0)) ? special.ammo : -1);

        draw_bar_indicator(kShieldBar, g.ship->health(), g.ship->max_health());
        draw_bar_indicator(kEnergyBar, g.ship->energy(), g.ship->max_energy());
        draw_bar_indicator(kBatteryBar, g.ship->battery(), g.ship->max_battery());
    }

    draw_build_time_bar();
    draw_money();
    draw_radar();
    draw_mini_screen();
}

void EraseSite() {}

static void update_triangle(SiteData& site, int32_t direction, int32_t distance, int32_t size) {
    int   count;
    Fixed fa, fb, fc;
    GetRotPoint(&fa, &fb, direction);

    fc = Fixed::from_long(-distance);
    fa = (fc * fa);
    fb = (fc * fb);

    Point a(mFixedToLong(fa), mFixedToLong(fb));
    a.offset(g.ship->sprite->where.h, g.ship->sprite->where.v);
    site.a = a;

    count = direction;
    mAddAngle(count, 30);
    GetRotPoint(&fa, &fb, count);
    fc = Fixed::from_long(size);
    fa = (fc * fa);
    fb = (fc * fb);

    Point b(a.h + mFixedToLong(fa), a.v + mFixedToLong(fb));
    site.b = b;

    count = direction;
    mAddAngle(count, -30);
    GetRotPoint(&fa, &fb, count);
    fc = Fixed::from_long(size);
    fa = (fc * fa);
    fb = (fc * fb);

    Point c(a.h + mFixedToLong(fa), a.v + mFixedToLong(fb));
    site.c = c;
}

void update_site(bool replay) {
    if (!g.ship.get()) {
        site.should_draw = false;
    } else if (!(g.ship->active && g.ship->sprite.get())) {
        site.should_draw = false;
    } else if (g.ship->offlineTime <= 0) {
        site.should_draw = true;
    } else {
        site.should_draw = (Randomize(g.ship->offlineTime) < 5);
    }

    if (site.should_draw) {
        update_triangle(site, g.ship->direction, kSiteDistance, kSiteSize);
    }
}

void draw_site(const PlayerShip& player) {
    if (site.should_draw) {
        Lines lines;
        lines.draw(site.a, site.b, site.light);
        lines.draw(site.a, site.c, site.light);
        lines.draw(site.b, site.c, site.dark);

        SiteData control = {};
        if (player.show_select()) {
            control.light = GetRGBTranslateColorShade(Hue::YELLOW, MEDIUM);
            control.dark  = GetRGBTranslateColorShade(Hue::YELLOW, DARKER + kSlightlyDarkerColor);
            control.should_draw = true;
        } else if (player.show_target()) {
            control.light = GetRGBTranslateColorShade(Hue::SKY_BLUE, MEDIUM);
            control.dark = GetRGBTranslateColorShade(Hue::SKY_BLUE, DARKER + kSlightlyDarkerColor);
            control.should_draw = true;
        }
        if (control.should_draw) {
            update_triangle(control, player.control_direction(), kSiteDistance - 3, kSiteSize - 6);
            lines.draw(control.a, control.b, control.light);
            lines.draw(control.a, control.c, control.light);
            lines.draw(control.b, control.c, control.dark);
        }
    }
}

void update_sector_lines() {
    should_draw_sector_lines = false;
    if (g.ship.get()) {
        if (g.ship->offlineTime <= 0) {
            should_draw_sector_lines = true;
        } else if (Randomize(g.ship->offlineTime) < 5) {
            should_draw_sector_lines = true;
        }
    }

    if ((gLastScale < kBlipThreshhold) != (gAbsoluteScale < kBlipThreshhold)) {
        sys.sound.zoom();
    }

    gLastScale        = gAbsoluteScale;
    gLastGlobalCorner = gGlobalCorner;
}

void draw_sector_lines() {
    Rects    rects;
    int32_t  x;
    uint32_t size, level, h, division;

    size  = kSubSectorSize / 4;
    level = 1;
    do {
        level *= 2;
        size *= 4;
        h = (size * gLastScale) >> SHIFT_SCALE;
    } while (h < kMinGraphicSectorSize);
    level /= 2;
    level *= level;

    x        = size - (gLastGlobalCorner.h & (size - 1));
    division = ((gLastGlobalCorner.h + x) >> kSubSectorShift) & 0x0000000f;
    x        = ((x * gLastScale) >> SHIFT_SCALE) + viewport().left;

    if (should_draw_sector_lines) {
        while ((x < implicit_cast<uint32_t>(viewport().right)) && (h > 0)) {
            RgbColor color;
            if (!division) {
                color = GetRGBTranslateColorShade(Hue::GREEN, kSectorLineBrightness);
            } else if (!(division & 0x3)) {
                color = GetRGBTranslateColorShade(Hue::SKY_BLUE, kSectorLineBrightness);
            } else {
                color = GetRGBTranslateColorShade(Hue::BLUE, kSectorLineBrightness);
            }

            // TODO(sfiera): +1 on bottom no longer needed.
            rects.fill({x, viewport().top, x + 1, viewport().bottom + 1}, color);
            division += level;
            division &= 0x0000000f;
            x += h;
        }
    }

    x        = size - (gLastGlobalCorner.v & (size - 1));
    division = ((gLastGlobalCorner.v + x) >> kSubSectorShift) & 0x0000000f;
    x        = ((x * gLastScale) >> SHIFT_SCALE) + viewport().top;

    if (should_draw_sector_lines) {
        while ((x < implicit_cast<uint32_t>(viewport().bottom)) && (h > 0)) {
            RgbColor color;
            if (!division) {
                color = GetRGBTranslateColorShade(Hue::GREEN, kSectorLineBrightness);
            } else if (!(division & 0x3)) {
                color = GetRGBTranslateColorShade(Hue::SKY_BLUE, kSectorLineBrightness);
            } else {
                color = GetRGBTranslateColorShade(Hue::BLUE, kSectorLineBrightness);
            }

            // TODO(sfiera): +1 on right no longer needed.
            rects.fill({viewport().left, x, viewport().right + 1, x + 1}, color);

            division += level;
            division &= 0x0000000f;

            x += h;
        }
    }
}

void InstrumentsHandleClick(const GameCursor& cursor) {
    const Point where = cursor.clamped_location();
    PlayerShipHandleClick(where, 0);
    MiniComputerHandleClick(where);
}

void InstrumentsHandleDoubleClick(const GameCursor& cursor) {
    const Point where = cursor.clamped_location();
    PlayerShipHandleClick(where, 0);
    MiniComputerHandleDoubleClick(where);
}

void InstrumentsHandleMouseUp(const GameCursor& cursor) {
    const Point where = cursor.clamped_location();
    MiniComputerHandleMouseUp(where);
}

void InstrumentsHandleMouseStillDown(const GameCursor& cursor) {
    const Point where = cursor.clamped_location();
    MiniComputerHandleMouseStillDown(where);
}

void draw_arbitrary_sector_lines(
        const coordPointType& corner, int32_t scale, int32_t minSectorSize, const Rect& bounds) {
    Rects    rects;
    uint32_t size, level, h, division;
    int32_t  x;
    RgbColor color;

    size  = kSubSectorSize >> 2L;
    level = 1;
    do {
        level <<= 1L;
        size <<= 2L;
        h = size;
        h *= scale;
        h >>= SHIFT_SCALE;
    } while (h < implicit_cast<uint32_t>(minSectorSize));
    level >>= 1L;
    level *= level;

    x = corner.h;
    x &= size - 1;
    x = size - x;

    division = corner.h + x;
    division >>= kSubSectorShift;
    division &= 0x0000000f;

    x *= scale;
    x >>= SHIFT_SCALE;
    x += bounds.left;

    while ((x < implicit_cast<uint32_t>(bounds.right)) && (h > 0)) {
        if (!division) {
            color = GetRGBTranslateColorShade(Hue::GREEN, DARKER);
        } else if (!(division & 0x3)) {
            color = GetRGBTranslateColorShade(Hue::SKY_BLUE, DARKER);
        } else {
            color = GetRGBTranslateColorShade(Hue::BLUE, DARKER);
        }

        rects.fill({x, bounds.top, x + 1, bounds.bottom}, color);
        division += level;
        division &= 0x0000000f;
        x += h;
    }

    x = corner.v;
    x &= size - 1;
    x = size - x;

    division = corner.v + x;
    division >>= kSubSectorShift;
    division &= 0x0000000f;

    x *= scale;
    x >>= SHIFT_SCALE;
    x += bounds.top;

    while ((x < implicit_cast<uint32_t>(bounds.bottom)) && (h > 0)) {
        if (!division) {
            color = GetRGBTranslateColorShade(Hue::GREEN, DARKER);
        } else if (!(division & 0x3)) {
            color = GetRGBTranslateColorShade(Hue::SKY_BLUE, DARKER);
        } else {
            color = GetRGBTranslateColorShade(Hue::BLUE, DARKER);
        }

        rects.fill({bounds.left, x, bounds.right, x + 1}, color);
        division += level;
        division &= 0x0000000f;
        x += h;
    }
}

void GetArbitrarySingleSectorBounds(
        coordPointType* corner, coordPointType* location, int32_t scale, int32_t minSectorSize,
        Rect* bounds, Rect* destRect) {
    uint32_t size, level, x, h, division, scaledLoc;
    Rect     clipRect;

    clipRect.left   = bounds->left;
    clipRect.right  = bounds->right;
    clipRect.top    = bounds->top;
    clipRect.bottom = bounds->bottom;

    destRect->left   = bounds->left;
    destRect->right  = bounds->right;
    destRect->top    = bounds->top;
    destRect->bottom = bounds->bottom;

    size  = kSubSectorSize >> 2L;
    level = 1;
    do {
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
    Rects rects;
    if (value > max) {
        value = max;
    }

    int32_t graphicValue;
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

    Hue  hue = gBarIndicator[which].hue;
    Rect bar(0, 0, kBarIndicatorWidth, kBarIndicatorHeight);
    bar.offset(
            kBarIndicatorLeft + play_screen().right, gBarIndicator[which].top + instrument_top());
    if (graphicValue < kBarIndicatorHeight) {
        Rect top_bar               = bar;
        top_bar.bottom             = top_bar.bottom - graphicValue;
        const RgbColor fill_color  = GetRGBTranslateColorShade(hue, DARK);
        const RgbColor light_color = GetRGBTranslateColorShade(hue, MEDIUM);
        const RgbColor dark_color  = GetRGBTranslateColorShade(hue, DARKER);
        draw_shaded_rect(rects, top_bar, fill_color, light_color, dark_color);
    }

    if (graphicValue > 0) {
        Rect bottom_bar            = bar;
        bottom_bar.top             = bottom_bar.bottom - graphicValue;
        const RgbColor fill_color  = GetRGBTranslateColorShade(hue, LIGHTER);
        const RgbColor light_color = GetRGBTranslateColorShade(hue, LIGHTEST);
        const RgbColor dark_color  = GetRGBTranslateColorShade(hue, MEDIUM);
        draw_shaded_rect(rects, bottom_bar, fill_color, light_color, dark_color);
    }

    gBarIndicator[which].thisValue = value;
}

void draw_build_time_bar() {
    auto build_at = GetAdmiralBuildAtObject(g.admiral);
    if (!build_at.get()) {
        return;
    }

    int32_t value = 0;
    if (build_at->totalBuildTime > ticks(0)) {
        value = build_at->buildTime * kMiniBuildTimeHeight / build_at->totalBuildTime;
    }

    Rects rects;
    value = kMiniBuildTimeHeight - value;

    const Rect clip = mini_build_time_rect();

    {
        const RgbColor color = GetRGBTranslateColorShade(Hue::PALE_PURPLE, MEDIUM);
        draw_vbracket(rects, clip, color);
    }

    Rect bar = clip;
    bar.inset(2, 2);

    {
        const RgbColor color = GetRGBTranslateColorShade(Hue::PALE_PURPLE, DARK);
        rects.fill(bar, color);
    }

    if (value > 0) {
        bar.top += value;
        const RgbColor color = GetRGBTranslateColorShade(Hue::PALE_PURPLE, LIGHT);
        rects.fill(bar, color);
    }
}

}  // namespace antares
