/*
Ares, a tactical space combat game.
Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

// Ares Controls.c
#ifndef __CONDITIONALMACROS__
#include "ConditionalMacros.h"
#endif // __CONDITIONALMACROS__

#if TARGET_OS_WIN32

	#ifndef __QUICKTIMEVR__
	#include <QuickTimeVR.h>
	#endif

	#ifndef __QTUtilities__
	#include "QTUtilities.h"
	#endif

	#ifndef __QTVRUtilities__
	#include "QTVRUtilities.h"
	#endif

	#include <TextUtils.h>
	#include <Script.h>
	#include <string.h>
#endif // TARGET_OS_WIN32

#include "Resources.h"

#include "Debug.h"
#include "Error.h"

#include "Player Interface Items.h"
#include "Player Interface Drawing.h"
#include "Color Translation.h"
#include "Key Map Translation.h"
#include "Handle Handling.h"
#include "Direct Text.h"

#ifndef kNateDraw
#include "NateDraw.h"
#endif

#ifndef kAnyChar
#include "AnyChar.h"
#endif

#define	kInterfaceLargeHBorder		13
#define	kInterfaceSmallHBorder		3
#define	kInterfaceVEdgeHeight		1//2
#define	kInterfaceVCornerHeight		2//3
#define	kInterfaceVLipHeight		1
#define	kInterfaceHTop				2
#define	kLabelBottomHeight			6
#define	kInterfaceContentBuffer		2

#define	kIndicatorVOffset			4
#define	kIndicatorInnerOffset		4
#define	kRadioIndicatorHOffset		4
#define	kCheckIndicatorHOffset		4

#define	kScrollArrowWidth			8
#define	kScrollArrowHBuffer			2
#define	kScrollArrowVBuffer			3

#define	kSmallArrowPointHeight		5
#define	kSmallArrowBaseHeight		5
#define	kLargeArrowPointHeight		9
#define	kLargeArrowBaseHeight		9

#define	kMaxLineLength				255

#define	kMaxKeyNameLength			4		// how many chars can be in name of key for plainButton

#define	kReturnChar					0x0d
#define	kSpaceChar					' '
#define	kWidestChar					'R'
#define	kInlineChar					'^'
#define	kInlinePictChar				'p'
#define	kInlinePictClearLineChar	'P'

#define mDrawPuffUpRect( mrect, mcolor, mshade) \
	SetTranslateColorShadeFore( mcolor, mshade);\
	PaintRect( &(mrect));\
	SetTranslateColorShadeFore( mcolor, (mshade) + kLighterColor);\
	MoveTo( (mrect).left, (mrect).bottom - 1);\
	MacLineTo( (mrect).left, (mrect).top);\
	MacLineTo( (mrect).right - 1, (mrect).top);\
	SetTranslateColorShadeFore( mcolor, (mshade) + kDarkerColor);\
	MacLineTo( (mrect).right - 1, (mrect).bottom - 1);\
	MacLineTo( (mrect).left, (mrect).bottom - 1);\

#define mDrawPuffUpOval( mrect, mcolor, mshade) \
	SetTranslateColorShadeFore( mcolor, (mshade) + kDarkerColor);\
	(mrect).left++;\
	(mrect).right++;\
	FrameOval( &(mrect));\
	(mrect).left--;\
	(mrect).right--;\
	(mrect).top++;\
	(mrect).bottom++;\
	FrameOval( (&mrect));\
	(mrect).top--;\
	(mrect).bottom--;\
	SetTranslateColorShadeFore( mcolor, (mshade) + kLighterColor);\
	(mrect).left--;\
	(mrect).right--;\
	FrameOval( (&mrect));\
	(mrect).left++;\
	(mrect).right++;\
	(mrect).top--;\
	(mrect).bottom--;\
	FrameOval( (&mrect));\
	(mrect).top++;\
	(mrect).bottom++;\
	SetTranslateColorShadeFore( mcolor, mshade);\
	PaintOval( &(mrect));\

#define mDrawPuffDownRect( mrect, mcolor, mshade) \
	SetTranslateColorFore( BLACK);\
	PaintRect( &(mrect));\
	SetTranslateColorShadeFore( mcolor, (mshade) + kDarkerColor);\
	MoveTo( (mrect).left - 1, (mrect).bottom);\
	MacLineTo( (mrect).left - 1, (mrect).top - 1);\
	MacLineTo( (mrect).right, (mrect).top - 1);\
	SetTranslateColorShadeFore( mcolor, (mshade) + kLighterColor);\
	MacLineTo( (mrect).right, (mrect).bottom);\
	MacLineTo( (mrect).left - 1, (mrect).bottom);\

#define mDrawPuffDownOval( mrect, mcolor, mshade) \
	SetTranslateColorShadeFore( mcolor, (mshade) + kLighterColor);\
	(mrect).left++;\
	(mrect).right++;\
	FrameOval( &(mrect));\
	(mrect).left--;\
	(mrect).right--;\
	(mrect).top++;\
	(mrect).bottom++;\
	FrameOval( (&mrect));\
	(mrect).top--;\
	(mrect).bottom--;\
	SetTranslateColorShadeFore( mcolor, (mshade) + kDarkerColor);\
	(mrect).left--;\
	(mrect).right--;\
	FrameOval( (&mrect));\
	(mrect).left++;\
	(mrect).right++;\
	(mrect).top--;\
	(mrect).bottom--;\
	FrameOval( (&mrect));\
	(mrect).top++;\
	(mrect).bottom++;\
	SetTranslateColorFore( BLACK);\
	PaintOval( &(mrect));\
	
#define	mDrawPuffUpTopBorder( mrect, mtrect, mcolor, mshade, mthisHBorder)\
	SetTranslateColorShadeFore( mcolor, mshade);\
	MacSetRect( &(mtrect), (mrect).left - mthisHBorder, \
		(mrect).top - kInterfaceVEdgeHeight - kInterfaceVCornerHeight,\
		(mrect).left, (mrect).top);\
	PaintRect( &(mtrect));\
	MacSetRect( &(mtrect), (mrect).right, \
		(mrect).top - kInterfaceVEdgeHeight - kInterfaceVCornerHeight,\
		(mrect).right + mthisHBorder, (mrect).top);\
	PaintRect( &(mtrect));\
	MacSetRect( &(mtrect), (mrect).left, \
		(mrect).top - kInterfaceVEdgeHeight - kInterfaceVCornerHeight,\
		(mrect).right, (mrect).top  - kInterfaceVLipHeight);\
	PaintRect( &(mtrect));\
	SetTranslateColorShadeFore( mcolor, mshade + kDarkerColor);\
	MoveTo( (mrect).left - mthisHBorder, (mrect).top);\
	MacLineTo( (mrect).left, (mrect).top);\
	MacLineTo( (mrect).left, (mrect).top - kInterfaceVLipHeight);\
	MacLineTo( (mrect).right, (mrect).top - kInterfaceVLipHeight);\
	MacLineTo( (mrect).right, (mrect).top);\
	MacLineTo( (mrect).right + mthisHBorder, (mrect).top);\
	MacLineTo( (mrect).right + mthisHBorder, (mrect).top - kInterfaceVEdgeHeight - kInterfaceVCornerHeight);\
	SetTranslateColorShadeFore( mcolor, mshade + kLighterColor);\
	MacLineTo( (mrect).left - mthisHBorder, (mrect).top - kInterfaceVEdgeHeight - kInterfaceVCornerHeight);\
	MacLineTo( (mrect).left - mthisHBorder, (mrect).top);\
		
#define	mDrawPuffUpBottomBorder( mrect, mtrect, mcolor, mshade, mthisHBorder)\
	SetTranslateColorShadeFore( mcolor, mshade);\
	MacSetRect( &(mtrect), (mrect).left - mthisHBorder, \
		(mrect).bottom,\
		(mrect).left, (mrect).bottom + kInterfaceVEdgeHeight + kInterfaceVCornerHeight);\
	PaintRect( &(mtrect));\
	MacSetRect( &(mtrect), (mrect).right, \
		(mrect).bottom,\
		(mrect).right + mthisHBorder, (mrect).bottom + kInterfaceVEdgeHeight + kInterfaceVCornerHeight);\
	PaintRect( &(mtrect));\
	MacSetRect( &(mtrect), (mrect).left, \
		(mrect).bottom + kInterfaceVLipHeight,\
		(mrect).right, (mrect).bottom + kInterfaceVEdgeHeight + kInterfaceVCornerHeight);\
	PaintRect( &(mtrect));\
	SetTranslateColorShadeFore( mcolor, mshade + kLighterColor);\
	MoveTo( (mrect).left - mthisHBorder, (mrect).bottom + kInterfaceVEdgeHeight + kInterfaceVCornerHeight);\
	MacLineTo( (mrect).left - mthisHBorder, (mrect).bottom);\
	MacLineTo( (mrect).left, (mrect).bottom);\
	MacLineTo( (mrect).left, (mrect).bottom + kInterfaceVLipHeight);\
	MacLineTo( (mrect).right, (mrect).bottom + kInterfaceVLipHeight);\
	MacLineTo( (mrect).right, (mrect).bottom);\
	MacLineTo( (mrect).right + mthisHBorder, (mrect).bottom);\
	SetTranslateColorShadeFore( mcolor, mshade + kDarkerColor);\
	MacLineTo( (mrect).right + mthisHBorder, (mrect).bottom + kInterfaceVEdgeHeight + kInterfaceVCornerHeight);\
	MacLineTo( (mrect).left - mthisHBorder, (mrect).bottom + kInterfaceVEdgeHeight + kInterfaceVCornerHeight);\
	
#define	mDrawPuffUpTBorder( mrect, mtrect, mcolor, mshade, msheight, mthisHBorder)\
	SetTranslateColorShadeFore( mcolor, mshade);\
	MacSetRect( &(mtrect), (mrect).left - mthisHBorder,\
		(mrect).top + msheight,\
		(mrect).left,\
		(mrect).top + msheight + kLabelBottomHeight);\
	PaintRect( &(mtrect));\
	MacSetRect( &(mtrect), (mrect).right,\
		(mrect).top + msheight,\
		(mrect).right + mthisHBorder,\
		(mrect).top + msheight + kLabelBottomHeight);\
	PaintRect( &(mtrect));\
	MacSetRect( &(mtrect), (mrect).left,\
		(mrect).top + msheight + kInterfaceVLipHeight,\
		(mrect).right,\
		(mrect).top + msheight + kLabelBottomHeight - kInterfaceVLipHeight);\
	PaintRect( &(mtrect));\
	SetTranslateColorShadeFore( mcolor, mshade + kLighterColor);\
	MoveTo( (mrect).left - mthisHBorder, (mrect).top + msheight + kLabelBottomHeight);\
	MacLineTo( (mrect).left - mthisHBorder, (mrect).top + msheight);\
	MacLineTo( (mrect).left, (mrect).top + msheight);\
	MacLineTo( (mrect).left, (mrect).top + msheight + kInterfaceVLipHeight);\
	MacLineTo( (mrect).right, (mrect).top + msheight + kInterfaceVLipHeight);\
	MacLineTo( (mrect).right, (mrect).top + msheight);\
	MacLineTo( (mrect).right + mthisHBorder, (mrect).top + msheight);\
	SetTranslateColorShadeFore( mcolor, (mshade) + kDarkerColor);\
	MacLineTo( (mrect).right + mthisHBorder, (mrect).top + msheight + kLabelBottomHeight);\
	MacLineTo( (mrect).right, (mrect).top + msheight + kLabelBottomHeight);\
	MacLineTo( (mrect).right, (mrect).top + msheight + kLabelBottomHeight - kInterfaceVLipHeight);\
	MacLineTo( (mrect).left, (mrect).top + msheight + kLabelBottomHeight - kInterfaceVLipHeight);\
	MacLineTo( (mrect).left, (mrect).top + msheight + kLabelBottomHeight);\
	MacLineTo( (mrect).left - mthisHBorder, (mrect).top + msheight + kLabelBottomHeight);\

typedef enum
{
	kNoKind = 0,
	kVPictKind = 1,
	kVClearPictKind = 2

} inlineKindType;

extern CWindowPtr		gTheWindow;
extern Handle			gDirectTextData, gColorTranslateTable;
extern long				gWhichDirectText;
extern directTextType	*gDirectText;

void DrawPlayerInterfacePlainRect( longRect	*dRect, unsigned char color,
				interfaceStyleType style, PixMap *destMap, long portLeft,
						long portTop)

{
	Rect			tRect, uRect;
	short			vcenter, thisHBorder = kInterfaceSmallHBorder;

#pragma unused ( destMap, portLeft, portTop)
	if ( style == kLarge) thisHBorder = kInterfaceLargeHBorder;
	LongRectToRect( dRect, &tRect);
	tRect.left -= kInterfaceContentBuffer;
	tRect.top -= kInterfaceContentBuffer;
	tRect.right += kInterfaceContentBuffer;
	tRect.bottom += kInterfaceContentBuffer;
	
	// top border
	mDrawPuffUpTopBorder( tRect, uRect, color, DARK, thisHBorder)
	// bottom border

	mDrawPuffUpBottomBorder( tRect, uRect, color, DARK, thisHBorder)
	
	// main part left border
		
	vcenter = ( tRect.bottom - tRect.top) / 2;
	
	MacSetRect( &uRect, tRect.left - thisHBorder,
		tRect.top + kInterfaceHTop,
		tRect.left + 1,
		tRect.top + vcenter - kInterfaceVLipHeight + 1);
	mDrawPuffUpRect( uRect, color, DARKER)	

	MacSetRect( &uRect, tRect.left - thisHBorder,
		tRect.bottom - vcenter + kInterfaceVLipHeight,
		tRect.left + 1,
		tRect.bottom - kInterfaceHTop + 1);
	mDrawPuffUpRect( uRect, color, VERY_DARK)	

	// right border
	
	MacSetRect( &uRect, tRect.right,
		tRect.top + kInterfaceHTop,
		tRect.right + thisHBorder + 1,
		tRect.top + vcenter - kInterfaceVLipHeight + 1);
	mDrawPuffUpRect( uRect, color, DARKER)	

	MacSetRect( &uRect, tRect.right,
		tRect.bottom - vcenter + kInterfaceVLipHeight,
		tRect.right + thisHBorder + 1,
		tRect.bottom - kInterfaceHTop + 1);
	mDrawPuffUpRect( uRect, color, VERY_DARK)	

	SetTranslateColorFore( BLACK);
}

void DrawPlayerInterfaceTabBox( longRect	*dRect, unsigned char color,
				interfaceStyleType style, PixMap *destMap, long portLeft,
						long portTop, short topRightBorderSize)

{
	Rect			tRect, uRect;
	short			vcenter, thisHBorder = kInterfaceSmallHBorder;
	unsigned char	shade;
	
#pragma unused ( destMap, portLeft, portTop)
	if ( style == kLarge) thisHBorder = kInterfaceLargeHBorder;
	LongRectToRect( dRect, &tRect);
	tRect.left -= kInterfaceContentBuffer;
	tRect.top -= kInterfaceContentBuffer;
	tRect.right += kInterfaceContentBuffer;
	tRect.bottom += kInterfaceContentBuffer;
	
	// top border
	shade = MEDIUM;
	SetTranslateColorShadeFore( color, shade);
	MacSetRect( &(uRect), (tRect).left - thisHBorder, 
		(tRect).top - 3 - kInterfaceVCornerHeight,
		(tRect).left, (tRect).top);
	PaintRect( &(uRect));
	MacSetRect( &(uRect), (tRect).right, 
		(tRect).top - 3 - kInterfaceVCornerHeight,
		(tRect).right + thisHBorder, (tRect).top);
	PaintRect( &(uRect));
	MacSetRect( &(uRect), (tRect).left, 
		(tRect).top - 3 - kInterfaceVCornerHeight,
		(tRect).left + 6, (tRect).top  - kInterfaceVLipHeight);
	PaintRect( &(uRect));
	MacSetRect( &(uRect), (tRect).right - topRightBorderSize, 
		(tRect).top - 3 - kInterfaceVCornerHeight,
		(tRect).right, (tRect).top  - kInterfaceVLipHeight);
	PaintRect( &(uRect));
	SetTranslateColorShadeFore( color, shade + kDarkerColor);
	MoveTo( (tRect).left - thisHBorder, (tRect).top);
	MacLineTo( (tRect).left, (tRect).top);
	MacLineTo( (tRect).left, (tRect).top - kInterfaceVLipHeight);
	MacLineTo( (tRect).left + 5, (tRect).top - kInterfaceVLipHeight);
	MoveTo( (tRect).right - topRightBorderSize, (tRect).top - kInterfaceVLipHeight);
	MacLineTo( (tRect).right, (tRect).top - kInterfaceVLipHeight);
	MacLineTo( (tRect).right, (tRect).top);
	MacLineTo( (tRect).right + thisHBorder, (tRect).top);
	MacLineTo( (tRect).right + thisHBorder, (tRect).top - 3 - kInterfaceVCornerHeight);
	SetTranslateColorShadeFore( color, shade + kLighterColor);
	MacLineTo( (tRect).right - topRightBorderSize, (tRect).top - 3 - kInterfaceVCornerHeight);
	MoveTo( (tRect).left + 5, (tRect).top - 3 - kInterfaceVCornerHeight);
	MacLineTo( (tRect).left - thisHBorder, (tRect).top - 3 - kInterfaceVCornerHeight);
	MacLineTo( (tRect).left - thisHBorder, (tRect).top);
	// bottom border

	mDrawPuffUpBottomBorder( tRect, uRect, color, DARK, thisHBorder)
	
	// main part left border
		
	vcenter = ( tRect.bottom - tRect.top) / 2;
	
	MacSetRect( &uRect, tRect.left - thisHBorder,
		tRect.top + kInterfaceHTop,
		tRect.left + 1,
		tRect.top + vcenter - kInterfaceVLipHeight + 1);
	mDrawPuffUpRect( uRect, color, DARKER)	

	MacSetRect( &uRect, tRect.left - thisHBorder,
		tRect.bottom - vcenter + kInterfaceVLipHeight,
		tRect.left + 1,
		tRect.bottom - kInterfaceHTop + 1);
	mDrawPuffUpRect( uRect, color, VERY_DARK)	

	// right border
	
	MacSetRect( &uRect, tRect.right,
		tRect.top + kInterfaceHTop,
		tRect.right + thisHBorder + 1,
		tRect.top + vcenter - kInterfaceVLipHeight + 1);
	mDrawPuffUpRect( uRect, color, DARKER)	

	MacSetRect( &uRect, tRect.right,
		tRect.bottom - vcenter + kInterfaceVLipHeight,
		tRect.right + thisHBorder + 1,
		tRect.bottom - kInterfaceHTop + 1);
	mDrawPuffUpRect( uRect, color, VERY_DARK)	

	SetTranslateColorFore( BLACK);
}

void DrawPlayerInterfaceButton( interfaceItemType *dItem, PixMap *destMap, long portLeft,
						long portTop)

{
	Rect			tRect, uRect, vRect;
	short			vcenter, swidth, sheight, thisHBorder = kInterfaceSmallHBorder;
	Str255			s;
	unsigned char	shade;
	unsigned char	color;
	transColorType	*transColor;
	
	if ( dItem->style == kLarge) thisHBorder = kInterfaceLargeHBorder;
	LongRectToRect( &(dItem->bounds), &tRect);
	
	uRect = tRect;
	uRect.right++;
	uRect.bottom++;
		
	tRect.left -= kInterfaceContentBuffer;
	tRect.top -= kInterfaceContentBuffer;
	tRect.right += kInterfaceContentBuffer;
	tRect.bottom += kInterfaceContentBuffer;
	
	// top border
	
	if ( dItem->item.plainButton.status == kDimmed)
		shade = VERY_DARK;
	else shade = MEDIUM;
	
	mDrawPuffUpTopBorder( tRect, uRect, dItem->color, shade, thisHBorder)
	// bottom border

	mDrawPuffUpBottomBorder( tRect, uRect, dItem->color, shade, thisHBorder)
	
	// side border top
	
	vcenter = ( tRect.bottom - tRect.top) / 2;

	MacSetRect( &uRect, tRect.left - thisHBorder, tRect.top + kInterfaceHTop, tRect.left + 1,
			tRect.bottom - kInterfaceHTop + 1);
	MacSetRect( &vRect, tRect.right, tRect.top + kInterfaceHTop, tRect.right + thisHBorder + 1,
			tRect.bottom - kInterfaceHTop + 1);
	if ( dItem->item.plainButton.status == kIH_Hilite)
	{
		shade = LIGHT;
		mDrawPuffUpRect( uRect, dItem->color, shade)
		mDrawPuffUpRect( vRect, dItem->color, shade)
	} else
	{
		if ( dItem->item.plainButton.status == kDimmed)
			shade = VERY_DARK;
		else shade = MEDIUM + kSlightlyLighterColor;
		mDrawPuffUpRect( uRect, dItem->color, shade)
		mDrawPuffUpRect( vRect, dItem->color, shade)
	}
	
	
	if ( dItem->item.plainButton.key == 0)
	{
		MacSetRect( &uRect, tRect.left +  kInterfaceContentBuffer,
			tRect.top + kInterfaceContentBuffer,
			tRect.left +  kInterfaceContentBuffer,
			tRect.bottom - kInterfaceContentBuffer);

		if ( dItem->item.plainButton.status == kIH_Hilite)
			shade = LIGHT;
		else shade = DARK;//DARKEST + kSlightlyLighterColor;
		MacSetRect( &uRect, tRect.left +  kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
						tRect.right - kInterfaceContentBuffer + 1,
						tRect.bottom - kInterfaceContentBuffer + 1);
		SetTranslateColorShadeFore( dItem->color, shade);
		PaintRect( &uRect);
		
		if ( dItem->item.plainButton.status == kIH_Hilite)
		{
			SetTranslateColorShadeFore( dItem->color, DARKEST);
			mGetTranslateColorShade( dItem->color, DARKEST, color, transColor)
		}
		else if ( dItem->item.plainButton.status == kDimmed)
		{
			SetTranslateColorShadeFore( dItem->color, VERY_DARK);
			mGetTranslateColorShade( dItem->color, VERY_DARK, color, transColor)
		}

		else
		{
			SetTranslateColorShadeFore( dItem->color, LIGHTER);
			mGetTranslateColorShade( dItem->color, LIGHTER, color, transColor)
		}
		GetIndString( s, dItem->item.plainButton.label.stringID,
					dItem->item.plainButton.label.stringNumber);
		swidth = GetInterfaceStringWidth( s, dItem->style);
		swidth = tRect.left + ( tRect.right - tRect.left) / 2 - swidth / 2;
		sheight = GetInterfaceFontAscent(dItem->style) + kInterfaceTextVBuffer + tRect.top;
		MoveTo( swidth, sheight);
		DrawInterfaceString( (anyCharType *)s, dItem->style, destMap, portLeft,
						portTop,  color);
	} else
	{
		// draw the key code
		
		
		if ( dItem->item.plainButton.status == kDimmed)
			shade = VERY_DARK;
		else shade = LIGHT;
		GetKeyNumName( s, dItem->item.plainButton.key);
		swidth = GetInterfaceFontWidth( dItem->style) * kMaxKeyNameLength;
		
		MacSetRect( &uRect, tRect.left +  kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
				tRect.left + kInterfaceContentBuffer + swidth + kInterfaceTextHBuffer * 2 + 1,
				tRect.bottom - kInterfaceContentBuffer + 1);
		mDrawPuffUpRect( uRect, dItem->color, shade)

		if ( dItem->item.plainButton.status == kIH_Hilite)
			shade = LIGHT;
		else shade = DARK;//DARKEST;
		MacSetRect( &vRect, tRect.left + kInterfaceContentBuffer + swidth + kInterfaceTextHBuffer * 2 + 2,
		tRect.top + kInterfaceContentBuffer,
						tRect.right - kInterfaceContentBuffer + 1,
						tRect.bottom - kInterfaceContentBuffer + 1);
		SetTranslateColorShadeFore( dItem->color, shade);
		PaintRect( &vRect);
		
		swidth = GetInterfaceStringWidth( s, dItem->style);
		swidth = uRect.left + ( uRect.right - uRect.left) / 2 - swidth / 2;
		MoveTo( swidth, uRect.top + GetInterfaceFontAscent(dItem->style));
		if ( dItem->item.plainButton.status == kDimmed)
		{
			SetTranslateColorShadeFore( dItem->color, VERY_DARK);
			mGetTranslateColorShade( dItem->color, VERY_DARK, color, transColor)
		}

		else 
		{
			SetTranslateColorShadeFore( dItem->color, DARKEST);
			mGetTranslateColorShade( dItem->color, DARKEST, color, transColor)
		}

		
		DrawInterfaceString( (anyCharType *)s, dItem->style, destMap, portLeft,
						portTop,  color);
		
		
		// draw the button title
		
		if ( dItem->item.plainButton.status == kIH_Hilite)
		{
			SetTranslateColorShadeFore( dItem->color, DARKEST);
			mGetTranslateColorShade( dItem->color, DARKEST, color, transColor)
		} else if ( dItem->item.plainButton.status == kDimmed)
		{
			SetTranslateColorShadeFore( dItem->color, DARKEST + kSlightlyLighterColor);
			mGetTranslateColorShade( dItem->color, DARKEST + kSlightlyLighterColor, color, transColor)
		}
		else
		{
			SetTranslateColorShadeFore( dItem->color, LIGHTER);
			mGetTranslateColorShade( dItem->color, LIGHTER, color, transColor)
		}
		GetIndString( s, dItem->item.plainButton.label.stringID,
					dItem->item.plainButton.label.stringNumber);
		swidth = GetInterfaceStringWidth( s, dItem->style);
		swidth = uRect.right + ( tRect.right - uRect.right) / 2 - swidth / 2;
		sheight = GetInterfaceFontAscent(dItem->style) + kInterfaceTextVBuffer + tRect.top;
		MoveTo( swidth, sheight);
		DrawInterfaceString( (anyCharType *)s, dItem->style, destMap, portLeft,
						portTop,  color);
	}

	SetTranslateColorFore( BLACK);
}

void DrawPlayerInterfaceTabBoxButton( interfaceItemType *dItem, PixMap *destMap, long portLeft,
						long portTop)

{
	Rect			tRect, uRect, vRect;
	short			vcenter, swidth, sheight, thisHBorder = kInterfaceSmallHBorder;
	Str255			s;
	unsigned char	shade;
	unsigned char	color;
	transColorType	*transColor;
	
	if ( dItem->style == kLarge) thisHBorder = kInterfaceLargeHBorder;
	LongRectToRect( &(dItem->bounds), &tRect);
	
	uRect = tRect;
	uRect.right++;
	uRect.bottom++;
		
	tRect.left -= kInterfaceContentBuffer;
	tRect.top -= kInterfaceContentBuffer;
	tRect.right += kInterfaceContentBuffer;
	tRect.bottom += kInterfaceContentBuffer;
	
	// top border
	
	if ( dItem->item.radioButton.status == kDimmed)
		shade = VERY_DARK;
	else shade = MEDIUM;
	
	mDrawPuffUpTopBorder( tRect, uRect, dItem->color, shade, thisHBorder)
	// bottom border

//	mDrawPuffUpBottomBorder( tRect, uRect, dItem->color, shade, thisHBorder)
	
	// side border top
	
	vcenter = ( tRect.bottom - tRect.top) / 2;

	MacSetRect( &uRect, tRect.left - thisHBorder, tRect.top + kInterfaceHTop, tRect.left + 1,
			tRect.bottom - kInterfaceHTop + 1);
	MacSetRect( &vRect, tRect.right, tRect.top + kInterfaceHTop, tRect.right + thisHBorder + 1,
			tRect.bottom - kInterfaceHTop + 1);
	if ( !dItem->item.radioButton.on)
	{
		if ( dItem->item.radioButton.status == kIH_Hilite)
		{
			shade = LIGHT;
			mDrawPuffUpRect( uRect, dItem->color, shade)
			mDrawPuffUpRect( vRect, dItem->color, shade)
		} else
		{
			if ( dItem->item.radioButton.status == kDimmed)
				shade = VERY_DARK;
			else shade = DARK;
			mDrawPuffUpRect( uRect, dItem->color, shade)
			mDrawPuffUpRect( vRect, dItem->color, shade)
		}
		MacSetRect( &uRect, uRect.left, uRect.bottom, uRect.right, uRect.bottom + 3);
		MacSetRect( &vRect, vRect.left, vRect.bottom, vRect.right, vRect.bottom + 3);
		SetTranslateColorFore( BLACK);
		PaintRect( &uRect);
		PaintRect( &vRect);
		MacSetRect( &uRect, uRect.left - 3, uRect.bottom, vRect.right + 3, uRect.bottom + 3);
		shade = MEDIUM;
		SetTranslateColorShadeFore( dItem->color, shade);
		PaintRect( &uRect);
		SetTranslateColorShadeFore( dItem->color, shade + kLighterColor);
		MoveTo( uRect.left, uRect.top - 1);
		MacLineTo( uRect.right - 1, uRect.top - 1);
		SetTranslateColorShadeFore( dItem->color, shade + kDarkerColor);
		MoveTo( uRect.left, uRect.bottom);
		MacLineTo( uRect.right - 1, uRect.bottom);		
	} else
	{
		if ( dItem->item.radioButton.status == kIH_Hilite)
		{
			shade = LIGHT;
		} else
		{
			if ( dItem->item.radioButton.status == kDimmed)
				shade = VERY_DARK;
			else shade = MEDIUM;
		}
		uRect.bottom += 7;
		vRect.bottom += 7;
		SetTranslateColorShadeFore( dItem->color, shade);
		PaintRect( &uRect);
		PaintRect( &vRect);
		SetTranslateColorShadeFore( dItem->color, shade + kLighterColor);
		MoveTo( uRect.right - 2, uRect.top);
		MacLineTo( uRect.left, uRect.top);
		MacLineTo( uRect.left, uRect.bottom - 5);
		MacLineTo( uRect.left - 3, uRect.bottom - 5);
		MoveTo( vRect.right - 2, vRect.top);
		MacLineTo( vRect.left, vRect.top);
		MacLineTo( vRect.left, vRect.bottom - 2);
		MoveTo( vRect.right, vRect.bottom - 5);
		MacLineTo( vRect.right + 2, vRect.bottom - 5);
		SetTranslateColorShadeFore( dItem->color, shade + kDarkerColor);
		MoveTo( uRect.right - 1, uRect.top);
		MacLineTo( uRect.right - 1, uRect.bottom - 1);
		MacLineTo( uRect.left - 3, uRect.bottom - 1);
		MoveTo( vRect.right - 1, vRect.top);
		MacLineTo( vRect.right - 1, vRect.bottom - 4);
		MoveTo( vRect.left, vRect.bottom - 1);
		MacLineTo( vRect.right + 2, vRect.bottom - 1);
		MacSetRect( &uRect, uRect.left - 3, uRect.bottom - 4, uRect.right - 1, uRect.bottom - 1);
		SetTranslateColorShadeFore( dItem->color, shade);
		PaintRect( &uRect);
		MacSetRect( &vRect, vRect.left + 1, vRect.bottom - 4, vRect.right + 3, vRect.bottom - 1);
		PaintRect( &vRect);
		uRect.top--;
		uRect.bottom++;
		uRect.left = uRect.right + 1;
		uRect.right = vRect.left - 1;
		SetTranslateColorFore( BLACK);
		PaintRect( &uRect);
	}
	
	
	if ( dItem->item.radioButton.key == 0)
	{
		MacSetRect( &uRect, tRect.left +  kInterfaceContentBuffer,
			tRect.top + kInterfaceContentBuffer,
			tRect.left +  kInterfaceContentBuffer,
			tRect.bottom - kInterfaceContentBuffer);

		if ( !dItem->item.radioButton.on)
		{
			if ( dItem->item.radioButton.status == kIH_Hilite)
				shade = LIGHT;
			else shade = DARKER;//DARKEST + kSlightlyLighterColor;
		} else
		{
			shade = MEDIUM;
		}
		MacSetRect( &uRect, tRect.left +  kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
						tRect.right - kInterfaceContentBuffer + 1,
						tRect.bottom - kInterfaceContentBuffer + 1);
		SetTranslateColorShadeFore( dItem->color, shade);
		PaintRect( &uRect);
		
		if ( !dItem->item.radioButton.on)
		{
			if ( dItem->item.radioButton.status == kIH_Hilite)
			{
				SetTranslateColorShadeFore( dItem->color, DARKEST);
				mGetTranslateColorShade( dItem->color, DARKEST, color, transColor)
			}
			else if ( dItem->item.radioButton.status == kDimmed)
			{
				SetTranslateColorShadeFore( dItem->color, VERY_DARK);
				mGetTranslateColorShade( dItem->color, VERY_DARK, color, transColor)
			} else
			{
				SetTranslateColorShadeFore( dItem->color, LIGHT);
				mGetTranslateColorShade( dItem->color, LIGHT, color, transColor)
			}
		} else
		{
			SetTranslateColorShadeFore( dItem->color, VERY_LIGHT);
			mGetTranslateColorShade( dItem->color, VERY_LIGHT, color, transColor)
		}
		GetIndString( s, dItem->item.radioButton.label.stringID,
					dItem->item.radioButton.label.stringNumber);
		swidth = GetInterfaceStringWidth( s, dItem->style);
		swidth = tRect.left + ( tRect.right - tRect.left) / 2 - swidth / 2;
		sheight = GetInterfaceFontAscent(dItem->style) + kInterfaceTextVBuffer + tRect.top;
		MoveTo( swidth, sheight);
		DrawInterfaceString( (anyCharType *)s, dItem->style, destMap, portLeft,
						portTop,  color);
	} else
	{
		// draw the key code
		
		
		if ( !dItem->item.radioButton.on)
		{
			if ( dItem->item.radioButton.status == kIH_Hilite)
				shade = VERY_LIGHT;
			else shade = DARK;//DARKEST + kSlightlyLighterColor;
		} else
		{
			shade = MEDIUM + kLighterColor;
		}
		GetKeyNumName( s, dItem->item.radioButton.key);
		swidth = GetInterfaceFontWidth( dItem->style) * kMaxKeyNameLength;
		
		MacSetRect( &uRect, tRect.left +  kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
				tRect.left + kInterfaceContentBuffer + swidth + kInterfaceTextHBuffer * 2 + 1,
				tRect.bottom - kInterfaceContentBuffer + 1);
		mDrawPuffUpRect( uRect, dItem->color, shade)

		if ( !dItem->item.radioButton.on)
		{
			if ( dItem->item.radioButton.status == kIH_Hilite)
				shade = VERY_LIGHT;
			else shade = DARKER;//DARKEST + kSlightlyLighterColor;
		} else
		{
			shade = MEDIUM;
		}
		MacSetRect( &vRect, tRect.left + kInterfaceContentBuffer + swidth + kInterfaceTextHBuffer * 2 + 2,
		tRect.top + kInterfaceContentBuffer,
						tRect.right - kInterfaceContentBuffer + 1,
						tRect.bottom - kInterfaceContentBuffer + 1);
		SetTranslateColorShadeFore( dItem->color, shade);
		PaintRect( &vRect);
		
		swidth = GetInterfaceStringWidth( s, dItem->style);
		swidth = uRect.left + ( uRect.right - uRect.left) / 2 - swidth / 2;
		MoveTo( swidth, uRect.top + GetInterfaceFontAscent(dItem->style));
		if ( dItem->item.radioButton.status == kDimmed)
		{
			SetTranslateColorShadeFore( dItem->color, VERY_DARK);
			mGetTranslateColorShade( dItem->color, VERY_DARK, color, transColor)
		}

		else 
		{
			SetTranslateColorShadeFore( dItem->color, DARKEST);
			mGetTranslateColorShade( dItem->color, DARKEST, color, transColor)
		}

		
		DrawInterfaceString( (anyCharType *)s, dItem->style, destMap, portLeft,
						portTop,  color);
		
		
		// draw the button title
		
		if ( !dItem->item.radioButton.on)
		{
			if ( dItem->item.radioButton.status == kIH_Hilite)
			{
				SetTranslateColorShadeFore( dItem->color, DARKEST);
				mGetTranslateColorShade( dItem->color, DARKEST, color, transColor)
			}
			else if ( dItem->item.radioButton.status == kDimmed)
			{
				SetTranslateColorShadeFore( dItem->color, VERY_DARK);
				mGetTranslateColorShade( dItem->color, VERY_DARK, color, transColor)
			} else
			{
				SetTranslateColorShadeFore( dItem->color, LIGHT);
				mGetTranslateColorShade( dItem->color, LIGHT, color, transColor)
			}
		} else
		{
			SetTranslateColorShadeFore( dItem->color, VERY_LIGHT);
			mGetTranslateColorShade( dItem->color, VERY_LIGHT, color, transColor)
		}
		GetIndString( s, dItem->item.radioButton.label.stringID,
					dItem->item.radioButton.label.stringNumber);
		swidth = GetInterfaceStringWidth( s, dItem->style);
		swidth = uRect.right + ( tRect.right - uRect.right) / 2 - swidth / 2;
		sheight = GetInterfaceFontAscent(dItem->style) + kInterfaceTextVBuffer + tRect.top;
		MoveTo( swidth, sheight);
		DrawInterfaceString( (anyCharType *)s, dItem->style, destMap, portLeft,
						portTop,  color);
	}

	SetTranslateColorFore( BLACK);
}


void DrawPlayerInterfaceRadioButton( interfaceItemType *dItem, PixMap *destMap, long portLeft,
						long portTop)

{
	Rect			tRect, uRect, vRect, wRect;
	short			vcenter, swidth, sheight, thisHBorder = kInterfaceSmallHBorder;
	Str255			s;
	unsigned char	shade;
	unsigned char	color;
	transColorType	*transColor;
	
	if ( dItem->style == kLarge) thisHBorder = kInterfaceLargeHBorder;
	LongRectToRect( &(dItem->bounds), &tRect);
	
	tRect.left -= kInterfaceContentBuffer;
	tRect.top -= kInterfaceContentBuffer;
	tRect.right += kInterfaceContentBuffer;
	tRect.bottom += kInterfaceContentBuffer;	
	
	// top border
	
	if ( dItem->item.radioButton.status == kDimmed)
		shade = VERY_DARK;
	else shade = MEDIUM;
	
	mDrawPuffUpTopBorder( tRect, uRect, dItem->color, shade, thisHBorder)
	// bottom border

	mDrawPuffUpBottomBorder( tRect, uRect, dItem->color, shade, thisHBorder)
	
	// side border top
	
	vcenter = ( tRect.bottom - tRect.top) / 2;
	swidth = (tRect.top - kInterfaceVEdgeHeight - kInterfaceVCornerHeight + kIndicatorVOffset);
	sheight = (tRect.bottom + kInterfaceVEdgeHeight + kInterfaceVCornerHeight - kIndicatorVOffset) -
	 		swidth;

	MacSetRect( &wRect, tRect.left - thisHBorder - kCheckIndicatorHOffset - sheight, swidth,
			tRect.left - thisHBorder - kCheckIndicatorHOffset + 1, swidth + sheight + 1);

	MacSetRect( &uRect, tRect.left - thisHBorder - kCheckIndicatorHOffset - 2,
			tRect.top + kInterfaceHTop,
			tRect.left + 1,
			/*tRect.top + vcenter - kInterfaceVLipHeight + 1*/
			tRect.bottom - kInterfaceHTop + 1);
	MacSetRect( &vRect, tRect.right, tRect.top + kInterfaceHTop, tRect.right + thisHBorder + 1,
			/*tRect.top + vcenter - kInterfaceVLipHeight + 1*/
			tRect.bottom - kInterfaceHTop + 1);
			
	if ( dItem->item.radioButton.status == kIH_Hilite)
	{
		shade = LIGHT;
		mDrawPuffUpRect( uRect, dItem->color, shade)
		mDrawPuffUpRect( vRect, dItem->color, shade)

		wRect.left += 2;
		wRect.right += 2;
		SetTranslateColorFore( BLACK);
		FrameOval( &wRect);
		wRect.left -= 2;
		wRect.right -= 2;
		mDrawPuffUpOval( wRect, dItem->color, shade)
		
		MacInsetRect( &wRect, 3, 3);
		mDrawPuffDownOval( wRect, dItem->color, shade)
		MacInsetRect( &wRect, 1, 1);
		if ( !dItem->item.radioButton.on) SetTranslateColorFore( BLACK);
		else SetTranslateColorShadeFore( dItem->color, VERY_LIGHT);
		PaintOval( &wRect);
	} else
	{
		if ( dItem->item.radioButton.status == kDimmed)
			shade = VERY_DARK;
		else shade = MEDIUM + kSlightlyLighterColor;
		mDrawPuffUpRect( uRect, dItem->color, shade)
		mDrawPuffUpRect( vRect, dItem->color, shade)
		wRect.left += 2;
		wRect.right += 2;
		SetTranslateColorFore( BLACK);
		FrameOval( &wRect);
		wRect.left -= 2;
		wRect.right -= 2;
		mDrawPuffUpOval( wRect, dItem->color, shade)
		
		MacInsetRect( &wRect, 3, 3);
		mDrawPuffDownOval( wRect, dItem->color, shade)
		MacInsetRect( &wRect, 1, 1);
		if ( !dItem->item.radioButton.on) SetTranslateColorFore( BLACK);
		else if ( dItem->item.radioButton.status == kActive)
			SetTranslateColorShadeFore( dItem->color, LIGHT);
		else SetTranslateColorShadeFore( dItem->color, MEDIUM);
		PaintOval( &wRect);
	}
		
	MacSetRect( &uRect, tRect.left +  kInterfaceContentBuffer,
		tRect.top + kInterfaceContentBuffer,
		tRect.left +  kInterfaceContentBuffer,
		tRect.bottom - kInterfaceContentBuffer);

	if ( dItem->item.radioButton.status == kIH_Hilite)
		shade = LIGHT;
	else shade = DARKEST + kSlightlyLighterColor;
	MacSetRect( &uRect, tRect.left +  kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
					tRect.right - kInterfaceContentBuffer + 1,
					tRect.bottom - kInterfaceContentBuffer + 1);
	SetTranslateColorShadeFore( dItem->color, shade);
	PaintRect( &uRect);
	
	if ( dItem->item.radioButton.status == kIH_Hilite)
	{
		SetTranslateColorShadeFore( dItem->color, DARKEST);
		mGetTranslateColorShade( dItem->color, DARKEST, color, transColor)
	}

	else if ( dItem->item.radioButton.status == kDimmed)
	{
		SetTranslateColorShadeFore( dItem->color, DARK);
		mGetTranslateColorShade( dItem->color, DARK, color, transColor)
	}
	else
	{
		SetTranslateColorShadeFore( dItem->color, LIGHT);
		mGetTranslateColorShade( dItem->color, LIGHT, color, transColor)
	}
	GetIndString( s, dItem->item.radioButton.label.stringID,
				dItem->item.radioButton.label.stringNumber);
	swidth = GetInterfaceStringWidth( s, dItem->style);
	swidth = tRect.left + ( tRect.right - tRect.left) / 2 - swidth / 2;
	sheight = GetInterfaceFontAscent(dItem->style) + kInterfaceTextVBuffer + tRect.top;
	MoveTo( swidth, sheight);
	DrawInterfaceString( (anyCharType *)s, dItem->style, destMap, portLeft,
						portTop,  color);
	
	SetTranslateColorFore( BLACK);
}

