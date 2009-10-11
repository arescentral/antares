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

#include "RetroText.hpp"

#include <algorithm>
#include <Quickdraw.h>
#include "DirectText.hpp"

namespace antares {

namespace {

/*
const char kReturnChar        = '\r';
const char kCodeChar          = '\\';
const char kCodeTabChar       = 't';
const char kCodeInvertChar    = 'i';
const char kCodeColorChar     = 'c';
const char kCodeRevertChar    = 'r';
const char kCodeForeColorChar = 'f';
const char kCodeBackColorChar = 'b';

struct retroTextSpecType {
    TypedHandle<unsigned char> text;
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
};

void DrawRetroTextCharInRect( retroTextSpecType *retroTextSpec, long charsToDo,
    Rect *bounds, Rect *clipRect, PixMap *destMap, long portLeft, long portTop)
{
    unsigned char   *thisChar = *(retroTextSpec->text), thisWord[kMaxRetroSize], charWidth, *widthPtr;
    Rect        cursorRect, lineRect, tlRect;
    long            oldx, wordLen, *lineLength = &(retroTextSpec->lineLength[retroTextSpec->lineCount]);
    unsigned char   tempColor, calcColor, calcShade;
    transColorType  *transColor;
    bool         drawCursor = ( charsToDo > 0);

    cursorRect.left = retroTextSpec->xpos;
    cursorRect.top = retroTextSpec->ypos -
        (mDirectFontAscent()  + retroTextSpec->topBuffer);
    cursorRect.right = cursorRect.left + gDirectText->logicalWidth;
    cursorRect.bottom = cursorRect.top + mDirectFontHeight() +
        retroTextSpec->topBuffer + retroTextSpec->bottomBuffer;
    mCopyAnyRect( tlRect, cursorRect);
    mClipAnyRect( tlRect, *bounds);
    if ( retroTextSpec->originalBackColor != WHITE)
        DrawNateRectClipped( destMap, &tlRect, clipRect, (portLeft << 2),
            portTop, retroTextSpec->originalBackColor);

    if ( charsToDo <= 0) charsToDo = retroTextSpec->lineLength[retroTextSpec->lineCount];

    while (( charsToDo > 0) && ( retroTextSpec->thisPosition <
        retroTextSpec->textLength))
    {
        thisChar = *(retroTextSpec->text) + retroTextSpec->thisPosition;
        if ( *thisChar == kCodeChar)
        {
            thisChar++;
            charsToDo--;
            (retroTextSpec->thisPosition)++;
            (retroTextSpec->linePosition)++;
            switch( *thisChar)
            {
                case kCodeTabChar:
                    wordLen = 0;
                    oldx = bounds->left;
                    cursorRect.left = retroTextSpec->xpos;
                    while ( oldx <= retroTextSpec->xpos)
                    {
                        oldx += retroTextSpec->tabSize;
                        wordLen++;
                    }
                    retroTextSpec->xpos = bounds->left + retroTextSpec->tabSize *
                        wordLen;
                    cursorRect.right = retroTextSpec->xpos;
                    mCopyAnyRect( tlRect, cursorRect);
                    mClipAnyRect( tlRect, *bounds);
                    if ( retroTextSpec->backColor != WHITE)
                        DrawNateRectClipped( destMap, &tlRect, clipRect, (portLeft << 2), portTop,
                            retroTextSpec->backColor);
                    break;

                case kCodeChar:
                    oldx = cursorRect.left = retroTextSpec->xpos;
                    cursorRect.top = retroTextSpec->ypos - (mDirectFontAscent() + retroTextSpec->topBuffer);
                    cursorRect.bottom = cursorRect.top + mDirectFontHeight() + retroTextSpec->topBuffer + retroTextSpec->bottomBuffer;
                    mDirectCharWidth( charWidth, *thisChar, widthPtr);
                    retroTextSpec->xpos += charWidth;
                    mCopyAnyRect( tlRect, cursorRect);
                    mClipAnyRect( tlRect, *bounds);
                    if ( retroTextSpec->backColor != WHITE)
                        DrawNateRectClipped( destMap, &tlRect, clipRect, (portLeft << 2),
                            portTop, retroTextSpec->backColor);
                    thisWord[0] = 1;
                    thisWord[1] = '\\';
                    cursorRect.right = retroTextSpec->xpos;
                    MoveTo( oldx, retroTextSpec->ypos);
                    DrawDirectTextStringClipped( thisWord,
                            (retroTextSpec->color==WHITE)?(BLACK):
                                (retroTextSpec->color),
                            destMap, clipRect, portLeft,
                            portTop);
                    break;

                case kCodeInvertChar:
                    tempColor = retroTextSpec->color;
                    retroTextSpec->nextColor = retroTextSpec->backColor;
                    retroTextSpec->nextBackColor = tempColor;
                    break;

                case kCodeForeColorChar:
                    thisChar++;
                    charsToDo--;
                    (retroTextSpec->thisPosition)++;
                    (retroTextSpec->linePosition)++;
                    calcColor = mHexDigitValue(*thisChar);
                    thisChar++;
                    charsToDo--;
                    (retroTextSpec->thisPosition)++;
                    (retroTextSpec->linePosition)++;
                    calcShade = mHexDigitValue(*thisChar);
                    mGetTranslateColorShade( calcColor, calcShade, retroTextSpec->nextColor, transColor);
                    break;

                case kCodeBackColorChar:
                    thisChar++;
                    charsToDo--;
                    (retroTextSpec->thisPosition)++;
                    (retroTextSpec->linePosition)++;
                    calcColor = mHexDigitValue(*thisChar);
                    thisChar++;
                    charsToDo--;
                    (retroTextSpec->thisPosition)++;
                    (retroTextSpec->linePosition)++;
                    calcShade = mHexDigitValue(*thisChar);
                    if (( calcColor) && (calcShade == 0))
                    {
                        retroTextSpec->nextBackColor = 0xff;
                    } else
                    {
                        mGetTranslateColorShade( calcColor, calcShade, retroTextSpec->nextBackColor, transColor);
                    }
                    break;

                case kCodeRevertChar:
                    retroTextSpec->nextColor = retroTextSpec->originalColor;
                    retroTextSpec->nextBackColor = retroTextSpec->originalBackColor;
                    break;
            }
            thisChar++;
            charsToDo--;
            (retroTextSpec->thisPosition)++;
            (retroTextSpec->linePosition)++;
        } else if ( *thisChar == kReturnChar)
        {
            thisChar++;
            (retroTextSpec->thisPosition)++;
            (retroTextSpec->linePosition)++;
            charsToDo--;
        } else
        {
            thisWord[0] = 1;
            if ( *thisChar == '_')
                thisWord[1] = ' ';
            else
                thisWord[1] = *thisChar;
//          thisWord[1] = ' ';

            retroTextSpec->color = retroTextSpec->nextColor;
            retroTextSpec->backColor = retroTextSpec->nextBackColor;
            cursorRect.left = retroTextSpec->xpos;
            MoveTo( retroTextSpec->xpos, retroTextSpec->ypos);
            mDirectCharWidth( charWidth, *thisChar, widthPtr);
            retroTextSpec->xpos += charWidth;
            cursorRect.right = retroTextSpec->xpos;
            mCopyAnyRect( tlRect, cursorRect);
            mClipAnyRect( tlRect, *bounds);
            if ( retroTextSpec->backColor != WHITE)
                DrawNateRectClipped( destMap, &tlRect, clipRect, (portLeft << 2), portTop,
                    retroTextSpec->backColor);
            DrawDirectTextStringClipped( thisWord,
                (retroTextSpec->color==WHITE)?(BLACK):(retroTextSpec->color),
                destMap, clipRect, portLeft, portTop);
            (retroTextSpec->thisPosition)++;
            (retroTextSpec->linePosition)++;
            charsToDo--;
        }
        if ( retroTextSpec->linePosition >= *lineLength)
        {
            lineRect.left = retroTextSpec->xpos;
            lineRect.right = bounds->right;
            lineRect.top = cursorRect.top;
            lineRect.bottom = cursorRect.bottom;
            mCopyAnyRect( tlRect, lineRect);
            mClipAnyRect( tlRect, *bounds);
            if ( retroTextSpec->backColor != WHITE)
                DrawNateRectClipped( destMap, &tlRect, clipRect, (portLeft << 2), portTop,
                    retroTextSpec->backColor);

            retroTextSpec->linePosition = 0;
            retroTextSpec->ypos += mDirectFontHeight() + retroTextSpec->topBuffer + retroTextSpec->bottomBuffer;
            retroTextSpec->xpos = bounds->left;
            (retroTextSpec->lineCount)++;
            lineLength++;
            cursorRect.top = retroTextSpec->ypos - (mDirectFontAscent() + retroTextSpec->topBuffer);
            cursorRect.bottom = cursorRect.top + mDirectFontHeight() + retroTextSpec->topBuffer + retroTextSpec->bottomBuffer;
        } else
        {
        }

    }
    if ( retroTextSpec->thisPosition < retroTextSpec->textLength)
    {
        cursorRect.left = retroTextSpec->xpos;
        cursorRect.right = cursorRect.left + gDirectText->logicalWidth;
        if ( drawCursor)
        {
            mCopyAnyRect( tlRect, cursorRect);
            mClipAnyRect( tlRect, *bounds);
            if ( retroTextSpec->backColor != WHITE)
                DrawNateRectClipped( destMap, &tlRect, clipRect, (portLeft << 2), portTop,
                    retroTextSpec->originalColor);
        }
    }
}
*/

int char_width(uint8_t ch) {
    uint8_t w;
    unsigned char* p;
    mDirectCharWidth(w, ch, p);
    return w;
}

int word_width(const std::string& str) {
    int sum = 0;
    for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
        sum += char_width(*it);
    }
    return sum;
}

// TODO(sfiera): replace use of "_" with e.g. "\_".
void split_lines(const std::string& in, std::vector<std::string>* out, int width) {
    std::vector<std::string> words;
    const char* start = in.c_str();
    const char* end = strchr(start, ' ');;
    while (end != NULL) {
        words.push_back(std::string(start, end - start));
        start = end + 1;
        end = strchr(start, ' ');
    }
    words.push_back(std::string(start, in.size() - (start - in.c_str())));

    const int space_width = char_width(' ');
    int line_width = 0;
    std::string line;
    for (std::vector<std::string>::const_iterator it = words.begin(); it != words.end(); ++it) {
        const int word_width = antares::word_width(*it);
        if (line.empty()) {
            line = *it;
            line_width = word_width;
        } else if (line_width + space_width + word_width <= width) {
            line += ' ' + *it;
            line_width += space_width + word_width;
        } else if (!it->empty()) {
            out->push_back(line);
            line = *it;
            line_width = word_width;
        }
    }
    out->push_back(line);
}

}  // namespace

