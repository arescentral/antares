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

#include "config/preferences.hpp"
#include "data/resource.hpp"
#include "drawing/color.hpp"
#include "drawing/pix-map.hpp"
#include "drawing/pix-table.hpp"
#include "lang/exception.hpp"
#include "video/text-driver.hpp"

using sfz::hex;
using sfz::path::dirname;
using std::unique_ptr;

namespace args = sfz::args;

namespace antares {
namespace {

void draw(pn::string_view name, Hue hue, ArrayPixMap& pix) {
    NatePixTable               table(name, hue);
    const NatePixTable::Frame& frame = table.at(9);
    pix.resize(Size(frame.width(), frame.height()));
    pix.copy(frame.pix_map());
}

class ShapeBuilder {
  public:
    ShapeBuilder(const sfz::optional<pn::string>& output_dir) {
        if (output_dir.has_value()) {
            _output_dir.emplace(output_dir->copy());
        }
    }
    ShapeBuilder(const ShapeBuilder&) = delete;
    ShapeBuilder& operator=(const ShapeBuilder&) = delete;

    void save(pn::string_view name, pn::string_view out, Hue hue) {
        ArrayPixMap pix(0, 0);
        draw(name, hue, pix);
        if (_output_dir.has_value()) {
            const pn::string path =
                    pn::format("{0}/{1}/{2}.png", *_output_dir, out, hex(static_cast<int>(hue)));
            sfz::makedirs(dirname(path), 0755);
            pn::file file = pn::open(path, "w");
            pix.encode(file);
        }
    }

  private:
    sfz::optional<pn::string> _output_dir;
};

void usage(pn::file_view out, pn::string_view progname, int retcode) {
    out.format(
            "usage: {0} [OPTIONS]\n"
            "\n"
            "  Draws shapes used in the long-range view\n"
            "\n"
            "  options:\n"
            "    -o, --output=OUTPUT place output in this directory\n"
            "    -h, --help          display this help screen\n",
            progname);
    exit(retcode);
}

void main(int argc, char* const* argv) {
    args::callbacks callbacks;

    callbacks.argument = [](pn::string_view arg) { return false; };

    sfz::optional<pn::string> output_dir;
    callbacks.short_option = [&argv, &output_dir](
                                     pn::rune opt, const args::callbacks::get_value_f& get_value) {
        switch (opt.value()) {
            case 'o': output_dir.emplace(get_value().copy()); return true;
            case 'h': usage(stdout, sfz::path::basename(argv[0]), 0); return true;
            default: return false;
        }
    };
    callbacks.long_option =
            [&callbacks](pn::string_view opt, const args::callbacks::get_value_f& get_value) {
                if (opt == "output") {
                    return callbacks.short_option(pn::rune{'o'}, get_value);
                } else if (opt == "help") {
                    return callbacks.short_option(pn::rune{'h'}, get_value);
                } else {
                    return false;
                }
            };

    args::parse(argc - 1, argv + 1, callbacks);

    struct {
        pn::string_view name, out;
    } ids[] = {
            {"ish/cruiser", "ishiman/cruiser"},     {"ish/fighter", "ishiman/fighter"},
            {"ish/transport", "ishiman/transport"}, {"obi/escort", "obish/escort"},
            {"gai/cruiser", "gaitori/cruiser"},     {"gai/fighter", "gaitori/fighter"},
            {"gai/transport", "gaitori/transport"}, {"obi/transport", "obish/transport"},
    };

    NullPrefsDriver prefs;
    TextVideoDriver video({640, 480}, output_dir);
    ShapeBuilder    builder(output_dir);
    for (auto id : ids) {
        for (int tint = 0; tint < 16; ++tint) {
            builder.save(id.name, id.out, static_cast<Hue>(tint));
        }
    }
}

}  // namespace
}  // namespace antares

int main(int argc, char* const* argv) { return antares::wrap_main(antares::main, argc, argv); }
