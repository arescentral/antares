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

#include "ScreenLabel.hpp"

#include <QDOffscreen.h>

#include "Admiral.hpp"            // hack for checking strength
#include "AresGlobalType.hpp"
#include "ColorTranslation.hpp"
#include "ConditionalMacros.h"
#include "Debug.hpp"
#include "DirectText.hpp"
#include "Error.hpp"
#include "HandleHandling.hpp"
#include "MathMacros.hpp"
//#include "MathSpecial.hpp"      // hack for checking strength
#include "OffscreenGWorld.hpp"
#include "Resources.h"
#include "SpriteCursor.hpp"  // for hint line
#include "StringHandling.hpp"

#define kScreenLabelError       "\pSCLB"
#define kUseLabels

#define kLabelBufferLeft        4
#define kLabelBufferRight       4
#define kLabelBufferTop         4
#define kLabelBufferBottom      4
#define kLabelInnerSpace        3
#define kLabelTotalInnerSpace   (kLabelInnerSpace<<1)

extern aresGlobalType   *gAresGlobal;
extern Handle           gDirectTextData;
extern long             gWhichDirectText, CLIP_LEFT, CLIP_TOP, CLIP_RIGHT,
                        CLIP_BOTTOM,
                        WORLD_WIDTH, WORLD_HEIGHT, gNatePortLeft, gNatePortTop; //temp hack?
extern directTextType   *gDirectText;
extern  GWorldPtr       gOffWorld, gRealWorld, gSaveWorld;
extern  PixMapHandle    thePixMapHandle;
extern  Handle          gColorTranslateTable, gSpaceObjectData;

//Handle    gAresGlobal->gScreenLabelData = nil;

// local function prototypes
static long String_Count_Lines( StringPtr s);

static StringPtr String_Get_Nth_Line( StringPtr dest, StringPtr source, long nth);
static void Auto_Animate_Line( Point *source, Point *dest);

int ScreenLabelInit( void)

{
#ifdef kUseLabels
    gAresGlobal->gScreenLabelData = NewHandle( sizeof( screenLabelType) * (long)kMaxLabelNum);
    if ( gAresGlobal->gScreenLabelData == nil)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, MEMORY_ERROR, -1, -1, -1, __FILE__, 1);
        return( MEMORY_ERROR);
    }

    /*
    MoveHHi( gAresGlobal->gScreenLabelData);
    HLock( gAresGlobal->gScreenLabelData);
    */
    mHandleLockAndRegister( gAresGlobal->gScreenLabelData, nil, nil, ResolveScreenLabels, "\pgAresGlobal->gScreenLabelData")

    ResetAllLabels();
    return( kNoError);
#else
    return( kNoError);
#endif
}

void ResetAllLabels( void)

{
#ifdef kUseLabels
    screenLabelType *label;
    short           i;

    label = (screenLabelType *)*gAresGlobal->gScreenLabelData;

    for ( i = 0; i < kMaxLabelNum; i++)
    {
        MacSetRect( &(label->thisRect), 0, 0, -1, -1);
        label->lastRect = label->thisRect;
        label->label[0] = 0;
        label->active = FALSE;
        label->killMe = FALSE;
        label->whichObject = kNoShip;
        label->object = nil;
        label->visibleState = -1;
        label->age = 0;
        label->objectLink = TRUE;
        label->lineNum = 1;
//      label->width = label->height = 0;
        label->keepOnScreenAnyway = false;
        label->attachedHintLine = false;
        label->retroCount = -1;
        label++;
    }
#endif
}

void ScreenLabelCleanup( void)

{
#ifdef kUseLabels
    if ( gAresGlobal->gScreenLabelData != nil) DisposeHandle( gAresGlobal->gScreenLabelData);
#endif
}

short AddScreenLabel( short h, short v, short hoff, short voff, const unsigned char *string,
                    spaceObjectType *object, Boolean objectLink, unsigned char color)

