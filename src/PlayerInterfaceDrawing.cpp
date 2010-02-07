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

#include "PlayerInterfaceDrawing.hpp"

#include "Quickdraw.h"

#include "rezin/MacRoman.hpp"
#include "sfz/String.hpp"
#include "ColorTranslation.hpp"
#include "Debug.hpp"
#include "DirectText.hpp"
#include "Error.hpp"
#include "InterfaceText.hpp"
#include "KeyMapTranslation.hpp"
#include "NateDraw.hpp"
#include "Picture.hpp"
#include "PlayerInterfaceItems.hpp"
#include "Resource.hpp"
#include "StringNumerics.hpp"

using rezin::mac_roman_encoding;
using sfz::BytesPiece;
using sfz::StringPiece;
using sfz::scoped_ptr;

namespace antares {

#define kInterfaceLargeHBorder      13
#define kInterfaceSmallHBorder      3
#define kInterfaceVEdgeHeight       1//2
#define kInterfaceVCornerHeight     2//3
#define kInterfaceVLipHeight        1
#define kInterfaceHTop              2
#define kLabelBottomHeight          6
#define kInterfaceContentBuffer     2

#define kIndicatorVOffset           4
#define kIndicatorInnerOffset       4
#define kRadioIndicatorHOffset      4
#define kCheckIndicatorHOffset      4

#define kScrollArrowWidth           8
#define kScrollArrowHBuffer         2
#define kScrollArrowVBuffer         3

#define kSmallArrowPointHeight      5
#define kSmallArrowBaseHeight       5
#define kLargeArrowPointHeight      9
#define kLargeArrowBaseHeight       9

#define kMaxLineLength              255

#define kMaxKeyNameLength           4       // how many chars can be in name of key for plainButton

enum inlineKindType {
    kNoKind = 0,
    kVPictKind = 1,
    kVClearPictKind = 2
};

extern directTextType* gDirectText;

inline void mDrawPuffUpRect(Rect& mrect, uint8_t mcolor, int mshade, PixMap* pix) {
    SetTranslateColorShadeFore(mcolor, mshade);
    RgbColor color;
    GetRGBTranslateColorShade(&color, mcolor, mshade);
    pix->view(mrect).fill(color);
    SetTranslateColorShadeFore(mcolor, mshade + kLighterColor);
    MoveTo(mrect.left, mrect.bottom - 1);
    MacLineTo(pix, mrect.left, mrect.top);
    MacLineTo(pix, mrect.right - 1, mrect.top);
    SetTranslateColorShadeFore(mcolor, mshade + kDarkerColor);
    MacLineTo(pix, mrect.right - 1, mrect.bottom - 1);
    MacLineTo(pix, mrect.left, mrect.bottom - 1);
}

inline void mDrawPuffUpOval(Rect& mrect, uint8_t mcolor, int mshade, PixMap* pix) {
    SetTranslateColorShadeFore(mcolor, mshade + kDarkerColor);
    mrect.left++;
    mrect.right++;
    FrameOval(mrect);
    mrect.left--;
    mrect.right--;
    mrect.top++;
    mrect.bottom++;
    FrameOval(mrect);
    mrect.top--;
    mrect.bottom--;
    SetTranslateColorShadeFore(mcolor, mshade + kLighterColor);
    mrect.left--;
    mrect.right--;
    FrameOval(mrect);
    mrect.left++;
    mrect.right++;
    mrect.top--;
    mrect.bottom--;
    FrameOval(mrect);
    mrect.top++;
    mrect.bottom++;
    SetTranslateColorShadeFore(mcolor, mshade);
    PaintOval(mrect);
}

inline void mDrawPuffDownRect(Rect& mrect, uint8_t mcolor, int mshade, PixMap* pix) {
    SetTranslateColorFore(BLACK);
    pix->view(mrect).fill(RgbColor::kBlack);
    SetTranslateColorShadeFore(mcolor, mshade + kDarkerColor);
    MoveTo(mrect.left - 1, mrect.bottom);
    MacLineTo(pix, mrect.left - 1, mrect.top - 1);
    MacLineTo(pix, mrect.right, mrect.top - 1);
    SetTranslateColorShadeFore(mcolor, mshade + kLighterColor);
    MacLineTo(pix, mrect.right, mrect.bottom);
    MacLineTo(pix, mrect.left - 1, mrect.bottom);
}

inline void mDrawPuffDownOval(Rect& mrect, uint8_t mcolor, int mshade, PixMap* pix) {
    SetTranslateColorShadeFore(mcolor, mshade + kLighterColor);
    mrect.left++;
    mrect.right++;
    FrameOval(mrect);
    mrect.left--;
    mrect.right--;
    mrect.top++;
    mrect.bottom++;
    FrameOval(mrect);
    mrect.top--;
    mrect.bottom--;
    SetTranslateColorShadeFore(mcolor, mshade + kDarkerColor);
    mrect.left--;
    mrect.right--;
    FrameOval(mrect);
    mrect.left++;
    mrect.right++;
    mrect.top--;
    mrect.bottom--;
    FrameOval(mrect);
    mrect.top++;
    mrect.bottom++;
    SetTranslateColorFore(BLACK);
    PaintOval(mrect);
}

inline void mDrawPuffUpTopBorder(Rect& mrect, Rect& mtrect, uint8_t mcolor, int mshade, int mthisHBorder, PixMap* pix) {
    RgbColor color;
    GetRGBTranslateColorShade(&color, mcolor, mshade);
    SetTranslateColorShadeFore(mcolor, mshade);
    mtrect = Rect(mrect.left - mthisHBorder,
        mrect.top - kInterfaceVEdgeHeight - kInterfaceVCornerHeight,
        mrect.left, mrect.top);
    pix->view(mtrect).fill(color);
    mtrect = Rect(mrect.right,
        mrect.top - kInterfaceVEdgeHeight - kInterfaceVCornerHeight,
        mrect.right + mthisHBorder, mrect.top);
    pix->view(mtrect).fill(color);
    mtrect = Rect(mrect.left,
        mrect.top - kInterfaceVEdgeHeight - kInterfaceVCornerHeight,
        mrect.right, mrect.top  - kInterfaceVLipHeight);
    pix->view(mtrect).fill(color);
    SetTranslateColorShadeFore(mcolor, mshade + kDarkerColor);
    MoveTo(mrect.left - mthisHBorder, mrect.top);
    MacLineTo(pix, mrect.left, mrect.top);
    MacLineTo(pix, mrect.left, mrect.top - kInterfaceVLipHeight);
    MacLineTo(pix, mrect.right, mrect.top - kInterfaceVLipHeight);
    MacLineTo(pix, mrect.right, mrect.top);
    MacLineTo(pix, mrect.right + mthisHBorder, mrect.top);
    MacLineTo(pix, mrect.right + mthisHBorder, mrect.top - kInterfaceVEdgeHeight - kInterfaceVCornerHeight);
    SetTranslateColorShadeFore(mcolor, mshade + kLighterColor);
    MacLineTo(pix, mrect.left - mthisHBorder, mrect.top - kInterfaceVEdgeHeight - kInterfaceVCornerHeight);
    MacLineTo(pix, mrect.left - mthisHBorder, mrect.top);
}

inline void mDrawPuffUpBottomBorder(Rect& mrect, Rect& mtrect, uint8_t mcolor, int mshade, int mthisHBorder, PixMap* pix) {
    RgbColor color;
    GetRGBTranslateColorShade(&color, mcolor, mshade);
    SetTranslateColorShadeFore(mcolor, mshade);
    mtrect = Rect(mrect.left - mthisHBorder,
        mrect.bottom,
        mrect.left, mrect.bottom + kInterfaceVEdgeHeight + kInterfaceVCornerHeight);
    pix->view(mtrect).fill(color);
    mtrect = Rect(mrect.right,
        mrect.bottom,
        mrect.right + mthisHBorder, mrect.bottom + kInterfaceVEdgeHeight + kInterfaceVCornerHeight);
    pix->view(mtrect).fill(color);
    mtrect = Rect(mrect.left,
        mrect.bottom + kInterfaceVLipHeight,
        mrect.right, mrect.bottom + kInterfaceVEdgeHeight + kInterfaceVCornerHeight);
    pix->view(mtrect).fill(color);
    SetTranslateColorShadeFore(mcolor, mshade + kLighterColor);
    MoveTo(mrect.left - mthisHBorder, mrect.bottom + kInterfaceVEdgeHeight + kInterfaceVCornerHeight);
    MacLineTo(pix, mrect.left - mthisHBorder, mrect.bottom);
    MacLineTo(pix, mrect.left, mrect.bottom);
    MacLineTo(pix, mrect.left, mrect.bottom + kInterfaceVLipHeight);
    MacLineTo(pix, mrect.right, mrect.bottom + kInterfaceVLipHeight);
    MacLineTo(pix, mrect.right, mrect.bottom);
    MacLineTo(pix, mrect.right + mthisHBorder, mrect.bottom);
    SetTranslateColorShadeFore(mcolor, mshade + kDarkerColor);
    MacLineTo(pix, mrect.right + mthisHBorder, mrect.bottom + kInterfaceVEdgeHeight + kInterfaceVCornerHeight);
    MacLineTo(pix, mrect.left - mthisHBorder, mrect.bottom + kInterfaceVEdgeHeight + kInterfaceVCornerHeight);
}

inline void mDrawPuffUpTBorder(Rect& mrect, Rect& mtrect, uint8_t mcolor, int mshade, int msheight, int mthisHBorder, PixMap* pix) {
    RgbColor color;
    GetRGBTranslateColorShade(&color, mcolor, mshade);
    SetTranslateColorShadeFore(mcolor, mshade);
    mtrect = Rect(mrect.left - mthisHBorder,
        mrect.top + msheight,
        mrect.left,
        mrect.top + msheight + kLabelBottomHeight);
    pix->view(mtrect).fill(color);
    mtrect = Rect(mrect.right,
        mrect.top + msheight,
        mrect.right + mthisHBorder,
        mrect.top + msheight + kLabelBottomHeight);
    pix->view(mtrect).fill(color);
    mtrect = Rect(mrect.left,
        mrect.top + msheight + kInterfaceVLipHeight,
        mrect.right,
        mrect.top + msheight + kLabelBottomHeight - kInterfaceVLipHeight);
    pix->view(mtrect).fill(color);
    SetTranslateColorShadeFore(mcolor, mshade + kLighterColor);
    MoveTo(mrect.left - mthisHBorder, mrect.top + msheight + kLabelBottomHeight);
    MacLineTo(pix, mrect.left - mthisHBorder, mrect.top + msheight);
    MacLineTo(pix, mrect.left, mrect.top + msheight);
    MacLineTo(pix, mrect.left, mrect.top + msheight + kInterfaceVLipHeight);
    MacLineTo(pix, mrect.right, mrect.top + msheight + kInterfaceVLipHeight);
    MacLineTo(pix, mrect.right, mrect.top + msheight);
    MacLineTo(pix, mrect.right + mthisHBorder, mrect.top + msheight);
    SetTranslateColorShadeFore(mcolor, mshade + kDarkerColor);
    MacLineTo(pix, mrect.right + mthisHBorder, mrect.top + msheight + kLabelBottomHeight);
    MacLineTo(pix, mrect.right, mrect.top + msheight + kLabelBottomHeight);
    MacLineTo(pix, mrect.right, mrect.top + msheight + kLabelBottomHeight - kInterfaceVLipHeight);
    MacLineTo(pix, mrect.left, mrect.top + msheight + kLabelBottomHeight - kInterfaceVLipHeight);
    MacLineTo(pix, mrect.left, mrect.top + msheight + kLabelBottomHeight);
    MacLineTo(pix, mrect.left - mthisHBorder, mrect.top + msheight + kLabelBottomHeight);
}

void DrawPlayerInterfacePlainRect(
        const Rect& rect, uint8_t color, interfaceStyleType style, PixMap* pix) {
    Rect            tRect, uRect;
    short           vcenter, thisHBorder = kInterfaceSmallHBorder;

    if (style == kLarge) {
        thisHBorder = kInterfaceLargeHBorder;
    }
    tRect = rect;
    tRect.left -= kInterfaceContentBuffer;
    tRect.top -= kInterfaceContentBuffer;
    tRect.right += kInterfaceContentBuffer;
    tRect.bottom += kInterfaceContentBuffer;

    // top border
    mDrawPuffUpTopBorder( tRect, uRect, color, DARK, thisHBorder, pix);
    // bottom border

    mDrawPuffUpBottomBorder( tRect, uRect, color, DARK, thisHBorder, pix);

    // main part left border

    vcenter = ( tRect.bottom - tRect.top) / 2;

    uRect = Rect(tRect.left - thisHBorder,
        tRect.top + kInterfaceHTop,
        tRect.left + 1,
        tRect.top + vcenter - kInterfaceVLipHeight + 1);
    mDrawPuffUpRect( uRect, color, DARKER, pix);

    uRect = Rect(tRect.left - thisHBorder,
        tRect.bottom - vcenter + kInterfaceVLipHeight,
        tRect.left + 1,
        tRect.bottom - kInterfaceHTop + 1);
    mDrawPuffUpRect( uRect, color, VERY_DARK, pix);

    // right border

    uRect = Rect(tRect.right,
        tRect.top + kInterfaceHTop,
        tRect.right + thisHBorder + 1,
        tRect.top + vcenter - kInterfaceVLipHeight + 1);
    mDrawPuffUpRect( uRect, color, DARKER, pix);

    uRect = Rect(tRect.right,
        tRect.bottom - vcenter + kInterfaceVLipHeight,
        tRect.right + thisHBorder + 1,
        tRect.bottom - kInterfaceHTop + 1);
    mDrawPuffUpRect( uRect, color, VERY_DARK, pix);

    SetTranslateColorFore( BLACK);
}

void DrawPlayerInterfaceTabBox(
        const Rect& rect, uint8_t color, interfaceStyleType style, PixMap* pix,
        int top_right_border_size) {
    Rect            tRect, uRect;
    short           vcenter, thisHBorder = kInterfaceSmallHBorder;
    unsigned char   shade;

    if ( style == kLarge) thisHBorder = kInterfaceLargeHBorder;
    tRect = rect;
    tRect.left -= kInterfaceContentBuffer;
    tRect.top -= kInterfaceContentBuffer;
    tRect.right += kInterfaceContentBuffer;
    tRect.bottom += kInterfaceContentBuffer;

    // top border
    shade = MEDIUM;
    SetTranslateColorShadeFore( color, shade);
    uRect = Rect((tRect).left - thisHBorder,
        (tRect).top - 3 - kInterfaceVCornerHeight,
        (tRect).left, (tRect).top);
    RgbColor rgb;
    GetRGBTranslateColorShade(&rgb, color, shade);
    pix->view(uRect).fill(rgb);
    uRect = Rect((tRect).right,
        (tRect).top - 3 - kInterfaceVCornerHeight,
        (tRect).right + thisHBorder, (tRect).top);
    pix->view(uRect).fill(rgb);
    uRect = Rect((tRect).left,
        (tRect).top - 3 - kInterfaceVCornerHeight,
        (tRect).left + 6, (tRect).top  - kInterfaceVLipHeight);
    pix->view(uRect).fill(rgb);
    uRect = Rect((tRect).right - top_right_border_size,
        (tRect).top - 3 - kInterfaceVCornerHeight,
        (tRect).right, (tRect).top  - kInterfaceVLipHeight);
    pix->view(uRect).fill(rgb);
    SetTranslateColorShadeFore( color, shade + kDarkerColor);
    MoveTo( (tRect).left - thisHBorder, (tRect).top);
    MacLineTo(pix, (tRect).left, (tRect).top);
    MacLineTo(pix, (tRect).left, (tRect).top - kInterfaceVLipHeight);
    MacLineTo(pix, (tRect).left + 5, (tRect).top - kInterfaceVLipHeight);
    MoveTo( (tRect).right - top_right_border_size, (tRect).top - kInterfaceVLipHeight);
    MacLineTo(pix, (tRect).right, (tRect).top - kInterfaceVLipHeight);
    MacLineTo(pix, (tRect).right, (tRect).top);
    MacLineTo(pix, (tRect).right + thisHBorder, (tRect).top);
    MacLineTo(pix, (tRect).right + thisHBorder, (tRect).top - 3 - kInterfaceVCornerHeight);
    SetTranslateColorShadeFore( color, shade + kLighterColor);
    MacLineTo(pix, (tRect).right - top_right_border_size, (tRect).top - 3 - kInterfaceVCornerHeight);
    MoveTo( (tRect).left + 5, (tRect).top - 3 - kInterfaceVCornerHeight);
    MacLineTo(pix, (tRect).left - thisHBorder, (tRect).top - 3 - kInterfaceVCornerHeight);
    MacLineTo(pix, (tRect).left - thisHBorder, (tRect).top);
    // bottom border

    mDrawPuffUpBottomBorder( tRect, uRect, color, DARK, thisHBorder, pix);

    // main part left border

    vcenter = ( tRect.bottom - tRect.top) / 2;

    uRect = Rect(tRect.left - thisHBorder,
        tRect.top + kInterfaceHTop,
        tRect.left + 1,
        tRect.top + vcenter - kInterfaceVLipHeight + 1);
    mDrawPuffUpRect( uRect, color, DARKER, pix);

    uRect = Rect(tRect.left - thisHBorder,
        tRect.bottom - vcenter + kInterfaceVLipHeight,
        tRect.left + 1,
        tRect.bottom - kInterfaceHTop + 1);
    mDrawPuffUpRect( uRect, color, VERY_DARK, pix);

    // right border

    uRect = Rect(tRect.right,
        tRect.top + kInterfaceHTop,
        tRect.right + thisHBorder + 1,
        tRect.top + vcenter - kInterfaceVLipHeight + 1);
    mDrawPuffUpRect( uRect, color, DARKER, pix);

    uRect = Rect(tRect.right,
        tRect.bottom - vcenter + kInterfaceVLipHeight,
        tRect.right + thisHBorder + 1,
        tRect.bottom - kInterfaceHTop + 1);
    mDrawPuffUpRect( uRect, color, VERY_DARK, pix);

    SetTranslateColorFore( BLACK);
}

void DrawPlayerInterfaceButton(const interfaceItemType& item, PixMap* pix) {
    Rect            tRect, uRect, vRect;
    short           vcenter, swidth, sheight, thisHBorder = kInterfaceSmallHBorder;
    Str255          s;
    unsigned char   shade;
    RgbColor        color;

    if (item.style == kLarge) {
        thisHBorder = kInterfaceLargeHBorder;
    }
    tRect = item.bounds;

    uRect = tRect;
    uRect.right++;
    uRect.bottom++;

    tRect.left -= kInterfaceContentBuffer;
    tRect.top -= kInterfaceContentBuffer;
    tRect.right += kInterfaceContentBuffer;
    tRect.bottom += kInterfaceContentBuffer;

    // top border

    if (item.item.plainButton.status == kDimmed) {
        shade = VERY_DARK;
    } else {
        shade = MEDIUM;
    }

    mDrawPuffUpTopBorder( tRect, uRect, item.color, shade, thisHBorder, pix);
    // bottom border

    mDrawPuffUpBottomBorder( tRect, uRect, item.color, shade, thisHBorder, pix);

    // side border top

    vcenter = ( tRect.bottom - tRect.top) / 2;

    uRect = Rect(tRect.left - thisHBorder, tRect.top + kInterfaceHTop, tRect.left + 1,
            tRect.bottom - kInterfaceHTop + 1);
    vRect = Rect(tRect.right, tRect.top + kInterfaceHTop, tRect.right + thisHBorder + 1,
            tRect.bottom - kInterfaceHTop + 1);
    if (item.item.plainButton.status == kIH_Hilite) {
        shade = LIGHT;
        mDrawPuffUpRect( uRect, item.color, shade, pix);
        mDrawPuffUpRect( vRect, item.color, shade, pix);
    } else {
        if (item.item.plainButton.status == kDimmed) {
            shade = VERY_DARK;
        } else {
            shade = MEDIUM + kSlightlyLighterColor;
        }
        mDrawPuffUpRect( uRect, item.color, shade, pix);
        mDrawPuffUpRect( vRect, item.color, shade, pix);
    }


    if (item.item.plainButton.key == 0)
    {
        uRect = Rect(tRect.left +  kInterfaceContentBuffer,
            tRect.top + kInterfaceContentBuffer,
            tRect.left +  kInterfaceContentBuffer,
            tRect.bottom - kInterfaceContentBuffer);

        if (item.item.plainButton.status == kIH_Hilite)
            shade = LIGHT;
        else shade = DARK;//DARKEST + kSlightlyLighterColor;
        uRect = Rect(tRect.left +  kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
                        tRect.right - kInterfaceContentBuffer + 1,
                        tRect.bottom - kInterfaceContentBuffer + 1);
        SetTranslateColorShadeFore(item.color, shade);

        GetRGBTranslateColorShade(&color, item.color, shade);
        pix->view(uRect).fill(color);

        if (item.item.plainButton.status == kIH_Hilite)
        {
            SetTranslateColorShadeFore(item.color, DARKEST);
            GetRGBTranslateColorShade(&color, item.color, DARKEST);
        }
        else if (item.item.plainButton.status == kDimmed)
        {
            SetTranslateColorShadeFore(item.color, VERY_DARK);
            GetRGBTranslateColorShade(&color, item.color, VERY_DARK);
        }

        else
        {
            SetTranslateColorShadeFore(item.color, LIGHTER);
            GetRGBTranslateColorShade(&color, item.color, LIGHTER);
        }
        GetIndString(s, item.item.plainButton.label.stringID,
                    item.item.plainButton.label.stringNumber);
        swidth = GetInterfaceStringWidth(s, item.style);
        swidth = tRect.left + ( tRect.right - tRect.left) / 2 - swidth / 2;
        sheight = GetInterfaceFontAscent(item.style) + kInterfaceTextVBuffer + tRect.top;
        MoveTo( swidth, sheight);
        DrawInterfaceString(s, item.style, pix, color);
    } else
    {
        // draw the key code

        if (item.item.plainButton.status == kDimmed)
            shade = VERY_DARK;
        else shade = LIGHT;
        GetKeyNumName(s, item.item.plainButton.key);
        swidth = GetInterfaceFontWidth(item.style) * kMaxKeyNameLength;

        uRect = Rect(tRect.left +  kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
                tRect.left + kInterfaceContentBuffer + swidth + kInterfaceTextHBuffer * 2 + 1,
                tRect.bottom - kInterfaceContentBuffer + 1);
        mDrawPuffUpRect(uRect, item.color, shade, pix);

        if (item.item.plainButton.status == kIH_Hilite)
            shade = LIGHT;
        else shade = DARK;//DARKEST;
        vRect = Rect(tRect.left + kInterfaceContentBuffer + swidth + kInterfaceTextHBuffer * 2 + 2,
        tRect.top + kInterfaceContentBuffer,
                        tRect.right - kInterfaceContentBuffer + 1,
                        tRect.bottom - kInterfaceContentBuffer + 1);
        SetTranslateColorShadeFore(item.color, shade);
        GetRGBTranslateColorShade(&color, item.color, shade);
        pix->view(vRect).fill(color);

        swidth = GetInterfaceStringWidth(s, item.style);
        swidth = uRect.left + ( uRect.right - uRect.left) / 2 - swidth / 2;
        MoveTo( swidth, uRect.top + GetInterfaceFontAscent(item.style));
        if (item.item.plainButton.status == kDimmed)
        {
            SetTranslateColorShadeFore(item.color, VERY_DARK);
            GetRGBTranslateColorShade(&color, item.color, VERY_DARK);
        }

        else
        {
            SetTranslateColorShadeFore(item.color, DARKEST);
            GetRGBTranslateColorShade(&color, item.color, DARKEST);
        }

        DrawInterfaceString(s, item.style, pix, color);

        // draw the button title

        if (item.item.plainButton.status == kIH_Hilite)
        {
            SetTranslateColorShadeFore(item.color, DARKEST);
            GetRGBTranslateColorShade(&color, item.color, DARKEST);
        } else if (item.item.plainButton.status == kDimmed)
        {
            SetTranslateColorShadeFore(item.color, DARKEST + kSlightlyLighterColor);
            GetRGBTranslateColorShade(&color, item.color, DARKEST + kSlightlyLighterColor);
        }
        else
        {
            SetTranslateColorShadeFore(item.color, LIGHTER);
            GetRGBTranslateColorShade(&color, item.color, LIGHTER);
        }
        GetIndString(s, item.item.plainButton.label.stringID,
                    item.item.plainButton.label.stringNumber);
        swidth = GetInterfaceStringWidth(s, item.style);
        swidth = uRect.right + ( tRect.right - uRect.right) / 2 - swidth / 2;
        sheight = GetInterfaceFontAscent(item.style) + kInterfaceTextVBuffer + tRect.top;
        MoveTo( swidth, sheight);
        DrawInterfaceString(s, item.style, pix, color);
    }

    SetTranslateColorFore( BLACK);
}

void DrawPlayerInterfaceTabBoxButton(const interfaceItemType& item, PixMap* pix) {
    Rect            tRect, uRect, vRect;
    short           vcenter, swidth, sheight, thisHBorder = kInterfaceSmallHBorder;
    Str255          s;
    unsigned char   shade;
    RgbColor        color;

    if ( item.style == kLarge) thisHBorder = kInterfaceLargeHBorder;
    tRect = item.bounds;

    uRect = tRect;
    uRect.right++;
    uRect.bottom++;

    tRect.left -= kInterfaceContentBuffer;
    tRect.top -= kInterfaceContentBuffer;
    tRect.right += kInterfaceContentBuffer;
    tRect.bottom += kInterfaceContentBuffer;

    // top border

    if ( item.item.radioButton.status == kDimmed)
        shade = VERY_DARK;
    else shade = MEDIUM;

    mDrawPuffUpTopBorder( tRect, uRect, item.color, shade, thisHBorder, pix);
    // bottom border

//  mDrawPuffUpBottomBorder( tRect, uRect, item.color, shade, thisHBorder)

    // side border top

    vcenter = ( tRect.bottom - tRect.top) / 2;

    uRect = Rect(tRect.left - thisHBorder, tRect.top + kInterfaceHTop, tRect.left + 1,
            tRect.bottom - kInterfaceHTop + 1);
    vRect = Rect(tRect.right, tRect.top + kInterfaceHTop, tRect.right + thisHBorder + 1,
            tRect.bottom - kInterfaceHTop + 1);
    if ( !item.item.radioButton.on)
    {
        if ( item.item.radioButton.status == kIH_Hilite)
        {
            shade = LIGHT;
            mDrawPuffUpRect( uRect, item.color, shade, pix);
            mDrawPuffUpRect( vRect, item.color, shade, pix);
        } else
        {
            if ( item.item.radioButton.status == kDimmed)
                shade = VERY_DARK;
            else shade = DARK;
            mDrawPuffUpRect( uRect, item.color, shade, pix);
            mDrawPuffUpRect( vRect, item.color, shade, pix);
        }
        uRect = Rect(uRect.left, uRect.bottom, uRect.right, uRect.bottom + 3);
        vRect = Rect(vRect.left, vRect.bottom, vRect.right, vRect.bottom + 3);
        SetTranslateColorFore( BLACK);
        pix->view(uRect).fill(RgbColor::kBlack);
        pix->view(vRect).fill(RgbColor::kBlack);
        uRect = Rect(uRect.left - 3, uRect.bottom, vRect.right + 3, uRect.bottom + 3);
        shade = MEDIUM;
        SetTranslateColorShadeFore( item.color, shade);
        GetRGBTranslateColorShade(&color, item.color, shade);
        pix->view(uRect).fill(color);
        SetTranslateColorShadeFore( item.color, shade + kLighterColor);
        MoveTo( uRect.left, uRect.top - 1);
        MacLineTo(pix, uRect.right - 1, uRect.top - 1);
        SetTranslateColorShadeFore( item.color, shade + kDarkerColor);
        MoveTo( uRect.left, uRect.bottom);
        MacLineTo(pix, uRect.right - 1, uRect.bottom);
    } else
    {
        if ( item.item.radioButton.status == kIH_Hilite)
        {
            shade = LIGHT;
        } else
        {
            if ( item.item.radioButton.status == kDimmed)
                shade = VERY_DARK;
            else shade = MEDIUM;
        }
        uRect.bottom += 7;
        vRect.bottom += 7;
        SetTranslateColorShadeFore( item.color, shade);
        GetRGBTranslateColorShade(&color, item.color, shade);
        pix->view(uRect).fill(color);
        pix->view(vRect).fill(color);
        SetTranslateColorShadeFore( item.color, shade + kLighterColor);
        MoveTo( uRect.right - 2, uRect.top);
        MacLineTo(pix, uRect.left, uRect.top);
        MacLineTo(pix, uRect.left, uRect.bottom - 5);
        MacLineTo(pix, uRect.left - 3, uRect.bottom - 5);
        MoveTo( vRect.right - 2, vRect.top);
        MacLineTo(pix, vRect.left, vRect.top);
        MacLineTo(pix, vRect.left, vRect.bottom - 2);
        MoveTo( vRect.right, vRect.bottom - 5);
        MacLineTo(pix, vRect.right + 2, vRect.bottom - 5);
        SetTranslateColorShadeFore( item.color, shade + kDarkerColor);
        MoveTo( uRect.right - 1, uRect.top);
        MacLineTo(pix, uRect.right - 1, uRect.bottom - 1);
        MacLineTo(pix, uRect.left - 3, uRect.bottom - 1);
        MoveTo( vRect.right - 1, vRect.top);
        MacLineTo(pix, vRect.right - 1, vRect.bottom - 4);
        MoveTo( vRect.left, vRect.bottom - 1);
        MacLineTo(pix, vRect.right + 2, vRect.bottom - 1);
        uRect = Rect(uRect.left - 3, uRect.bottom - 4, uRect.right - 1, uRect.bottom - 1);
        SetTranslateColorShadeFore( item.color, shade);
        GetRGBTranslateColorShade(&color, item.color, shade);
        pix->view(uRect).fill(color);
        vRect = Rect(vRect.left + 1, vRect.bottom - 4, vRect.right + 3, vRect.bottom - 1);
        pix->view(vRect).fill(color);
        uRect.top--;
        uRect.bottom++;
        uRect.left = uRect.right + 1;
        uRect.right = vRect.left - 1;
        SetTranslateColorFore( BLACK);
        pix->view(uRect).fill(RgbColor::kBlack);
    }


    if ( item.item.radioButton.key == 0)
    {
        uRect = Rect(tRect.left +  kInterfaceContentBuffer,
            tRect.top + kInterfaceContentBuffer,
            tRect.left +  kInterfaceContentBuffer,
            tRect.bottom - kInterfaceContentBuffer);

        if ( !item.item.radioButton.on)
        {
            if ( item.item.radioButton.status == kIH_Hilite)
                shade = LIGHT;
            else shade = DARKER;//DARKEST + kSlightlyLighterColor;
        } else
        {
            shade = MEDIUM;
        }
        uRect = Rect(tRect.left +  kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
                        tRect.right - kInterfaceContentBuffer + 1,
                        tRect.bottom - kInterfaceContentBuffer + 1);
        SetTranslateColorShadeFore( item.color, shade);
        GetRGBTranslateColorShade(&color, item.color, shade);
        pix->view(uRect).fill(color);

        if ( !item.item.radioButton.on)
        {
            if ( item.item.radioButton.status == kIH_Hilite)
            {
                SetTranslateColorShadeFore( item.color, DARKEST);
                GetRGBTranslateColorShade(&color, item.color, DARKEST);
            }
            else if ( item.item.radioButton.status == kDimmed)
            {
                SetTranslateColorShadeFore( item.color, VERY_DARK);
                GetRGBTranslateColorShade(&color, item.color, VERY_DARK);
            } else
            {
                SetTranslateColorShadeFore( item.color, LIGHT);
                GetRGBTranslateColorShade(&color, item.color, LIGHT);
            }
        } else
        {
            SetTranslateColorShadeFore( item.color, VERY_LIGHT);
            GetRGBTranslateColorShade(&color, item.color, VERY_LIGHT);
        }
        GetIndString( s, item.item.radioButton.label.stringID,
                    item.item.radioButton.label.stringNumber);
        swidth = GetInterfaceStringWidth( s, item.style);
        swidth = tRect.left + ( tRect.right - tRect.left) / 2 - swidth / 2;
        sheight = GetInterfaceFontAscent(item.style) + kInterfaceTextVBuffer + tRect.top;
        MoveTo( swidth, sheight);
        DrawInterfaceString(s, item.style, pix, color);
    } else
    {
        // draw the key code


        if ( !item.item.radioButton.on)
        {
            if ( item.item.radioButton.status == kIH_Hilite)
                shade = VERY_LIGHT;
            else shade = DARK;//DARKEST + kSlightlyLighterColor;
        } else
        {
            shade = MEDIUM + kLighterColor;
        }
        GetKeyNumName( s, item.item.radioButton.key);
        swidth = GetInterfaceFontWidth( item.style) * kMaxKeyNameLength;

        uRect = Rect(tRect.left +  kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
                tRect.left + kInterfaceContentBuffer + swidth + kInterfaceTextHBuffer * 2 + 1,
                tRect.bottom - kInterfaceContentBuffer + 1);
        mDrawPuffUpRect( uRect, item.color, shade, pix);

        if ( !item.item.radioButton.on)
        {
            if ( item.item.radioButton.status == kIH_Hilite)
                shade = VERY_LIGHT;
            else shade = DARKER;//DARKEST + kSlightlyLighterColor;
        } else
        {
            shade = MEDIUM;
        }
        vRect = Rect(tRect.left + kInterfaceContentBuffer + swidth + kInterfaceTextHBuffer * 2 + 2,
        tRect.top + kInterfaceContentBuffer,
                        tRect.right - kInterfaceContentBuffer + 1,
                        tRect.bottom - kInterfaceContentBuffer + 1);
        SetTranslateColorShadeFore( item.color, shade);
        GetRGBTranslateColorShade(&color, item.color, shade);
        pix->view(vRect).fill(color);

        swidth = GetInterfaceStringWidth( s, item.style);
        swidth = uRect.left + ( uRect.right - uRect.left) / 2 - swidth / 2;
        MoveTo( swidth, uRect.top + GetInterfaceFontAscent(item.style));
        if ( item.item.radioButton.status == kDimmed)
        {
            SetTranslateColorShadeFore( item.color, VERY_DARK);
            GetRGBTranslateColorShade(&color, item.color, VERY_DARK);
        }

        else
        {
            SetTranslateColorShadeFore( item.color, DARKEST);
            GetRGBTranslateColorShade(&color, item.color, DARKEST);
        }


        DrawInterfaceString(s, item.style, pix, color);


        // draw the button title

        if ( !item.item.radioButton.on)
        {
            if ( item.item.radioButton.status == kIH_Hilite)
            {
                SetTranslateColorShadeFore( item.color, DARKEST);
                GetRGBTranslateColorShade(&color, item.color, DARKEST);
            }
            else if ( item.item.radioButton.status == kDimmed)
            {
                SetTranslateColorShadeFore( item.color, VERY_DARK);
                GetRGBTranslateColorShade(&color, item.color, VERY_DARK);
            } else
            {
                SetTranslateColorShadeFore( item.color, LIGHT);
                GetRGBTranslateColorShade(&color, item.color, LIGHT);
            }
        } else
        {
            SetTranslateColorShadeFore( item.color, VERY_LIGHT);
            GetRGBTranslateColorShade(&color, item.color, VERY_LIGHT);
        }
        GetIndString( s, item.item.radioButton.label.stringID,
                    item.item.radioButton.label.stringNumber);
        swidth = GetInterfaceStringWidth( s, item.style);
        swidth = uRect.right + ( tRect.right - uRect.right) / 2 - swidth / 2;
        sheight = GetInterfaceFontAscent(item.style) + kInterfaceTextVBuffer + tRect.top;
        MoveTo( swidth, sheight);
        DrawInterfaceString(s, item.style, pix, color);
    }

    SetTranslateColorFore( BLACK);
}


void DrawPlayerInterfaceRadioButton(const interfaceItemType& item, PixMap* pix) {
    Rect            tRect, uRect, vRect, wRect;
    short           vcenter, swidth, sheight, thisHBorder = kInterfaceSmallHBorder;
    Str255          s;
    unsigned char   shade;
    RgbColor        color;

    if ( item.style == kLarge) thisHBorder = kInterfaceLargeHBorder;
    tRect = item.bounds;

    tRect.left -= kInterfaceContentBuffer;
    tRect.top -= kInterfaceContentBuffer;
    tRect.right += kInterfaceContentBuffer;
    tRect.bottom += kInterfaceContentBuffer;

    // top border

    if ( item.item.radioButton.status == kDimmed)
        shade = VERY_DARK;
    else shade = MEDIUM;

    mDrawPuffUpTopBorder( tRect, uRect, item.color, shade, thisHBorder, pix);
    // bottom border

    mDrawPuffUpBottomBorder( tRect, uRect, item.color, shade, thisHBorder, pix);

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
            /*tRect.top + vcenter - kInterfaceVLipHeight + 1*/
            tRect.bottom - kInterfaceHTop + 1);
    vRect = Rect(tRect.right, tRect.top + kInterfaceHTop, tRect.right + thisHBorder + 1,
            /*tRect.top + vcenter - kInterfaceVLipHeight + 1*/
            tRect.bottom - kInterfaceHTop + 1);

