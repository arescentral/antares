// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2018 The Antares Authors
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

#include "ui/widget.hpp"

#include "config/gamepad.hpp"
#include "config/keys.hpp"
#include "drawing/interface.hpp"
#include "drawing/text.hpp"

using std::unique_ptr;
using std::vector;

namespace antares {

static const int32_t kInterfaceLargeHBorder  = 13;
static const int32_t kInterfaceSmallHBorder  = 3;
static const int32_t kInterfaceVEdgeHeight   = 1;
static const int32_t kInterfaceVCornerHeight = 2;
static const int32_t kInterfaceVLipHeight    = 1;
static const int32_t kInterfaceHTop          = 2;
static const int32_t kLabelBottomHeight      = 6;
static const int32_t kInterfaceContentBuffer = 2;

static const int32_t kIndicatorVOffset      = 4;
static const int32_t kRadioIndicatorHOffset = 4;
static const int32_t kCheckIndicatorHOffset = 4;

static const int32_t kMaxKeyNameLength =
        4;  // how many chars can be in name of key for plainButton

// DrawInterfaceString:
//  Relies on roman alphabet for upper/lower casing.  NOT WORLD-READY!

static void DrawInterfaceString(
        Point p, pn::string_view s, InterfaceStyle style, const RgbColor& color) {
    interface_font(style).draw(p, s, color);
}

static int16_t GetInterfaceStringWidth(pn::string_view s, InterfaceStyle style) {
    return interface_font(style).string_width(s);
}

// GetInterfaceFontWidth:       -- NOT WORLD-READY! --
//
//  We're not using fontInfo.widMax because we know we're never going to use the ultra-wide
//  characters like &oelig; and the like, and we're not using a mono-spaced font.  Therefore, we're
//  using the width of 'R' which is about as wide as our normal letters get.
//

static int16_t GetInterfaceFontWidth(InterfaceStyle style) {
    return interface_font(style).logicalWidth;
}

static int16_t GetInterfaceFontHeight(InterfaceStyle style) {
    return interface_font(style).height;
}

static int16_t GetInterfaceFontAscent(InterfaceStyle style) {
    return interface_font(style).ascent;
}

inline void mDrawPuffUpRect(const Rects& rects, Rect r, Hue hue, int mshade) {
    const RgbColor color = GetRGBTranslateColorShade(hue, mshade);
    rects.fill(r, color);
    const RgbColor lighter = GetRGBTranslateColorShade(hue, mshade + kLighterColor);
    rects.fill(Rect(r.left, r.top, r.left + 1, r.bottom), lighter);
    rects.fill(Rect(r.left, r.top, r.right - 1, r.top + 1), lighter);
    const RgbColor darker = GetRGBTranslateColorShade(hue, mshade + kDarkerColor);
    rects.fill(Rect(r.right - 1, r.top, r.right, r.bottom), darker);
    rects.fill(Rect(r.left + 1, r.bottom - 1, r.right, r.bottom), darker);
}

inline void mDrawPuffDownRect(const Rects& rects, Rect r, Hue hue, int mshade) {
    rects.fill(r, RgbColor::black());
    const RgbColor darker = GetRGBTranslateColorShade(hue, mshade + kDarkerColor);
    rects.fill(Rect(r.left - 1, r.top - 1, r.left, r.bottom + 1), darker);
    rects.fill(Rect(r.left - 1, r.top - 1, r.right, r.top), darker);
    const RgbColor lighter = GetRGBTranslateColorShade(hue, mshade + kLighterColor);
    rects.fill(Rect(r.right, r.top - 1, r.right + 1, r.bottom + 1), lighter);
    rects.fill(Rect(r.left, r.bottom, r.right + 1, r.bottom + 1), lighter);
}

static void mDrawPuffUpTopBorder(const Rects& rects, Rect r, Hue hue, int shade, int h_border) {
    // For historical reasons, this function assumes r has closed intervals.
    ++r.right;
    ++r.bottom;

    Rect outer(
            r.left - h_border, r.top - kInterfaceVEdgeHeight - kInterfaceVCornerHeight,
            r.right + h_border, r.top - kInterfaceVLipHeight);
    const RgbColor color = GetRGBTranslateColorShade(hue, shade);
    rects.fill(Rect(outer.left, outer.top, r.left, r.top), color);
    rects.fill(Rect(r.right, outer.top, outer.right, r.top), color);
    rects.fill(Rect(r.left, outer.top, r.right, outer.bottom), color);

    const RgbColor darker = GetRGBTranslateColorShade(hue, shade + kDarkerColor);
    rects.fill(Rect(outer.left, r.top, r.left + 1, r.top + 1), darker);
    rects.fill(Rect(r.left, outer.bottom, r.right, outer.bottom + 1), darker);
    rects.fill(Rect(r.right - 1, r.top, outer.right, r.top + 1), darker);
    rects.fill(Rect(outer.right - 1, outer.top + 1, outer.right, r.top), darker);

    const RgbColor lighter = GetRGBTranslateColorShade(hue, shade + kLighterColor);
    rects.fill(Rect(outer.left, outer.top, outer.left + 1, r.top), lighter);
    rects.fill(Rect(outer.left, outer.top, outer.right, outer.top + 1), lighter);
}

static void mDrawPuffUpBottomBorder(const Rects& rects, Rect r, Hue hue, int shade, int h_border) {
    // For historical reasons, this function assumes r has closed intervals.
    ++r.right;
    ++r.bottom;

    Rect outer(
            r.left - h_border, r.bottom + kInterfaceVLipHeight, r.right + h_border,
            r.bottom + kInterfaceVEdgeHeight + kInterfaceVCornerHeight);

    const RgbColor color = GetRGBTranslateColorShade(hue, shade);
    rects.fill(Rect(outer.left, r.bottom, r.left, outer.bottom), color);
    rects.fill(Rect(r.right, r.bottom, outer.right, outer.bottom), color);
    rects.fill(Rect(r.left, outer.top, r.right, outer.bottom), color);

    const RgbColor lighter = GetRGBTranslateColorShade(hue, shade + kLighterColor);
    rects.fill(Rect(outer.left, r.bottom - 1, outer.left + 1, outer.bottom), lighter);
    rects.fill(Rect(outer.left, r.bottom - 1, r.left + 1, r.bottom), lighter);
    rects.fill(Rect(r.left, outer.top - 1, r.right, outer.top), lighter);
    rects.fill(Rect(r.right - 1, r.bottom - 1, outer.right, r.bottom), lighter);

    const RgbColor darker = GetRGBTranslateColorShade(hue, shade + kDarkerColor);
    rects.fill(Rect(outer.left + 1, outer.bottom - 1, outer.right, outer.bottom), darker);
    rects.fill(Rect(outer.right - 1, r.bottom - 1, outer.right, outer.bottom), darker);
}

static void mDrawPuffUpTBorder(
        const Rects& rects, Rect r, Hue hue, int mshade, int msheight, int h_border) {
    ++r.right;
    ++r.bottom;

    const RgbColor color = GetRGBTranslateColorShade(hue, mshade);
    rects.fill(
            Rect(r.left - h_border, r.top + msheight, r.left + 1,
                 r.top + msheight + kLabelBottomHeight + 1),
            color);
    rects.fill(
            Rect(r.right - 1, r.top + msheight, r.right + h_border,
                 r.top + msheight + kLabelBottomHeight + 1),
            color);
    rects.fill(
            Rect(r.left, r.top + msheight + kInterfaceVLipHeight, r.right,
                 r.top + msheight + kLabelBottomHeight - kInterfaceVLipHeight + 1),
            color);

    const RgbColor lighter = GetRGBTranslateColorShade(hue, mshade + kLighterColor);
    rects.fill(
            Rect(r.left - h_border, r.top + msheight, r.left - h_border + 1,
                 r.top + msheight + kLabelBottomHeight + 1),
            lighter);
    rects.fill(
            Rect(r.left - h_border, r.top + msheight, r.left + 1, r.top + msheight + 1), lighter);
    rects.fill(
            Rect(r.left, r.top + msheight + kInterfaceVLipHeight, r.right,
                 r.top + msheight + kInterfaceVLipHeight + 1),
            lighter);
    rects.fill(
            Rect(r.right - 1, r.top + msheight, r.right + h_border - 1, r.top + msheight + 1),
            lighter);

    const RgbColor darker = GetRGBTranslateColorShade(hue, mshade + kDarkerColor);
    rects.fill(
            Rect(r.left - h_border + 1, r.top + msheight + kLabelBottomHeight, r.left + 1,
                 r.top + msheight + kLabelBottomHeight + 1),
            darker);
    rects.fill(
            Rect(r.left, r.top + msheight + kLabelBottomHeight - kInterfaceVLipHeight, r.right,
                 r.top + msheight + kLabelBottomHeight - kInterfaceVLipHeight + 1),
            darker);
    rects.fill(
            Rect(r.right - 1, r.top + msheight + kLabelBottomHeight, r.right + h_border,
                 r.top + msheight + kLabelBottomHeight + 1),
            darker);
    rects.fill(
            Rect(r.right + h_border - 1, r.top + msheight, r.right + h_border,
                 r.top + msheight + kLabelBottomHeight + 1),
            darker);
}

template <typename T>
static void draw_plain_rect(Point origin, const T& item) {
    Rects          rects;
    Rect           tRect, uRect;
    int16_t        vcenter, thisHBorder = kInterfaceSmallHBorder;
    Hue            color = item.hue;
    InterfaceStyle style = item.style;

    if (style == InterfaceStyle::LARGE) {
        thisHBorder = kInterfaceLargeHBorder;
    }
    tRect = item.bounds;
    tRect.offset(origin.h, origin.v);
    tRect.left -= kInterfaceContentBuffer;
    tRect.top -= kInterfaceContentBuffer;
    tRect.right += kInterfaceContentBuffer;
    tRect.bottom += kInterfaceContentBuffer;

    // top border
    mDrawPuffUpTopBorder(rects, tRect, color, DARK, thisHBorder);
    // bottom border

    mDrawPuffUpBottomBorder(rects, tRect, color, DARK, thisHBorder);

    // main part left border

    vcenter = (tRect.bottom - tRect.top) / 2;

    uRect =
            Rect(tRect.left - thisHBorder, tRect.top + kInterfaceHTop, tRect.left + 1,
                 tRect.top + vcenter - kInterfaceVLipHeight + 1);
    mDrawPuffUpRect(rects, uRect, color, DARKER);

    uRect =
            Rect(tRect.left - thisHBorder, tRect.bottom - vcenter + kInterfaceVLipHeight,
                 tRect.left + 1, tRect.bottom - kInterfaceHTop + 1);
    mDrawPuffUpRect(rects, uRect, color, VERY_DARK);

    // right border

    uRect =
            Rect(tRect.right, tRect.top + kInterfaceHTop, tRect.right + thisHBorder + 1,
                 tRect.top + vcenter - kInterfaceVLipHeight + 1);
    mDrawPuffUpRect(rects, uRect, color, DARKER);

    uRect =
            Rect(tRect.right, tRect.bottom - vcenter + kInterfaceVLipHeight,
                 tRect.right + thisHBorder + 1, tRect.bottom - kInterfaceHTop + 1);
    mDrawPuffUpRect(rects, uRect, color, VERY_DARK);
}

static void draw_labeled_box(Point origin, const BoxRectData& item) {
    Rect     tRect, uRect;
    int16_t  vcenter, swidth, sheight, thisHBorder = kInterfaceSmallHBorder;
    uint8_t  shade;
    RgbColor color;

    if (item.style == InterfaceStyle::LARGE) {
        thisHBorder = kInterfaceLargeHBorder;
    }
    tRect = item.bounds;
    tRect.offset(origin.h, origin.v);
    tRect.left -= kInterfaceContentBuffer;
    tRect.top -= kInterfaceContentBuffer + GetInterfaceFontHeight(item.style) +
                 kInterfaceTextVBuffer * 2 + kLabelBottomHeight;
    tRect.right += kInterfaceContentBuffer;
    tRect.bottom += kInterfaceContentBuffer;

    shade = DARK;

    mDrawPuffUpTopBorder(Rects(), tRect, item.hue, shade, thisHBorder);
    // bottom border

    mDrawPuffUpBottomBorder(Rects(), tRect, item.hue, shade, thisHBorder);

    // draw the string

    pn::string_view s = *item.label;
    swidth            = GetInterfaceStringWidth(s, item.style) + kInterfaceTextHBuffer * 2;
    swidth            = (tRect.right - tRect.left) - swidth;
    sheight           = GetInterfaceFontHeight(item.style) + kInterfaceTextVBuffer * 2;

    uRect =
            Rect(tRect.left + kInterfaceTextHBuffer - 1, tRect.top + kInterfaceHTop,
                 tRect.right - swidth - kInterfaceTextHBuffer + 1,
                 tRect.top + sheight - kInterfaceHTop);
    color = GetRGBTranslateColorShade(item.hue, VERY_DARK);
    Rects().fill(uRect, color);

    color = GetRGBTranslateColorShade(item.hue, LIGHT);

    DrawInterfaceString(
            Point(tRect.left + kInterfaceTextHBuffer,
                  tRect.top + GetInterfaceFontAscent(item.style) + kInterfaceTextVBuffer),
            s, item.style, color);

    // string left border

    shade   = MEDIUM;
    vcenter = sheight / 2;

    uRect =
            Rect(tRect.left - thisHBorder, tRect.top + kInterfaceHTop, tRect.left + 1,
                 tRect.top + sheight - kInterfaceHTop + 1);
    mDrawPuffUpRect(Rects(), uRect, item.hue, shade);

    // string right border

    shade = MEDIUM;
    uRect =
            Rect(tRect.right - swidth, tRect.top + kInterfaceHTop, tRect.right - 2,
                 tRect.top + sheight - kInterfaceHTop + 1);
    mDrawPuffUpRect(Rects(), uRect, item.hue, shade);
    uRect =
            Rect(tRect.right, tRect.top + kInterfaceHTop, tRect.right + thisHBorder + 1,
                 tRect.top + sheight - kInterfaceHTop + 1);
    mDrawPuffUpRect(Rects(), uRect, item.hue, shade);

    // string bottom border

    mDrawPuffUpTBorder(Rects(), tRect, item.hue, DARK, sheight, thisHBorder);

    // main part left border

    tRect.top += sheight + kLabelBottomHeight;

    vcenter = (tRect.bottom - tRect.top) / 2;

    uRect =
            Rect(tRect.left - thisHBorder, tRect.top + kInterfaceHTop, tRect.left + 1,
                 tRect.top + vcenter - kInterfaceVLipHeight + 1);
    mDrawPuffUpRect(Rects(), uRect, item.hue, DARKER);

    uRect =
            Rect(tRect.left - thisHBorder, tRect.bottom - vcenter + kInterfaceVLipHeight,
                 tRect.left + 1, tRect.bottom - kInterfaceHTop + 1);
    mDrawPuffUpRect(Rects(), uRect, item.hue, VERY_DARK);

    // right border

    uRect =
            Rect(tRect.right, tRect.top + kInterfaceHTop, tRect.right + thisHBorder + 1,
                 tRect.top + vcenter - kInterfaceVLipHeight + 1);
    mDrawPuffUpRect(Rects(), uRect, item.hue, DARKER);

    uRect =
            Rect(tRect.right, tRect.bottom - vcenter + kInterfaceVLipHeight,
                 tRect.right + thisHBorder + 1, tRect.bottom - kInterfaceHTop + 1);
    mDrawPuffUpRect(Rects(), uRect, item.hue, VERY_DARK);
}

static Rect initialize_bounds(const InterfaceItemData& item) {
    Rect bounds = item.bounds;
    bounds.left -= kInterfaceContentBuffer;
    bounds.top -= kInterfaceContentBuffer;
    bounds.right += kInterfaceContentBuffer + 1;
    bounds.bottom += kInterfaceContentBuffer + 1;
    return bounds;
}

template <typename T>
static int h_border(T& t) {
    return t.style == InterfaceStyle::LARGE ? kInterfaceLargeHBorder : kInterfaceSmallHBorder;
}

void BoxRect::draw(Point offset, InputMode) const {
    if (data.label.has_value()) {
        draw_labeled_box(offset, data);
    } else {
        draw_plain_rect(offset, data);
    }
}

Rect BoxRect::inner_bounds() const { return data.bounds; }

Rect BoxRect::outer_bounds() const {
    Rect bounds = initialize_bounds(data);
    bounds.left -= h_border(data);
    bounds.right += h_border(data);
    if (data.label.has_value()) {
        bounds.top -= GetInterfaceFontHeight(data.style) + kInterfaceTextVBuffer * 2 +
                      kLabelBottomHeight + kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
    } else {
        bounds.top -= kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
    }
    bounds.bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
    return bounds;
}

void TextRect::draw(Point offset, InputMode) const {
    Rect bounds = data.bounds;
    bounds.offset(offset.h, offset.v);
    draw_text_in_rect(bounds, data.text, data.style, data.hue);
}

Rect TextRect::inner_bounds() const { return data.bounds; }

Rect TextRect::outer_bounds() const {
    Rect bounds = initialize_bounds(data);
    bounds.left -= h_border(data);
    bounds.right += h_border(data);
    bounds.top -= kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
    bounds.bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
    return bounds;
}

void PictureRect::draw(Point offset, InputMode) const {
    Rect bounds = data.bounds;
    bounds.offset(offset.h, offset.v);
    texture.draw(bounds.left, bounds.top);
}

Rect PictureRect::inner_bounds() const { return data.bounds; }

Rect PictureRect::outer_bounds() const {
    Rect bounds = initialize_bounds(data);
    bounds.left -= kInterfaceSmallHBorder;
    bounds.right += kInterfaceSmallHBorder;
    bounds.top -= kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
    bounds.bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
    return bounds;
}

void PlainButton::draw(Point offset, InputMode mode) const {
    Rect     tRect, uRect, vRect;
    int16_t  swidth, sheight, thisHBorder = kInterfaceSmallHBorder;
    uint8_t  shade;
    RgbColor color;

    {
        Rects rects;
        if (data.style == InterfaceStyle::LARGE) {
            thisHBorder = kInterfaceLargeHBorder;
        }
        tRect = data.bounds;
        tRect.offset(offset.h, offset.v);

        uRect = tRect;
        uRect.right++;
        uRect.bottom++;

        tRect.left -= kInterfaceContentBuffer;
        tRect.top -= kInterfaceContentBuffer;
        tRect.right += kInterfaceContentBuffer;
        tRect.bottom += kInterfaceContentBuffer;

        // top border

        if (state == ButtonState::DISABLED) {
            shade = VERY_DARK;
        } else {
            shade = MEDIUM;
        }

        mDrawPuffUpTopBorder(rects, tRect, data.hue, shade, thisHBorder);
        // bottom border

        mDrawPuffUpBottomBorder(rects, tRect, data.hue, shade, thisHBorder);

        // side border top

        uRect =
                Rect(tRect.left - thisHBorder, tRect.top + kInterfaceHTop, tRect.left + 1,
                     tRect.bottom - kInterfaceHTop + 1);
        vRect =
                Rect(tRect.right, tRect.top + kInterfaceHTop, tRect.right + thisHBorder + 1,
                     tRect.bottom - kInterfaceHTop + 1);
        if (state == ButtonState::ACTIVE) {
            shade = LIGHT;
            mDrawPuffUpRect(rects, uRect, data.hue, shade);
            mDrawPuffUpRect(rects, vRect, data.hue, shade);
        } else {
            if (state == ButtonState::DISABLED) {
                shade = VERY_DARK;
            } else {
                shade = MEDIUM + kSlightlyLighterColor;
            }
            mDrawPuffUpRect(rects, uRect, data.hue, shade);
            mDrawPuffUpRect(rects, vRect, data.hue, shade);
        }
    }

    bool       draw_shortcut = false;
    pn::string shortcut_text;
    if ((mode == KEYBOARD_MOUSE) && data.key) {
        draw_shortcut = true;
        GetKeyNumName(data.key, shortcut_text);
    } else if ((mode == GAMEPAD) && data.gamepad) {
        draw_shortcut = true;
        Gamepad::name(data.gamepad, shortcut_text);
    }

    if (!draw_shortcut) {
        uRect =
                Rect(tRect.left + kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
                     tRect.left + kInterfaceContentBuffer, tRect.bottom - kInterfaceContentBuffer);

        if (state == ButtonState::ACTIVE)
            shade = LIGHT;
        else
            shade = DARK;  // DARKEST + kSlightlyLighterColor;
        uRect =
                Rect(tRect.left + kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
                     tRect.right - kInterfaceContentBuffer + 1,
                     tRect.bottom - kInterfaceContentBuffer + 1);

        color = GetRGBTranslateColorShade(data.hue, shade);
        Rects().fill(uRect, color);

        if (state == ButtonState::ACTIVE) {
            color = GetRGBTranslateColorShade(data.hue, DARKEST);
        } else if (state == ButtonState::DISABLED) {
            color = GetRGBTranslateColorShade(data.hue, VERY_DARK);
        } else {
            color = GetRGBTranslateColorShade(data.hue, LIGHTER);
        }
        swidth  = GetInterfaceStringWidth(data.label, data.style);
        swidth  = tRect.left + (tRect.right - tRect.left) / 2 - swidth / 2;
        sheight = GetInterfaceFontAscent(data.style) + kInterfaceTextVBuffer + tRect.top;
        DrawInterfaceString(Point(swidth, sheight), data.label, data.style, color);
    } else {
        // draw the key code
        {
            Rects rects;
            if (state == ButtonState::DISABLED)
                shade = VERY_DARK;
            else
                shade = LIGHT;
            swidth = GetInterfaceFontWidth(data.style) * kMaxKeyNameLength;

            uRect = Rect(
                    tRect.left + kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
                    tRect.left + kInterfaceContentBuffer + swidth + kInterfaceTextHBuffer * 2 + 1,
                    tRect.bottom - kInterfaceContentBuffer + 1);
            mDrawPuffUpRect(rects, uRect, data.hue, shade);

            if (state == ButtonState::ACTIVE)
                shade = LIGHT;
            else
                shade = DARK;  // DARKEST;
            vRect = Rect(
                    tRect.left + kInterfaceContentBuffer + swidth + kInterfaceTextHBuffer * 2 + 2,
                    tRect.top + kInterfaceContentBuffer, tRect.right - kInterfaceContentBuffer + 1,
                    tRect.bottom - kInterfaceContentBuffer + 1);
            color = GetRGBTranslateColorShade(data.hue, shade);
            rects.fill(vRect, color);

            swidth = GetInterfaceStringWidth(shortcut_text, data.style);
            swidth = uRect.left + (uRect.right - uRect.left) / 2 - swidth / 2;
            if (state == ButtonState::DISABLED) {
                color = GetRGBTranslateColorShade(data.hue, VERY_DARK);
            } else {
                color = GetRGBTranslateColorShade(data.hue, DARKEST);
            }
        }

        DrawInterfaceString(
                Point(swidth, uRect.top + GetInterfaceFontAscent(data.style)), shortcut_text,
                data.style, color);

        // draw the button title
        {
            if (state == ButtonState::ACTIVE) {
                color = GetRGBTranslateColorShade(data.hue, DARKEST);
            } else if (state == ButtonState::DISABLED) {
                color = GetRGBTranslateColorShade(data.hue, DARKEST + kSlightlyLighterColor);
            } else {
                color = GetRGBTranslateColorShade(data.hue, LIGHTER);
            }

            pn::string_view s = data.label;
            swidth            = GetInterfaceStringWidth(s, data.style);
            swidth            = uRect.right + (tRect.right - uRect.right) / 2 - swidth / 2;
            sheight = GetInterfaceFontAscent(data.style) + kInterfaceTextVBuffer + tRect.top;
            DrawInterfaceString(Point(swidth, sheight), s, data.style, color);
        }
    }
}

Rect PlainButton::inner_bounds() const { return data.bounds; }

Rect PlainButton::outer_bounds() const {
    Rect bounds = initialize_bounds(data);
    bounds.left -= h_border(data);
    bounds.right += h_border(data);
    bounds.top -= kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
    bounds.bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
    return bounds;
}

const PlainButtonData* PlainButton::item() const { return &data; }

void CheckboxButton::draw(Point offset, InputMode) const {
    Rect     tRect, uRect, vRect, wRect;
    int16_t  swidth, sheight, thisHBorder = kInterfaceSmallHBorder;
    uint8_t  shade;
    RgbColor color;

    if (data.style == InterfaceStyle::LARGE)
        thisHBorder = kInterfaceLargeHBorder;
    tRect = data.bounds;
    tRect.offset(offset.h, offset.v);

    tRect.left -= kInterfaceContentBuffer;
    tRect.top -= kInterfaceContentBuffer;
    tRect.right += kInterfaceContentBuffer;
    tRect.bottom += kInterfaceContentBuffer;

    // top border

    if (state == ButtonState::DISABLED)
        shade = VERY_DARK;
    else
        shade = MEDIUM;

    mDrawPuffUpTopBorder(Rects(), tRect, data.hue, shade, thisHBorder);
    // bottom border

    mDrawPuffUpBottomBorder(Rects(), tRect, data.hue, shade, thisHBorder);

    // side border top

    swidth = (tRect.top - kInterfaceVEdgeHeight - kInterfaceVCornerHeight + kIndicatorVOffset);
    sheight =
            (tRect.bottom + kInterfaceVEdgeHeight + kInterfaceVCornerHeight - kIndicatorVOffset) -
            swidth;

    wRect =
            Rect(tRect.left - thisHBorder - kCheckIndicatorHOffset - sheight, swidth,
                 tRect.left - thisHBorder - kCheckIndicatorHOffset + 1, swidth + sheight + 1);

    uRect =
            Rect(tRect.left - thisHBorder - kCheckIndicatorHOffset + 2, tRect.top + kInterfaceHTop,
                 tRect.left + 1, tRect.bottom - kInterfaceHTop + 1);
    vRect =
            Rect(tRect.right, tRect.top + kInterfaceHTop, tRect.right + thisHBorder + 1,
                 tRect.bottom - kInterfaceHTop + 1);

    if (state == ButtonState::ACTIVE) {
        shade = LIGHT;
        mDrawPuffUpRect(Rects(), uRect, data.hue, shade);
        mDrawPuffUpRect(Rects(), vRect, data.hue, shade);
        mDrawPuffUpRect(Rects(), wRect, data.hue, shade);
        wRect.inset(3, 3);
        mDrawPuffDownRect(Rects(), wRect, data.hue, shade);
        wRect.inset(1, 1);
        if (!on) {
            color = RgbColor::black();
        } else {
            color = GetRGBTranslateColorShade(data.hue, VERY_LIGHT);
        }
        Rects().fill(wRect, color);
    } else {
        if (state == ButtonState::DISABLED)
            shade = VERY_DARK;
        else
            shade = MEDIUM + kSlightlyLighterColor;
        mDrawPuffUpRect(Rects(), uRect, data.hue, shade);
        mDrawPuffUpRect(Rects(), vRect, data.hue, shade);
        mDrawPuffUpRect(Rects(), wRect, data.hue, shade);
        wRect.inset(3, 3);
        mDrawPuffDownRect(Rects(), wRect, data.hue, shade);
        wRect.inset(1, 1);
        if (!on) {
            color = RgbColor::black();
        } else if (state == ButtonState::ENABLED) {
            color = GetRGBTranslateColorShade(data.hue, LIGHT);
        } else {
            color = GetRGBTranslateColorShade(data.hue, MEDIUM);
        }
        Rects().fill(wRect, color);
    }

    uRect =
            Rect(tRect.left + kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
                 tRect.left + kInterfaceContentBuffer, tRect.bottom - kInterfaceContentBuffer);

    if (state == ButtonState::ACTIVE)
        shade = LIGHT;
    else
        shade = DARKEST + kSlightlyLighterColor;
    uRect = Rect(
            tRect.left + kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
            tRect.right - kInterfaceContentBuffer + 1, tRect.bottom - kInterfaceContentBuffer + 1);
    color = GetRGBTranslateColorShade(data.hue, shade);
    Rects().fill(uRect, color);

    if (state == ButtonState::ACTIVE) {
        color = GetRGBTranslateColorShade(data.hue, DARKEST);
    } else if (state == ButtonState::DISABLED) {
        color = GetRGBTranslateColorShade(data.hue, DARK);
    } else {
        color = GetRGBTranslateColorShade(data.hue, LIGHT);
    }

    pn::string_view s = data.label;
    swidth            = GetInterfaceStringWidth(s, data.style);
    swidth            = tRect.left + (tRect.right - tRect.left) / 2 - swidth / 2;
    sheight           = GetInterfaceFontAscent(data.style) + kInterfaceTextVBuffer + tRect.top;
    DrawInterfaceString(Point(swidth, sheight), s, data.style, color);
}

Rect CheckboxButton::inner_bounds() const { return data.bounds; }

Rect CheckboxButton::outer_bounds() const {
    Rect bounds = initialize_bounds(data);
    bounds.left -= bounds.bottom - bounds.top + 2 * kInterfaceVEdgeHeight +
                   2 * kInterfaceVCornerHeight - 2 * kIndicatorVOffset + h_border(data) +
                   kCheckIndicatorHOffset;
    bounds.right += h_border(data);
    bounds.top -= kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
    bounds.bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
    return bounds;
}

const CheckboxButtonData* CheckboxButton::item() const { return &data; }

void RadioButton::draw(Point offset, InputMode) const {
    /*
    Rect     tRect, uRect, vRect, wRect;
    int16_t  vcenter, swidth, sheight, thisHBorder = kInterfaceSmallHBorder;
    uint8_t  shade;
    RgbColor color;

    if (item.style == InterfaceStyle::LARGE)
        thisHBorder = kInterfaceLargeHBorder;
    tRect = bounds;

    tRect.left -= kInterfaceContentBuffer;
    tRect.top -= kInterfaceContentBuffer;
    tRect.right += kInterfaceContentBuffer;
    tRect.bottom += kInterfaceContentBuffer;

    // top border

    if (state == ButtonState::DISABLED)
        shade = VERY_DARK;
    else
        shade = MEDIUM;

    mDrawPuffUpTopBorder(tRect, uRect, item.hue, shade, thisHBorder, pix);
    // bottom border

    mDrawPuffUpBottomBorder(tRect, uRect, item.hue, shade, thisHBorder, pix);

    // side border top

    vcenter = (tRect.bottom - tRect.top) / 2;
    swidth  = (tRect.top - kInterfaceVEdgeHeight - kInterfaceVCornerHeight + kIndicatorVOffset);
    sheight =
            (tRect.bottom + kInterfaceVEdgeHeight + kInterfaceVCornerHeight - kIndicatorVOffset) -
            swidth;

    wRect =
            Rect(tRect.left - thisHBorder - kCheckIndicatorHOffset - sheight, swidth,
                 tRect.left - thisHBorder - kCheckIndicatorHOffset + 1, swidth + sheight + 1);

    uRect =
            Rect(tRect.left - thisHBorder - kCheckIndicatorHOffset - 2, tRect.top + kInterfaceHTop,
                 tRect.left + 1,
                 // tRect.top + vcenter - kInterfaceVLipHeight + 1
                 tRect.bottom - kInterfaceHTop + 1);
    vRect =
            Rect(tRect.right, tRect.top + kInterfaceHTop, tRect.right + thisHBorder + 1,
                 // tRect.top + vcenter - kInterfaceVLipHeight + 1
                 tRect.bottom - kInterfaceHTop + 1);

    if (state == ButtonState::ACTIVE) {
        shade = LIGHT;
        mDrawPuffUpRect(uRect, item.hue, shade, pix);
        mDrawPuffUpRect(vRect, item.hue, shade, pix);

        wRect.left += 2;
        wRect.right += 2;
        FrameOval(pix, wRect, RgbColor::black());
        wRect.left -= 2;
        wRect.right -= 2;
        mDrawPuffUpOval(wRect, item.hue, shade, pix);

        wRect.inset(3, 3);
        mDrawPuffDownOval(wRect, item.hue, shade, pix);
        wRect.inset(1, 1);

        if (!on) {
            PaintOval(pix, wRect, RgbColor::black());
        } else {
            const RgbColor color = GetRGBTranslateColorShade(item.hue, VERY_LIGHT);
            PaintOval(pix, wRect, color);
        }
    } else {
        if (state == ButtonState::DISABLED)
            shade = VERY_DARK;
        else
            shade = MEDIUM + kSlightlyLighterColor;
        mDrawPuffUpRect(uRect, item.hue, shade, pix);
        mDrawPuffUpRect(vRect, item.hue, shade, pix);
        wRect.left += 2;
        wRect.right += 2;
        FrameOval(pix, wRect, RgbColor::black());
        wRect.left -= 2;
        wRect.right -= 2;
        mDrawPuffUpOval(wRect, item.hue, shade, pix);

        wRect.inset(3, 3);
        mDrawPuffDownOval(wRect, item.hue, shade, pix);
        wRect.inset(1, 1);
        if (!on) {
            PaintOval(pix, wRect, RgbColor::black());
        } else if (state == kActive) {
            const RgbColor color = GetRGBTranslateColorShade(item.hue, LIGHT);
            PaintOval(pix, wRect, color);
        } else {
            const RgbColor color = GetRGBTranslateColorShade(item.hue, MEDIUM);
            PaintOval(pix, wRect, color);
        }
    }

    uRect =
            Rect(tRect.left + kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
                 tRect.left + kInterfaceContentBuffer, tRect.bottom - kInterfaceContentBuffer);

    if (state == ButtonState::ACTIVE)
        shade = LIGHT;
    else
        shade = DARKEST + kSlightlyLighterColor;
    uRect = Rect(
            tRect.left + kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
            tRect.right - kInterfaceContentBuffer + 1, tRect.bottom - kInterfaceContentBuffer + 1);
    color = GetRGBTranslateColorShade(item.hue, shade);
    pix->view(uRect).fill(color);

    if (state == ButtonState::ACTIVE) {
        color = GetRGBTranslateColorShade(item.hue, DARKEST);
    } else if (state == ButtonState::DISABLED) {
        color = GetRGBTranslateColorShade(item.hue, DARK);
    } else {
        color = GetRGBTranslateColorShade(item.hue, LIGHT);
    }
    StringSlice s = item.label;
    swidth        = GetInterfaceStringWidth(s, item.style);
    swidth        = tRect.left + (tRect.right - tRect.left) / 2 - swidth / 2;
    sheight       = GetInterfaceFontAscent(item.style) + kInterfaceTextVBuffer + tRect.top;
    DrawInterfaceString(Point(swidth, sheight), s, item.style, pix, color);
    */
}

Rect RadioButton::inner_bounds() const { return data.bounds; }

Rect RadioButton::outer_bounds() const {
    Rect bounds = initialize_bounds(data);
    bounds.left -= bounds.bottom - bounds.top + 2 * kInterfaceVEdgeHeight +
                   2 * kInterfaceVCornerHeight - 2 * kIndicatorVOffset + h_border(data) +
                   kRadioIndicatorHOffset;
    bounds.right += h_border(data);
    bounds.top -= kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
    bounds.bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
    return bounds;
}

const RadioButtonData* RadioButton::item() const { return &data; }

void TabBoxButton::draw(Point offset, InputMode) const {
    Rect     tRect;
    int16_t  swidth, sheight, h_border = kInterfaceSmallHBorder;
    uint8_t  shade;
    RgbColor color;

    if (data.style == InterfaceStyle::LARGE) {
        h_border = kInterfaceLargeHBorder;
    }
    tRect = data.bounds;
    tRect.offset(offset.h, offset.v);

    tRect.left -= kInterfaceContentBuffer;
    tRect.top -= kInterfaceContentBuffer;
    tRect.right += kInterfaceContentBuffer;
    tRect.bottom += kInterfaceContentBuffer;

    // top border

    if (state == ButtonState::DISABLED) {
        shade = VERY_DARK;
    } else {
        shade = MEDIUM;
    }

    mDrawPuffUpTopBorder(Rects(), tRect, data.hue, shade, h_border);

    // side border top

    Rect left(
            tRect.left - h_border, tRect.top + kInterfaceHTop, tRect.left + 1,
            tRect.bottom - kInterfaceHTop + 1);
    Rect right(
            tRect.right, tRect.top + kInterfaceHTop, tRect.right + h_border + 1,
            tRect.bottom - kInterfaceHTop + 1);
    if (!on) {
        Rects rects;
        if (state == ButtonState::ACTIVE) {
            shade = LIGHT;
            mDrawPuffUpRect(rects, left, data.hue, shade);
            mDrawPuffUpRect(rects, right, data.hue, shade);
        } else {
            if (state == ButtonState::DISABLED) {
                shade = VERY_DARK;
            } else
                shade = DARK;
            mDrawPuffUpRect(rects, left, data.hue, shade);
            mDrawPuffUpRect(rects, right, data.hue, shade);
        }
        left  = Rect(left.left, left.bottom, left.right, left.bottom + 3);
        right = Rect(right.left, right.bottom, right.right, right.bottom + 3);
        rects.fill(left, RgbColor::black());
        rects.fill(right, RgbColor::black());
        shade = MEDIUM;
        color = GetRGBTranslateColorShade(data.hue, shade);
        rects.fill(Rect(left.left - 3, left.bottom, right.right + 3, left.bottom + 3), color);

        const RgbColor lighter = GetRGBTranslateColorShade(data.hue, shade + kLighterColor);
        rects.fill(Rect(left.left - 3, left.bottom - 1, right.right + 3, left.bottom), lighter);
        const RgbColor darker = GetRGBTranslateColorShade(data.hue, shade + kDarkerColor);
        rects.fill(Rect(left.left - 3, left.bottom + 3, right.right + 3, left.bottom + 4), darker);
    } else {
        Rects rects;
        if (state == ButtonState::ACTIVE) {
            shade = LIGHT;
        } else if (state == ButtonState::DISABLED) {
            shade = VERY_DARK;
        } else {
            shade = MEDIUM;
        }
        left.bottom += 7;
        right.bottom += 7;
        color = GetRGBTranslateColorShade(data.hue, shade);
        rects.fill(left, color);
        rects.fill(right, color);

        const RgbColor lighter = GetRGBTranslateColorShade(data.hue, shade + kLighterColor);
        rects.fill(Rect(left.left, left.top, left.right - 1, left.top + 1), lighter);
        rects.fill(Rect(left.left, left.top, left.left + 1, left.bottom - 5), lighter);
        rects.fill(Rect(left.left - 3, left.bottom - 5, left.left + 1, left.bottom - 4), lighter);
        rects.fill(Rect(right.left, right.top, right.right - 1, right.top + 1), lighter);
        rects.fill(
                Rect(right.right, right.bottom - 5, right.right + 3, right.bottom - 4), lighter);
        rects.fill(Rect(right.left, right.top, right.left + 1, right.bottom - 1), lighter);

        const RgbColor darker = GetRGBTranslateColorShade(data.hue, shade + kDarkerColor);
        rects.fill(Rect(left.left - 3, left.bottom - 1, left.right, left.bottom), darker);
        rects.fill(Rect(left.right - 1, left.top, left.right, left.bottom), darker);
        rects.fill(Rect(right.right - 1, right.top, right.right, right.bottom - 4), darker);
        rects.fill(Rect(right.left, right.bottom - 1, right.right + 3, right.bottom), darker);

        Rect           uRect(left.left - 3, left.bottom - 4, left.right - 1, left.bottom - 1);
        const RgbColor color = GetRGBTranslateColorShade(data.hue, shade);
        rects.fill(uRect, color);
        Rect vRect(right.left + 1, right.bottom - 4, right.right + 3, right.bottom - 1);
        rects.fill(vRect, color);
        uRect.top--;
        uRect.bottom++;
        uRect.left  = uRect.right + 1;
        uRect.right = vRect.left - 1;
        rects.fill(uRect, RgbColor::black());
    }

    if (data.key == 0) {
        Rect uRect(
                tRect.left + kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
                tRect.left + kInterfaceContentBuffer, tRect.bottom - kInterfaceContentBuffer);

        if (on) {
            shade = MEDIUM;
        } else if (state == ButtonState::ACTIVE) {
            shade = LIGHT;
        } else {
            shade = DARKER;  // DARKEST + kSlightlyLighterColor;
        }
        uRect =
                Rect(tRect.left + kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
                     tRect.right - kInterfaceContentBuffer + 1,
                     tRect.bottom - kInterfaceContentBuffer + 1);
        color = GetRGBTranslateColorShade(data.hue, shade);
        Rects().fill(uRect, color);

        if (!on) {
            if (state == ButtonState::ACTIVE) {
                color = GetRGBTranslateColorShade(data.hue, DARKEST);
            } else if (state == ButtonState::DISABLED) {
                color = GetRGBTranslateColorShade(data.hue, VERY_DARK);
            } else {
                color = GetRGBTranslateColorShade(data.hue, LIGHT);
            }
        } else {
            color = GetRGBTranslateColorShade(data.hue, VERY_LIGHT);
        }

        pn::string_view s = data.label;
        swidth            = GetInterfaceStringWidth(s, data.style);
        swidth            = tRect.left + (tRect.right - tRect.left) / 2 - swidth / 2;
        sheight           = GetInterfaceFontAscent(data.style) + kInterfaceTextVBuffer + tRect.top;
        DrawInterfaceString(Point(swidth, sheight), s, data.style, color);
    } else {
        // draw the key code
        if (on) {
            shade = MEDIUM + kLighterColor;
        } else if (state == ButtonState::ACTIVE) {
            shade = VERY_LIGHT;
        } else {
            shade = DARK;  // DARKEST + kSlightlyLighterColor;
        }
        pn::string s;
        GetKeyNumName(data.key, s);
        swidth = GetInterfaceFontWidth(data.style) * kMaxKeyNameLength;

        Rect uRect(
                tRect.left + kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
                tRect.left + kInterfaceContentBuffer + swidth + kInterfaceTextHBuffer * 2 + 1,
                tRect.bottom - kInterfaceContentBuffer + 1);
        mDrawPuffUpRect(Rects(), uRect, data.hue, shade);

        if (on) {
            shade = MEDIUM;
        } else if (state == ButtonState::ACTIVE) {
            shade = VERY_LIGHT;
        } else {
            shade = DARKER;  // DARKEST + kSlightlyLighterColor;
        }
        Rect vRect(
                tRect.left + kInterfaceContentBuffer + swidth + kInterfaceTextHBuffer * 2 + 2,
                tRect.top + kInterfaceContentBuffer, tRect.right - kInterfaceContentBuffer + 1,
                tRect.bottom - kInterfaceContentBuffer + 1);
        color = GetRGBTranslateColorShade(data.hue, shade);
        Rects().fill(vRect, color);

        swidth = GetInterfaceStringWidth(s, data.style);
        swidth = uRect.left + (uRect.right - uRect.left) / 2 - swidth / 2;
        if (state == ButtonState::DISABLED) {
            color = GetRGBTranslateColorShade(data.hue, VERY_DARK);
        } else {
            color = GetRGBTranslateColorShade(data.hue, DARKEST);
        }

        DrawInterfaceString(
                Point(swidth, uRect.top + GetInterfaceFontAscent(data.style)), s, data.style,
                color);

        // draw the button title
        if (!on) {
            if (state == ButtonState::ACTIVE) {
                color = GetRGBTranslateColorShade(data.hue, DARKEST);
            } else if (state == ButtonState::DISABLED) {
                color = GetRGBTranslateColorShade(data.hue, VERY_DARK);
            } else {
                color = GetRGBTranslateColorShade(data.hue, LIGHT);
            }
        } else {
            color = GetRGBTranslateColorShade(data.hue, VERY_LIGHT);
        }

        {
            pn::string_view s = data.label;
            swidth            = GetInterfaceStringWidth(s, data.style);
            swidth            = uRect.right + (tRect.right - uRect.right) / 2 - swidth / 2;
            sheight = GetInterfaceFontAscent(data.style) + kInterfaceTextVBuffer + tRect.top;
            DrawInterfaceString(Point(swidth, sheight), s, data.style, color);
        }
    }
}

Rect TabBoxButton::inner_bounds() const { return data.bounds; }

Rect TabBoxButton::outer_bounds() const {
    Rect bounds = initialize_bounds(data);
    bounds.left -= h_border(data) + 5;
    bounds.right += h_border(data) + 5;
    bounds.top -= kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
    bounds.bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight + 2;
    return bounds;
}

const TabBoxButtonData* TabBoxButton::item() const { return &data; }

void TabBox::draw(Point offset, InputMode) const {
    Rects          rects;
    Rect           uRect;
    int16_t        vcenter, h_border = kInterfaceSmallHBorder;
    uint8_t        shade;
    Hue            color                 = data.hue;
    InterfaceStyle style                 = data.style;
    int16_t        top_right_border_size = data.top_right_border_size;

    Rect r = data.bounds;
    r.offset(offset.h, offset.v);
    if (style == InterfaceStyle::LARGE)
        h_border = kInterfaceLargeHBorder;
    r.left -= kInterfaceContentBuffer;
    r.top -= kInterfaceContentBuffer;
    r.right += kInterfaceContentBuffer;
    r.bottom += kInterfaceContentBuffer;
    Rect outer(
            r.left - h_border, r.top - 3 - kInterfaceVCornerHeight, r.right + h_border,
            r.top - kInterfaceVLipHeight);

    // top border
    shade              = MEDIUM;
    const RgbColor rgb = GetRGBTranslateColorShade(color, shade);
    rects.fill(Rect(outer.left, outer.top, r.left, r.top), rgb);
    rects.fill(Rect(r.right, outer.top, outer.right, r.top), rgb);
    rects.fill(Rect(r.left, outer.top, r.left + 6, outer.bottom), rgb);
    rects.fill(Rect(r.right - top_right_border_size, outer.top, r.right, outer.bottom), rgb);

    const RgbColor darker = GetRGBTranslateColorShade(color, shade + kDarkerColor);
    rects.fill(Rect(outer.left, r.top, r.left + 1, r.top + 1), darker);
    rects.fill(Rect(r.left, outer.bottom, r.left + 6, outer.bottom + 1), darker);
    rects.fill(
            Rect(r.right - top_right_border_size, outer.bottom, r.right + 1, outer.bottom + 1),
            darker);
    rects.fill(Rect(r.right, r.top, outer.right + 1, r.top + 1), darker);
    rects.fill(Rect(outer.right, outer.top, outer.right + 1, r.top), darker);

    const RgbColor lighter = GetRGBTranslateColorShade(color, shade + kLighterColor);
    rects.fill(Rect(outer.left, outer.top, outer.left + 1, r.top), lighter);
    rects.fill(Rect(outer.left, outer.top, r.left + 6, outer.top + 1), lighter);
    rects.fill(
            Rect(r.right - top_right_border_size, outer.top, outer.right + 1, outer.top + 1),
            lighter);

    // bottom border

    mDrawPuffUpBottomBorder(rects, r, color, DARK, h_border);

    // main part left border

    vcenter = (r.bottom - r.top) / 2;

    uRect =
            Rect(outer.left, r.top + kInterfaceHTop, r.left + 1,
                 r.top + vcenter - kInterfaceVLipHeight + 1);
    mDrawPuffUpRect(rects, uRect, color, DARKER);

    uRect =
            Rect(outer.left, r.bottom - vcenter + kInterfaceVLipHeight, r.left + 1,
                 r.bottom - kInterfaceHTop + 1);
    mDrawPuffUpRect(rects, uRect, color, VERY_DARK);

    // right border

    uRect =
            Rect(r.right, r.top + kInterfaceHTop, outer.right + 1,
                 r.top + vcenter - kInterfaceVLipHeight + 1);
    mDrawPuffUpRect(rects, uRect, color, DARKER);

    uRect =
            Rect(r.right, r.bottom - vcenter + kInterfaceVLipHeight, outer.right + 1,
                 r.bottom - kInterfaceHTop + 1);
    mDrawPuffUpRect(rects, uRect, color, VERY_DARK);
}

Rect TabBox::inner_bounds() const { return data.bounds; }

Rect TabBox::outer_bounds() const {
    Rect bounds = initialize_bounds(data);
    bounds.left -= h_border(data);
    bounds.right += h_border(data);
    bounds.top -= kInterfaceVEdgeHeight + kInterfaceVCornerHeight + 2;
    bounds.bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
    return bounds;
}

}  // namespace antares
