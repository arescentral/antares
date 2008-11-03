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

#ifndef ANTARES_UNIVERSE_UNIT_HPP_
#define ANTARES_UNIVERSE_UNIT_HPP_

// Universe Unit.h

#define kUniversalCenter                1073741823L//92680L
#define kMaximumRelevantDistance        46340L
#define kMaximumRelevantDistanceSquared 2147395600L // maximumrelevant ^ 2
#define kMaximumAngleDistance           32767L      // maximum distance we can calc angle for

#define kSubSectorSize                  512L
#define kSectorSize                     4096L
#define kSuperSectorSize                32768L

#define kSubSectorShift                 9L
#define kSectorShift                    12L
#define kSuperSectorShift               15L

#define kSectorDivision                 16

#endif // ANTARES_UNIVERSE_UNIT_HPP_