    if ( item.item.radioButton.status == kIH_Hilite)
    {
        shade = LIGHT;
        mDrawPuffUpRect( uRect, item.color, shade, pix);
        mDrawPuffUpRect( vRect, item.color, shade, pix);

        wRect.left += 2;
        wRect.right += 2;
        SetTranslateColorFore( BLACK);
        FrameOval(wRect);
        wRect.left -= 2;
        wRect.right -= 2;
        mDrawPuffUpOval( wRect, item.color, shade, pix);

        wRect.inset(3, 3);
        mDrawPuffDownOval( wRect, item.color, shade, pix);
        wRect.inset(1, 1);
        if ( !item.item.radioButton.on) SetTranslateColorFore( BLACK);
        else SetTranslateColorShadeFore( item.color, VERY_LIGHT);
        PaintOval(wRect);
    } else
    {
        if ( item.item.radioButton.status == kDimmed)
            shade = VERY_DARK;
        else shade = MEDIUM + kSlightlyLighterColor;
        mDrawPuffUpRect( uRect, item.color, shade, pix);
        mDrawPuffUpRect( vRect, item.color, shade, pix);
        wRect.left += 2;
        wRect.right += 2;
        SetTranslateColorFore( BLACK);
        FrameOval(wRect);
        wRect.left -= 2;
        wRect.right -= 2;
        mDrawPuffUpOval( wRect, item.color, shade, pix);

        wRect.inset(3, 3);
        mDrawPuffDownOval( wRect, item.color, shade, pix);
        wRect.inset(1, 1);
        if ( !item.item.radioButton.on) SetTranslateColorFore( BLACK);
        else if ( item.item.radioButton.status == kActive)
            SetTranslateColorShadeFore( item.color, LIGHT);
        else SetTranslateColorShadeFore( item.color, MEDIUM);
        PaintOval(wRect);
    }

