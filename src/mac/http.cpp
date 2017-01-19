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
#include <sfz/sfz.hpp>

#include "mac/core-foundation.hpp"

using sfz::Exception;
using sfz::StringSlice;
using sfz::WriteTarget;
using sfz::format;

namespace antares {
namespace http {

void get(const StringSlice& url, WriteTarget out) {
    cf::Url  cfurl(url);
    cf::Data cfdata;
    SInt32   error;
    if (CFURLCreateDataAndPropertiesFromResource(
                NULL, cfurl.c_obj(), &cfdata.c_obj(), NULL, NULL, &error)) {
        write(out, cfdata.data());
    } else {
        throw Exception(format("Couldn't load requested url {0}", url));
    }
}

}  // namespace http
}  // namespace antares
