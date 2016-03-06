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

// sound 0-14 always used -- loaded at start; 15+ may be swapped around
const int kMinVolatileSound = 15;

const double kHackRangeMultiplier = 0.0025;

// see if there's a channel with the same sound at same or lower volume
static bool same_sound_channel(
        int& channel, int16_t id, uint8_t amplitude, soundPriorityType priority) {
    if (priority > kVeryLowPrioritySound) {
        for (int i = 0; i < kMaxChannelNum; ++i) {
            if ((sys.channels[i].whichSound == id) &&
                (sys.channels[i].soundVolume <= amplitude)) {
                channel = i;
                return true;
            }
        }
    }
    return false;
}

// see if there's a channel at lower volume
static bool quieter_channel(int& channel, uint8_t amplitude) {
    for (int i = 0; i < kMaxChannelNum; ++i) {
        if (sys.channels[i].soundVolume < amplitude) {
            channel = i;
            return true;
        }
    }
    return false;
}

// see if there's a channel at lower priority
static bool lower_priority_channel(int& channel, soundPriorityType priority) {
    for (int i = 0; i < kMaxChannelNum; ++i) {
        if (sys.channels[i].soundPriority < priority) {
            channel = i;
            return true;
        }
    }
    return false;
}

// take the oldest sound if past minimum persistence
static bool oldest_available_channel(int& channel) {
    usecs oldestSoundTime(0);
    bool result = false;
    for (int i = 0; i < kMaxChannelNum; ++i) {
        auto past_reservation = now() - sys.channels[i].reserved_until;
        if (past_reservation > oldestSoundTime) {
            oldestSoundTime = past_reservation;
            channel = i;
            result = true;
        }
    }
    return result;
}

static bool best_channel(
        int& channel,
        int16_t sound_id, uint8_t amplitude, usecs persistence,
        soundPriorityType priority) {
    return same_sound_channel(channel, sound_id, amplitude, priority)
        || quieter_channel(channel, amplitude)
        || lower_priority_channel(channel, priority)
        || oldest_available_channel(channel);
}

void PlayVolumeSound(
        int16_t whichSoundID, uint8_t amplitude, usecs persistence,
        soundPriorityType priority) {
    int32_t whichChannel = -1;
    // TODO(sfiera): don't play sound at all if the game is muted.
    if (amplitude > 0) {
        if (!best_channel(whichChannel, whichSoundID, amplitude, persistence, priority)) {
            return;
        }

        int whichSound = 0;
        while ((sys.sounds[whichSound].id != whichSoundID) && (whichSound < kSoundNum)) {
            whichSound++;
        }
        if (whichSound == kSoundNum) {
            return;
        }

        sys.channels[whichChannel].whichSound = whichSoundID;
        sys.channels[whichChannel].reserved_until = now() + persistence;
        sys.channels[whichChannel].soundPriority = priority;
        sys.channels[whichChannel].soundVolume = amplitude;

        sys.channels[whichChannel].channelPtr->quiet();

        sys.channels[whichChannel].channelPtr->amp(amplitude);
        sys.channels[whichChannel].channelPtr->activate();
        sys.sounds[whichSound].soundHandle->play();
    }
}

void PlayLocalizedSound(
        uint32_t sx, uint32_t sy, uint32_t dx, uint32_t dy,
        Fixed hvel, Fixed vvel, int16_t whichSoundID, int16_t amplitude,
        usecs persistence, soundPriorityType priority) {
    static_cast<void>(sx);
    static_cast<void>(sy);
    static_cast<void>(dx);
    static_cast<void>(dy);
    static_cast<void>(hvel);
    static_cast<void>(vvel);

    PlayVolumeSound(whichSoundID, amplitude, persistence, priority);
}

void SetAllSoundsNoKeep() {
    for (int count = kMinVolatileSound; count < kSoundNum; count++) {
        sys.sounds[count].keepMe = false;
    }
}

