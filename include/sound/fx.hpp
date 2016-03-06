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

#ifndef ANTARES_SOUND_FX_HPP_
#define ANTARES_SOUND_FX_HPP_

#include <stdint.h>

#include "data/handle.hpp"
#include "math/fixed.hpp"
#include "math/units.hpp"

namespace antares {

const int32_t kMaxVolumePreference = 8;

const uint8_t kMediumLowVolume  = 64;
const uint8_t kMediumVolume     = 128;
const uint8_t kMediumLoudVolume = 192;
const uint8_t kMaxSoundVolume   = 255;

const ticks kShortPersistence         = ticks(10);
const ticks kMediumPersistence        = ticks(20);
const ticks kMediumLongPersistence    = ticks(40);
const ticks kLongPersistence          = ticks(60);

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
const int16_t kWarp[4]          = {526, 527, 528, 529};
const int16_t kTeletype         = 535;

class Sound;
class SoundChannel;
struct SpaceObject;

enum soundPriorityType {
    kNoSound = 0,
    kVeryLowPrioritySound = 1,
    kLowPrioritySound = 2,
    kPrioritySound = 3,
    kHighPrioritySound = 4,
    kMustPlaySound = 5
};

struct smartSoundChannel {
    int32_t             whichSound;
    wall_time           reserved_until;
    int16_t             soundVolume;
    soundPriorityType   soundPriority;
    std::unique_ptr<SoundChannel> channelPtr;
};

struct smartSoundHandle {
    int16_t                 id;
    std::unique_ptr<Sound>  soundHandle;
};

class SoundFX {
    public:
        void init();
        void load(int16_t id);
        void reset();
        void stop();

        void play(int16_t id, uint8_t volume, usecs persistence, soundPriorityType priority);
        void play_at(int16_t id, int32_t volume, usecs persistence, soundPriorityType priority,
                     Handle<SpaceObject> object);

    private:
        bool same_sound_channel(
                int& channel, int16_t id, uint8_t amplitude, soundPriorityType priority);
        bool quieter_channel(int& channel, uint8_t amplitude);
        bool lower_priority_channel(int& channel, soundPriorityType priority);
        bool oldest_available_channel(int& channel);
        bool best_channel(
                int& channel,
                int16_t sound_id, uint8_t amplitude, usecs persistence,
                soundPriorityType priority);

        std::vector<smartSoundHandle>   sounds;
        std::vector<smartSoundChannel>  channels;
};

}  // namespace antares

#endif // ANTARES_SOUND_FX_HPP_
