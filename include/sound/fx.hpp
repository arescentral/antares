// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2011 Ares Central
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
// License along with this program.  If not, see
// <http://www.gnu.org/licenses/>.

#ifndef ANTARES_SOUND_FX_HPP_
#define ANTARES_SOUND_FX_HPP_

#include <stdint.h>

#include "math/fixed.hpp"

namespace antares {

const int32_t kSoundNum         = 48;
const int32_t kMaxChannelNum    = 3;

const int32_t kMaxVolumePreference = 8;

const uint8_t kMediumLowVolume  = 64;
const uint8_t kMediumVolume     = 128;
const uint8_t kMediumLoudVolume = 192;
const uint8_t kMaxSoundVolume   = 255;

const int16_t kShortPersistence         = 10;      // in ticks
const int16_t kMediumPersistence        = 20;
const int16_t kMediumLongPersistence    = 40;
const int16_t kLongPersistence          = 60;

const int16_t kMorseBeepSound   = 506;  // ship receives order
const int16_t kComputerBeep1    = 507;  // ship selected
const int16_t kComputerBeep2    = 508;  // ship built
const int16_t kComputerBeep3    = 509;  // button push
const int16_t kComputerBeep4    = 510;  // change range
const int16_t kWarningTone      = 511;  // naughty beep
const int16_t kLandingWoosh     = 513;
const int16_t kCloakOff         = 522;
const int16_t kCloakOn          = 523;
const int16_t kKlaxon           = 525;
const int16_t kWarpOne          = 526;
const int16_t kWarpTwo          = 527;
const int16_t kWarpThree        = 528;
const int16_t kWarpFour         = 529;
const int16_t kTeletype         = 535;

struct Sound;
struct SoundChannel;
struct spaceObjectType;

enum soundPriorityType {
    kNoSound = 0,
    kVeryLowPrioritySound = 1,
    kLowPrioritySound = 2,
    kPrioritySound = 3,
    kHighPrioritySound = 4,
    kMustPlaySound = 5
};

struct smartSoundChannel {
    long                whichSound;
    long                soundAge;
    short               soundVolume;
    soundPriorityType   soundPriority;
    sfz::scoped_ptr<SoundChannel> channelPtr;
};

struct smartSoundHandle {
    sfz::scoped_ptr<Sound>   soundHandle;
    short               id;
    bool             keepMe;
};

void InitSoundFX();
void SetAllSoundsNoKeep();
void KeepSound(int sound_id);
int AddSound(int sound_id);
void RemoveAllUnusedSounds();
void ResetAllSounds();
void PlayVolumeSound(
        short whichSoundID, uint8_t amplitude, short persistence, soundPriorityType priority);
void PlayLocalizedSound(
        unsigned long sx, unsigned long sy, unsigned long dx, unsigned long dy,
        Fixed hvel, Fixed vvel, short whichSoundID, short amplitude,
        short persistence, soundPriorityType priority);
void SoundFXCleanup();

void mPlayDistanceSound(
        long mvolume, spaceObjectType* mobjectptr, long msoundid, long msoundpersistence,
        soundPriorityType msoundpriority);

}  // namespace antares

#endif // ANTARES_SOUND_FX_HPP_