RetroText::RetroText(const char* data, size_t len, int font, int fore_color, int back_color)
        : _text(data, len),
          _font(font),
          _fore_color(fore_color),
          _back_color(back_color) { }

RetroText::~RetroText() {
}

int RetroText::height_for_width(int width) {
    mSetDirectFont(_font);
    int line_height = mDirectFontHeight() + 2;
    std::vector<std::string> lines;
    split_lines(_text, &lines, width);
    return line_height * lines.size();
}

void RetroText::draw(PixMap* pix, const Rect& bounds) {
    mSetDirectFont(_font);
    int line_height = mDirectFontHeight() + 2;
    int y = mDirectFontAscent() + 2;
    std::vector<std::string> lines;
    split_lines(_text, &lines, bounds.width());
    for (std::vector<std::string>::const_iterator it = lines.begin(); it != lines.end(); ++it) {
        MoveTo(bounds.left, bounds.top + y);
        y += line_height;
        unsigned char pstr[255];
        pstr[0] = std::min<int>(255, it->size());
        memcpy(pstr + 1, it->c_str(), pstr[0]);
        Rect mutable_bounds = bounds;
        DrawDirectTextStringClipped(pstr, _fore_color, pix, &mutable_bounds, 0, 0);
    }
}

}  // namespace antares
