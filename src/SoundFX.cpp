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

// Sound Player.c

#include "SoundFX.hpp"

#include "AresGlobalType.hpp"
#include "Debug.hpp"
#include "Error.hpp"
#include "FakeSounds.hpp"
#include "MathMacros.hpp"
#include "MathSpecial.hpp"
#include "Options.hpp"
#include "Resources.h"
#include "SpaceObject.hpp"
#include "UniverseUnit.hpp"

#define kSoundFXError           "\pSNDX"
#define kSoundResID             500

#define kSoundResFileName       "\p:Ares Data Folder:Ares Sounds"

#define kMinVolatileSound       15  // sound 0-14 always used -- loaded at start; 15+ may be swapped around

#define kHackRangeMultiplier    0.0025

inline float mSmallFixedToVelocity(Fixed mvel) {
    return ((.125) * implicit_cast<float>(mvel)) / 256;
}

OSStatus MyPanSoundFromRightToLeft( Handle);
OSStatus My3DSoundInit( void);
#ifdef kAllowSoundSprocket
SSpSourceReference MyCreateSource( void);
#endif
SndChannelPtr MyCreateLocalizedChannel( void);

extern TypedHandle<spaceObjectType> gSpaceObjectData;
extern coordPointType gGlobalCorner;

#ifdef kAllowSoundSprocket
SSpListenerReference    gListener;
#endif

int InitSoundFX( void)

{
    OSErr           err;
    int             i;
    aresGlobalType  *glob = globals();

//  glob->gOptions |= kOptionSoundAvailable;

    for ( i = 0; i < kMaxChannelNum; i++)
    {
        glob->gChannel[i].channelPtr = nil;//(SndChannelPtr)NewPtr( sizeof( SndChannel));
    }

#ifdef kkkAllowSoundSprocket
    // check for sound sprocket
    // If SoundSprocket is not present then create a listener and set things up
    if (((Ptr) SSpConfigureSpeakerSetup == (Ptr) kUnresolvedCFragSymbolAddress) ||
        ( (glob->gOptions & kOptionMusicDriver)))
#endif
    {
        // no sound sprocket; set up channels normally
        glob->gOptions &= ~kOptionSoundSprocketOn;
        for ( i = 0; i < kMaxChannelNum; i++)
        {
            glob->gChannel[i].soundAge = 0;
            glob->gChannel[i].soundPriority = kNoSound;
            glob->gChannel[i].whichSound = -1;
            glob->gChannel[i].useSoundSprocket = false;

            err = SndNewChannel( &(glob->gChannel[i].channelPtr),
                    sampledSynth, initMono, nil);
            if (( err != noErr) || ( glob->gChannel[i].channelPtr == nil))
            {
                ShowErrorOfTypeOccurred( eContinueOnlyErr, kErrorStrID, SOUND_CHANNEL_ERROR, err, __FILE__, 0);

//              ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, SOUND_CHANNEL_ERROR, -1, -1, -1, __FILE__, 1);
                SoundFXCleanup();
                glob->gOptions &= kOptionSoundAvailable;
                return ( SOUND_CHANNEL_ERROR);
            }
        }
    }
#ifdef kkkAllowSoundSprocket
    else
    {
        SysBeep(20);
        // we have SoundSprocket; set it up!
        mWriteDebugString("\pInitializing SoundSprocket!");
        globals()->gOptions |= kOptionSoundSprocketOn;

        My3DSoundInit(); // sets up gListener

        for ( i = 0; i < kMaxChannelNum; i++)
        {
            globals()->gChannel[i].source = MyCreateSource();
            globals()->gChannel[i].channelPtr = MyCreateLocalizedChannel();
            if ( globals()->gChannel[i].channelPtr == nil)
            {
                ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, SOUND_CHANNEL_ERROR, -1, -1, -1, __FILE__, 91);
            }

            globals()->gChannel[i].soundAge = 0;
            globals()->gChannel[i].soundPriority = kNoSound;
            globals()->gChannel[i].whichSound = -1;
            globals()->gChannel[i].useSoundSprocket = true;
        }
//      SetListenerLocation( 0, 0);
    }
