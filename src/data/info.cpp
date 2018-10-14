// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2018 The Antares Authors
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

#include "data/info.hpp"

#include "data/field.hpp"

namespace antares {

static bool valid_sha1(pn::string_view s) {
    if (s.size() != 40) {
        return false;
    }
    for (pn::rune r : s) {
        if (!(((pn::rune{'0'} <= r) && (r <= pn::rune{'9'})) ||
              ((pn::rune{'a'} <= r) && (r <= pn::rune{'f'})))) {
            return false;
        }
    }
    return true;
}

FIELD_READER(Info::Identifier) {
    auto id = read_field<sfz::optional<pn::string_view>>(x);
    if (id.has_value() && !valid_sha1(*id)) {
        throw std::runtime_error(
                pn::format("{0}invalid identifier (must be lowercase sha1 digest)", x.prefix())
                        .c_str());
    }
    return {id.has_value() ? id->copy() : ""};
}

static Info fill_identifier(Info info) {
    if (!info.identifier.hash.empty()) {
        return info;
    }
    sfz::sha1 sha;
    sha.write(info.title);
    info.identifier.hash = sha.compute().hex();
    return info;
}

Info info(path_value x) {
    return fill_identifier(required_struct<Info>(
            x, {{"title", &Info::title},
                {"identifier", &Info::identifier},
                {"format", &Info::format},
                {"download_url", &Info::download_url},
                {"author", &Info::author},
                {"author_url", &Info::author_url},
                {"version", &Info::version},
                {"intro", &Info::intro},
                {"about", &Info::about}}));
}

}  // namespace antares
