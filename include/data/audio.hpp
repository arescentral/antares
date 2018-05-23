// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2015-2017 The Antares Authors
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

#ifndef ANTARES_DATA_SNDFILE_HPP_
#define ANTARES_DATA_SNDFILE_HPP_

#include <pn/data>

namespace antares {

struct SoundData {
    pn::data data;  // 16-bit signed LPCM
    int      channels;
    int      frequency;
};

namespace sndfile {
SoundData convert(pn::data_view in);
}  // namespace sndfile

namespace modplug {
SoundData convert(pn::data_view in);
}  // namespace modplug

}  // namespace antares

#endif  // ANTARES_DATA_SNDFILE_HPP_
