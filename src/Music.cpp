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

#include "Music.h"

#include "AIFF.h"
#include "AresGlobalType.h"
#include "ConditionalMacros.h"
#include "Debug.h"
#include "Error.h"
#include "MAD.h"
#include "Options.h"
#include "RDriver.h"
#include "Resources.h"
#include "Sound.h"
#include "SoundFX.h"

/********************************************************/
/*
    Player PRO 4.4 -- Music Driver EXAMPLE

    Library Version 3.5

    To use with MusicLibrary 3.5 for Think C & CodeWarrior

    Antoine ROSSET
    16 Tranchees
    1206 GENEVA
    SWITZERLAND

    FAX: (41 22) 346 11 97
    Compuserve: 100277,164
    Internet: rosset@dial.eunet.ch
*/
/********************************************************/

#define kTrackNumber    4

#define kMusicError     "\pMUSC"

#ifndef __CFM68K__
#if TARGET_OS_MAC
#define kUseMusic
#endif
#endif

extern aresGlobalType *gAresGlobal;
//extern unsigned long  gAresGlobal->gOptions;
//extern long               gAresGlobal->gSoundVolume;

int MusicInit( void)

{
    MADDriverSettings   init;
    OSErr               error;

    if ( gAresGlobal->gOptions & kOptionMusicDriver)
    {
    #ifdef kUseMusic
#ifndef powerc
/*      init.numChn             = 4;
        init.outPutBits         = 8;
        init.outPutRate         = rate22khz;
        init.outPutMode         = MonoOutPut;
        init.driverMode         = SoundManagerDriver;
        init.antiAliasing       = false;
        init.repeatMusic        = true;
        init.sysMemory          = false;
        init.Interpolation      = false;
        init.MicroDelay         = false;
        init.MicroDelaySize     = 0;
        init.surround           = false;
        init.Reverb             = false;                                // Reverb effect active? true/false
        init.ReverbSize         = 0;                            // Reverb delay duration (in ms, min = 25 ms, max 1 sec = 1000 ms)
        init.ReverbStrength     = 0;                        // Reverb strength in % (0 <-> 70)
        init.TickRemover        = false;                        // Remove volume/sample/loop ticks.
*/
init.numChn             = 4;
init.outPutBits         = 8;
init.outPutRate         = rate22khz;
init.outPutMode         = MonoOutPut;
init.driverMode         = SoundManagerDriver;
init.antiAliasing       = false;
init.repeatMusic        = true;
init.Interpolation      = false;
init.MicroDelay         = false;
init.MicroDelaySize     = 40;
init.surround           = false;
init.sysMemory          = false;
//init.Reverb               = false;
//init.ReverbSize           = 45;
//init.ReverbStrength       = 60;
//init.TickRemover      = false;
#else
/*      init.numChn             = 4;
        init.outPutBits         = 16;
        init.outPutRate         = rate22khz;
        init.outPutMode         = StereoOutPut;
        init.driverMode         = SoundManagerDriver;
        init.antiAliasing       = false;
        init.repeatMusic        = true;
        init.sysMemory          = false;
        init.Interpolation      = false;
        init.MicroDelay         = true;
        init.MicroDelaySize     = 40;
        init.surround           = true;
        init.Reverb             = false;                                // Reverb effect active? true/false
        init.ReverbSize         = 0;                            // Reverb delay duration (in ms, min = 25 ms, max 1 sec = 1000 ms)
        init.ReverbStrength     = 0;                        // Reverb strength in % (0 <-> 70)
        init.TickRemover        = true;                     // Remove volume/sample/loop ticks.
*/
init.numChn             = 4;
init.outPutBits         = 8;
init.outPutRate         = rate22khz;
init.outPutMode         = StereoOutPut;
init.driverMode         = SoundManagerDriver;
init.antiAliasing       = false;
init.repeatMusic        = true;
init.Interpolation      = false;
init.MicroDelay         = true;
init.MicroDelaySize     = 40;
init.surround           = true;
init.sysMemory          = false;
//init.Reverb               = false;
//init.ReverbSize           = 45;
//init.ReverbStrength       = 60;
//init.TickRemover      = false;
#endif
        error = MADInitLibrary("\p"/*, false*/);
        if( error != noErr)
        {
            mWriteDebugString("\pMusic Init Err");
            gAresGlobal->gOptions &= ~kOptionMusicDriver;
            ShowErrorOfTypeOccurred( eQuitErr, kErrorStrID, kInitMusicLibraryError, error, __FILE__, 1);
            return( MEMORY_ERROR);
        }
    /*  MADGetBestDriver( &init);
        if( error != noErr)
        {
            ShowErrorRecover( MEMORY_ERROR, "\pMADGetBestDriver", error);
            return( MEMORY_ERROR);
        }
    */  error = MADCreateDriver( &init);
        if( error != noErr)
        {
            mWriteDebugString("\pCreate Driver Err");
            gAresGlobal->gOptions &= ~kOptionMusicDriver;
            ShowErrorOfTypeOccurred( eQuitErr, kErrorStrID, kCreateMusicDriverError, error, __FILE__, 2);
            return( MEMORY_ERROR);
        }
    #endif
    }
    return ( kNoError);
}

