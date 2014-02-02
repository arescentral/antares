// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2012 The Antares Authors
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

#include "data/picture.hpp"

#include "data/resource.hpp"

using sfz::BytesSlice;
using sfz::String;
using sfz::StringSlice;
using sfz::format;

namespace antares {

Picture::Picture(int32_t id):
        Picture(String(format("pictures/{0}.png", id))) { }

Picture::Picture(StringSlice resource)
        : ArrayPixMap(0, 0) {
    Resource rsrc(resource);
    BytesSlice in(rsrc.data());
    read(in, *this);
}

}  // namespace antares
