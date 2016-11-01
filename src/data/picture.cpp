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

#include "data/picture.hpp"

#include "data/pn.hpp"
#include "data/resource.hpp"
#include "game/sys.hpp"
#include "video/driver.hpp"

using sfz::BytesSlice;
using sfz::Exception;
using sfz::String;
using sfz::StringSlice;
using sfz::format;

namespace antares {

Picture::Picture(int32_t id, bool hidpi) : Picture(sfz2pn(format("pictures/{0}", id))) {}

Picture::Picture(pn::string_view resource, bool hidpi)
        : ArrayPixMap(0, 0), _scale(hidpi ? sys.video->scale() : 1) {
    while (true) {
        try {
            _path = resource.copy();
            if (_scale > 1) {
                _path += sfz2pn(format("@{0}x.png", _scale));
            } else {
                _path += ".png";
            }
            Resource   rsrc(_path);
            BytesSlice in(rsrc.data());
            read(in, *this);
            break;
        } catch (Exception& e) {
            if (_scale > 1) {
                _scale >>= 1;
            } else {
                throw;
            }
        }
    }
}

Texture Picture::texture() const {
    return sys.video->texture(sfz2pn(format("/{0}", pn2sfz(_path))), *this);
}

}  // namespace antares