{
#ifdef kUseLabels
    short           whichLabel = 0;
    screenLabelType *label;
    long            strlen, lineNum, maxWidth, i;
    unsigned char   *getwidchar, *getwidwid;
    Str255          tString;

    label = (screenLabelType *)*gAresGlobal->gScreenLabelData;
    while (( whichLabel < kMaxLabelNum) && ( label->active)) { label++; whichLabel++;}
    if ( whichLabel >= kMaxLabelNum) return ( -1);  // no free label

    label->active = TRUE;
    label->killMe = FALSE;
    label->where.h = h;
    label->where.v = v;
    label->offset.h = hoff;
    label->offset.v = voff;
    label->color = color;
    label->object = object;
    label->objectLink = objectLink;
    label->keepOnScreenAnyway = false;
    label->attachedHintLine = false;
    if ( objectLink)
    {
        label->whichObject = object->entryNumber;
        if ( label->object == nil)
            label->visibleState = -1;
        else label->visibleState = 1;
    } else
    {
        label->visibleState = 1;
    }

    if ( string != nil)
    {
        CopyPString( (unsigned char *)label->label, (unsigned char *)string);
        RecalcScreenLabelSize( whichLabel);

/*      mSetDirectFont( kTacticalFontNum)
        lineNum = String_Count_Lines( label->label);

        if ( lineNum > 1)
        {
            label->lineNum = lineNum;
            maxWidth = 0;
            for ( i = 1; i <= lineNum; i++)
            {
                String_Get_Nth_Line( tString, label->label, i);
                mGetDirectStringDimensions( tString, label->width, label->height, strlen, getwidchar, getwidwid)
                if ( label->width > maxWidth)
                    maxWidth = label->width;
            }
            label->width = maxWidth;
            label->lineHeight = label->height;
            label->height = label->height * lineNum;
        } else
        {
            label->lineNum = 1;
            mGetDirectStringDimensions( label->label, label->width, label->height, strlen, getwidchar, getwidwid)
            label->lineHeight = label->height;
        }
*/
    } else
    {
        *(label->label) = 0;
        label->lineNum = label->lineHeight = label->width = label->height = 0;
    }

    return ( whichLabel);
#else
    return( -1);
#endif
}

void RemoveScreenLabel( long which)

{
#ifdef kUseLabels
    screenLabelType *label;

    label = (screenLabelType *)*gAresGlobal->gScreenLabelData + which;

    MacSetRect( &(label->thisRect), 0, 0, -1, -1);
    label->lastRect = label->thisRect;
    label->label[0] = 0;
    label->active = FALSE;
    label->killMe = FALSE;
    label->object = nil;
    label->width = label->height = label->lineNum = label->lineHeight = 0;
#endif
}

void EraseAllLabels( void)

{
#ifdef kUseLabels
    short           i = 0;
    screenLabelType *label;
    PixMapHandle    savePixBase, offPixBase;

    savePixBase = GetGWorldPixMap( gSaveWorld);
    offPixBase = GetGWorldPixMap( gOffWorld);
    label = (screenLabelType *)*gAresGlobal->gScreenLabelData;
    for ( i = 0; i < kMaxLabelNum; i++)
    {
        if (( label->active) && ( label->visibleState >= 0))
        {
            if (( label->thisRect.right > label->thisRect.left) &&
                ( label->thisRect.bottom > label->thisRect.top))
            {
            //  ChunkCopyPixMapToPixMap( *savePixBase, &(label->thisRect), *offPixBase);
                ChunkErasePixMap( *offPixBase, &(label->thisRect));
            }
            if ( label->killMe)
                label->lastRect = label->thisRect;
        }
        label++;
    }
#endif
}

void DrawAllLabels( void)

