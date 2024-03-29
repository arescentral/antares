// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2015-2022 The Antares Authors
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

#ifndef ANTARES_WIN_OFFSCREEN_HPP_
#define ANTARES_WIN_OFFSCREEN_HPP_

#define GLX_GLXEXT_PROTOTYPES

#include <memory>
#include <utility>

#include "math/geometry.hpp"

namespace antares {

class Offscreen {
  public:
    Offscreen(Size size, std::pair<int, int> gl_version);
    ~Offscreen();
};

}  // namespace antares

#endif  // ANTARES_WIN_OFFSCREEN_HPP_
