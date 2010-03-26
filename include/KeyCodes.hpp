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

#ifndef ANTARES_KEY_CODES_HPP_
#define ANTARES_KEY_CODES_HPP_

#include "KeyMapTranslation.hpp"
#include "Preferences.hpp"

namespace antares {

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

#define kUpKey                      0x00000001
#define kDownKey                    0x00000002
#define kLeftKey                    0x00000004
#define kRightKey                   0x00000008
#define kOneKey                     0x00000010
#define kTwoKey                     0x00000020
#define kEnterKey                   0x00000040
#define kWarpKey                    0x00000080
#define kSelectFriendKey            0x00000100
#define kSelectFoeKey               0x00000200
#define kSelectBaseKey              0x00000400
#define kDestinationKey             0x00000800
#define kOrderKey                   0x00001000
#define kZoomInKey                  0x00002000
#define kZoomOutKey                 0x00004000
#define kCompUpKey                  0x00008000
#define kCompDownKey                0x00010000
#define kCompAcceptKey              0x00020000
#define kCompCancelKey              0x00040000

#define kAutoPilotKey               0x00080000  // just a flag used to set autopilot on
                                                // added for networking
#define kGiveCommandKey             0x00100000
#define kAdoptTargetKey             0x00200000  // used so player ship can set its dest
                                                // based on admiral's target
#define kMouseMask                  0x80000000  // for disabling for tutorial
#define kReturnKeyMask              0x40000000  // ''
#define kShortcutZoomMask           0x20000000  // ''
#define kComputerBuildMenu          0x10000000  // ''
#define kComputerSpecialMenu        0x08000000  // ''
#define kComputerMessageMenu        0x04000000  // ''
#define kManualOverrideFlag         0x80000000

#define kSpecialKeyMask             (kAutoPilotKey | kGiveCommandKey | kAdoptTargetKey)
#define kMotionKeyMask              (kUpKey | kDownKey | kRightKey | kLeftKey)
#define kWeaponKeyMask              (kOneKey | kTwoKey | kEnterKey)
#define kMiscKeyMask                (~(kMotionKeyMask | kWeaponKeyMask | kSpecialKeyMask))

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

#define mPauseKey               mCapsLockKey
#define mEnterTextKey           mReturnKey
inline bool mQuitKey1(const KeyMap& km)            { return mQKey(km); }
inline bool mQuitKey2(const KeyMap& km)            { return mCommandKey(km); }
inline bool mQuitKeys(const KeyMap& km)            { return mQuitKey1(km) && mQuitKey2(km); }
#define mRestartResumeKey       mEscKey

}  // namespace antares

#endif // ANTARES_KEY_CODES_HPP_
