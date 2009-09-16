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

#ifndef ANTARES_MINICOMPUTER_HPP_
#define ANTARES_MINICOMPUTER_HPP_

// Minicomputer.h

#include <Base.h>

#include "SpaceObject.hpp"

int MiniScreenInit( void);
void MiniScreenCleanup( void);
void SetMiniScreenStatusStrList( short);
void DisposeMiniScreenStatusStrList( void);
void ClearMiniScreenLines( void);
void ClearMiniObjectData( void);
void DrawMiniScreen( void);
void DrawAndShowMiniScreenLine( long);
void ShowWholeMiniScreen( void);
void MakeMiniScreenFromIndString( short);
void MiniComputerHandleKeys( unsigned long, unsigned long);
void MiniComputerHandleNull( long);
long MiniComputerGetPriceOfCurrentSelection( void);
void UpdateMiniScreenLines( void);
void UpdatePlayerAmmo(long, long, long);
void UpdateMiniShipData( spaceObjectType *, spaceObjectType *, unsigned char, short, short);
void MiniComputerDoAccept( void);
void MiniComputerExecute( long, long, long);
void MiniComputerDoCancel( void);
void MiniComputerSetBuildStrings( void);
void MiniComputerHandleClick( Point);
void MiniComputerHandleDoubleClick( Point);
void MiniComputerHandleMouseUp( Point);
void MiniComputerHandleMouseStillDown( Point);
void MiniComputer_SetScreenAndLineHack( long whichScreen, long whichLine);

#endif // ANTARES_MINICOMPUTER_HPP_
