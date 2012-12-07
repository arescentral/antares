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

#ifndef ANTARES_DATA_REPLAY_HPP_
#define ANTARES_DATA_REPLAY_HPP_

#include <stdint.h>
#include <vector>
#include <sfz/sfz.hpp>

#include "data/resource.hpp"
#include "game/main.hpp"
#include "ui/card.hpp"

namespace antares {

struct ReplayData {
    int32_t chapter_id;
    int32_t global_seed;

    struct Item {
        enum Type {
            WAIT = 0,
            KEY_DOWN = 1,
            KEY_UP = 2,
        };
        Type type;
        union {
            uint32_t wait;
            uint8_t key_down;
            uint8_t key_up;
        } data;
    };
    std::vector<Item> items;

    ReplayData();
    ReplayData(sfz::BytesSlice in);

    void wait(uint32_t ticks);
    void key_down(uint32_t key);
    void key_up(uint32_t key);
};
void read_from(sfz::ReadSource in, ReplayData& replay);
void read_from(sfz::ReadSource in, ReplayData::Item& replay);
void write_to(sfz::WriteTarget out, const ReplayData& replay);
void write_to(sfz::WriteTarget out, const ReplayData::Item& replay);

}  // namespace antares

#endif  // ANTARES_DATA_REPLAY_HPP_
