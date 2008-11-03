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

#ifndef ANTARES_ANY_CHAR_HPP_
#define ANTARES_ANY_CHAR_HPP_

// AnyChar.h

/* ONE BYTE CHARACTER SYSTEM */

#pragma options align=mac68k

#define kAnyCharSpace   ' '

#define kAnyCharPStringMaxLen   254

typedef unsigned char anyCharType;

#define mGetAnyCharPStringLength( mAnyCharPtr) (*mAnyCharPtr)
#define mGetPStringFromSingleAnyChar( mDestAnyCharPtr, mAnyChar)\
{\
    *mDestAnyCharPtr = 1;\
    *(mDestAnyCharPtr + 1) = mAnyChar;\
}

void CopyAnyCharPString( anyCharType *, anyCharType *);
void InsertAnyCharPStringInPString( anyCharType *, anyCharType *, long);
void CutCharsFromAnyCharPString( anyCharType *, long, long);

#pragma options align=reset

#endif // ANTARES_ANY_CHAR_HPP_
