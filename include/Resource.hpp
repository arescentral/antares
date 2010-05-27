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

#ifndef ANTARES_RESOURCE_HPP_
#define ANTARES_RESOURCE_HPP_

#include <stdint.h>
#include "sfz/sfz.hpp"

namespace antares {

class Resource {
  public:
    Resource(const sfz::StringPiece& type, const sfz::StringPiece& extension, int id);
    ~Resource();

    sfz::BytesPiece data() const;

  private:
    sfz::scoped_ptr<sfz::MappedFile> _file;
};

}  // namespace antares

#endif // ANTARES_RESOURCE_HPP_
