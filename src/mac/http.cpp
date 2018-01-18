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

#include "net/http.hpp"

#include <CoreFoundation/CoreFoundation.h>
#include <pn/file>

#include "mac/core-foundation.hpp"
#include "net/http.hpp"

namespace antares {
namespace http {

void get(pn::string_view url, pn::file_view out) {
    cf::Url  cfurl(url);
    cf::Data cfdata;
    SInt32   error;
    if (CFURLCreateDataAndPropertiesFromResource(
                NULL, cfurl.c_obj(), &cfdata.c_obj(), NULL, NULL, &error)) {
        out.write(cfdata.data());
    } else {
        throw std::runtime_error(pn::format("Couldn't load requested url {0}", url).c_str());
    }
}

}  // namespace http
}  // namespace antares