{
#ifdef kUseLabels
    short           i = 0, originalLength;
    screenLabelType *label;
    PixMapHandle    offPixBase;
    longRect        clipRect, tRect;
    unsigned char   color;//, *getwidchar, *getwidwid;
    transColorType  *transColor;
//  long            width, height, strlen;

    mSetDirectFont( kTacticalFontNum)

    label = (screenLabelType *)*gAresGlobal->gScreenLabelData;
    SetLongRect( &clipRect, CLIP_LEFT, CLIP_TOP, CLIP_RIGHT, CLIP_BOTTOM);
    offPixBase = GetGWorldPixMap( gOffWorld);
    for ( i = 0; i < kMaxLabelNum; i++)
    {
        if (( label->active) && ( !label->killMe) && ( *(label->label) > 0) && ( label->visibleState > 0))
        {
//          mGetDirectStringDimensions( label->label, width, height, strlen, getwidchar, getwidwid)

            label->thisRect.left = label->where.h;
            label->thisRect.right = label->where.h + label->width;
            label->thisRect.top = label->where.v;
            label->thisRect.bottom = label->where.v + label->height;
            if ( label->thisRect.left < CLIP_LEFT) label->thisRect.left = CLIP_LEFT;
            if ( label->thisRect.right > CLIP_RIGHT) label->thisRect.right = CLIP_RIGHT;
            if ( label->thisRect.top < CLIP_TOP) label->thisRect.top = CLIP_TOP;
            if ( label->thisRect.bottom > CLIP_BOTTOM) label->thisRect.bottom = CLIP_BOTTOM;
            if (( label->thisRect.right > label->thisRect.left) &&
                ( label->thisRect.bottom > label->thisRect.top))
            {

                RectToLongRect( &(label->thisRect), &tRect);
                mGetTranslateColorShade( label->color, VERY_DARK, color, transColor)

                if ( label->keepOnScreenAnyway)
                {
                    longRect    tc;

                    SetLongRect( &tc, 0, 0, WORLD_WIDTH, WORLD_HEIGHT);
//                  DrawNateLine( *offPixBase, &tc, label->where.h, label->where.v,
//                      10, 10, 0, 0, 0);
                }
                originalLength = label->label[0];
                if ( label->retroCount >= 0)
                {
                    label->retroCount += 3;
                    if ( label->retroCount > originalLength)
                    {
                        label->retroCount = -1;
                        label->label[0] = originalLength;
                    } else
                    {
                        label->label[0] = label->retroCount;
                        PlayVolumeSound(  kTeletype, kMediumLowVolume, kShortPersistence, kLowPrioritySound);
                    }
                }
                if ( label->lineNum > 1)
                {
                    long    j, y;
                    Str255  s;

                    DrawNateRectVScan( *offPixBase, &tRect, 0, 0, color);
                    mGetTranslateColorShade( label->color, VERY_LIGHT, color, transColor)
                    y = label->where.v + gDirectText->ascent + kLabelInnerSpace;
                    for ( j = 1; j <= label->lineNum; j++)
                    {
                        String_Get_Nth_Line( s, label->label, j);
                        MoveTo( label->where.h+1+kLabelInnerSpace, y+1);
                        DrawDirectTextStringClipped( s, BLACK, *offPixBase, &clipRect,
                            0, 0);
                        MoveTo( label->where.h-1+kLabelInnerSpace, y-1);
                        DrawDirectTextStringClipped( s, BLACK, *offPixBase, &clipRect,
                            0, 0);
                        MoveTo( label->where.h+kLabelInnerSpace, y);
                        DrawDirectTextStringClipped( s, color, *offPixBase, &clipRect,
                            0, 0);
                        y += label->lineHeight;
                    }

                } else
                {
                    DrawNateRectVScan( *offPixBase, &tRect, 0, 0, color);
                    mGetTranslateColorShade( label->color, VERY_LIGHT, color, transColor)
                    MoveTo( label->where.h+1+kLabelInnerSpace, label->where.v +
                        gDirectText->ascent +1 + kLabelInnerSpace);
                    DrawDirectTextStringClipped( label->label, BLACK, *offPixBase, &clipRect,
                        0, 0);
                    MoveTo( label->where.h + kLabelInnerSpace,
                        label->where.v + gDirectText->ascent + kLabelInnerSpace);
                    DrawDirectTextStringClipped( label->label, color, *offPixBase, &clipRect,
                        0, 0);
                }
                label->label[0] = originalLength;

            }
        } else label->thisRect.left = label->thisRect.right = 0;
        label++;
    }
#endif
}

void ShowAllLabels( void)

{
#ifdef kUseLabels
    Rect            tRect;
    short           i = 0;
    screenLabelType *label;
    PixMapHandle    pixMap;

    label = (screenLabelType *)*gAresGlobal->gScreenLabelData;
    pixMap = GetGWorldPixMap( gOffWorld);
    for ( i = 0; i < kMaxLabelNum; i++)
    {
        if (( label->active) && ( label->visibleState >= 0))
        {
            tRect = label->thisRect;
            if ( ( tRect.right <= tRect.left) || ( tRect.bottom <= tRect.top) ||
                ( label->lastRect.right <= label->lastRect.left) ||
                ( label->lastRect.bottom <= label->lastRect.top) ||
                (ABS( tRect.left - label->lastRect.left) > ( (tRect.right - tRect.left) * 4)) ||
                (ABS( tRect.top - label->lastRect.top) > (( tRect.bottom - tRect.top) * 4)))
            {
                if ( !(( tRect.right <= tRect.left) || ( tRect.bottom <= tRect.top)))
                    ChunkCopyPixMapToScreenPixMap( *pixMap, &tRect, *thePixMapHandle);
                if ( !(( label->lastRect.right <= label->lastRect.left) || ( label->lastRect.bottom <= label->lastRect.top)))
                    ChunkCopyPixMapToScreenPixMap( *pixMap, &(label->lastRect),
                            *thePixMapHandle);

                if ( label->keepOnScreenAnyway)
                {
                    longRect    tc;

                    SetLongRect( &tc, 0, 0, WORLD_WIDTH, WORLD_HEIGHT);
//                  CopyNateLine( *pixMap, *thePixMapHandle,
//                      &tc, label->where.h, label->where.v,
//                      10, 10, gNatePortLeft << 2, gNatePortTop);
                }
            } else

            {
                BiggestRect( &tRect, &(label->lastRect));
                ChunkCopyPixMapToScreenPixMap( *pixMap, &tRect, *thePixMapHandle);
            }
            label->lastRect = label->thisRect;
            if ( label->killMe)
                label->active = FALSE;
            if ( label->visibleState == 0) label->visibleState = -1;
        }
        label++;
    }
#endif
}

