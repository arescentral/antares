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

#ifndef ANTARES_MESSAGE_SCREEN_HPP_
#define ANTARES_MESSAGE_SCREEN_HPP_

// Message Screen.h

#include "AnyChar.h"
#include "NateDraw.h"

#pragma options align=mac68k

#define kMessageStringID        3100
#define kZoomStringOffset       1
#define kAutoPilotOnString      9
#define kAutoPilotOffString     10

#define kStatusLabelColor       AQUA
#define kStatusWarnColor        PINK

#define kMaxLineNumber          48


typedef struct
{
    Handle          text;
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
    unsigned char   color;
    unsigned char   backColor;
    unsigned char   originalColor;
    unsigned char   originalBackColor;
    unsigned char   nextColor;
    unsigned char   nextBackColor;
} retroTextSpecType;

typedef enum
{
    kNoStage = 0,
    kStartStage = 1,
    kClipStage = 2,
    kShowStage = 3,
    kEndStage = 4
} longMessageStageType;

typedef struct
{
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
    anyCharType             stringMessage[kAnyCharPStringMaxLen];
    anyCharType             lastStringMessage[kAnyCharPStringMaxLen];
    Boolean                 newStringMessage;
    retroTextSpecType       retroTextSpec;
    Boolean                 labelMessage;
    Boolean                 lastLabelMessage;
    short                   labelMessageID;
} longMessageType;


int InitMessageScreen( void);
void MessageScreenCleanup( void);
void ClearMessage( void);
void AppendStringToMessage( anyCharType *);
void StartMessage( void);
void EndMessage( void);
void StartLongMessage( short, short);
void StartStringMessage( anyCharType *);
void ClipToCurrentLongMessage( void);
void DrawCurrentLongMessage( long);
void EndLongMessage( void);
void AdvanceCurrentLongMessage( void);
void PreviousCurrentLongMessage( void);
void ReplayLastLongMessage( void);
void DrawMessageScreen( long);
void SetStatusString( anyCharType *, Boolean, unsigned char);
void UpdateStatusString( void);
long DetermineDirectTextHeightInWidth( retroTextSpecType *, long);
void DrawDirectTextInRect( retroTextSpecType *, longRect *, longRect *, PixMap *, long, long);
void DrawRetroTextCharInRect( retroTextSpecType *, long, longRect *, longRect *, PixMap *, long, long);

#pragma options align=reset

#endif // ANTARES_MESSAGE_SCREEN_HPP_
