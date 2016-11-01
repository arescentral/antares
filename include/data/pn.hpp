// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2016 The Antares Authors
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

#ifndef ANTARES_DATA_PN_HPP_
#define ANTARES_DATA_PN_HPP_

#include <pn/string>
#include <sfz/sfz.hpp>

namespace antares {

inline sfz::String pn2sfz(pn::string_view s) {
    return sfz::String(
            sfz::utf8::decode(sfz::Bytes(reinterpret_cast<const uint8_t*>(s.data()), s.size())));
}

inline pn::string sfz2pn(sfz::StringSlice s) {
    sfz::Bytes bytes(sfz::utf8::encode(s));
    return pn::string(reinterpret_cast<char*>(bytes.data()), bytes.size());
}

template <typename T>
inline pn::string sfz2pn(sfz::EncodedString<T> s) {
    return sfz2pn(sfz::String(s));
}
inline pn::string sfz2pn(sfz::Format<16> f) { return sfz2pn(sfz::String(f)); }
inline pn::string sfz2pn(int i) { return sfz2pn(sfz::String(i)); }
inline pn::string sfz2pn(double d) { return sfz2pn(sfz::String(d)); }

}  // namespace antares

#endif  // ANTARES_DATA_PN_HPP_
