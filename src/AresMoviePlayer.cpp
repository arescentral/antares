/*
Ares, a tactical space combat game.
Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

// Ares Movie Player.c

#include "AresMoviePlayer.h"

#include <Palettes.h>

#include "AresGlobalType.h"
#include "ConditionalMacros.h"
#include "Debug.h"
#include "Error.h"
#include "KeyMapTranslation.h"
#include "Options.h"
#include "Randomize.h"

#define kSoundVolumeMultiplier  32  // = 256 (1.0) / 8 (kMaxSoundVolumePreference)

extern aresGlobalType   *gAresGlobal;
//extern unsigned long  gAresGlobal->gOptions;
//extern long               gAresGlobal->gSoundVolume;

#define kUseMovies

void InitMoviePlayer( void)
{
#ifdef kUseMovies
    OSErr                       err;

    if ( gAresGlobal->gOptions & kOptionQuicktime)
    {
        err = EnterMovies();
    }
#endif
}

void CleanupMoviePlayer( void)
{
#ifdef kUseMovies
    if ( gAresGlobal->gOptions & kOptionQuicktime)
    {
        ExitMovies();
    }
#endif
}


void PlayMovieByName( StringPtr filePath, WindowPtr aWindow, Boolean doubleIt, GDHandle device)

{
#ifdef kUseMovies
    OSErr                       err;
    FSSpec                      fileSpec;
    Movie                       aMovie = nil;
    short                       movieResFile, count;
    Rect                        movieBox;
    Boolean                     done = false, depthSet = false;
    CTabHandle                  theClut = nil;
    PaletteHandle               thePalette = nil, originalPalette = nil;
    Fixed                       movieRate;
    TimeValue               movieTime;

#pragma unused ( device)
    if ( gAresGlobal->gOptions & kOptionQuicktime)
    {

        err = FSMakeFSSpec( 0, 0L, filePath, &fileSpec);
        if ( err == noErr)
        {
            err = OpenMovieFile ( &fileSpec, &movieResFile, fsRdPerm);
            if (err == noErr)
            {
                short               movieResID = 0;                     /* want first movie */
                Str255              movieName;
                Boolean             wasChanged;

                err = NewMovieFromFile (&aMovie, movieResFile, &movieResID, movieName,  newMovieActive,
                                        &wasChanged);
                CloseMovieFile (movieResFile);
            } else
            {
                WriteDebugLine((char *)"\pCouldn't Open Movie");
                WriteDebugLine( (char *)filePath);
            }
        } else WriteDebugLine( (char *)"\pCouldn't Find Movie");
        if ( aMovie != nil)
        {
/*          if ( HasDepth( device, 16, 1, 1))
            {
                depthSet = true;
                SetDepth( device, 16, 1, 1);
            }
*/
            GetMovieBox (aMovie, &movieBox);

    /*
            // make a new window centered in the device of choice
            windowRect = movieBox;
            OffsetRect (&windowRect,
                (((*device)->gdRect.right - (*device)->gdRect.left) / 2) -
                ((windowRect.right - windowRect.left) / 2),
                (((*device)->gdRect.bottom - (*device)->gdRect.top) / 2) -
                ((windowRect.bottom - windowRect.top) / 2));
            movieWindow = (CWindowPtr)NewCWindow (nil, &windowRect, "\p", false, plainDBox,
                            (WindowPtr)-1, false, 703);
            ShowWindow( (WindowPtr)movieWindow);
    */

            // if we have a custom color table for this movie
            MacFillRect( &(aWindow->portRect), &(qd.black));
            if ( GetMovieColorTable( aMovie, &theClut) != noErr)
            {
                MyDebugString("\pCan't GetMovieColorTable");
            }

            if ( theClut == nil)
            {
                WriteDebugLine((char *)"\pNo CLUT!");
            }

//          TRY TO SCREW AROUND WITH THE TABLE
            if ( ShiftKey())
            {
                for (count = 0; count <= (**theClut).ctSize; count++)
                {
                    if ((((**theClut).ctTable[count].rgb.red >> 12L) +
                        ((**theClut).ctTable[count].rgb.green >> 12L) +
                        ((**theClut).ctTable[count].rgb.blue >> 12L)) == 0)
                    {
        //              Debugger();
                        (**theClut).ctTable[count].rgb.red =
                            (**theClut).ctTable[count].rgb.green =
                            (**theClut).ctTable[count].rgb.blue = 0;//Randomize( 32768);
                    }
                }

                if ( SetMovieColorTable( aMovie, theClut) != noErr)
                {
                    MyDebugString("\pCan't SetMovieColorTable");
                }
            }

            thePalette = NewPalette( (**theClut).ctSize, theClut, pmExplicit + pmTolerant, 0);


            originalPalette = GetPalette( aWindow);
            if ( originalPalette != nil)
            {
                SetPalette( (WindowPtr)aWindow, thePalette, false);
                ActivatePalette( (WindowPtr)aWindow);
            }

            if ( (( movieBox.right - movieBox.left) <= (( aWindow->portRect.right -
                aWindow->portRect.left) / 2)) && (( movieBox.bottom - movieBox.top) <= (( aWindow->portRect.bottom -
                aWindow->portRect.top) / 2)) && ( doubleIt))
            {
                movieBox.right *= 2;
                movieBox.bottom *= 2;
            }
            MacOffsetRect (&movieBox,
                ((aWindow->portRect.right - aWindow->portRect.left) / 2) -
                ((movieBox.right - movieBox.left) / 2),
                ((aWindow->portRect.bottom - aWindow->portRect.top) / 2) -
                ((movieBox.bottom - movieBox.top) / 2));
            SetMovieBox( aMovie, &movieBox);
            SetMovieGWorld( aMovie, (CGrafPtr)aWindow, nil);
            SetMovieVolume( aMovie, kSoundVolumeMultiplier * gAresGlobal->gSoundVolume);

            HideCursor();
            movieRate = GetMovieRate( aMovie);
            movieTime = 0;//GetMovieDuration( aMovie);
            if ( PrerollMovie ( aMovie, movieTime, movieRate) != noErr) SysBeep( 20);
            StartMovie (aMovie);
            while ( !IsMovieDone(aMovie) && !done )
            {
                MoviesTask (aMovie, DoTheRightThing);
                if ( !ShiftKey())
                    done = AnyEvent();
            }
            MacShowCursor();
            DisposeMovie (aMovie);
            MacFillRect( &(aWindow->portRect), &(qd.black));

    //      DisposeWindow( (WindowPtr)movieWindow);
            if ( theClut != nil) DisposeCTable( theClut);
            if ( originalPalette != nil)
            {
                SetPalette( (WindowPtr)aWindow, originalPalette, false);
                ActivatePalette( (WindowPtr)aWindow);
            }
            if ( thePalette != nil) DisposePalette( thePalette);

/*          if ( depthSet)
            {
                SetDepth( device, 8, 1, 1);
            }
*/      }
    }