    uRect = Rect(tRect.left +  kInterfaceContentBuffer,
        tRect.top + kInterfaceContentBuffer,
        tRect.left +  kInterfaceContentBuffer,
        tRect.bottom - kInterfaceContentBuffer);

    if ( item.item.radioButton.status == kIH_Hilite)
        shade = LIGHT;
    else shade = DARKEST + kSlightlyLighterColor;
    uRect = Rect(tRect.left +  kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
                    tRect.right - kInterfaceContentBuffer + 1,
                    tRect.bottom - kInterfaceContentBuffer + 1);
    SetTranslateColorShadeFore( item.color, shade);
    GetRGBTranslateColorShade(&color, item.color, shade);
    pix->view(uRect).fill(color);

    if ( item.item.radioButton.status == kIH_Hilite)
    {
        SetTranslateColorShadeFore( item.color, DARKEST);
        GetRGBTranslateColorShade(&color, item.color, DARKEST);
    }

    else if ( item.item.radioButton.status == kDimmed)
    {
        SetTranslateColorShadeFore( item.color, DARK);
        GetRGBTranslateColorShade(&color, item.color, DARK);
    }
    else
    {
        SetTranslateColorShadeFore( item.color, LIGHT);
        GetRGBTranslateColorShade(&color, item.color, LIGHT);
    }
    GetIndString( s, item.item.radioButton.label.stringID,
                item.item.radioButton.label.stringNumber);
    swidth = GetInterfaceStringWidth( s, item.style);
    swidth = tRect.left + ( tRect.right - tRect.left) / 2 - swidth / 2;
    sheight = GetInterfaceFontAscent(item.style) + kInterfaceTextVBuffer + tRect.top;
    MoveTo( swidth, sheight);
    DrawInterfaceString(s, item.style, pix, color);

