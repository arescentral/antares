// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2012 The Antares Authors
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

#ifndef ANTARES_GAME_MESSAGES_HPP_
#define ANTARES_GAME_MESSAGES_HPP_

#include <queue>
#include <sfz/sfz.hpp>

#include "drawing/color.hpp"
#include "drawing/retro-text.hpp"
#include "drawing/shapes.hpp"
#include "math/geometry.hpp"

namespace antares {

const int16_t kMessageStringID      = 3100;
const int16_t kZoomStringOffset     = 1;
const int16_t kAutoPilotOnString    = 9;
const int16_t kAutoPilotOffString   = 10;

const uint8_t kStatusLabelColor     = AQUA;
const uint8_t kStatusWarnColor      = PINK;

const size_t kMaxLineNumber = 48;

struct retroTextSpecType {
    sfz::scoped_ptr<sfz::String> text;
    long            textLength;
    long            lineLength[kMaxLineNumber];
    long            lineNumber;
    long            lineCount;
    long            linePosition;
    long            thisPosition;
    long            tabSize;
    long            xpos;
    long            ypos;
    long            autoHeight;
    long            autoWidth;
    long            topBuffer;
    long            bottomBuffer;
    RgbColor        color;
    RgbColor        backColor;
    RgbColor        originalColor;
    RgbColor        originalBackColor;
    RgbColor        nextColor;
    RgbColor        nextBackColor;
};

enum longMessageStageType {
    kNoStage = 0,
    kStartStage = 1,
    kClipStage = 2,
    kShowStage = 3,
    kEndStage = 4
};

struct longMessageType {
    longMessageStageType    stage;
    long                    charDelayCount;
    Rect                    pictBounds;
    long                    pictDelayCount;
    long                    pictCurrentLeft;
    long                    pictCurrentTop;
    long                    time;
    long                    textHeight;
    short                   startResID;
    short                   endResID;
    short                   currentResID;
    short                   lastResID;
    short                   previousStartResID;
    short                   previousEndResID;
    short                   pictID;
    unsigned char           backColor;
    sfz::String             stringMessage;
    sfz::String             lastStringMessage;
    bool                 newStringMessage;
    sfz::String             text;
    sfz::scoped_ptr<RetroText> retro_text;
    Point                   retro_origin;
    int32_t                 at_char;
    bool                 labelMessage;
    bool                 lastLabelMessage;
    short                   labelMessageID;
};

class MessageData {
    std::queue<sfz::linked_ptr<sfz::String> > queue;
};

void InitMessageScreen( void);
void MessageScreenCleanup( void);
void ClearMessage( void);
void AddMessage(const sfz::PrintItem& message);
void StartLongMessage( short, short);
void ClipToCurrentLongMessage( void);
void DrawCurrentLongMessage(int32_t time_pass);
void EndLongMessage( void);
void AdvanceCurrentLongMessage( void);
void PreviousCurrentLongMessage( void);
void ReplayLastLongMessage( void);
void DrawMessageScreen(int32_t by_units);
void SetStatusString(const sfz::StringSlice& status, unsigned char color);
long DetermineDirectTextHeightInWidth( retroTextSpecType *, long);
void draw_message();

}  // namespace antares

#endif // ANTARES_GAME_MESSAGES_HPP_
