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

#ifndef ANTARES_NATE_PIX_TABLE_HPP_
#define ANTARES_NATE_PIX_TABLE_HPP_

/* NatePixTable.h */

//#include "NatePix.h"
#include "SpriteHandling.h"

#pragma options align=mac68k

Handle CreateNatePixTable( void);
void MoveNatePixTableData( Handle, long, long);
unsigned long GetNatePixTableSize( Handle);
void SetNatePixTableSize( Handle, unsigned long);
long GetNatePixTablePixNum( Handle);
void SetNatePixTablePixNum( Handle, long);
unsigned long GetNatePixTablePixOffset( Handle, long);
void SetNatePixTablePixOffset( Handle, long, unsigned long);
int GetNatePixTableNatePixWidth( Handle, long);
void SetNatePixTableNatePixWidth( Handle, long, int);
int GetNatePixTableNatePixHeight( Handle, long);
void SetNatePixTableNatePixHeight( Handle, long, int);
int GetNatePixTableNatePixHRef( Handle, long);
void SetNatePixTableNatePixHRef( Handle, long, int);
int GetNatePixTableNatePixVRef( Handle, long);
void SetNatePixTableNatePixVRef( Handle, long, int);
char *GetNatePixTableNatePixData( Handle, long);
unsigned char GetNatePixTableNatePixDataPixel( Handle, long, int, int);
void SetNatePixTableNatePixDataPixel( Handle, long, int, int, unsigned char);
Handle GetNatePixTableNatePixDataCopy( Handle, long);
/*
void GetNatePixTableNatePixPtr( natePix *, Handle, int);
void GetNatePixTableNatePixDuplicate( natePix *, Handle, int);
*/
unsigned long GetNatePixTableNatePixDataSize( Handle, long);
void InsertNatePix( Handle, Rect *, int);
void DeleteNatePix( Handle, int);
void RemapNatePixTableColor( Handle);
void ColorizeNatePixTableColor( Handle, unsigned char);
void RetromapNatePixTableColor( Handle);

#pragma options align=reset

#endif // ANTARES_NATE_PIX_TABLE_HPP_