#endif

    ResetAllSounds();
    AddSound( kComputerBeep4);
    AddSound( kComputerBeep1);
    AddSound( kComputerBeep2);
    AddSound( kComputerBeep3);
    AddSound( kMorseBeepSound);
    AddSound( kWarningTone);
    AddSound( kLandingWoosh);
    AddSound( kCloakOn);
    AddSound( kCloakOff);
    AddSound( kKlaxon);
    AddSound( kWarpOne);
    AddSound( kWarpTwo);
    AddSound( kWarpThree);
    AddSound( kWarpFour);
    AddSound( kTeletype);
    /*
    for ( i = 0; i < kSoundNum; i++)
    {
        globals()->gSound[i].soundHandle = GetResource( 'snd ', kSoundResID + i);
        if ( globals()->gSound[i].soundHandle == nil)
        {
            ShowErrorRecover( RESOURCE_ERROR, kSoundFXError, 2);
            SoundFXCleanup();
            return ( RESOURCE_ERROR);
        }
        GetSoundHeaderOffset( (SndListHandle)globals()->gSound[i].soundHandle, &(globals()->gSound[i].offset));
        if ( err != noErr)
        {
            ShowErrorRecover( SOUND_CHANNEL_ERROR, kSoundFXError, 3);
            SoundFXCleanup();
            return ( SOUND_CHANNEL_ERROR);
        }
        DetachResource( globals()->gSound[i].soundHandle);
        mHandleLockAndRegister( globals()->gSound[i].soundHandle, UnlockSoundCallback, nil, nil)

    }
    */

    // check for sound sprocket
    // *    If SoundSprocket is present then create a listener and set things up
    /*
    if ((Ptr) SSpConfigureSpeakerSetup == (Ptr) kUnresolvedCFragSymbolAddress)
    {
        globals()->gOptions &= ~kOptionSoundSprocketOn;
        return( -1);  // no sound sprocket
    } else
    {
        OSStatus status = noErr;

        // *    We have sound sprocket, so now we install the filters and create source and listener objects.
        globals()->gOptions |= kOptionSoundSprocketOn;

        // *    Create the listener
        status = SSpListener_New(&gListener);
        if (status)
            ShowSimpleStringAlert("\pCould not create a sound sprocket listener.",
                nil, nil, nil);

        // *    Define our unit of distance measurement
        status = SSpListener_SetMetersPerUnit(gListener, 0.05);
        if (status)
            ShowSimpleStringAlert ("\pCould not set reference distance for listener.",
                nil, nil, nil);

        // *    Attach the sound localization component to each of the sound
        // *    channels that we'll be playing through.
        for (i = 0; i < kMaxChannelNum; i++)
        {
            SoundComponentLink  myLink;

            // *    Create the source
            status = SSpSource_New(&globals()->gChannel[i].source);
            if (status)
                ShowSimpleStringAlert("\pCould not create a sound sprocket source.",
                    nil, nil, nil);

            // *    Install the filter
            myLink.description.componentType = kSoundEffectsType;
            myLink.description.componentSubType = kSSpLocalizationSubType;
            myLink.description.componentManufacturer = 0;
            myLink.description.componentFlags = 0;
            myLink.description.componentFlagsMask = 0;
            myLink.mixerID = nil;
            myLink.linkID = nil;

            status = SndSetInfo(globals()->gChannel[i].channelPtr, siPreMixerSoundComponent, &myLink);
            if (status)
                ShowSimpleStringAlert("\pCould not install the sound sprocket filter into the channel.",
                    nil, nil, nil);
        }
        SetListenerLocation( 0, 0);
    }
    */
//  MyPanSoundFromRightToLeft( globals()->gSound[6].soundHandle);
//  MyPanSoundFromRightToLeft( globals()->gSound[4].soundHandle);
    return ( kNoError);
}

