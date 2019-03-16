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

#include "lang/exception.hpp"

namespace args = sfz::args;

namespace antares {
namespace {

void usage(pn::file_view out, pn::string_view progname, int retcode) {
    out.format(
            "usage: {0} [OPTIONS] directory\n"
            "\n"
            "  Prints the tree digest of a directory\n"
            "\n"
            "  arguments:\n"
            "    directory           the directory to take the digest of\n"
            "\n"
            "  options:\n"
            "    -h, --help          display this help screen\n",
            progname);
    exit(retcode);
}

void main(int argc, char* const* argv) {
    args::callbacks callbacks;

    sfz::optional<pn::string> directory;
    callbacks.argument = [&directory](pn::string_view arg) {
        if (!directory.has_value()) {
            directory.emplace(arg.copy());
        } else {
            return false;
        }
        return true;
    };

    callbacks.short_option = [&argv](pn::rune opt, const args::callbacks::get_value_f& get_value) {
        switch (opt.value()) {
            case 'h': usage(stdout, sfz::path::basename(argv[0]), 0); return true;
            default: return false;
        }
    };

    callbacks.long_option =
            [&callbacks](pn::string_view opt, const args::callbacks::get_value_f& get_value) {
                if (opt == "help") {
                    return callbacks.short_option(pn::rune{'h'}, get_value);
                } else {
                    return false;
                }
            };

    args::parse(argc - 1, argv + 1, callbacks);
    if (!directory.has_value()) {
        throw std::runtime_error("missing required argument 'replay'");
    }

    pn::file_view{stdout}.format("{0}\n", sfz::tree_digest(*directory).hex());
}

}  // namespace
}  // namespace antares

int main(int argc, char* const* argv) { return antares::wrap_main(antares::main, argc, argv); }
