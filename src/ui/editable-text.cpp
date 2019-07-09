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

template <typename Iterator>
static bool is_word(Iterator begin, Iterator end, Iterator it) {
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
            return (*it).isalnum() && (*jt).isalnum();
    }
}

static pn::string::iterator next_para_beginning(
        pn::string::iterator it, pn::string::iterator end) {
    for (; it != end; ++it) {
        if (*it == pn::rune{'\n'}) {
            return ++it;
        }
    }
    return it;
}

static pn::string::iterator next_para_end(pn::string::iterator it, pn::string::iterator end) {
    for (++it; it != end; ++it) {
        if (*it == pn::rune{'\n'}) {
            break;
        }
    }
    return it;
}

static pn::string::reverse_iterator next_para_beginning(
        pn::string::reverse_iterator it, pn::string::reverse_iterator end) {
    for (++it; it != end; ++it) {
        if (*it == pn::rune{'\n'}) {
            break;
        }
    }
    return it;
}

static pn::string::reverse_iterator next_para_end(
        pn::string::reverse_iterator it, pn::string::reverse_iterator end) {
    for (; it != end; ++it) {
        if (*it == pn::rune{'\n'}) {
            return ++it;
        }
    }
    return it;
}

// Returns {new iterator, can go further?}
template <typename Iterator>
static std::pair<Iterator, bool> advance(
        Iterator begin, Iterator end, Iterator it, TextReceiver::OffsetUnit unit) {
    if (it == end) {
        return {it, false};
    }

    switch (unit) {
        case TextReceiver::LINE_GLYPHS:
        case TextReceiver::PARA_GLYPHS:
            if (*it == pn::rune{'\n'}) {
                return {it, false};
            }
            while ((++it != end) && ((*it).width() == 0)) {
            }
            return {it, true};

        case TextReceiver::GLYPHS:
            while ((++it != end) && ((*it).width() == 0)) {
            }
            return {it, true};

        case TextReceiver::WORDS:
            while (!is_word(begin, end, it)) {
                if (++it == end) {
                    return {end, false};
                }
            }
            for (; (it != end) && is_word(begin, end, it); ++it) {
            }
            return {it, true};

        case TextReceiver::LINES: return {end, false};

        case TextReceiver::PARA_BEGINNINGS: return {next_para_beginning(it, end), true};

        case TextReceiver::PARA_ENDS: return {next_para_end(it, end), true};
    }
}

template <typename Iterator>
static Iterator advance_by(
        Iterator begin, Iterator end, Iterator it, int by, TextReceiver::OffsetUnit unit) {
    std::pair<Iterator, bool> it_loop = {it, true};
    for (; (by > 0) && it_loop.second; --by) {
        it_loop = advance(begin, end, it_loop.first, unit);
    }
    return it_loop.first;
}

int EditableText::offset(int origin, int by, OffsetUnit unit) const {
    if (by > 0) {
        pn::string::iterator it{_text.data(), _text.size(), origin};
        return advance_by(_text.begin(), _text.end(), it, by, unit).offset();
    } else if (by < 0) {
        by = (by == INT_MIN) ? INT_MAX : -by;
        pn::string::reverse_iterator it{_text.data(), _text.size(), origin};
        return advance_by(_text.rbegin(), _text.rend(), it, by, unit).offset();
    } else {
        return origin;
    }
}

int EditableText::size() const { return _text.size(); }

EditableText::range<int> EditableText::selection() const { return _selection; }

EditableText::range<int> EditableText::mark() const { return _mark; }

pn::string_view EditableText::text(range<int> range) const {
    return _text.substr(range.begin, range.end - range.begin);
}

}  // namespace antares
