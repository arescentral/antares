// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2017 The Antares Authors
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

#include <pn/string>

namespace antares {

const size_t kKeyControlNum         = 8;
const size_t kKeyExtendedControlNum = 44;
const size_t kKeyMapBufferNum       = 256;
const size_t kHotKeyNum             = 10;

// USB Key codes.
class Key {
  public:
    enum Value {
        NONE = 0x00,

        // Letters, A-Z.
        A = 0x04,
        B = 0x05,
        C = 0x06,
        D = 0x07,
        E = 0x08,
        F = 0x09,
        G = 0x0a,
        H = 0x0b,
        I = 0x0c,
        J = 0x0d,
        K = 0x0e,
        L = 0x0f,
        M = 0x10,
        N = 0x11,
        O = 0x12,
        P = 0x13,
        Q = 0x14,
        R = 0x15,
        S = 0x16,
        T = 0x17,
        U = 0x18,
        V = 0x19,
        W = 0x1a,
        X = 0x1b,
        Y = 0x1c,
        Z = 0x1d,

        // Numbers  0-9.
        K1 = 0x1e,
        K2 = 0x1f,
        K3 = 0x20,
        K4 = 0x21,
        K5 = 0x22,
        K6 = 0x23,
        K7 = 0x24,
        K8 = 0x25,
        K9 = 0x26,
        K0 = 0x27,

        RETURN    = 0x28,
        ESCAPE    = 0x29,
        BACKSPACE = 0x2a,
        TAB       = 0x2b,
        SPACE     = 0x2c,

        MINUS     = 0x2d,
        EQUALS    = 0x2e,
        L_BRACKET = 0x2f,
        R_BRACKET = 0x30,
        BACKSLASH = 0x31,
        SEMICOLON = 0x33,
        QUOTE     = 0x34,
        BACKTICK  = 0x35,
        COMMA     = 0x36,
        PERIOD    = 0x37,
        SLASH     = 0x38,

        // Modifier keys.
        CAPS_LOCK = 0x39,
        CONTROL   = 0xe0,
        SHIFT     = 0xe1,
        OPTION    = 0xe2,
        COMMAND   = 0xe3,
        R_CONTROL = 0xe4,
        R_SHIFT   = 0xe5,
        R_OPTION  = 0xe6,
        R_COMMAND = 0xe7,

        // Arrow keys.
        RIGHT_ARROW = 0x4f,
        LEFT_ARROW  = 0x50,
        DOWN_ARROW  = 0x51,
        UP_ARROW    = 0x52,

        // Keys above arrow keys.
        HOME      = 0x4a,
        PAGE_UP   = 0x4b,
        DEL       = 0x4c,
        END       = 0x4d,
        PAGE_DOWN = 0x4e,
        HELP      = 0x75,

        // Numeric keypad, numbers 0-9.
        N1 = 0x59,
        N2 = 0x5a,
        N3 = 0x5b,
        N4 = 0x5c,
        N5 = 0x5d,
        N6 = 0x5e,
        N7 = 0x5f,
        N8 = 0x60,
        N9 = 0x61,
        N0 = 0x62,

        // Numeric keypad, non-numbers.
        N_CLEAR  = 0x53,
        N_DIVIDE = 0x54,
        N_TIMES  = 0x55,
        N_MINUS  = 0x56,
        N_PLUS   = 0x57,
        N_ENTER  = 0x58,
        N_PERIOD = 0x63,
        N_EQUALS = 0x67,

        // Function keys.
        F1  = 0x3a,
        F2  = 0x3b,
        F3  = 0x3c,
        F4  = 0x3d,
        F5  = 0x3e,
        F6  = 0x3f,
        F7  = 0x40,
        F8  = 0x41,
        F9  = 0x42,
        F10 = 0x43,
        F11 = 0x44,
        F12 = 0x45,
        F13 = 0x68,
        F14 = 0x69,
        F15 = 0x6a,
        F16 = 0x6b,
        F17 = 0x6c,
        F18 = 0x6d,
        F19 = 0x6e,
        F20 = 0x6f,
        F21 = 0x70,
        F22 = 0x71,
        F23 = 0x72,
        F24 = 0x73,

        // Media keys
        OPEN     = 0x74,
        PROPS    = 0x76,
        FRONT    = 0x77,
        STOP     = 0x78,
        AGAIN    = 0x79,
        UNDO     = 0x7a,
        CUT      = 0x7b,
        COPY     = 0x7c,
        PASTE    = 0x7d,
        FIND     = 0x7e,
        MUTE     = 0x7f,
        VOL_UP   = 0x80,
        VOL_DOWN = 0x81,

        // Miscellaneous.
        POWER = 0x66,

        // Japanese
        J_UNDERSCORE = 0x87,
        J_YEN        = 0x89,
        K_COMMA      = 0x85,
        J_KANA       = 0x88,
    };

    Key() : _value{NONE} {}
    Key(Value value) : _value{value} {}
    explicit Key(int value) : _value{value} {}
    int value() const { return _value; }

