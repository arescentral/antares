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

#ifndef ANTARES_CONFIG_KEYS_HPP_
#define ANTARES_CONFIG_KEYS_HPP_

#include "config/preferences.hpp"

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

        // Numeric keypad, non-numbers.
        N_PERIOD    = 0x41,
        N_TIMES     = 0x43,
        N_PLUS      = 0x45,
        N_CLEAR     = 0x47,
        N_DIVIDE    = 0x4b,
        N_MINUS     = 0x4e,
        N_ENTER     = 0x4c,
        N_EQUALS    = 0x51,

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

enum {
    kUpKeyNum = 0,   // thrust
    kDownKeyNum = 1,   // stop
    kLeftKeyNum = 2,   // counter-clock
    kRightKeyNum = 3,   // clock
    kOneKeyNum = 4,   // fire 1
    kTwoKeyNum = 5,   // fire 2
    kEnterKeyNum = 6,   // special
    kWarpKeyNum = 7,

    kSelectFriendKeyNum = 8,

    kSelectFoeKeyNum = 9,

    kSelectBaseKeyNum = 10,


    kDestinationKeyNum = 11,
    kOrderKeyNum = 12,

    kZoomInKeyNum = 13,
    kZoomOutKeyNum = 14,

    kCompUpKeyNum = 15,  // go
    kCompDownKeyNum = 16,
    kCompAcceptKeyNum = 17,

    kCompCancelKeyNum = 18,

    kTransferKeyNum = 19,
    kScale121KeyNum = 20,
    kScale122KeyNum = 21,
    kScale124KeyNum = 22,
    kScale1216KeyNum = 23,
    kScaleHostileKeyNum = 24,
    kScaleObjectKeyNum = 25,
    kScaleAllKeyNum = 26,
    kMessageNextKeyNum = 27,
    kHelpKeyNum = 28,
    kVolumeDownKeyNum = 29,
    kVolumeUpKeyNum = 30,
    kActionMusicKeyNum = 31,
    kNetSettingsKeyNum = 32,
    kFastMotionKeyNum = 33,

    kFirstHotKeyNum = 34,

    KEY_COUNT = 44,
};

enum {
    kUpKey                  = 0x00000001,
    kDownKey                = 0x00000002,
    kLeftKey                = 0x00000004,
    kRightKey               = 0x00000008,
    kOneKey                 = 0x00000010,
    kTwoKey                 = 0x00000020,
    kEnterKey               = 0x00000040,
    kWarpKey                = 0x00000080,
    kSelectFriendKey        = 0x00000100,
    kSelectFoeKey           = 0x00000200,
    kSelectBaseKey          = 0x00000400,
    kDestinationKey         = 0x00000800,
    kOrderKey               = 0x00001000,
    kZoomInKey              = 0x00002000,
    kZoomOutKey             = 0x00004000,
    kCompUpKey              = 0x00008000,
    kCompDownKey            = 0x00010000,
    kCompAcceptKey          = 0x00020000,
    kCompCancelKey          = 0x00040000,

    kAutoPilotKey           = 0x00080000,  // just a flag used to set autopilot on
                                           // added for networking
    kGiveCommandKey         = 0x00100000,
    kAdoptTargetKey         = 0x00200000,  // used so player ship can set its dest
                                           // based on admiral's target
    kMouseMask              = 0x80000000,  // for disabling for tutorial
    kReturnKeyMask          = 0x40000000,  // ''
    kShortcutZoomMask       = 0x20000000,  // ''
    kComputerBuildMenu      = 0x10000000,  // ''
    kComputerSpecialMenu    = 0x08000000,  // ''
    kComputerMessageMenu    = 0x04000000,  // ''
    kManualOverrideFlag     = 0x80000000,

    kSpecialKeyMask         = (kAutoPilotKey | kGiveCommandKey | kAdoptTargetKey),
    kMotionKeyMask          = (kUpKey | kDownKey | kRightKey | kLeftKey),
    kWeaponKeyMask          = (kOneKey | kTwoKey | kEnterKey),
    kMiscKeyMask            = (~(kMotionKeyMask | kWeaponKeyMask | kSpecialKeyMask)),
};

