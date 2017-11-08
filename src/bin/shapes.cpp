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

#include "drawing/color.hpp"
#include "drawing/pix-map.hpp"
#include "drawing/shapes.hpp"

using sfz::Optional;
using sfz::ScopedFd;
using sfz::String;
using sfz::StringSlice;
using sfz::args::help;
using sfz::args::store;
using sfz::dec;
using sfz::format;
using sfz::path::dirname;
using sfz::write;
using std::unique_ptr;

namespace io   = sfz::io;
namespace utf8 = sfz::utf8;
namespace args = sfz::args;

namespace antares {
namespace {

enum Shape {
    SQUARE,
    PLUS,
    TRIANGLE,
    DIAMOND,
};

const char* name(Shape shape) {
    switch (shape) {
        case SQUARE: return "square";
        case PLUS: return "plus";
        case TRIANGLE: return "triangle";
        case DIAMOND: return "diamond";
    }
    abort();
}

void draw(Shape shape, PixMap& pix) {
    RgbColor color = rgb(255, 0, 0);
    switch (shape) {
        case SQUARE: pix.fill(color); break;
        case PLUS: draw_compat_plus(&pix, color); break;
        case TRIANGLE: draw_triangle_up(&pix, color); break;
        case DIAMOND: draw_compat_diamond(&pix, color); break;
    }
}

class ShapeBuilder {
  public:
    ShapeBuilder(const Optional<String>& output_dir) : _output_dir(output_dir) {}

    void save(Shape shape, int size) {
        ArrayPixMap pix(size, size);
        pix.fill(RgbColor::clear());
        draw(shape, pix);
        if (_output_dir.has()) {
            const String path(format("{0}/{1}/{2}.png", *_output_dir, name(shape), dec(size, 2)));
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

    ShapeBuilder builder(output_dir);
    Shape        shapes[] = {SQUARE, PLUS, TRIANGLE, DIAMOND};
    for (size_t i = 0; i < 4; ++i) {
        for (int size = 1; size < 16; ++size) {
            builder.save(shapes[i], size);
        }
    }

    return 0;
}

}  // namespace
}  // namespace antares

int main(int argc, char** argv) {
    return antares::main(argc, argv);
}
