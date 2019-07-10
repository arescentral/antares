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

EditableText::EditableText(pn::string_view prefix, pn::string_view suffix)
        : _prefix{prefix.copy()},
          _suffix{suffix.copy()},
          _text{pn::format("{}{}", _prefix, _suffix)},
          _selection{prefix.size(), prefix.size()},
          _mark{-1, -1} {}

void EditableText::replace(range<int> replace, pn::string_view text) {
    _text.replace(replace.begin + _prefix.size(), replace.end - replace.begin, text);
    int cursor = replace.begin + text.size() + _prefix.size();
    _selection = {cursor, cursor};
    _mark      = {-1, -1};
    update();
}

void EditableText::select(range<int> select) {
    _selection = {select.begin + _prefix.size(), select.end + _prefix.size()};
    _mark      = {-1, -1};
    update();
}

void EditableText::mark(range<int> mark) {
    _mark = {mark.begin + _prefix.size(), mark.end + _prefix.size()};
}

void EditableText::accept() { replace(selection(), "\n"); }

void EditableText::newline() { replace(selection(), "\n"); }

void EditableText::tab() { replace(selection(), "\t"); }

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
    pn::string::iterator       it{text().data(), text().size(), origin};
    const pn::string::iterator begin = text().begin(), end = text().end();

    if ((offset < 0) && (it == begin)) {
        return 0;
    } else if ((offset > 0) && (it == end)) {
        return text().size();
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

int EditableText::size() const { return _text.size() - _prefix.size() - _suffix.size(); }

EditableText::range<int> EditableText::selection() const {
    return {_selection.begin - _prefix.size(), _selection.end - _prefix.size()};
}

EditableText::range<int> EditableText::mark() const {
    return {_mark.begin - _prefix.size(), _mark.end - _prefix.size()};
}

pn::string_view EditableText::text(range<int> range) const {
    return _text.substr(range.begin + _prefix.size(), range.end - range.begin);
}

pn::string_view EditableText::text() const { return _text.substr(_prefix.size(), size()); }

}  // namespace antares
