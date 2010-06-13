// Ares, a tactical space combat game.
// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#include "sfz/sfz.hpp"
#include "DataExtractor.hpp"
#include "FoundationHttpDriver.hpp"
#include "HttpDriver.hpp"

using sfz::String;
using sfz::StringPiece;
using sfz::print;

namespace io = sfz::io;
namespace utf8 = sfz::utf8;

namespace antares {

void usage(const StringPiece& program_name) {
    print(io::err, format(
            "usage: {0} SOURCE DESTINATION\n",
            program_name));
    exit(1);
}

class PrintStatusObserver : public DataExtractor::Observer {
  public:
    virtual void status(const sfz::StringPiece& status) {
        print(io::err, format("{0}\n", status));
    }
};

void ExtractDataMain(int argc, char* const* argv) {
    String program_name(utf8::decode(argv[0]));

    if (argc != 3) {
        print(io::err, format("{0}: wrong number of arguments\n", program_name));
        usage(program_name);
    }
    HttpDriver::set_driver(new FoundationHttpDriver());
    String source(utf8::decode(argv[1]));
    String dest(utf8::decode(argv[2]));
    DataExtractor extractor(source, dest);

    if (extractor.current()) {
        print(io::err, format("{0} is up-to-date!\n", dest));
    } else {
        print(io::err, format("Extracting to {0}...\n", dest));
        PrintStatusObserver observer;
        extractor.extract(&observer);
        print(io::err, StringPiece("done.\n"));
    }
}

}  // namespace antares

int main(int argc, char* const* argv) {
    antares::ExtractDataMain(argc, argv);
    return 0;
}
