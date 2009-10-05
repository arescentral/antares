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

#ifndef ANTARES_OPTIONS_HPP_
#define ANTARES_OPTIONS_HPP_

// Options.h
//#define   kNonPlayableDemo

namespace antares {

#define kOptionAutoPlay                 0x00000001
#define kOptionMusicIdle                0x00000002
#define kOptionMusicPlay                0x00000004
#define kOptionScreenSmall              0x00000008
#define kOptionScreenMedium             0x00000010
#define kOptionScreenLarge              0x00000020
#define kOptionQDOnly                   0x00000040
#define kOptionRowSkip                  0x00000080
#define kOptionBlackground              0x00000100
#define kOptionNoScaleUp                0x00000200
#define kOptionRecord                   0x00000400
#define kOptionReplay                   0x00000800
#define kOptionSpeechOn                 0x00001000
#define kOptionSubstituteFKeys          0x00002000
#define kOptionHaveSeenIntro            0x00004000
#define kOptionNoSinglePlayer           0x00008000
#define kOptionUseSystemHideMenuBar     0x00010000
#define kOption18                       0x00020000
#define kOption19                       0x00040000
#define kOption20                       0x00080000
#define kOption21                       0x00100000
#define kOption22                       0x00200000
#define kOption23                       0x00400000
#define kOption24                       0x00800000
#define kOptionSoundAvailable           0x01000000
#define kOptionNetworkAvailable         0x02000000
#define kOptionSpeechAvailable          0x04000000
#define kOptionSoundSprocketOn          0x08000000
#define kOptionNetworkOn                0x10000000
#define kOptionMusicDriver              0x20000000
#define kOptionQuicktime                0x40000000
#define kOptionInBackground             0x80000000

#define kCarryOverOptionMask            (kOptionMusicDriver | kOptionQuicktime | kOptionSoundSprocketOn | kOptionSpeechAvailable | kOptionNetworkAvailable | kOptionNoSinglePlayer | kOptionSoundAvailable | kOptionUseSystemHideMenuBar)

}  // namespace antares

#endif // ANTARES_OPTIONS_HPP_
