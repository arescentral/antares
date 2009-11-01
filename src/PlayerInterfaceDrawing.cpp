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

#include <assert.h>
#include "Quickdraw.h"

#include "ColorTranslation.hpp"
#include "Debug.hpp"
#include "DirectText.hpp"
#include "Error.hpp"
#include "KeyMapTranslation.hpp"
#include "NateDraw.hpp"
#include "Picture.hpp"
#include "PlayerInterfaceItems.hpp"
#include "Resource.hpp"
#include "StringNumerics.hpp"

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

#define kReturnChar                 0x0d
#define kSpaceChar                  ' '
#define kWidestChar                 'R'
#define kInlineChar                 '^'
#define kInlinePictChar             'p'
#define kInlinePictClearLineChar    'P'

enum inlineKindType {
    kNoKind = 0,
    kVPictKind = 1,
    kVClearPictKind = 2
};

extern directTextType* gDirectText;
extern PixMap* gActiveWorld;

inline void mDrawPuffUpRect(Rect& mrect, uint8_t mcolor, int mshade) {
    SetTranslateColorShadeFore(mcolor, mshade);
    gActiveWorld->view(mrect).fill(GetTranslateColorShade(mcolor, mshade));
    SetTranslateColorShadeFore(mcolor, mshade + kLighterColor);
    MoveTo(mrect.left, mrect.bottom - 1);
    MacLineTo(mrect.left, mrect.top);
    MacLineTo(mrect.right - 1, mrect.top);
    SetTranslateColorShadeFore(mcolor, mshade + kDarkerColor);
    MacLineTo(mrect.right - 1, mrect.bottom - 1);
    MacLineTo(mrect.left, mrect.bottom - 1);
}

