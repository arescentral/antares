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

#include <stdlib.h>
#include <pn/file>
#include <sfz/sfz.hpp>

#include "config/dirs.hpp"
#include "data/scenario-list.hpp"

namespace args = sfz::args;

namespace antares {

void usage(pn::file_view out, pn::string_view progname, int retcode) {
    pn::format(
            out,
            "usage: {0} [OPTIONS]\n"
            "\n"
            "  Lists installed Antares scenarios\n"
            "\n"
            "  options:\n"
            "    -f, --factory       set path to factory scenario\n"
            "                        (default: {1})\n"
            "    -h, --help          display this help screen\n",
            progname, default_factory_scenario_path());
    exit(retcode);
}

void main(int argc, char* const* argv) {
    pn::string_view progname = sfz::path::basename(argv[0]);

    args::callbacks callbacks;

    callbacks.argument = [](pn::string_view arg) { return false; };

    callbacks.short_option = [&progname](
                                     pn::rune opt, const args::callbacks::get_value_f& get_value) {
        switch (opt.value()) {
            case 'f': set_factory_scenario_path(get_value()); return true;
            case 'h': usage(stdout, progname, 0); return true;
            default: return false;
        }
    };

    callbacks.long_option =
            [&callbacks](pn::string_view opt, const args::callbacks::get_value_f& get_value) {
                if (opt == "factory-scenario") {
                    return callbacks.short_option(pn::rune{'f'}, get_value);
                } else if (opt == "help") {
                    return callbacks.short_option(pn::rune{'h'}, get_value);
                } else {
                    return false;
                }
            };

    args::parse(argc - 1, argv + 1, callbacks);

    bool         found_at_least_one = false;
    ScenarioList list;
    for (size_t i = 0; i < list.size(); ++i) {
        if (!list.at(i).installed) {
            continue;
        }
        pn::format(stderr, "{0}:\n", list.at(i).identifier);
        pn::format(stderr, "    title: {0}\n", list.at(i).title);
        pn::format(stderr, "    download url: {0}\n", list.at(i).download_url);
        pn::format(stderr, "    author: {0}\n", list.at(i).author);
        pn::format(stderr, "    author url: {0}\n", list.at(i).author_url);
        pn::format(stderr, "    version: {0}\n", list.at(i).version);
        found_at_least_one = true;
    }
    if (!found_at_least_one) {
        exit(1);
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
