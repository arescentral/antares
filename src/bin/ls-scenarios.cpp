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
#include "lang/exception.hpp"

namespace args = sfz::args;

namespace antares {

void usage(pn::file_view out, pn::string_view progname, int retcode) {
    out.format(
            "usage: {0} [OPTIONS]\n"
            "\n"
            "  Lists installed Antares scenarios\n"
            "\n"
            "  options:\n"
            "    -a, --application-data  set path to application data\n"
            "                            (default: {1})\n"
            "    -f, --factory-scenario  set path to factory scenario\n"
            "                            (default: {2})\n"
            "    -h, --help              display this help screen\n",
            progname, default_application_path(), default_factory_scenario_path());
    exit(retcode);
}

pn::value maybe_string(const sfz::optional<pn::string>& s) {
    if (s.has_value()) {
        return s->copy();
    }
    return nullptr;
}

void main(int argc, char* const* argv) {
    pn::string_view progname = sfz::path::basename(argv[0]);

    args::callbacks callbacks;

    callbacks.argument = [](pn::string_view arg) { return false; };

    callbacks.short_option = [&progname](
                                     pn::rune opt, const args::callbacks::get_value_f& get_value) {
        switch (opt.value()) {
            case 'a': set_application_path(get_value()); return true;
            case 'f': set_factory_scenario_path(get_value()); return true;
            case 'h': usage(stdout, progname, 0); return true;
            default: return false;
        }
    };

    callbacks.long_option =
            [&callbacks](pn::string_view opt, const args::callbacks::get_value_f& get_value) {
                if (opt == "application-data") {
                    return callbacks.short_option(pn::rune{'a'}, get_value);
                } else if (opt == "factory-scenario") {
                    return callbacks.short_option(pn::rune{'f'}, get_value);
                } else if (opt == "help") {
                    return callbacks.short_option(pn::rune{'h'}, get_value);
                } else {
                    return false;
                }
            };

    args::parse(argc - 1, argv + 1, callbacks);

    std::vector<Info> scenarios = scenario_list();
    pn::map           m;
    for (const Info& s : scenarios) {
        m[s.identifier.hash.copy()] = pn::map{
                {"title", s.title.copy()},     {"download_url", maybe_string(s.download_url)},
                {"author", s.author.copy()},   {"author_url", maybe_string(s.author_url)},
                {"version", s.version.copy()},
        };
    }

    if (m.empty()) {
        exit(1);
    }
    pn::file_view{stdout}.dump(m);
}

}  // namespace antares

int main(int argc, char* const* argv) { return antares::wrap_main(antares::main, argc, argv); }
