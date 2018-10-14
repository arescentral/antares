// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2017 The Antares Authors
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

#ifndef ANTARES_MAC_C_ANTARES_CONTROLLER_H_
#define ANTARES_MAC_C_ANTARES_CONTROLLER_H_

#include <CoreFoundation/CoreFoundation.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AntaresDrivers AntaresDrivers;
AntaresDrivers*               antares_controller_create_drivers(CFStringRef* error_message);
void                          antares_controller_destroy_drivers(AntaresDrivers* drivers);
bool antares_controller_loop(AntaresDrivers* drivers, CFStringRef* error_message);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // ANTARES_MAC_C_ANTARES_CONTROLLER_H_