    int digit() const;

  private:
    int _value;
};

inline bool operator==(Key k1, Key k2) { return k1.value() == k2.value(); }
inline bool operator!=(Key k1, Key k2) { return k1.value() != k2.value(); }

constexpr char kKeyNameStrings[]     = "keyboard/short";
constexpr char kKeyLongNameStrings[] = "keyboard/long";
const int      kKeyNameLength        = 4;

void GetKeyNumName(Key key_num, pn::string& out);
bool GetKeyNameNum(pn::string_view name, Key& out);

enum KeyNum {
    kUpKeyNum    = 0,  // thrust
    kDownKeyNum  = 1,  // stop
    kLeftKeyNum  = 2,  // counter-clock
    kRightKeyNum = 3,  // clock
    kOneKeyNum   = 4,  // fire 1
    kTwoKeyNum   = 5,  // fire 2
    kEnterKeyNum = 6,  // special
    kWarpKeyNum  = 7,

    kSelectFriendKeyNum = 8,

    kSelectFoeKeyNum = 9,

    kSelectBaseKeyNum = 10,

    kDestinationKeyNum = 11,
    kOrderKeyNum       = 12,

    kZoomInKeyNum  = 13,
    kZoomOutKeyNum = 14,

    kCompUpKeyNum     = 15,  // go
    kCompDownKeyNum   = 16,
    kCompAcceptKeyNum = 17,

    kCompCancelKeyNum = 18,

    kTransferKeyNum     = 19,
    kScale121KeyNum     = 20,
    kScale122KeyNum     = 21,
    kScale124KeyNum     = 22,
    kScale1216KeyNum    = 23,
    kScaleHostileKeyNum = 24,
    kScaleObjectKeyNum  = 25,
    kScaleAllKeyNum     = 26,
    kMessageNextKeyNum  = 27,
    kHelpKeyNum         = 28,
    kVolumeDownKeyNum   = 29,
    kVolumeUpKeyNum     = 30,
    kActionMusicKeyNum  = 31,
    kNetSettingsKeyNum  = 32,
    kFastMotionKeyNum   = 33,

    kFirstHotKeyNum = 34,

    kHotKey1Num  = kFirstHotKeyNum + 0,
    kHotKey2Num  = kFirstHotKeyNum + 1,
    kHotKey3Num  = kFirstHotKeyNum + 2,
    kHotKey4Num  = kFirstHotKeyNum + 3,
    kHotKey5Num  = kFirstHotKeyNum + 4,
    kHotKey6Num  = kFirstHotKeyNum + 5,
    kHotKey7Num  = kFirstHotKeyNum + 6,
    kHotKey8Num  = kFirstHotKeyNum + 7,
    kHotKey9Num  = kFirstHotKeyNum + 8,
    kHotKey10Num = kFirstHotKeyNum + 9,

    KEY_COUNT = 44,  // Number of real keys that can be set in prefs
};

enum {
    kUpKey           = 0x00000001,
    kDownKey         = 0x00000002,
    kLeftKey         = 0x00000004,
    kRightKey        = 0x00000008,
    kPulseKey        = 0x00000010,
    kBeamKey         = 0x00000020,
    kSpecialKey      = 0x00000040,
    kWarpKey         = 0x00000080,
    kSelectFriendKey = 0x00000100,
    kSelectFoeKey    = 0x00000200,
    kSelectBaseKey   = 0x00000400,
    kDestinationKey  = 0x00000800,
    kOrderKey        = 0x00001000,
    kZoomInKey       = 0x00002000,
    kZoomOutKey      = 0x00004000,
    kCompUpKey       = 0x00008000,
    kCompDownKey     = 0x00010000,
    kCompAcceptKey   = 0x00020000,
    kCompCancelKey   = 0x00040000,

    kAutoPilotKey = 0x00080000,  // just a flag used to set autopilot on
                                 // added for networking
    kGiveCommandKey = 0x00100000,
    kAdoptTargetKey = 0x00200000,       // used so player ship can set its dest
                                        // based on admiral's target
    kMouseMask           = 0x80000000,  // for disabling for tutorial
    kReturnKeyMask       = 0x40000000,  // ''
    kShortcutZoomMask    = 0x20000000,  // ''
    kComputerBuildMenu   = 0x10000000,  // ''
    kComputerSpecialMenu = 0x08000000,  // ''
    kComputerMessageMenu = 0x04000000,  // ''
    kManualOverrideFlag  = 0x80000000,

    kSpecialKeyMask = (kAutoPilotKey | kGiveCommandKey | kAdoptTargetKey),
    kMotionKeyMask  = (kUpKey | kDownKey | kRightKey | kLeftKey),
    kWeaponKeyMask  = (kPulseKey | kBeamKey | kSpecialKey),
    kMiscKeyMask    = (~(kMotionKeyMask | kWeaponKeyMask | kSpecialKeyMask)),
};

}  // namespace antares

#endif  // ANTARES_CONFIG_KEYS_HPP_
