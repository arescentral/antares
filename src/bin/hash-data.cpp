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

#include <sfz/sfz.hpp>

using sfz::String;
using sfz::StringSlice;
using sfz::format;
using sfz::args::help;
using sfz::args::store;
using sfz::print;

namespace args = sfz::args;
namespace io   = sfz::io;
namespace utf8 = sfz::utf8;

namespace antares {

void main(int argc, char* const* argv) {
    args::Parser parser(argv[0], "Prints the tree digest of a directory");

    String directory;
    parser.add_argument("directory", store(directory))
            .help("the directory to take the digest of")
            .required();
    parser.add_argument("-h", "--help", help(parser, 0)).help("display this help screen");

    String error;
    if (!parser.parse_args(argc - 1, argv + 1, error)) {
        print(io::err, format("{0}: {1}\n", parser.name(), error));
        exit(1);
    }

    print(io::out, format("{0}\n", tree_digest(directory)));
}

}  // namespace antares

int main(int argc, char* const* argv) {
    antares::main(argc, argv);
    return 0;
}
