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

#include <stdint.h>
#include "sfz/Macros.hpp"
#include "sfz/String.hpp"

namespace antares {

class KeyMap {
  public:
    KeyMap();

    bool get(size_t index) const;
    void set(size_t index, bool value);

    bool any() const;
    bool equals(const KeyMap& other) const;
    void copy(const KeyMap& other);
    void clear();

  private:
    typedef uint8_t Data[16];
    static const size_t kDataSize = sizeof(Data);

    Data _data;

    DISALLOW_COPY_AND_ASSIGN(KeyMap);
};

bool operator==(const KeyMap& a, const KeyMap& b);
bool operator!=(const KeyMap& a, const KeyMap& b);

void GetKeys(KeyMap* keys);

struct Keys {
    enum Key {
        // Letters, A-Z.
        A           = 0x00,
        B           = 0x0b,
        C           = 0x08,
        D           = 0x02,
        E           = 0x0e,
        F           = 0x03,
        G           = 0x05,
        H           = 0x04,
        I           = 0x22,
        J           = 0x26,
        K           = 0x28,
        L           = 0x25,
        M           = 0x2e,
        N           = 0x2d,
        O           = 0x1f,
        P           = 0x23,
        Q           = 0x0c,
        R           = 0x0f,
        S           = 0x01,
        T           = 0x11,
        U           = 0x20,
        V           = 0x09,
        W           = 0x0d,
        X           = 0x07,
        Y           = 0x10,
        Z           = 0x06,

        // Numbers, 0-9.
        K0          = 0x1d,
        K1          = 0x12,
        K2          = 0x13,
        K3          = 0x14,
        K4          = 0x15,
        K5          = 0x17,
        K6          = 0x16,
        K7          = 0x1a,
        K8          = 0x1c,
        K9          = 0x19,

        MINUS       = 0x18,
        PLUS        = 0x1b,

        SPACE       = 0x31,
        TAB         = 0x30,
        RETURN      = 0x24,
        BACKSPACE   = 0x33,
        ESCAPE      = 0x35,

        // Modifier keys.
        COMMAND     = 0x37,
        SHIFT       = 0x38,
        CAPS_LOCK   = 0x39,
        OPTION      = 0x3a,
        CONTROL     = 0x3b,

        // Arrow keys.
        LEFT_ARROW  = 0x7b,
        RIGHT_ARROW = 0x7c,
        UP_ARROW    = 0x7e,
        DOWN_ARROW  = 0x7d,

        // Keys above arrow keys.
        DELETE      = 0x75,
        PAGE_DOWN   = 0x77,
        END         = 0x79,
        HELP        = 0x72,
        PAGE_UP     = 0x73,
        HOME        = 0x74,

        // Numeric keypad, numbers 0-9.
        N0          = 0x52,
        N1          = 0x53,
        N2          = 0x54,
        N3          = 0x55,
        N4          = 0x56,
        N5          = 0x57,
        N6          = 0x58,
        N7          = 0x59,
        N8          = 0x5b,
        N9          = 0x5c,

        // Function keys.
        F1          = 0x7a,
        F2          = 0x78,
        F3          = 0x63,
        F4          = 0x76,
        F5          = 0x60,
        F6          = 0x61,
        F7          = 0x62,
        F8          = 0x64,
        F9          = 0x65,
        F10         = 0x6d,
        F11         = 0x67,
        F12         = 0x6f,
        F13         = 0x69,
        F14         = 0x6b,
        F15         = 0x71,

        // Miscellaneous.
        POWER       = 0x7f,
    };
};

enum {
    KEY_NAMES = 1000,
    KEY_LONG_NAMES = 1002,
};
const int kKeyNameLength = 4;

inline bool mDeleteKey(const KeyMap& km)     { return km.get(Keys::BACKSPACE); }
inline bool mCapsLockKey(const KeyMap& km)   { return km.get(Keys::CAPS_LOCK); }
inline bool mReturnKey(const KeyMap& km)     { return km.get(Keys::RETURN); }
inline bool mEscKey(const KeyMap& km)        { return km.get(Keys::ESCAPE); }
inline bool mQKey(const KeyMap& km)          { return km.get(Keys::Q); }
inline bool mCommandKey(const KeyMap& km)    { return km.get(Keys::COMMAND); }
inline bool mLeftArrowKey(const KeyMap& km)  { return km.get(Keys::LEFT_ARROW); }

void GetKeyMapFromKeyNum(int key_num, KeyMap* key_map);
int GetKeyNumFromKeyMap(const KeyMap& key_map);
bool CommandKey( void);
void GetKeyNumName(int key_num, sfz::String* out);
bool AnyRealKeyDown();
bool AnyKeyButThisOne(const KeyMap& key_map, int key_num);
long GetAsciiFromKeyMap(const KeyMap&, const KeyMap&);

}  // namespace antares

#endif // ANTARES_KEY_MAP_TRANSLATION_HPP_
