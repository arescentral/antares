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

#include <exception>
#include <pn/string>

#include "lang/exception.hpp"
#include "win/main.hpp"

int main(int argc, char* const* argv) {
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
