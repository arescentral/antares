// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2023 The Antares Authors
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

#ifndef ANTARES_DATA_MAP_HPP_
#define ANTARES_DATA_MAP_HPP_

#include <pn/fwd>
#include <vector>

#include "data/field.hpp"

namespace antares {

template <typename T>
class id_map : public std::vector<T> {};

template <typename T, T (*F)(path_value x)>
static id_map<T> optional_map(path_value x) {
    id_map<T> result;
    if (x.value().is_map()) {
        for (pn::key_value_cref kv : x.value().as_map()) {
            auto x = F(kv.value());
            x.set_id(kv.key().copy());
            result.emplace_back(std::move(x));
        }
    } else if (x.value().is_array()) {
        int i = 0;
        for (pn::value_cref v : x.value().as_array()) {
            auto x = F(v);
            x.set_id(pn::dump(i++, pn::dump_short));
            result.emplace_back(std::move(F(v)));
        }
    } else if (!x.value().is_null()) {
        throw std::runtime_error(pn::format(
                                         "{0}must be map, array, or null (was {1})", x.prefix(),
                                         type_string(x.value()))
                                         .c_str());
    }
    return result;
}

template <typename T>
struct field_reader<id_map<T>> {
    static id_map<T> read(path_value x) { return optional_map<T, field_reader<T>::read>(x); }
};

}  // namespace antares

#endif  // ANTARES_DATA_MAP_HPP_
