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

#ifndef ANTARES_SCREEN_LABEL_HPP_
#define ANTARES_SCREEN_LABEL_HPP_

// Screen Label.h

#include "AnyChar.hpp"
#include "SpaceObject.hpp"

#define kNoLabel        -1
#define kMaxLabelLen            250
#define kMaxLabelNum            16

#define kLabelOffVisibleTime    60

struct screenLabelType {
    Point           where;
    Point           offset;
    Rect            thisRect;
    Rect            lastRect;
    long            width;
    long            height;
    long            age;
    unsigned char   label[kMaxLabelLen];
    unsigned char   color;
    bool         active;
    bool         killMe;
    short           visibleState;   // < 0 = invisible 0 = turn me invisible, 1 = visible
    long            whichObject;
    spaceObjectType *object;
    bool         objectLink;     // true if label requires an object to be seen
    long            lineNum;
    long            lineHeight;
    bool         keepOnScreenAnyway; // if not attached to object, keep on screen if it's off
    bool         attachedHintLine;
    Point           attachedToWhere;
    long            retroCount;
};


int ScreenLabelInit( void);
void ResetAllLabels( void);
void ScreenLabelCleanup( void);
short AddScreenLabel( short, short, short, short, const unsigned char *, spaceObjectType *,
                    bool, unsigned char);
void RemoveScreenLabel( long);
void EraseAllLabels( void);
void DrawAllLabels( void);
void ShowAllLabels( void);
void SetScreenLabelPosition( long, short, short);
void UpdateAllLabelPositions( long);
void SetScreenLabelObject( long, spaceObjectType *);
void SetScreenLabelAge( long, long);
void SetScreenLabelString( long, const unsigned char *);
void SetScreenLabelColor( long, unsigned char);
void SetScreenLabelOffset( long which, long hoff, long voff);
long GetScreenLabelWidth( long which);
void SetScreenLabelKeepOnScreenAnyway( long which, bool keepOnScreenAnyWay);
void SetScreenLabelAttachedHintLine( long which, bool attachedHintLine, Point toWhere);
unsigned char* GetScreenLabelStringPtr(long);
void RecalcScreenLabelSize( long);

#endif // ANTARES_SCREEN_LABEL_HPP_
