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

#ifndef ANTARES_MESSAGE_SCREEN_HPP_
#define ANTARES_MESSAGE_SCREEN_HPP_

// Message Screen.h

#include "sfz/SmartPtr.hpp"
#include "AnyChar.hpp"
#include "ColorTable.hpp"
#include "NateDraw.hpp"

namespace antares {

#define kMessageStringID        3100
#define kZoomStringOffset       1
#define kAutoPilotOnString      9
#define kAutoPilotOffString     10

#define kStatusLabelColor       AQUA
#define kStatusWarnColor        PINK

#define kMaxLineNumber          48

struct retroTextSpecType {
    sfz::scoped_ptr<std::string> text;
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
    unsigned char           stringMessage[kAnyCharPStringMaxLen];
    unsigned char           lastStringMessage[kAnyCharPStringMaxLen];
    bool                 newStringMessage;
    retroTextSpecType       retroTextSpec;
    bool                 labelMessage;
    bool                 lastLabelMessage;
    short                   labelMessageID;
};

class MessageData {
  public:
    MessageData(size_t size)
            : _first_char(0),
              _first_free(0),
              _data(new unsigned char[size]) { }

    int32_t _first_char;
    int32_t _first_free;
    sfz::scoped_array<unsigned char> _data;
};

int InitMessageScreen( void);
void MessageScreenCleanup( void);
void ClearMessage( void);
void AppendStringToMessage(const unsigned char*);
void StartMessage( void);
void EndMessage( void);
void StartLongMessage( short, short);
void StartStringMessage(unsigned char*);
void ClipToCurrentLongMessage( void);
void DrawCurrentLongMessage( long);
void EndLongMessage( void);
void AdvanceCurrentLongMessage( void);
void PreviousCurrentLongMessage( void);
void ReplayLastLongMessage( void);
void DrawMessageScreen( long);
void SetStatusString(const unsigned char *, bool, unsigned char);
void UpdateStatusString( void);
long DetermineDirectTextHeightInWidth( retroTextSpecType *, long);
void DrawDirectTextInRect( retroTextSpecType *, const Rect&, const Rect&, PixMap *, long, long);
void DrawRetroTextCharInRect( retroTextSpecType *, long, const Rect&, const Rect&, PixMap *, long, long);

}  // namespace antares

#endif // ANTARES_MESSAGE_SCREEN_HPP_
