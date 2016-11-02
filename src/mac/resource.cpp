// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2013 The Antares Authors
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

#include "data/resource.hpp"

#include <CoreFoundation/CoreFoundation.h>
#include <sfz/sfz.hpp>

#include "mac/core-foundation.hpp"

using sfz::Exception;
using sfz::String;

namespace utf8 = sfz::utf8;

namespace antares {

const String application_path() {
    cf::Url    url(CFBundleCopyResourcesDirectoryURL(CFBundleGetMainBundle()));
    cf::String url_string(CFStringCreateCopy(NULL, CFURLGetString(url.c_obj())));
    char       path_buffer[PATH_MAX];
    if (!CFURLGetFileSystemRepresentation(
                url.c_obj(), true, reinterpret_cast<UInt8*>(path_buffer), PATH_MAX)) {
        throw Exception("couldn't get application_path()");
    }
    return String(utf8::decode(path_buffer));
}

}  // namespace antares
