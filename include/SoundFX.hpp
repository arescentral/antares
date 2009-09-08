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

#ifndef ANTARES_SOUND_FX_HPP_
#define ANTARES_SOUND_FX_HPP_

// SOUND FX.H

#include <Base.h>

#include "MathSpecial.hpp"
#include "Sound.h"

//#ifdef powerc
//#define kAllowSoundSprocket
//#endif

#ifdef kAllowSoundSprocket
#include <SoundSprocket.h>
#include <SoundComponents.h>
#endif

#define kSoundNum           48
#define kMaxChannelNum      3

#define kMaxVolumePreference    8

#define kMediumLoudVolume   192
#define kMediumVolume       128
#define kMediumLowVolume    64
#define kMaxSoundVolume     255         // minimum = 0

#define kShortPersistence       10      // in ticks
#define kMediumPersistence      20
#define kMediumLongPersistence  40
#define kLongPersistence        60

#define kPlayerBeamSound        500
#define kBasicExplosionSound    501
#define kPlayerPulseSound       502
#define kSmallHitSound          503
#define kAlienBeamSound         504
#define kAlienPulseSound        505
#define kMorseBeepSound         506 // ship receives order
#define kComputerBeep1          507 // ship selected
#define kComputerBeep2          508 // ship built
#define kComputerBeep3          509 // button push
#define kComputerBeep4          510 // change range
#define kWarningTone            511 // naughty beep
#define kHarshBeam              512
#define kLandingWoosh           513
#define kCloakOff               522
#define kCloakOn                523
#define kKlaxon                 525
#define kWarpOne                526
#define kWarpTwo                527
#define kWarpThree              528
#define kWarpFour               529
#define kTeletype               535


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
    SndChannelPtr       channelPtr;
    Boolean             useSoundSprocket;
#ifdef kAllowSoundSprocket
    SSpSourceReference  source;
#endif
};

struct smartSoundHandle {
    Handle              soundHandle;
    long                offset;
    short               id;
    Boolean             keepMe;
};

int OpenSoundFile( void);
int InitSoundFX( void);
void SetListenerLocation( long, long);
void SetAllSoundsNoKeep( void);
void KeepSound( short);
void RemoveAllUnusedSounds( void);
void ResetAllSounds( void);
short AddSound( short);
void PlayVolumeSound( short, short, short, soundPriorityType);
void PlayLocalizedSound( unsigned long, unsigned long, unsigned long, unsigned long, smallFixedType, smallFixedType, short, short, short, soundPriorityType);
void SoundFXCleanup( void);
void UnlockSoundCallback( Handle);

struct spaceObjectType;
void mPlayDistanceSound(
        long& mdistance, long mvolume, spaceObjectType* mobjectptr, long msoundid,
        long msoundpersistence, soundPriorityType msoundpriority, unsigned long& mul1,
        unsigned long& mul2, spaceObjectType*& mplayerobjectptr);

#endif // ANTARES_SOUND_FX_HPP_