#endif
}

/* the "mini movie" routines are for playing a movie within a rect without disturbing the rest of the display.
The PlayMovieByName is for playing a movie in one step, automatically clearing the diplay and all.
*/

OSErr LoadMiniMovie( StringPtr filePath, Movie *aMovie, Rect *destRect, WindowPtr aWindow, Boolean doubleIt)

{
#ifdef kUseMovies
    OSErr                       err;
    FSSpec                      fileSpec;
    short                       movieResFile, offH, offV;
    Rect                        movieBox;
    Boolean                     done = false;
    Fixed                       movieRate;
    TimeValue                   movieTime;

    if ( gAresGlobal->gOptions & kOptionQuicktime)
    {

        err = FSMakeFSSpec( 0, 0L, filePath, &fileSpec);
        if ( err == noErr)
        {
            err = OpenMovieFile ( &fileSpec, &movieResFile, fsRdPerm);
            if (err == noErr)
            {
                short               movieResID = 0;                     /* want first movie */
                Str255              movieName;
                Boolean             wasChanged;

                err = NewMovieFromFile ( aMovie, movieResFile, &movieResID, movieName,  newMovieActive,
                                        &wasChanged);
                CloseMovieFile (movieResFile);
            } else
            {
                WriteDebugLine((char *)"\pCouldn't Open Movie");
                WriteDebugLine( (char *)filePath);
                return( err);
            }
        } else
        {
            WriteDebugLine( (char *)"\pCouldn't Find Movie");
            return( err);
        }
        if ( *aMovie != nil)
        {
            MacSetPort( aWindow);

            GetMovieBox (*aMovie, &movieBox);

            if ( (( movieBox.right - movieBox.left) <= (( destRect->right -
                destRect->left) / 2)) && (( movieBox.bottom - movieBox.top) <= (( destRect->bottom -
                destRect->top) / 2)) && ( doubleIt))
            {
                movieBox.right *= 2;
                movieBox.bottom *= 2;
            }
            offH = ((destRect->right - destRect->left) / 2) -
                ((movieBox.right - movieBox.left) / 2) + destRect->left;
            offV = ((destRect->bottom - destRect->top) / 2) -
                ((movieBox.bottom - movieBox.top) / 2) + destRect->top;

            MacOffsetRect (&movieBox, offH, offV);

            MacFillRect( &movieBox, &(qd.black));

            SetMovieGWorld (*aMovie, (CGrafPtr)aWindow, nil);
            SetMovieVolume( *aMovie, kSoundVolumeMultiplier * gAresGlobal->gSoundVolume);
            SetMovieBox (*aMovie, &movieBox);

            movieRate = GetMovieRate( *aMovie);
            movieTime = 0;//GetMovieDuration( *aMovie);

            err = PrerollMovie ( *aMovie, movieTime, movieRate);
            if ( err != noErr) return( err);
        } else
        {
            WriteDebugLine((char *)"\pMovie = nil");
            return ( -1);
        }
    } else
    {
        *aMovie = nil;
    }
#endif

    return( noErr);
}