inline bool mCheckKeyMap(const KeyMap& mKeyMap, int mki) {
    return mKeyMap.get(Preferences::preferences()->key(mki) - 1);
}

inline bool mHelpKey(const KeyMap& km)             { return mCheckKeyMap(km, kHelpKeyNum); }
inline bool mVolumeDownKey(const KeyMap& km)       { return mCheckKeyMap(km, kVolumeDownKeyNum); }
inline bool mVolumeUpKey(const KeyMap& km)         { return mCheckKeyMap(km, kVolumeUpKeyNum); }
inline bool mActionMusicKey(const KeyMap& km)      { return mCheckKeyMap(km, kActionMusicKeyNum); }
inline bool mNetSettingsKey(const KeyMap& km)      { return mCheckKeyMap(km, kNetSettingsKeyNum); }
inline bool mMessageNextKey(const KeyMap& km)      { return mCheckKeyMap(km, kMessageNextKeyNum); }

inline bool mTransferKey(const KeyMap& km)         { return mCheckKeyMap(km, kTransferKeyNum); }
inline bool mFastMotionKey(const KeyMap& km)       { return mCheckKeyMap(km, kFastMotionKeyNum); }

inline bool mScale121Key(const KeyMap& km)         { return mCheckKeyMap(km, kScale121KeyNum); }
inline bool mScale122Key(const KeyMap& km)         { return mCheckKeyMap(km, kScale122KeyNum); }
inline bool mScale124Key(const KeyMap& km)         { return mCheckKeyMap(km, kScale124KeyNum); }
inline bool mScale1216Key(const KeyMap& km)        { return mCheckKeyMap(km, kScale1216KeyNum); }
inline bool mScaleHostileKey(const KeyMap& km)     { return mCheckKeyMap(km, kScaleHostileKeyNum); }
inline bool mScaleObjectKey(const KeyMap& km)      { return mCheckKeyMap(km, kScaleObjectKeyNum); }
inline bool mScaleAllKey(const KeyMap& km)         { return mCheckKeyMap(km, kScaleAllKeyNum); }

inline bool mNOFHelpKey(const KeyMap& km)          { return mCheckKeyMap(km, kHelpKeyNum); }
inline bool mNOFVolumeDownKey(const KeyMap& km)    { return mCheckKeyMap(km, kVolumeDownKeyNum); }
inline bool mNOFVolumeUpKey(const KeyMap& km)      { return mCheckKeyMap(km, kVolumeUpKeyNum); }
inline bool mNOFNetSettingsKey(const KeyMap& km)   { return mCheckKeyMap(km, kNetSettingsKeyNum); }

inline bool mNOFFastMotionKey(const KeyMap& km)    { return mCheckKeyMap(km, kFastMotionKeyNum); }

inline bool mNOFScale121Key(const KeyMap& km)      { return mCheckKeyMap(km, kScale121KeyNum); }
inline bool mNOFScale122Key(const KeyMap& km)      { return mCheckKeyMap(km, kScale122KeyNum); }
inline bool mNOFScale124Key(const KeyMap& km)      { return mCheckKeyMap(km, kScale124KeyNum); }
inline bool mNOFScaleHostileKey(const KeyMap& km)  { return mCheckKeyMap(km, kScaleHostileKeyNum); }
inline bool mNOFScaleObjectKey(const KeyMap& km)   { return mCheckKeyMap(km, kScaleObjectKeyNum); }
inline bool mNOFScaleAllKey(const KeyMap& km)      { return mCheckKeyMap(km, kScaleAllKeyNum); }

inline bool mPauseKey(const KeyMap& km)            { return mCapsLockKey(km); }
inline bool mEnterTextKey(const KeyMap& km)        { return mReturnKey(km); }
inline bool mQuitKey1(const KeyMap& km)            { return mQKey(km); }
inline bool mQuitKey2(const KeyMap& km)            { return mCommandKey(km); }
inline bool mQuitKeys(const KeyMap& km)            { return mQuitKey1(km) && mQuitKey2(km); }
inline bool mRestartResumeKey(const KeyMap& km)    { return mEscKey(km); }

}  // namespace antares

#endif // ANTARES_CONFIG_KEYS_HPP_
