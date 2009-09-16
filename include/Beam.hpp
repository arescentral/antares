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

#ifndef ANTARES_BEAM_HPP_
#define ANTARES_BEAM_HPP_

// Beam.h

#include "SpaceObject.hpp"

#pragma options align=mac68k

short InitBeams( void);
void CleanupBeams( void);
void ResetBeams( void);
beamType *AddBeam(coordPointType *, unsigned char, beamKindType, long, long, long *);
void SetSpecialBeamAttributes( spaceObjectType *, spaceObjectType *);
void DrawAllBeams( void);
void EraseAllBeams( void);
void ShowAllBeams( void);
void CullBeams( void);

#pragma options align=reset

#endif // ANTARES_BEAM_HPP_
