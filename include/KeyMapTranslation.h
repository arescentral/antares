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

// Key Map Translation.h
#ifndef __CONDITIONALMACROS__
#include "ConditionalMacros.h"
#endif // __CONDITIONALMACROS__

#pragma options align=mac68k

#define kKeyMapNameID           1000
#define kKeyMapNameLongID       1002

#define kKeyNameLength      4

#if TARGET_OS_MAC
#define mF1Key(mKeyMap)         (((mKeyMap)[3] >> 2) & 0x01 )       // F1
#define mF2Key(mKeyMap)         ((mKeyMap)[3] & 0x01 )          // F2
#define mF3Key(mKeyMap)         (((mKeyMap)[3] >> 27) & 0x01 )  // F3
#define mF4Key(mKeyMap)         (((mKeyMap)[3] >> 14) & 0x01 )  // F4

#define mF5Key(mKeyMap)         (((mKeyMap)[3] >> 24) & 0x01 )  // F5
#define mF6Key(mKeyMap)         (((mKeyMap)[3] >> 25) & 0x01 )  // F6
#define mF7Key(mKeyMap)         (((mKeyMap)[3] >> 26) & 0x01 )  // F7
#define mF8Key(mKeyMap)         (((mKeyMap)[3] >> 28) & 0x01 )  // F8

#define mF9Key(mKeyMap)         (((mKeyMap)[3] >> 29) & 0x01 )  // F9
#define mF10Key(mKeyMap)        (((mKeyMap)[3] >> 21) & 0x01 )  // F10
#define mF11Key(mKeyMap)        (((mKeyMap)[3] >> 31) & 0x01 )  // F11
#define mF12Key(mKeyMap)        (((mKeyMap)[3] >> 23) & 0x01 )  // F12

#define mF13Key(mKeyMap)        (((mKeyMap)[3] >> 17) & 0x01 )  // F13
#define mF14Key(mKeyMap)        (((mKeyMap)[3] >> 19) & 0x01 )  // F14
#define mF15Key(mKeyMap)        (((mKeyMap)[3] >> 9) & 0x01 )       // F15

#define m1Key(mKeyMap)          (((mKeyMap)[0] >> 10) & 0x01 )      // 1
#define m2Key(mKeyMap)          (((mKeyMap)[0] >> 11) & 0x01 )  // 2
#define m3Key(mKeyMap)          (((mKeyMap)[0] >> 12) & 0x01 )  // 3
#define m4Key(mKeyMap)          (((mKeyMap)[0] >> 13) & 0x01 )  // 4
#define m5Key(mKeyMap)          (((mKeyMap)[0] >> 15) & 0x01 )  // 5
#define m6Key(mKeyMap)          (((mKeyMap)[0] >> 14) & 0x01 )  // 6
#define m7Key(mKeyMap)          (((mKeyMap)[0] >> 2) & 0x01 )   // 7
#define m8Key(mKeyMap)          (((mKeyMap)[0] >> 4) & 0x01 )   // 8
#define m9Key(mKeyMap)          (((mKeyMap)[0] >> 1) & 0x01 )   // 9
#define m0Key(mKeyMap)          (((mKeyMap)[0] >> 5) & 0x01 )   // 0
#define mMinusKey(mKeyMap)      (((mKeyMap)[0] >> 3) & 0x01 )   // -
#define mPlusKey(mKeyMap)       ((mKeyMap)[0] & 0x01)           // +


#define mDeleteKey(mKeyMap)     (((mKeyMap)[1] >> 11) & 0x01 )  // delete
#define mCapsLockKey( mKeyMap)  (((mKeyMap)[1] >> 1) & 0x01 )   // CAPS-LOCK
#define mReturnKey( mKeyMap)    (((mKeyMap)[1] >> 28) & 0x01 )  // return
#define mEscKey( mKeyMap)       (((mKeyMap)[1] >> 13) & 0x01 )  // esc key
#define mQKey( mKeyMap)         (((mKeyMap)[0] >> 20) & 0x01)   // Q for Quitting
#define mCommandKey( mKeyMap)   (((mKeyMap)[1] >> 15) & 0x01)   // command key
#define mLeftArrowKey( mKeyMap) (((mKeyMap)[3] >> 3) & 0x01 )   // left arrow/backspace
#define mAnythingButReturn
#define kModifierKeyMask    (0x0008 | 0x8000 | 0x0004 | 0x0001 | 0x0002) // applies only to keyMap[1]

#else   // not MAC!

#define mF1Key(mKeyMap)         (((EndianU32_BtoN((mKeyMap)[3].bigEndianValue)) >> 2) & 0x01 )      // F1
#define mF2Key(mKeyMap)         ((EndianU32_BtoN((mKeyMap)[3].bigEndianValue)) & 0x01 )         // F2
#define mF3Key(mKeyMap)         (((EndianU32_BtoN((mKeyMap)[3].bigEndianValue)) >> 27) & 0x01 ) // F3
#define mF4Key(mKeyMap)         (((EndianU32_BtoN((mKeyMap)[3].bigEndianValue)) >> 14) & 0x01 ) // F4

#define mF5Key(mKeyMap)         (((EndianU32_BtoN((mKeyMap)[3].bigEndianValue)) >> 24) & 0x01 ) // F5
#define mF6Key(mKeyMap)         (((EndianU32_BtoN((mKeyMap)[3].bigEndianValue)) >> 25) & 0x01 ) // F6
#define mF7Key(mKeyMap)         (((EndianU32_BtoN((mKeyMap)[3].bigEndianValue)) >> 26) & 0x01 ) // F7
#define mF8Key(mKeyMap)         (((EndianU32_BtoN((mKeyMap)[3].bigEndianValue)) >> 28) & 0x01 ) // F8

