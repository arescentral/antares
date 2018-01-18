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

#include <pn/file>
#include <sfz/sfz.hpp>

#include "config/dirs.hpp"
#include "data/extractor.hpp"
#include "net/http.hpp"

namespace args = sfz::args;

namespace antares {
namespace {

class PrintStatusObserver : public DataExtractor::Observer {
  public:
    virtual void status(pn::string_view status) { pn::format(stderr, "{0}\n", status); }
};

void usage(pn::file_view out, pn::string_view progname, int retcode) {
    pn::format(
            out,
            "usage: {0} [OPTIONS] [plugin]\n"
            "\n"
            "  Downloads and extracts game data\n"
            "\n"
            "  arguments:\n"
            "    plugin              a plugin to install (default: install factory scenario)\n"
            "\n"
            "  options:\n"
            "    -s, --source=SOURCE directory in which to store or expect zip files\n"
            "    -d, --dest=DEST     place output in this directory\n"
            "    -c, --check         don't install, just check if up-to-date\n"
            "    -h, --help          display this help screen\n",
            progname);
    exit(retcode);
}

void main(int argc, char* const* argv) {
    args::callbacks callbacks;

    sfz::optional<pn::string> plugin;
    callbacks.argument = [&plugin](pn::string_view arg) {
        if (!plugin.has_value()) {
            plugin.emplace(arg.copy());
        } else {
            return false;
        }
        return true;
    };

    pn::string source      = dirs().downloads.copy();
    pn::string dest        = dirs().scenarios.copy();
    bool       check       = false;
    callbacks.short_option = [&argv, &source, &dest, &check](
                                     pn::rune opt, const args::callbacks::get_value_f& get_value) {
        switch (opt.value()) {
            case 's': source = get_value().copy(); return true;
            case 'd': dest = get_value().copy(); return true;
            case 'c': check = true; return true;
            case 'h': usage(stdout, sfz::path::basename(argv[0]), 0); return true;
            default: return false;
        }
    };

    callbacks.long_option =
            [&callbacks](pn::string_view opt, const args::callbacks::get_value_f& get_value) {
                if (opt == "source") {
                    return callbacks.short_option(pn::rune{'s'}, get_value);
                } else if (opt == "dest") {
                    return callbacks.short_option(pn::rune{'d'}, get_value);
                } else if (opt == "check") {
                    return callbacks.short_option(pn::rune{'c'}, get_value);
                } else if (opt == "help") {
                    return callbacks.short_option(pn::rune{'h'}, get_value);
                } else {
                    return false;
                }
            };

    args::parse(argc - 1, argv + 1, callbacks);

    DataExtractor extractor(source, dest);
    if (plugin.has_value()) {
        extractor.set_plugin_file(*plugin);
    }

    if (extractor.current()) {
        pn::format(stderr, "{0} is up-to-date!\n", dest);
    } else if (check) {
        pn::format(stderr, "{0} is not up-to-date.\n", dest);
        exit(1);
    } else {
        pn::format(stderr, "Extracting to {0}...\n", dest);
        PrintStatusObserver observer;
        extractor.extract(&observer);
        pn::format(stderr, "done.\n");
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