void SetScreenLabelPosition( long which, short h, short v)

{
#ifdef kUseLabels
    screenLabelType *label;

    label = (screenLabelType *)*gAresGlobal->gScreenLabelData + which;
    label->where.h = h + label->offset.h;
    label->where.v = v + label->offset.v;
#endif
}

void UpdateAllLabelPositions( long unitsDone)

{
#ifdef kUseLabels
    short           i = 0;
    screenLabelType *label;
    anyCharType     nilLabel = 0;
    Boolean         isOffScreen = FALSE;
    Point           source, dest;

    label = (screenLabelType *)*gAresGlobal->gScreenLabelData;
    for ( i = 0; i < kMaxLabelNum; i++)
    {
        if (( label->active) && ( !label->killMe))
        {
            if (( label->object != nil) && ( label->object->sprite != nil))
            {
                if ( label->object->active)
                {
                    isOffScreen = FALSE;
                    label->where.h = label->object->sprite->where.h + label->offset.h;

                    if ( label->where.h < ( CLIP_LEFT + kLabelBufferLeft))
                    {
                        isOffScreen = TRUE;
                        label->where.h = CLIP_LEFT + kLabelBufferLeft;
                    }
                    if ( label->where.h > ( CLIP_RIGHT - kLabelBufferRight - label->width))
                    {
                        isOffScreen = TRUE;
                        label->where.h = CLIP_RIGHT - kLabelBufferRight - label->width;
                    }


                    label->where.v = label->object->sprite->where.v + label->offset.v;

                    if ( label->where.v < (CLIP_TOP + kLabelBufferTop))
                    {
                        isOffScreen = TRUE;
                        label->where.v = CLIP_TOP + kLabelBufferTop;
                    }
                    if ( label->where.v > (CLIP_BOTTOM - kLabelBufferBottom - label->height))
                    {
                        isOffScreen = TRUE;
                        label->where.v = CLIP_BOTTOM - kLabelBufferBottom - label->height;
                    }

                    if ( !(label->object->seenByPlayerFlags & ( 1 << gAresGlobal->gPlayerAdmiralNumber)))
                    {
                        isOffScreen = true;
                    }

                    if ( !label->keepOnScreenAnyway)
                    {
                        if ( isOffScreen)
                        {
                            if ( label->age == 0) label->age = -kLabelOffVisibleTime;
                        } else if ( label->age < 0)
                        {
                            label->age = 0;
                            label->visibleState = 1;
                        }
                    }
                    if (( label->attachedHintLine) && ( label->label[0] != 0))
                    {
                        dest = label->attachedToWhere = label->object->sprite->where;
                        source.h = label->where.h + (label->width >> 2);

                        if ( label->attachedToWhere.v < label->where.v)
                        {
                            source.v = label->where.v - 2;
                        } else
                        {
                            source.v = label->where.v + label->height + 2;
                        }
                        Auto_Animate_Line( &source, &dest);
                        ShowHintLine(  source, dest,
                            label->color, DARK);
                    }
                } else
                {
                    SetScreenLabelString( i, &nilLabel);
                    if ( label->attachedHintLine)
                    {
                        HideHintLine();
                    }
                }

            } else if ( label->keepOnScreenAnyway)
            {
                    if ( label->where.h < ( CLIP_LEFT + kLabelBufferLeft))
                    {
                        label->where.h = CLIP_LEFT + kLabelBufferLeft;
                    }
                    if ( label->where.h > ( CLIP_RIGHT - kLabelBufferRight - label->width))
                    {
                        label->where.h = CLIP_RIGHT - kLabelBufferRight - label->width;
                    }

                    if ( label->where.v < (CLIP_TOP + kLabelBufferTop))
                    {
                        label->where.v = CLIP_TOP + kLabelBufferTop;
                    }
                    if ( label->where.v > (CLIP_BOTTOM - kLabelBufferBottom - label->height))
                    {
                        label->where.v = CLIP_BOTTOM - kLabelBufferBottom - label->height;
                    }
                    if (( label->attachedHintLine) && ( label->label[0] != 0))
                    {

                        dest = label->attachedToWhere;

                        source.v = label->where.v + (label->height / 2);

                        if ( label->attachedToWhere.h < label->where.h)
                        {
                            source.h = label->where.h - 2;
                        } else
                        {
                            source.h = label->where.h + label->width + 2;
                        }
                        Auto_Animate_Line( &source, &dest);
                        ShowHintLine(  source, dest,
                            label->color, VERY_LIGHT);
                    }
            }
            if ( label->age > 0)
            {
                label->age -= unitsDone;
                if ( label->age <= 0)
                {
                    label->visibleState = 0;
                    label->age = 0;
                    label->object = nil;
                    label->label[0] = 0;
                    if ( label->attachedHintLine)
                    {
                        HideHintLine();
                    }
                }
            } else if ( label->age < 0)
            {
                label->age += unitsDone;
                if ( label->age >= 0)
                {
                    label->age = 0;
                    label->visibleState = 0;
                }
            }
        }
        label++;
    }
#endif
}

