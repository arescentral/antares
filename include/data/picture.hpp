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

#ifndef ANTARES_DATA_PICTURE_HPP_
#define ANTARES_DATA_PICTURE_HPP_

#include <stdint.h>
#include <pn/string>

#include "drawing/pix-map.hpp"

namespace antares {

class Texture;

class Picture : public ArrayPixMap {
  public:
    Picture(int32_t id, bool hidpi = false);
    Picture(pn::string_view resource, bool hidpi = false);

    pn::string_view path() const { return _path; }
    int             scale() const { return _scale; }

    Texture texture() const;

  private:
    pn::string _path;
    int        _scale;
};

}  // namespace antares

#endif  // ANTARES_DATA_PICTURE_HPP_
