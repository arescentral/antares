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

#ifndef ANTARES_DATA_STRING_LIST_HPP_
#define ANTARES_DATA_STRING_LIST_HPP_

#include <vector>
#include <sfz/sfz.hpp>

namespace antares {

class StringList {
  public:
    StringList(int id);
    ~StringList();

    void clear();
    ssize_t index_of(const sfz::StringSlice& result) const;
    size_t size() const;
    const sfz::String& at(size_t index) const;

  private:
    std::vector<sfz::String*> _strings;

    DISALLOW_COPY_AND_ASSIGN(StringList);
};

void string_to_pstring(const sfz::String& src, unsigned char* dest);

}  // namespace antares

#endif // ANTARES_DATA_STRING_LIST_HPP_
