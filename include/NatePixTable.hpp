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

#ifndef ANTARES_NATE_PIX_TABLE_HPP_
#define ANTARES_NATE_PIX_TABLE_HPP_

// NatePixTable.h

//#include "NatePix.hpp"
#include "SpriteHandling.hpp"

#pragma options align=mac68k

struct natePixType;

natePixType** CreateNatePixTable( void);
void MoveNatePixTableData(natePixType**, long, long);
unsigned long GetNatePixTableSize(natePixType**);
void SetNatePixTableSize(natePixType**, unsigned long);
long GetNatePixTablePixNum(natePixType**);
void SetNatePixTablePixNum(natePixType**, long);
unsigned long GetNatePixTablePixOffset(natePixType**, long);
void SetNatePixTablePixOffset(natePixType**, long, unsigned long);
int GetNatePixTableNatePixWidth(natePixType**, long);
void SetNatePixTableNatePixWidth(natePixType**, long, int);
int GetNatePixTableNatePixHeight(natePixType**, long);
void SetNatePixTableNatePixHeight(natePixType**, long, int);
int GetNatePixTableNatePixHRef(natePixType**, long);
void SetNatePixTableNatePixHRef(natePixType**, long, int);
int GetNatePixTableNatePixVRef(natePixType**, long);
void SetNatePixTableNatePixVRef(natePixType**, long, int);
char *GetNatePixTableNatePixData(natePixType**, long);
unsigned char GetNatePixTableNatePixDataPixel(natePixType**, long, int, int);
void SetNatePixTableNatePixDataPixel(natePixType**, long, int, int, unsigned char);
Handle GetNatePixTableNatePixDataCopy(natePixType**, long);
// void GetNatePixTableNatePixPtr( natePix *, Handle, int);
// void GetNatePixTableNatePixDuplicate( natePix *, Handle, int);
unsigned long GetNatePixTableNatePixDataSize(natePixType**, long);
void InsertNatePix(natePixType**, Rect *, int);
void DeleteNatePix(natePixType**, int);
void RemapNatePixTableColor(natePixType**);
void ColorizeNatePixTableColor(natePixType**, unsigned char);
void RetromapNatePixTableColor(natePixType**);

#pragma options align=reset

#endif // ANTARES_NATE_PIX_TABLE_HPP_
