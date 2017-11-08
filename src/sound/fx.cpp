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

#include "sound/fx.hpp"

#include <sfz/sfz.hpp>

#include "config/preferences.hpp"
#include "data/base-object.hpp"
#include "game/globals.hpp"
#include "game/motion.hpp"
#include "game/space-object.hpp"
#include "game/time.hpp"
#include "lang/defines.hpp"
#include "math/macros.hpp"
#include "math/special.hpp"
#include "math/units.hpp"
#include "sound/driver.hpp"
#include "video/driver.hpp"

using sfz::Exception;
using sfz::format;

namespace antares {

static const int32_t kMaxChannelNum = 3;

// sound 0-14 always used -- loaded at start; 15+ may be swapped around
static const int kMinVolatileSound = 15;

enum {
    kMorseBeepSound = 506,  // ship receives order
    kComputerBeep1  = 507,  // ship selected
    kComputerBeep2  = 508,  // ship built
    kComputerBeep3  = 509,  // button push
    kComputerBeep4  = 510,  // change range
    kWarningTone    = 511,  // naughty beep
    kLandingWoosh   = 513,
    kCloakOff       = 522,
    kCloakOn        = 523,
    kKlaxon         = 525,
    kWarp           = 526,
    kTeletype       = 535,
};

static const int16_t kFixedSounds[kMinVolatileSound] = {
        kComputerBeep4, kComputerBeep1, kComputerBeep2, kComputerBeep3, kMorseBeepSound,
        kWarningTone,   kLandingWoosh,  kCloakOn,       kCloakOff,      kKlaxon,
        kWarp + 0,      kWarp + 1,      kWarp + 2,      kWarp + 3,      kTeletype,
};

enum {
    kMediumLowVolume  = 64,
    kMediumVolume     = 128,
    kMediumLoudVolume = 192,
    kMaxSoundVolume   = 255,
};

static const ticks kShortPersistence      = ticks(10);
static const ticks kMediumPersistence     = ticks(20);
static const ticks kMediumLongPersistence = ticks(40);
static const ticks kLongPersistence       = ticks(60);

enum {
    kNoSound              = 0,
    kVeryLowPrioritySound = 1,
    kLowPrioritySound     = 2,
    kPrioritySound        = 3,
    kHighPrioritySound    = 4,
    kMustPlaySound        = 5
};

struct SoundFX::smartSoundChannel {
    int32_t                       whichSound;
    wall_time                     reserved_until;
    int16_t                       soundVolume;
    uint8_t                       soundPriority;
    std::unique_ptr<SoundChannel> channelPtr;
};

struct SoundFX::smartSoundHandle {
    int16_t                id;
    std::unique_ptr<Sound> soundHandle;
};

// see if there's a channel with the same sound at same or lower volume
bool SoundFX::same_sound_channel(int& channel, int16_t id, uint8_t amplitude, uint8_t priority) {
    if (priority > kVeryLowPrioritySound) {
        for (int i = 0; i < kMaxChannelNum; ++i) {
            if ((channels[i].whichSound == id) && (channels[i].soundVolume <= amplitude)) {
                channel = i;
                return true;
            }
        }
    }
    return false;
}

// see if there's a channel at lower volume
bool SoundFX::quieter_channel(int& channel, uint8_t amplitude) {
    for (int i = 0; i < kMaxChannelNum; ++i) {
        if (channels[i].soundVolume < amplitude) {
            channel = i;
            return true;
        }
    }
    return false;
}

// see if there's a channel at lower priority
bool SoundFX::lower_priority_channel(int& channel, uint8_t priority) {
    for (int i = 0; i < kMaxChannelNum; ++i) {
        if (channels[i].soundPriority < priority) {
            channel = i;
            return true;
        }
    }
    return false;
}

// take the oldest sound if past minimum persistence
bool SoundFX::oldest_available_channel(int& channel) {
    usecs oldestSoundTime(0);
    bool  result = false;
    for (int i = 0; i < kMaxChannelNum; ++i) {
        auto past_reservation = now() - channels[i].reserved_until;
        if (past_reservation > oldestSoundTime) {
            oldestSoundTime = past_reservation;
            channel         = i;
            result          = true;
        }
    }
    return result;
}

bool SoundFX::best_channel(
        int& channel, int16_t sound_id, uint8_t amplitude, usecs persistence, uint8_t priority) {
    return same_sound_channel(channel, sound_id, amplitude, priority) ||
           quieter_channel(channel, amplitude) || lower_priority_channel(channel, priority) ||
           oldest_available_channel(channel);
}

void SoundFX::play(int16_t whichSoundID, uint8_t amplitude, usecs persistence, uint8_t priority) {
    int32_t whichChannel = -1;
    // TODO(sfiera): don't play sound at all if the game is muted.
    if (amplitude > 0) {
        if (!best_channel(whichChannel, whichSoundID, amplitude, persistence, priority)) {
            return;
        }

        int whichSound = 0;
        while ((sounds[whichSound].id != whichSoundID) && (whichSound < sounds.size())) {
            whichSound++;
        }
        if (whichSound == sounds.size()) {
            return;
        }

        channels[whichChannel].whichSound     = whichSoundID;
        channels[whichChannel].reserved_until = now() + persistence;
        channels[whichChannel].soundPriority  = priority;
        channels[whichChannel].soundVolume    = amplitude;

        channels[whichChannel].channelPtr->quiet();

        channels[whichChannel].channelPtr->amp(amplitude);
        channels[whichChannel].channelPtr->activate();
        sounds[whichSound].soundHandle->play();
    }
}

static void PlayLocalizedSound(
        uint32_t sx, uint32_t sy, uint32_t dx, uint32_t dy, Fixed hvel, Fixed vvel,
        int16_t whichSoundID, int16_t amplitude, usecs persistence, uint8_t priority) {
    static_cast<void>(sx);
    static_cast<void>(sy);
    static_cast<void>(dx);
    static_cast<void>(dy);
    static_cast<void>(hvel);
    static_cast<void>(vvel);

    sys.sound.play(whichSoundID, amplitude, persistence, priority);
}

SoundFX::SoundFX() {}
SoundFX::~SoundFX() {}

void SoundFX::init() {
    channels.resize(kMaxChannelNum);
    for (int i = 0; i < kMaxChannelNum; i++) {
        channels[i].reserved_until = wall_time();
        channels[i].soundPriority  = kNoSound;
        channels[i].soundVolume    = 0;
        channels[i].whichSound     = -1;
        channels[i].channelPtr     = sys.audio->open_channel();
    }

    reset();
}

void SoundFX::reset() {
    sounds.resize(kMinVolatileSound);
    for (int i = 0; i < kMinVolatileSound; ++i) {
        if (!sounds[i].soundHandle.get()) {
            auto id               = kFixedSounds[i];
            sounds[i].id          = id;
            sounds[i].soundHandle = sys.audio->open_sound(format("/sounds/{0}", id));
        }
    }
}

void SoundFX::load(int16_t id) {
    int whichSound = 0;
    while ((sounds[whichSound].id != id) && (whichSound < sounds.size())) {
        whichSound++;
    }

    if (whichSound == sounds.size()) {
        sounds.emplace_back();
        sounds.back().id          = id;
        sounds.back().soundHandle = sys.audio->open_sound(format("/sounds/{0}", id));
    }
}

void SoundFX::stop() {
    for (int i = 0; i < kMaxChannelNum; i++) {
        channels[i].channelPtr->quiet();
    }
}

//
// Listing 2-27(S) Playing a sound resource    Sound Manager
// Inside Macintosh: Sound ,  page 2-57
// This listing is documented in full on page 2-57 of Inside Macintosh: Sound.
// FUNCTION MyPlaySampledSound (chan: SndChannelPtr; sndHandle: Handle): OSErr;
// VAR
//     myOffset:               LongInt;
//     mySndCmd:               SndCommand;                         {a sound command}
//     myErr:                  OSErr;
// BEGIN
//     myErr := GetSoundHeaderOffset(sndHandle, myOffset);
//     IF myErr = noErr THEN
//     BEGIN
//         HLock(sndHandle);
//         mySndCmd.cmd := bufferCmd;                              {command is bufferCmd}
//         mySndCmd.param1 := 0;                                   {unused with bufferCmd}
//         mySndCmd.param2 := LongInt(ORD4(sndHandle^) + myOffset);
//         myErr := SndDoImmediate(chan, mySndCmd);
//     END;
//     MyPlaySampledSound := myErr;
// END;
//

void SoundFX::play_at(
        int16_t msoundid, int32_t mvolume, usecs msoundpersistence, uint8_t msoundpriority,
        Handle<SpaceObject> mobjectptr) {
    if (mobjectptr->distanceFromPlayer < kMaximumRelevantDistanceSquared) {
        int32_t  mdistance = mobjectptr->distanceFromPlayer;
        uint32_t mul1;
        uint32_t mul2;
        auto     player = g.ship;

        if (mdistance == 0) {
            if (player.get() && player->active) {
                mul1      = ABS<int>(player->location.h - mobjectptr->location.h);
                mul2      = mul1;
                mul1      = ABS<int>(player->location.v - mobjectptr->location.v);
                mdistance = mul1;
                if ((mul2 < kMaximumRelevantDistance) && (mdistance < kMaximumRelevantDistance)) {
                    mdistance = mdistance * mdistance + mul2 * mul2;
                } else {
                    mdistance = kMaximumRelevantDistanceSquared;
                }
                mdistance = lsqrt(mdistance);
                if (mdistance > 480) {
                    mdistance -= 480;
                    if (mdistance > 1920) {
                        mvolume = 0;
                    } else {
                        mvolume = ((1920 - mdistance) * mvolume) / 1920;
                    }
                }
                if (mvolume > 0) {
                    PlayLocalizedSound(
                            player->location.h, player->location.v, mobjectptr->location.h,
                            mobjectptr->location.v, player->velocity.h - mobjectptr->velocity.h,
                            player->velocity.v - mobjectptr->velocity.v, msoundid, mvolume,
                            msoundpersistence, msoundpriority);
                }
            } else {
                mul1      = ABS<int>(gGlobalCorner.h - mobjectptr->location.h);
                mul2      = mul1;
                mul1      = ABS<int>(gGlobalCorner.v - mobjectptr->location.v);
                mdistance = mul1;
                if ((mul2 < kMaximumRelevantDistance) && (mdistance < kMaximumRelevantDistance)) {
                    mdistance = mdistance * mdistance + mul2 * mul2;
                } else {
                    mdistance = kMaximumRelevantDistanceSquared;
                }
                mdistance = lsqrt(mdistance);
                if (mdistance > 480) {
                    mdistance -= 480;
                    if (mdistance > 1920) {
                        mvolume = 0;
                    } else {
                        mvolume = ((1920 - mdistance) * mvolume) / 1920;
                    }
                }
                if (mvolume > 0) {
                    PlayLocalizedSound(
                            gGlobalCorner.h, gGlobalCorner.v, mobjectptr->location.h,
                            mobjectptr->location.v, mobjectptr->velocity.h, mobjectptr->velocity.v,
                            msoundid, mvolume, msoundpersistence, msoundpriority);
                }
            }
        } else {
            mdistance = lsqrt(mdistance);
            if (mdistance > 480) {
                mdistance -= 480;
                if (mdistance > 1920) {
                    mvolume = 0;
                } else {
                    mvolume = ((1920 - mdistance) * mvolume) / 1920;
                }
            }
            if (player.get() && player->active) {
                if (mvolume > 0) {
                    PlayLocalizedSound(
                            player->location.h, player->location.v, mobjectptr->location.h,
                            mobjectptr->location.v, player->velocity.h - mobjectptr->velocity.h,
                            player->velocity.v - mobjectptr->velocity.v, msoundid, mvolume,
                            msoundpersistence, msoundpriority);
                }
            } else {
                if (mvolume > 0) {
                    PlayLocalizedSound(
                            gGlobalCorner.h, gGlobalCorner.v, mobjectptr->location.h,
                            mobjectptr->location.v, mobjectptr->velocity.h, mobjectptr->velocity.v,
                            msoundid, mvolume, msoundpersistence, msoundpriority);
                }
            }
        }
    }
}

void SoundFX::select() {
    play(kComputerBeep1, kMediumLoudVolume, kMediumPersistence, kLowPrioritySound);
}

void SoundFX::build() {
    play(kComputerBeep2, kMediumVolume, kMediumPersistence, kLowPrioritySound);
}

void SoundFX::click() {
    play(kComputerBeep3, kMediumVolume, kMediumPersistence, kLowPrioritySound);
}

void SoundFX::zoom() {
    play(kComputerBeep4, kMediumVolume, kMediumPersistence, kLowPrioritySound);
}

void SoundFX::pause() {
    play(kComputerBeep4, kMaxSoundVolume, kShortPersistence, kMustPlaySound);
}

void SoundFX::klaxon() {
    play(kKlaxon, kMediumVolume, kMediumLongPersistence, kPrioritySound);
}

void SoundFX::loud_klaxon() {
    play(kKlaxon, kMaxSoundVolume, kLongPersistence, kMustPlaySound);
}

void SoundFX::order() {
    play(kMorseBeepSound, kMediumVolume, kMediumPersistence, kLowPrioritySound);
}

void SoundFX::warning() {
    play(kWarningTone, kMediumVolume, kMediumPersistence, kLowPrioritySound);
}

void SoundFX::teletype() {
    play(kTeletype, kMediumLowVolume, kShortPersistence, kLowPrioritySound);
}

void SoundFX::cloak_on() {
    play(kCloakOn, kMediumLoudVolume, kShortPersistence, kMustPlaySound);
}

void SoundFX::cloak_off() {
    play(kCloakOff, kMediumLoudVolume, kShortPersistence, kMustPlaySound);
}

void SoundFX::warp(int n, Handle<SpaceObject> object) {
    play_at(kWarp + n, kMaxSoundVolume, kMediumPersistence, kPrioritySound, object);
}

void SoundFX::cloak_on_at(Handle<SpaceObject> object) {
    play_at(kCloakOn, kMaxSoundVolume, kMediumPersistence, kPrioritySound, object);
}

void SoundFX::cloak_off_at(Handle<SpaceObject> object) {
    play_at(kCloakOff, kMaxSoundVolume, kMediumPersistence, kPrioritySound, object);
}

}  // namespace antares