    SetTranslateColorFore( BLACK);
}

void DrawPlayerInterfaceCheckBox(const interfaceItemType& item, PixMap* pix) {
    Rect            tRect, uRect, vRect, wRect;
    short           vcenter, swidth, sheight, thisHBorder = kInterfaceSmallHBorder;
    Str255          s;
    unsigned char   shade;
    RgbColor        color;

    if ( item.style == kLarge) thisHBorder = kInterfaceLargeHBorder;
    tRect = item.bounds;

    tRect.left -= kInterfaceContentBuffer;
    tRect.top -= kInterfaceContentBuffer;
    tRect.right += kInterfaceContentBuffer;
    tRect.bottom += kInterfaceContentBuffer;

    // top border

    if ( item.item.checkboxButton.status == kDimmed)
        shade = VERY_DARK;
    else shade = MEDIUM;

    mDrawPuffUpTopBorder( tRect, uRect, item.color, shade, thisHBorder, pix);
    // bottom border

    mDrawPuffUpBottomBorder( tRect, uRect, item.color, shade, thisHBorder, pix);

    // side border top

    vcenter = ( tRect.bottom - tRect.top) / 2;
    swidth = (tRect.top - kInterfaceVEdgeHeight - kInterfaceVCornerHeight + kIndicatorVOffset);
    sheight = (tRect.bottom + kInterfaceVEdgeHeight + kInterfaceVCornerHeight - kIndicatorVOffset) -
            swidth;

    wRect = Rect(tRect.left - thisHBorder - kCheckIndicatorHOffset - sheight, swidth,
            tRect.left - thisHBorder - kCheckIndicatorHOffset + 1, swidth + sheight + 1);

    uRect = Rect(tRect.left - thisHBorder - kCheckIndicatorHOffset + 2,
            tRect.top + kInterfaceHTop,
            tRect.left + 1,
            /*tRect.top + vcenter - kInterfaceVLipHeight + 1*/
            tRect.bottom - kInterfaceHTop + 1);
    vRect = Rect(tRect.right, tRect.top + kInterfaceHTop, tRect.right + thisHBorder + 1,
            /*tRect.top + vcenter - kInterfaceVLipHeight + 1*/
            tRect.bottom - kInterfaceHTop + 1);

    if ( item.item.checkboxButton.status == kIH_Hilite)
    {
        shade = LIGHT;
        mDrawPuffUpRect( uRect, item.color, shade, pix);
        mDrawPuffUpRect( vRect, item.color, shade, pix);
        mDrawPuffUpRect( wRect, item.color, shade, pix);
        wRect.inset(3, 3);
        mDrawPuffDownRect( wRect, item.color, shade, pix);
        wRect.inset(1, 1);
        if ( !item.item.checkboxButton.on) {
            color = RgbColor::kBlack;
        } else {
            GetRGBTranslateColorShade(&color, item.color, VERY_LIGHT);
        }
        RGBForeColor(color);
        pix->view(wRect).fill(color);
    } else
    {
        if ( item.item.checkboxButton.status == kDimmed)
            shade = VERY_DARK;
        else shade = MEDIUM + kSlightlyLighterColor;
        mDrawPuffUpRect( uRect, item.color, shade, pix);
        mDrawPuffUpRect( vRect, item.color, shade, pix);
        mDrawPuffUpRect( wRect, item.color, shade, pix);
        wRect.inset(3, 3);
        mDrawPuffDownRect( wRect, item.color, shade, pix);
        wRect.inset(1, 1);
        if (!item.item.checkboxButton.on) {
            color = RgbColor::kBlack;
        } else if (item.item.checkboxButton.status == kActive) {
            GetRGBTranslateColorShade(&color, item.color, LIGHT);
        } else {
            GetRGBTranslateColorShade(&color, item.color, MEDIUM);
        }
        RGBForeColor(color);
        pix->view(wRect).fill(color);
    }

    uRect = Rect(tRect.left +  kInterfaceContentBuffer,
        tRect.top + kInterfaceContentBuffer,
        tRect.left +  kInterfaceContentBuffer,
        tRect.bottom - kInterfaceContentBuffer);

    if ( item.item.checkboxButton.status == kIH_Hilite)
        shade = LIGHT;
    else shade = DARKEST + kSlightlyLighterColor;
    uRect = Rect(tRect.left +  kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
                    tRect.right - kInterfaceContentBuffer + 1,
                    tRect.bottom - kInterfaceContentBuffer + 1);
    SetTranslateColorShadeFore( item.color, shade);
    GetRGBTranslateColorShade(&color, item.color, shade);
    pix->view(uRect).fill(color);

    if ( item.item.checkboxButton.status == kIH_Hilite)
    {
        SetTranslateColorShadeFore( item.color, DARKEST);
        GetRGBTranslateColorShade(&color, item.color, DARKEST);
    } else if ( item.item.checkboxButton.status == kDimmed)
    {
        SetTranslateColorShadeFore( item.color, DARK);
        GetRGBTranslateColorShade(&color, item.color, DARK);
    } else
    {
        SetTranslateColorShadeFore( item.color, LIGHT);
        GetRGBTranslateColorShade(&color, item.color, LIGHT);
    }
    GetIndString( s, item.item.checkboxButton.label.stringID,
                item.item.checkboxButton.label.stringNumber);
    swidth = GetInterfaceStringWidth( s, item.style);
    swidth = tRect.left + ( tRect.right - tRect.left) / 2 - swidth / 2;
    sheight = GetInterfaceFontAscent(item.style) + kInterfaceTextVBuffer + tRect.top;
    MoveTo( swidth, sheight);
    DrawInterfaceString(s, item.style, pix, color);

    SetTranslateColorFore( BLACK);
}

