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

#ifndef ANTARES_DATA_SCENARIO_LIST_HPP_
#define ANTARES_DATA_SCENARIO_LIST_HPP_

#include <pn/string>
#include <vector>

#include "data/pn.hpp"

namespace antares {

struct Version {
    std::vector<int> components;
};
pn::string stringify(const Version& v);
Version    u32_to_version(uint32_t in);

class ScenarioList {
  public:
    struct Entry {
        pn::string identifier;
        pn::string title;
        pn::string download_url;
        pn::string author;
        pn::string author_url;
        Version    version;
        bool       installed;
    };

    ScenarioList();
    ScenarioList(const ScenarioList&) = delete;
    ScenarioList& operator=(const ScenarioList&) = delete;

    size_t       size() const;
    const Entry& at(size_t index) const;

  private:
    std::vector<Entry> _scenarios;
};

}  // namespace antares

#endif  // ANTARES_DATA_SCENARIO_LIST_HPP_
