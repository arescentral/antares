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
#include "lang/exception.hpp"
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
    out.format(
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

void load_object_data(const NamedHandle<const BaseObject>& o) {
    load_object(o);
    for (const auto* w : {&o->weapons.pulse, &o->weapons.beam, &o->weapons.special}) {
        if (w->has_value()) {
            load_object_data((*w)->base);
        }
    }
    for (const Action& a : o->activate.action) {
        switch (a.type()) {
            case Action::Type::CREATE: load_object_data(a.create.base); break;
            case Action::Type::MORPH: load_object_data(a.morph.base); break;
            case Action::Type::EQUIP: load_object_data(a.equip.base); break;
            default: continue;
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
    pn::string_view   ids[] = {
            "gai/cruiser",
            "gai/gunship",
            "gai/fighter",
            "gai/transport",
            "gai/engineer",
            "gai/hvd",
            "can/carrier",
            "can/defdrone",
            "can/cruiser",
            "can/aslttran",
            "can/gunship",
            "can/fighter",
            "can/etc/border-drone",
            "can/transport",
            "can/engineer",
            "can/hvc",
            "can/schooner",
            "sal/carrier",
            "sal/cruiser",
            "sal/aslttran",
            "sal/gunship",
            "sal/fighter",
            "sal/transport",
            "loc/power",
            "loc/moor",
            "loc/outpost",
            "loc/bunker",
            "loc/flak",
            "ish/carrier",
            "ish/defdrone",
            "ish/cruiser",
            "ish/aslttran",
            "ish/gunship",
            "ish/fighter",
            "ish/etc/research",
            "ish/etc/tug",
            "ish/etc/astrominer",
            "ish/etc/cargo",
            "ish/transport",
            "ish/engineer",
            "ish/hvc",
            "ish/hvd",
            "ele/cruiser",
            "ele/etc/escape-pod",
            "ele/etc/liner",
            "obi/transport",
            "obi/escort",
            "aud/carrier",
            "aud/cruiser",
            "aud/aslttran",
            "aud/gunship",
            "aud/fighter",
            "ast/regular/big",
            "uns/carrier",
            "uns/cruiser",
            "uns/gunship",
            "uns/fighter",
            "baz/bttlship",
            "zzz/obi/bttlcrsr",
    };
    for (pn::string_view id : ids) {
        NamedHandle<const BaseObject> object(id);
        load_object_data(object);
        builder.save(*object, *object->portrait);
    }
}

}  // namespace
}  // namespace antares

int main(int argc, char* const* argv) { return antares::wrap_main(antares::main, argc, argv); }