void DrawPlayerInterfaceCheckBox( interfaceItemType *dItem, PixMap *destMap, long portLeft,
						long portTop)

{
	Rect			tRect, uRect, vRect, wRect;
	short			vcenter, swidth, sheight, thisHBorder = kInterfaceSmallHBorder;
	Str255			s;
	unsigned char	shade;
	unsigned char	color;
	transColorType	*transColor;
	
	if ( dItem->style == kLarge) thisHBorder = kInterfaceLargeHBorder;
	LongRectToRect( &(dItem->bounds), &tRect);
	
	tRect.left -= kInterfaceContentBuffer;
	tRect.top -= kInterfaceContentBuffer;
	tRect.right += kInterfaceContentBuffer;
	tRect.bottom += kInterfaceContentBuffer;	
	
	// top border
	
	if ( dItem->item.checkboxButton.status == kDimmed)
		shade = VERY_DARK;
	else shade = MEDIUM;
	
	mDrawPuffUpTopBorder( tRect, uRect, dItem->color, shade, thisHBorder)
	// bottom border

	mDrawPuffUpBottomBorder( tRect, uRect, dItem->color, shade, thisHBorder)
	
	// side border top
	
	vcenter = ( tRect.bottom - tRect.top) / 2;
	swidth = (tRect.top - kInterfaceVEdgeHeight - kInterfaceVCornerHeight + kIndicatorVOffset);
	sheight = (tRect.bottom + kInterfaceVEdgeHeight + kInterfaceVCornerHeight - kIndicatorVOffset) -
	 		swidth;

	MacSetRect( &wRect, tRect.left - thisHBorder - kCheckIndicatorHOffset - sheight, swidth,
			tRect.left - thisHBorder - kCheckIndicatorHOffset + 1, swidth + sheight + 1);

	MacSetRect( &uRect, tRect.left - thisHBorder - kCheckIndicatorHOffset + 2,
			tRect.top + kInterfaceHTop,
			tRect.left + 1,
			/*tRect.top + vcenter - kInterfaceVLipHeight + 1*/
			tRect.bottom - kInterfaceHTop + 1);
	MacSetRect( &vRect, tRect.right, tRect.top + kInterfaceHTop, tRect.right + thisHBorder + 1,
			/*tRect.top + vcenter - kInterfaceVLipHeight + 1*/
			tRect.bottom - kInterfaceHTop + 1);
			
	if ( dItem->item.checkboxButton.status == kIH_Hilite)
	{
		shade = LIGHT;
		mDrawPuffUpRect( uRect, dItem->color, shade)
		mDrawPuffUpRect( vRect, dItem->color, shade)
		mDrawPuffUpRect( wRect, dItem->color, shade)
		MacInsetRect( &wRect, 3, 3);
		mDrawPuffDownRect( wRect, dItem->color, shade)
		MacInsetRect( &wRect, 1, 1);
		if ( !dItem->item.checkboxButton.on) SetTranslateColorFore( BLACK);
		else SetTranslateColorShadeFore( dItem->color, VERY_LIGHT);
		PaintRect( &wRect);
	} else
	{
		if ( dItem->item.checkboxButton.status == kDimmed)
			shade = VERY_DARK;
		else shade = MEDIUM + kSlightlyLighterColor;
		mDrawPuffUpRect( uRect, dItem->color, shade)
		mDrawPuffUpRect( vRect, dItem->color, shade)
		mDrawPuffUpRect( wRect, dItem->color, shade)
		MacInsetRect( &wRect, 3, 3);
		mDrawPuffDownRect( wRect, dItem->color, shade)
		MacInsetRect( &wRect, 1, 1);
		if ( !dItem->item.checkboxButton.on) SetTranslateColorFore( BLACK);
		else if ( dItem->item.checkboxButton.status == kActive)
			SetTranslateColorShadeFore( dItem->color, LIGHT);
		else SetTranslateColorShadeFore( dItem->color, MEDIUM);
		PaintRect( &wRect);
	}
		
	MacSetRect( &uRect, tRect.left +  kInterfaceContentBuffer,
		tRect.top + kInterfaceContentBuffer,
		tRect.left +  kInterfaceContentBuffer,
		tRect.bottom - kInterfaceContentBuffer);

	if ( dItem->item.checkboxButton.status == kIH_Hilite)
		shade = LIGHT;
	else shade = DARKEST + kSlightlyLighterColor;
	MacSetRect( &uRect, tRect.left +  kInterfaceContentBuffer, tRect.top + kInterfaceContentBuffer,
					tRect.right - kInterfaceContentBuffer + 1,
					tRect.bottom - kInterfaceContentBuffer + 1);
	SetTranslateColorShadeFore( dItem->color, shade);
	PaintRect( &uRect);
	
	if ( dItem->item.checkboxButton.status == kIH_Hilite)
	{
		SetTranslateColorShadeFore( dItem->color, DARKEST);
		mGetTranslateColorShade( dItem->color, DARKEST, color, transColor)
	} else if ( dItem->item.checkboxButton.status == kDimmed)
	{
		SetTranslateColorShadeFore( dItem->color, DARK);
		mGetTranslateColorShade( dItem->color, DARK, color, transColor)
	} else
	{
		SetTranslateColorShadeFore( dItem->color, LIGHT);
		mGetTranslateColorShade( dItem->color, LIGHT, color, transColor)
	}
	GetIndString( s, dItem->item.checkboxButton.label.stringID,
				dItem->item.checkboxButton.label.stringNumber);
	swidth = GetInterfaceStringWidth( s, dItem->style);
	swidth = tRect.left + ( tRect.right - tRect.left) / 2 - swidth / 2;
	sheight = GetInterfaceFontAscent(dItem->style) + kInterfaceTextVBuffer + tRect.top;
	MoveTo( swidth, sheight);
	DrawInterfaceString( (anyCharType *)s, dItem->style, destMap, portLeft,
						portTop,  color);
	
	SetTranslateColorFore( BLACK);
}

