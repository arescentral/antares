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

#ifndef ANTARES_INPUT_SOURCE_HPP_
#define ANTARES_INPUT_SOURCE_HPP_

#include <stdint.h>
#include "sfz/BinaryReader.hpp"
#include "sfz/Macros.hpp"
#include "Resource.hpp"

namespace antares {

class InputSource {
  public:
    virtual ~InputSource();

    virtual uint32_t random_seed() const = 0;
    virtual bool next(uint32_t* key_map) = 0;
};

class ReplayInputSource : public InputSource {
  public:
    ReplayInputSource(int32_t id);

    virtual uint32_t random_seed() const;
    virtual bool next(uint32_t* key_map);

  private:
    Resource _resource;
    sfz::BytesBinaryReader _bin;

    uint32_t _random_seed;
    uint32_t _turn_num;
    uint32_t _keys;

    DISALLOW_COPY_AND_ASSIGN(ReplayInputSource);
};

}  // namespace antares

#endif  // ANTARES_INPUT_SOURCE_HPP_
