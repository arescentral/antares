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

#ifndef ANTARES_MATH_SPECIAL_HPP_
#define ANTARES_MATH_SPECIAL_HPP_

// Math Special.h

#include <Base.h>

#include "ConditionalMacros.h"

#pragma options align=mac68k

#define kMathSpecial

#ifndef powerc
#define WideSubtract    MyWideSubtract
#endif

//
//  MAX VALUE FOR SMALLFIXEDTYPE:
//
//  8,388,607       normal
//  4,194,303       addition
//  2,896           multiplication -- see below
//  32,768          division
//

#define kFixedOneQuarter            0x00000040
#define kFixedPlusPointFive         0x00000080
#define kFixedNegativePointFive     0xffffff80
#define kFixedOne                   0x00000100
#define kFixedNegativeOne           0xfffff100
#define kFixedTwo                   0x00000200
#define kFixedNegativeTwo           0xfffff200

#define kFixedWholeMultiplier       256
#define kFixedBitShiftNumber        (long)8

#define mLongToFixed( m_l)          ((m_l) << kFixedBitShiftNumber)
#define mFloatToFixed( m_r)         (m_r) * (float)(kFixedWholeMultiplier)
#define mFixedToFloat( m_f)         ((float)(m_f) / (float)kFixedWholeMultiplier)
#define mFixedToLong( m_f)          (( (m_f) < 0) ? (( (m_f) >> kFixedBitShiftNumber) + 1):( (m_f) >> kFixedBitShiftNumber))

// the max safe # we can do is 181 for signed multiply if we don't know other value
// if -1 <= other value <= 1 then we can do 32767
#define mMultiplyFixed( m_f1, m_f2)     (( (m_f1) * (m_f2)) >> kFixedBitShiftNumber)
//#define   mDivideFixed( f1, f2)       ( (f1) / ((f2) >> kFixedBitShiftNumber))
#define mDivideFixed( m_f1, m_f2)       ( ((m_f1) << kFixedBitShiftNumber) / (m_f2))

typedef long smallFixedType;    // distinct from Mac OS's Fixed type

typedef struct
{
    smallFixedType      h;
    smallFixedType      v;
} fixedPointType;


unsigned long lsqrt (unsigned long);

#ifndef powerc
wide *MyWideSubtract( wide *, const wide *);
#endif
#ifdef powerc
Fixed MyFixRatio( short, short);
#endif
void MyMulDoubleLong( long, long, wide *);
void MyWideAddC( wide *, const wide *);
#ifdef powerc
#define MyWideAdd( mtarget, msource)    WideAdd( (wide *)mtarget, (wide *)msource)
#else
asm void MyWideAdd( UnsignedWide *, const UnsignedWide *);
#endif
#define mWideIsGreaterThan( mleft, mright) (((mleft).hi==(mright).hi)?((mleft).lo>(mright).lo):((mleft).hi>(mright).hi))
#define mWideIsGreaterThanOrEqual( mleft, mright) (((mleft).hi==(mright).hi)?((mleft).lo>=(mright).lo):((mleft).hi>(mright).hi))

#if TARGET_OS_MAC
    #ifdef powerc
    #define MyWideMul( mlong1, mlong2, mwide) WideMultiply( mlong1, mlong2, (wide *)mwide)
    #else
    #define MyWideMul( mlong1, mlong2, mwide) LongMul( mlong1, mlong2, (Int64Bit *)mwide)
    #endif powerc
#else
    #define MyWideMul( mlong1, mlong2, mwide) WideMultiply( mlong1, mlong2, (wide *)mwide)
#endif TARGET_OS_MAC

#pragma options align=reset

#endif // ANTARES_MATH_SPECIAL_HPP_