void SetListenerLocation( long x, long y)
{
#ifdef kAllowSoundSprocket

OSStatus            theErr = noErr;
TQ3CameraPlacement  location;

    if (!((globals()->gOptions & kOptionSoundSprocketOn) && ( globals()->gOptions & kOptionSoundAvailable)))
        return;

    // *    Set the listener in the bottom middle of the screen
    location.cameraLocation.x = 0;
    location.cameraLocation.y = 0;
    location.cameraLocation.z = 0;

    // *    Focus the listener's attention on the exact middle of the screen
    location.pointOfInterest.x = 0;
    location.pointOfInterest.y = 1;
    location.pointOfInterest.z = 0;

    location.upVector.x = 0;
    location.upVector.y = 0;
    location.upVector.z = -1;

    theErr = SSpListener_SetCameraPlacement (gListener, &location);
    if (theErr)
        ShowSimpleStringAlert ("\pFailed to set the listener location",
            nil, nil, nil);
#else
#pragma unused ( x, y)
#endif
}

void PlayVolumeSound( short whichSoundID, short amplitude, short persistence, soundPriorityType priority)

{
    SndCommand      cmd;
    OSErr           err;
    short           count, oldestSoundTime = -kLongPersistence, whichChannel = -1, whichSound;
    long            timeDif, newvol;

    if ((globals()->gOptions & kOptionSoundSprocketOn) || ( globals()->gOptions & kOptionSoundAvailable))
        return;
    if (( globals()->gSoundVolume > 0) && ( amplitude > 0))
    {
        timeDif = TickCount() - globals()->gLastSoundTime;
        for ( count = 0; count < kMaxChannelNum; count++)
            globals()->gChannel[count].soundAge += timeDif;

        // if not see if there's another channel with the same sound at same or lower volume

        count = 0;
        if ( priority > kVeryLowPrioritySound)
        {
            while (( count < kMaxChannelNum) && ( whichChannel == -1))
            {
                if (( globals()->gChannel[count].whichSound == whichSoundID) &&
                    ( globals()->gChannel[count].soundVolume <= amplitude))
                    whichChannel = count;
                count++;
            }
        }

        // first see if there's an unused channel

/*      if ( whichChannel == -1)
        {
            count = 0;
            while (( count < kMaxChannelNum) && ( whichChannel == -1))
            {
                err = SndChannelStatus( globals()->gChannel[count].channelPtr, (short)sizeof( SCStatus),
                        (SCStatusPtr)&status);
                if ( !status.scChannelBusy)
                    whichChannel = count;
                count++;
            }
        }
*/
        // if not see if there's another channel at lower volume

        if ( whichChannel == -1)
        {
            count = 0;
            while (( count < kMaxChannelNum) && ( whichChannel == -1))
            {
                if ( globals()->gChannel[count].soundVolume < amplitude)
                    whichChannel = count;
                count++;
            }
        }

        // if not see if there's another channel at lower priority

        if ( whichChannel == -1)
        {
            count = 0;
            while (( count < kMaxChannelNum) && ( whichChannel == -1))
            {
                if ( globals()->gChannel[count].soundPriority < priority)
                    whichChannel = count;
                count++;
            }
        }

        // if not, take the oldest sound if past minimum persistence
        if ( whichChannel == -1)
        {
            count = 0;
            while ( count < kMaxChannelNum)
            {
                if (( globals()->gChannel[count].soundAge > 0) && ( globals()->gChannel[count].soundAge > oldestSoundTime))
                {
                    oldestSoundTime = globals()->gChannel[count].soundAge;
                    whichChannel = count;
                }
                count++;
            }
        }

        // we're not checking for importance

        globals()->gLastSoundTime = TickCount();

        whichSound = 0;
        while ((globals()->gSound[whichSound].id != whichSoundID) && ( whichSound < kSoundNum)) { whichSound++;}
        if ( whichSound == kSoundNum) whichChannel = -1;

        if ( whichChannel >= 0)
        {
    //      WriteDebugLong(whichChannel);

            globals()->gChannel[whichChannel].whichSound = whichSoundID;
            globals()->gChannel[whichChannel].soundAge = -persistence;
            globals()->gChannel[whichChannel].soundPriority = priority;
            globals()->gChannel[whichChannel].soundVolume = amplitude;


            cmd.param1 = 0;
            cmd.param2 = 0;
            cmd.cmd = quietCmd;
            err = SndDoImmediate( globals()->gChannel[whichChannel].channelPtr, &cmd);
            if ( err != noErr)
            {
                WriteDebugLine("\pSnd Err:");
                WriteDebugLong(err);
            }

            cmd.param1 = 0;
            cmd.param2 = 0;
            cmd.cmd = flushCmd;
            err = SndDoImmediate( globals()->gChannel[whichChannel].channelPtr, &cmd);
            if ( err != noErr)
            {
                WriteDebugLine("\pSnd Err:");
                WriteDebugLong(err);
            }

            newvol = amplitude;
            newvol *= globals()->gSoundVolume;
            newvol >>= 3;

            cmd.param1 = newvol;
            cmd.param2 = 0;
            cmd.cmd = ampCmd;

            err = SndDoCommand( globals()->gChannel[whichChannel].channelPtr, &cmd, false);
    //      err = SndDoImmediate( globals()->gChannel[whichChannel].channelPtr, &cmd);
            if ( err != noErr)
            {
                WriteDebugLine("\pSnd Err:");
                WriteDebugLong(err);
            }

/*          cmd.param1 = 0;
            cmd.param2 = (unsigned long)( *(globals()->gSound[whichSound].soundHandle) + globals()->gSound[whichSound].offset);
            cmd.cmd = bufferCmd;
            err = SndDoCommand( globals()->gChannel[whichChannel].channelPtr, &cmd, true);
            if ( err != noErr)
            {
                WriteDebugLine((char *)"\pSnd Err:");
                WriteDebugLong(err);
            }
*/
            err = SndPlay( globals()->gChannel[whichChannel].channelPtr, globals()->gSound[whichSound].soundHandle, true);

        }
    }
}