#define mF9Key(mKeyMap)         (((EndianU32_BtoN((mKeyMap)[3].bigEndianValue)) >> 29) & 0x01 ) // F9
#define mF10Key(mKeyMap)        (((EndianU32_BtoN((mKeyMap)[3].bigEndianValue)) >> 21) & 0x01 ) // F10
#define mF11Key(mKeyMap)        (((EndianU32_BtoN((mKeyMap)[3].bigEndianValue)) >> 31) & 0x01 ) // F11
#define mF12Key(mKeyMap)        (((EndianU32_BtoN((mKeyMap)[3].bigEndianValue)) >> 23) & 0x01 ) // F12

#define mF13Key(mKeyMap)        (((EndianU32_BtoN((mKeyMap)[3].bigEndianValue)) >> 17) & 0x01 ) // F13
#define mF14Key(mKeyMap)        (((EndianU32_BtoN((mKeyMap)[3].bigEndianValue)) >> 19) & 0x01 ) // F14
#define mF15Key(mKeyMap)        (((EndianU32_BtoN((mKeyMap)[3].bigEndianValue)) >> 9) & 0x01 )      // F15

#define m1Key(mKeyMap)          (((EndianU32_BtoN((mKeyMap)[0].bigEndianValue)) >> 10) & 0x01 )     // 1
#define m2Key(mKeyMap)          (((EndianU32_BtoN((mKeyMap)[0].bigEndianValue)) >> 11) & 0x01 ) // 2
#define m3Key(mKeyMap)          (((EndianU32_BtoN((mKeyMap)[0].bigEndianValue)) >> 12) & 0x01 ) // 3
#define m4Key(mKeyMap)          (((EndianU32_BtoN((mKeyMap)[0].bigEndianValue)) >> 13) & 0x01 ) // 4
#define m5Key(mKeyMap)          (((EndianU32_BtoN((mKeyMap)[0].bigEndianValue)) >> 15) & 0x01 ) // 5
#define m6Key(mKeyMap)          (((EndianU32_BtoN((mKeyMap)[0].bigEndianValue)) >> 14) & 0x01 ) // 6
#define m7Key(mKeyMap)          (((EndianU32_BtoN((mKeyMap)[0].bigEndianValue)) >> 2) & 0x01 )  // 7
#define m8Key(mKeyMap)          (((EndianU32_BtoN((mKeyMap)[0].bigEndianValue)) >> 4) & 0x01 )  // 8
#define m9Key(mKeyMap)          (((EndianU32_BtoN((mKeyMap)[0].bigEndianValue)) >> 1) & 0x01 )  // 9
#define m0Key(mKeyMap)          (((EndianU32_BtoN((mKeyMap)[0].bigEndianValue)) >> 5) & 0x01 )  // 0
#define mMinusKey(mKeyMap)      (((EndianU32_BtoN((mKeyMap)[0].bigEndianValue)) >> 3) & 0x01 )  // -
#define mPlusKey(mKeyMap)       ((EndianU32_BtoN((mKeyMap)[0].bigEndianValue)) & 0x01)          // +


#define mDeleteKey(mKeyMap)     (((EndianU32_BtoN((mKeyMap)[1].bigEndianValue)) >> 11) & 0x01 ) // delete
#define mCapsLockKey( mKeyMap)  (((EndianU32_BtoN((mKeyMap)[1].bigEndianValue)) >> 1) & 0x01 )  // CAPS-LOCK
#define mReturnKey( mKeyMap)    (((EndianU32_BtoN((mKeyMap)[1].bigEndianValue)) >> 28) & 0x01 ) // return
#define mEscKey( mKeyMap)       (((EndianU32_BtoN((mKeyMap)[1].bigEndianValue)) >> 13) & 0x01 ) // esc key
#define mQKey( mKeyMap)         (((EndianU32_BtoN((mKeyMap)[0].bigEndianValue)) >> 20) & 0x01)  // Q for Quitting
#define mCommandKey( mKeyMap)   (((EndianU32_BtoN((mKeyMap)[1].bigEndianValue)) >> 15) & 0x01)  // command key
#define mLeftArrowKey( mKeyMap) (((EndianU32_BtoN((mKeyMap)[3].bigEndianValue)) >> 3) & 0x01 )  // left arrow/backspace
#define mAnythingButReturn
#define kModifierKeyMask    (0x0008 | 0x8000 | 0x0004 | 0x0001 | 0x0002) // applies only to keyMap[1]

#endif

void GetKeyMapFromKeyNum( short, KeyMap);
short GetKeyNumFromKeyMap( KeyMap);
Boolean DoesKeyMapContainKeyNum( KeyMap, short);
void WaitForAnyEvent( void);
Boolean TimedWaitForAnyEvent( long);
Boolean AnyEvent( void);
Boolean ControlKey( void);
Boolean CommandKey( void);
Boolean OptionKey( void);
Boolean ShiftKey( void);
Boolean EscapeKey( void);
Boolean PeriodKey( void);
Boolean QKey( void);
Boolean AnyCancelKeys( void);
void GetKeyNumName( StringPtr, short);
Boolean AnyRealKeyDown( void);
Boolean AnyModifierKeyDown( void);
Boolean AnyKeyButThisOne( KeyMap, long, long);
long GetAsciiFromKeyMap( KeyMap, KeyMap);
long GetAsciiFromKeyNum( short);

#pragma options align=reset