void DrawPlayerInterfaceLabeledBox( interfaceItemType *dItem, PixMap *destMap, long portLeft,
						long portTop)

{
	Rect			tRect, uRect;
	short			vcenter, swidth, sheight, thisHBorder = kInterfaceSmallHBorder;
	Str255			s;
	unsigned char	shade;
	unsigned char	color;
	transColorType	*transColor;
	
	if ( dItem->style == kLarge) thisHBorder = kInterfaceLargeHBorder;
	LongRectToRect( &(dItem->bounds), &tRect);
	tRect.left -= kInterfaceContentBuffer;
	tRect.top -= kInterfaceContentBuffer + GetInterfaceFontHeight( dItem->style) +
			kInterfaceTextVBuffer * 2 + kLabelBottomHeight;
	tRect.right += kInterfaceContentBuffer;
	tRect.bottom += kInterfaceContentBuffer;
	
	shade = DARK;
	
	mDrawPuffUpTopBorder( tRect, uRect, dItem->color, shade, thisHBorder)
	// bottom border

	mDrawPuffUpBottomBorder( tRect, uRect, dItem->color, shade, thisHBorder)
	
	
	// draw the string
	
	GetIndString( s, dItem->item.labeledRect.label.stringID, dItem->item.labeledRect.label.stringNumber);
	swidth = GetInterfaceStringWidth( s, dItem->style) + kInterfaceTextHBuffer * 2;
	swidth = ( tRect.right - tRect.left) - swidth;
	sheight = GetInterfaceFontHeight( dItem->style) + kInterfaceTextVBuffer * 2;

	MacSetRect( &uRect, tRect.left + kInterfaceTextHBuffer - 1,
		tRect.top + kInterfaceHTop,
		tRect.right - swidth - kInterfaceTextHBuffer + 1,
		tRect.top + sheight - kInterfaceHTop);
	SetTranslateColorShadeFore( dItem->color, VERY_DARK);
	PaintRect( &uRect);

	SetTranslateColorShadeFore( dItem->color, LIGHT);
	mGetTranslateColorShade( dItem->color, LIGHT, color, transColor)

	MoveTo( tRect.left + kInterfaceTextHBuffer, tRect.top + GetInterfaceFontAscent( dItem->style) +
			kInterfaceTextVBuffer);
	DrawInterfaceString( (anyCharType *)s, dItem->style, destMap, portLeft,
						portTop,  color);
	
	// string left border
	
	shade = MEDIUM;
	vcenter = sheight / 2; 
	
	MacSetRect( &uRect, tRect.left - thisHBorder,
			tRect.top + kInterfaceHTop,
			tRect.left + 1, tRect.top + sheight - kInterfaceHTop + 1);
	mDrawPuffUpRect( uRect, dItem->color, shade)	
			
	// string right border
	
	shade = MEDIUM;
	MacSetRect( &uRect, tRect.right - swidth,
		tRect.top + kInterfaceHTop,
		tRect.right - 2,
		tRect.top + sheight - kInterfaceHTop + 1);
	mDrawPuffUpRect( uRect, dItem->color, shade)	
	MacSetRect( &uRect, tRect.right,
		tRect.top + kInterfaceHTop,
		tRect.right + thisHBorder + 1,
		tRect.top + sheight - kInterfaceHTop + 1);
	mDrawPuffUpRect( uRect, dItem->color, shade)	
	
	/*
	SetTranslateColorShadeFore( dItem->color, MEDIUM);
	MoveTo( tRect.right - swidth, tRect.top + kInterfaceHTop);
	MacLineTo( tRect.right + thisHBorder, tRect.top + kInterfaceHTop);
	MacLineTo( tRect.right + thisHBorder, tRect.top + vcenter - kInterfaceVLipHeight);
	MacLineTo( tRect.right - swidth, tRect.top + vcenter - kInterfaceVLipHeight);
	MacLineTo( tRect.right - swidth, tRect.top + kInterfaceHTop);

	SetTranslateColorShadeFore( dItem->color, DARK);
	MoveTo( tRect.right - swidth, tRect.top + sheight - vcenter + kInterfaceVLipHeight);
	MacLineTo( tRect.right + thisHBorder, tRect.top + sheight - vcenter + kInterfaceVLipHeight);
	MacLineTo( tRect.right + thisHBorder, tRect.top + sheight - kInterfaceHTop);
	MacLineTo( tRect.right - swidth, tRect.top + sheight - kInterfaceHTop);
	MacLineTo( tRect.right - swidth, tRect.top + sheight - vcenter + kInterfaceVLipHeight);
	*/
	// string bottom border
	
	mDrawPuffUpTBorder( tRect, uRect, dItem->color, DARK, sheight, thisHBorder)	

	// main part left border
	
	tRect.top += sheight + kLabelBottomHeight;	
	
	vcenter = ( tRect.bottom - tRect.top) / 2;
	
	MacSetRect( &uRect, tRect.left - thisHBorder,
		tRect.top + kInterfaceHTop,
		tRect.left + 1,
		tRect.top + vcenter - kInterfaceVLipHeight + 1);
	mDrawPuffUpRect( uRect, dItem->color, DARKER)	

	MacSetRect( &uRect, tRect.left - thisHBorder,
		tRect.bottom - vcenter + kInterfaceVLipHeight,
		tRect.left + 1,
		tRect.bottom - kInterfaceHTop + 1);
	mDrawPuffUpRect( uRect, dItem->color, VERY_DARK)	

	// right border
	
	MacSetRect( &uRect, tRect.right,
		tRect.top + kInterfaceHTop,
		tRect.right + thisHBorder + 1,
		tRect.top + vcenter - kInterfaceVLipHeight + 1);
	mDrawPuffUpRect( uRect, dItem->color, DARKER)	

	MacSetRect( &uRect, tRect.right,
		tRect.bottom - vcenter + kInterfaceVLipHeight,
		tRect.right + thisHBorder + 1,
		tRect.bottom - kInterfaceHTop + 1);
	mDrawPuffUpRect( uRect, dItem->color, VERY_DARK)	

	SetTranslateColorFore( BLACK);	
}

