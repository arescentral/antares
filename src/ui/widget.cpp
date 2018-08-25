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
#include "data/resource.hpp"
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

static Rect initialize_bounds(Rect bounds) {
    bounds.left -= kInterfaceContentBuffer;
    bounds.top -= kInterfaceContentBuffer;
    bounds.right += kInterfaceContentBuffer + 1;
    bounds.bottom += kInterfaceContentBuffer + 1;
    return bounds;
}

static int h_border(InterfaceStyle style) {
    return style == InterfaceStyle::LARGE ? kInterfaceLargeHBorder : kInterfaceSmallHBorder;
}

Widget::~Widget() = default;

Widget* Widget::accept_click(Point where) { return nullptr; }
Widget* Widget::accept_key(Key which) { return nullptr; }
Widget* Widget::accept_button(Gamepad::Button which) { return nullptr; }
void    Widget::action() {}

void Widget::activate() {}
void Widget::deactivate() {}

std::vector<const Widget*> Widget::children() const { return std::vector<const Widget*>{}; }
std::vector<Widget*>       Widget::children() { return std::vector<Widget*>{}; }

BoxRect::BoxRect(const BoxRectData& data)
        : _inner_bounds{data.bounds},
          _id{data.id},
          _label{data.label.has_value() ? sfz::make_optional<pn::string>(data.label->copy())
                                        : sfz::nullopt},
          _hue{data.hue},
          _style{data.style} {}

void BoxRect::draw(Point offset, InputMode) const {
    if (_label.has_value()) {
        draw_labeled_box(offset);
    } else {
        draw_plain_rect(offset);
    }
}

void BoxRect::draw_labeled_box(Point origin) const {
    Rect     tRect, uRect;
    int16_t  vcenter, swidth, sheight, thisHBorder = kInterfaceSmallHBorder;
    uint8_t  shade;
    RgbColor color;

    if (_style == InterfaceStyle::LARGE) {
        thisHBorder = kInterfaceLargeHBorder;
    }
    tRect = _inner_bounds;
    tRect.offset(origin.h, origin.v);
    tRect.left -= kInterfaceContentBuffer;
    tRect.top -= kInterfaceContentBuffer + GetInterfaceFontHeight(_style) +
                 kInterfaceTextVBuffer * 2 + kLabelBottomHeight;
    tRect.right += kInterfaceContentBuffer;
    tRect.bottom += kInterfaceContentBuffer;

    shade = DARK;

    mDrawPuffUpTopBorder(Rects(), tRect, _hue, shade, thisHBorder);
    // bottom border

    mDrawPuffUpBottomBorder(Rects(), tRect, _hue, shade, thisHBorder);

    // draw the string

    pn::string_view s = *_label;
    swidth            = GetInterfaceStringWidth(s, _style) + kInterfaceTextHBuffer * 2;
    swidth            = (tRect.right - tRect.left) - swidth;
    sheight           = GetInterfaceFontHeight(_style) + kInterfaceTextVBuffer * 2;

    uRect =
            Rect(tRect.left + kInterfaceTextHBuffer - 1, tRect.top + kInterfaceHTop,
                 tRect.right - swidth - kInterfaceTextHBuffer + 1,
                 tRect.top + sheight - kInterfaceHTop);
    color = GetRGBTranslateColorShade(_hue, VERY_DARK);
    Rects().fill(uRect, color);

    color = GetRGBTranslateColorShade(_hue, LIGHT);

    DrawInterfaceString(
            Point(tRect.left + kInterfaceTextHBuffer,
                  tRect.top + GetInterfaceFontAscent(_style) + kInterfaceTextVBuffer),
            s, _style, color);

    // string left border

    shade   = MEDIUM;
    vcenter = sheight / 2;

    uRect =
            Rect(tRect.left - thisHBorder, tRect.top + kInterfaceHTop, tRect.left + 1,
                 tRect.top + sheight - kInterfaceHTop + 1);
    mDrawPuffUpRect(Rects(), uRect, _hue, shade);

    // string right border

    shade = MEDIUM;
    uRect =
            Rect(tRect.right - swidth, tRect.top + kInterfaceHTop, tRect.right - 2,
                 tRect.top + sheight - kInterfaceHTop + 1);
    mDrawPuffUpRect(Rects(), uRect, _hue, shade);
    uRect =
            Rect(tRect.right, tRect.top + kInterfaceHTop, tRect.right + thisHBorder + 1,
                 tRect.top + sheight - kInterfaceHTop + 1);
    mDrawPuffUpRect(Rects(), uRect, _hue, shade);

    // string bottom border

    mDrawPuffUpTBorder(Rects(), tRect, _hue, DARK, sheight, thisHBorder);

    // main part left border

    tRect.top += sheight + kLabelBottomHeight;

    vcenter = (tRect.bottom - tRect.top) / 2;

    uRect =
            Rect(tRect.left - thisHBorder, tRect.top + kInterfaceHTop, tRect.left + 1,
                 tRect.top + vcenter - kInterfaceVLipHeight + 1);
    mDrawPuffUpRect(Rects(), uRect, _hue, DARKER);

    uRect =
            Rect(tRect.left - thisHBorder, tRect.bottom - vcenter + kInterfaceVLipHeight,
                 tRect.left + 1, tRect.bottom - kInterfaceHTop + 1);
    mDrawPuffUpRect(Rects(), uRect, _hue, VERY_DARK);

    // right border

    uRect =
            Rect(tRect.right, tRect.top + kInterfaceHTop, tRect.right + thisHBorder + 1,
                 tRect.top + vcenter - kInterfaceVLipHeight + 1);
    mDrawPuffUpRect(Rects(), uRect, _hue, DARKER);

    uRect =
            Rect(tRect.right, tRect.bottom - vcenter + kInterfaceVLipHeight,
                 tRect.right + thisHBorder + 1, tRect.bottom - kInterfaceHTop + 1);
    mDrawPuffUpRect(Rects(), uRect, _hue, VERY_DARK);
}

