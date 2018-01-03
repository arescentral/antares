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
#include <getopt.h>
#include <pn/file>
#include <sfz/sfz.hpp>

#include "config/preferences.hpp"
#include "data/base-object.hpp"
#include "data/plugin.hpp"
#include "data/pn.hpp"
#include "drawing/color.hpp"
#include "drawing/text.hpp"
#include "game/globals.hpp"
#include "game/level.hpp"
#include "ui/interface-handling.hpp"
#include "video/text-driver.hpp"

using sfz::Optional;
using sfz::args::help;
using sfz::args::store;
using sfz::dec;
using sfz::write;
using std::unique_ptr;

namespace args = sfz::args;

namespace antares {
namespace {

class ObjectDataBuilder {
  public:
    ObjectDataBuilder(const Optional<sfz::String>& output_dir) : _output_dir(output_dir) {}
    ObjectDataBuilder(const ObjectDataBuilder&) = delete;
    ObjectDataBuilder& operator=(const ObjectDataBuilder&) = delete;

    void save(Handle<BaseObject> object, int pict_id) {
        pn::string data;
        CreateObjectDataText(data, object);
        if (_output_dir.has()) {
            pn::string path = pn::format(
                    "{0}/{1}.txt", sfz2pn(*_output_dir), sfz2pn(sfz::String(dec(pict_id, 5))));
            pn::file file = pn::open(path, "w");
            file.write(data);
        }
    }

  private:
    const Optional<sfz::String> _output_dir;
};

int main(int argc, char** argv) {
    args::Parser parser(argv[0], "Builds all of the scrolling text images in the game");

    Optional<sfz::String> output_dir;
    parser.add_argument("-o", "--output", store(output_dir))
            .help("place output in this directory");
    parser.add_argument("-h", "--help", help(parser, 0)).help("display this help screen");

    sfz::String error;
    if (!parser.parse_args(argc - 1, argv + 1, error)) {
        pn::format(stderr, "{0}: {1}\n", sfz2pn(parser.name()), sfz2pn(error));
        exit(1);
    }

    if (output_dir.has()) {
        makedirs(*output_dir, 0755);
    }

    NullPrefsDriver prefs;
    TextVideoDriver video({640, 480}, {});
    init_globals();
    PluginInit();

    ObjectDataBuilder builder(output_dir);
    for (auto object : BaseObject::all()) {
        const int pict_id = object->pictPortraitResID;
        if (pict_id <= 0) {
            continue;
        }
        builder.save(object, pict_id);
    }

    return 0;
}

}  // namespace
}  // namespace antares

int main(int argc, char** argv) { return antares::main(argc, argv); }