void DrawPlayerInterfaceList( interfaceItemType *dItem, PixMap *destMap, long portLeft,
						long portTop)

{
	Rect			tRect, uRect;
	short			vcenter, swidth, sheight, thisHBorder = kInterfaceSmallHBorder;
	Str255			s;
	RgnHandle		clipRgn = nil;
	unsigned char	color;
	transColorType	*transColor;
	
	if ( dItem->style == kLarge) thisHBorder = kInterfaceLargeHBorder;
	LongRectToRect( &(dItem->bounds), &tRect);

	tRect.left -= kInterfaceContentBuffer;
	tRect.top -= kInterfaceContentBuffer + GetInterfaceFontAscent( dItem->style) +
			kInterfaceTextVBuffer * 2 + kLabelBottomHeight;
	tRect.right += kInterfaceContentBuffer;
	tRect.bottom += kInterfaceContentBuffer;

	SetTranslateColorShadeFore( dItem->color, DARK);
	
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
	
	SetTranslateColorShadeFore( dItem->color, LIGHT);
	mGetTranslateColorShade( dItem->color, LIGHT, color, transColor)

	GetIndString( s, dItem->item.listRect.label.stringID, dItem->item.listRect.label.stringNumber);
	swidth = GetInterfaceStringWidth( s, dItem->style) + kInterfaceTextHBuffer * 2;
	sheight = GetInterfaceFontAscent(dItem->style) + kInterfaceTextVBuffer * 2;
	MoveTo( tRect.left + kInterfaceTextHBuffer, tRect.top + sheight - kInterfaceTextVBuffer);
	DrawInterfaceString( (anyCharType *)s, dItem->style, destMap, portLeft,
						portTop,  color);
	
	// string left border
	
	SetTranslateColorShadeFore( dItem->color, MEDIUM);
	vcenter = sheight / 2; 
	swidth = ( tRect.right - tRect.left) - swidth;
	
	MoveTo( tRect.left, tRect.top + kInterfaceHTop);
	MacLineTo( tRect.left - thisHBorder, tRect.top + kInterfaceHTop);
	MacLineTo( tRect.left - thisHBorder, tRect.top + vcenter - kInterfaceVLipHeight);
	MacLineTo( tRect.left, tRect.top + vcenter - kInterfaceVLipHeight);
	MacLineTo( tRect.left, tRect.top + kInterfaceHTop);
	
	SetTranslateColorShadeFore( dItem->color, DARK);
	MoveTo( tRect.left, tRect.top + sheight - vcenter + kInterfaceVLipHeight);
	MacLineTo( tRect.left - thisHBorder, tRect.top + sheight- vcenter + kInterfaceVLipHeight);
	MacLineTo( tRect.left - thisHBorder, tRect.top + sheight - kInterfaceHTop);
	MacLineTo( tRect.left, tRect.top + sheight - kInterfaceHTop);
	MacLineTo( tRect.left, tRect.top + sheight - vcenter + kInterfaceVLipHeight);
	
	// string right border
	
	SetTranslateColorShadeFore( dItem->color, MEDIUM);
	MoveTo( tRect.right - swidth, tRect.top + kInterfaceHTop);
	MacLineTo( tRect.right + thisHBorder, tRect.top + kInterfaceHTop);
	MacLineTo( tRect.right + thisHBorder, tRect.top + vcenter - kInterfaceVLipHeight);
	MacLineTo( tRect.right - swidth, tRect.top + vcenter - kInterfaceVLipHeight);
	MacLineTo( tRect.right - swidth, tRect.top + kInterfaceHTop);

	SetTranslateColorShadeFore( dItem->color, DARK);
	MoveTo( tRect.right - swidth, tRect.top + sheight - vcenter + kInterfaceVLipHeight);
	MacLineTo( tRect.right + thisHBorder, tRect.top + sheight - vcenter + kInterfaceVLipHeight);
	MacLineTo( tRect.right + thisHBorder, tRect.top + sheight - kInterfaceHTop);
	MacLineTo( tRect.right - swidth, tRect.top + sheight - kInterfaceHTop);
	MacLineTo( tRect.right - swidth, tRect.top + sheight - vcenter + kInterfaceVLipHeight);
	
	// string bottom border
	
	SetTranslateColorShadeFore( dItem->color, DARK);
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
	
	SetTranslateColorShadeFore( dItem->color, DARKER);
	MoveTo( tRect.left, tRect.top + kInterfaceHTop);
	MacLineTo( tRect.left - thisHBorder, tRect.top + kInterfaceHTop);
	MacLineTo( tRect.left - thisHBorder, tRect.top + vcenter - kInterfaceVLipHeight);
	MacLineTo( tRect.left, tRect.top + vcenter - kInterfaceVLipHeight);
	MacLineTo( tRect.left, tRect.top + kInterfaceHTop);

	SetTranslateColorShadeFore( dItem->color, VERY_DARK);
	MoveTo( tRect.left, tRect.bottom - vcenter + kInterfaceVLipHeight);
	MacLineTo( tRect.left - thisHBorder, tRect.bottom - vcenter + kInterfaceVLipHeight);
	MacLineTo( tRect.left - thisHBorder, tRect.bottom - kInterfaceHTop);
	MacLineTo( tRect.left, tRect.bottom - kInterfaceHTop);
	MacLineTo( tRect.left, tRect.bottom - vcenter + kInterfaceVLipHeight);
	
	// draw the list

	if (( dItem->item.listRect.getListLength == nil) || ( dItem->item.listRect.getItemString == nil))
	{
		dItem->item.listRect.lineUpStatus = dItem->item.listRect.pageUpStatus = 
		dItem->item.listRect.lineDownStatus = dItem->item.listRect.pageDownStatus = kDimmed;
	} else
	{
		clipRgn = NewRgn();
		GetClip( clipRgn);
		LongRectToRect( &(dItem->bounds), &tRect);
		ClipRect( &tRect);
		
		SetTranslateColorShadeFore( dItem->color, VERY_LIGHT);
		sheight = (*(dItem->item.listRect.getListLength))();
		vcenter = tRect.top;
		swidth = dItem->item.listRect.topItem;
		while (( swidth < (*(dItem->item.listRect.getListLength))()) &&
				( vcenter < tRect.bottom))
		{
			vcenter += GetInterfaceFontAscent(dItem->style) + kInterfaceTextVBuffer;
			MacSetRect( &uRect, tRect.left, vcenter - GetInterfaceFontAscent(dItem->style) -
					kInterfaceTextVBuffer, tRect.right, vcenter);
			if ( (*(dItem->item.listRect.itemHilited))( swidth, FALSE))
			{
				SetTranslateColorShadeFore( dItem->color, LIGHT);
				PaintRect( &uRect);
				SetTranslateColorShadeFore( dItem->color, VERY_DARK);
				mGetTranslateColorShade( dItem->color, VERY_DARK, color, transColor)
			} else
			{
				DefaultColors();
				PaintRect( &uRect);
				SetTranslateColorShadeFore( dItem->color, LIGHT);
				mGetTranslateColorShade( dItem->color, LIGHT, color, transColor)
			}
			MoveTo( tRect.left, vcenter);
			(*(dItem->item.listRect.getItemString))( swidth, (anyCharType *)s);
			DrawInterfaceString( (anyCharType *)s, dItem->style, destMap, portLeft,
						portTop,  color);
			swidth++;
		}
		SetClip( clipRgn);
		DisposeRgn( clipRgn);
	
		
		if ( dItem->item.listRect.topItem == 0)
			dItem->item.listRect.lineUpStatus = dItem->item.listRect.pageUpStatus = kDimmed;
		else dItem->item.listRect.lineUpStatus = dItem->item.listRect.pageUpStatus = kActive;
		if ( vcenter < tRect.bottom)
			dItem->item.listRect.lineDownStatus = dItem->item.listRect.pageDownStatus = kDimmed;
		else dItem->item.listRect.lineDownStatus = dItem->item.listRect.pageDownStatus = kActive;
	}
	// right border
	
	DrawPlayerListLineUp( dItem);
	DrawPlayerListPageUp( dItem);
	DrawPlayerListLineDown( dItem);
	DrawPlayerListPageDown( dItem);
	SetTranslateColorFore( BLACK);	
}

