// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2011 Ares Central
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
// License along with this program.  If not, see
// <http://www.gnu.org/licenses/>.

#ifndef ANTARES_GAME_LABELS_HPP_
#define ANTARES_GAME_LABELS_HPP_

#include <sfz/sfz.hpp>

#include "data/space-object.hpp"

namespace antares {

const int32_t kNoLabel = -1;
const int32_t kLabelOffVisibleTime = 60;

struct screenLabelType {
    Point               where;
    Point               offset;
    Rect                thisRect;
    long                width;
    long                height;
    long                age;
    sfz::String         text;
    unsigned char       color;
    bool                active;
    bool                killMe;
    bool                visible;
    long                whichObject;
    spaceObjectType*    object;
    bool                objectLink;     // true if label requires an object to be seen
    long                lineNum;
    long                lineHeight;
    bool                keepOnScreenAnyway; // if not attached to object, keep on screen if it's off
    bool                attachedHintLine;
    Point               attachedToWhere;
    int32_t             retroCount;

    screenLabelType();
};

void zero(screenLabelType& label);

void ScreenLabelInit();
void ResetAllLabels();
short AddScreenLabel(
        short h, short v, short hoff, short voff, spaceObjectType* object, bool objectLink,
        unsigned char color);
void RemoveScreenLabel( long);
void draw_labels();
void update_all_label_contents(int32_t units_done);
void update_all_label_positions(int32_t units_done);
void ShowAllLabels();
void SetScreenLabelPosition( long, short, short);
void SetScreenLabelObject( long, spaceObjectType *);
void SetScreenLabelAge( long, long);
void SetScreenLabelString(long which, const sfz::StringSlice& string);
void ClearScreenLabelString(long which);
void SetScreenLabelColor( long, unsigned char);
void SetScreenLabelOffset( long which, long hoff, long voff);
long GetScreenLabelWidth( long which);
void SetScreenLabelKeepOnScreenAnyway( long which, bool keepOnScreenAnyWay);
void SetScreenLabelAttachedHintLine( long which, bool attachedHintLine, Point toWhere);
sfz::String* GetScreenLabelStringPtr(long);  // TODO(sfiera): encapsulate.
void RecalcScreenLabelSize( long);

}  // namespace antares

#endif // ANTARES_GAME_LABELS_HPP_