inline void mDrawPuffUpOval(Rect& mrect, uint8_t mcolor, int mshade) {
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

inline void mDrawPuffDownRect(Rect& mrect, uint8_t mcolor, int mshade) {
    SetTranslateColorFore(BLACK);
    gActiveWorld->view(mrect).fill(BLACK);
    SetTranslateColorShadeFore(mcolor, mshade + kDarkerColor);
    MoveTo(mrect.left - 1, mrect.bottom);
    MacLineTo(mrect.left - 1, mrect.top - 1);
    MacLineTo(mrect.right, mrect.top - 1);
    SetTranslateColorShadeFore(mcolor, mshade + kLighterColor);
    MacLineTo(mrect.right, mrect.bottom);
    MacLineTo(mrect.left - 1, mrect.bottom);
}

inline void mDrawPuffDownOval(Rect& mrect, uint8_t mcolor, int mshade) {
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

inline void mDrawPuffUpTopBorder(Rect& mrect, Rect& mtrect, uint8_t mcolor, int mshade, int mthisHBorder) {
    SetTranslateColorShadeFore(mcolor, mshade);
    mtrect = Rect(mrect.left - mthisHBorder,
        mrect.top - kInterfaceVEdgeHeight - kInterfaceVCornerHeight,
        mrect.left, mrect.top);
    gActiveWorld->view(mtrect).fill(GetTranslateColorShade(mcolor, mshade));
    mtrect = Rect(mrect.right,
        mrect.top - kInterfaceVEdgeHeight - kInterfaceVCornerHeight,
        mrect.right + mthisHBorder, mrect.top);
    gActiveWorld->view(mtrect).fill(GetTranslateColorShade(mcolor, mshade));
    mtrect = Rect(mrect.left,
        mrect.top - kInterfaceVEdgeHeight - kInterfaceVCornerHeight,
        mrect.right, mrect.top  - kInterfaceVLipHeight);
    gActiveWorld->view(mtrect).fill(GetTranslateColorShade(mcolor, mshade));
    SetTranslateColorShadeFore(mcolor, mshade + kDarkerColor);
    MoveTo(mrect.left - mthisHBorder, mrect.top);
    MacLineTo(mrect.left, mrect.top);
    MacLineTo(mrect.left, mrect.top - kInterfaceVLipHeight);
    MacLineTo(mrect.right, mrect.top - kInterfaceVLipHeight);
    MacLineTo(mrect.right, mrect.top);
    MacLineTo(mrect.right + mthisHBorder, mrect.top);
    MacLineTo(mrect.right + mthisHBorder, mrect.top - kInterfaceVEdgeHeight - kInterfaceVCornerHeight);
    SetTranslateColorShadeFore(mcolor, mshade + kLighterColor);
    MacLineTo(mrect.left - mthisHBorder, mrect.top - kInterfaceVEdgeHeight - kInterfaceVCornerHeight);
    MacLineTo(mrect.left - mthisHBorder, mrect.top);
}

inline void mDrawPuffUpBottomBorder(Rect& mrect, Rect& mtrect, uint8_t mcolor, int mshade, int mthisHBorder) {
    SetTranslateColorShadeFore(mcolor, mshade);
    mtrect = Rect(mrect.left - mthisHBorder,
        mrect.bottom,
        mrect.left, mrect.bottom + kInterfaceVEdgeHeight + kInterfaceVCornerHeight);
    gActiveWorld->view(mtrect).fill(GetTranslateColorShade(mcolor, mshade));
    mtrect = Rect(mrect.right,
        mrect.bottom,
        mrect.right + mthisHBorder, mrect.bottom + kInterfaceVEdgeHeight + kInterfaceVCornerHeight);
    gActiveWorld->view(mtrect).fill(GetTranslateColorShade(mcolor, mshade));
    mtrect = Rect(mrect.left,
        mrect.bottom + kInterfaceVLipHeight,
        mrect.right, mrect.bottom + kInterfaceVEdgeHeight + kInterfaceVCornerHeight);
    gActiveWorld->view(mtrect).fill(GetTranslateColorShade(mcolor, mshade));
    SetTranslateColorShadeFore(mcolor, mshade + kLighterColor);
    MoveTo(mrect.left - mthisHBorder, mrect.bottom + kInterfaceVEdgeHeight + kInterfaceVCornerHeight);
    MacLineTo(mrect.left - mthisHBorder, mrect.bottom);
    MacLineTo(mrect.left, mrect.bottom);
    MacLineTo(mrect.left, mrect.bottom + kInterfaceVLipHeight);
    MacLineTo(mrect.right, mrect.bottom + kInterfaceVLipHeight);
    MacLineTo(mrect.right, mrect.bottom);
    MacLineTo(mrect.right + mthisHBorder, mrect.bottom);
    SetTranslateColorShadeFore(mcolor, mshade + kDarkerColor);
    MacLineTo(mrect.right + mthisHBorder, mrect.bottom + kInterfaceVEdgeHeight + kInterfaceVCornerHeight);
    MacLineTo(mrect.left - mthisHBorder, mrect.bottom + kInterfaceVEdgeHeight + kInterfaceVCornerHeight);
}

inline void mDrawPuffUpTBorder(Rect& mrect, Rect& mtrect, uint8_t mcolor, int mshade, int msheight, int mthisHBorder) {
    SetTranslateColorShadeFore(mcolor, mshade);
    mtrect = Rect(mrect.left - mthisHBorder,
        mrect.top + msheight,
        mrect.left,
        mrect.top + msheight + kLabelBottomHeight);
    gActiveWorld->view(mtrect).fill(GetTranslateColorShade(mcolor, mshade));
    mtrect = Rect(mrect.right,
        mrect.top + msheight,
        mrect.right + mthisHBorder,
        mrect.top + msheight + kLabelBottomHeight);
    gActiveWorld->view(mtrect).fill(GetTranslateColorShade(mcolor, mshade));
    mtrect = Rect(mrect.left,
        mrect.top + msheight + kInterfaceVLipHeight,
        mrect.right,
        mrect.top + msheight + kLabelBottomHeight - kInterfaceVLipHeight);
    gActiveWorld->view(mtrect).fill(GetTranslateColorShade(mcolor, mshade));
    SetTranslateColorShadeFore(mcolor, mshade + kLighterColor);
    MoveTo(mrect.left - mthisHBorder, mrect.top + msheight + kLabelBottomHeight);
    MacLineTo(mrect.left - mthisHBorder, mrect.top + msheight);
    MacLineTo(mrect.left, mrect.top + msheight);
    MacLineTo(mrect.left, mrect.top + msheight + kInterfaceVLipHeight);
    MacLineTo(mrect.right, mrect.top + msheight + kInterfaceVLipHeight);
    MacLineTo(mrect.right, mrect.top + msheight);
    MacLineTo(mrect.right + mthisHBorder, mrect.top + msheight);
    SetTranslateColorShadeFore(mcolor, mshade + kDarkerColor);
    MacLineTo(mrect.right + mthisHBorder, mrect.top + msheight + kLabelBottomHeight);
    MacLineTo(mrect.right, mrect.top + msheight + kLabelBottomHeight);
    MacLineTo(mrect.right, mrect.top + msheight + kLabelBottomHeight - kInterfaceVLipHeight);
    MacLineTo(mrect.left, mrect.top + msheight + kLabelBottomHeight - kInterfaceVLipHeight);
    MacLineTo(mrect.left, mrect.top + msheight + kLabelBottomHeight);
    MacLineTo(mrect.left - mthisHBorder, mrect.top + msheight + kLabelBottomHeight);
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
    mDrawPuffUpTopBorder( tRect, uRect, color, DARK, thisHBorder);
    // bottom border

    mDrawPuffUpBottomBorder( tRect, uRect, color, DARK, thisHBorder);

    // main part left border

    vcenter = ( tRect.bottom - tRect.top) / 2;

    uRect = Rect(tRect.left - thisHBorder,
        tRect.top + kInterfaceHTop,
        tRect.left + 1,
        tRect.top + vcenter - kInterfaceVLipHeight + 1);
    mDrawPuffUpRect( uRect, color, DARKER);

    uRect = Rect(tRect.left - thisHBorder,
        tRect.bottom - vcenter + kInterfaceVLipHeight,
        tRect.left + 1,
        tRect.bottom - kInterfaceHTop + 1);
    mDrawPuffUpRect( uRect, color, VERY_DARK);

    // right border

    uRect = Rect(tRect.right,
        tRect.top + kInterfaceHTop,
        tRect.right + thisHBorder + 1,
        tRect.top + vcenter - kInterfaceVLipHeight + 1);
    mDrawPuffUpRect( uRect, color, DARKER);

    uRect = Rect(tRect.right,
        tRect.bottom - vcenter + kInterfaceVLipHeight,
        tRect.right + thisHBorder + 1,
        tRect.bottom - kInterfaceHTop + 1);
    mDrawPuffUpRect( uRect, color, VERY_DARK);

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
    gActiveWorld->view(uRect).fill(GetTranslateColorShade(color, shade));
    uRect = Rect((tRect).right,
        (tRect).top - 3 - kInterfaceVCornerHeight,
        (tRect).right + thisHBorder, (tRect).top);
    gActiveWorld->view(uRect).fill(GetTranslateColorShade(color, shade));
    uRect = Rect((tRect).left,
        (tRect).top - 3 - kInterfaceVCornerHeight,
        (tRect).left + 6, (tRect).top  - kInterfaceVLipHeight);
    gActiveWorld->view(uRect).fill(GetTranslateColorShade(color, shade));
    uRect = Rect((tRect).right - top_right_border_size,
        (tRect).top - 3 - kInterfaceVCornerHeight,
        (tRect).right, (tRect).top  - kInterfaceVLipHeight);
    gActiveWorld->view(uRect).fill(GetTranslateColorShade(color, shade));
    SetTranslateColorShadeFore( color, shade + kDarkerColor);
    MoveTo( (tRect).left - thisHBorder, (tRect).top);
    MacLineTo( (tRect).left, (tRect).top);
    MacLineTo( (tRect).left, (tRect).top - kInterfaceVLipHeight);
    MacLineTo( (tRect).left + 5, (tRect).top - kInterfaceVLipHeight);
    MoveTo( (tRect).right - top_right_border_size, (tRect).top - kInterfaceVLipHeight);
    MacLineTo( (tRect).right, (tRect).top - kInterfaceVLipHeight);
    MacLineTo( (tRect).right, (tRect).top);
    MacLineTo( (tRect).right + thisHBorder, (tRect).top);
    MacLineTo( (tRect).right + thisHBorder, (tRect).top - 3 - kInterfaceVCornerHeight);
    SetTranslateColorShadeFore( color, shade + kLighterColor);
    MacLineTo( (tRect).right - top_right_border_size, (tRect).top - 3 - kInterfaceVCornerHeight);
    MoveTo( (tRect).left + 5, (tRect).top - 3 - kInterfaceVCornerHeight);
    MacLineTo( (tRect).left - thisHBorder, (tRect).top - 3 - kInterfaceVCornerHeight);
    MacLineTo( (tRect).left - thisHBorder, (tRect).top);
    // bottom border

    mDrawPuffUpBottomBorder( tRect, uRect, color, DARK, thisHBorder);

    // main part left border

    vcenter = ( tRect.bottom - tRect.top) / 2;

    uRect = Rect(tRect.left - thisHBorder,
        tRect.top + kInterfaceHTop,
        tRect.left + 1,
        tRect.top + vcenter - kInterfaceVLipHeight + 1);
    mDrawPuffUpRect( uRect, color, DARKER);

    uRect = Rect(tRect.left - thisHBorder,
        tRect.bottom - vcenter + kInterfaceVLipHeight,
        tRect.left + 1,
        tRect.bottom - kInterfaceHTop + 1);
    mDrawPuffUpRect( uRect, color, VERY_DARK);

    // right border

    uRect = Rect(tRect.right,
        tRect.top + kInterfaceHTop,
        tRect.right + thisHBorder + 1,
        tRect.top + vcenter - kInterfaceVLipHeight + 1);
    mDrawPuffUpRect( uRect, color, DARKER);

    uRect = Rect(tRect.right,
        tRect.bottom - vcenter + kInterfaceVLipHeight,
        tRect.right + thisHBorder + 1,
        tRect.bottom - kInterfaceHTop + 1);
    mDrawPuffUpRect( uRect, color, VERY_DARK);

    SetTranslateColorFore( BLACK);
}

void DrawPlayerInterfaceButton(const interfaceItemType& item, PixMap* pix) {
    Rect            tRect, uRect, vRect;
    short           vcenter, swidth, sheight, thisHBorder = kInterfaceSmallHBorder;
    Str255          s;
    unsigned char   shade;
    unsigned char   color;
    transColorType  *transColor;

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

    mDrawPuffUpTopBorder( tRect, uRect, item.color, shade, thisHBorder);
    // bottom border

    mDrawPuffUpBottomBorder( tRect, uRect, item.color, shade, thisHBorder);

    // side border top

    vcenter = ( tRect.bottom - tRect.top) / 2;

    uRect = Rect(tRect.left - thisHBorder, tRect.top + kInterfaceHTop, tRect.left + 1,
            tRect.bottom - kInterfaceHTop + 1);
    vRect = Rect(tRect.right, tRect.top + kInterfaceHTop, tRect.right + thisHBorder + 1,
            tRect.bottom - kInterfaceHTop + 1);
    if (item.item.plainButton.status == kIH_Hilite) {
        shade = LIGHT;
        mDrawPuffUpRect( uRect, item.color, shade);
        mDrawPuffUpRect( vRect, item.color, shade);
    } else {
        if (item.item.plainButton.status == kDimmed) {
            shade = VERY_DARK;
        } else {
            shade = MEDIUM + kSlightlyLighterColor;
        }
        mDrawPuffUpRect( uRect, item.color, shade);
        mDrawPuffUpRect( vRect, item.color, shade);
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
        gActiveWorld->view(uRect).fill(GetTranslateColorShade(item.color, shade));

        if (item.item.plainButton.status == kIH_Hilite)
        {
            SetTranslateColorShadeFore(item.color, DARKEST);
            mGetTranslateColorShade(item.color, DARKEST, color, transColor);
        }
        else if (item.item.plainButton.status == kDimmed)
        {
            SetTranslateColorShadeFore(item.color, VERY_DARK);
            mGetTranslateColorShade(item.color, VERY_DARK, color, transColor);
        }

        else
        {
            SetTranslateColorShadeFore(item.color, LIGHTER);
            mGetTranslateColorShade(item.color, LIGHTER, color, transColor);
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
        mDrawPuffUpRect(uRect, item.color, shade);

        if (item.item.plainButton.status == kIH_Hilite)
            shade = LIGHT;
        else shade = DARK;//DARKEST;
        vRect = Rect(tRect.left + kInterfaceContentBuffer + swidth + kInterfaceTextHBuffer * 2 + 2,
        tRect.top + kInterfaceContentBuffer,
                        tRect.right - kInterfaceContentBuffer + 1,
                        tRect.bottom - kInterfaceContentBuffer + 1);
        SetTranslateColorShadeFore(item.color, shade);
        gActiveWorld->view(vRect).fill(GetTranslateColorShade(item.color, shade));

        swidth = GetInterfaceStringWidth(s, item.style);
        swidth = uRect.left + ( uRect.right - uRect.left) / 2 - swidth / 2;
        MoveTo( swidth, uRect.top + GetInterfaceFontAscent(item.style));
        if (item.item.plainButton.status == kDimmed)
        {
            SetTranslateColorShadeFore(item.color, VERY_DARK);
            mGetTranslateColorShade(item.color, VERY_DARK, color, transColor);
        }

        else
        {
            SetTranslateColorShadeFore(item.color, DARKEST);
            mGetTranslateColorShade(item.color, DARKEST, color, transColor);
        }

        DrawInterfaceString(s, item.style, pix, color);

        // draw the button title

        if (item.item.plainButton.status == kIH_Hilite)
        {
            SetTranslateColorShadeFore(item.color, DARKEST);
            mGetTranslateColorShade(item.color, DARKEST, color, transColor);
        } else if (item.item.plainButton.status == kDimmed)
        {
            SetTranslateColorShadeFore(item.color, DARKEST + kSlightlyLighterColor);
            mGetTranslateColorShade(item.color, DARKEST + kSlightlyLighterColor, color, transColor);
        }
        else
        {
            SetTranslateColorShadeFore(item.color, LIGHTER);
            mGetTranslateColorShade(item.color, LIGHTER, color, transColor);
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
    unsigned char   color;
    transColorType  *transColor;

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

    mDrawPuffUpTopBorder( tRect, uRect, item.color, shade, thisHBorder);
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
            mDrawPuffUpRect( uRect, item.color, shade);
            mDrawPuffUpRect( vRect, item.color, shade);
        } else
        {
            if ( item.item.radioButton.status == kDimmed)
                shade = VERY_DARK;
            else shade = DARK;
            mDrawPuffUpRect( uRect, item.color, shade);
            mDrawPuffUpRect( vRect, item.color, shade);
        }
        uRect = Rect(uRect.left, uRect.bottom, uRect.right, uRect.bottom + 3);
        vRect = Rect(vRect.left, vRect.bottom, vRect.right, vRect.bottom + 3);
        SetTranslateColorFore( BLACK);
        gActiveWorld->view(uRect).fill(BLACK);
        gActiveWorld->view(vRect).fill(BLACK);
        uRect = Rect(uRect.left - 3, uRect.bottom, vRect.right + 3, uRect.bottom + 3);
        shade = MEDIUM;
        SetTranslateColorShadeFore( item.color, shade);
        gActiveWorld->view(uRect).fill(GetTranslateColorShade(item.color, shade));
        SetTranslateColorShadeFore( item.color, shade + kLighterColor);
        MoveTo( uRect.left, uRect.top - 1);
        MacLineTo( uRect.right - 1, uRect.top - 1);
        SetTranslateColorShadeFore( item.color, shade + kDarkerColor);
        MoveTo( uRect.left, uRect.bottom);
        MacLineTo( uRect.right - 1, uRect.bottom);
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
        gActiveWorld->view(uRect).fill(GetTranslateColorShade(item.color, shade));
        gActiveWorld->view(vRect).fill(GetTranslateColorShade(item.color, shade));
        SetTranslateColorShadeFore( item.color, shade + kLighterColor);
        MoveTo( uRect.right - 2, uRect.top);
        MacLineTo( uRect.left, uRect.top);
        MacLineTo( uRect.left, uRect.bottom - 5);
        MacLineTo( uRect.left - 3, uRect.bottom - 5);
        MoveTo( vRect.right - 2, vRect.top);
        MacLineTo( vRect.left, vRect.top);
        MacLineTo( vRect.left, vRect.bottom - 2);
        MoveTo( vRect.right, vRect.bottom - 5);
        MacLineTo( vRect.right + 2, vRect.bottom - 5);
        SetTranslateColorShadeFore( item.color, shade + kDarkerColor);
        MoveTo( uRect.right - 1, uRect.top);
        MacLineTo( uRect.right - 1, uRect.bottom - 1);
        MacLineTo( uRect.left - 3, uRect.bottom - 1);
        MoveTo( vRect.right - 1, vRect.top);
        MacLineTo( vRect.right - 1, vRect.bottom - 4);
        MoveTo( vRect.left, vRect.bottom - 1);
        MacLineTo( vRect.right + 2, vRect.bottom - 1);
        uRect = Rect(uRect.left - 3, uRect.bottom - 4, uRect.right - 1, uRect.bottom - 1);
        SetTranslateColorShadeFore( item.color, shade);
        gActiveWorld->view(uRect).fill(GetTranslateColorShade(item.color, shade));
        vRect = Rect(vRect.left + 1, vRect.bottom - 4, vRect.right + 3, vRect.bottom - 1);
        gActiveWorld->view(vRect).fill(GetTranslateColorShade(item.color, shade));
        uRect.top--;
        uRect.bottom++;
        uRect.left = uRect.right + 1;
        uRect.right = vRect.left - 1;
        SetTranslateColorFore( BLACK);
        gActiveWorld->view(uRect).fill(BLACK);
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
        gActiveWorld->view(uRect).fill(GetTranslateColorShade(item.color, shade));

        if ( !item.item.radioButton.on)
        {
            if ( item.item.radioButton.status == kIH_Hilite)
            {
                SetTranslateColorShadeFore( item.color, DARKEST);
                mGetTranslateColorShade( item.color, DARKEST, color, transColor);
            }
            else if ( item.item.radioButton.status == kDimmed)
            {
                SetTranslateColorShadeFore( item.color, VERY_DARK);
                mGetTranslateColorShade( item.color, VERY_DARK, color, transColor);
            } else
            {
                SetTranslateColorShadeFore( item.color, LIGHT);
                mGetTranslateColorShade( item.color, LIGHT, color, transColor);
            }
        } else
        {
            SetTranslateColorShadeFore( item.color, VERY_LIGHT);
            mGetTranslateColorShade( item.color, VERY_LIGHT, color, transColor);
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
        mDrawPuffUpRect( uRect, item.color, shade);

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
        gActiveWorld->view(vRect).fill(GetTranslateColorShade(item.color, shade));

        swidth = GetInterfaceStringWidth( s, item.style);
        swidth = uRect.left + ( uRect.right - uRect.left) / 2 - swidth / 2;
        MoveTo( swidth, uRect.top + GetInterfaceFontAscent(item.style));
        if ( item.item.radioButton.status == kDimmed)
        {
            SetTranslateColorShadeFore( item.color, VERY_DARK);
            mGetTranslateColorShade( item.color, VERY_DARK, color, transColor);
        }

        else
        {
            SetTranslateColorShadeFore( item.color, DARKEST);
            mGetTranslateColorShade( item.color, DARKEST, color, transColor);
        }


        DrawInterfaceString(s, item.style, pix, color);


        // draw the button title

        if ( !item.item.radioButton.on)
        {
            if ( item.item.radioButton.status == kIH_Hilite)
            {
                SetTranslateColorShadeFore( item.color, DARKEST);
                mGetTranslateColorShade( item.color, DARKEST, color, transColor);
            }
            else if ( item.item.radioButton.status == kDimmed)
            {
                SetTranslateColorShadeFore( item.color, VERY_DARK);
                mGetTranslateColorShade( item.color, VERY_DARK, color, transColor);
            } else
            {
                SetTranslateColorShadeFore( item.color, LIGHT);
                mGetTranslateColorShade( item.color, LIGHT, color, transColor);
            }
        } else
        {
            SetTranslateColorShadeFore( item.color, VERY_LIGHT);
            mGetTranslateColorShade( item.color, VERY_LIGHT, color, transColor);
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
    unsigned char   color;
    transColorType  *transColor;

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

    mDrawPuffUpTopBorder( tRect, uRect, item.color, shade, thisHBorder);
    // bottom border

    mDrawPuffUpBottomBorder( tRect, uRect, item.color, shade, thisHBorder);

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
        mDrawPuffUpRect( uRect, item.color, shade);
        mDrawPuffUpRect( vRect, item.color, shade);

        wRect.left += 2;
        wRect.right += 2;
        SetTranslateColorFore( BLACK);
        FrameOval(wRect);
        wRect.left -= 2;
        wRect.right -= 2;
        mDrawPuffUpOval( wRect, item.color, shade);

        wRect.inset(3, 3);
        mDrawPuffDownOval( wRect, item.color, shade);
        wRect.inset(1, 1);
        if ( !item.item.radioButton.on) SetTranslateColorFore( BLACK);
        else SetTranslateColorShadeFore( item.color, VERY_LIGHT);
        PaintOval(wRect);
    } else
    {
        if ( item.item.radioButton.status == kDimmed)
            shade = VERY_DARK;
        else shade = MEDIUM + kSlightlyLighterColor;
        mDrawPuffUpRect( uRect, item.color, shade);
        mDrawPuffUpRect( vRect, item.color, shade);
        wRect.left += 2;
        wRect.right += 2;
        SetTranslateColorFore( BLACK);
        FrameOval(wRect);
        wRect.left -= 2;
        wRect.right -= 2;
        mDrawPuffUpOval( wRect, item.color, shade);

        wRect.inset(3, 3);
        mDrawPuffDownOval( wRect, item.color, shade);
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
    gActiveWorld->view(uRect).fill(GetTranslateColorShade(item.color, shade));

    if ( item.item.radioButton.status == kIH_Hilite)
    {
        SetTranslateColorShadeFore( item.color, DARKEST);
        mGetTranslateColorShade( item.color, DARKEST, color, transColor);
    }

    else if ( item.item.radioButton.status == kDimmed)
    {
        SetTranslateColorShadeFore( item.color, DARK);
        mGetTranslateColorShade( item.color, DARK, color, transColor);
    }
    else
    {
        SetTranslateColorShadeFore( item.color, LIGHT);
        mGetTranslateColorShade( item.color, LIGHT, color, transColor);
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
    unsigned char   color;
    transColorType  *transColor;

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

    mDrawPuffUpTopBorder( tRect, uRect, item.color, shade, thisHBorder);
    // bottom border

    mDrawPuffUpBottomBorder( tRect, uRect, item.color, shade, thisHBorder);

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
        mDrawPuffUpRect( uRect, item.color, shade);
        mDrawPuffUpRect( vRect, item.color, shade);
        mDrawPuffUpRect( wRect, item.color, shade);
        wRect.inset(3, 3);
        mDrawPuffDownRect( wRect, item.color, shade);
        wRect.inset(1, 1);
        uint8_t color;
        if ( !item.item.checkboxButton.on) {
            color = BLACK;
        } else {
            color = GetTranslateColorShade(item.color, VERY_LIGHT);
        }
        SetTranslateColorFore(color);
        gActiveWorld->view(wRect).fill(color);
    } else
    {
        if ( item.item.checkboxButton.status == kDimmed)
            shade = VERY_DARK;
        else shade = MEDIUM + kSlightlyLighterColor;
        mDrawPuffUpRect( uRect, item.color, shade);
        mDrawPuffUpRect( vRect, item.color, shade);
        mDrawPuffUpRect( wRect, item.color, shade);
        wRect.inset(3, 3);
        mDrawPuffDownRect( wRect, item.color, shade);
        wRect.inset(1, 1);
        uint8_t color;
        if (!item.item.checkboxButton.on) {
            color = BLACK;
        } else if (item.item.checkboxButton.status == kActive) {
            color = GetTranslateColorShade(item.color, LIGHT);
        } else {
            color = GetTranslateColorShade(item.color, MEDIUM);
        }
        SetTranslateColorFore(color);
        gActiveWorld->view(wRect).fill(color);
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
    gActiveWorld->view(uRect).fill(GetTranslateColorShade(item.color, shade));

    if ( item.item.checkboxButton.status == kIH_Hilite)
    {
        SetTranslateColorShadeFore( item.color, DARKEST);
        mGetTranslateColorShade( item.color, DARKEST, color, transColor);
    } else if ( item.item.checkboxButton.status == kDimmed)
    {
        SetTranslateColorShadeFore( item.color, DARK);
        mGetTranslateColorShade( item.color, DARK, color, transColor);
    } else
    {
        SetTranslateColorShadeFore( item.color, LIGHT);
        mGetTranslateColorShade( item.color, LIGHT, color, transColor);
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
    unsigned char   color;
    transColorType  *transColor;

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

    mDrawPuffUpTopBorder( tRect, uRect, item.color, shade, thisHBorder);
    // bottom border

    mDrawPuffUpBottomBorder( tRect, uRect, item.color, shade, thisHBorder);


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
    gActiveWorld->view(uRect).fill(GetTranslateColorShade(item.color, VERY_DARK));

    SetTranslateColorShadeFore( item.color, LIGHT);
    mGetTranslateColorShade( item.color, LIGHT, color, transColor);

    MoveTo( tRect.left + kInterfaceTextHBuffer, tRect.top + GetInterfaceFontAscent( item.style) +
            kInterfaceTextVBuffer);
    DrawInterfaceString(s, item.style, pix, color);

    // string left border

    shade = MEDIUM;
    vcenter = sheight / 2;

    uRect = Rect(tRect.left - thisHBorder,
            tRect.top + kInterfaceHTop,
            tRect.left + 1, tRect.top + sheight - kInterfaceHTop + 1);
    mDrawPuffUpRect( uRect, item.color, shade);

    // string right border

    shade = MEDIUM;
    uRect = Rect(tRect.right - swidth,
        tRect.top + kInterfaceHTop,
        tRect.right - 2,
        tRect.top + sheight - kInterfaceHTop + 1);
    mDrawPuffUpRect( uRect, item.color, shade);
    uRect = Rect(tRect.right,
        tRect.top + kInterfaceHTop,
        tRect.right + thisHBorder + 1,
        tRect.top + sheight - kInterfaceHTop + 1);
    mDrawPuffUpRect( uRect, item.color, shade);

    // string bottom border

    mDrawPuffUpTBorder( tRect, uRect, item.color, DARK, sheight, thisHBorder);

    // main part left border

    tRect.top += sheight + kLabelBottomHeight;

    vcenter = ( tRect.bottom - tRect.top) / 2;

    uRect = Rect(tRect.left - thisHBorder,
        tRect.top + kInterfaceHTop,
        tRect.left + 1,
        tRect.top + vcenter - kInterfaceVLipHeight + 1);
    mDrawPuffUpRect( uRect, item.color, DARKER);

    uRect = Rect(tRect.left - thisHBorder,
        tRect.bottom - vcenter + kInterfaceVLipHeight,
        tRect.left + 1,
        tRect.bottom - kInterfaceHTop + 1);
    mDrawPuffUpRect( uRect, item.color, VERY_DARK);

    // right border

    uRect = Rect(tRect.right,
        tRect.top + kInterfaceHTop,
        tRect.right + thisHBorder + 1,
        tRect.top + vcenter - kInterfaceVLipHeight + 1);
    mDrawPuffUpRect( uRect, item.color, DARKER);

    uRect = Rect(tRect.right,
        tRect.bottom - vcenter + kInterfaceVLipHeight,
        tRect.right + thisHBorder + 1,
        tRect.bottom - kInterfaceHTop + 1);
    mDrawPuffUpRect( uRect, item.color, VERY_DARK);

    SetTranslateColorFore( BLACK);
}

void DrawPlayerInterfaceList(const interfaceItemType& item, PixMap* pix) {
    Rect            tRect, uRect;
    short           vcenter, swidth, sheight, thisHBorder = kInterfaceSmallHBorder;
    Str255          s;
    RgnHandle       clipRgn = nil;
    unsigned char   color;
    transColorType  *transColor;

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
    MacLineTo( tRect.left - thisHBorder, tRect.top);
    MacLineTo( tRect.left - thisHBorder, tRect.top - kInterfaceVEdgeHeight);
    MacLineTo( tRect.left, tRect.top - kInterfaceVEdgeHeight - kInterfaceVCornerHeight);
    MacLineTo( tRect.right, tRect.top - kInterfaceVEdgeHeight - kInterfaceVCornerHeight);
    MacLineTo( tRect.right + thisHBorder, tRect.top + - kInterfaceVEdgeHeight);
    MacLineTo( tRect.right + thisHBorder, tRect.top);
    MacLineTo( tRect.right, tRect.top);
    MacLineTo( tRect.right, tRect.top - kInterfaceVLipHeight);
    MacLineTo( tRect.left, tRect.top - kInterfaceVLipHeight);
    MacLineTo( tRect.left, tRect.top);

    // bottom border

    MoveTo( tRect.left, tRect.bottom);
    MacLineTo( tRect.left - thisHBorder, tRect.bottom);
    MacLineTo( tRect.left - thisHBorder, tRect.bottom + kInterfaceVEdgeHeight);
    MacLineTo( tRect.left, tRect.bottom + kInterfaceVEdgeHeight + kInterfaceVCornerHeight);
    MacLineTo( tRect.right, tRect.bottom + kInterfaceVEdgeHeight + kInterfaceVCornerHeight);
    MacLineTo( tRect.right + thisHBorder, tRect.bottom + kInterfaceVEdgeHeight);
    MacLineTo( tRect.right + thisHBorder, tRect.bottom);
    MacLineTo( tRect.right, tRect.bottom);
    MacLineTo( tRect.right, tRect.bottom + kInterfaceVLipHeight);
    MacLineTo( tRect.left, tRect.bottom + kInterfaceVLipHeight);
    MacLineTo( tRect.left, tRect.bottom);

    // draw the string

    SetTranslateColorShadeFore( item.color, LIGHT);
    mGetTranslateColorShade( item.color, LIGHT, color, transColor);

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
    MacLineTo( tRect.left - thisHBorder, tRect.top + kInterfaceHTop);
    MacLineTo( tRect.left - thisHBorder, tRect.top + vcenter - kInterfaceVLipHeight);
    MacLineTo( tRect.left, tRect.top + vcenter - kInterfaceVLipHeight);
    MacLineTo( tRect.left, tRect.top + kInterfaceHTop);

    SetTranslateColorShadeFore( item.color, DARK);
    MoveTo( tRect.left, tRect.top + sheight - vcenter + kInterfaceVLipHeight);
    MacLineTo( tRect.left - thisHBorder, tRect.top + sheight- vcenter + kInterfaceVLipHeight);
    MacLineTo( tRect.left - thisHBorder, tRect.top + sheight - kInterfaceHTop);
    MacLineTo( tRect.left, tRect.top + sheight - kInterfaceHTop);
    MacLineTo( tRect.left, tRect.top + sheight - vcenter + kInterfaceVLipHeight);

    // string right border

    SetTranslateColorShadeFore( item.color, MEDIUM);
    MoveTo( tRect.right - swidth, tRect.top + kInterfaceHTop);
    MacLineTo( tRect.right + thisHBorder, tRect.top + kInterfaceHTop);
    MacLineTo( tRect.right + thisHBorder, tRect.top + vcenter - kInterfaceVLipHeight);
    MacLineTo( tRect.right - swidth, tRect.top + vcenter - kInterfaceVLipHeight);
    MacLineTo( tRect.right - swidth, tRect.top + kInterfaceHTop);

    SetTranslateColorShadeFore( item.color, DARK);
    MoveTo( tRect.right - swidth, tRect.top + sheight - vcenter + kInterfaceVLipHeight);
    MacLineTo( tRect.right + thisHBorder, tRect.top + sheight - vcenter + kInterfaceVLipHeight);
    MacLineTo( tRect.right + thisHBorder, tRect.top + sheight - kInterfaceHTop);
    MacLineTo( tRect.right - swidth, tRect.top + sheight - kInterfaceHTop);
    MacLineTo( tRect.right - swidth, tRect.top + sheight - vcenter + kInterfaceVLipHeight);

    // string bottom border

    SetTranslateColorShadeFore( item.color, DARK);
    MoveTo( tRect.left, tRect.top + sheight);
    MacLineTo( tRect.left, tRect.top + sheight + kInterfaceVLipHeight);
    MacLineTo( tRect.right, tRect.top + sheight + kInterfaceVLipHeight);
    MacLineTo( tRect.right, tRect.top + sheight);
    MacLineTo( tRect.right + thisHBorder, tRect.top + sheight);
    MacLineTo( tRect.right + thisHBorder, tRect.top + sheight + kLabelBottomHeight);
    MacLineTo( tRect.right, tRect.top + sheight + kLabelBottomHeight);
    MacLineTo( tRect.right, tRect.top + sheight + kLabelBottomHeight - kInterfaceVLipHeight);
    MacLineTo( tRect.left, tRect.top + sheight + kLabelBottomHeight - kInterfaceVLipHeight);
    MacLineTo( tRect.left, tRect.top + sheight + kLabelBottomHeight);
    MacLineTo( tRect.left - thisHBorder, tRect.top + sheight + kLabelBottomHeight);
    MacLineTo( tRect.left - thisHBorder, tRect.top + sheight);
    MacLineTo( tRect.left, tRect.top + sheight);

    // main part left border

    tRect.top += sheight + kLabelBottomHeight;

    vcenter = ( tRect.bottom - tRect.top) / 2;

    SetTranslateColorShadeFore( item.color, DARKER);
    MoveTo( tRect.left, tRect.top + kInterfaceHTop);
    MacLineTo( tRect.left - thisHBorder, tRect.top + kInterfaceHTop);
    MacLineTo( tRect.left - thisHBorder, tRect.top + vcenter - kInterfaceVLipHeight);
    MacLineTo( tRect.left, tRect.top + vcenter - kInterfaceVLipHeight);
    MacLineTo( tRect.left, tRect.top + kInterfaceHTop);

    SetTranslateColorShadeFore( item.color, VERY_DARK);
    MoveTo( tRect.left, tRect.bottom - vcenter + kInterfaceVLipHeight);
    MacLineTo( tRect.left - thisHBorder, tRect.bottom - vcenter + kInterfaceVLipHeight);
    MacLineTo( tRect.left - thisHBorder, tRect.bottom - kInterfaceHTop);
    MacLineTo( tRect.left, tRect.bottom - kInterfaceHTop);
    MacLineTo( tRect.left, tRect.bottom - vcenter + kInterfaceVLipHeight);

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
                gActiveWorld->view(uRect).fill(GetTranslateColorShade(item.color, LIGHT));
                SetTranslateColorShadeFore( item.color, VERY_DARK);
                mGetTranslateColorShade( item.color, VERY_DARK, color, transColor);
            } else
            {
                DefaultColors();
                gActiveWorld->view(uRect).fill(BLACK);
                SetTranslateColorShadeFore( item.color, LIGHT);
                mGetTranslateColorShade( item.color, LIGHT, color, transColor);
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

    DrawPlayerListLineUp(item, lineUpStatus);
    DrawPlayerListPageUp(item, pageUpStatus);
    DrawPlayerListLineDown(item, lineDownStatus);
    DrawPlayerListPageDown(item, pageDownStatus);
    SetTranslateColorFore( BLACK);
}

void DrawPlayerInterfaceListEntry(
        const interfaceItemType& item, short whichEntry, PixMap* pix) {
    Str255          s;
    RgnHandle       clipRgn = nil;
    Rect            tRect, uRect;
    short           swidth, vcenter;
    unsigned char   color;
    transColorType  *transColor;

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
                gActiveWorld->view(uRect).fill(GetTranslateColorShade(item.color, LIGHT));
                SetTranslateColorShadeFore( item.color, VERY_DARK);
                mGetTranslateColorShade( item.color, VERY_DARK, color, transColor);
            } else
            {
                gActiveWorld->view(uRect).fill(GetTranslateColorShade(item.color, VERY_LIGHT));
                SetTranslateColorShadeFore( item.color, LIGHT);
                mGetTranslateColorShade( item.color, LIGHT, color, transColor);
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

void DrawPlayerListLineUp(const interfaceItemType& item, interfaceItemStatusEnum status) {
    Rect        tRect;
    short       thisHBorder = kInterfaceSmallHBorder;

    if ( item.style == kLarge) thisHBorder = kInterfaceLargeHBorder;
    GetPlayerListLineUpRect(item, &tRect);

    if (status != kIH_Hilite)
    {
        if (status == kDimmed) SetTranslateColorShadeFore( item.color, DARKER);
        else SetTranslateColorShadeFore( item.color, LIGHT);
        MacFrameRect(tRect);
        SetTranslateColorFore( BLACK);
        tRect.inset(1, 1);
        gActiveWorld->view(tRect).fill(BLACK);
        if (status == kDimmed) SetTranslateColorShadeFore( item.color, DARKER);
        else SetTranslateColorShadeFore( item.color, LIGHT);
        tRect.inset(-1, -1);
    } else
    {
        SetTranslateColorShadeFore( item.color, VERY_LIGHT);
        gActiveWorld->view(tRect).fill(GetTranslateColorShade(item.color, VERY_LIGHT));
        SetTranslateColorFore( BLACK);
    }

    MoveTo( tRect.left + kScrollArrowHBuffer + kScrollArrowWidth / 2, tRect.top +
            kScrollArrowVBuffer);
    MacLineTo( tRect.left + kScrollArrowWidth + kScrollArrowHBuffer, tRect.top +
            kScrollArrowVBuffer + kSmallArrowPointHeight);
    MacLineTo( tRect.left + kScrollArrowWidth + kScrollArrowHBuffer, tRect.top + kScrollArrowVBuffer +
            kSmallArrowPointHeight + kSmallArrowBaseHeight);
    MacLineTo( tRect.left + kScrollArrowHBuffer, tRect.top + kScrollArrowVBuffer +
            kSmallArrowPointHeight + kSmallArrowBaseHeight);
    MacLineTo( tRect.left + kScrollArrowHBuffer, tRect.top + kScrollArrowVBuffer +
            kSmallArrowPointHeight);
    MacLineTo( tRect.left + kScrollArrowHBuffer + kScrollArrowWidth / 2, tRect.top +
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

void DrawPlayerListPageUp(const interfaceItemType& item, interfaceItemStatusEnum status) {
    Rect        tRect;
    short       thisHBorder = kInterfaceSmallHBorder;

    if ( item.style == kLarge) thisHBorder = kInterfaceLargeHBorder;
    GetPlayerListPageUpRect(item, &tRect);

    if (status != kIH_Hilite)
    {
        if (status == kDimmed) SetTranslateColorShadeFore( item.color, DARKER);
        else SetTranslateColorShadeFore( item.color, LIGHT);
        MacFrameRect(tRect);
        SetTranslateColorFore( BLACK);
        tRect.inset(1, 1);
        gActiveWorld->view(tRect).fill(BLACK);
        if (status == kDimmed) SetTranslateColorShadeFore( item.color, DARKER);
        else SetTranslateColorShadeFore( item.color, LIGHT);
        tRect.inset(-1, -1);
    } else
    {
        SetTranslateColorShadeFore( item.color, VERY_LIGHT);
        gActiveWorld->view(tRect).fill(GetTranslateColorShade(item.color, VERY_LIGHT));
        SetTranslateColorFore( BLACK);
    }

    MoveTo( tRect.left + kScrollArrowHBuffer + kScrollArrowWidth / 2, tRect.top +
            kScrollArrowVBuffer);
    MacLineTo( tRect.left + kScrollArrowWidth + kScrollArrowHBuffer, tRect.top +
            kScrollArrowVBuffer + kLargeArrowPointHeight);
    MacLineTo( tRect.left + kScrollArrowWidth + kScrollArrowHBuffer, tRect.top + kScrollArrowVBuffer +
            kLargeArrowPointHeight + kLargeArrowBaseHeight);
    MacLineTo( tRect.left + kScrollArrowHBuffer, tRect.top + kScrollArrowVBuffer +
            kLargeArrowPointHeight + kLargeArrowBaseHeight);
    MacLineTo( tRect.left + kScrollArrowHBuffer, tRect.top + kScrollArrowVBuffer +
            kLargeArrowPointHeight);
    MacLineTo( tRect.left + kScrollArrowHBuffer + kScrollArrowWidth / 2, tRect.top +
            kScrollArrowVBuffer);
}

void GetPlayerListPageUpRect(const interfaceItemType& item, Rect *dRect) {
    GetPlayerListLineUpRect(item, dRect);
    dRect->top = dRect->bottom + kInterfaceVLipHeight;
    dRect->bottom = dRect->top + kScrollArrowVBuffer * 2 + kLargeArrowPointHeight +
            kLargeArrowBaseHeight + 1;
}

void DrawPlayerListLineDown(const interfaceItemType& item, interfaceItemStatusEnum status) {
    Rect        tRect;
    short       thisHBorder = kInterfaceSmallHBorder;

    if ( item.style == kLarge) thisHBorder = kInterfaceLargeHBorder;
    GetPlayerListLineDownRect( item, &tRect);

    if (status != kIH_Hilite)
    {
        if (status == kDimmed) SetTranslateColorShadeFore( item.color, DARKER);
        else SetTranslateColorShadeFore( item.color, LIGHT);
        MacFrameRect(tRect);
        SetTranslateColorFore( BLACK);
        tRect.inset(1, 1);
        gActiveWorld->view(tRect).fill(BLACK);
        if (status == kDimmed) SetTranslateColorShadeFore( item.color, DARKER);
        else SetTranslateColorShadeFore( item.color, LIGHT);
        tRect.inset(-1, -1);
    } else
    {
        SetTranslateColorShadeFore( item.color, VERY_LIGHT);
        gActiveWorld->view(tRect).fill(GetTranslateColorShade(item.color, VERY_LIGHT));
        SetTranslateColorFore( BLACK);
    }

    tRect.bottom -= 1;

    MoveTo( tRect.left + kScrollArrowHBuffer + kScrollArrowWidth / 2, tRect.bottom -
            kScrollArrowVBuffer);
    MacLineTo( tRect.left + kScrollArrowWidth + kScrollArrowHBuffer, tRect.bottom -
            (kScrollArrowVBuffer + kSmallArrowPointHeight));
    MacLineTo( tRect.left + kScrollArrowWidth + kScrollArrowHBuffer, tRect.bottom -
            (kScrollArrowVBuffer + kSmallArrowPointHeight + kSmallArrowBaseHeight));
    MacLineTo( tRect.left + kScrollArrowHBuffer, tRect.bottom - (kScrollArrowVBuffer +
            kSmallArrowPointHeight + kSmallArrowBaseHeight));
    MacLineTo( tRect.left + kScrollArrowHBuffer, tRect.bottom - (kScrollArrowVBuffer +
            kSmallArrowPointHeight));
    MacLineTo( tRect.left + kScrollArrowHBuffer + kScrollArrowWidth / 2, tRect.bottom -
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

void DrawPlayerListPageDown(const interfaceItemType& item, interfaceItemStatusEnum status) {
    Rect        tRect;
    short       thisHBorder = kInterfaceSmallHBorder;

    if ( item.style == kLarge) thisHBorder = kInterfaceLargeHBorder;
    GetPlayerListPageDownRect( item, &tRect);

    if (status != kIH_Hilite)
    {
        if (status == kDimmed) SetTranslateColorShadeFore( item.color, DARKER);
        else SetTranslateColorShadeFore( item.color, LIGHT);
        MacFrameRect(tRect);
        SetTranslateColorFore( BLACK);
        tRect.inset(1, 1);
        gActiveWorld->view(tRect).fill(BLACK);
        if (status == kDimmed) SetTranslateColorShadeFore( item.color, DARKER);
        else SetTranslateColorShadeFore( item.color, LIGHT);
        tRect.inset(-1, -1);
    } else
    {
        SetTranslateColorShadeFore( item.color, VERY_LIGHT);
        gActiveWorld->view(tRect).fill(GetTranslateColorShade(item.color, VERY_LIGHT));
        SetTranslateColorFore( BLACK);
    }

    tRect.bottom -= 1;

    MoveTo( tRect.left + kScrollArrowHBuffer + kScrollArrowWidth / 2, tRect.bottom -
            kScrollArrowVBuffer);
    MacLineTo( tRect.left + kScrollArrowWidth + kScrollArrowHBuffer, tRect.bottom -
            (kScrollArrowVBuffer + kLargeArrowPointHeight));
    MacLineTo( tRect.left + kScrollArrowWidth + kScrollArrowHBuffer, tRect.bottom -
            (kScrollArrowVBuffer + kLargeArrowPointHeight + kLargeArrowBaseHeight));
    MacLineTo( tRect.left + kScrollArrowHBuffer, tRect.bottom - (kScrollArrowVBuffer +
            kLargeArrowPointHeight + kLargeArrowBaseHeight));
    MacLineTo( tRect.left + kScrollArrowHBuffer, tRect.bottom - (kScrollArrowVBuffer +
            kLargeArrowPointHeight));
    MacLineTo( tRect.left + kScrollArrowHBuffer + kScrollArrowWidth / 2, tRect.bottom -
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
    scoped_ptr<std::string> textData;
    long            length;
    RgnHandle       clipRgn = nil;
    Rect            tRect;
    unsigned char   *dChar, *wordlen, *theLine, thisLen;
    short           vline = 0, hleft = 0, fheight = 0, xpos = 0;
    unsigned char   color, *charwidthptr, charwidth;
    transColorType  *transColor;

    if ( item.item.textRect.visibleBounds)
        DrawPlayerInterfacePlainRect(item.bounds, item.color, item.style, pix);
    clipRgn = NewRgn();
    GetClip( clipRgn);
    tRect = item.bounds;
    ClipRect(tRect);

    hleft = tRect.left + kInterfaceTextHBuffer;
    fheight = GetInterfaceFontHeight( item.style) + kInterfaceTextVBuffer;
    vline = tRect.top - ( fheight - GetInterfaceFontAscent( item.style));

    theLine = new unsigned char[kMaxLineLength];
    if ( theLine != nil)
    {
        wordlen = theLine;

        textData.reset(new std::string(Resource::get_data('TEXT', item.item.textRect.textID)));
        if (textData.get() != nil) {
            length = textData->size();
            const char* sChar = textData->c_str();

            SetTranslateColorShadeFore( item.color, VERY_LIGHT);

            while (( length > 0) && ( vline < tRect.bottom))
            {
                vline += fheight;
                xpos = hleft;
                *wordlen = 0;
                thisLen = 0;
                dChar = wordlen + 1;
                const char* aheadChar = sChar;
                while (( *aheadChar == kSpaceChar) && ( length > 0)) { aheadChar++; length--;}

                while (( xpos < tRect.right - kInterfaceTextHBuffer) && ( length > 0))
                {
                    *wordlen += thisLen;
                    sChar = aheadChar;
                    length -= thisLen;
                    thisLen = 0;
                    do
                    {
                        if ((( *aheadChar >= 'A') && ( *aheadChar <= 'Z')) || (( *aheadChar >= '0') &&
                                ( *aheadChar <= '9')))
                            SetInterfaceLargeUpperFont( item.style);
                        else SetInterfaceLargeLowerFont( item.style);
                        if (( *aheadChar >= 'a') && ( *aheadChar <= 'z'))
                        {
                            mDirectCharWidth( charwidth, *aheadChar/* - 'a' + 'A'*/, charwidthptr);
                            xpos += charwidth;
                        }
                        else
                        {
                            mDirectCharWidth( charwidth, *aheadChar, charwidthptr);
                            xpos += charwidth;
                        }
                        if ( *aheadChar == kReturnChar) *dChar++ = *aheadChar;
                        else *dChar++ = *aheadChar;
                        aheadChar++;
                        thisLen++;
                    } while (( *aheadChar != kSpaceChar) && ( length - thisLen > 0) &&
                            ( *aheadChar != kReturnChar) && ( *aheadChar != kInlineChar) &&
                            !(( *wordlen == 0) && (xpos >= tRect.right - kInterfaceTextHBuffer)));

                    if ( *aheadChar == kReturnChar)
                    {
                        if ( xpos < tRect.right - kInterfaceTextHBuffer)
                        {
                            *wordlen += thisLen;
                            length -= thisLen;
                            sChar = aheadChar;
                        }
                        xpos = tRect.right;     // to force us out of loop
                    } else if (( *wordlen == 0) && (xpos >= tRect.right - kInterfaceTextHBuffer))
                    {
                        thisLen--;
                        *wordlen = thisLen;
                        length -= thisLen;
                        sChar = aheadChar - 1;
                    }
                }
                MoveTo( hleft, vline);

                SetTranslateColorShadeFore( item.color, VERY_LIGHT);
                mGetTranslateColorShade( item.color, VERY_LIGHT, color, transColor);

                DrawInterfaceString( theLine, item.style, pix, color);
            }
            textData.reset();
        }
        delete[] theLine;
    }
    SetClip( clipRgn);
    DisposeRgn( clipRgn);
}

void DrawInterfaceTextInRect(
        const Rect& tRect, const unsigned char *textData, long length, interfaceStyleType style,
        unsigned char textcolor, PixMap* pix, inlinePictType* inlinePict) {
    RgnHandle       clipRgn = nil;
    const unsigned char* sChar;
    const unsigned char* aheadChar;
    unsigned char   *dChar, *wordlen, *theLine, thisLen;
    short           vline = 0, hleft = 0, fheight = 0, xpos = 0, inlinePictNum = 0, i;
    bool         processInline = false;
    Str255          inlineString;
    inlineKindType  inlineKind = kNoKind;
    long            inlineValue = 0;
    scoped_ptr<Picture> thePicture;
    Rect            uRect;
    unsigned char   color, *charwidthptr, charwidth;
    transColorType  *transColor;
    inlinePictType  *thisInlinePict = NULL;

    clipRgn = NewRgn();
    if ( clipRgn == nil) return;
    GetClip( clipRgn);
    ClipRect(tRect);

    hleft = tRect.left + kInterfaceTextHBuffer;
    fheight = GetInterfaceFontHeight( style) + kInterfaceTextVBuffer;
    vline = tRect.top - ( fheight - GetInterfaceFontAscent( style));

    if ( inlinePict != nil)
    {
        thisInlinePict = inlinePict;

        for ( i = 0; i < kMaxInlinePictNum; i++)
        {
            thisInlinePict->id = -1;
            thisInlinePict->bounds.left = thisInlinePict->bounds.right = -1;
            thisInlinePict++;
        }
        thisInlinePict = inlinePict;
    }

    theLine = new unsigned char[kMaxLineLength];
    if ( theLine != nil)
    {
        wordlen = theLine;

        if ( textData != nil)
        {
            sChar = textData;

            SetTranslateColorShadeFore( textcolor, VERY_LIGHT);

            while (( length > 0) && ( vline < tRect.bottom))
            {
                vline += fheight;
                xpos = hleft;
                *wordlen = 0;
                thisLen = 0;
                dChar = wordlen + 1;
                aheadChar = sChar;
                while (( *aheadChar == kSpaceChar) && ( length > 0)) { aheadChar++; length--;}

                processInline= false;
                inlineValue = 0;
                inlineString[0] = 0;
                inlineKind = kNoKind;

                while (( xpos < tRect.right - kInterfaceTextHBuffer) && ( length > 0))
                {
                    *wordlen += thisLen;
                    sChar = aheadChar;
                    length -= thisLen;
                    thisLen = 0;
                    do
                    {
                        while ( *aheadChar == kInlineChar)
                        {
                            inlineString[0] = 0;
                            inlineKind = kNoKind;
                            aheadChar++;
                            length--;

                            if (( length - thisLen) > 0)
                            {
                                switch( *aheadChar)
                                {
                                    case kInlinePictChar:
                                        aheadChar++;
                                        length--;
                                        while ((( length - thisLen) > 0) && ( *aheadChar != kInlineChar))
                                        {
                                            inlineString[0] += 1;
                                            inlineString[inlineString[0]] = *aheadChar;
                                            aheadChar++;
                                            length--;
                                        }
                                        StringToNum( inlineString, &inlineValue);
                                        inlineKind = kVPictKind;
                                        aheadChar++;
                                        length--;
                                        break;

                                    case kInlinePictClearLineChar:
                                        aheadChar++;
                                        length--;
                                        while ((( length - thisLen) > 0) && ( *aheadChar != kInlineChar))
                                        {
                                            inlineString[0] += 1;
                                            inlineString[inlineString[0]] = *aheadChar;
                                            aheadChar++;
                                            length--;
                                        }
                                        StringToNum( inlineString, &inlineValue);
                                        inlineKind = kVClearPictKind;
                                        aheadChar++;
                                        length--;
                                        xpos = tRect.right;
                                        break;
                                }
                            }
                        }
                        if ((( *aheadChar >= 'A') && ( *aheadChar <= 'Z')) || (( *aheadChar >= '0') &&
                                ( *aheadChar <= '9')))
                            SetInterfaceLargeUpperFont( style);
                        else SetInterfaceLargeLowerFont( style);
                        if (( *aheadChar >= 'a') && ( *aheadChar <= 'z'))
                        {
                            mDirectCharWidth( charwidth, *aheadChar/* - 'a' + 'A'*/, charwidthptr);
                            xpos += charwidth;
                        }
                        else
                        {
                            mDirectCharWidth( charwidth, *aheadChar, charwidthptr);
                            xpos += charwidth;
                        }
                        if ( *aheadChar == kReturnChar) *dChar++ = *aheadChar;
                        else *dChar++ = *aheadChar;
                        thisLen++;
                        aheadChar++;

                    } while (( *aheadChar != kSpaceChar) && ( length - thisLen > 0) &&
                            ( *aheadChar != kReturnChar) && ( inlineKind == kNoKind) &&
                            !(( *wordlen == 0) && (xpos >= tRect.right - kInterfaceTextHBuffer)));

                    if ( *aheadChar == kReturnChar)
                    {
                        if ( xpos < tRect.right - kInterfaceTextHBuffer)
                        {
                            *wordlen += thisLen;
                            length -= thisLen;
                            sChar = aheadChar;
                        }
                        xpos = tRect.right;        // to force us out of loop
                    } else if (( *wordlen == 0) && (xpos >= tRect.right - kInterfaceTextHBuffer))
                    {
                        thisLen--;
                        *wordlen = thisLen;
                        length -= thisLen;
                        sChar = aheadChar - 1;
                    } else
                    {
                        switch( inlineKind)
                        {
                            case kVClearPictKind:
                                thisLen--;
                                *wordlen += thisLen;
                                length -= thisLen;
                                sChar = aheadChar - 1;
                                break;

                            case kNoKind:
                            case kVPictKind:
                                break;
                        }
                    }

                }
                MoveTo( hleft, vline);
                SetTranslateColorShadeFore( textcolor, VERY_LIGHT);
                mGetTranslateColorShade( textcolor, VERY_LIGHT, color, transColor);
                DrawInterfaceString( theLine, style, pix, color);
                switch( inlineKind)
                {
                    case kVPictKind:
                    case kVClearPictKind:
                        vline += ( fheight - GetInterfaceFontAscent( style));
                        thePicture.reset(new Picture(inlineValue));
                        xpos = hleft;
                        if ( *theLine == 0) vline -= fheight;
                        if (thePicture.get() != nil) {
                            uRect = thePicture->bounds();
                            uRect.offset(-uRect.left + xpos - kInterfaceTextHBuffer +
                                        ( tRect.right - tRect.left) / 2 - ( uRect.right -
                                        uRect.left) / 2,
                                        -uRect.top + vline);
                            CopyBits(thePicture.get(), gActiveWorld, thePicture->bounds(), uRect);
                            thePicture.reset();
                            vline += uRect.bottom - uRect.top;
                            xpos = hleft;
                            if (( inlinePict != nil) && ( inlinePictNum < kMaxInlinePictNum))
                            {
                                thisInlinePict->bounds = uRect;
                                thisInlinePict->id = inlineValue;
                                thisInlinePict++;
                                inlinePictNum++;
                            }
                        }
                        inlineKind = kNoKind;
                        break;

                    case kNoKind:
                        break;
                }
            }
        }
        delete[] theLine;
    }
    SetClip( clipRgn);
    DisposeRgn( clipRgn);
}

short GetInterfaceTextHeightFromWidth(
        const unsigned char* textData, long length, interfaceStyleType style, short boundsWidth) {
    const unsigned char* sChar;
    const unsigned char* aheadChar;
    unsigned char   *dChar, *wordlen, *theLine, thisLen;
    short           vline = 0, hleft = 0, fheight = 0, xpos = 0;
    Str255          inlineString;
    inlineKindType  inlineKind = kNoKind;
    long            inlineValue = 0;
    scoped_ptr<Picture> thePicture;
    Rect            uRect;
    unsigned char   *charwidthptr, charwidth;

    hleft = kInterfaceTextHBuffer;
    fheight = GetInterfaceFontHeight( style) + kInterfaceTextVBuffer;
    vline = 0 - ( fheight - GetInterfaceFontAscent( style));

    theLine = new unsigned char[kMaxLineLength];
    if ( theLine != nil)
    {
        wordlen = theLine;

        if ( textData != nil)
        {
            sChar = textData;

            while ( length > 0)
            {
                vline += fheight;
                xpos = hleft;
                *wordlen = 0;
                thisLen = 0;
                dChar = wordlen + 1;
                aheadChar = sChar;
                while (( *aheadChar == kSpaceChar) && ( length > 0)) { aheadChar++; length--;}

                inlineValue = 0;
                inlineString[0] = 0;
                inlineKind = kNoKind;

                while (( xpos < boundsWidth - kInterfaceTextHBuffer) && ( length > 0))
                {
                    *wordlen += thisLen;
                    sChar = aheadChar;
                    length -= thisLen;
                    thisLen = 0;
                    do
                    {
                        while ( *aheadChar == kInlineChar)
                        {
                            inlineString[0] = 0;
                            inlineKind = kNoKind;
                            aheadChar++;
                            length--;

                            if (( length - thisLen) > 0)
                            {
                                switch( *aheadChar)
                                {
                                    case kInlinePictChar:
                                        aheadChar++;
                                        length--;
                                        while ((( length - thisLen) > 0) && ( *aheadChar != kInlineChar))
                                        {
                                            inlineString[0] += 1;
                                            inlineString[inlineString[0]] = *aheadChar;
                                            aheadChar++;
                                            length--;
                                        }
                                        StringToNum( inlineString, &inlineValue);
                                        inlineKind = kVPictKind;
                                        aheadChar++;
                                        length--;
                                        break;

                                    case kInlinePictClearLineChar:
                                        aheadChar++;
                                        length--;
                                        while ((( length - thisLen) > 0) && ( *aheadChar != kInlineChar))
                                        {
                                            inlineString[0] += 1;
                                            inlineString[inlineString[0]] = *aheadChar;
                                            aheadChar++;
                                            length--;
                                        }
                                        StringToNum( inlineString, &inlineValue);
                                        inlineKind = kVClearPictKind;
                                        aheadChar++;
                                        length--;
                                        xpos = boundsWidth;
                                        break;
                                }
                            }
                        }
                        if ((( *aheadChar >= 'A') && ( *aheadChar <= 'Z')) || (( *aheadChar >= '0') &&
                                ( *aheadChar <= '9')))
                            SetInterfaceLargeUpperFont( style);
                        else SetInterfaceLargeLowerFont( style);
                        if (( *aheadChar >= 'a') && ( *aheadChar <= 'z'))
                        {
                            mDirectCharWidth( charwidth, *aheadChar/* - 'a' + 'A'*/, charwidthptr);
                            xpos += charwidth;
                        }
                        else
                        {
                            mDirectCharWidth( charwidth, *aheadChar, charwidthptr);
                            xpos += charwidth;
                        }
                        if ( *aheadChar == kReturnChar) *dChar++ = *aheadChar;
                        else *dChar++ = *aheadChar;
                        aheadChar++;
                        thisLen++;
                    } while (( *aheadChar != kSpaceChar) && ( length - thisLen > 0) &&
                            ( *aheadChar != kReturnChar) && ( inlineKind == kNoKind) &&
                            !(( *wordlen == 0) && (xpos >= boundsWidth - kInterfaceTextHBuffer)));

                    if ( *aheadChar == kReturnChar)
                    {
                        if ( xpos < boundsWidth - kInterfaceTextHBuffer)
                        {
                            *wordlen += thisLen;
                            length -= thisLen;
                            sChar = aheadChar;
                        }
                        xpos = boundsWidth;     // to force us out of loop
                    } else if (( *wordlen == 0) && (xpos >= boundsWidth - kInterfaceTextHBuffer))
                    {
                        thisLen--;
                        *wordlen = thisLen;
                        length -= thisLen;
                        sChar = aheadChar - 1;
                    } else
                    {
                        switch( inlineKind)
                        {
                            case kVClearPictKind:
                                thisLen--;
                                *wordlen += thisLen;
                                length -= thisLen;
                                sChar = aheadChar - 1;
                                break;

                            case kNoKind:
                            case kVPictKind:
                                break;
                        }
                    }

                }

                switch( inlineKind)
                {
                    case kVPictKind:
                    case kVClearPictKind:
                        thePicture.reset(new Picture(inlineValue));
                        xpos = hleft;
                        vline += ( fheight - GetInterfaceFontAscent( style));
                        if ( *theLine == 0) vline -= fheight;
                        if (thePicture.get() != nil) {
                            uRect = thePicture->bounds();
                            uRect.offset(-uRect.left + xpos, -uRect.top + vline);
                            thePicture.reset();
                            vline += uRect.bottom - uRect.top;
                            xpos = hleft;
                        }
                        inlineKind = kNoKind;
                        break;

                    case kNoKind:
                        break;
                }
            }
        }
        delete[] theLine;
    }
    return( vline + ( fheight - GetInterfaceFontAscent( style)) );
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
        CopyBits(thePicture.get(), gActiveWorld, thePicture->bounds(), uRect);
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
        unsigned char* s, interfaceStyleType style, PixMap* pix, unsigned char color) {
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
