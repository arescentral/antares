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

#include "config/dirs.hpp"
#include "data/extractor.hpp"
#include "net/http.hpp"

using sfz::Exception;
using sfz::Optional;
using sfz::String;
using sfz::StringSlice;
using sfz::print;
using sfz::args::help;
using sfz::args::store;
using sfz::args::store_const;

namespace args = sfz::args;
namespace io   = sfz::io;
namespace utf8 = sfz::utf8;

namespace antares {

class PrintStatusObserver : public DataExtractor::Observer {
  public:
    virtual void status(const sfz::StringSlice& status) {
        print(io::err, format("{0}\n", status));
    }
};

void ExtractDataMain(int argc, char* const* argv) {
    args::Parser parser(argv[0], "Downloads and extracts game data");

    String           source(dirs().downloads);
    String           dest(dirs().scenarios);
    bool             check = false;
    Optional<String> plugin;
    parser.add_argument("plugin", store(plugin))
            .help("a plugin to install (default: install factory scenario)");
    parser.add_argument("-s", "--source", store(source))
            .help("directory in which to store or expect zip files");
    parser.add_argument("-d", "--dest", store(dest)).help("place output in this directory");
    parser.add_argument("-c", "--check", store_const(check, true))
            .help("don't install, just check if up-to-date");
    parser.add_argument("-h", "--help", help(parser, 0)).help("display this help screen");

    try {
        String error;
        if (!parser.parse_args(argc - 1, argv + 1, error)) {
            print(io::err, format("{0}: {1}\n", parser.name(), error));
            exit(1);
        }

        DataExtractor extractor(source, dest);
        if (plugin.has()) {
            extractor.set_plugin_file(*plugin);
        }

        if (extractor.current()) {
            print(io::err, format("{0} is up-to-date!\n", dest));
        } else if (check) {
            print(io::err, format("{0} is not up-to-date.\n", dest));
            exit(1);
        } else {
            print(io::err, format("Extracting to {0}...\n", dest));
            PrintStatusObserver observer;
            extractor.extract(&observer);
            print(io::err, StringSlice("done.\n"));
        }
    } catch (Exception& e) {
        print(io::err, format("{0}: {1}\n", utf8::decode(argv[0]), e.message()));
        exit(1);
    }
}

}  // namespace antares

int main(int argc, char* const* argv) {
    antares::ExtractDataMain(argc, argv);
    return 0;
}
