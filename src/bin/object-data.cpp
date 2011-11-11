// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2011 Ares Central
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
// License along with this program.  If not, see
// <http://www.gnu.org/licenses/>.

#include <fcntl.h>
#include <getopt.h>
#include <sfz/sfz.hpp>

#include "config/preferences.hpp"
#include "data/space-object.hpp"
#include "drawing/color.hpp"
#include "drawing/text.hpp"
#include "game/space-object.hpp"
#include "ui/interface-handling.hpp"

using sfz::Optional;
using sfz::ScopedFd;
using sfz::String;
using sfz::args::help;
using sfz::args::store;
using sfz::dec;
using sfz::format;
using sfz::scoped_ptr;
using sfz::write;

namespace args = sfz::args;
namespace io = sfz::io;
namespace utf8 = sfz::utf8;

namespace antares {
namespace {

class ObjectDataBuilder {
  public:
    ObjectDataBuilder(const Optional<String>& output_dir)
            : _output_dir(output_dir) { }

    void save(int id, int pict_id) {
        String data;
        CreateObjectDataText(&data, id);
        if (_output_dir.has()) {
            String path(format("{0}/{1}.txt", *_output_dir, dec(pict_id, 5)));
            ScopedFd fd(open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644));
            write(fd, utf8::encode(data));
        }
    }

  private:
    const Optional<String> _output_dir;

    DISALLOW_COPY_AND_ASSIGN(ObjectDataBuilder);
};


int main(int argc, char** argv) {
    args::Parser parser(argv[0], "Builds all of the scrolling text images in the game");

    Optional<String> output_dir;
    parser.add_argument("-o", "--output", store(output_dir))
        .help("place output in this directory");
    parser.add_argument("-h", "--help", help(parser, 0))
        .help("display this help screen");

    String error;
    if (!parser.parse_args(argc - 1, argv + 1, error)) {
        print(io::err, format("{0}: {1}\n", parser.name(), error));
        exit(1);
    }

    if (output_dir.has()) {
        makedirs(*output_dir, 0755);
    }

    NullPrefsDriver prefs;
    InitDirectText();
    init_globals();
    SpaceObjectHandlingInit();

    ObjectDataBuilder builder(output_dir);
    for (int id = 0; id < globals()->maxBaseObject; ++id) {
        const int pict_id = gBaseObjectData.get()[id].pictPortraitResID;
        if (pict_id <= 0) {
            continue;
        }
        builder.save(id, pict_id);
    }

    return 0;
}

}  // namespace
}  // namespace antares

int main(int argc, char** argv) {
    return antares::main(argc, argv);
}
