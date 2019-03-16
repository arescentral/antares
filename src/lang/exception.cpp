// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2018 The Antares Authors
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

#include "lang/exception.hpp"

#include <pn/file>
#include <pn/string>
#include <sfz/sfz.hpp>

namespace antares {

static void extend_exception_string(const std::exception& e, pn::string_ref what) {
    what += ": ";
    what += e.what();
    try {
        std::rethrow_if_nested(e);
    } catch (const std::exception& e) {
        extend_exception_string(e, what);
    }
}

pn::string full_exception_string(const std::exception& e) {
    pn::string what = e.what();
    try {
        std::rethrow_if_nested(e);
    } catch (const std::exception& e2) {
        extend_exception_string(e2, what);
    }
    return what;
}

int wrap_main(
        const std::function<void(int argc, char* const* argv)>& main, int argc,
        char* const* argv) {
    try {
        main(argc, argv);
    } catch (std::exception& e) {
        pn::file_view{stderr}.format(
                "{0}: {1}\n", sfz::path::basename(argv[0]), full_exception_string(e));
        return 1;
    }
    return 0;
}

}  // namespace antares
