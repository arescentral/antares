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

#include "SoundFX.hpp"

#include "sfz/Exception.hpp"
#include "AresGlobalType.hpp"
#include "FakeSounds.hpp"
#include "MathMacros.hpp"
#include "MathSpecial.hpp"
#include "Options.hpp"
#include "Preferences.hpp"
#include "SpaceObject.hpp"
#include "UniverseUnit.hpp"

using sfz::Exception;
using sfz::scoped_array;

namespace antares {

// sound 0-14 always used -- loaded at start; 15+ may be swapped around
const int kMinVolatileSound = 15;

const double kHackRangeMultiplier = 0.0025;

extern scoped_array<spaceObjectType> gSpaceObjectData;
extern coordPointType gGlobalCorner;

void InitSoundFX() {
    for (int i = 0; i < kMaxChannelNum; i++) {
        globals()->gChannel[i].channelPtr = nil;
    }

    for (int i = 0; i < kMaxChannelNum; i++) {
        globals()->gChannel[i].soundAge = 0;
        globals()->gChannel[i].soundPriority = kNoSound;
        globals()->gChannel[i].whichSound = -1;
        globals()->gChannel[i].channelPtr = SoundDriver::driver()->new_channel();
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
    AddSound(kWarpOne);
    AddSound(kWarpTwo);
    AddSound(kWarpThree);
    AddSound(kWarpFour);
    AddSound(kTeletype);
}

void PlayVolumeSound(
        short whichSoundID, short amplitude, short persistence, soundPriorityType priority) {
    short oldestSoundTime = -kLongPersistence, whichChannel = -1;
    const int global_volume = Preferences::preferences()->volume();

    if ((global_volume > 0) && (amplitude > 0)) {
        int timeDif = TickCount() - globals()->gLastSoundTime;
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

        globals()->gLastSoundTime = TickCount();

        int whichSound = 0;
        while ((globals()->gSound[whichSound].id != whichSoundID) && (whichSound < kSoundNum)) {
            whichSound++;
        }
        if (whichSound == kSoundNum) {
            whichChannel = -1;
        }

        if (whichChannel >= 0) {
            globals()->gChannel[whichChannel].whichSound = whichSoundID;
            globals()->gChannel[whichChannel].soundAge = -persistence;
            globals()->gChannel[whichChannel].soundPriority = priority;
            globals()->gChannel[whichChannel].soundVolume = amplitude;

            globals()->gChannel[whichChannel].channelPtr->quiet();

            int newvol = (amplitude * global_volume) >> 3;

            globals()->gChannel[whichChannel].channelPtr->amp(newvol);
            globals()->gChannel[whichChannel].channelPtr->play(
                    globals()->gSound[whichSound].soundHandle.get());
        }
    }
}

void PlayLocalizedSound(
        unsigned long sx, unsigned long sy, unsigned long dx, unsigned long dy,
        smallFixedType hvel, smallFixedType vvel, short whichSoundID, short amplitude,
        short persistence, soundPriorityType priority) {
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
                (globals()->gSound[count].soundHandle.get() != nil)) {
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
    short whichSound;

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
        while ((globals()->gSound[whichSound].soundHandle.get() != nil) &&
                (whichSound < kSoundNum)) {
            whichSound++;
        }

        if (whichSound == kSoundNum) {
            throw Exception("Can't manage any more sounds");
        }

        globals()->gSound[whichSound].soundHandle.reset(new Sound(soundID));
        globals()->gSound[whichSound].id = soundID;
    }
    return whichSound;
}

void SoundFXCleanup() {
    for (int i = 0; i < kMaxChannelNum; i++) {
        if (globals()->gChannel[i].channelPtr != nil) {
            delete globals()->gChannel[i].channelPtr;
        }
    }

    for (int i = 0; i < kSoundNum; i++) {
        globals()->gSound[i].soundHandle.reset();
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
        long mvolume, spaceObjectType* mobjectptr, long msoundid, long msoundpersistence,
        soundPriorityType msoundpriority) {
    if (mobjectptr->distanceFromPlayer < kMaximumRelevantDistanceSquared) {
        int32_t mdistance = mobjectptr->distanceFromPlayer;
        uint32_t mul1;
        uint32_t mul2;
        spaceObjectType* mplayerobjectptr;

        if (mdistance == 0) {
            if (globals()->gPlayerShipNumber >= 0) {
                mplayerobjectptr = gSpaceObjectData.get() + globals()->gPlayerShipNumber;
            } else {
                mplayerobjectptr = nil;
            }
            if ((mplayerobjectptr != nil) && (mplayerobjectptr->active)) {
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
                mplayerobjectptr = gSpaceObjectData.get() + globals()->gPlayerShipNumber;
            } else {
                mplayerobjectptr = nil;
            }
            if ((mplayerobjectptr != nil) && (mplayerobjectptr->active)) {
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
