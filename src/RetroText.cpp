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
#include <limits>
#include <Quickdraw.h>
#include "ColorTranslation.hpp"
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

int hex_digit(char c) {
    assert(isxdigit(c));
    if (c >= 'a') {
        return c - 'a' + 10;
    } else if (c >= 'A') {
        return c - 'A' + 10;
    } else {
        return c - '0';
    }
}

}  // namespace

RetroText::RetroText(const char* data, size_t len, int font, uint8_t fore_color, uint8_t back_color)
        : _font(font) {
    const uint8_t original_fore_color = fore_color;
    const uint8_t original_back_color = back_color;
    transColorType* trans_color;

    for (size_t i = 0; i < len; ++i) {
        switch (data[i]) {
          case '\r':
            _chars.push_back(RetroChar('\r', LINE_BREAK, fore_color, back_color));
            break;

          case '_':
            // TODO(sfiera): replace use of "_" with e.g. "\_".
            _chars.push_back(RetroChar(' ', NONE, fore_color, back_color));
            break;

          case ' ':
            _chars.push_back(RetroChar(' ', WORD_BREAK, fore_color, back_color));
            break;

          case '\\':
            if (i >= len) {
                fprintf(stderr, "not enough input for special code.\n");
                exit(1);
            }
            ++i;
            switch (data[i]) {
              case 'i':
                std::swap(fore_color, back_color);
                break;

              case 'f':
                if (i + 2 >= len) {
                    fprintf(stderr, "not enough input for foreground code.\n");
                    exit(1);
                }
                mGetTranslateColorShade(
                        hex_digit(data[i + 1]), hex_digit(data[i + 2]), fore_color, trans_color);
                i += 2;
                break;

              case 'b':
                if (i + 2 >= len) {
                    fprintf(stderr, "not enough input for foreground code.\n");
                    exit(1);
                }
                mGetTranslateColorShade(
                        hex_digit(data[i + 1]), hex_digit(data[i + 2]), back_color, trans_color);
                i += 2;
                break;

              case 'r':
                fore_color = original_fore_color;
                back_color = original_back_color;
                break;

              case 't':
                _chars.push_back(RetroChar('\t', TAB, fore_color, back_color));
                break;

              case '\\':
                _chars.push_back(RetroChar('\\', NONE, fore_color, back_color));
                break;

              default:
                fprintf(stderr, "found bad special character '%c'.\n", data[i]);
                exit(1);
            }
            break;

          default:
            _chars.push_back(RetroChar(data[i], NONE, fore_color, back_color));
            break;
        }
    }

    wrap_to(std::numeric_limits<int>::max(), 0);
}

RetroText::~RetroText() {
}

void RetroText::wrap_to(int width, int line_spacing) {
    mSetDirectFont(_font);
    _width = width;
    _line_spacing = line_spacing;
    int h = 0;
    int v = 0;

    for (size_t i = 0; i < _chars.size(); ++i) {
        _chars[i].h = h;
        _chars[i].v = v;
        switch (_chars[i].special) {
          case NONE:
            h += char_width(_chars[i].character);
            if (h > _width) {
                v += mDirectFontHeight() + _line_spacing;
                h = move_word_down(i, v);
            }
            break;

          case TAB:
            if (h >= _width / 2) {
                // If we're already at or past the midpoint of the line, shift a line down.
                v += mDirectFontHeight() + _line_spacing;
            }
            h = _width / 2;
            break;

          case LINE_BREAK:
            h = 0;
            v += mDirectFontHeight() + _line_spacing;
            break;

          case WORD_BREAK:
            h += char_width(_chars[i].character);
            break;
        }
    }
    _height = v + mDirectFontHeight() + _line_spacing;
}

int RetroText::width() const {
    return _width;
}

int RetroText::height() const {
    return _height;
}

void RetroText::draw(PixMap* pix, const Rect& bounds) const {
    mSetDirectFont(_font);
    const int line_height = mDirectFontHeight() + _line_spacing;
    const int char_adjust = mDirectFontAscent() + _line_spacing;
    for (std::vector<RetroChar>::const_iterator it = _chars.begin(); it != _chars.end(); ++it) {
        Point corner(bounds.left + it->h, bounds.top + it->v);
        switch (it->special) {
          case NONE:
          case WORD_BREAK:
            {
                if (it->back_color != 0xFF) {
                    Rect char_rect(0, 0, char_width(it->character), line_height);
                    char_rect.offset(corner.h, corner.v);
                    DrawNateRect(pix, &char_rect, 0, 0, it->back_color);
                }
                MoveTo(corner.h, corner.v + char_adjust);
                unsigned char pstr[2] = {1, it->character};
                DrawDirectTextStringClipped(pstr, it->fore_color, pix, bounds, 0, 0);
            }
            break;

          case TAB:
            if (it->back_color != 0xFF) {
                if (it->h >= _width) {
                    // If the tab starts at or past the midpoint of the line, first draw one box to
                    // the end of the line.
                    Rect line_rect(0, 0, bounds.width() - it->h, line_height);
                    line_rect.offset(corner.h, corner.v);
                    DrawNateRect(pix, &line_rect, 0, 0, it->back_color);

                    // Then draw a box from the start of the next line to its midpoint.
                    Rect tab_rect(0, 0, bounds.width() / 2, line_height);
                    tab_rect.offset(bounds.left, corner.v + line_height);
                    DrawNateRect(pix, &tab_rect, 0, 0, it->back_color);
                } else {
                    // Otherwise, we just have to draw a box to the midpoint of the line.
                    Rect tab_rect(0, 0, bounds.width() / 2 - it->h, line_height);
                    tab_rect.offset(corner.h, corner.v);
                    DrawNateRect(pix, &tab_rect, 0, 0, it->back_color);
                }
            }
            break;

          case LINE_BREAK:
            if (it->back_color != 0xFF) {
                Rect line_rect(0, 0, bounds.width() - it->h, line_height);
                line_rect.offset(corner.h, corner.v);
                DrawNateRect(pix, &line_rect, 0, 0, it->back_color);
            }
            break;
        }
    }
}

int RetroText::move_word_down(int index, int v) {
    for (int i = index; i >= 0; --i) {
        switch (_chars[i].special) {
          case LINE_BREAK:
            return 0;

          case WORD_BREAK:
          case TAB:
            {
                if (_chars[i + 1].h == 0) {
                    return 0;
                }

                int h = 0;
                for (int j = i + 1; j <= index; ++j) {
                    _chars[j].h = h;
                    _chars[j].v = v;
                    h += char_width(_chars[j].character);
                }
                return h;
            }

          case NONE:
            break;
        }
    }
    return 0;
}

RetroText::RetroChar::RetroChar(
        char character, SpecialChar special, uint8_t fore_color, uint8_t back_color)
        : character(character),
          special(special),
          fore_color(fore_color),
          back_color(back_color),
          h(0),
          v(0) { }

}  // namespace antares
