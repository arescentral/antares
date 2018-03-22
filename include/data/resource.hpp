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

#ifndef ANTARES_DATA_RESOURCE_HPP_
#define ANTARES_DATA_RESOURCE_HPP_

#include <stdint.h>
#include <pn/string>
#include <sfz/sfz.hpp>
#include <vector>

#include "data/enums.hpp"

namespace antares {

class Texture;
class NatePixTable;

class Resource {
  public:
    static bool     exists(pn::string_view path);
    static Resource path(pn::string_view path);

    static Resource                font(pn::string_view name);
    static Resource                interface(pn::string_view name);
    static Resource                replay(int id);
    static std::vector<int32_t>    rotation_table();
    static NatePixTable            sprite(pn::string_view name, Hue hue);
    static std::vector<pn::string> strings(int id);
    static pn::string              text(int id);
    static Texture                 texture(pn::string_view name);
    static Texture                 texture(int16_t id);

    Resource(Resource&&) = default;
    Resource& operator=(Resource&&) = default;

    ~Resource();

    pn::data_view   data() const;
    pn::string_view string() const;

  private:
    static pn::value procyon(pn::string_view path);

    Resource(std::unique_ptr<sfz::mapped_file> file);
    std::unique_ptr<sfz::mapped_file> _file;
};

}  // namespace antares

#endif  // ANTARES_DATA_RESOURCE_HPP_
