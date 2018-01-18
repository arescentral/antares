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

#include <stdlib.h>
#include <pn/file>

#include "data/scenario-list.hpp"

namespace antares {

void main(int argc, char* const* argv) {
    if (argc != 1) {
        pn::format(stderr, "usage: ls-scenarios\n");
        exit(1);
    }

    ScenarioList list;
    for (size_t i = 0; i < list.size(); ++i) {
        if (!list.at(i).installed) {
            continue;
        }
        pn::format(stderr, "{0}:\n", list.at(i).identifier);
        pn::format(stderr, "    title: {0}\n", list.at(i).title);
        pn::format(stderr, "    download url: {0}\n", list.at(i).download_url);
        pn::format(stderr, "    author: {0}\n", list.at(i).author);
        pn::format(stderr, "    author url: {0}\n", list.at(i).author_url);
        pn::format(stderr, "    version: {0}\n", stringify(list.at(i).version));
    }
}

}  // namespace antares

int main(int argc, char* const* argv) {
    antares::main(argc, argv);
    return 0;
}
