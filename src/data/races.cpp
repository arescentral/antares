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

#include "data/races.hpp"

#include <sfz/sfz.hpp>

#include "data/resource.hpp"
#include "data/scenario.hpp"
#include "game/globals.hpp"
#include "lang/defines.hpp"

using sfz::BytesSlice;
using sfz::Exception;
using sfz::ReadSource;
using sfz::read;
using std::unique_ptr;

namespace antares {

static const int16_t kRaceResID = 500;

static ANTARES_GLOBAL unique_ptr<Race[]> gRaceData;

void InitRaces() {
    if (gRaceData.get() == NULL) {
        Resource rsrc("races", "race", kRaceResID);
        BytesSlice in(rsrc.data());
        size_t count = rsrc.data().size() / Race::byte_size;
        if (count != kRaceNum) {
            throw Exception("got unexpected number of races");
        }
        gRaceData.reset(new Race[count]);
        for (size_t i = 0; i < count; ++i) {
            read(in, gRaceData[i]);
        }
        if (!in.empty()) {
            throw Exception("didn't consume all of race data");
        }
    }
}

void CleanupRaces() {
    gRaceData.reset();
}

int16_t GetRaceIDFromNum(size_t raceNum) {
    if (raceNum < kRaceNum) {
        return gRaceData[raceNum].id;
    } else {
        return -1;
    }
}

void read_from(ReadSource in, Race& race) {
    read(in, race.id);
    read(in, race.apparentColor);
    in.shift(1);
    read(in, race.illegalColors);
    read(in, race.advantage);
}

}  // namespace antares
