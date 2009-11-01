// Ares, a tactical space combat game.
// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#ifndef ANTARES_FAKES_HPP_
#define ANTARES_FAKES_HPP_

#include <stdint.h>
#include <string>
#include "Base.h"
#include "AresGlobalType.hpp"

namespace antares {

void FakeInit(int argc, char* const* argv);
int GetDemoScenario();
const std::string& get_output_dir();
int Munger(std::string* data, int pos, const unsigned char* search, size_t search_len,
        const unsigned char* replace, size_t replace_len);

}  // namespace antares

#endif  // ANTARES_FAKES_HPP_