void RemoveAllUnusedSounds() {
    for (int count = kMinVolatileSound; count < kSoundNum; count++) {
        if ((!sys.sounds[count].keepMe) &&
                (sys.sounds[count].soundHandle.get() != NULL)) {
            sys.sounds[count].soundHandle.reset();
            sys.sounds[count].id = -1;
        }
    }
}

void ResetAllSounds() {
    sys.sounds.resize(kSoundNum);
    for (int count = 0; count < kSoundNum; count++) {
        sys.sounds[count].keepMe = false;
        sys.sounds[count].id = -1;
    }
}

void KeepSound(int soundID) {
    int16_t whichSound;

    whichSound = 0;
    while ((sys.sounds[whichSound].id != soundID) && (whichSound < kSoundNum)) {
        whichSound++;
    }

    if (whichSound < kSoundNum) {
        sys.sounds[whichSound].keepMe = true;
    }
}

int AddSound(int soundID) {
    int whichSound = 0;
    while ((sys.sounds[whichSound].id != soundID) && (whichSound < kSoundNum)) {
        whichSound++;
    }

    if (whichSound == kSoundNum) {
        whichSound = 0;
        while ((sys.sounds[whichSound].soundHandle.get() != NULL) &&
                (whichSound < kSoundNum)) {
            whichSound++;
        }

        if (whichSound == kSoundNum) {
            throw Exception("Can't manage any more sounds");
        }

        sys.sounds[whichSound].soundHandle = sys.audio->open_sound(
                format("/sounds/{0}", soundID));
        sys.sounds[whichSound].id = soundID;
    }
    return whichSound;
}

void quiet_all() {
    for (int i = 0; i < kMaxChannelNum; i++) {
        sys.channels[i].channelPtr->quiet();
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

void mPlayDistanceSound(
        int32_t mvolume, Handle<SpaceObject> mobjectptr, int32_t msoundid,
        usecs msoundpersistence, soundPriorityType msoundpriority) {
    if (mobjectptr->distanceFromPlayer < kMaximumRelevantDistanceSquared) {
        int32_t mdistance = mobjectptr->distanceFromPlayer;
        uint32_t mul1;
        uint32_t mul2;
        auto player = g.ship;

        if (mdistance == 0) {
            if (player.get() && player->active) {
                mul1 = ABS<int>(player->location.h - mobjectptr->location.h);
                mul2 = mul1;
                mul1 =  ABS<int>(player->location.v - mobjectptr->location.v);
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
                            player->location.h, player->location.v,
                            mobjectptr->location.h, mobjectptr->location.v,
                            player->velocity.h - mobjectptr->velocity.h,
                            player->velocity.v - mobjectptr->velocity.v,
                            msoundid, mvolume, msoundpersistence, msoundpriority);
                }
            } else {
                mul1 = ABS<int>(gGlobalCorner.h - mobjectptr->location.h);
                mul2 = mul1;
                mul1 =  ABS<int>(gGlobalCorner.v - mobjectptr->location.v);
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
                            gGlobalCorner.h, gGlobalCorner.v,
                            mobjectptr->location.h, mobjectptr->location.v,
                            mobjectptr->velocity.h, mobjectptr->velocity.v,
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
                            player->location.h, player->location.v,
                            mobjectptr->location.h, mobjectptr->location.v,
                            player->velocity.h - mobjectptr->velocity.h,
                            player->velocity.v - mobjectptr->velocity.v,
                            msoundid, mvolume, msoundpersistence, msoundpriority);
                }
            } else {
                if (mvolume > 0) {
                    PlayLocalizedSound(
                            gGlobalCorner.h, gGlobalCorner.v,
                            mobjectptr->location.h, mobjectptr->location.v,
                            mobjectptr->velocity.h, mobjectptr->velocity.v,
                            msoundid, mvolume, msoundpersistence, msoundpriority);
                }
            }
        }
    }
}

}  // namespace antares
