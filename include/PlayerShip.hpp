/*
Ares, a tactical space combat game.
Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef ANTARES_PLAYER_SHIP_HPP_
#define ANTARES_PLAYER_SHIP_HPP_

// Player Ship.h

#pragma options align=mac68k

#define kShapeNum               24
#define kPixResID               501

#define kZoomLevelNum           8

#define kTimesTwoZoom           0
#define kActualSizeZoom         1
#define kHalfSizeZoom           2
#define kQuarterSizeZoom        3
#define kEighthSizeZoom         4
#define kNearestFoeZoom         5
#define kNearestAnythingZoom    6
#define kSmallestZoom           7

void StartPlayerShip( long, short);
void ResetPlayerShip( long);
Boolean PlayerShipGetKeys( long, unsigned long, Boolean *);
void PlayerShipHandleClick( Point);
void SetPlayerSelectShip( long, Boolean, long);
void ChangePlayerShipNumber( long, long);
void TogglePlayerAutoPilot( spaceObjectType *);
Boolean IsPlayerShipOnAutoPilot( void);
void PlayerShipGiveCommand( long);
void PlayerShipBodyExpire( spaceObjectType *, Boolean);
void HandleTextMessageKeys( KeyMap, KeyMap, Boolean *);

#pragma options align=reset

#endif // ANTARES_PLAYER_SHIP_HPP_