void SetScreenLabelObject( long which, spaceObjectType *object)

{
#ifdef kUseLabels
    screenLabelType *label;
//  smallFixedType  f;

    label = (screenLabelType *)*gAresGlobal->gScreenLabelData + which;
    label->object = object;

    if ( label->object != nil)
    {
//      WriteDebugDivider();
//      WriteDebugSmallFixed( object->currentTargetValue);
//      f = HackGetObjectStrength( object);
//      WriteDebugLong( object->duty);

        label->age = 0;
        label->visibleState = 1;
        label->whichObject = object->entryNumber;
    } else
    {
        label->visibleState = -1;
        label->age = 0;
        label->whichObject = kNoShip;
    }
#endif
}

void SetScreenLabelAge( long which, long age)

{
#ifdef kUseLabels
    screenLabelType *label;

    label = (screenLabelType *)*gAresGlobal->gScreenLabelData + which;
    label->age = age;
        label->visibleState = 1;
//  else label->visibleState = 0;
#endif
}

void SetScreenLabelString( long which, const unsigned char *string)

{
#ifdef kUseLabels
    screenLabelType *label;
    unsigned char   *getwidchar, *getwidwid;
    long            strlen;

    label = (screenLabelType *)*gAresGlobal->gScreenLabelData + which;
    if ( string == nil)
    {
        *(label->label) = 0;
        label->width = label->height = 0;
    } else
    {
        CopyPString( (unsigned char *)label->label, (unsigned char *)string);
//      mSetDirectFont( kTacticalFontNum)
//      mGetDirectStringDimensions( label->label, label->width, label->height, strlen, getwidchar, getwidwid)
        RecalcScreenLabelSize( which);
    }
#endif
}

void SetScreenLabelColor( long which, unsigned char color)

{
#ifdef kUseLabels
    screenLabelType *label;

    label = (screenLabelType *)*gAresGlobal->gScreenLabelData + which;
    label->color = color;
#endif
}

void SetScreenLabelKeepOnScreenAnyway( long which, Boolean keepOnScreenAnyway)

{
#ifdef kUseLabels
    screenLabelType *label;

    label = (screenLabelType *)*gAresGlobal->gScreenLabelData + which;
    label->keepOnScreenAnyway = keepOnScreenAnyway;
    label->retroCount = 0;
#endif
}

void SetScreenLabelAttachedHintLine( long which, Boolean attachedHintLine, Point toWhere)

{
#ifdef kUseLabels
    screenLabelType *label;

    label = (screenLabelType *)*gAresGlobal->gScreenLabelData + which;
    if ( label->attachedHintLine) HideHintLine();
    label->attachedHintLine = attachedHintLine;
    label->attachedToWhere = toWhere;
    label->retroCount = 0;
#endif
}

void SetScreenLabelOffset( long which, long hoff, long voff)
{
#ifdef kUseLabels
    screenLabelType *label;

    label = (screenLabelType *)*gAresGlobal->gScreenLabelData + which;
    label->offset.h = hoff;
    label->offset.v = voff;

#endif
}