void PlayLocalizedSound( unsigned long sx, unsigned long sy, unsigned long dx,
    unsigned long dy, smallFixedType hvel, smallFixedType vvel, short whichSoundID,
    short amplitude, short persistence,
    soundPriorityType priority)

{
    SndCommand      cmd;
    OSErr           err;
    short           count, oldestSoundTime = -kLongPersistence, whichChannel = -1, whichSound;
    long            timeDif;

#pragma unused ( sx, sy, dx, dy, hvel, vvel)
    PlayVolumeSound( whichSoundID, amplitude, persistence, priority);
    return;


    if (( globals()->gSoundVolume > 0) && ( amplitude > 0))
    {
        timeDif = TickCount() - globals()->gLastSoundTime;
        for ( count = 0; count < kMaxChannelNum; count++)
            globals()->gChannel[count].soundAge += timeDif;

        // if not see if there's another channel with the same sound at same or lower volume

        count = 0;
        while (( count < kMaxChannelNum) && ( whichChannel == -1))
        {
            if (( globals()->gChannel[count].whichSound == whichSoundID) &&
                ( globals()->gChannel[count].soundVolume <= amplitude))
                whichChannel = count;
            count++;
        }

        // if not see if there's another channel at lower volume

        if ( whichChannel == -1)
        {
            count = 0;
            while (( count < kMaxChannelNum) && ( whichChannel == -1))
            {
                if ( globals()->gChannel[count].soundVolume < amplitude)
                    whichChannel = count;
                count++;
            }
        }

        // if not see if there's another channel at lower priority

        if ( whichChannel == -1)
        {
            count = 0;
            while (( count < kMaxChannelNum) && ( whichChannel == -1))
            {
                if ( globals()->gChannel[count].soundPriority < priority)
                    whichChannel = count;
                count++;
            }
        }

        // if not, take the oldest sound if past minimum persistence
        if ( whichChannel == -1)
        {
            count = 0;
            while ( count < kMaxChannelNum)
            {
                if (( globals()->gChannel[count].soundAge > 0) && ( globals()->gChannel[count].soundAge > oldestSoundTime))
                {
                    oldestSoundTime = globals()->gChannel[count].soundAge;
                    whichChannel = count;
                }
                count++;
            }
        }

        // we're not checking for importance

        globals()->gLastSoundTime = TickCount();

        whichSound = 0;
        while ((globals()->gSound[whichSound].id != whichSoundID) && ( whichSound < kSoundNum)) { whichSound++;}
        if ( whichSound == kSoundNum) whichChannel = -1;

        if ( whichChannel >= 0)
        {
            globals()->gChannel[whichChannel].whichSound = whichSoundID;
            globals()->gChannel[whichChannel].soundAge = -persistence;
            globals()->gChannel[whichChannel].soundPriority = priority;
            globals()->gChannel[whichChannel].soundVolume = amplitude;


            cmd.param1 = 0;
            cmd.param2 = 0;
            cmd.cmd = quietCmd;
            err = SndDoImmediate( globals()->gChannel[whichChannel].channelPtr, &cmd);
            if ( err != noErr)
            {
                WriteDebugLine("\pSnd Err:");
                WriteDebugLong(err);
            }

            cmd.param1 = 0;
            cmd.param2 = 0;
            cmd.cmd = flushCmd;
            err = SndDoImmediate( globals()->gChannel[whichChannel].channelPtr, &cmd);
            if ( err != noErr)
            {
                WriteDebugLine("\pSnd Err:");
                WriteDebugLong(err);
            }


#ifdef kAllowSoundSprocket
            if (globals()->gOptions & kOptionSoundSprocketOn)
            {
                TQ3Point3D          myPoint;
                TQ3Vector3D         myVector;
                SSpLocalizationData myLocalization;
                TQ3CameraPlacement  location;
                SSpLocalizationData localization;

                if ( dx < sx)
                {
                    x = sx - dx;
                    x = 0-x;
                } else
                {
                    x = dx - sx;
                }

                if ( dy < sy)
                {
                    y = sy - dy;
                    y = 0-y;
                } else
                {
                    y = dy - sy;
                }

                myPoint.x = 0.0;
                myPoint.y = y;
                myPoint.y *= kHackRangeMultiplier;
                myPoint.z = x;
                myPoint.z *= kHackRangeMultiplier;

                myVector.x = 0.0;
                myVector.y = 0;//mSmallFixedToVelocity( vvel);
                myVector.z = 0;//mSmallFixedToVelocity( hvel);

                // retrieve update info - send it to localization component
                theErr = SSpSource_SetPosition( globals()->gChannel[whichChannel].source, &myPoint);
                if (theErr)
                    ShowSimpleStringAlert ("\pFailed to set the source position",
                        nil, nil, nil);

                theErr = SSpSource_SetVelocity( globals()->gChannel[whichChannel].source,
                    &myVector);
                if (theErr)
                    ShowSimpleStringAlert ("\pFailed to set the velocity.",
                        nil, nil, nil);

                theErr = SSpSource_CalcLocalization( globals()->gChannel[whichChannel].source, gListener,
                    &myLocalization);
                if (theErr)
                    ShowSimpleStringAlert ("\pFailed to calculate the localization",
                        nil, nil, nil);

                theErr = SndSetInfo( globals()->gChannel[whichChannel].channelPtr, siSSpLocalization,
                    &myLocalization);
                if (theErr)
                    ShowSimpleStringAlert("\pFailed to localize the channel",
                        nil, nil, nil);
/*

                // *    Position the sound in space.  The lower left corner of the screen is (0, 0)
                location.cameraLocation.x = myPoint.x;
                location.cameraLocation.y = myPoint.y;
                location.cameraLocation.z = 0;

                // *    Orient the sound so that it is down directed down towards the listener
                location.pointOfInterest.x = 320;
                location.pointOfInterest.y = 240;
                location.pointOfInterest.z = 0;

                location.upVector.x = 0;
                location.upVector.y = 0;
                location.upVector.z = -1;

                theErr = SSpSource_SetCameraPlacement (globals()->gChannel[whichChannel].source, &location);
                if (theErr)
                    ShowSimpleStringAlert ("\pFailed to set the source position",
                        nil, nil, nil);

                theErr = SSpSource_CalcLocalization (globals()->gChannel[whichChannel].source, gListener, &localization);
                if (theErr)
                    ShowSimpleStringAlert ("\pFailed to calculate the localization",
                        nil, nil, nil);

                // We don't do doppler, since we only localize the sound at the instant it is played.
                localization.currentLocation.sourceVelocity = 0;
                localization.currentLocation.listenerVelocity = 0;

                // We set the reference distance to a reasonable number that seems to get good results
                // We also provide a minimum distance or else we run into really LOUD, really CLIPPED sounds
                localization.referenceDistance = 20.0;
                if (localization.currentLocation.distance < 5.0)
                    localization.currentLocation.distance = 5.0;

                theErr = SndSetInfo (globals()->gChannel[whichChannel].channelPtr, siSSpLocalization, &localization);
                if (theErr)
                    ShowSimpleStringAlert("\pFailed to localize the channel",
                        nil, nil, nil);
*/
            } else
            {

                newvol = amplitude;
                newvol *= globals()->gSoundVolume;
                newvol >>= 3;

                cmd.param1 = newvol;
                cmd.param2 = 0;
                cmd.cmd = ampCmd;

                err = SndDoCommand( globals()->gChannel[whichChannel].channelPtr, &cmd, false);
                if ( err != noErr)
                {
                    WriteDebugLine("\pSnd Err:");
                    WriteDebugLong(err);
                }

            }
#endif
            err = SndPlay( globals()->gChannel[whichChannel].channelPtr,
                globals()->gSound[whichSound].soundHandle, true);

        }
    }
}

