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
#include <shellapi.h>
#include <vector>
#include <pn/string>

int main(int argc, char* const* argv);

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

    return main(argc, argv);
}
