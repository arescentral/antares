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

#ifndef ANTARES_KEY_MAP_TRANSLATION_HPP_
#define ANTARES_KEY_MAP_TRANSLATION_HPP_

#include "Base.h"
#include "sfz/String.hpp"

namespace antares {

#define kKeyMapNameID           1000
#define kKeyMapNameLongID       1002

#define kKeyNameLength      4

inline bool mF1Key(KeyMap km)         { return (km[3] >> 2) & 0x01; }       // F1
inline bool mF2Key(KeyMap km)         { return km[3] & 0x01; }          // F2
inline bool mF3Key(KeyMap km)         { return (km[3] >> 27) & 0x01; }  // F3
inline bool mF4Key(KeyMap km)         { return (km[3] >> 14) & 0x01; }  // F4

inline bool mF5Key(KeyMap km)         { return (km[3] >> 24) & 0x01; }  // F5
inline bool mF6Key(KeyMap km)         { return (km[3] >> 25) & 0x01; }  // F6
inline bool mF7Key(KeyMap km)         { return (km[3] >> 26) & 0x01; }  // F7
inline bool mF8Key(KeyMap km)         { return (km[3] >> 28) & 0x01; }  // F8

inline bool mF9Key(KeyMap km)         { return (km[3] >> 29) & 0x01; }  // F9
inline bool mF10Key(KeyMap km)        { return (km[3] >> 21) & 0x01; }  // F10
inline bool mF11Key(KeyMap km)        { return (km[3] >> 31) & 0x01; }  // F11
inline bool mF12Key(KeyMap km)        { return (km[3] >> 23) & 0x01; }  // F12

inline bool mF13Key(KeyMap km)        { return (km[3] >> 17) & 0x01; }  // F13
inline bool mF14Key(KeyMap km)        { return (km[3] >> 19) & 0x01; }  // F14
inline bool mF15Key(KeyMap km)        { return (km[3] >> 9) & 0x01; }       // F15

inline bool m1Key(KeyMap km)          { return (km[0] >> 10) & 0x01; }      // 1
inline bool m2Key(KeyMap km)          { return (km[0] >> 11) & 0x01; }  // 2
inline bool m3Key(KeyMap km)          { return (km[0] >> 12) & 0x01; }  // 3
inline bool m4Key(KeyMap km)          { return (km[0] >> 13) & 0x01; }  // 4
inline bool m5Key(KeyMap km)          { return (km[0] >> 15) & 0x01; }  // 5
inline bool m6Key(KeyMap km)          { return (km[0] >> 14) & 0x01; }  // 6
inline bool m7Key(KeyMap km)          { return (km[0] >> 2) & 0x01; }   // 7
inline bool m8Key(KeyMap km)          { return (km[0] >> 4) & 0x01; }   // 8
inline bool m9Key(KeyMap km)          { return (km[0] >> 1) & 0x01; }   // 9
inline bool m0Key(KeyMap km)          { return (km[0] >> 5) & 0x01; }   // 0
inline bool mMinusKey(KeyMap km)      { return (km[0] >> 3) & 0x01; }   // -
inline bool mPlusKey(KeyMap km)       { return (km[0] & 0x01); }         // +


inline bool mDeleteKey(KeyMap km)     { return (km[1] >> 11) & 0x01; }  // delete
inline bool mCapsLockKey(KeyMap km)   { return (km[1] >> 1) & 0x01; }   // CAPS-LOCK
inline bool mReturnKey(KeyMap km)     { return (km[1] >> 28) & 0x01; }  // return
inline bool mEscKey(KeyMap km)        { return (km[1] >> 13) & 0x01; }  // esc key
inline bool mQKey(KeyMap km)          { return (km[0] >> 20) & 0x01; }   // Q for Quitting
inline bool mCommandKey(KeyMap km)    { return (km[1] >> 15) & 0x01; }   // command key
inline bool mLeftArrowKey(KeyMap km)  { return (km[3] >> 3) & 0x01; }   // left arrow/backspace
#define mAnythingButReturn
#define kModifierKeyMask    (0x0008 | 0x8000 | 0x0004 | 0x0001 | 0x0002) // applies only to keyMap[1]

void GetKeyMapFromKeyNum( short, KeyMap);
short GetKeyNumFromKeyMap( KeyMap);
bool DoesKeyMapContainKeyNum( KeyMap, short);
void WaitForAnyEvent( void);
bool TimedWaitForAnyEvent( long);
bool AnyEvent( void);
bool ControlKey( void);
bool CommandKey( void);
bool OptionKey( void);
bool ShiftKey( void);
bool EscapeKey( void);
bool PeriodKey( void);
bool QKey( void);
bool AnyCancelKeys( void);
void GetKeyNumName(short key_num, sfz::String* out);
bool AnyRealKeyDown( void);
bool AnyModifierKeyDown( void);
bool AnyKeyButThisOne( KeyMap, long, long);
long GetAsciiFromKeyMap( KeyMap, KeyMap);
long GetAsciiFromKeyNum( short);

}  // namespace antares

#endif // ANTARES_KEY_MAP_TRANSLATION_HPP_