void DrawPlayerInterfaceListEntry( interfaceItemType *dItem, short whichEntry, PixMap *destMap, long portLeft,
						long portTop)

{
	Str255			s;
	RgnHandle		clipRgn = nil;
	Rect			tRect, uRect;
	short			swidth, vcenter;
	unsigned char	color;
	transColorType	*transColor;
	
	if (( dItem->item.listRect.getListLength != nil) && ( dItem->item.listRect.getItemString != nil))
	{
		clipRgn = NewRgn();
		GetClip( clipRgn);
		LongRectToRect( &(dItem->bounds), &tRect);
		ClipRect( &tRect);
		
		SetTranslateColorShadeFore( dItem->color, VERY_LIGHT);
		vcenter = tRect.top + ( whichEntry - dItem->item.listRect.topItem) *
				(GetInterfaceFontAscent(dItem->style) + kInterfaceTextVBuffer);
		swidth = whichEntry;
		if (( swidth < (*(dItem->item.listRect.getListLength))()) &&
				( vcenter < tRect.bottom))
		{
			vcenter += GetInterfaceFontAscent(dItem->style) + kInterfaceTextVBuffer;
			MacSetRect( &uRect, tRect.left, vcenter - GetInterfaceFontAscent(dItem->style) -
					kInterfaceTextVBuffer, tRect.right, vcenter);
			if ( (*(dItem->item.listRect.itemHilited))( swidth, FALSE))
			{
				SetTranslateColorShadeFore( dItem->color, LIGHT);
				PaintRect( &uRect);
				SetTranslateColorShadeFore( dItem->color, VERY_DARK);
				mGetTranslateColorShade( dItem->color, VERY_DARK, color, transColor)
			} else
			{
				DefaultColors();
				PaintRect( &uRect);
				SetTranslateColorShadeFore( dItem->color, LIGHT);
				mGetTranslateColorShade( dItem->color, LIGHT, color, transColor)
			}
			MoveTo( tRect.left, vcenter);
			(*(dItem->item.listRect.getItemString))( swidth, (anyCharType *)s);
			DrawInterfaceString( (anyCharType *)s, dItem->style, destMap, portLeft,
						portTop,  color);
			swidth++;
		}
		SetClip( clipRgn);
		DisposeRgn( clipRgn);
	
	}
}	