void BoxRect::draw_plain_rect(Point origin) const {
    Rects          rects;
    Rect           tRect, uRect;
    int16_t        vcenter, thisHBorder = kInterfaceSmallHBorder;
    Hue            color = _hue;
    InterfaceStyle style = _style;

    if (style == InterfaceStyle::LARGE) {
        thisHBorder = kInterfaceLargeHBorder;
    }
    tRect = _inner_bounds;
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

Rect BoxRect::inner_bounds() const { return _inner_bounds; }

Rect BoxRect::outer_bounds() const {
    Rect bounds = initialize_bounds(_inner_bounds);
    bounds.left -= h_border(_style);
    bounds.right += h_border(_style);
    if (_label.has_value()) {
        bounds.top -= GetInterfaceFontHeight(_style) + kInterfaceTextVBuffer * 2 +
                      kLabelBottomHeight + kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
    } else {
        bounds.top -= kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
    }
    bounds.bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
    return bounds;
}

TextRect::TextRect(const TextRectData& data)
        : _inner_bounds{data.bounds},
          _id{data.id},
          _text{data.text.has_value() ? sfz::make_optional(data.text->copy()) : sfz::nullopt},
          _hue{data.hue},
          _style{data.style} {}

void TextRect::draw(Point offset, InputMode) const {
    Rect bounds = _inner_bounds;
    bounds.offset(offset.h, offset.v);
    draw_text_in_rect(bounds, _text.has_value() ? _text->copy() : pn::string_view{}, _style, _hue);
}

Rect TextRect::inner_bounds() const { return _inner_bounds; }

Rect TextRect::outer_bounds() const {
    Rect bounds = initialize_bounds(_inner_bounds);
    bounds.left -= h_border(_style);
    bounds.right += h_border(_style);
    bounds.top -= kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
    bounds.bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
    return bounds;
}

PictureRect::PictureRect(const PictureRectData& data)
        : _inner_bounds(data.bounds), _id{data.id}, _texture{Resource::texture(data.picture)} {}

void PictureRect::draw(Point offset, InputMode) const {
    Rect bounds = _inner_bounds;
    bounds.offset(offset.h, offset.v);
    _texture.draw(bounds.left, bounds.top);
}

Rect PictureRect::inner_bounds() const { return _inner_bounds; }

Rect PictureRect::outer_bounds() const {
    Rect bounds = initialize_bounds(_inner_bounds);
    bounds.left -= kInterfaceSmallHBorder;
    bounds.right += kInterfaceSmallHBorder;
    bounds.top -= kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
    bounds.bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
    return bounds;
}

Button::Button(const ButtonData& data)
        : _id{data.id},
          _label{data.label.copy()},
          _key{data.key},
          _gamepad{data.gamepad},
          _hue{data.hue},
          _style{data.style} {}

Widget* Button::accept_click(Point where) {
    if (enabled() && (outer_bounds().contains(where))) {
        return this;
    }
    return nullptr;
}

Widget* Button::accept_key(Key which) {
    if (enabled() && (_key == which)) {
        return this;
    }
    return nullptr;
}

Widget* Button::accept_button(Gamepad::Button which) {
    if (enabled() && (_gamepad == which)) {
        return this;
    }
    return nullptr;
}

PlainButton::PlainButton(const PlainButtonData& data) : Button{data}, _inner_bounds{data.bounds} {}

void PlainButton::bind(Action a) { _action = a; }

void PlainButton::action() {
    if (_action.perform) {
        _action.perform();
    }
}

bool PlainButton::enabled() const {
    if (!_action.perform) {
        return false;
    } else if (!_action.possible) {
        return true;
    } else {
        return _action.possible();
    }
}

void PlainButton::draw(Point offset, InputMode mode) const {
    Rect     tRect, uRect, vRect;
    int16_t  swidth, sheight, thisHBorder = kInterfaceSmallHBorder;
    uint8_t  shade;
    RgbColor color;

    {
        Rects rects;
        if (style() == InterfaceStyle::LARGE) {
            thisHBorder = kInterfaceLargeHBorder;
        }
        tRect = _inner_bounds;
        tRect.offset(offset.h, offset.v);

        uRect = tRect;
        uRect.right++;
        uRect.bottom++;

        tRect.left -= kInterfaceContentBuffer;
        tRect.top -= kInterfaceContentBuffer;
        tRect.right += kInterfaceContentBuffer;
        tRect.bottom += kInterfaceContentBuffer;

        // top border

        if (!enabled()) {
            shade = VERY_DARK;
        } else {
            shade = MEDIUM;
        }

        mDrawPuffUpTopBorder(rects, tRect, hue(), shade, thisHBorder);
        // bottom border

        mDrawPuffUpBottomBorder(rects, tRect, hue(), shade, thisHBorder);

        // side border top

        uRect =
                Rect(tRect.left - thisHBorder, tRect.top + kInterfaceHTop, tRect.left + 1,
                     tRect.bottom - kInterfaceHTop + 1);
        vRect =
                Rect(tRect.right, tRect.top + kInterfaceHTop, tRect.right + thisHBorder + 1,
                     tRect.bottom - kInterfaceHTop + 1);
        if (active()) {
            shade = LIGHT;
            mDrawPuffUpRect(rects, uRect, hue(), shade);
            mDrawPuffUpRect(rects, vRect, hue(), shade);
        } else {
            if (!enabled()) {
                shade = VERY_DARK;
            } else {
                shade = MEDIUM + kSlightlyLighterColor;
            }
            mDrawPuffUpRect(rects, uRect, hue(), shade);
            mDrawPuffUpRect(rects, vRect, hue(), shade);
        }
    }

    bool       draw_shortcut = false;
    pn::string shortcut_text;
    if ((mode == KEYBOARD_MOUSE) && (key() != Key::NONE)) {
        draw_shortcut = true;
        GetKeyNumName(key(), shortcut_text);
    } else if ((mode == GAMEPAD) && (gamepad() != Gamepad::Button::NONE)) {
        draw_shortcut = true;
        Gamepad::name(gamepad(), shortcut_text);
    }

    if (!draw_shortcut) {
        uRect =
                Rect(tRect.left + kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
                     tRect.left + kInterfaceContentBuffer, tRect.bottom - kInterfaceContentBuffer);

        if (active())
            shade = LIGHT;
        else
            shade = DARK;  // DARKEST + kSlightlyLighterColor;
        uRect =
                Rect(tRect.left + kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
                     tRect.right - kInterfaceContentBuffer + 1,
                     tRect.bottom - kInterfaceContentBuffer + 1);

        color = GetRGBTranslateColorShade(hue(), shade);
        Rects().fill(uRect, color);

        if (active()) {
            color = GetRGBTranslateColorShade(hue(), DARKEST);
        } else if (!enabled()) {
            color = GetRGBTranslateColorShade(hue(), VERY_DARK);
        } else {
            color = GetRGBTranslateColorShade(hue(), LIGHTER);
        }
        swidth  = GetInterfaceStringWidth(label(), style());
        swidth  = tRect.left + (tRect.right - tRect.left) / 2 - swidth / 2;
        sheight = GetInterfaceFontAscent(style()) + kInterfaceTextVBuffer + tRect.top;
        DrawInterfaceString(Point(swidth, sheight), label(), style(), color);
    } else {
        // draw the key code
        {
            Rects rects;
            if (!enabled())
                shade = VERY_DARK;
            else
                shade = LIGHT;
            swidth = GetInterfaceFontWidth(style()) * kMaxKeyNameLength;

            uRect = Rect(
                    tRect.left + kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
                    tRect.left + kInterfaceContentBuffer + swidth + kInterfaceTextHBuffer * 2 + 1,
                    tRect.bottom - kInterfaceContentBuffer + 1);
            mDrawPuffUpRect(rects, uRect, hue(), shade);

            if (active())
                shade = LIGHT;
            else
                shade = DARK;  // DARKEST;
            vRect = Rect(
                    tRect.left + kInterfaceContentBuffer + swidth + kInterfaceTextHBuffer * 2 + 2,
                    tRect.top + kInterfaceContentBuffer, tRect.right - kInterfaceContentBuffer + 1,
                    tRect.bottom - kInterfaceContentBuffer + 1);
            color = GetRGBTranslateColorShade(hue(), shade);
            rects.fill(vRect, color);

            swidth = GetInterfaceStringWidth(shortcut_text, style());
            swidth = uRect.left + (uRect.right - uRect.left) / 2 - swidth / 2;
            if (!enabled()) {
                color = GetRGBTranslateColorShade(hue(), VERY_DARK);
            } else {
                color = GetRGBTranslateColorShade(hue(), DARKEST);
            }
        }

        DrawInterfaceString(
                Point(swidth, uRect.top + GetInterfaceFontAscent(style())), shortcut_text, style(),
                color);

        // draw the button title
        {
            if (active()) {
                color = GetRGBTranslateColorShade(hue(), DARKEST);
            } else if (!enabled()) {
                color = GetRGBTranslateColorShade(hue(), DARKEST + kSlightlyLighterColor);
            } else {
                color = GetRGBTranslateColorShade(hue(), LIGHTER);
            }

            pn::string_view s = label();
            swidth            = GetInterfaceStringWidth(s, style());
            swidth            = uRect.right + (tRect.right - uRect.right) / 2 - swidth / 2;
            sheight = GetInterfaceFontAscent(style()) + kInterfaceTextVBuffer + tRect.top;
            DrawInterfaceString(Point(swidth, sheight), s, style(), color);
        }
    }
}

Rect PlainButton::inner_bounds() const { return _inner_bounds; }

Rect PlainButton::outer_bounds() const {
    Rect bounds = initialize_bounds(_inner_bounds);
    bounds.left -= h_border(style());
    bounds.right += h_border(style());
    bounds.top -= kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
    bounds.bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
    return bounds;
}

CheckboxButton::CheckboxButton(const CheckboxButtonData& data)
        : Button{data}, _inner_bounds{data.bounds} {}

void CheckboxButton::bind(Value v) { _value = v; }
bool CheckboxButton::get() const { return _value.get && _value.get(); }
void CheckboxButton::set(bool on) { _value.set(on); }
bool CheckboxButton::enabled() const {
    if (!_value.get) {
        return false;
    } else if (!_value.modifiable) {
        return true;
    } else {
        return _value.modifiable();
    }
}

void CheckboxButton::action() { set(!get()); }

void CheckboxButton::draw(Point offset, InputMode) const {
    Rect     tRect, uRect, vRect, wRect;
    int16_t  swidth, sheight, thisHBorder = kInterfaceSmallHBorder;
    uint8_t  shade;
    RgbColor color;

    if (style() == InterfaceStyle::LARGE)
        thisHBorder = kInterfaceLargeHBorder;
    tRect = _inner_bounds;
    tRect.offset(offset.h, offset.v);

    tRect.left -= kInterfaceContentBuffer;
    tRect.top -= kInterfaceContentBuffer;
    tRect.right += kInterfaceContentBuffer;
    tRect.bottom += kInterfaceContentBuffer;

    // top border

    if (!enabled())
        shade = VERY_DARK;
    else
        shade = MEDIUM;

    mDrawPuffUpTopBorder(Rects(), tRect, hue(), shade, thisHBorder);
    // bottom border

    mDrawPuffUpBottomBorder(Rects(), tRect, hue(), shade, thisHBorder);

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

    if (active()) {
        shade = LIGHT;
        mDrawPuffUpRect(Rects(), uRect, hue(), shade);
        mDrawPuffUpRect(Rects(), vRect, hue(), shade);
        mDrawPuffUpRect(Rects(), wRect, hue(), shade);
        wRect.inset(3, 3);
        mDrawPuffDownRect(Rects(), wRect, hue(), shade);
        wRect.inset(1, 1);
        if (!get()) {
            color = RgbColor::black();
        } else {
            color = GetRGBTranslateColorShade(hue(), LIGHTEST);
        }
        Rects().fill(wRect, color);
    } else {
        if (!enabled())
            shade = VERY_DARK;
        else
            shade = MEDIUM + kSlightlyLighterColor;
        mDrawPuffUpRect(Rects(), uRect, hue(), shade);
        mDrawPuffUpRect(Rects(), vRect, hue(), shade);
        mDrawPuffUpRect(Rects(), wRect, hue(), shade);
        wRect.inset(3, 3);
        mDrawPuffDownRect(Rects(), wRect, hue(), shade);
        wRect.inset(1, 1);
        if (!get()) {
            color = RgbColor::black();
        } else if (enabled()) {
            color = GetRGBTranslateColorShade(hue(), LIGHT);
        } else {
            color = GetRGBTranslateColorShade(hue(), MEDIUM);
        }
        Rects().fill(wRect, color);
    }

    uRect =
            Rect(tRect.left + kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
                 tRect.left + kInterfaceContentBuffer, tRect.bottom - kInterfaceContentBuffer);

    if (active())
        shade = LIGHT;
    else
        shade = DARKEST + kSlightlyLighterColor;
    uRect = Rect(
            tRect.left + kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
            tRect.right - kInterfaceContentBuffer + 1, tRect.bottom - kInterfaceContentBuffer + 1);
    color = GetRGBTranslateColorShade(hue(), shade);
    Rects().fill(uRect, color);

    if (active()) {
        color = GetRGBTranslateColorShade(hue(), DARKEST);
    } else if (!enabled()) {
        color = GetRGBTranslateColorShade(hue(), DARK);
    } else {
        color = GetRGBTranslateColorShade(hue(), LIGHT);
    }

    pn::string_view s = label();
    swidth            = GetInterfaceStringWidth(s, style());
    swidth            = tRect.left + (tRect.right - tRect.left) / 2 - swidth / 2;
    sheight           = GetInterfaceFontAscent(style()) + kInterfaceTextVBuffer + tRect.top;
    DrawInterfaceString(Point(swidth, sheight), s, style(), color);
}

Rect CheckboxButton::inner_bounds() const { return _inner_bounds; }

Rect CheckboxButton::outer_bounds() const {
    Rect bounds = initialize_bounds(_inner_bounds);
    bounds.left -= bounds.bottom - bounds.top + 2 * kInterfaceVEdgeHeight +
                   2 * kInterfaceVCornerHeight - 2 * kIndicatorVOffset + h_border(style()) +
                   kCheckIndicatorHOffset;
    bounds.right += h_border(style());
    bounds.top -= kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
    bounds.bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
    return bounds;
}

RadioButton::RadioButton(const RadioButtonData& data) : Button{data}, _inner_bounds{data.bounds} {}

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

    if (!enabled())
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

    if (active()) {
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

        if (!on()) {
            PaintOval(pix, wRect, RgbColor::black());
        } else {
            const RgbColor color = GetRGBTranslateColorShade(item.hue, LIGHTEST);
            PaintOval(pix, wRect, color);
        }
    } else {
        if (!enabled())
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
        if (!on()) {
            PaintOval(pix, wRect, RgbColor::black());
        } else if (state() == kActive) {
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

    if (active())
        shade = LIGHT;
    else
        shade = DARKEST + kSlightlyLighterColor;
    uRect = Rect(
            tRect.left + kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
            tRect.right - kInterfaceContentBuffer + 1, tRect.bottom - kInterfaceContentBuffer + 1);
    color = GetRGBTranslateColorShade(item.hue, shade);
    pix->view(uRect).fill(color);

    if (active()) {
        color = GetRGBTranslateColorShade(item.hue, DARKEST);
    } else if (!enabled()) {
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

Rect RadioButton::inner_bounds() const { return _inner_bounds; }

Rect RadioButton::outer_bounds() const {
    Rect bounds = initialize_bounds(_inner_bounds);
    bounds.left -= bounds.bottom - bounds.top + 2 * kInterfaceVEdgeHeight +
                   2 * kInterfaceVCornerHeight - 2 * kIndicatorVOffset + h_border(style()) +
                   kRadioIndicatorHOffset;
    bounds.right += h_border(style());
    bounds.top -= kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
    bounds.bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
    return bounds;
}

static PlainButtonData tab_button_data(
        const TabBox& box, const TabBoxData::Tab& tab, Rect bounds) {
    PlainButtonData button;
    button.id     = tab.id;
    button.bounds = bounds;
    button.label  = tab.label.copy();
    button.hue    = box.hue();
    button.style  = box.style();
    return button;
}

TabButton::TabButton(TabBox* box, const TabBoxData::Tab& data, Rect bounds)
        : Button{tab_button_data(*box, data, bounds)}, _parent(box), _inner_bounds{bounds} {
    for (const auto& item : data.content) {
        _content.push_back(Widget::from(item));
    }
}

void TabButton::action() { parent()->select(*this); }

void TabButton::draw(Point offset, InputMode) const {
    Rect     tRect;
    int16_t  swidth, sheight, h_border = kInterfaceSmallHBorder;
    uint8_t  shade;
    RgbColor color;

    if (style() == InterfaceStyle::LARGE) {
        h_border = kInterfaceLargeHBorder;
    }
    tRect = _inner_bounds;
    tRect.offset(offset.h, offset.v);

    tRect.left -= kInterfaceContentBuffer;
    tRect.top -= kInterfaceContentBuffer;
    tRect.right += kInterfaceContentBuffer;
    tRect.bottom += kInterfaceContentBuffer;

    // top border

    if (!enabled()) {
        shade = VERY_DARK;
    } else {
        shade = MEDIUM;
    }

    mDrawPuffUpTopBorder(Rects(), tRect, hue(), shade, h_border);

    // side border top

    Rect left(
            tRect.left - h_border, tRect.top + kInterfaceHTop, tRect.left + 1,
            tRect.bottom - kInterfaceHTop + 1);
    Rect right(
            tRect.right, tRect.top + kInterfaceHTop, tRect.right + h_border + 1,
            tRect.bottom - kInterfaceHTop + 1);
    if (!on()) {
        Rects rects;
        if (active()) {
            shade = LIGHT;
            mDrawPuffUpRect(rects, left, hue(), shade);
            mDrawPuffUpRect(rects, right, hue(), shade);
        } else {
            if (!enabled()) {
                shade = VERY_DARK;
            } else
                shade = DARK;
            mDrawPuffUpRect(rects, left, hue(), shade);
            mDrawPuffUpRect(rects, right, hue(), shade);
        }
        left  = Rect(left.left, left.bottom, left.right, left.bottom + 3);
        right = Rect(right.left, right.bottom, right.right, right.bottom + 3);
        rects.fill(left, RgbColor::black());
        rects.fill(right, RgbColor::black());
        shade = MEDIUM;
        color = GetRGBTranslateColorShade(hue(), shade);
        rects.fill(Rect(left.left - 3, left.bottom, right.right + 3, left.bottom + 3), color);

        const RgbColor lighter = GetRGBTranslateColorShade(hue(), shade + kLighterColor);
        rects.fill(Rect(left.left - 3, left.bottom - 1, right.right + 3, left.bottom), lighter);
        const RgbColor darker = GetRGBTranslateColorShade(hue(), shade + kDarkerColor);
        rects.fill(Rect(left.left - 3, left.bottom + 3, right.right + 3, left.bottom + 4), darker);
    } else {
        Rects rects;
        if (active()) {
            shade = LIGHT;
        } else if (!enabled()) {
            shade = VERY_DARK;
        } else {
            shade = MEDIUM;
        }
        left.bottom += 7;
        right.bottom += 7;
        color = GetRGBTranslateColorShade(hue(), shade);
        rects.fill(left, color);
        rects.fill(right, color);

        const RgbColor lighter = GetRGBTranslateColorShade(hue(), shade + kLighterColor);
        rects.fill(Rect(left.left, left.top, left.right - 1, left.top + 1), lighter);
        rects.fill(Rect(left.left, left.top, left.left + 1, left.bottom - 5), lighter);
        rects.fill(Rect(left.left - 3, left.bottom - 5, left.left + 1, left.bottom - 4), lighter);
        rects.fill(Rect(right.left, right.top, right.right - 1, right.top + 1), lighter);
        rects.fill(
                Rect(right.right, right.bottom - 5, right.right + 3, right.bottom - 4), lighter);
        rects.fill(Rect(right.left, right.top, right.left + 1, right.bottom - 1), lighter);

        const RgbColor darker = GetRGBTranslateColorShade(hue(), shade + kDarkerColor);
        rects.fill(Rect(left.left - 3, left.bottom - 1, left.right, left.bottom), darker);
        rects.fill(Rect(left.right - 1, left.top, left.right, left.bottom), darker);
        rects.fill(Rect(right.right - 1, right.top, right.right, right.bottom - 4), darker);
        rects.fill(Rect(right.left, right.bottom - 1, right.right + 3, right.bottom), darker);

        Rect           uRect(left.left - 3, left.bottom - 4, left.right - 1, left.bottom - 1);
        const RgbColor color = GetRGBTranslateColorShade(hue(), shade);
        rects.fill(uRect, color);
        Rect vRect(right.left + 1, right.bottom - 4, right.right + 3, right.bottom - 1);
        rects.fill(vRect, color);
        uRect.top--;
        uRect.bottom++;
        uRect.left  = uRect.right + 1;
        uRect.right = vRect.left - 1;
        rects.fill(uRect, RgbColor::black());
    }

    if (key() == Key::NONE) {
        Rect uRect(
                tRect.left + kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
                tRect.left + kInterfaceContentBuffer, tRect.bottom - kInterfaceContentBuffer);

        if (on()) {
            shade = MEDIUM;
        } else if (active()) {
            shade = LIGHT;
        } else {
            shade = DARKER;  // DARKEST + kSlightlyLighterColor;
        }
        uRect =
                Rect(tRect.left + kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
                     tRect.right - kInterfaceContentBuffer + 1,
                     tRect.bottom - kInterfaceContentBuffer + 1);
        color = GetRGBTranslateColorShade(hue(), shade);
        Rects().fill(uRect, color);

        if (!on()) {
            if (active()) {
                color = GetRGBTranslateColorShade(hue(), DARKEST);
            } else if (!enabled()) {
                color = GetRGBTranslateColorShade(hue(), VERY_DARK);
            } else {
                color = GetRGBTranslateColorShade(hue(), LIGHT);
            }
        } else {
            color = GetRGBTranslateColorShade(hue(), LIGHTEST);
        }

        pn::string_view s = label();
        swidth            = GetInterfaceStringWidth(s, style());
        swidth            = tRect.left + (tRect.right - tRect.left) / 2 - swidth / 2;
        sheight           = GetInterfaceFontAscent(style()) + kInterfaceTextVBuffer + tRect.top;
        DrawInterfaceString(Point(swidth, sheight), s, style(), color);
    } else {
        // draw the key code
        if (on()) {
            shade = MEDIUM + kLighterColor;
        } else if (active()) {
            shade = LIGHTEST;
        } else {
            shade = DARK;  // DARKEST + kSlightlyLighterColor;
        }
        pn::string s;
        GetKeyNumName(key(), s);
        swidth = GetInterfaceFontWidth(style()) * kMaxKeyNameLength;

        Rect uRect(
                tRect.left + kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
                tRect.left + kInterfaceContentBuffer + swidth + kInterfaceTextHBuffer * 2 + 1,
                tRect.bottom - kInterfaceContentBuffer + 1);
        mDrawPuffUpRect(Rects(), uRect, hue(), shade);

        if (on()) {
            shade = MEDIUM;
        } else if (active()) {
            shade = LIGHTEST;
        } else {
            shade = DARKER;  // DARKEST + kSlightlyLighterColor;
        }
        Rect vRect(
                tRect.left + kInterfaceContentBuffer + swidth + kInterfaceTextHBuffer * 2 + 2,
                tRect.top + kInterfaceContentBuffer, tRect.right - kInterfaceContentBuffer + 1,
                tRect.bottom - kInterfaceContentBuffer + 1);
        color = GetRGBTranslateColorShade(hue(), shade);
        Rects().fill(vRect, color);

        swidth = GetInterfaceStringWidth(s, style());
        swidth = uRect.left + (uRect.right - uRect.left) / 2 - swidth / 2;
        if (!enabled()) {
            color = GetRGBTranslateColorShade(hue(), VERY_DARK);
        } else {
            color = GetRGBTranslateColorShade(hue(), DARKEST);
        }

        DrawInterfaceString(
                Point(swidth, uRect.top + GetInterfaceFontAscent(style())), s, style(), color);

        // draw the button title
        if (!on()) {
            if (active()) {
                color = GetRGBTranslateColorShade(hue(), DARKEST);
            } else if (!enabled()) {
                color = GetRGBTranslateColorShade(hue(), VERY_DARK);
            } else {
                color = GetRGBTranslateColorShade(hue(), LIGHT);
            }
        } else {
            color = GetRGBTranslateColorShade(hue(), LIGHTEST);
        }

        {
            pn::string_view s = label();
            swidth            = GetInterfaceStringWidth(s, style());
            swidth            = uRect.right + (tRect.right - uRect.right) / 2 - swidth / 2;
            sheight = GetInterfaceFontAscent(style()) + kInterfaceTextVBuffer + tRect.top;
            DrawInterfaceString(Point(swidth, sheight), s, style(), color);
        }
    }
}

Rect TabButton::inner_bounds() const { return _inner_bounds; }

Rect TabButton::outer_bounds() const {
    Rect bounds = initialize_bounds(_inner_bounds);
    bounds.left -= h_border(style()) + 5;
    bounds.right += h_border(style()) + 5;
    bounds.top -= kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
    bounds.bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight + 2;
    return bounds;
}

TabBox::TabBox(const TabBoxData& data)
        : _inner_bounds{data.bounds},
          _id{data.id},
          _current_tab{0},
          _hue{data.hue},
          _style{data.style} {
    Rect button_bounds = {_inner_bounds.left + 22, _inner_bounds.top - 20, 0,
                          _inner_bounds.top - 10};
    for (const auto& tab : data.tabs) {
        button_bounds.right = button_bounds.left + tab.width;
        _tabs.emplace_back(new TabButton{this, tab, button_bounds});
        button_bounds.left = button_bounds.right + 37;
    }
    _tabs[0]->on()         = true;
    _top_right_border_size = (data.bounds.right - 17) - button_bounds.right;
}

Widget* TabBox::accept_click(Point where) {
    for (const std::unique_ptr<TabButton>& tab : _tabs) {
        if (Widget* w = tab->accept_click(where)) {
            return w;
        }
    }
    for (const std::unique_ptr<Widget>& widget : _tabs[_current_tab]->content()) {
        if (Widget* w = widget->accept_click(where)) {
            return w;
        }
    }
    return nullptr;
}

Widget* TabBox::accept_key(Key which) {
    for (const std::unique_ptr<TabButton>& tab : _tabs) {
        if (Widget* w = tab->accept_key(which)) {
            return w;
        }
    }
    for (const std::unique_ptr<Widget>& widget : _tabs[_current_tab]->content()) {
        if (Widget* w = widget->accept_key(which)) {
            return w;
        }
    }
    return nullptr;
}

Widget* TabBox::accept_button(Gamepad::Button which) {
    for (const std::unique_ptr<TabButton>& tab : _tabs) {
        if (Widget* w = tab->accept_button(which)) {
            return w;
        }
    }
    for (const std::unique_ptr<Widget>& widget : _tabs[_current_tab]->content()) {
        if (Widget* w = widget->accept_button(which)) {
            return w;
        }
    }
    return nullptr;
}

void TabBox::draw(Point offset, InputMode mode) const {
    Rects          rects;
    Rect           uRect;
    int16_t        vcenter, h_border = kInterfaceSmallHBorder;
    uint8_t        shade;
    Hue            color                 = _hue;
    InterfaceStyle style                 = _style;
    int16_t        top_right_border_size = _top_right_border_size;

    Rect r = _inner_bounds;
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

    for (const auto& tab : _tabs) {
        tab->draw(offset, mode);
    }
    for (const auto& widget : _tabs[_current_tab]->content()) {
        widget->draw(offset, mode);
    }
}

Rect TabBox::inner_bounds() const { return _inner_bounds; }

Rect TabBox::outer_bounds() const {
    Rect bounds = initialize_bounds(_inner_bounds);
    bounds.left -= h_border(_style);
    bounds.right += h_border(_style);
    bounds.top -= kInterfaceVEdgeHeight + kInterfaceVCornerHeight + 20;
    bounds.bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
    return bounds;
}

std::vector<const Widget*> TabBox::children() const { return children<const Widget>(); }
std::vector<Widget*>       TabBox::children() { return children<Widget>(); }

template <typename MaybeConstWidget>
std::vector<MaybeConstWidget*> TabBox::children() const {
    std::vector<MaybeConstWidget*> children;
    for (const std::unique_ptr<TabButton>& tab : _tabs) {
        children.push_back(tab.get());
        for (const std::unique_ptr<Widget>& widget : tab->content()) {
            children.push_back(widget.get());
        }
    }
    return children;
}

void TabBox::select(const TabButton& tab) {
    for (const auto& t : _tabs) {
        if (t.get() == &tab) {
            t->on()      = true;
            _current_tab = &t - _tabs.data();
        } else {
            t->on() = false;
        }
    }
}

std::unique_ptr<Widget> Widget::from(const WidgetData& data) {
    switch (data.type()) {
        case WidgetDataBase::Type::NONE: return nullptr;
        case WidgetDataBase::Type::RECT: return std::unique_ptr<Widget>(new BoxRect{data.rect});
        case WidgetDataBase::Type::TEXT: return std::unique_ptr<Widget>(new TextRect{data.text});
        case WidgetDataBase::Type::PICTURE:
            return std::unique_ptr<Widget>(new PictureRect{data.picture});
        case WidgetDataBase::Type::BUTTON:
            return std::unique_ptr<Widget>(new PlainButton{data.button});
        case WidgetDataBase::Type::CHECKBOX:
            return std::unique_ptr<Widget>(new CheckboxButton{data.checkbox});
        case WidgetDataBase::Type::RADIO:
            return std::unique_ptr<Widget>(new RadioButton{data.radio});
        case WidgetDataBase::Type::TAB_BOX:
            return std::unique_ptr<Widget>(new TabBox{data.tab_box});
    }
}

}  // namespace antares
