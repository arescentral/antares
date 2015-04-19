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
#include "data/space-object.hpp"
#include "game/globals.hpp"
#include "game/motion.hpp"
#include "game/space-object.hpp"
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

void InitSoundFX() {
    for (int i = 0; i < kMaxChannelNum; i++) {
        globals()->gChannel[i].soundAge = 0;
        globals()->gChannel[i].soundPriority = kNoSound;
        globals()->gChannel[i].whichSound = -1;
        globals()->gChannel[i].channelPtr = SoundDriver::driver()->open_channel();
    }

    ResetAllSounds();
    AddSound(kComputerBeep4);
    AddSound(kComputerBeep1);
    AddSound(kComputerBeep2);
    AddSound(kComputerBeep3);
    AddSound(kMorseBeepSound);
    AddSound(kWarningTone);
    AddSound(kLandingWoosh);
    AddSound(kCloakOn);
    AddSound(kCloakOff);
    AddSound(kKlaxon);
    AddSound(kWarp[0]);
    AddSound(kWarp[1]);
    AddSound(kWarp[2]);
    AddSound(kWarp[3]);
    AddSound(kTeletype);
}

void PlayVolumeSound(
        int16_t whichSoundID, uint8_t amplitude, int16_t persistence, soundPriorityType priority) {
    int32_t oldestSoundTime = -ticks_to_usecs(kLongPersistence), whichChannel = -1;
    // TODO(sfiera): don't play sound at all if the game is muted.
    if (amplitude > 0) {
        int timeDif = VideoDriver::driver()->usecs() - globals()->gLastSoundTime;
        for (int count = 0; count < kMaxChannelNum; count++) {
            globals()->gChannel[count].soundAge += timeDif;
        }

        // if not see if there's another channel with the same sound at same or lower volume
        int count = 0;
        if (priority > kVeryLowPrioritySound) {
            while ((count < kMaxChannelNum) && (whichChannel == -1)) {
                if ((globals()->gChannel[count].whichSound == whichSoundID) &&
                    (globals()->gChannel[count].soundVolume <= amplitude)) {
                    whichChannel = count;
                }
                count++;
            }
        }

        // if not see if there's another channel at lower volume
        if (whichChannel == -1) {
            count = 0;
            while ((count < kMaxChannelNum) && (whichChannel == -1)) {
                if (globals()->gChannel[count].soundVolume < amplitude) {
                    whichChannel = count;
                }
                count++;
            }
        }

        // if not see if there's another channel at lower priority
        if (whichChannel == -1) {
            count = 0;
            while ((count < kMaxChannelNum) && (whichChannel == -1)) {
                if (globals()->gChannel[count].soundPriority < priority) {
                    whichChannel = count;
                }
                count++;
            }
        }

        // if not, take the oldest sound if past minimum persistence
        if (whichChannel == -1) {
            count = 0;
            while (count < kMaxChannelNum) {
                if ((globals()->gChannel[count].soundAge > 0)
                        && (globals()->gChannel[count].soundAge > oldestSoundTime)) {
                    oldestSoundTime = globals()->gChannel[count].soundAge;
                    whichChannel = count;
                }
                count++;
            }
        }

        // we're not checking for importance

        globals()->gLastSoundTime = VideoDriver::driver()->usecs();

        int whichSound = 0;
        while ((globals()->gSound[whichSound].id != whichSoundID) && (whichSound < kSoundNum)) {
            whichSound++;
        }
        if (whichSound == kSoundNum) {
            whichChannel = -1;
        }

        if (whichChannel >= 0) {
            globals()->gChannel[whichChannel].whichSound = whichSoundID;
            globals()->gChannel[whichChannel].soundAge = -ticks_to_usecs(persistence);
            globals()->gChannel[whichChannel].soundPriority = priority;
            globals()->gChannel[whichChannel].soundVolume = amplitude;

            globals()->gChannel[whichChannel].channelPtr->quiet();

            globals()->gChannel[whichChannel].channelPtr->amp(amplitude);
            globals()->gChannel[whichChannel].channelPtr->activate();
            globals()->gSound[whichSound].soundHandle->play();
        }
    }
}