void DrawPlayerInterfaceLabeledBox(const interfaceItemType& item, PixMap* pix) {
    Rect            tRect, uRect;
    short           vcenter, swidth, sheight, thisHBorder = kInterfaceSmallHBorder;
    Str255          s;
    unsigned char   shade;
    RgbColor        color;

    if (item.style == kLarge) {
        thisHBorder = kInterfaceLargeHBorder;
    }
    tRect = item.bounds;
    tRect.left -= kInterfaceContentBuffer;
    tRect.top -= kInterfaceContentBuffer + GetInterfaceFontHeight(item.style) +
            kInterfaceTextVBuffer * 2 + kLabelBottomHeight;
    tRect.right += kInterfaceContentBuffer;
    tRect.bottom += kInterfaceContentBuffer;

    shade = DARK;

    mDrawPuffUpTopBorder( tRect, uRect, item.color, shade, thisHBorder, pix);
    // bottom border

    mDrawPuffUpBottomBorder( tRect, uRect, item.color, shade, thisHBorder, pix);


    // draw the string

    GetIndString( s, item.item.labeledRect.label.stringID, item.item.labeledRect.label.stringNumber);
    swidth = GetInterfaceStringWidth( s, item.style) + kInterfaceTextHBuffer * 2;
    swidth = ( tRect.right - tRect.left) - swidth;
    sheight = GetInterfaceFontHeight( item.style) + kInterfaceTextVBuffer * 2;

    uRect = Rect(tRect.left + kInterfaceTextHBuffer - 1,
        tRect.top + kInterfaceHTop,
        tRect.right - swidth - kInterfaceTextHBuffer + 1,
        tRect.top + sheight - kInterfaceHTop);
    SetTranslateColorShadeFore( item.color, VERY_DARK);
    GetRGBTranslateColorShade(&color, item.color, VERY_DARK);
    pix->view(uRect).fill(color);

    SetTranslateColorShadeFore( item.color, LIGHT);
    GetRGBTranslateColorShade(&color, item.color, LIGHT);

    MoveTo( tRect.left + kInterfaceTextHBuffer, tRect.top + GetInterfaceFontAscent( item.style) +
            kInterfaceTextVBuffer);
    DrawInterfaceString(s, item.style, pix, color);

    // string left border

    shade = MEDIUM;
    vcenter = sheight / 2;

    uRect = Rect(tRect.left - thisHBorder,
            tRect.top + kInterfaceHTop,
            tRect.left + 1, tRect.top + sheight - kInterfaceHTop + 1);
    mDrawPuffUpRect( uRect, item.color, shade, pix);

    // string right border

    shade = MEDIUM;
    uRect = Rect(tRect.right - swidth,
        tRect.top + kInterfaceHTop,
        tRect.right - 2,
        tRect.top + sheight - kInterfaceHTop + 1);
    mDrawPuffUpRect( uRect, item.color, shade, pix);
    uRect = Rect(tRect.right,
        tRect.top + kInterfaceHTop,
        tRect.right + thisHBorder + 1,
        tRect.top + sheight - kInterfaceHTop + 1);
    mDrawPuffUpRect( uRect, item.color, shade, pix);

    // string bottom border

    mDrawPuffUpTBorder( tRect, uRect, item.color, DARK, sheight, thisHBorder, pix);

    // main part left border

    tRect.top += sheight + kLabelBottomHeight;

    vcenter = ( tRect.bottom - tRect.top) / 2;

    uRect = Rect(tRect.left - thisHBorder,
        tRect.top + kInterfaceHTop,
        tRect.left + 1,
        tRect.top + vcenter - kInterfaceVLipHeight + 1);
    mDrawPuffUpRect( uRect, item.color, DARKER, pix);

    uRect = Rect(tRect.left - thisHBorder,
        tRect.bottom - vcenter + kInterfaceVLipHeight,
        tRect.left + 1,
        tRect.bottom - kInterfaceHTop + 1);
    mDrawPuffUpRect( uRect, item.color, VERY_DARK, pix);

    // right border

    uRect = Rect(tRect.right,
        tRect.top + kInterfaceHTop,
        tRect.right + thisHBorder + 1,
        tRect.top + vcenter - kInterfaceVLipHeight + 1);
    mDrawPuffUpRect( uRect, item.color, DARKER, pix);

    uRect = Rect(tRect.right,
        tRect.bottom - vcenter + kInterfaceVLipHeight,
        tRect.right + thisHBorder + 1,
        tRect.bottom - kInterfaceHTop + 1);
    mDrawPuffUpRect( uRect, item.color, VERY_DARK, pix);

    SetTranslateColorFore( BLACK);
}

