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
        : _prefix{prefix.copy()}, _suffix{suffix.copy()} {}

void EditableText::replace(range<int> replace, pn::string_view text) {
    pn::string new_text = styled_text().text().copy();
    new_text.replace(replace.begin + _prefix.size(), replace.end - replace.begin, text);
    int        cursor    = replace.begin + text.size() + _prefix.size();
    range<int> selection = {cursor, cursor};
    range<int> mark      = {-1, -1};
    update(new_text, selection, mark);
}

void EditableText::select(range<int> select) {
    range<int> selection = {select.begin + _prefix.size(), select.end + _prefix.size()};
    range<int> mark      = {-1, -1};
    update(styled_text().text(), selection, mark);
}

void EditableText::mark(range<int> mark) {
    range<int> selection = {styled_text().selection().first, styled_text().selection().second};
    mark.begin += _prefix.size();
    mark.end += _prefix.size();
    update(styled_text().text(), selection, mark);
}

void EditableText::accept() { replace(selection(), "\n"); }

void EditableText::newline() { replace(selection(), "\n"); }

void EditableText::tab() { replace(selection(), "\t"); }

void EditableText::escape() {}

int EditableText::offset(int origin, Offset offset, OffsetUnit unit) const {
    const int at = styled_text().offset(origin + _prefix.size(), offset, unit) - _prefix.size();
    return std::max(0, std::min(at, size()));
}

int EditableText::size() const {
    return styled_text().text().size() - _prefix.size() - _suffix.size();
}

EditableText::range<int> EditableText::selection() const {
    return {styled_text().selection().first - _prefix.size(),
            styled_text().selection().second - _prefix.size()};
}

EditableText::range<int> EditableText::mark() const {
    return {styled_text().mark().first - _prefix.size(),
            styled_text().mark().second - _prefix.size()};
}

pn::string_view EditableText::text(range<int> range) const {
    return styled_text().text().substr(range.begin + _prefix.size(), range.end - range.begin);
}

pn::string_view EditableText::text() const {
    return styled_text().text().substr(_prefix.size(), size());
}

}  // namespace antares