void SetAllSoundsNoKeep( void)

{
    long    count;

    for ( count = kMinVolatileSound; count < kSoundNum; count++)
    {
        globals()->gSound[count].keepMe = false;
    }

}

void RemoveAllUnusedSounds( void)
{
    long    count;

    for ( count = kMinVolatileSound; count < kSoundNum; count++)
    {
        if ((!globals()->gSound[count].keepMe) &&
                (globals()->gSound[count].soundHandle.get() != nil)) {
            globals()->gSound[count].soundHandle.destroy();
            globals()->gSound[count].id = -1;
        }
    }
}

void ResetAllSounds( void)

{
    long    count;

    for ( count = 0; count < kSoundNum; count++)
    {
        globals()->gSound[count].keepMe = false;
        globals()->gSound[count].id = -1;
    }
}

void KeepSound( short soundID)

{
    short       whichSound;

    whichSound = 0;
    while ((globals()->gSound[whichSound].id != soundID) && ( whichSound < kSoundNum)) { whichSound++;}
    if ( whichSound < kSoundNum)
    {
        globals()->gSound[whichSound].keepMe = true;
    }
}

short AddSound( short soundID)

{
    short       whichSound;
    Str255      debugstr = "\pglobals()->gSound[XX].soundHandle";

    whichSound = 0;
    while ((globals()->gSound[whichSound].id != soundID) && ( whichSound < kSoundNum))
    {
        whichSound++;
    }
    if ( whichSound == kSoundNum)
    {
        whichSound = 0;
        while ((globals()->gSound[whichSound].soundHandle.get() != nil) &&
            ( whichSound < kSoundNum))
        {
            whichSound++;
        }

        if ( whichSound == kSoundNum)
        {
            ShowErrorAny( eContinueOnlyErr, kErrorStrID, nil, nil, nil, nil, kNoMoreSoundsError, -1, -1, -1, __FILE__, soundID);
            return( -1);
        } else
        {
            WriteDebugLine("\pADDSND>");
            WriteDebugLong( soundID);

            globals()->gSound[whichSound].soundHandle = GetSound(soundID);
            if (globals()->gSound[whichSound].soundHandle.get() == nil) {
                ShowErrorAny( eContinueOnlyErr, kErrorStrID, nil, nil, nil, nil, kLoadSoundError, -1, -1, -1, __FILE__, soundID);
//              Debugger();
                return ( -1);
            }
//          HLockHi( globals()->gSound[whichSound].soundHandle);

            debugstr[8] = '0' + (whichSound / 10);
            debugstr[9] = '0' + (whichSound % 10);
//          mDataHandleLockAndRegister( globals()->gSound[whichSound].soundHandle, UnlockSoundCallback, nil, nil, debugstr)
//          HHCheckAllHandles();
            globals()->gSound[whichSound].id = soundID;
            return( whichSound);
        }
    } else return( whichSound);
}