void DrawPlayerInterfaceList(const interfaceItemType& item, PixMap* pix) {
    Rect            tRect, uRect;
    short           vcenter, swidth, sheight, thisHBorder = kInterfaceSmallHBorder;
    Str255          s;
    RgnHandle       clipRgn = nil;
    RgbColor        color;

    if ( item.style == kLarge) thisHBorder = kInterfaceLargeHBorder;
    tRect = item.bounds;

    tRect.left -= kInterfaceContentBuffer;
    tRect.top -= kInterfaceContentBuffer + GetInterfaceFontAscent( item.style) +
            kInterfaceTextVBuffer * 2 + kLabelBottomHeight;
    tRect.right += kInterfaceContentBuffer;
    tRect.bottom += kInterfaceContentBuffer;

    SetTranslateColorShadeFore( item.color, DARK);

    // top border

    MoveTo( tRect.left, tRect.top);
    MacLineTo(pix, tRect.left - thisHBorder, tRect.top);
    MacLineTo(pix, tRect.left - thisHBorder, tRect.top - kInterfaceVEdgeHeight);
    MacLineTo(pix, tRect.left, tRect.top - kInterfaceVEdgeHeight - kInterfaceVCornerHeight);
    MacLineTo(pix, tRect.right, tRect.top - kInterfaceVEdgeHeight - kInterfaceVCornerHeight);
    MacLineTo(pix, tRect.right + thisHBorder, tRect.top + - kInterfaceVEdgeHeight);
    MacLineTo(pix, tRect.right + thisHBorder, tRect.top);
    MacLineTo(pix, tRect.right, tRect.top);
    MacLineTo(pix, tRect.right, tRect.top - kInterfaceVLipHeight);
    MacLineTo(pix, tRect.left, tRect.top - kInterfaceVLipHeight);
    MacLineTo(pix, tRect.left, tRect.top);

    // bottom border

    MoveTo( tRect.left, tRect.bottom);
    MacLineTo(pix, tRect.left - thisHBorder, tRect.bottom);
    MacLineTo(pix, tRect.left - thisHBorder, tRect.bottom + kInterfaceVEdgeHeight);
    MacLineTo(pix, tRect.left, tRect.bottom + kInterfaceVEdgeHeight + kInterfaceVCornerHeight);
    MacLineTo(pix, tRect.right, tRect.bottom + kInterfaceVEdgeHeight + kInterfaceVCornerHeight);
    MacLineTo(pix, tRect.right + thisHBorder, tRect.bottom + kInterfaceVEdgeHeight);
    MacLineTo(pix, tRect.right + thisHBorder, tRect.bottom);
    MacLineTo(pix, tRect.right, tRect.bottom);
    MacLineTo(pix, tRect.right, tRect.bottom + kInterfaceVLipHeight);
    MacLineTo(pix, tRect.left, tRect.bottom + kInterfaceVLipHeight);
    MacLineTo(pix, tRect.left, tRect.bottom);

    // draw the string

    SetTranslateColorShadeFore( item.color, LIGHT);
    GetRGBTranslateColorShade(&color, item.color, LIGHT);

    GetIndString( s, item.item.listRect.label.stringID, item.item.listRect.label.stringNumber);
    swidth = GetInterfaceStringWidth( s, item.style) + kInterfaceTextHBuffer * 2;
    sheight = GetInterfaceFontAscent(item.style) + kInterfaceTextVBuffer * 2;
    MoveTo( tRect.left + kInterfaceTextHBuffer, tRect.top + sheight - kInterfaceTextVBuffer);
    DrawInterfaceString(s, item.style, pix, color);

    // string left border

    SetTranslateColorShadeFore( item.color, MEDIUM);
    vcenter = sheight / 2;
    swidth = ( tRect.right - tRect.left) - swidth;

    MoveTo( tRect.left, tRect.top + kInterfaceHTop);
    MacLineTo(pix, tRect.left - thisHBorder, tRect.top + kInterfaceHTop);
    MacLineTo(pix, tRect.left - thisHBorder, tRect.top + vcenter - kInterfaceVLipHeight);
    MacLineTo(pix, tRect.left, tRect.top + vcenter - kInterfaceVLipHeight);
    MacLineTo(pix, tRect.left, tRect.top + kInterfaceHTop);

    SetTranslateColorShadeFore( item.color, DARK);
    MoveTo( tRect.left, tRect.top + sheight - vcenter + kInterfaceVLipHeight);
    MacLineTo(pix, tRect.left - thisHBorder, tRect.top + sheight- vcenter + kInterfaceVLipHeight);
    MacLineTo(pix, tRect.left - thisHBorder, tRect.top + sheight - kInterfaceHTop);
    MacLineTo(pix, tRect.left, tRect.top + sheight - kInterfaceHTop);
    MacLineTo(pix, tRect.left, tRect.top + sheight - vcenter + kInterfaceVLipHeight);

    // string right border

    SetTranslateColorShadeFore( item.color, MEDIUM);
    MoveTo( tRect.right - swidth, tRect.top + kInterfaceHTop);
    MacLineTo(pix, tRect.right + thisHBorder, tRect.top + kInterfaceHTop);
    MacLineTo(pix, tRect.right + thisHBorder, tRect.top + vcenter - kInterfaceVLipHeight);
    MacLineTo(pix, tRect.right - swidth, tRect.top + vcenter - kInterfaceVLipHeight);
    MacLineTo(pix, tRect.right - swidth, tRect.top + kInterfaceHTop);

    SetTranslateColorShadeFore( item.color, DARK);
    MoveTo( tRect.right - swidth, tRect.top + sheight - vcenter + kInterfaceVLipHeight);
    MacLineTo(pix, tRect.right + thisHBorder, tRect.top + sheight - vcenter + kInterfaceVLipHeight);
    MacLineTo(pix, tRect.right + thisHBorder, tRect.top + sheight - kInterfaceHTop);
    MacLineTo(pix, tRect.right - swidth, tRect.top + sheight - kInterfaceHTop);
    MacLineTo(pix, tRect.right - swidth, tRect.top + sheight - vcenter + kInterfaceVLipHeight);

    // string bottom border

    SetTranslateColorShadeFore( item.color, DARK);
    MoveTo( tRect.left, tRect.top + sheight);
    MacLineTo(pix, tRect.left, tRect.top + sheight + kInterfaceVLipHeight);
    MacLineTo(pix, tRect.right, tRect.top + sheight + kInterfaceVLipHeight);
    MacLineTo(pix, tRect.right, tRect.top + sheight);
    MacLineTo(pix, tRect.right + thisHBorder, tRect.top + sheight);
    MacLineTo(pix, tRect.right + thisHBorder, tRect.top + sheight + kLabelBottomHeight);
    MacLineTo(pix, tRect.right, tRect.top + sheight + kLabelBottomHeight);
    MacLineTo(pix, tRect.right, tRect.top + sheight + kLabelBottomHeight - kInterfaceVLipHeight);
    MacLineTo(pix, tRect.left, tRect.top + sheight + kLabelBottomHeight - kInterfaceVLipHeight);
    MacLineTo(pix, tRect.left, tRect.top + sheight + kLabelBottomHeight);
    MacLineTo(pix, tRect.left - thisHBorder, tRect.top + sheight + kLabelBottomHeight);
    MacLineTo(pix, tRect.left - thisHBorder, tRect.top + sheight);
    MacLineTo(pix, tRect.left, tRect.top + sheight);

    // main part left border

    tRect.top += sheight + kLabelBottomHeight;

    vcenter = ( tRect.bottom - tRect.top) / 2;

    SetTranslateColorShadeFore( item.color, DARKER);
    MoveTo( tRect.left, tRect.top + kInterfaceHTop);
    MacLineTo(pix, tRect.left - thisHBorder, tRect.top + kInterfaceHTop);
    MacLineTo(pix, tRect.left - thisHBorder, tRect.top + vcenter - kInterfaceVLipHeight);
    MacLineTo(pix, tRect.left, tRect.top + vcenter - kInterfaceVLipHeight);
    MacLineTo(pix, tRect.left, tRect.top + kInterfaceHTop);

    SetTranslateColorShadeFore( item.color, VERY_DARK);
    MoveTo( tRect.left, tRect.bottom - vcenter + kInterfaceVLipHeight);
    MacLineTo(pix, tRect.left - thisHBorder, tRect.bottom - vcenter + kInterfaceVLipHeight);
    MacLineTo(pix, tRect.left - thisHBorder, tRect.bottom - kInterfaceHTop);
    MacLineTo(pix, tRect.left, tRect.bottom - kInterfaceHTop);
    MacLineTo(pix, tRect.left, tRect.bottom - vcenter + kInterfaceVLipHeight);

    // draw the list

    interfaceItemStatusEnum lineUpStatus = kDimmed;
    interfaceItemStatusEnum lineDownStatus = kDimmed;
    interfaceItemStatusEnum pageUpStatus = kDimmed;
    interfaceItemStatusEnum pageDownStatus = kDimmed;

    if ((item.item.listRect.getListLength != nil) && (item.item.listRect.getItemString != nil)) {
        clipRgn = NewRgn();
        GetClip( clipRgn);
        tRect = item.bounds;
        ClipRect(tRect);

        SetTranslateColorShadeFore( item.color, VERY_LIGHT);
        sheight = (*(item.item.listRect.getListLength))();
        vcenter = tRect.top;
        swidth = item.item.listRect.topItem;
        while (( swidth < (*(item.item.listRect.getListLength))()) &&
                ( vcenter < tRect.bottom))
        {
            vcenter += GetInterfaceFontAscent(item.style) + kInterfaceTextVBuffer;
            uRect = Rect(tRect.left, vcenter - GetInterfaceFontAscent(item.style) -
                    kInterfaceTextVBuffer, tRect.right, vcenter);
            if ( (*(item.item.listRect.itemHilited))( swidth, false))
            {
                SetTranslateColorShadeFore( item.color, LIGHT);
                GetRGBTranslateColorShade(&color, item.color, LIGHT);
                pix->view(uRect).fill(color);
                SetTranslateColorShadeFore( item.color, VERY_DARK);
                GetRGBTranslateColorShade(&color, item.color, VERY_DARK);
            } else
            {
                DefaultColors();
                pix->view(uRect).fill(RgbColor::kBlack);
                SetTranslateColorShadeFore( item.color, LIGHT);
                GetRGBTranslateColorShade(&color, item.color, LIGHT);
            }
            MoveTo( tRect.left, vcenter);
            (*(item.item.listRect.getItemString))( swidth, s);
            DrawInterfaceString(s, item.style, pix, color);
            swidth++;
        }
        SetClip( clipRgn);
        DisposeRgn( clipRgn);


        if (item.item.listRect.topItem == 0) {
            lineUpStatus = pageUpStatus = kDimmed;
        } else {
            lineUpStatus = pageUpStatus = kActive;
        }
        if (vcenter < tRect.bottom) {
            lineDownStatus = pageDownStatus = kDimmed;
        } else {
            lineDownStatus = pageDownStatus = kActive;
        }
    }
    // right border

    DrawPlayerListLineUp(item, lineUpStatus, pix);
    DrawPlayerListPageUp(item, pageUpStatus, pix);
    DrawPlayerListLineDown(item, lineDownStatus, pix);
    DrawPlayerListPageDown(item, pageDownStatus, pix);
    SetTranslateColorFore( BLACK);
}

void DrawPlayerInterfaceListEntry(
        const interfaceItemType& item, short whichEntry, PixMap* pix) {
    Str255          s;
    RgnHandle       clipRgn = nil;
    Rect            tRect, uRect;
    short           swidth, vcenter;
    RgbColor        color;

    if (( item.item.listRect.getListLength != nil) && ( item.item.listRect.getItemString != nil))
    {
        clipRgn = NewRgn();
        GetClip( clipRgn);
        tRect = item.bounds;
        ClipRect(tRect);

        SetTranslateColorShadeFore( item.color, VERY_LIGHT);
        vcenter = tRect.top + ( whichEntry - item.item.listRect.topItem) *
                (GetInterfaceFontAscent(item.style) + kInterfaceTextVBuffer);
        swidth = whichEntry;
        if (( swidth < (*(item.item.listRect.getListLength))()) &&
                ( vcenter < tRect.bottom))
        {
            vcenter += GetInterfaceFontAscent(item.style) + kInterfaceTextVBuffer;
            uRect = Rect(tRect.left, vcenter - GetInterfaceFontAscent(item.style) -
                    kInterfaceTextVBuffer, tRect.right, vcenter);
            if ( (*(item.item.listRect.itemHilited))( swidth, false))
            {
                SetTranslateColorShadeFore( item.color, LIGHT);
                GetRGBTranslateColorShade(&color, item.color, LIGHT);
                pix->view(uRect).fill(color);
                SetTranslateColorShadeFore( item.color, VERY_DARK);
                GetRGBTranslateColorShade(&color, item.color, VERY_DARK);
            } else
            {
                GetRGBTranslateColorShade(&color, item.color, VERY_LIGHT);
                pix->view(uRect).fill(color);
                SetTranslateColorShadeFore( item.color, LIGHT);
                GetRGBTranslateColorShade(&color, item.color, LIGHT);
            }
            MoveTo( tRect.left, vcenter);
            (*(item.item.listRect.getItemString))(swidth, s);
            DrawInterfaceString(s, item.style, pix, color);
            swidth++;
        }
        SetClip( clipRgn);
        DisposeRgn( clipRgn);

    }
}