void DrawPlayerListLineUp( interfaceItemType *dItem)

{
	Rect		tRect;
	short		thisHBorder = kInterfaceSmallHBorder;
	
	if ( dItem->style == kLarge) thisHBorder = kInterfaceLargeHBorder;
	GetPlayerListLineUpRect( dItem, &tRect);
	
	if ( dItem->item.listRect.lineUpStatus != kIH_Hilite)
	{
		if (  dItem->item.listRect.lineUpStatus == kDimmed) SetTranslateColorShadeFore( dItem->color, DARKER);
		else SetTranslateColorShadeFore( dItem->color, LIGHT);
		MacFrameRect( &tRect);
		SetTranslateColorFore( BLACK);
		MacInsetRect( &tRect, 1, 1);
		PaintRect( &tRect);
		if (  dItem->item.listRect.lineUpStatus == kDimmed) SetTranslateColorShadeFore( dItem->color, DARKER);
		else SetTranslateColorShadeFore( dItem->color, LIGHT);
		MacInsetRect( &tRect, -1, -1);
	} else
	{
		SetTranslateColorShadeFore( dItem->color, VERY_LIGHT);
		PaintRect( &tRect);
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

void GetPlayerListLineUpRect( interfaceItemType *dItem, Rect *dRect)

{
	short		thisHBorder = kInterfaceSmallHBorder;

	if ( dItem->style == kLarge) thisHBorder = kInterfaceLargeHBorder;
	LongRectToRect( &(dItem->bounds), dRect);
	dRect->left = dRect->right + kInterfaceContentBuffer;
	dRect->right = dRect->left + thisHBorder + 1;
	dRect->top = dRect->top - kInterfaceContentBuffer + kInterfaceHTop;
	dRect->bottom = dRect->top + kScrollArrowVBuffer * 2 + kSmallArrowPointHeight +
		kSmallArrowBaseHeight + 1;
}

void DrawPlayerListPageUp( interfaceItemType *dItem)

{
	Rect		tRect;
	short		thisHBorder = kInterfaceSmallHBorder;

	if ( dItem->style == kLarge) thisHBorder = kInterfaceLargeHBorder;
	GetPlayerListPageUpRect( dItem, &tRect);
	
	if (  dItem->item.listRect.pageUpStatus != kIH_Hilite)
	{
		if ( dItem->item.listRect.pageUpStatus == kDimmed) SetTranslateColorShadeFore( dItem->color, DARKER);
		else SetTranslateColorShadeFore( dItem->color, LIGHT);
		MacFrameRect( &tRect);
		SetTranslateColorFore( BLACK);
		MacInsetRect( &tRect, 1, 1);
		PaintRect( &tRect);
		if ( dItem->item.listRect.pageUpStatus == kDimmed) SetTranslateColorShadeFore( dItem->color, DARKER);
		else SetTranslateColorShadeFore( dItem->color, LIGHT);
		MacInsetRect( &tRect, -1, -1);
	} else
	{
		SetTranslateColorShadeFore( dItem->color, VERY_LIGHT);
		PaintRect( &tRect);
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

void GetPlayerListPageUpRect( interfaceItemType *dItem, Rect *dRect)

{
	short		thisHBorder = kInterfaceSmallHBorder;

	GetPlayerListLineUpRect( dItem, dRect);
	dRect->top = dRect->bottom + kInterfaceVLipHeight;
	dRect->bottom = dRect->top + kScrollArrowVBuffer * 2 + kLargeArrowPointHeight +
			kLargeArrowBaseHeight + 1;
}

void DrawPlayerListLineDown( interfaceItemType *dItem)

{
	Rect		tRect;
	short		thisHBorder = kInterfaceSmallHBorder;
	
	if ( dItem->style == kLarge) thisHBorder = kInterfaceLargeHBorder;
	GetPlayerListLineDownRect( dItem, &tRect);

	if ( dItem->item.listRect.lineDownStatus != kIH_Hilite)
	{
		if ( dItem->item.listRect.lineDownStatus == kDimmed) SetTranslateColorShadeFore( dItem->color, DARKER);
		else SetTranslateColorShadeFore( dItem->color, LIGHT);
		MacFrameRect( &tRect);
		SetTranslateColorFore( BLACK);
		MacInsetRect( &tRect, 1, 1);
		PaintRect( &tRect);
		if ( dItem->item.listRect.lineDownStatus == kDimmed) SetTranslateColorShadeFore( dItem->color, DARKER);
		else SetTranslateColorShadeFore( dItem->color, LIGHT);
		MacInsetRect( &tRect, -1, -1);
	} else
	{
		SetTranslateColorShadeFore( dItem->color, VERY_LIGHT);
		PaintRect( &tRect);
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

void GetPlayerListLineDownRect( interfaceItemType *dItem, Rect *dRect)

{
	short		thisHBorder = kInterfaceSmallHBorder;

	if ( dItem->style == kLarge) thisHBorder = kInterfaceLargeHBorder;
	LongRectToRect( &(dItem->bounds), dRect);

	dRect->left = dRect->right + kInterfaceContentBuffer;
	dRect->right = dRect->left + thisHBorder + 1;
	dRect->top = dRect->bottom + kInterfaceContentBuffer - kInterfaceHTop -
			( kScrollArrowVBuffer * 2 + kSmallArrowPointHeight + kSmallArrowBaseHeight);
	dRect->bottom = dRect->bottom + kInterfaceContentBuffer - kInterfaceHTop + 1;
}

void DrawPlayerListPageDown( interfaceItemType *dItem)

{
	Rect		tRect;
	short		thisHBorder = kInterfaceSmallHBorder;
	
	if ( dItem->style == kLarge) thisHBorder = kInterfaceLargeHBorder;
	GetPlayerListPageDownRect( dItem, &tRect);

	if ( dItem->item.listRect.pageDownStatus != kIH_Hilite)
	{
		if ( dItem->item.listRect.pageDownStatus == kDimmed) SetTranslateColorShadeFore( dItem->color, DARKER);
		else SetTranslateColorShadeFore( dItem->color, LIGHT);
		MacFrameRect( &tRect);
		SetTranslateColorFore( BLACK);
		MacInsetRect( &tRect, 1, 1);
		PaintRect( &tRect);
		if ( dItem->item.listRect.pageDownStatus == kDimmed) SetTranslateColorShadeFore( dItem->color, DARKER);
		else SetTranslateColorShadeFore( dItem->color, LIGHT);
		MacInsetRect( &tRect, -1, -1);
	} else
	{
		SetTranslateColorShadeFore( dItem->color, VERY_LIGHT);
		PaintRect( &tRect);
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

void GetPlayerListPageDownRect( interfaceItemType *dItem, Rect *dRect)

{
	short		thisHBorder = kInterfaceSmallHBorder;

	if ( dItem->style == kLarge) thisHBorder = kInterfaceLargeHBorder;
	GetPlayerListLineDownRect( dItem, dRect);
	dRect->bottom = dRect->top - kInterfaceVLipHeight;
	dRect->top = dRect->bottom - kScrollArrowVBuffer * 2 - kLargeArrowPointHeight -
			kLargeArrowBaseHeight;
}

void DrawInterfaceTextRect( interfaceItemType *dItem, PixMap *destMap, long portLeft,
						long portTop)

{
	Handle			textData = nil;
	long			length;
	RgnHandle		clipRgn = nil;
	Rect			tRect;
	anyCharType		*sChar, *dChar, *aheadChar, *wordlen, *theLine, thisLen;
	short			vline = 0, hleft = 0, fheight = 0, xpos = 0;
	unsigned char	color, *charwidthptr, charwidth;
	transColorType	*transColor;
	
	if ( dItem->item.textRect.visibleBounds)
		DrawPlayerInterfacePlainRect( &(dItem->bounds), dItem->color, dItem->style,
				destMap, portLeft, portTop);
	clipRgn = NewRgn();
	GetClip( clipRgn);
	LongRectToRect( &(dItem->bounds), &tRect);
	ClipRect( &tRect);
	
	hleft = tRect.left + kInterfaceTextHBuffer;
	fheight = GetInterfaceFontHeight( dItem->style) + kInterfaceTextVBuffer;
	vline = tRect.top - ( fheight - GetInterfaceFontAscent( dItem->style));
	
	theLine = ( anyCharType *)NewPtr( sizeof( anyCharType) * kMaxLineLength);
	if ( theLine != nil)
	{
		wordlen = theLine;
		
		textData = HHGetResource( 'TEXT', dItem->item.textRect.textID);
		if ( textData != nil)
		{
			HLockHi( textData);
			
			length = GetHandleSize( textData);
			sChar = ( anyCharType *)*textData;
		
			SetTranslateColorShadeFore( dItem->color, VERY_LIGHT);
			
			while (( length > 0) && ( vline < tRect.bottom))
			{
				vline += fheight;
				xpos = hleft;
				*wordlen = 0;
				thisLen = 0;
				dChar = wordlen + 1;
				aheadChar = sChar;
				while (( *aheadChar == kSpaceChar) && ( length > 0)) { aheadChar++; length--;}
				
				while (( xpos < tRect.right - kInterfaceTextHBuffer) && ( length > 0))
				{
					*wordlen += (anyCharType)thisLen;
					sChar = aheadChar;
					length -= thisLen;
					thisLen = 0;
					do
					{
						if ((( *aheadChar >= 'A') && ( *aheadChar <= 'Z')) || (( *aheadChar >= '0') &&
								( *aheadChar <= '9')))
							SetInterfaceLargeUpperFont( dItem->style);
						else SetInterfaceLargeLowerFont( dItem->style);
						if (( *aheadChar >= 'a') && ( *aheadChar <= 'z'))
						{
							mDirectCharWidth( charwidth, *aheadChar/* - 'a' + 'A'*/, charwidthptr)
							xpos += charwidth;
						}
						else
						{
							mDirectCharWidth( charwidth, *aheadChar, charwidthptr)
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
						xpos = tRect.right;		// to force us out of loop						
					} else if (( *wordlen == 0) && (xpos >= tRect.right - kInterfaceTextHBuffer))
					{
						thisLen--;
						*wordlen = thisLen;
						length -= thisLen;
						sChar = aheadChar - 1;
					}
				}
				MoveTo( hleft, vline);		
			
				SetTranslateColorShadeFore( dItem->color, VERY_LIGHT);
				mGetTranslateColorShade( dItem->color, VERY_LIGHT, color, transColor)
		
				DrawInterfaceString( theLine, dItem->style, destMap, portLeft,
						portTop,  color);
			}
			ReleaseResource( textData);
		}
		DisposePtr( (Ptr)theLine);
	}
	SetClip( clipRgn);
	DisposeRgn( clipRgn);
}

void DrawInterfaceTextInRect( Rect *tRect, anyCharType *textData, long length,
							interfaceStyleType style, unsigned char textcolor,
							PixMap *destMap,
							long portLeft, long portTop, inlinePictType *inlinePict)

{
	RgnHandle		clipRgn = nil;
	anyCharType		*sChar, *dChar, *aheadChar, *wordlen, *theLine, thisLen;
	short			vline = 0, hleft = 0, fheight = 0, xpos = 0, inlinePictNum = 0, i;
	Boolean			processInline = false;
	Str255			inlineString;
	inlineKindType	inlineKind = kNoKind;
	long			inlineValue = 0;
	PicHandle		thePicture = nil;
	Rect			uRect;
	unsigned char	color, *charwidthptr, charwidth;
	transColorType	*transColor;
	inlinePictType	*thisInlinePict;
	
	clipRgn = NewRgn();
	if ( clipRgn == nil) return;
	GetClip( clipRgn);
	ClipRect( tRect);
	
	hleft = tRect->left + kInterfaceTextHBuffer;
	fheight = GetInterfaceFontHeight( style) + kInterfaceTextVBuffer;
	vline = tRect->top - ( fheight - GetInterfaceFontAscent( style));
	
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
	
	theLine = ( anyCharType *)NewPtr( sizeof( anyCharType) * kMaxLineLength);
	if ( theLine != nil)
	{
		wordlen = theLine;
		
		if ( textData != nil)
		{
			sChar = ( anyCharType *)textData;
		
			SetTranslateColorShadeFore( textcolor, VERY_LIGHT);	
			
			while (( length > 0) && ( vline < tRect->bottom))
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

				while (( xpos < tRect->right - kInterfaceTextHBuffer) && ( length > 0))
				{
					*wordlen += (anyCharType)thisLen;
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
										xpos = tRect->right;
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
							mDirectCharWidth( charwidth, *aheadChar/* - 'a' + 'A'*/, charwidthptr)
							xpos += charwidth;
						}
						else
						{
							mDirectCharWidth( charwidth, *aheadChar, charwidthptr)
							xpos += charwidth;
						}
						if ( *aheadChar == kReturnChar) *dChar++ = *aheadChar;
						else *dChar++ = *aheadChar;
						thisLen++;
						aheadChar++;

					} while (( *aheadChar != kSpaceChar) && ( length - thisLen > 0) &&
							( *aheadChar != kReturnChar) && ( inlineKind == kNoKind) &&
							!(( *wordlen == 0) && (xpos >= tRect->right - kInterfaceTextHBuffer)));
				
					if ( *aheadChar == kReturnChar)
					{
						if ( xpos < tRect->right - kInterfaceTextHBuffer)
						{
							*wordlen += thisLen;
							length -= thisLen;
							sChar = aheadChar;
						}
						xpos = tRect->right;		// to force us out of loop
					} else if (( *wordlen == 0) && (xpos >= tRect->right - kInterfaceTextHBuffer))
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
						}
					}

				}
				MoveTo( hleft, vline);		
				SetTranslateColorShadeFore( textcolor, VERY_LIGHT);
				mGetTranslateColorShade( textcolor, VERY_LIGHT, color, transColor)
				DrawInterfaceString( theLine, style, destMap, portLeft,
						portTop,  color);
				switch( inlineKind)
				{
					case kVPictKind: case kVClearPictKind:
						vline += ( fheight - GetInterfaceFontAscent( style));
						thePicture = (PicHandle)HHGetResource( 'PICT', inlineValue); //HHGetResource
						xpos = hleft;
						if ( *theLine == 0) vline -= fheight;
						if ( thePicture != nil)
						{
							uRect = (**thePicture).picFrame;
							MacOffsetRect( &uRect, -uRect.left + xpos - kInterfaceTextHBuffer +
										( tRect->right - tRect->left) / 2 - ( uRect.right -
										uRect.left) / 2,
										-uRect.top + vline);
							DrawPicture( thePicture, &uRect);
							ReleaseResource( (Handle)thePicture);
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
				}
			}
		}
		DisposePtr( (Ptr)theLine);
	}
	SetClip( clipRgn);
	DisposeRgn( clipRgn);
}

short GetInterfaceTextHeightFromWidth( anyCharType *textData, long length,
							interfaceStyleType style, short boundsWidth)

{
	anyCharType		*sChar, *dChar, *aheadChar, *wordlen, *theLine, thisLen;
	short			vline = 0, hleft = 0, fheight = 0, xpos = 0;
	Str255			inlineString;
	inlineKindType	inlineKind = kNoKind;
	long			inlineValue = 0;
	PicHandle		thePicture = nil;
	Rect			uRect;
	unsigned char	*charwidthptr, charwidth;
	
	hleft = kInterfaceTextHBuffer;
	fheight = GetInterfaceFontHeight( style) + kInterfaceTextVBuffer;
	vline = 0 - ( fheight - GetInterfaceFontAscent( style));
	
	theLine = ( anyCharType *)NewPtr( sizeof( anyCharType) * kMaxLineLength);
	if ( theLine != nil)
	{
		wordlen = theLine;
		
		if ( textData != nil)
		{
			sChar = ( anyCharType *)textData;
					
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
					*wordlen += (anyCharType)thisLen;
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
							mDirectCharWidth( charwidth, *aheadChar/* - 'a' + 'A'*/, charwidthptr)
							xpos += charwidth;
						}
						else
						{
							mDirectCharWidth( charwidth, *aheadChar, charwidthptr)
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
						xpos = boundsWidth;		// to force us out of loop
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
						}
					}

				}
				switch( inlineKind)
				{
					case kVPictKind: case kVClearPictKind:
						thePicture = (PicHandle)HHGetResource( 'PICT', inlineValue);
						xpos = hleft;
						vline += ( fheight - GetInterfaceFontAscent( style));
						if ( *theLine == 0) vline -= fheight;
						if ( thePicture != nil)
						{
							uRect = (**thePicture).picFrame;
							MacOffsetRect( &uRect, -uRect.left + xpos, -uRect.top + vline);
//							DrawPicture( thePicture, &uRect);
							ReleaseResource( (Handle)thePicture);
							vline += uRect.bottom - uRect.top;
							xpos = hleft;
						}
						inlineKind = kNoKind;
						break;
				}
			}
		}
		DisposePtr( (Ptr)theLine);
	}
	return( vline + ( fheight - GetInterfaceFontAscent( style)) );
}

void DrawInterfacePictureRect( interfaceItemType *dItem, PixMap *destMap, long portLeft,
						long portTop)

{
	PicHandle		thePicture = nil;
	RgnHandle		clipRgn = nil;
	Rect			tRect, uRect;
	
	if ( dItem->item.pictureRect.visibleBounds)
		DrawPlayerInterfacePlainRect( &(dItem->bounds), dItem->color, dItem->style,
				destMap, portLeft, portTop);
		
	clipRgn = NewRgn();
	GetClip( clipRgn);
	LongRectToRect( &(dItem->bounds), &tRect);
	ClipRect( &tRect);
	
//	thePicture = GetPicture( dItem->item.pictureRect.pictureID);
	thePicture = (PicHandle)HHGetResource( 'PICT', dItem->item.pictureRect.pictureID);
	if ( thePicture != nil)
	{
		uRect = (**thePicture).picFrame;
		MacOffsetRect( &uRect, -uRect.left + tRect.left, -uRect.top + tRect.top);
		DrawPicture( thePicture, &uRect);
		ReleaseResource( (Handle)thePicture);
	}

	SetClip( clipRgn);
	DisposeRgn( clipRgn);
}
		
void DrawAnyInterfaceItem( interfaceItemType *anItem, PixMap *destMap, long portLeft,
						long portTop)

{
	switch ( anItem->kind)
	{
		case kPlainRect:
			if (anItem->item.pictureRect.visibleBounds)
				DrawPlayerInterfacePlainRect( &(anItem->bounds), anItem->color, anItem->style,
					destMap, portLeft, portTop);
			break;
		case kTabBox:
			DrawPlayerInterfaceTabBox( &(anItem->bounds), anItem->color, anItem->style,
				destMap, portLeft, portTop, anItem->item.tabBox.topRightBorderSize);
			break;
		case kLabeledRect:
			DrawPlayerInterfaceLabeledBox( anItem,
				destMap, portLeft, portTop);
			break;
		case kListRect:
			DrawPlayerInterfaceList( anItem,
				destMap, portLeft, portTop);
			break;
		case kTextRect:
			 DrawInterfaceTextRect( anItem,
				destMap, portLeft, portTop);
			break;
		case kPlainButton:
			DrawPlayerInterfaceButton( anItem,
				destMap, portLeft, portTop);
			break;
		case kRadioButton:
			DrawPlayerInterfaceRadioButton( anItem,
				destMap, portLeft, portTop);
			break;
		case kTabBoxButton:
			DrawPlayerInterfaceTabBoxButton( anItem,
				destMap, portLeft, portTop);
			break;
			
		case kCheckboxButton:
			DrawPlayerInterfaceCheckBox( anItem,
				destMap, portLeft, portTop);
			break;
		case kPictureRect:
			DrawInterfacePictureRect( anItem,
				destMap, portLeft, portTop);
			break;
		default:
			WriteDebugLine( (char *)"\pHuh?");
			break;
	}
}

void GetAnyInterfaceItemGraphicBounds( interfaceItemType *anItem, Rect *bounds)

{
	short	thisHBorder = kInterfaceSmallHBorder;
	
	LongRectToRect( &anItem->bounds, bounds);
	
	if ( anItem->style == kLarge)
		thisHBorder = kInterfaceLargeHBorder;
		
	bounds->left -= kInterfaceContentBuffer;
	bounds->top -= kInterfaceContentBuffer;
	bounds->right += kInterfaceContentBuffer + 1;
	bounds->bottom += kInterfaceContentBuffer + 1;

	switch ( anItem->kind)
	{
		case kPlainRect:
			bounds->left -= thisHBorder;
			bounds->right += thisHBorder;
			bounds->top -= kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
			bounds->bottom += kInterfaceVEdgeHeight + kInterfaceVCornerHeight;
			break;
			
		case kLabeledRect:
			bounds->left -= thisHBorder;
			bounds->right += thisHBorder;
			bounds->top -= GetInterfaceFontHeight( anItem->style) + kInterfaceTextVBuffer * 2 +
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
			bounds->top -= GetInterfaceFontAscent( anItem->style) + kInterfaceTextVBuffer * 2 +
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
			WriteDebugLine( (char *)"\pHuh?");
			break;
	}
}

void GetAnyInterfaceItemContentBounds( interfaceItemType *anItem, Rect *bounds)

{
	LongRectToRect( &anItem->bounds, bounds);
}

short GetInterfaceStringWidth( anyCharType *s, interfaceStyleType style)

{
/*	short	len, width = 0;
	
	len = (short)*s++;
	while ( len > 0)
	{
		if ((( *s >= 'A') && ( *s <= 'Z')) || (( *s >= '0') && ( *s <= '9')))
			SetInterfaceLargeUpperFont( style);
		else SetInterfaceLargeLowerFont( style);
		
		if (( *s >= 'a') && ( *s <= 'z'))
			width += CharWidth( *s - 'a' + 'A');
		else width += CharWidth( *s);
		s++;
		len--;
	}
*/
	unsigned char	*getwidchar, *getwidwid;
	long			width, height, strlen;

	SetInterfaceLargeUpperFont( style);
	mGetDirectStringDimensions( s, width, height, strlen, getwidchar, getwidwid)

	return ( width);
}

/* GetInterfaceFontWidth:		-- NOT WORLD-READY! --

	We're not using fontInfo.widMax because we know we're never going to use the ultra-wide
	characters like Î and the like, and we're not using a mono-spaced font.  Therefore, we're
	using the width of 'R' which is about as wide as our normal letters get.
*/

short GetInterfaceFontWidth( interfaceStyleType style)

{
	
	SetInterfaceLargeUpperFont( style);
//	return( CharWidth( kWidestChar));
//	mSetDirectFont( kButtonFontNum)
	return( gDirectText->logicalWidth);
}

short GetInterfaceFontHeight( interfaceStyleType style)

{
//	FontInfo	f;
	
	SetInterfaceLargeUpperFont( style);
//	GetFontInfo( &f);
//	return( f.ascent);
//	mSetDirectFont( kButtonFontNum)
	return( gDirectText->height);
}

short GetInterfaceFontAscent( interfaceStyleType style)

{
//	FontInfo	f;
	
	SetInterfaceLargeUpperFont( style);
//	GetFontInfo( &f);
//	return( f.ascent);
//	mSetDirectFont( kButtonFontNum)
	return( gDirectText->ascent);
}

/* DrawInterfaceString:
	Relies on roman alphabet for upper/lower casing.  NOT WORLD-READY!
*/

void DrawInterfaceString( anyCharType *s, interfaceStyleType style, PixMap *destMap, long portLeft,
						long portTop, unsigned char color)

{
	longRect	clipRect;
	
	RectToLongRect( &(destMap->bounds), &clipRect);
//	mSetDirectFont( kButtonFontNum)
	SetInterfaceLargeUpperFont( style);
	DrawDirectTextStringClipped( s, color, destMap, &clipRect, portLeft, portTop);
	
	/*
	len = (short)*s++;
	while ( len > 0)
	{
		if ((( *s >= 'A') && ( *s <= 'Z')) || (( *s >= '0') && ( *s <= '9')))
			SetInterfaceLargeUpperFont( style);
		else SetInterfaceLargeLowerFont( style);
		
		if (( *s >= 'a') && ( *s <= 'z'))
			DrawChar( *s - 'a' + 'A');
		else DrawChar( *s);
		s++;
		len--;
	}
	*/
}

void SetInterfaceLargeUpperFont( interfaceStyleType style)

{
	if ( style == kSmall)
	{
		mSetDirectFont( kButtonSmallFontNum)
/*		TextFont( monaco);
		TextSize( 9);
		TextFace(  0);
*/	} else
	{
		mSetDirectFont( kButtonFontNum)
/*		TextFont( helvetica);
		TextSize( 12);
		TextFace( bold | extend);
*/	}
}

void SetInterfaceLargeLowerFont( interfaceStyleType style)

{
	if ( style == kSmall)
	{
		mSetDirectFont( kButtonSmallFontNum)
/*		TextFont( monaco);
		TextSize( 9);
		TextFace(  0);
*/	} else
	{
		mSetDirectFont( kButtonFontNum)
/*		SetFontByString( "\pgeneva");
		TextSize( 10);
		TextFace( bold | extend);
*/	}
}

