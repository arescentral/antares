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
#include "drawing/color.hpp"
#include "drawing/text.hpp"
#include "game/globals.hpp"
#include "game/level.hpp"
#include "ui/interface-handling.hpp"
#include "video/text-driver.hpp"

using sfz::dec;
using std::unique_ptr;

namespace args = sfz::args;

namespace antares {
namespace {

class ObjectDataBuilder {
  public:
    ObjectDataBuilder(const sfz::optional<pn::string>& output_dir) {
        if (output_dir.has_value()) {
            _output_dir.emplace(output_dir->copy());
        }
    }
    ObjectDataBuilder(const ObjectDataBuilder&) = delete;
    ObjectDataBuilder& operator=(const ObjectDataBuilder&) = delete;

    void save(const BaseObject& object, pn::string_view portrait) {
        pn::string data;
        CreateObjectDataText(data, object);
        if (_output_dir.has_value()) {
            pn::string      path = pn::format("{0}/{1}.txt", *_output_dir, portrait);
            pn::string_view dir  = sfz::path::dirname(path);
            try {
                sfz::makedirs(dir, 0755);
            } catch (...) {
                std::throw_with_nested(std::runtime_error(dir.copy().c_str()));
            }
            try {
                pn::file file = pn::open(path, "w").check();
                file.write(data);
            } catch (...) {
                std::throw_with_nested(std::runtime_error(path.copy().c_str()));
            }
        }
    }

  private:
    sfz::optional<pn::string> _output_dir;
};

void usage(pn::file_view out, pn::string_view progname, int retcode) {
    pn::format(
            out,
            "usage: {0} [OPTIONS]\n"
            "\n"
            "  Builds all of the scrolling text images in the game\n"
            "\n"
            "  options:\n"
            "    -o, --output=OUTPUT place output in this directory\n"
            "    -h, --help          display this help screen\n",
            progname);
    exit(retcode);
}

void load_object_data(Handle<const BaseObject> o) {
    load_object(o);
    for (const auto& w : {o->pulse, o->beam, o->special}) {
        if (w.has_value()) {
            load_object_data(w->base);
        }
    }
    for (const std::unique_ptr<const Action>& a : o->activate) {
        if (a->created_base()) {
            load_object_data(*a->created_base());
        }
    }
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

    if (output_dir.has_value()) {
        sfz::makedirs(*output_dir, 0755);
    }

    NullPrefsDriver prefs;
    TextVideoDriver video({640, 480}, {});
    init_globals();
    PluginInit();

    ObjectDataBuilder builder(output_dir);
    for (auto id : {0,   7,   13,  14,  17,  19,  24,  25,  37,  38,  42,  45,  47,  54,  56,
                    59,  61,  62,  64,  67,  75,  78,  80,  86,  88,  89,  93,  94,  95,  101,
                    104, 109, 110, 112, 113, 114, 118, 119, 120, 126, 127, 128, 129, 138, 139,
                    140, 142, 147, 148, 149, 151, 164, 165, 166, 167, 184, 243, 250, 270}) {
        Handle<const BaseObject> object(id);
        load_object_data(object);
        builder.save(*object, object->portrait);
    }
}

void print_nested_exception(const std::exception& e) {
    pn::format(stderr, ": {0}", e.what());
    try {
        std::rethrow_if_nested(e);
    } catch (const std::exception& e) {
        print_nested_exception(e);
    }
}

void print_exception(pn::string_view progname, const std::exception& e) {
    pn::format(stderr, "{0}: {1}", sfz::path::basename(progname), e.what());
    try {
        std::rethrow_if_nested(e);
    } catch (const std::exception& e) {
        print_nested_exception(e);
    }
    pn::format(stderr, "\n");
}

}  // namespace
}  // namespace antares

int main(int argc, char* const* argv) {
    try {
        antares::main(argc, argv);
    } catch (const std::exception& e) {
        antares::print_exception(argv[0], e);
        return 1;
    }
    return 0;
}
