// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2016-2017 The Antares Authors
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

#ifndef ANTARES_GAME_INITIAL_HPP_
#define ANTARES_GAME_INITIAL_HPP_

#include <sfz/sfz.hpp>

#include "data/handle.hpp"

namespace antares {

struct Initial;
struct ObjectRef;

void                create_initial(Handle<const Initial> initial);
void                set_initial_destination(Handle<const Initial> initial, bool preserve);
void                UnhideInitialObject(Handle<const Initial> initial);
Handle<SpaceObject> GetObjectFromInitialNumber(Handle<const Initial> initial);
Handle<SpaceObject> resolve_object_ref(const ObjectRef& ref);
Handle<SpaceObject> resolve_object_ref(const sfz::optional<ObjectRef>& ref);

}  // namespace antares

#endif  // ANTARES_GAME_INITIAL_HPP_