void DrawPlayerListLineUp(const interfaceItemType& item, interfaceItemStatusEnum status, PixMap* pix) {
    Rect        tRect;
    short       thisHBorder = kInterfaceSmallHBorder;
    RgbColor color;

    if ( item.style == kLarge) thisHBorder = kInterfaceLargeHBorder;
    GetPlayerListLineUpRect(item, &tRect);

    if (status != kIH_Hilite)
    {
        if (status == kDimmed) SetTranslateColorShadeFore( item.color, DARKER);
        else SetTranslateColorShadeFore( item.color, LIGHT);
        FrameRect(pix, tRect);
        SetTranslateColorFore( BLACK);
        tRect.inset(1, 1);
        pix->view(tRect).fill(RgbColor::kBlack);
        if (status == kDimmed) SetTranslateColorShadeFore( item.color, DARKER);
        else SetTranslateColorShadeFore( item.color, LIGHT);
        tRect.inset(-1, -1);
    } else
    {
        SetTranslateColorShadeFore( item.color, VERY_LIGHT);
        GetRGBTranslateColorShade(&color, item.color, VERY_LIGHT);
        pix->view(tRect).fill(color);
        SetTranslateColorFore( BLACK);
    }

    MoveTo( tRect.left + kScrollArrowHBuffer + kScrollArrowWidth / 2, tRect.top +
            kScrollArrowVBuffer);
    MacLineTo(pix, tRect.left + kScrollArrowWidth + kScrollArrowHBuffer, tRect.top +
            kScrollArrowVBuffer + kSmallArrowPointHeight);
    MacLineTo(pix, tRect.left + kScrollArrowWidth + kScrollArrowHBuffer, tRect.top + kScrollArrowVBuffer +
            kSmallArrowPointHeight + kSmallArrowBaseHeight);
    MacLineTo(pix, tRect.left + kScrollArrowHBuffer, tRect.top + kScrollArrowVBuffer +
            kSmallArrowPointHeight + kSmallArrowBaseHeight);
    MacLineTo(pix, tRect.left + kScrollArrowHBuffer, tRect.top + kScrollArrowVBuffer +
            kSmallArrowPointHeight);
    MacLineTo(pix, tRect.left + kScrollArrowHBuffer + kScrollArrowWidth / 2, tRect.top +
            kScrollArrowVBuffer);
}

void GetPlayerListLineUpRect(const interfaceItemType& item, Rect *dRect) {
    short       thisHBorder = kInterfaceSmallHBorder;

    if ( item.style == kLarge) thisHBorder = kInterfaceLargeHBorder;
    *dRect = item.bounds;
    dRect->left = dRect->right + kInterfaceContentBuffer;
    dRect->right = dRect->left + thisHBorder + 1;
    dRect->top = dRect->top - kInterfaceContentBuffer + kInterfaceHTop;
    dRect->bottom = dRect->top + kScrollArrowVBuffer * 2 + kSmallArrowPointHeight +
        kSmallArrowBaseHeight + 1;
}

void DrawPlayerListPageUp(const interfaceItemType& item, interfaceItemStatusEnum status, PixMap* pix) {
    Rect        tRect;
    short       thisHBorder = kInterfaceSmallHBorder;
    RgbColor color;

    if ( item.style == kLarge) thisHBorder = kInterfaceLargeHBorder;
    GetPlayerListPageUpRect(item, &tRect);

    if (status != kIH_Hilite)
    {
        if (status == kDimmed) SetTranslateColorShadeFore( item.color, DARKER);
        else SetTranslateColorShadeFore( item.color, LIGHT);
        FrameRect(pix, tRect);
        SetTranslateColorFore( BLACK);
        tRect.inset(1, 1);
        pix->view(tRect).fill(RgbColor::kBlack);
        if (status == kDimmed) SetTranslateColorShadeFore( item.color, DARKER);
        else SetTranslateColorShadeFore( item.color, LIGHT);
        tRect.inset(-1, -1);
    } else
    {
        SetTranslateColorShadeFore( item.color, VERY_LIGHT);
        GetRGBTranslateColorShade(&color, item.color, VERY_LIGHT);
        pix->view(tRect).fill(color);
        SetTranslateColorFore( BLACK);
    }

    MoveTo( tRect.left + kScrollArrowHBuffer + kScrollArrowWidth / 2, tRect.top +
            kScrollArrowVBuffer);
    MacLineTo(pix, tRect.left + kScrollArrowWidth + kScrollArrowHBuffer, tRect.top +
            kScrollArrowVBuffer + kLargeArrowPointHeight);
    MacLineTo(pix, tRect.left + kScrollArrowWidth + kScrollArrowHBuffer, tRect.top + kScrollArrowVBuffer +
            kLargeArrowPointHeight + kLargeArrowBaseHeight);
    MacLineTo(pix, tRect.left + kScrollArrowHBuffer, tRect.top + kScrollArrowVBuffer +
            kLargeArrowPointHeight + kLargeArrowBaseHeight);
    MacLineTo(pix, tRect.left + kScrollArrowHBuffer, tRect.top + kScrollArrowVBuffer +
            kLargeArrowPointHeight);
    MacLineTo(pix, tRect.left + kScrollArrowHBuffer + kScrollArrowWidth / 2, tRect.top +
            kScrollArrowVBuffer);
}

void GetPlayerListPageUpRect(const interfaceItemType& item, Rect *dRect) {
    GetPlayerListLineUpRect(item, dRect);
    dRect->top = dRect->bottom + kInterfaceVLipHeight;
    dRect->bottom = dRect->top + kScrollArrowVBuffer * 2 + kLargeArrowPointHeight +
            kLargeArrowBaseHeight + 1;
}

void DrawPlayerListLineDown(const interfaceItemType& item, interfaceItemStatusEnum status, PixMap* pix) {
    Rect        tRect;
    short       thisHBorder = kInterfaceSmallHBorder;
    RgbColor color;

    if ( item.style == kLarge) thisHBorder = kInterfaceLargeHBorder;
    GetPlayerListLineDownRect( item, &tRect);

    if (status != kIH_Hilite)
    {
        if (status == kDimmed) SetTranslateColorShadeFore( item.color, DARKER);
        else SetTranslateColorShadeFore( item.color, LIGHT);
        FrameRect(pix, tRect);
        SetTranslateColorFore( BLACK);
        tRect.inset(1, 1);
        pix->view(tRect).fill(RgbColor::kBlack);
        if (status == kDimmed) SetTranslateColorShadeFore( item.color, DARKER);
        else SetTranslateColorShadeFore( item.color, LIGHT);
        tRect.inset(-1, -1);
    } else
    {
        SetTranslateColorShadeFore( item.color, VERY_LIGHT);
        GetRGBTranslateColorShade(&color, item.color, VERY_LIGHT);
        pix->view(tRect).fill(color);
        SetTranslateColorFore( BLACK);
    }

    tRect.bottom -= 1;

    MoveTo( tRect.left + kScrollArrowHBuffer + kScrollArrowWidth / 2, tRect.bottom -
            kScrollArrowVBuffer);
    MacLineTo(pix, tRect.left + kScrollArrowWidth + kScrollArrowHBuffer, tRect.bottom -
            (kScrollArrowVBuffer + kSmallArrowPointHeight));
    MacLineTo(pix, tRect.left + kScrollArrowWidth + kScrollArrowHBuffer, tRect.bottom -
            (kScrollArrowVBuffer + kSmallArrowPointHeight + kSmallArrowBaseHeight));
    MacLineTo(pix, tRect.left + kScrollArrowHBuffer, tRect.bottom - (kScrollArrowVBuffer +
            kSmallArrowPointHeight + kSmallArrowBaseHeight));
    MacLineTo(pix, tRect.left + kScrollArrowHBuffer, tRect.bottom - (kScrollArrowVBuffer +
            kSmallArrowPointHeight));
    MacLineTo(pix, tRect.left + kScrollArrowHBuffer + kScrollArrowWidth / 2, tRect.bottom -
            kScrollArrowVBuffer);
}

void GetPlayerListLineDownRect(const interfaceItemType& item, Rect *dRect) {
    short       thisHBorder = kInterfaceSmallHBorder;

    if ( item.style == kLarge) thisHBorder = kInterfaceLargeHBorder;
    *dRect = item.bounds;

    dRect->left = dRect->right + kInterfaceContentBuffer;
    dRect->right = dRect->left + thisHBorder + 1;
    dRect->top = dRect->bottom + kInterfaceContentBuffer - kInterfaceHTop -
            ( kScrollArrowVBuffer * 2 + kSmallArrowPointHeight + kSmallArrowBaseHeight);
    dRect->bottom = dRect->bottom + kInterfaceContentBuffer - kInterfaceHTop + 1;
}

