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

#ifndef ANTARES_DATA_STRING_LIST_HPP_
#define ANTARES_DATA_STRING_LIST_HPP_

#include <pn/string>
#include <sfz/sfz.hpp>
#include <vector>

namespace antares {

class StringList {
  public:
    StringList(int id);

    ssize_t         index_of(pn::string_view result) const;
    size_t          size() const;
    pn::string_view at(size_t index) const;

  private:
    friend std::vector<pn::string> to_vector(StringList&& strl);
    std::vector<pn::string>        _strings;

    DISALLOW_COPY_AND_ASSIGN(StringList);
};

inline std::vector<pn::string> to_vector(StringList&& strl) {
    std::vector<pn::string> result;
    std::swap(result, strl._strings);
    return result;
}

}  // namespace antares

#endif  // ANTARES_DATA_STRING_LIST_HPP_
