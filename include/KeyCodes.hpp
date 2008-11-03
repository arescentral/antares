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

/* Key Codes */
//#define   kModeKeyNum                 7
//#define   kMapToggleKeyNum            10

#define kUpKeyNum                   0   // thrust
#define kUpKey                      0x00000001
#define kDownKeyNum                 1   // stop
#define kDownKey                    0x00000002
#define kLeftKeyNum                 2   // counter-clock
#define kLeftKey                    0x00000004
#define kRightKeyNum                3   // clock
#define kRightKey                   0x00000008
#define kOneKeyNum                  4   // fire 1
#define kOneKey                     0x00000010
#define kTwoKeyNum                  5   // fire 2
#define kTwoKey                     0x00000020
#define kEnterKeyNum                6   // special
#define kEnterKey                   0x00000040
#define kWarpKeyNum                 7
#define kWarpKey                    0x00000080

#define kSelectFriendKeyNum         8
#define kSelectFriendKey            0x00000100

#define kSelectFoeKeyNum            9
#define kSelectFoeKey               0x00000200

#define kSelectBaseKeyNum           10
#define kSelectBaseKey              0x00000400


#define kDestinationKeyNum          11
#define kDestinationKey             0x00000800
#define kOrderKeyNum                12
#define kOrderKey                   0x00001000

#define kZoomInKeyNum               13
#define kZoomInKey                  0x00002000
#define kZoomOutKeyNum              14
#define kZoomOutKey                 0x00004000

#define kCompUpKeyNum               15  // go
#define kCompUpKey                  0x00008000
#define kCompDownKeyNum             16
#define kCompDownKey                0x00010000
#define kCompAcceptKeyNum           17
#define kCompAcceptKey              0x00020000

#define kCompCancelKeyNum           18
#define kCompCancelKey              0x00040000

#define kTransferKeyNum             19
#define kScale121KeyNum             20
#define kScale122KeyNum             21
#define kScale124KeyNum             22
#define kScale1216KeyNum            23
#define kScaleHostileKeyNum         24
#define kScaleObjectKeyNum          25
#define kScaleAllKeyNum             26
#define kMessageNextKeyNum          27
#define kHelpKeyNum                 28
#define kVolumeDownKeyNum           29
#define kVolumeUpKeyNum             30
#define kActionMusicKeyNum          31
#define kNetSettingsKeyNum          32
#define kFastMotionKeyNum           33

#define kFirstHotKeyNum             34

//#define   kAutoPilotKeyNum            19          // doesn't really exist
#define kAutoPilotKey               0x00080000  // just a flag used to set autopilot on
                                                // added for networking

//#define   kGiveCommandKeyNum          20          // same deal; doesn't exist
#define kGiveCommandKey             0x00100000

//#define   kAdoptTargetKeyNum          21          // doesn't exist
#define kAdoptTargetKey             0x00200000  // used so player ship can set its dest
                                                // based on admiral's target

#define kMouseMask                  0x80000000  // for disabling for tutorial
#define kReturnKeyMask              0x40000000  // ''
#define kShortcutZoomMask           0x20000000  // ''
#define kComputerBuildMenu          0x10000000  // ''
#define kComputerSpecialMenu        0x08000000  // ''
#define kComputerMessageMenu        0x04000000  // ''
#define kManualOverrideFlag         0x80000000

/*
#define kCompDownKeyNum             18
#define kCompDownKey                0x00040000
#define kCompAcceptKeyNum           19
#define kCompAcceptKey              0x00080000
#define kCompCancelKeyNum           20
#define kCompCancelKey              0x00100000
*/

#define kSpecialKeyMask             (kAutoPilotKey | kGiveCommandKey | kAdoptTargetKey)
#define kMotionKeyMask              (kUpKey | kDownKey | kRightKey | kLeftKey)
#define kWeaponKeyMask              (kOneKey | kTwoKey | kEnterKey)
#define kMiscKeyMask                (~(kMotionKeyMask | kWeaponKeyMask | kSpecialKeyMask))