void DrawPlayerListPageDown(const interfaceItemType& item, interfaceItemStatusEnum status, PixMap* pix) {
    Rect        tRect;
    short       thisHBorder = kInterfaceSmallHBorder;
    RgbColor color;

    if ( item.style == kLarge) thisHBorder = kInterfaceLargeHBorder;
    GetPlayerListPageDownRect( item, &tRect);

    if (status != kIH_Hilite)
    {
        if (status == kDimmed) SetTranslateColorShadeFore( item.color, DARKER);
        else SetTranslateColorShadeFore( item.color, LIGHT);
        FrameRect(pix, tRect);
        SetTranslateColorFore( BLACK);
        tRect.inset(1, 1);
        pix->view(tRect).fill(RgbColor::kBlack);
        if (status == kDimmed) SetTranslateColorShadeFore( item.color, DARKER);
        else SetTranslateColorShadeFore( item.color, LIGHT);
        tRect.inset(-1, -1);
    } else
    {
        SetTranslateColorShadeFore( item.color, VERY_LIGHT);
        GetRGBTranslateColorShade(&color, item.color, VERY_LIGHT);
        pix->view(tRect).fill(color);
        SetTranslateColorFore( BLACK);
    }

    tRect.bottom -= 1;

    MoveTo( tRect.left + kScrollArrowHBuffer + kScrollArrowWidth / 2, tRect.bottom -
            kScrollArrowVBuffer);
    MacLineTo(pix, tRect.left + kScrollArrowWidth + kScrollArrowHBuffer, tRect.bottom -
            (kScrollArrowVBuffer + kLargeArrowPointHeight));
    MacLineTo(pix, tRect.left + kScrollArrowWidth + kScrollArrowHBuffer, tRect.bottom -
            (kScrollArrowVBuffer + kLargeArrowPointHeight + kLargeArrowBaseHeight));
    MacLineTo(pix, tRect.left + kScrollArrowHBuffer, tRect.bottom - (kScrollArrowVBuffer +
            kLargeArrowPointHeight + kLargeArrowBaseHeight));
    MacLineTo(pix, tRect.left + kScrollArrowHBuffer, tRect.bottom - (kScrollArrowVBuffer +
            kLargeArrowPointHeight));
    MacLineTo(pix, tRect.left + kScrollArrowHBuffer + kScrollArrowWidth / 2, tRect.bottom -
            kScrollArrowVBuffer);
}

void GetPlayerListPageDownRect(const interfaceItemType& item, Rect *dRect) {
    short       thisHBorder = kInterfaceSmallHBorder;

    if ( item.style == kLarge) thisHBorder = kInterfaceLargeHBorder;
    GetPlayerListLineDownRect( item, dRect);
    dRect->bottom = dRect->top - kInterfaceVLipHeight;
    dRect->top = dRect->bottom - kScrollArrowVBuffer * 2 - kLargeArrowPointHeight -
            kLargeArrowBaseHeight;
}

void DrawInterfaceTextRect(const interfaceItemType& item, PixMap* pix) {
    Resource rsrc('TEXT', item.item.textRect.textID);
    DrawInterfaceTextInRect(item.bounds, rsrc.data().data(), rsrc.data().size(), item.style,
            item.color, pix, NULL);
}

void DrawInterfaceTextInRect(
        const Rect& tRect, const unsigned char *textData, long length, interfaceStyleType style,
        unsigned char textcolor, PixMap* pix, inlinePictType* inlinePict) {
    RgbColor color;
    GetRGBTranslateColorShade(&color, textcolor, VERY_LIGHT);
    StringPiece text(BytesPiece(textData, length), mac_roman_encoding());
    InterfaceText interface_text(text, style, color);
    interface_text.wrap_to(tRect.width(), kInterfaceTextHBuffer, kInterfaceTextVBuffer);

    if (inlinePict != NULL) {
        for (size_t i = 0; i < kMaxInlinePictNum; ++i) {
            if (i < interface_text.inline_picts().size()) {
                inlinePict[i] = interface_text.inline_picts()[i];
            } else {
                inlinePict[i].id = -1;
                inlinePict[i].bounds = Rect(0, 0, 0, 0);
            }
        }
    }

    interface_text.draw(pix, tRect);
}

short GetInterfaceTextHeightFromWidth(
        const unsigned char* textData, long length, interfaceStyleType style, short boundsWidth) {
    StringPiece text(BytesPiece(textData, length), mac_roman_encoding());
    InterfaceText interface_text(text, style, RgbColor::kBlack);
    interface_text.wrap_to(boundsWidth, kInterfaceTextHBuffer, kInterfaceTextVBuffer);
    return interface_text.height();
}

void DrawInterfacePictureRect(const interfaceItemType& item, PixMap* pix) {
    scoped_ptr<Picture> thePicture;
    RgnHandle       clipRgn = nil;
    Rect            tRect, uRect;

    if ( item.item.pictureRect.visibleBounds)
        DrawPlayerInterfacePlainRect(item.bounds, item.color, item.style, pix);

    clipRgn = NewRgn();
    GetClip( clipRgn);
    tRect = item.bounds;
    ClipRect(tRect);

    thePicture.reset(new Picture(item.item.pictureRect.pictureID));
    if (thePicture.get() != nil) {
        uRect = thePicture->bounds();
        uRect.offset(-uRect.left + tRect.left, -uRect.top + tRect.top);
        CopyBits(thePicture.get(), pix, thePicture->bounds(), uRect);
    }

    SetClip( clipRgn);
    DisposeRgn( clipRgn);
}

void DrawAnyInterfaceItem(const interfaceItemType& item, PixMap* pix) {
    switch (item.kind) {
        case kPlainRect:
            if (item.item.pictureRect.visibleBounds) {
                DrawPlayerInterfacePlainRect(item.bounds, item.color, item.style, pix);
            }
            break;

        case kTabBox:
            DrawPlayerInterfaceTabBox(
                    item.bounds, item.color, item.style, pix, item.item.tabBox.topRightBorderSize);
            break;

        case kLabeledRect:
            DrawPlayerInterfaceLabeledBox(item, pix);
            break;

        case kListRect:
            DrawPlayerInterfaceList(item, pix);
            break;

        case kTextRect:
             DrawInterfaceTextRect(item, pix);
            break;

        case kPlainButton:
            DrawPlayerInterfaceButton(item, pix);
            break;

        case kRadioButton:
            DrawPlayerInterfaceRadioButton(item, pix);
            break;

        case kTabBoxButton:
            DrawPlayerInterfaceTabBoxButton(item, pix);
            break;

        case kCheckboxButton:
            DrawPlayerInterfaceCheckBox(item, pix);
            break;

        case kPictureRect:
            DrawInterfacePictureRect(item, pix);
            break;

        default:
            WriteDebugLine( "\pHuh?");
            break;
    }
}

void GetAnyInterfaceItemGraphicBounds(const interfaceItemType& item, Rect *bounds) {
    short   thisHBorder = kInterfaceSmallHBorder;

    *bounds = item.bounds;

    if (item.style == kLarge) {
        thisHBorder = kInterfaceLargeHBorder;
    }

    bounds->left -= kInterfaceContentBuffer;
    bounds->top -= kInterfaceContentBuffer;
    bounds->right += kInterfaceContentBuffer + 1;
    bounds->bottom += kInterfaceContentBuffer + 1;

    switch (item.kind) {
        case kPlainRect:
            bounds->left -= thisHBorder;
            bounds->right += thisHBorder;
            bounds->top -= kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
            bounds->bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
            break;

        case kLabeledRect:
            bounds->left -= thisHBorder;
            bounds->right += thisHBorder;
            bounds->top -= GetInterfaceFontHeight(item.style) + kInterfaceTextVBuffer * 2 +
                            kLabelBottomHeight + kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
            bounds->bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
            break;

        case kTabBox:
            bounds->left -= thisHBorder;
            bounds->right += thisHBorder;
            bounds->top -= kInterfaceVEdgeHeight + kInterfaceVCornerHeight + 2;
            bounds->bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
            break;

        case kListRect:
            bounds->left -= thisHBorder;
            bounds->right += thisHBorder;
            bounds->top -= GetInterfaceFontAscent(item.style) + kInterfaceTextVBuffer * 2 +
                            kLabelBottomHeight + kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
            bounds->bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
            break;

        case kTextRect:
            bounds->left -= thisHBorder;
            bounds->right += thisHBorder;
            bounds->top -= kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
            bounds->bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
            break;

        case kPlainButton:
            bounds->left -= thisHBorder;
            bounds->right += thisHBorder;
            bounds->top -= kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
            bounds->bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
            break;

        case kRadioButton:
            bounds->left -= bounds->bottom - bounds->top + 2 * kInterfaceVEdgeHeight +
                            2 * kInterfaceVCornerHeight - 2 * kIndicatorVOffset + thisHBorder +
                            kRadioIndicatorHOffset;
            bounds->right += thisHBorder;
            bounds->top -= kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
            bounds->bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
            break;

        case kTabBoxButton:
            bounds->left -= thisHBorder + 5;
            bounds->right += thisHBorder + 5;
            bounds->top -= kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
            bounds->bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight + 2;
            break;

        case kCheckboxButton:
            bounds->left -= bounds->bottom - bounds->top + 2 * kInterfaceVEdgeHeight +
                            2 * kInterfaceVCornerHeight - 2 * kIndicatorVOffset + thisHBorder +
                            kCheckIndicatorHOffset;
            bounds->right += thisHBorder;
            bounds->top -= kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
            bounds->bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
            break;

        case kPictureRect:
            bounds->left -= thisHBorder;
            bounds->right += thisHBorder;
            bounds->top -= kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
            bounds->bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
            break;

        default:
            WriteDebugLine( "\pHuh?");
            break;
    }
}

void GetAnyInterfaceItemContentBounds(const interfaceItemType& item, Rect *bounds) {
    *bounds = item.bounds;
}

short GetInterfaceStringWidth(unsigned char* s, interfaceStyleType style) {
    long            width, height;

    SetInterfaceLargeUpperFont( style);
    mGetDirectStringDimensions(s, width, height);

    return ( width);
}

// GetInterfaceFontWidth:       -- NOT WORLD-READY! --
//
//  We're not using fontInfo.widMax because we know we're never going to use the ultra-wide
//  characters like &oelig; and the like, and we're not using a mono-spaced font.  Therefore, we're
//  using the width of 'R' which is about as wide as our normal letters get.
//

short GetInterfaceFontWidth(interfaceStyleType style) {
    SetInterfaceLargeUpperFont(style);
    return gDirectText->logicalWidth;
}

short GetInterfaceFontHeight(interfaceStyleType style) {
    SetInterfaceLargeUpperFont(style);
    return gDirectText->height;
}

short GetInterfaceFontAscent( interfaceStyleType style) {
    SetInterfaceLargeUpperFont(style);
    return gDirectText->ascent;
}

// DrawInterfaceString:
//  Relies on roman alphabet for upper/lower casing.  NOT WORLD-READY!

void DrawInterfaceString(
        unsigned char* s, interfaceStyleType style, PixMap* pix, const RgbColor& color) {
    Rect    clipRect;

    clipRect = pix->bounds();
    SetInterfaceLargeUpperFont( style);
    DrawDirectTextStringClipped(s, color, pix, clipRect, 0, 0);
}

void SetInterfaceLargeUpperFont(interfaceStyleType style) {
    if ( style == kSmall) {
        mSetDirectFont( kButtonSmallFontNum);
    } else {
        mSetDirectFont( kButtonFontNum);
    }
}

void SetInterfaceLargeLowerFont(interfaceStyleType style) {
    if ( style == kSmall) {
        mSetDirectFont( kButtonSmallFontNum);
    } else {
        mSetDirectFont( kButtonFontNum);
    }
}

}  // namespace antares
