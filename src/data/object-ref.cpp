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

#include "data/object-ref.hpp"

#include <pn/map>
#include <pn/value>

#include "data/field.hpp"

namespace antares {

DEFINE_FIELD_READER(ObjectRef) {
    if (!x.value().is_map()) {
        throw std::runtime_error(pn::format("{0}must be map", x.prefix()).c_str());
    }

    pn::map_cref m = x.value().as_map();
    if (m.size() != 1) {
        throw std::runtime_error(pn::format("{0}must have single element", x.prefix()).c_str());
    }
    pn::key_value_cref kv = *m.begin();

    ObjectRef o;
    if (kv.key() == "initial") {
        o.type = ObjectRef::Type::INITIAL;
    } else if (kv.key() == "flagship") {
        o.type = ObjectRef::Type::FLAGSHIP;
    } else if (kv.key() == "control") {
        o.type = ObjectRef::Type::CONTROL;
    } else if (kv.key() == "target") {
        o.type = ObjectRef::Type::TARGET;
    } else {
        throw std::runtime_error(pn::format(
                                         "{0}key must be one of [\"initial\", \"flagship\", "
                                         "\"control\", \"target\"]",
                                         x.prefix())
                                         .c_str());
    }

    switch (o.type) {
        case ObjectRef::Type::INITIAL:
            o.initial = read_field<Handle<const Initial>>(m.get("initial"));
            break;
        case ObjectRef::Type::FLAGSHIP:
            o.admiral = read_field<Handle<Admiral>>(m.get("flagship"));
            break;
        case ObjectRef::Type::CONTROL:
            o.admiral = read_field<Handle<Admiral>>(m.get("control"));
            break;
        case ObjectRef::Type::TARGET:
            o.admiral = read_field<Handle<Admiral>>(m.get("target"));
            break;
    }
    return o;
}

DEFINE_FIELD_READER(sfz::optional<ObjectRef>) {
    if (x.value().is_null()) {
        return sfz::nullopt;
    } else if (x.value().is_map()) {
        return sfz::make_optional(read_field<ObjectRef>(x));
    } else {
        throw std::runtime_error(pn::format("{0}must be null or map", x.prefix()).c_str());
    }
}

}  // namespace antares
