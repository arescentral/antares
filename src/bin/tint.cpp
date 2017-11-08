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

#include <fcntl.h>
#include <sfz/sfz.hpp>

#include "config/preferences.hpp"
#include "drawing/color.hpp"
#include "drawing/pix-map.hpp"
#include "drawing/pix-table.hpp"
#include "video/text-driver.hpp"

using sfz::Optional;
using sfz::ScopedFd;
using sfz::String;
using sfz::StringSlice;
using sfz::args::help;
using sfz::args::store;
using sfz::format;
using sfz::hex;
using sfz::path::dirname;
using sfz::write;
using std::unique_ptr;

namespace io   = sfz::io;
namespace utf8 = sfz::utf8;
namespace args = sfz::args;

namespace antares {
namespace {

const char* name(int16_t id) {
    switch (id) {
        case 501: return "ishiman/cruiser";
        case 510: return "ishiman/fighter";
        case 515: return "ishiman/transport";
        case 532: return "obish/escort";
        case 550: return "gaitori/cruiser";
        case 551: return "gaitori/fighter";
        case 563: return "gaitori/transport";
        case 567: return "obish/transport";
    }
    abort();
}

void draw(int16_t id, uint8_t color, ArrayPixMap& pix) {
    NatePixTable               table(id, color);
    const NatePixTable::Frame& frame = table.at(9);
    pix.resize(Size(frame.width(), frame.height()));
    pix.copy(frame.pix_map());
}

class ShapeBuilder {
  public:
    ShapeBuilder(const Optional<String>& output_dir) : _output_dir(output_dir) {}

    void save(int16_t id, uint8_t color) {
        ArrayPixMap pix(0, 0);
        draw(id, color, pix);
        if (_output_dir.has()) {
            const String path(format("{0}/{1}/{2}.png", *_output_dir, name(id), hex(color)));
            makedirs(dirname(path), 0755);
            ScopedFd fd(open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644));
            write(fd, pix);
        }
    }

  private:
    const Optional<String> _output_dir;

    DISALLOW_COPY_AND_ASSIGN(ShapeBuilder);
};

int main(int argc, char* const* argv) {
    args::Parser parser(argv[0], "Draws shapes used in the long-range view");

    Optional<String> output_dir;
    parser.add_argument("-o", "--output", store(output_dir))
            .help("place output in this directory");
    parser.add_argument("-h", "--help", help(parser, 0)).help("display this help screen");

    String error;
    if (!parser.parse_args(argc - 1, argv + 1, error)) {
        print(io::err, format("{0}: {1}\n", parser.name(), error));
        exit(1);
    }

    NullPrefsDriver prefs;
    TextVideoDriver video({640, 480}, output_dir);
    ShapeBuilder    builder(output_dir);
    int16_t         ids[] = {501, 510, 515, 532, 550, 551, 563, 567};
    for (int16_t id : ids) {
        for (int tint = 0; tint < 16; ++tint) {
            builder.save(id, tint);
        }
    }

    return 0;
}

}  // namespace
}  // namespace antares

int main(int argc, char** argv) {
    return antares::main(argc, argv);
}
