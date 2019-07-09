// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2019 The Antares Authors
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

#ifndef ANTARES_UI_EDITABLE_TEXT_HPP_
#define ANTARES_UI_EDITABLE_TEXT_HPP_

#include <pn/string>
#include <vector>

#include "config/keys.hpp"
#include "data/base-object.hpp"
#include "game/cursor.hpp"
#include "ui/event.hpp"

namespace antares {

class EditableText : public TextReceiver {
    using TextReceiver::range;

  public:
    EditableText();

    virtual void replace(range<int> replace, pn::string_view text);
    virtual void select(range<int> select);
    virtual void mark(range<int> mark);
    virtual void accept();
    virtual void newline();
    virtual void tab();
    virtual void escape();

    virtual int             offset(int origin, Offset offset, OffsetUnit unit) const;
    virtual int             size() const;
    virtual range<int>      selection() const;
    virtual range<int>      mark() const;
    virtual pn::string_view text(range<int> range) const;
    pn::string_view         text() const { return _text; }

  protected:
    virtual void update() = 0;

  private:
    pn::string _text;
    range<int> _selection;
    range<int> _mark;
};

}  // namespace antares

#endif  // ANTARES_UI_EDITABLE_TEXT_HPP_