void SoundFXCleanup( void)

{
    int     i;
    OSErr   err;

//  CloseResFile( globals()->gSoundFileRefID);

    for ( i = 0; i < kMaxChannelNum; i++)
    {
        if (  globals()->gChannel[i].channelPtr != nil)
        {
            err = SndDisposeChannel( globals()->gChannel[i].channelPtr, true);
            if ( err != noErr)
            {
                WriteDebugLine("\pSnd Err:");
                WriteDebugLong(err);
            }
        }
    }

    WriteDebugLine("\p<SndChannels");
    for ( i = 0; i < kSoundNum; i++)
    {
        if (globals()->gSound[i].soundHandle.get() != nil) {
            globals()->gSound[i].soundHandle.destroy();
        }
    }
    WriteDebugLine("\p<SndHndles");
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

OSStatus MyPanSoundFromRightToLeft( Handle mySndResource)
{
#ifdef kAllowSoundSprocket
    OSStatus            myStatus;
    TQ3Point3D          myPoint;
    SSpLocalizationData myLocalization;
    float               myZPos;
    long                myTicks;

    mWriteDebugString("\pBEGIN PANNING");
    // start playing sound

    myStatus = SndPlay(globals()->gChannel[0].channelPtr, (SndListHandle)mySndResource, true);
    if ( myStatus != noErr)
    {
        mWriteDebugString("\pPLAY ERR");
        WriteDebugLong( myStatus);
        return( myStatus);
    }

    // slowly move source from right to left

    for ( myZPos = 10.0; myZPos >= -10.0; myZPos -= 1)
    {
        // set source position
//      Q3Point3D_Set( myPoint, 1, 0, myZPos);
        myPoint.x = 1;
        myPoint.y = 0;
        myPoint.z = myZPos;
        SSpSource_SetPosition( globals()->gChannel[0].source, &myPoint);

        // retrieve update info - send it to localization component
        SSpSource_CalcLocalization( globals()->gChannel[0].source, gListener,
            &myLocalization);
        SndSetInfo( globals()->gChannel[0].channelPtr, siSSpLocalization,
            &myLocalization);
        Delay( 15, &myTicks);
    }

    mWriteDebugString("\pEnd Panning");
#else
#pragma unused ( mySndResource)
#endif
    return( noErr);
}

// Using SoundSprocket p1-20 listin 1-2

OSStatus My3DSoundInit( void)
{
#ifdef kAllowSoundSprocket
    OSStatus    myStatus;

    myStatus = SSpListener_New( &gListener);

    if ( myStatus == noErr)
    {
        SSpListener_SetMetersPerUnit( gListener, 1);
    } else
    {
        ShowSimpleStringAlert("\pCould not create a SoundSprocket listener.",
            nil, nil, nil);
    }
    return( myStatus);
#else
    return( 0);
#endif
}

// Using SoundSprocket pp1-20 - 1-21 listing 1-3

#ifdef kAllowSoundSprocket
SSpSourceReference MyCreateSource( void)
{
    SSpSourceReference  mySource = nil;
    OSStatus            myStatus;

    myStatus = SSpSource_New( &mySource);
    if ( myStatus != noErr)
    {
        ShowSimpleStringAlert("\pCould not create a sound sprocket source.",
            nil, nil, nil);
    }
    return( mySource);
}
#endif

// Using SoundSprocket p1-17 listing 1-1

SndChannelPtr MyCreateLocalizedChannel( void)
{
#ifdef kAllowSoundSprocket
    OSStatus            myErr;
    SndChannelPtr       mySndChannel = nil;
    SoundComponentLink  myLink;

    // create the sound channel
    myErr = SndNewChannel( &mySndChannel, sampledSynth, initMono, nil);
    if ( myErr != noErr) return (nil);

    // define the type of localization component
    myLink.description.componentType = kSoundEffectsType;
    myLink.description.componentSubType = kSSpLocalizationSubType;
    myLink.description.componentManufacturer = 0;
    myLink.description.componentFlags = 0;
    myLink.description.componentFlagsMask = 0;
    myLink.mixerID = nil;
    myLink.linkID = nil;

    // install the localization compnent BEFORE the Apple Mixer
    myErr = SndSetInfo( mySndChannel, siPreMixerSoundComponent, &myLink);
    if ( myErr != noErr)
    {
        SndDisposeChannel( mySndChannel, true);
        return( nil);
    } else
    {
        return( mySndChannel);
    }
#else
    return( nil);
#endif
}

void mPlayDistanceSound(
        long& mdistance, long mvolume, spaceObjectType* mobjectptr, long msoundid,
        long msoundpersistence, soundPriorityType msoundpriority, unsigned long& mul1,
        unsigned long& mul2, spaceObjectType*& mplayerobjectptr) {
    if (mobjectptr->distanceFromPlayer < kMaximumRelevantDistanceSquared) {
        mdistance = mobjectptr->distanceFromPlayer;
        if ( mdistance == 0)
        {
            if ( globals()->gPlayerShipNumber >= 0)
                mplayerobjectptr = *gSpaceObjectData + globals()->gPlayerShipNumber;
            else mplayerobjectptr = nil;
            if (( mplayerobjectptr != nil) && ( mplayerobjectptr->active))
            {
                mul1 = ABS<int>(mplayerobjectptr->location.h - mobjectptr->location.h);
                mul2 = mul1;
                mul1 =  ABS<int>(mplayerobjectptr->location.v - mobjectptr->location.v);
                mdistance = mul1;
                if (( mul2 < kMaximumRelevantDistance) && ( mdistance < kMaximumRelevantDistance))
                    mdistance = mdistance * mdistance + mul2 * mul2;
                else mdistance = kMaximumRelevantDistanceSquared;
                mdistance = lsqrt( mdistance);
                if ( mdistance > 480)
                {
                    mdistance -= 480;
                    if ( mdistance > 1920) mvolume = 0;
                    else
                        mvolume = ( (1920 - mdistance) * mvolume) / 1920;
                }
                if ( mvolume > 0)
                    PlayLocalizedSound( mplayerobjectptr->location.h, mplayerobjectptr->location.v, mobjectptr->location.h, mobjectptr->location.v, mplayerobjectptr->velocity.h - mobjectptr->velocity.h, mplayerobjectptr->velocity.v - mobjectptr->velocity.v, msoundid, mvolume, msoundpersistence, msoundpriority);
            } else
            {
                mul1 = ABS<int>(gGlobalCorner.h - mobjectptr->location.h);
                mul2 = mul1;
                mul1 =  ABS<int>(gGlobalCorner.v - mobjectptr->location.v);
                mdistance = mul1;
                if (( mul2 < kMaximumRelevantDistance) && ( mdistance < kMaximumRelevantDistance))
                    mdistance = mdistance * mdistance + mul2 * mul2;
                else mdistance = kMaximumRelevantDistanceSquared;
                mdistance = lsqrt( mdistance);
                if ( mdistance > 480)
                {
                    mdistance -= 480;
                    if ( mdistance > 1920) mvolume = 0;
                    else
                        mvolume = ( (1920 - mdistance) * mvolume) / 1920;
                }
                if ( mvolume > 0)
                    PlayLocalizedSound( gGlobalCorner.h, gGlobalCorner.v, mobjectptr->location.h, mobjectptr->location.v, mobjectptr->velocity.h, mobjectptr->velocity.v, msoundid, mvolume, msoundpersistence, msoundpriority);
            }
        } else
        {
            mdistance = lsqrt( mdistance);
            if ( mdistance > 480)
            {
                mdistance -= 480;
                if ( mdistance > 1920) mvolume = 0;
                else
                    mvolume = ( (1920 - mdistance) * mvolume) / 1920;
            }
            if ( globals()->gPlayerShipNumber >= 0)
                mplayerobjectptr = *gSpaceObjectData + globals()->gPlayerShipNumber;
            else mplayerobjectptr = nil;
            if (( mplayerobjectptr != nil) && ( mplayerobjectptr->active))
            {
                if ( mvolume > 0)
                    PlayLocalizedSound( mplayerobjectptr->location.h, mplayerobjectptr->location.v, mobjectptr->location.h, mobjectptr->location.v, mplayerobjectptr->velocity.h - mobjectptr->velocity.h, mplayerobjectptr->velocity.v - mobjectptr->velocity.v, msoundid, mvolume, msoundpersistence, msoundpriority);
            } else
            {
                if ( mvolume > 0)
                    PlayLocalizedSound( gGlobalCorner.h, gGlobalCorner.v, mobjectptr->location.h, mobjectptr->location.v, mobjectptr->velocity.h, mobjectptr->velocity.v, msoundid, mvolume, msoundpersistence, msoundpriority);
            }
        }
    }
}