void PlayLocalizedSound(
        uint32_t sx, uint32_t sy, uint32_t dx, uint32_t dy,
        Fixed hvel, Fixed vvel, int16_t whichSoundID, int16_t amplitude,
        int16_t persistence, soundPriorityType priority) {
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
        globals()->gSound[count].keepMe = false;
    }
}

void RemoveAllUnusedSounds() {
    for (int count = kMinVolatileSound; count < kSoundNum; count++) {
        if ((!globals()->gSound[count].keepMe) &&
                (globals()->gSound[count].soundHandle.get() != NULL)) {
            globals()->gSound[count].soundHandle.reset();
            globals()->gSound[count].id = -1;
        }
    }
}

void ResetAllSounds() {
    for (int count = 0; count < kSoundNum; count++) {
        globals()->gSound[count].keepMe = false;
        globals()->gSound[count].id = -1;
    }
}

void KeepSound(int soundID) {
    int16_t whichSound;

    whichSound = 0;
    while ((globals()->gSound[whichSound].id != soundID) && (whichSound < kSoundNum)) {
        whichSound++;
    }

    if (whichSound < kSoundNum) {
        globals()->gSound[whichSound].keepMe = true;
    }
}

int AddSound(int soundID) {
    int whichSound = 0;
    while ((globals()->gSound[whichSound].id != soundID) && (whichSound < kSoundNum)) {
        whichSound++;
    }

    if (whichSound == kSoundNum) {
        whichSound = 0;
        while ((globals()->gSound[whichSound].soundHandle.get() != NULL) &&
                (whichSound < kSoundNum)) {
            whichSound++;
        }

        if (whichSound == kSoundNum) {
            throw Exception("Can't manage any more sounds");
        }

        globals()->gSound[whichSound].soundHandle = SoundDriver::driver()->open_sound(
                format("/sounds/{0}", soundID));
        globals()->gSound[whichSound].id = soundID;
    }
    return whichSound;
}

void SoundFXCleanup() {
    for (int i = 0; i < kMaxChannelNum; i++) {
        globals()->gChannel[i].channelPtr.reset();
    }

    for (int i = 0; i < kSoundNum; i++) {
        globals()->gSound[i].soundHandle.reset();
    }
}

void quiet_all() {
    for (int i = 0; i < kMaxChannelNum; i++) {
        globals()->gChannel[i].channelPtr->quiet();
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
        int32_t mvolume, SpaceObject* mobjectptr, int32_t msoundid, int32_t msoundpersistence,
        soundPriorityType msoundpriority) {
    if (mobjectptr->distanceFromPlayer < kMaximumRelevantDistanceSquared) {
        int32_t mdistance = mobjectptr->distanceFromPlayer;
        uint32_t mul1;
        uint32_t mul2;
        SpaceObject* mplayerobjectptr;

        if (mdistance == 0) {
            if (globals()->gPlayerShipNumber >= 0) {
                mplayerobjectptr = mGetSpaceObjectPtr(globals()->gPlayerShipNumber);
            } else {
                mplayerobjectptr = NULL;
            }
            if ((mplayerobjectptr != NULL) && (mplayerobjectptr->active)) {
                mul1 = ABS<int>(mplayerobjectptr->location.h - mobjectptr->location.h);
                mul2 = mul1;
                mul1 =  ABS<int>(mplayerobjectptr->location.v - mobjectptr->location.v);
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
                            mplayerobjectptr->location.h, mplayerobjectptr->location.v,
                            mobjectptr->location.h, mobjectptr->location.v,
                            mplayerobjectptr->velocity.h - mobjectptr->velocity.h,
                            mplayerobjectptr->velocity.v - mobjectptr->velocity.v,
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
            if (globals()->gPlayerShipNumber >= 0) {
                mplayerobjectptr = mGetSpaceObjectPtr(globals()->gPlayerShipNumber);
            } else {
                mplayerobjectptr = NULL;
            }
            if ((mplayerobjectptr != NULL) && (mplayerobjectptr->active)) {
                if (mvolume > 0) {
                    PlayLocalizedSound(
                            mplayerobjectptr->location.h, mplayerobjectptr->location.v,
                            mobjectptr->location.h, mobjectptr->location.v,
                            mplayerobjectptr->velocity.h - mobjectptr->velocity.h,
                            mplayerobjectptr->velocity.v - mobjectptr->velocity.v,
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
