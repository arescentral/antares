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

#include "drawing/interface.hpp"

#include <sfz/sfz.hpp>

#include "config/gamepad.hpp"
#include "config/keys.hpp"
#include "data/interface.hpp"
#include "drawing/color.hpp"
#include "drawing/styled-text.hpp"
#include "drawing/text.hpp"
#include "game/sys.hpp"
#include "video/driver.hpp"

using sfz::BytesSlice;
using sfz::Exception;
using sfz::String;
using sfz::StringSlice;
using sfz::format;
using std::unique_ptr;
using std::vector;

namespace utf8 = sfz::utf8;

namespace antares {

namespace {

const int32_t kInterfaceLargeHBorder  = 13;
const int32_t kInterfaceSmallHBorder  = 3;
const int32_t kInterfaceVEdgeHeight   = 1;
const int32_t kInterfaceVCornerHeight = 2;
const int32_t kInterfaceVLipHeight    = 1;
const int32_t kInterfaceHTop          = 2;
const int32_t kLabelBottomHeight      = 6;
const int32_t kInterfaceContentBuffer = 2;

const int32_t kIndicatorVOffset      = 4;
const int32_t kRadioIndicatorHOffset = 4;
const int32_t kCheckIndicatorHOffset = 4;

const int32_t kMaxKeyNameLength = 4;  // how many chars can be in name of key for plainButton

// DrawInterfaceString:
//  Relies on roman alphabet for upper/lower casing.  NOT WORLD-READY!

const Font* interface_font(interfaceStyleType style) {
    if (style == kSmall) {
        return sys.fonts.small_button;
    } else {
        return sys.fonts.button;
    }
}

void DrawInterfaceString(Point p, StringSlice s, interfaceStyleType style, const RgbColor& color) {
    interface_font(style)->draw(p, s, color);
}

int16_t GetInterfaceStringWidth(const StringSlice& s, interfaceStyleType style) {
    return interface_font(style)->string_width(s);
}

// GetInterfaceFontWidth:       -- NOT WORLD-READY! --
//
//  We're not using fontInfo.widMax because we know we're never going to use the ultra-wide
//  characters like &oelig; and the like, and we're not using a mono-spaced font.  Therefore, we're
//  using the width of 'R' which is about as wide as our normal letters get.
//

int16_t GetInterfaceFontWidth(interfaceStyleType style) {
    return interface_font(style)->logicalWidth;
}

int16_t GetInterfaceFontHeight(interfaceStyleType style) {
    return interface_font(style)->height;
}

int16_t GetInterfaceFontAscent(interfaceStyleType style) {
    return interface_font(style)->ascent;
}

enum inlineKindType { kNoKind = 0, kVPictKind = 1, kVClearPictKind = 2 };

inline void mDrawPuffUpRect(const Rects& rects, Rect r, uint8_t mcolor, int mshade) {
    const RgbColor color = GetRGBTranslateColorShade(mcolor, mshade);
    rects.fill(r, color);
    const RgbColor lighter = GetRGBTranslateColorShade(mcolor, mshade + kLighterColor);
    rects.fill(Rect(r.left, r.top, r.left + 1, r.bottom), lighter);
    rects.fill(Rect(r.left, r.top, r.right - 1, r.top + 1), lighter);
    const RgbColor darker = GetRGBTranslateColorShade(mcolor, mshade + kDarkerColor);
    rects.fill(Rect(r.right - 1, r.top, r.right, r.bottom), darker);
    rects.fill(Rect(r.left + 1, r.bottom - 1, r.right, r.bottom), darker);
}

inline void mDrawPuffDownRect(const Rects& rects, Rect r, uint8_t mcolor, int mshade) {
    rects.fill(r, RgbColor::black());
    const RgbColor darker = GetRGBTranslateColorShade(mcolor, mshade + kDarkerColor);
    rects.fill(Rect(r.left - 1, r.top - 1, r.left, r.bottom + 1), darker);
    rects.fill(Rect(r.left - 1, r.top - 1, r.right, r.top), darker);
    const RgbColor lighter = GetRGBTranslateColorShade(mcolor, mshade + kLighterColor);
    rects.fill(Rect(r.right, r.top - 1, r.right + 1, r.bottom + 1), lighter);
    rects.fill(Rect(r.left, r.bottom, r.right + 1, r.bottom + 1), lighter);
}

inline void mDrawPuffUpTopBorder(
        const Rects& rects, Rect r, uint8_t hue, int shade, int h_border) {
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

inline void mDrawPuffUpBottomBorder(
        const Rects& rects, Rect r, uint8_t hue, int shade, int h_border) {
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

inline void mDrawPuffUpTBorder(
        const Rects& rects, Rect r, uint8_t mcolor, int mshade, int msheight, int h_border) {
    ++r.right;
    ++r.bottom;

    const RgbColor color = GetRGBTranslateColorShade(mcolor, mshade);
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

    const RgbColor lighter = GetRGBTranslateColorShade(mcolor, mshade + kLighterColor);
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

    const RgbColor darker = GetRGBTranslateColorShade(mcolor, mshade + kDarkerColor);
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
void draw_plain_rect(Point origin, const T& item) {
    Rects              rects;
    Rect               tRect, uRect;
    int16_t            vcenter, thisHBorder = kInterfaceSmallHBorder;
    uint8_t            color = item.hue;
    interfaceStyleType style = item.style;

    if (style == kLarge) {
        thisHBorder = kInterfaceLargeHBorder;
    }
    tRect = item.bounds();
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

void draw_tab_box(Point origin, const TabBox& item) {
    Rects              rects;
    Rect               uRect;
    int16_t            vcenter, h_border = kInterfaceSmallHBorder;
    uint8_t            shade;
    uint8_t            color                 = item.hue;
    interfaceStyleType style                 = item.style;
    int16_t            top_right_border_size = item.top_right_border_size;

    Rect r = item.bounds();
    r.offset(origin.h, origin.v);
    if (style == kLarge)
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

void draw_button(Point origin, InputMode mode, const PlainButton& item) {
    Rect     tRect, uRect, vRect;
    int16_t  swidth, sheight, thisHBorder = kInterfaceSmallHBorder;
    uint8_t  shade;
    RgbColor color;

    {
        Rects rects;
        if (item.style == kLarge) {
            thisHBorder = kInterfaceLargeHBorder;
        }
        tRect = item.bounds();
        tRect.offset(origin.h, origin.v);

        uRect = tRect;
        uRect.right++;
        uRect.bottom++;

        tRect.left -= kInterfaceContentBuffer;
        tRect.top -= kInterfaceContentBuffer;
        tRect.right += kInterfaceContentBuffer;
        tRect.bottom += kInterfaceContentBuffer;

        // top border

        if (item.status == kDimmed) {
            shade = VERY_DARK;
        } else {
            shade = MEDIUM;
        }

        mDrawPuffUpTopBorder(rects, tRect, item.hue, shade, thisHBorder);
        // bottom border

        mDrawPuffUpBottomBorder(rects, tRect, item.hue, shade, thisHBorder);

        // side border top

        uRect =
                Rect(tRect.left - thisHBorder, tRect.top + kInterfaceHTop, tRect.left + 1,
                     tRect.bottom - kInterfaceHTop + 1);
        vRect =
                Rect(tRect.right, tRect.top + kInterfaceHTop, tRect.right + thisHBorder + 1,
                     tRect.bottom - kInterfaceHTop + 1);
        if (item.status == kIH_Hilite) {
            shade = LIGHT;
            mDrawPuffUpRect(rects, uRect, item.hue, shade);
            mDrawPuffUpRect(rects, vRect, item.hue, shade);
        } else {
            if (item.status == kDimmed) {
                shade = VERY_DARK;
            } else {
                shade = MEDIUM + kSlightlyLighterColor;
            }
            mDrawPuffUpRect(rects, uRect, item.hue, shade);
            mDrawPuffUpRect(rects, vRect, item.hue, shade);
        }
    }

    bool   draw_shortcut = false;
    String shortcut_text;
    if ((mode == KEYBOARD_MOUSE) && item.key) {
        draw_shortcut = true;
        GetKeyNumName(item.key, &shortcut_text);
    } else if ((mode == GAMEPAD) && item.gamepad) {
        draw_shortcut = true;
        Gamepad::name(item.gamepad, shortcut_text);
    }

    if (!draw_shortcut) {
        uRect =
                Rect(tRect.left + kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
                     tRect.left + kInterfaceContentBuffer, tRect.bottom - kInterfaceContentBuffer);

        if (item.status == kIH_Hilite)
            shade = LIGHT;
        else
            shade = DARK;  // DARKEST + kSlightlyLighterColor;
        uRect =
                Rect(tRect.left + kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
                     tRect.right - kInterfaceContentBuffer + 1,
                     tRect.bottom - kInterfaceContentBuffer + 1);

        color = GetRGBTranslateColorShade(item.hue, shade);
        Rects().fill(uRect, color);

        if (item.status == kIH_Hilite) {
            color = GetRGBTranslateColorShade(item.hue, DARKEST);
        } else if (item.status == kDimmed) {
            color = GetRGBTranslateColorShade(item.hue, VERY_DARK);
        } else {
            color = GetRGBTranslateColorShade(item.hue, LIGHTER);
        }
        swidth  = GetInterfaceStringWidth(item.label, item.style);
        swidth  = tRect.left + (tRect.right - tRect.left) / 2 - swidth / 2;
        sheight = GetInterfaceFontAscent(item.style) + kInterfaceTextVBuffer + tRect.top;
        DrawInterfaceString(Point(swidth, sheight), item.label, item.style, color);
    } else {
        // draw the key code
        {
            Rects rects;
            if (item.status == kDimmed)
                shade = VERY_DARK;
            else
                shade = LIGHT;
            swidth    = GetInterfaceFontWidth(item.style) * kMaxKeyNameLength;

            uRect = Rect(
                    tRect.left + kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
                    tRect.left + kInterfaceContentBuffer + swidth + kInterfaceTextHBuffer * 2 + 1,
                    tRect.bottom - kInterfaceContentBuffer + 1);
            mDrawPuffUpRect(rects, uRect, item.hue, shade);

            if (item.status == kIH_Hilite)
                shade = LIGHT;
            else
                shade = DARK;  // DARKEST;
            vRect     = Rect(
                    tRect.left + kInterfaceContentBuffer + swidth + kInterfaceTextHBuffer * 2 + 2,
                    tRect.top + kInterfaceContentBuffer, tRect.right - kInterfaceContentBuffer + 1,
                    tRect.bottom - kInterfaceContentBuffer + 1);
            color = GetRGBTranslateColorShade(item.hue, shade);
            rects.fill(vRect, color);

            swidth = GetInterfaceStringWidth(shortcut_text, item.style);
            swidth = uRect.left + (uRect.right - uRect.left) / 2 - swidth / 2;
            if (item.status == kDimmed) {
                color = GetRGBTranslateColorShade(item.hue, VERY_DARK);
            } else {
                color = GetRGBTranslateColorShade(item.hue, DARKEST);
            }
        }

        DrawInterfaceString(
                Point(swidth, uRect.top + GetInterfaceFontAscent(item.style)), shortcut_text,
                item.style, color);

        // draw the button title
        {
            if (item.status == kIH_Hilite) {
                color = GetRGBTranslateColorShade(item.hue, DARKEST);
            } else if (item.status == kDimmed) {
                color = GetRGBTranslateColorShade(item.hue, DARKEST + kSlightlyLighterColor);
            } else {
                color = GetRGBTranslateColorShade(item.hue, LIGHTER);
            }

            StringSlice s = item.label;
            swidth        = GetInterfaceStringWidth(s, item.style);
            swidth        = uRect.right + (tRect.right - uRect.right) / 2 - swidth / 2;
            sheight       = GetInterfaceFontAscent(item.style) + kInterfaceTextVBuffer + tRect.top;
            DrawInterfaceString(Point(swidth, sheight), s, item.style, color);
        }
    }
}

void draw_tab_box_button(Point origin, const TabBoxButton& item) {
    Rect     tRect;
    int16_t  swidth, sheight, h_border = kInterfaceSmallHBorder;
    uint8_t  shade;
    RgbColor color;

    if (item.style == kLarge) {
        h_border = kInterfaceLargeHBorder;
    }
    tRect = item.bounds();
    tRect.offset(origin.h, origin.v);

    tRect.left -= kInterfaceContentBuffer;
    tRect.top -= kInterfaceContentBuffer;
    tRect.right += kInterfaceContentBuffer;
    tRect.bottom += kInterfaceContentBuffer;

    // top border

    if (item.status == kDimmed) {
        shade = VERY_DARK;
    } else {
        shade = MEDIUM;
    }

    mDrawPuffUpTopBorder(Rects(), tRect, item.hue, shade, h_border);

    // side border top

    Rect left(
            tRect.left - h_border, tRect.top + kInterfaceHTop, tRect.left + 1,
            tRect.bottom - kInterfaceHTop + 1);
    Rect right(
            tRect.right, tRect.top + kInterfaceHTop, tRect.right + h_border + 1,
            tRect.bottom - kInterfaceHTop + 1);
    if (!item.on) {
        Rects rects;
        if (item.status == kIH_Hilite) {
            shade = LIGHT;
            mDrawPuffUpRect(rects, left, item.hue, shade);
            mDrawPuffUpRect(rects, right, item.hue, shade);
        } else {
            if (item.status == kDimmed) {
                shade = VERY_DARK;
            } else
                shade = DARK;
            mDrawPuffUpRect(rects, left, item.hue, shade);
            mDrawPuffUpRect(rects, right, item.hue, shade);
        }
        left  = Rect(left.left, left.bottom, left.right, left.bottom + 3);
        right = Rect(right.left, right.bottom, right.right, right.bottom + 3);
        rects.fill(left, RgbColor::black());
        rects.fill(right, RgbColor::black());
        shade = MEDIUM;
        color = GetRGBTranslateColorShade(item.hue, shade);
        rects.fill(Rect(left.left - 3, left.bottom, right.right + 3, left.bottom + 3), color);

        const RgbColor lighter = GetRGBTranslateColorShade(item.hue, shade + kLighterColor);
        rects.fill(Rect(left.left - 3, left.bottom - 1, right.right + 3, left.bottom), lighter);
        const RgbColor darker = GetRGBTranslateColorShade(item.hue, shade + kDarkerColor);
        rects.fill(Rect(left.left - 3, left.bottom + 3, right.right + 3, left.bottom + 4), darker);
    } else {
        Rects rects;
        if (item.status == kIH_Hilite) {
            shade = LIGHT;
        } else if (item.status == kDimmed) {
            shade = VERY_DARK;
        } else {
            shade = MEDIUM;
        }
        left.bottom += 7;
        right.bottom += 7;
        color = GetRGBTranslateColorShade(item.hue, shade);
        rects.fill(left, color);
        rects.fill(right, color);

        const RgbColor lighter = GetRGBTranslateColorShade(item.hue, shade + kLighterColor);
        rects.fill(Rect(left.left, left.top, left.right - 1, left.top + 1), lighter);
        rects.fill(Rect(left.left, left.top, left.left + 1, left.bottom - 5), lighter);
        rects.fill(Rect(left.left - 3, left.bottom - 5, left.left + 1, left.bottom - 4), lighter);
        rects.fill(Rect(right.left, right.top, right.right - 1, right.top + 1), lighter);
        rects.fill(
                Rect(right.right, right.bottom - 5, right.right + 3, right.bottom - 4), lighter);
        rects.fill(Rect(right.left, right.top, right.left + 1, right.bottom - 1), lighter);

        const RgbColor darker = GetRGBTranslateColorShade(item.hue, shade + kDarkerColor);
        rects.fill(Rect(left.left - 3, left.bottom - 1, left.right, left.bottom), darker);
        rects.fill(Rect(left.right - 1, left.top, left.right, left.bottom), darker);
        rects.fill(Rect(right.right - 1, right.top, right.right, right.bottom - 4), darker);
        rects.fill(Rect(right.left, right.bottom - 1, right.right + 3, right.bottom), darker);

        Rect           uRect(left.left - 3, left.bottom - 4, left.right - 1, left.bottom - 1);
        const RgbColor color = GetRGBTranslateColorShade(item.hue, shade);
        rects.fill(uRect, color);
        Rect vRect(right.left + 1, right.bottom - 4, right.right + 3, right.bottom - 1);
        rects.fill(vRect, color);
        uRect.top--;
        uRect.bottom++;
        uRect.left  = uRect.right + 1;
        uRect.right = vRect.left - 1;
        rects.fill(uRect, RgbColor::black());
    }

    if (item.key == 0) {
        Rect uRect(
                tRect.left + kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
                tRect.left + kInterfaceContentBuffer, tRect.bottom - kInterfaceContentBuffer);

        if (item.on) {
            shade = MEDIUM;
        } else if (item.status == kIH_Hilite) {
            shade = LIGHT;
        } else {
            shade = DARKER;  // DARKEST + kSlightlyLighterColor;
        }
        uRect =
                Rect(tRect.left + kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
                     tRect.right - kInterfaceContentBuffer + 1,
                     tRect.bottom - kInterfaceContentBuffer + 1);
        color = GetRGBTranslateColorShade(item.hue, shade);
        Rects().fill(uRect, color);

        if (!item.on) {
            if (item.status == kIH_Hilite) {
                color = GetRGBTranslateColorShade(item.hue, DARKEST);
            } else if (item.status == kDimmed) {
                color = GetRGBTranslateColorShade(item.hue, VERY_DARK);
            } else {
                color = GetRGBTranslateColorShade(item.hue, LIGHT);
            }
        } else {
            color = GetRGBTranslateColorShade(item.hue, VERY_LIGHT);
        }

        StringSlice s = item.label;
        swidth        = GetInterfaceStringWidth(s, item.style);
        swidth        = tRect.left + (tRect.right - tRect.left) / 2 - swidth / 2;
        sheight       = GetInterfaceFontAscent(item.style) + kInterfaceTextVBuffer + tRect.top;
        DrawInterfaceString(Point(swidth, sheight), s, item.style, color);
    } else {
        // draw the key code
        if (item.on) {
            shade = MEDIUM + kLighterColor;
        } else if (item.status == kIH_Hilite) {
            shade = VERY_LIGHT;
        } else {
            shade = DARK;  // DARKEST + kSlightlyLighterColor;
        }
        String s;
        GetKeyNumName(item.key, &s);
        swidth = GetInterfaceFontWidth(item.style) * kMaxKeyNameLength;

        Rect uRect(
                tRect.left + kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
                tRect.left + kInterfaceContentBuffer + swidth + kInterfaceTextHBuffer * 2 + 1,
                tRect.bottom - kInterfaceContentBuffer + 1);
        mDrawPuffUpRect(Rects(), uRect, item.hue, shade);

        if (item.on) {
            shade = MEDIUM;
        } else if (item.status == kIH_Hilite) {
            shade = VERY_LIGHT;
        } else {
            shade = DARKER;  // DARKEST + kSlightlyLighterColor;
        }
        Rect vRect(
                tRect.left + kInterfaceContentBuffer + swidth + kInterfaceTextHBuffer * 2 + 2,
                tRect.top + kInterfaceContentBuffer, tRect.right - kInterfaceContentBuffer + 1,
                tRect.bottom - kInterfaceContentBuffer + 1);
        color = GetRGBTranslateColorShade(item.hue, shade);
        Rects().fill(vRect, color);

        swidth = GetInterfaceStringWidth(s, item.style);
        swidth = uRect.left + (uRect.right - uRect.left) / 2 - swidth / 2;
        if (item.status == kDimmed) {
            color = GetRGBTranslateColorShade(item.hue, VERY_DARK);
        } else {
            color = GetRGBTranslateColorShade(item.hue, DARKEST);
        }

        DrawInterfaceString(
                Point(swidth, uRect.top + GetInterfaceFontAscent(item.style)), s, item.style,
                color);

        // draw the button title
        if (!item.on) {
            if (item.status == kIH_Hilite) {
                color = GetRGBTranslateColorShade(item.hue, DARKEST);
            } else if (item.status == kDimmed) {
                color = GetRGBTranslateColorShade(item.hue, VERY_DARK);
            } else {
                color = GetRGBTranslateColorShade(item.hue, LIGHT);
            }
        } else {
            color = GetRGBTranslateColorShade(item.hue, VERY_LIGHT);
        }

        {
            StringSlice s = item.label;
            swidth        = GetInterfaceStringWidth(s, item.style);
            swidth        = uRect.right + (tRect.right - uRect.right) / 2 - swidth / 2;
            sheight       = GetInterfaceFontAscent(item.style) + kInterfaceTextVBuffer + tRect.top;
            DrawInterfaceString(Point(swidth, sheight), s, item.style, color);
        }
    }
}

/*
void DrawPlayerInterfaceRadioButton(Rect bounds, const RadioButton& item, PixMap* pix) {
    Rect            tRect, uRect, vRect, wRect;
    int16_t         vcenter, swidth, sheight, thisHBorder = kInterfaceSmallHBorder;
    uint8_t         shade;
    RgbColor        color;

    if ( item.style == kLarge) thisHBorder = kInterfaceLargeHBorder;
    tRect = bounds;

    tRect.left -= kInterfaceContentBuffer;
    tRect.top -= kInterfaceContentBuffer;
    tRect.right += kInterfaceContentBuffer;
    tRect.bottom += kInterfaceContentBuffer;

    // top border

    if ( item.status == kDimmed)
        shade = VERY_DARK;
    else shade = MEDIUM;

    mDrawPuffUpTopBorder( tRect, uRect, item.hue, shade, thisHBorder, pix);
    // bottom border

    mDrawPuffUpBottomBorder( tRect, uRect, item.hue, shade, thisHBorder, pix);

    // side border top

    vcenter = ( tRect.bottom - tRect.top) / 2;
    swidth = (tRect.top - kInterfaceVEdgeHeight - kInterfaceVCornerHeight + kIndicatorVOffset);
    sheight = (tRect.bottom + kInterfaceVEdgeHeight + kInterfaceVCornerHeight - kIndicatorVOffset) -
            swidth;

    wRect = Rect(tRect.left - thisHBorder - kCheckIndicatorHOffset - sheight, swidth,
            tRect.left - thisHBorder - kCheckIndicatorHOffset + 1, swidth + sheight + 1);

    uRect = Rect(tRect.left - thisHBorder - kCheckIndicatorHOffset - 2,
            tRect.top + kInterfaceHTop,
            tRect.left + 1,
            */ /*tRect.top + vcenter - kInterfaceVLipHeight + 1*/ /*
            tRect.bottom - kInterfaceHTop + 1);
    vRect = Rect(tRect.right, tRect.top + kInterfaceHTop, tRect.right + thisHBorder + 1,
            */ /*tRect.top + vcenter - kInterfaceVLipHeight + 1*/ /*
                                                                                                                                                                                      tRect.bottom - kInterfaceHTop + 1);

                                                                                                                                                                              if (
                                                                                                                                                                          item.status
                                                                                                                                                                          ==
                                                                                                                                                                          kIH_Hilite)
                                                                                                                                                                              {
                                                                                                                                                                                  shade
                                                                                                                                                                          = LIGHT;
                                                                                                                                                                                  mDrawPuffUpRect(
                                                                                                                                                                          uRect,
                                                                                                                                                                          item.hue,
                                                                                                                                                                          shade, pix);
                                                                                                                                                                                  mDrawPuffUpRect(
                                                                                                                                                                          vRect,
                                                                                                                                                                          item.hue,
                                                                                                                                                                          shade, pix);

                                                                                                                                                                                  wRect.left
                                                                                                                                                                          += 2;
                                                                                                                                                                                  wRect.right
                                                                                                                                                                          += 2;
                                                                                                                                                                                  FrameOval(pix,
                                                                                                                                                                          wRect,
                                                                                                                                                                          RgbColor::black());
                                                                                                                                                                                  wRect.left
                                                                                                                                                                          -= 2;
                                                                                                                                                                                  wRect.right
                                                                                                                                                                          -= 2;
                                                                                                                                                                                  mDrawPuffUpOval(wRect,
                                                                                                                                                                          item.hue,
                                                                                                                                                                          shade, pix);

                                                                                                                                                                                  wRect.inset(3,
                                                                                                                                                                          3);
                                                                                                                                                                                  mDrawPuffDownOval(wRect,
                                                                                                                                                                          item.hue,
                                                                                                                                                                          shade, pix);
                                                                                                                                                                                  wRect.inset(1,
                                                                                                                                                                          1);

                                                                                                                                                                                  if
                                                                                                                                                                          (!item.on) {
                                                                                                                                                                                      PaintOval(pix, wRect, RgbColor::black());
                                                                                                                                                                                  }
                                                                                                                                                                          else {
                                                                                                                                                                                      const RgbColor color =
                                                                                                                                                                          GetRGBTranslateColorShade(item.hue,
                                                                                                                                                                          VERY_LIGHT);
                                                                                                                                                                                      PaintOval(pix, wRect, color);
                                                                                                                                                                                  }
                                                                                                                                                                              } else
                                                                                                                                                                              {
                                                                                                                                                                                  if (
                                                                                                                                                                          item.status
                                                                                                                                                                          == kDimmed)
                                                                                                                                                                                      shade = VERY_DARK;
                                                                                                                                                                                  else
                                                                                                                                                                          shade =
                                                                                                                                                                          MEDIUM +
                                                                                                                                                                          kSlightlyLighterColor;
                                                                                                                                                                                  mDrawPuffUpRect(
                                                                                                                                                                          uRect,
                                                                                                                                                                          item.hue,
                                                                                                                                                                          shade, pix);
                                                                                                                                                                                  mDrawPuffUpRect(
                                                                                                                                                                          vRect,
                                                                                                                                                                          item.hue,
                                                                                                                                                                          shade, pix);
                                                                                                                                                                                  wRect.left
                                                                                                                                                                          += 2;
                                                                                                                                                                                  wRect.right
                                                                                                                                                                          += 2;
                                                                                                                                                                                  FrameOval(pix,
                                                                                                                                                                          wRect,
                                                                                                                                                                          RgbColor::black());
                                                                                                                                                                                  wRect.left
                                                                                                                                                                          -= 2;
                                                                                                                                                                                  wRect.right
                                                                                                                                                                          -= 2;
                                                                                                                                                                                  mDrawPuffUpOval(wRect,
                                                                                                                                                                          item.hue,
                                                                                                                                                                          shade, pix);

                                                                                                                                                                                  wRect.inset(3,
                                                                                                                                                                          3);
                                                                                                                                                                                  mDrawPuffDownOval(wRect,
                                                                                                                                                                          item.hue,
                                                                                                                                                                          shade, pix);
                                                                                                                                                                                  wRect.inset(1,
                                                                                                                                                                          1);
                                                                                                                                                                                  if
                                                                                                                                                                          (!item.on) {
                                                                                                                                                                                      PaintOval(pix, wRect, RgbColor::black());
                                                                                                                                                                                  }
                                                                                                                                                                          else if
                                                                                                                                                                          (item.status
                                                                                                                                                                          == kActive) {
                                                                                                                                                                                      const RgbColor color =
                                                                                                                                                                          GetRGBTranslateColorShade(item.hue,
                                                                                                                                                                          LIGHT);
                                                                                                                                                                                      PaintOval(pix, wRect, color);
                                                                                                                                                                                  }
                                                                                                                                                                          else {
                                                                                                                                                                                      const RgbColor color =
                                                                                                                                                                          GetRGBTranslateColorShade(item.hue,
                                                                                                                                                                          MEDIUM);
                                                                                                                                                                                      PaintOval(pix, wRect, color);
                                                                                                                                                                                  }
                                                                                                                                                                              }

                                                                                                                                                                              uRect =
                                                                                                                                                                          Rect(tRect.left
                                                                                                                                                                          +
                                                                                                                                                                          kInterfaceContentBuffer,
                                                                                                                                                                                  tRect.top
                                                                                                                                                                          +
                                                                                                                                                                          kInterfaceContentBuffer,
                                                                                                                                                                                  tRect.left
                                                                                                                                                                          +
                                                                                                                                                                          kInterfaceContentBuffer,
                                                                                                                                                                                  tRect.bottom
                                                                                                                                                                          -
                                                                                                                                                                          kInterfaceContentBuffer);

                                                                                                                                                                              if (
                                                                                                                                                                          item.status
                                                                                                                                                                          ==
                                                                                                                                                                          kIH_Hilite)
                                                                                                                                                                                  shade
                                                                                                                                                                          = LIGHT;
                                                                                                                                                                              else
                                                                                                                                                                          shade =
                                                                                                                                                                          DARKEST +
                                                                                                                                                                          kSlightlyLighterColor;
                                                                                                                                                                              uRect =
                                                                                                                                                                          Rect(tRect.left
                                                                                                                                                                          +
                                                                                                                                                                          kInterfaceContentBuffer,
                                                                                                                                                                          tRect.top +
                                                                                                                                                                          kInterfaceContentBuffer,
                                                                                                                                                                                              tRect.right -
                                                                                                                                                                          kInterfaceContentBuffer
                                                                                                                                                                          + 1,
                                                                                                                                                                                              tRect.bottom -
                                                                                                                                                                          kInterfaceContentBuffer
                                                                                                                                                                          + 1);
                                                                                                                                                                              color =
                                                                                                                                                                          GetRGBTranslateColorShade(item.hue,
                                                                                                                                                                          shade);
                                                                                                                                                                              pix->view(uRect).fill(color);

                                                                                                                                                                              if
                                                                                                                                                                          (item.status
                                                                                                                                                                          ==
                                                                                                                                                                          kIH_Hilite) {
                                                                                                                                                                                  color
                                                                                                                                                                          =
                                                                                                                                                                          GetRGBTranslateColorShade(item.hue,
                                                                                                                                                                          DARKEST);
                                                                                                                                                                              } else if
                                                                                                                                                                          (item.status
                                                                                                                                                                          == kDimmed) {
                                                                                                                                                                                  color
                                                                                                                                                                          =
                                                                                                                                                                          GetRGBTranslateColorShade(item.hue,
                                                                                                                                                                          DARK);
                                                                                                                                                                              } else {
                                                                                                                                                                                  color
                                                                                                                                                                          =
                                                                                                                                                                          GetRGBTranslateColorShade(item.hue,
                                                                                                                                                                          LIGHT);
                                                                                                                                                                              }
                                                                                                                                                                              StringSlice
                                                                                                                                                                          s =
                                                                                                                                                                          item.label;
                                                                                                                                                                              swidth =
                                                                                                                                                                          GetInterfaceStringWidth(
                                                                                                                                                                          s,
                                                                                                                                                                          item.style);
                                                                                                                                                                              swidth =
                                                                                                                                                                          tRect.left +
                                                                                                                                                                          ( tRect.right
                                                                                                                                                                          - tRect.left)
                                                                                                                                                                          /
                                                                                                                                                                          2 - swidth /
                                                                                                                                                                          2;
                                                                                                                                                                              sheight =
                                                                                                                                                                          GetInterfaceFontAscent(item.style)
                                                                                                                                                                          +
                                                                                                                                                                          kInterfaceTextVBuffer
                                                                                                                                                                          + tRect.top;
                                                                                                                                                                              DrawInterfaceString(Point(swidth,
                                                                                                                                                                          sheight), s,
                                                                                                                                                                          item.style,
                                                                                                                                                                          pix, color);
                                                                                                                                                                          }
                                                                                                                                                                          */

void draw_checkbox(Point origin, const CheckboxButton& item) {
    Rect     tRect, uRect, vRect, wRect;
    int16_t  swidth, sheight, thisHBorder = kInterfaceSmallHBorder;
    uint8_t  shade;
    RgbColor color;

    if (item.style == kLarge)
        thisHBorder = kInterfaceLargeHBorder;
    tRect           = item.bounds();
    tRect.offset(origin.h, origin.v);

    tRect.left -= kInterfaceContentBuffer;
    tRect.top -= kInterfaceContentBuffer;
    tRect.right += kInterfaceContentBuffer;
    tRect.bottom += kInterfaceContentBuffer;

    // top border

    if (item.status == kDimmed)
        shade = VERY_DARK;
    else
        shade = MEDIUM;

    mDrawPuffUpTopBorder(Rects(), tRect, item.hue, shade, thisHBorder);
    // bottom border

    mDrawPuffUpBottomBorder(Rects(), tRect, item.hue, shade, thisHBorder);

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

    if (item.status == kIH_Hilite) {
        shade = LIGHT;
        mDrawPuffUpRect(Rects(), uRect, item.hue, shade);
        mDrawPuffUpRect(Rects(), vRect, item.hue, shade);
        mDrawPuffUpRect(Rects(), wRect, item.hue, shade);
        wRect.inset(3, 3);
        mDrawPuffDownRect(Rects(), wRect, item.hue, shade);
        wRect.inset(1, 1);
        if (!item.on) {
            color = RgbColor::black();
        } else {
            color = GetRGBTranslateColorShade(item.hue, VERY_LIGHT);
        }
        Rects().fill(wRect, color);
    } else {
        if (item.status == kDimmed)
            shade = VERY_DARK;
        else
            shade = MEDIUM + kSlightlyLighterColor;
        mDrawPuffUpRect(Rects(), uRect, item.hue, shade);
        mDrawPuffUpRect(Rects(), vRect, item.hue, shade);
        mDrawPuffUpRect(Rects(), wRect, item.hue, shade);
        wRect.inset(3, 3);
        mDrawPuffDownRect(Rects(), wRect, item.hue, shade);
        wRect.inset(1, 1);
        if (!item.on) {
            color = RgbColor::black();
        } else if (item.status == kActive) {
            color = GetRGBTranslateColorShade(item.hue, LIGHT);
        } else {
            color = GetRGBTranslateColorShade(item.hue, MEDIUM);
        }
        Rects().fill(wRect, color);
    }

    uRect =
            Rect(tRect.left + kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
                 tRect.left + kInterfaceContentBuffer, tRect.bottom - kInterfaceContentBuffer);

    if (item.status == kIH_Hilite)
        shade = LIGHT;
    else
        shade = DARKEST + kSlightlyLighterColor;
    uRect     = Rect(
            tRect.left + kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
            tRect.right - kInterfaceContentBuffer + 1, tRect.bottom - kInterfaceContentBuffer + 1);
    color = GetRGBTranslateColorShade(item.hue, shade);
    Rects().fill(uRect, color);

    if (item.status == kIH_Hilite) {
        color = GetRGBTranslateColorShade(item.hue, DARKEST);
    } else if (item.status == kDimmed) {
        color = GetRGBTranslateColorShade(item.hue, DARK);
    } else {
        color = GetRGBTranslateColorShade(item.hue, LIGHT);
    }

    StringSlice s = item.label;
    swidth        = GetInterfaceStringWidth(s, item.style);
    swidth        = tRect.left + (tRect.right - tRect.left) / 2 - swidth / 2;
    sheight       = GetInterfaceFontAscent(item.style) + kInterfaceTextVBuffer + tRect.top;
    DrawInterfaceString(Point(swidth, sheight), s, item.style, color);
}

void draw_labeled_box(Point origin, const LabeledRect& item) {
    Rect     tRect, uRect;
    int16_t  vcenter, swidth, sheight, thisHBorder = kInterfaceSmallHBorder;
    uint8_t  shade;
    RgbColor color;

    if (item.style == kLarge) {
        thisHBorder = kInterfaceLargeHBorder;
    }
    tRect = item.bounds();
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

    StringSlice s = item.label;
    swidth        = GetInterfaceStringWidth(s, item.style) + kInterfaceTextHBuffer * 2;
    swidth        = (tRect.right - tRect.left) - swidth;
    sheight       = GetInterfaceFontHeight(item.style) + kInterfaceTextVBuffer * 2;

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

void draw_text_rect(Point origin, const TextRect& item) {
    Rect bounds = item.bounds();
    bounds.offset(origin.h, origin.v);
    draw_text_in_rect(bounds, item.text, item.style, item.hue);
}

}  // namespace

void draw_text_in_rect(
        Rect tRect, const StringSlice& text, interfaceStyleType style, uint8_t textcolor) {
    RgbColor   color = GetRGBTranslateColorShade(textcolor, VERY_LIGHT);
    StyledText interface_text(interface_font(style));
    interface_text.set_fore_color(color);
    interface_text.set_interface_text(text);
    interface_text.wrap_to(tRect.width(), kInterfaceTextHBuffer, kInterfaceTextVBuffer);
    tRect.offset(0, -kInterfaceTextVBuffer);
    interface_text.draw(tRect);
}

int16_t GetInterfaceTextHeightFromWidth(
        const StringSlice& text, interfaceStyleType style, int16_t boundsWidth) {
    StyledText interface_text(interface_font(style));
    interface_text.set_interface_text(text);
    interface_text.wrap_to(boundsWidth, kInterfaceTextHBuffer, kInterfaceTextVBuffer);
    return interface_text.height();
}

void draw_picture_rect(Point origin, const PictureRect& item) {
    Rect bounds = item.bounds();
    bounds.offset(origin.h, origin.v);
    if (item.visible_bounds) {
        draw_plain_rect(origin, item);
    }
    item.texture.draw(bounds.left, bounds.top);
}

namespace {

struct DrawInterfaceItemVisitor : InterfaceItem::Visitor {
    Point     p;
    InputMode mode;
    DrawInterfaceItemVisitor(Point p, InputMode mode) : p(p), mode(mode) {}

    virtual void visit_plain_rect(const PlainRect& i) const { draw_plain_rect(p, i); }
    virtual void visit_labeled_rect(const LabeledRect& i) const { draw_labeled_box(p, i); }
    virtual void visit_text_rect(const TextRect& i) const { draw_text_rect(p, i); }
    virtual void visit_picture_rect(const PictureRect& i) const { draw_picture_rect(p, i); }
    virtual void visit_plain_button(const PlainButton& i) const { draw_button(p, mode, i); }
    virtual void visit_radio_button(const RadioButton& i) const {}
    virtual void visit_checkbox_button(const CheckboxButton& i) const { draw_checkbox(p, i); }
    virtual void visit_tab_box(const TabBox& i) const { draw_tab_box(p, i); }
    virtual void visit_tab_box_button(const TabBoxButton& i) const { draw_tab_box_button(p, i); }
};

struct GetBoundsInterfaceItemVisitor : InterfaceItem::Visitor {
    Rect* bounds;
    GetBoundsInterfaceItemVisitor(Rect* bounds) : bounds(bounds) {}

    void initialize_bounds(const InterfaceItem& item) const {
        *bounds = item.bounds();
        bounds->left -= kInterfaceContentBuffer;
        bounds->top -= kInterfaceContentBuffer;
        bounds->right += kInterfaceContentBuffer + 1;
        bounds->bottom += kInterfaceContentBuffer + 1;
    }

    template <typename T>
    int h_border(T& t) const {
        return t.style == kLarge ? kInterfaceLargeHBorder : kInterfaceSmallHBorder;
    }

    virtual void visit_plain_rect(const PlainRect& item) const {
        initialize_bounds(item);
        bounds->left -= h_border(item);
        bounds->right += h_border(item);
        bounds->top -= kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
        bounds->bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
    }

    virtual void visit_labeled_rect(const LabeledRect& item) const {
        initialize_bounds(item);
        bounds->left -= h_border(item);
        bounds->right += h_border(item);
        bounds->top -= GetInterfaceFontHeight(item.style) + kInterfaceTextVBuffer * 2 +
                       kLabelBottomHeight + kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
        bounds->bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
    }

    virtual void visit_text_rect(const TextRect& item) const {
        initialize_bounds(item);
        bounds->left -= h_border(item);
        bounds->right += h_border(item);
        bounds->top -= kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
        bounds->bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
    }

    virtual void visit_picture_rect(const PictureRect& item) const {
        initialize_bounds(item);
        bounds->left -= h_border(item);
        bounds->right += h_border(item);
        bounds->top -= kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
        bounds->bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
    }

    virtual void visit_plain_button(const PlainButton& item) const {
        initialize_bounds(item);
        bounds->left -= h_border(item);
        bounds->right += h_border(item);
        bounds->top -= kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
        bounds->bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
    }

    virtual void visit_radio_button(const RadioButton& item) const {
        initialize_bounds(item);
        bounds->left -= bounds->bottom - bounds->top + 2 * kInterfaceVEdgeHeight +
                        2 * kInterfaceVCornerHeight - 2 * kIndicatorVOffset + h_border(item) +
                        kRadioIndicatorHOffset;
        bounds->right += h_border(item);
        bounds->top -= kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
        bounds->bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
    }

    virtual void visit_checkbox_button(const CheckboxButton& item) const {
        initialize_bounds(item);
        bounds->left -= bounds->bottom - bounds->top + 2 * kInterfaceVEdgeHeight +
                        2 * kInterfaceVCornerHeight - 2 * kIndicatorVOffset + h_border(item) +
                        kCheckIndicatorHOffset;
        bounds->right += h_border(item);
        bounds->top -= kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
        bounds->bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
    }

    virtual void visit_tab_box(const TabBox& item) const {
        initialize_bounds(item);
        bounds->left -= h_border(item);
        bounds->right += h_border(item);
        bounds->top -= kInterfaceVEdgeHeight + kInterfaceVCornerHeight + 2;
        bounds->bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
    }

    virtual void visit_tab_box_button(const TabBoxButton& item) const {
        initialize_bounds(item);
        bounds->left -= h_border(item) + 5;
        bounds->right += h_border(item) + 5;
        bounds->top -= kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
        bounds->bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight + 2;
    }

    /*
        case kListRect:
            bounds->left -= thisHBorder;
            bounds->right += thisHBorder;
            bounds->top -= GetInterfaceFontAscent(item.style) + kInterfaceTextVBuffer * 2 +
                            kLabelBottomHeight + kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
            bounds->bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
            break;
            */
};

}  // namespace

void draw_interface_item(const InterfaceItem& item, InputMode mode) {
    item.accept(DrawInterfaceItemVisitor({0, 0}, mode));
}

void draw_interface_item(const InterfaceItem& item, InputMode mode, Point origin) {
    item.accept(DrawInterfaceItemVisitor(origin, mode));
}

void GetAnyInterfaceItemGraphicBounds(const InterfaceItem& item, Rect* bounds) {
    item.accept(GetBoundsInterfaceItemVisitor(bounds));
}

}  // namespace antares
