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

#ifndef ANTARES_RADAR_HPP_
#define ANTARES_RADAR_HPP_

// Radar.h

#pragma options align=mac68k

#define kMiniBuildTimeHeight    ( 25)

int InstrumentInit( void);
void UpdateRadar( long);
void InstrumentCleanup( void);
void ResetInstruments( void);
void ResetSectorLines( void);
void DrawInstrumentPanel( WindowPtr);
//void EraseDrawMainSectorLines( void);
//void ShowMainSectorLines( void);
void EraseSite( void);
void EraseSectorLines( void);
void DrawSite( void);
void DrawSectorLines( void);
void ShowSite( void);
void ShowSectorLines( void);
void InstrumentsHandleClick( void);
void InstrumentsHandleDoubleClick( void);
void InstrumentsHandleMouseUp( void);
void InstrumentsHandleMouseStillDown( void);
void DrawArbitrarySectorLines( coordPointType *, long, long, Rect *, PixMapHandle, long, long);
void GetArbitrarySingleSectorBounds( coordPointType *, coordPointType *, long, long, Rect *,
                                Rect *);
void UpdateBarIndicator( short, long, long, PixMapHandle);
void DrawBuildTimeBar( long);

#pragma options align=reset

#endif // ANTARES_RADAR_HPP_
