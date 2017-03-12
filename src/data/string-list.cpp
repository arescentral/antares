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

#include "data/string-list.hpp"

#include <pn/array>
#include <pn/file>
#include <pn/value>
#include <sfz/sfz.hpp>

#include "data/pn.hpp"
#include "data/resource.hpp"

using std::vector;

namespace antares {

StringList::StringList(int id) {
    Resource  rsrc("strings", "pn", id);
    pn::value strings;
    if (!pn::parse(rsrc.data().open(), strings, nullptr)) {
        throw std::runtime_error(pn::format("Couldn't parse strings/{0}.pn", id).c_str());
    }
    pn::array_cref l = strings.as_array();
    for (pn::value_cref x : l) {
        pn::string_view s = x.as_string();
        _strings.push_back(s.copy());
    }
}

ssize_t StringList::index_of(pn::string_view result) const {
    for (size_t i = 0; i < size(); ++i) {
        if (at(i) == result) {
            return i;
        }
    }
    return -1;
}

size_t StringList::size() const { return _strings.size(); }

pn::string_view StringList::at(size_t index) const { return _strings.at(index); }

}  // namespace antares