long GetScreenLabelWidth( long which)
{
#ifdef kUseLabels
    screenLabelType *label;

    label = (screenLabelType *)*gAresGlobal->gScreenLabelData + which;

    return ( label->width);

#endif
}

anyCharType *GetScreenLabelStringPtr( long which)
{
#ifdef kUseLabels
    screenLabelType *label;

    label = (screenLabelType *)*gAresGlobal->gScreenLabelData + which;
    return( label->label);
#else
    return( nil);
#endif
}

void RecalcScreenLabelSize( long which) // do this if you mess with its string
{
#ifdef kUseLabels
    screenLabelType *label;
    unsigned char   *getwidchar, *getwidwid;
    long            strlen, lineNum, i, maxWidth;
    Str255          tString;

    label = (screenLabelType *)*gAresGlobal->gScreenLabelData + which;
    mSetDirectFont( kTacticalFontNum)
//  mGetDirectStringDimensions( label->label, label->width, label->height, strlen, getwidchar, getwidwid)

        lineNum = String_Count_Lines( label->label);

        if ( lineNum > 1)
        {
            label->lineNum = lineNum;
            maxWidth = 0;
            for ( i = 1; i <= lineNum; i++)
            {
                String_Get_Nth_Line( tString, label->label, i);
                mGetDirectStringDimensions( tString, label->width, label->height, strlen, getwidchar, getwidwid)
                label->width += kLabelTotalInnerSpace;
                if ( label->width > maxWidth)
                    maxWidth = label->width;
            }
            label->width = maxWidth;
            label->lineHeight = label->height;
            label->height = label->height * lineNum;
            label->height += kLabelTotalInnerSpace;
        } else
        {
            label->lineNum = 1;
            mGetDirectStringDimensions( label->label, label->width, label->height, strlen, getwidchar, getwidwid)
            label->width += kLabelTotalInnerSpace;
            label->lineHeight = label->height;
            label->height += kLabelTotalInnerSpace;
        }

#endif
}

// for handle handling callback
void ResolveScreenLabels( Handle labelData)

{
    short   i;
    screenLabelType *label;

//  WriteDebugLine((char *)"\pLabel CB");

    label = (screenLabelType *)*labelData;

    for ( i = 0; i < kMaxLabelNum; i++)
    {
        if (( label->object != nil) && ( label->whichObject != kNoShip))
            label->object = (spaceObjectType *)*gSpaceObjectData + label->whichObject;
        label++;
    }
}

// String_Count_Lines
//  9/99
//
//  for emergency support of multi-line labels for on screen help

static long String_Count_Lines( StringPtr s)
{
    long    len, i = 0, result = 1;

    if ( s == nil) return 0;
    len = s[0];

    while ( i < len)
    {
        i++;
        if ( s[i] == '\r') result++;
    }
    return result;
}

static StringPtr String_Get_Nth_Line( StringPtr dest, StringPtr source, long nth)
{
    long    len, i = 1, lineNum = 1;

    if (( source == nil) || ( dest == nil)) return dest;
    dest[0] = 0;
    len = source[0];
    if ( len == 0) return dest;
    while ( nth > lineNum)
    {
        if ( i > len) return dest;
        if ( source[i] == '\r') lineNum++;
        i++;
    }
    while (( source[i] != '\r') && ( i <= len))
    {
        dest[0] += 1;
        dest[dest[0]] = source[i];
        i++;
    }
    return dest;
}

static void Auto_Animate_Line( Point *source, Point *dest)
{
    switch( (gAresGlobal->gGameTime>>3) & 0x03)
    {
        case 0:
            dest->h = source->h + ((dest->h - source->h) >> 2);
            dest->v = source->v + ((dest->v - source->v) >> 2);
            break;

        case 1:
            dest->h = source->h + ((dest->h - source->h) >> 1);
            dest->v = source->v + ((dest->v - source->v) >> 1);
//          source->h = source->h + ((dest->h - source->h) >> 2);
//          source->v = source->v + ((dest->v - source->v) >> 2);
            break;

        case 2:
            dest->h = dest->h + (( source->h - dest->h) >> 2);
            dest->v = dest->v + (( source->v - dest->v) >> 2);
//          source->h = source->h + ((dest->h - source->h) >> 1);
//          source->v = source->v + ((dest->v - source->v) >> 1);
            break;

        case 3:
//          source->h = dest->h + (( source->h - dest->h) >> 2);
//          source->v = dest->v + (( source->v - dest->v) >> 2);
            break;
    }
}