#define mCheckKeyMap(mKeyMap, mki) ((gAresGlobal->gKeyControl[mki][0] & (mKeyMap[0])) || (gAresGlobal->gKeyControl[mki][1] & (mKeyMap[1])) || (gAresGlobal->gKeyControl[mki][2] & (mKeyMap[2])) || (gAresGlobal->gKeyControl[mki][3] & (mKeyMap[3])))
//#define   mHelpKey                mF1Key
//#define   mVolumeDownKey          mF2Key
//#define   mVolumeUpKey            mF3Key
//#define   mActionMusicKey         mF4Key
//#define   mNetSettingsKey         mF5Key
//#define mTransferKey          mF6Key
//#define   mSlowMotionKey          mF7Key
//#define   mFastMotionKey          mF8Key
//#define   mScale221Key            mF9Key
//#define   mScale121Key            mF10Key
//#define   mScale122Key            mF11Key
//#define   mScale124Key            mF12Key
//#define   mScaleHostileKey        mF13Key
//#define   mScaleObjectKey         mF14Key
//#define   mScaleAllKey            mF15Key
#define mHelpKey(mKeyMap)           mCheckKeyMap(mKeyMap, kHelpKeyNum)
#define mVolumeDownKey(mKeyMap)     mCheckKeyMap(mKeyMap, kVolumeDownKeyNum)
#define mVolumeUpKey(mKeyMap)       mCheckKeyMap(mKeyMap, kVolumeUpKeyNum)
#define mActionMusicKey(mKeyMap)    mCheckKeyMap(mKeyMap, kActionMusicKeyNum)
#define mNetSettingsKey(mKeyMap)    mCheckKeyMap(mKeyMap, kNetSettingsKeyNum)
#define mMessageNextKey(mKeyMap)    mCheckKeyMap(mKeyMap, kMessageNextKeyNum)

#define mTransferKey(mKeyMap)       mCheckKeyMap(mKeyMap, kTransferKeyNum)
#define mFastMotionKey(mKeyMap)     mCheckKeyMap(mKeyMap, kFastMotionKeyNum)

#define mScale121Key(mKeyMap)       mCheckKeyMap(mKeyMap, kScale121KeyNum)
#define mScale122Key(mKeyMap)       mCheckKeyMap(mKeyMap, kScale122KeyNum)
#define mScale124Key(mKeyMap)       mCheckKeyMap(mKeyMap, kScale124KeyNum)
#define mScale1216Key(mKeyMap)      mCheckKeyMap(mKeyMap, kScale1216KeyNum)
#define mScaleHostileKey(mKeyMap)   mCheckKeyMap(mKeyMap, kScaleHostileKeyNum)
#define mScaleObjectKey(mKeyMap)    mCheckKeyMap(mKeyMap, kScaleObjectKeyNum)
#define mScaleAllKey(mKeyMap)       mCheckKeyMap(mKeyMap, kScaleAllKeyNum)

#define mNOFHelpKey(mKeyMap)            mCheckKeyMap(mKeyMap, kHelpKeyNum)
#define mNOFVolumeDownKey(mKeyMap)      mCheckKeyMap(mKeyMap, kVolumeDownKeyNum)
#define mNOFVolumeUpKey(mKeyMap)        mCheckKeyMap(mKeyMap, kVolumeUpKeyNum)
#define mNOFNetSettingsKey(mKeyMap)     mCheckKeyMap(mKeyMap, kNetSettingsKeyNum)

#define mTransferKey(mKeyMap)           mCheckKeyMap(mKeyMap, kTransferKeyNum)
#define mNOFFastMotionKey(mKeyMap)      mCheckKeyMap(mKeyMap, kFastMotionKeyNum)

#define mNOFScale121Key(mKeyMap)        mCheckKeyMap(mKeyMap, kScale121KeyNum)
#define mNOFScale122Key(mKeyMap)        mCheckKeyMap(mKeyMap, kScale122KeyNum)
#define mNOFScale124Key(mKeyMap)        mCheckKeyMap(mKeyMap, kScale124KeyNum)
#define mNOFScaleHostileKey(mKeyMap)    mCheckKeyMap(mKeyMap, kScaleHostileKeyNum)
#define mNOFScaleObjectKey(mKeyMap) mCheckKeyMap(mKeyMap, kScaleObjectKeyNum)
#define mNOFScaleAllKey(mKeyMap)        mCheckKeyMap(mKeyMap, kScaleAllKeyNum)

#define mPauseKey               mCapsLockKey
#define mEnterTextKey           mReturnKey
#define mQuitKey1( mmkeymap)    (mQKey( mmkeymap))
#define mQuitKey2( mmkeymap)    (mCommandKey( mmkeymap))
#define mQuitKeys( mmkeymap)    ((mQuitKey1( mmkeymap)) && ( mQuitKey2(mmkeymap)))
#define mRestartResumeKey       mEscKey

//#define   mNOFHelpKey             m1Key
//#define   mNOFVolumeDownKey       mMinusKey
//#define   mNOFVolumeUpKey         mPlusKey
//#define   mNOFNetSettingsKey      m2Key
//#define   mNOFFastMotionKey       m3Key
//#define   mNOFScale221Key         m4Key
//#define   mNOFScale121Key         m5Key
//#define   mNOFScale122Key         m6Key
//#define   mNOFScale124Key         m7Key
//#define   mNOFScaleHostileKey     m8Key
//#define   mNOFScaleObjectKey      m9Key
//#define   mNOFScaleAllKey         m0Key

#endif // ANTARES_KEY_CODES_HPP_
