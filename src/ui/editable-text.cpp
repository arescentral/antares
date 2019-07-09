// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2017 The Antares Authors
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

#include "ui/editable-text.hpp"

#include "drawing/styled-text.hpp"
#include "game/sys.hpp"

namespace antares {

EditableText::EditableText() : _text{}, _selection{0, 0}, _mark{-1, -1} {}

void EditableText::replace(range<int> replace, pn::string_view text) {
    _text.replace(replace.begin, replace.end - replace.begin, text);
    _selection = {replace.begin + text.size(), replace.begin + text.size()};
    _mark      = {-1, -1};
    update();
}

void EditableText::select(range<int> select) {
    _selection = select;
    _mark      = {-1, -1};
    update();
}

void EditableText::mark(range<int> mark) { _mark = mark; }

void EditableText::accept() { replace(_selection, "\n"); }

void EditableText::newline() { replace(_selection, "\n"); }

void EditableText::tab() { replace(_selection, "\t"); }

void EditableText::escape() {}

static bool is_word(
        pn::string::iterator begin, pn::string::iterator end, pn::string::iterator it) {
    if ((*it).isalnum()) {
        return true;
    } else if ((it == begin) || (it == end)) {
        return false;
    }

    // A single ' or . is part of a word if surrounded by alphanumeric characters on both sides.
    switch ((*it).value()) {
        default: return false;
        case '.':
        case '\'':
            auto jt = it++;
            --jt;
            return (it != end) && (*it).isalnum() && (*jt).isalnum();
    }
}

bool is_glyph_boundary(
        pn::string::iterator begin, pn::string::iterator end, pn::string::iterator it) {
    return (it == end) || ((*it).width() != 0);
}

bool is_word_start(pn::string::iterator begin, pn::string::iterator end, pn::string::iterator it) {
    return is_word(begin, end, it) && ((it == begin) || !is_word(begin, end, --it));
}

bool is_word_end(pn::string::iterator begin, pn::string::iterator end, pn::string::iterator it) {
    return !is_word(begin, end, it) && ((it == begin) || is_word(begin, end, --it));
}

bool is_paragraph_start(
        pn::string::iterator begin, pn::string::iterator end, pn::string::iterator it) {
    return (it == begin) || (*--it == pn::rune{'\n'});
}

bool is_paragraph_end(
        pn::string::iterator begin, pn::string::iterator end, pn::string::iterator it) {
    return (it == end) || (*it == pn::rune{'\n'});
}

bool is_start(
        pn::string::iterator begin, pn::string::iterator end, pn::string::iterator it,
        TextReceiver::OffsetUnit unit) {
    switch (unit) {
        case TextReceiver::GLYPHS: return is_glyph_boundary(begin, end, it);
        case TextReceiver::WORDS: return is_word_start(begin, end, it);
        case TextReceiver::LINES:
        case TextReceiver::PARAGRAPHS: return is_paragraph_start(begin, end, it);
    }
}

bool is_end(
        pn::string::iterator begin, pn::string::iterator end, pn::string::iterator it,
        TextReceiver::OffsetUnit unit) {
    switch (unit) {
        case TextReceiver::GLYPHS: return is_glyph_boundary(begin, end, it);
        case TextReceiver::WORDS: return is_word_end(begin, end, it);
        case TextReceiver::LINES:
        case TextReceiver::PARAGRAPHS: return is_paragraph_end(begin, end, it);
    }
}

int EditableText::offset(int origin, Offset offset, OffsetUnit unit) const {
    pn::string::iterator       it{_text.data(), _text.size(), origin};
    const pn::string::iterator begin = _text.begin(), end = _text.end();

    if ((offset < 0) && (it == begin)) {
        return 0;
    } else if ((offset > 0) && (it == end)) {
        return _text.size();
    }

    switch (offset) {
        case PREV_SAME: return this->offset(origin, PREV_START, PARAGRAPHS);
        case NEXT_SAME: return this->offset(origin, NEXT_END, PARAGRAPHS);

        case PREV_START:
            while (--it != begin) {
                if (is_start(begin, end, it, unit)) {
                    break;
                }
            }
            return it.offset();

        case PREV_END:
            while (--it != begin) {
                if (is_end(begin, end, it, unit)) {
                    break;
                }
            }
            return it.offset();

        case THIS_START:
            do {
                if (is_start(begin, end, it, unit)) {
                    break;
                }
            } while (--it != begin);
            return it.offset();

        case THIS_END:
            do {
                if (is_end(begin, end, it, unit)) {
                    break;
                }
            } while (++it != end);
            return it.offset();

        case NEXT_START:
            while (++it != end) {
                if (is_start(begin, end, it, unit)) {
                    break;
                }
            }
            return it.offset();

        case NEXT_END:
            while (++it != end) {
                if (is_end(begin, end, it, unit)) {
                    break;
                }
            }
            return it.offset();
    }
}

int EditableText::size() const { return _text.size(); }

EditableText::range<int> EditableText::selection() const { return _selection; }

EditableText::range<int> EditableText::mark() const { return _mark; }

pn::string_view EditableText::text(range<int> range) const {
    return _text.substr(range.begin, range.end - range.begin);
}

}  // namespace antares
