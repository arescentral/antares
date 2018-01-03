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
#include <pn/file>
#include <sfz/sfz.hpp>

#include "data/pn.hpp"
#include "drawing/color.hpp"
#include "drawing/pix-map.hpp"
#include "drawing/shapes.hpp"

using sfz::Optional;
using sfz::args::help;
using sfz::args::store;
using sfz::dec;
using sfz::path::dirname;
using sfz::write;
using std::unique_ptr;

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
    ShapeBuilder(const Optional<sfz::String>& output_dir) : _output_dir(output_dir) {}
    ShapeBuilder(const ShapeBuilder&) = delete;
    ShapeBuilder& operator=(const ShapeBuilder&) = delete;

    void save(Shape shape, int size) {
        ArrayPixMap pix(size, size);
        pix.fill(RgbColor::clear());
        draw(shape, pix);
        if (_output_dir.has()) {
            const pn::string path = pn::format(
                    "{0}/{1}/{2}.png", sfz2pn(*_output_dir), name(shape),
                    sfz2pn(sfz::String(dec(size, 2))));
            makedirs(dirname(pn2sfz(path)), 0755);
            pn::file file = pn::open(path, "w");
            pix.encode(file);
        }
    }

  private:
    const Optional<sfz::String> _output_dir;
};

int main(int argc, char* const* argv) {
    args::Parser parser(argv[0], "Draws shapes used in the long-range view");

    Optional<sfz::String> output_dir;
    parser.add_argument("-o", "--output", store(output_dir))
            .help("place output in this directory");
    parser.add_argument("-h", "--help", help(parser, 0)).help("display this help screen");

    sfz::String error;
    if (!parser.parse_args(argc - 1, argv + 1, error)) {
        pn::format(stderr, "{0}: {1}\n", sfz2pn(parser.name()), sfz2pn(error));
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

int main(int argc, char** argv) { return antares::main(argc, argv); }