void MusicCleanup( void)

{
    if ( gAresGlobal->gOptions & kOptionMusicDriver)
    {
#ifdef kUseMusic
    MADDriver->Reading = false;
    MADStop();                          // Stop the driver
    MADDisposeMusic();                  // Clear music
    MADDisposeDriver();                 // Dispose driver
    MADDisposeLibrary();                // Close Music Library
#endif
    }
}

void PlaySong( void)

{
    OSErr   err;

    if ( gAresGlobal->gOptions & kOptionMusicDriver)
    {
    #ifdef kUseMusic
        err = MADPlay();
        if ( err != noErr)
        {
            ShowErrorOfTypeOccurred( eContinueOnlyErr, kErrorStrID, kPlaySongError, err, __FILE__, 3);
        } else
        {
            MADDriver->Reading = TRUE;
        }
    //  MADDriver->Reading = !MADDriver->Reading; // toggles playing of music
    #endif
    }
}

Boolean SongIsPlaying( void)
{
    if ( gAresGlobal->gOptions & kOptionMusicDriver)
    {
    #ifdef kUseMusic
        return ( MADDriver->Reading);
    #else
        return( false);
    #endif
    } else return( false);
}

void ToggleSong( void)
{
    if ( gAresGlobal->gOptions & kOptionMusicDriver)
    {
    #ifdef kUseMusic
//      MADDriver->Reading = !MADDriver->Reading; // toggles playing of music
        if ( SongIsPlaying())
        {
            MADDriver->Reading = false;
            MADStop();
        } else
        {
            MADPlay();
            MADDriver->Reading = true;
        }
    #endif
    }
}

void StopAndUnloadSong( void)

{
    if (( gAresGlobal->gOptions & kOptionMusicDriver) && ( SongIsPlaying()))
    {
    #ifdef kUseMusic
        MADDriver->Reading = false;
        MADStop();
        MADDisposeMusic();
    #endif
    }
}

void LoadSong( short resID)

{
    OSErr   err;

    if ( resID <= 0) return;

    if ( gAresGlobal->gOptions & kOptionMusicDriver)
    {
    #ifdef kUseMusic
        err = MADLoadMusicRsrc( 'MADH', resID);
        if( err != noErr)
        {
            ShowErrorOfTypeOccurred( eContinueOnlyErr, kErrorStrID, kLoadSongError, err, __FILE__, 4);
        }
    #endif
    }
}

long GetSongVolume( void)

{
#ifdef kUseMusic
    if ( gAresGlobal->gOptions & kOptionMusicDriver)
    {
        return( (long)MADDriver->VolExt[0]);
    } else return( 0);
#else
    return( 0);
#endif
}

void SetSongVolume( long volume)

{
    volume = gAresGlobal->gSoundVolume * kMaxMusicVolume;
    volume /= kMaxVolumePreference;
    if ( gAresGlobal->gOptions & kOptionMusicDriver)
    {
    #ifdef kUseMusic
        short   i;

        for ( i = 0; i < kTrackNumber; i++)
        {
            MADDriver->VolExt[i] = volume;
        }
    #endif
    }
}

/*** END PLAYERPRO LIBRARY 3.5 ***/
