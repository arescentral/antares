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

#ifndef ANTARES_GAME_INPUT_SOURCE_HPP_
#define ANTARES_GAME_INPUT_SOURCE_HPP_

#include <stdint.h>
#include <sfz/sfz.hpp>

#include "config/keys.hpp"
#include "ui/event.hpp"

namespace antares {

struct ReplayData;

class InputSource {
  public:
    virtual ~InputSource();

    virtual bool next(EventReceiver& key_map) = 0;
};

class ReplayInputSource : public InputSource {
  public:
    explicit ReplayInputSource(ReplayData* data);

    virtual bool next(EventReceiver& receiver);

  private:
    bool advance(EventReceiver& receiver);

    const ReplayData* _data;
    size_t _data_index;
    uint32_t _wait_ticks;
    KeyMap _key_map;

    DISALLOW_COPY_AND_ASSIGN(ReplayInputSource);
};

}  // namespace antares

#endif  // ANTARES_GAME_INPUT_SOURCE_HPP_
