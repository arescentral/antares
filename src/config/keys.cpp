// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2012 The Antares Authors
//
// This file is part of Antares, a tactical space combat game.
//
// Antares is free software: you can redistribute it and/or modify it
// under the terms of the Lesser GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Antares is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with Antares.  If not, see http://www.gnu.org/licenses/

#include "config/keys.hpp"

#include <strings.h>

#include "data/string-list.hpp"
#include "video/driver.hpp"

namespace antares {

KeyMap::KeyMap(): _data{} {}

bool KeyMap::get(size_t index) const {
    return _data[index >> 3] & (1 << (index & 0x7));
}

void KeyMap::set(size_t index, bool value) {
    if (get(index) != value) {
        _data[index >> 3] ^= (1 << (index & 0x7));
    }
}

bool KeyMap::any() const {
    const Data zero = { };
    return memcmp(_data, zero, kDataSize) == 0;
}

bool KeyMap::equals(const KeyMap& other) const {
    return memcmp(_data, other._data, kDataSize) == 0;
}

void KeyMap::copy(const KeyMap& other) {
    memcpy(_data, other._data, kDataSize);
}

void KeyMap::clear() {
    bzero(_data, kDataSize);
}

bool operator==(const KeyMap& a, const KeyMap& b) {
    return a.equals(b);
}

bool operator!=(const KeyMap& a, const KeyMap& b) {
    return !a.equals(b);
}

void GetKeyMapFromKeyNum(int key_num, KeyMap* key_map) {
    key_map->clear();
    key_map->set(key_num, true);
}

int GetKeyNumFromKeyMap(const KeyMap& key_map) {
    for (int i = 0; i < 128; ++i) {
        if (key_map.get(i)) {
            return i + 1;
        }
    }
    return 0;
}

bool CommandKey() {
    KeyMap key_map;
    VideoDriver::driver()->get_keys(&key_map);
    return key_map.get(Keys::COMMAND);
}

void GetKeyNumName(int key_num, sfz::String* out) {
    StringList strings(KEY_NAMES);
    out->assign(strings.at(key_num - 1));
}

bool GetKeyNameNum(sfz::StringSlice name, int& out) {
    StringList strings(KEY_NAMES);
    bool result = false;
    for (int i = 0; i < strings.size(); ++i) {
        if (strings.at(i) == name) {
            out = i + 1;
            result = true;
        }
    }
    return result;
}

// returns true if any keys OTHER THAN POWER ON AND CAPS LOCK are down

bool AnyRealKeyDown() {
    KeyMap key_map;
    VideoDriver::driver()->get_keys(&key_map);

    key_map.set(Keys::POWER, false);
    key_map.set(Keys::CAPS_LOCK, false);

    return key_map.any();
}

bool AnyKeyButThisOne(const KeyMap& key_map, int key_num) {
    KeyMap others;
    others.copy(key_map);
    others.set(key_num, false);
    return others.any();
}

long GetAsciiFromKeyMap(const KeyMap& sourceKeyMap, const KeyMap& previousKeyMap) {
    // TODO(sfiera): write a new implementation of this method.
    static_cast<void>(sourceKeyMap);
    static_cast<void>(previousKeyMap);
    return 0;
    /*
    short           whichKeyCode = 0, modifiers = 0, count;
    long            result;
    Ptr             KCHRPtr;
    KeyMap          keyMap;

    if ( previousKeyMap == nil) {
        for ( count = 0; count < 4; count++)
            keyMap[count] = sourceKeyMap[count];
    } else {
        keyMap[0] = sourceKeyMap[0] & (~previousKeyMap[0]);
        keyMap[1] = sourceKeyMap[1] & ((~previousKeyMap[1]) | kModifierKeyMask);
        keyMap[2] = sourceKeyMap[2] & (~previousKeyMap[2]);
        keyMap[3] = sourceKeyMap[3] & (~previousKeyMap[3]);
    }

    if ( keyMap[1] & 0x0008) {
        keyMap[1] &= ~0x0008;
    }   // turn off control key

    if ( keyMap[1] & 0x8000) {
        modifiers |= cmdKey;
        keyMap[1] &= ~0x8000;   // turn off command key
    }

    if ( keyMap[1] & 0x0004) {
        modifiers |= optionKey;
        keyMap[1] &= ~0x0004;   // turn off option key
    }

    if ( keyMap[1] & 0x0001) {
        modifiers |= shiftKey;
        keyMap[1] &= ~0x0001;   // turn off shift key
    }

    if ( keyMap[1] & 0x0002) {
        modifiers |= alphaLock;
        keyMap[1] &= 0x0002;    // turn caps lock key
    }

    whichKeyCode = (GetKeyNumFromKeyMap( keyMap) - 1) | modifiers;
    KCHRPtr = reinterpret_cast<Ptr>(GetScriptManagerVariable( smKCHRCache));
    result = KeyTranslate( KCHRPtr, whichKeyCode, &gKeyTranslateState);

    return result;
    */
}

}  // namespace antares
