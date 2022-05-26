// Copyright (C) 2022 The Antares Authors
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

#include <windows.h>

#undef min
#undef max
#undef interface
#include <GLFW/glfw3.h>
#include <shellapi.h>
#include <time.h>
#include <pn/output>
#include <pn/string>
#include <sfz/sfz.hpp>
#include <vector>

#include "config/dirs.hpp"
#include "config/file-prefs-driver.hpp"
#include "config/ledger.hpp"
#include "config/preferences.hpp"
#include "data/extractor.hpp"
#include "game/sys.hpp"
#include "glfw/video-driver.hpp"
#include "lang/exception.hpp"
#include "ui/flows/master.hpp"

#ifdef _MSC_VER
#include "sound/xaudio2-driver.hpp"
#else
#include "sound/driver.hpp"
#endif

using sfz::range;

namespace args = sfz::args;

namespace antares {
namespace {

pn::string_view default_config_path() {
    static pn::string path = pn::format("{0}/config.pn", dirs().root);
    return path;
}

void usage(pn::output_view out, pn::string_view progname, int retcode) {
    out.format(
            "usage: {0} [OPTIONS] [scenario]\n"
            "\n"
            "  Antares: a tactical space combat game\n"
            "\n"
            "  arguments:\n"
            "    scenario            path to plugin file (default: factory scenario)\n"
            "\n"
            "  options:\n"
            "    -a, --app-data      set path to application data\n"
            "                        (default: {1})\n"
            "    -c, --config        set path to config file\n"
            "                        (default: {2})\n"
            "        --console       allocate console\n"
            "    -f, --factory       set path to factory scenario\n"
            "                        (default: {3})\n"
            "    -h, --help          display this help screen\n",
            progname, default_application_path(), default_config_path(),
            default_factory_scenario_path());
    exit(retcode);
}

class NullStatusObserver : public DataExtractor::Observer {
  public:
    virtual void status(pn::string_view) {}
};

void extract_data() {
    DataExtractor extractor(dirs().downloads, dirs().scenarios);
    if (extractor.current()) {
        return;
    }
    NullStatusObserver observer;
    extractor.extract(&observer);
}

void main(int argc, char* const* argv) {
    pn::string_view progname = sfz::path::basename(argv[0]);

    args::callbacks callbacks;

    sfz::optional<pn::string_view> scenario;
    callbacks.argument = [&scenario](pn::string_view arg) {
        if (!scenario.has_value()) {
            scenario.emplace(arg);
        } else {
            return false;
        }
        return true;
    };

    pn::string_view config_path = default_config_path();
    callbacks.short_option      = [&progname, &config_path](
                                     pn::rune opt, const args::callbacks::get_value_f& get_value) {
        switch (opt.value()) {
            case 'a': set_application_path(get_value()); return true;
            case 'c': config_path = get_value(); return true;
            case 'f': set_factory_scenario_path(get_value()); return true;
            case 'h': usage(pn::out, progname, 0); return true;
            default: return false;
        }
    };

    callbacks.long_option =
            [&callbacks](pn::string_view opt, const args::callbacks::get_value_f& get_value) {
                if (opt == "app-data") {
                    return callbacks.short_option(pn::rune{'a'}, get_value);
                } else if (opt == "config") {
                    return callbacks.short_option(pn::rune{'c'}, get_value);
                } else if (opt == "factory-scenario") {
                    return callbacks.short_option(pn::rune{'f'}, get_value);
                } else if (opt == "help") {
                    return callbacks.short_option(pn::rune{'h'}, get_value);
                } else if (opt == "console") {
                    FILE* f;
                    if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
                        AllocConsole();
                    }
                    freopen_s(&f, "CONIN$", "r", stdin);
                    freopen_s(&f, "CONOUT$", "w", stdout);
                    freopen_s(&f, "CONOUT$", "w", stderr);
                    return true;
                } else {
                    return false;
                }
            };

    args::parse(argc - 1, argv + 1, callbacks);

    if (!sfz::path::isdir(application_path())) {
        throw std::runtime_error(
                "Application data not found."
                " Please keep Antares next to the data folder.");
    }

    extract_data();

    FilePrefsDriver prefs(config_path);

    DirectoryLedger ledger;
#ifdef _MSC_VER
    XAudio2SoundDriver sound;
#else
    NullSoundDriver sound;
#endif
    GLFWVideoDriver video;
    video.loop(new Master(scenario, time(NULL)));
}

}  // namespace
}  // namespace antares

int APIENTRY WinMain(HINSTANCE h_inst, HINSTANCE h_inst_prev, PSTR cmd_line, int cmd_show) {
    LPWSTR* wargv;
    int     argc;

    wargv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!wargv) {
        return -1;
    }

    std::vector<std::string> arg_strings;
    std::vector<char*>       argv_vector;

    for (int i = 0; i < argc; i++) {
        const wchar_t* warg = wargv[i];
        arg_strings.push_back(pn::string(warg).cpp_str());
    }

    LocalFree(wargv);

    for (int i = 0; i < argc; i++) {
        argv_vector.push_back(arg_strings[i].data());
    }

    char* const* argv = nullptr;
    if (argc > 0) {
        argv = &argv_vector[0];
    }

    try {
        antares::main(argc, argv);
    } catch (std::exception& e) {
        MessageBox(
                NULL, antares::full_exception_string(e).c_str(), "Antares Error",
                MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);
        return 1;
    }
    return 0;
}
