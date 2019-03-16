// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2015-2017 The Antares Authors
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

#include <GLFW/glfw3.h>

#include <time.h>
#include <pn/file>
#include <sfz/sfz.hpp>

#include "config/dirs.hpp"
#include "config/file-prefs-driver.hpp"
#include "config/ledger.hpp"
#include "config/preferences.hpp"
#include "data/scenario-list.hpp"
#include "game/sys.hpp"
#include "glfw/video-driver.hpp"
#include "lang/exception.hpp"
#include "sound/openal-driver.hpp"
#include "ui/flows/master.hpp"

using sfz::range;

namespace args = sfz::args;

namespace antares {
namespace {

void usage(pn::file_view out, pn::string_view progname, int retcode) {
    out.format(
            "usage: {0} [OPTIONS] [scenario]\n"
            "\n"
            "  Antares: a tactical space combat game\n"
            "\n"
            "  arguments:\n"
            "    scenario            select scenario\n"
            "\n"
            "  options:\n"
            "    -a, --app-data      set path to application data\n"
            "                        (default: {1})\n"
            "    -f, --factory       set path to factory scenario\n"
            "                        (default: {2})\n"
            "    -h, --help          display this help screen\n",
            progname, default_application_path(), default_factory_scenario_path());
    exit(retcode);
}

void main(int argc, char* const* argv) {
    pn::string_view progname = sfz::path::basename(argv[0]);

    args::callbacks callbacks;

    sfz::optional<pn::string> scenario;
    callbacks.argument = [&scenario](pn::string_view arg) {
        if (!scenario.has_value()) {
            scenario.emplace(arg.copy());
        } else {
            return false;
        }
        return true;
    };

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
                if (opt == "app-data") {
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

    if (!scenario.has_value()) {
        scenario.emplace(kFactoryScenarioIdentifier);
    }

    if (!sfz::path::isdir(application_path())) {
        if (application_path() == default_application_path()) {
            throw std::runtime_error(
                    "application data not installed\n"
                    "\n"
                    "Please install it, or specify a path with --app-data");
        } else {
            throw std::runtime_error(
                    pn::format("{0}: application data not found", application_path()).c_str());
        }
        exit(1);
    }

    FilePrefsDriver prefs;

    if (scenario.has_value()) {
        sys.prefs->set_scenario_identifier(*scenario);
        bool              have_scenario = false;
        std::vector<Info> scenarios     = scenario_list();
        for (const Info& entry : scenarios) {
            if (entry.identifier.hash == *scenario) {
                have_scenario = true;
                break;
            }
        }
        if (!have_scenario) {
            throw std::runtime_error(
                    pn::format("{1}: scenario not installed\n", progname, *scenario).c_str());
        }
    }

    DirectoryLedger   ledger;
    OpenAlSoundDriver sound;
    GLFWVideoDriver   video;
    video.loop(new Master(time(NULL)));
}

}  // namespace
}  // namespace antares

int main(int argc, char* const* argv) { return antares::wrap_main(antares::main, argc, argv); }