OSErr StartMiniMovie( Movie aMovie)

{
#ifdef kUseMovies
    OSErr err;

    if ( aMovie != nil) StartMovie( aMovie);
    err = GetMoviesError();
//  if ( err != noErr) Debugger();

    return( err);
#else
    return( noErr);
#endif
}

// returns true if movie is done
Boolean DoMiniMovieTask( Movie aMovie)

{
#ifdef kUseMovies
    OSErr   err;
    Boolean done = false;

    if ( aMovie == nil) return( true);
    else
    {
        done = IsMovieDone(aMovie);
        err = GetMoviesError();
        if ( err != noErr) return true;//Debugger();
        if ( done) return( true);
        else
        {
            MoviesTask( aMovie, DoTheRightThing);
            err = GetMoviesError();
//          if ( err != noErr) Debugger();
            return( false);
        }
    }
#endif
    return( true);
}

OSErr CleanUpMiniMovie( Movie *aMovie)

{
#ifdef kUseMovies
    OSErr   err;
    Rect    movieBox;

    if ( *aMovie != nil)
    {
        GetMovieBox( *aMovie, &movieBox);
        err = GetMoviesError();
        if ( err != noErr) return err;//Debugger();
        MacFillRect( &movieBox, &(qd.black));
        StopMovie (*aMovie);
        err = GetMoviesError();
        if ( err != noErr) return err;//Debugger();
        DisposeMovie( *aMovie);
        err = GetMoviesError();
        if ( err != noErr) return err;//Debugger();
        *aMovie = nil;
    }
#endif
    return ( noErr);
}
