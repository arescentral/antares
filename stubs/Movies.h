#ifndef ANTARES_STUB_MOVIES_H_
#define ANTARES_STUB_MOVIES_H_

#include <Base.h>
#include <Quickdraw.h>

enum {
    DoTheRightThing = 9000,

    newMovieActive = 9100,
};

typedef void* Movie;

typedef double TimeValue;

STUB0(EnterMovies, OSErr(), noErr);
STUB0(ExitMovies, void());
STUB3(OpenMovieFile, OSErr(FSSpec* fsspec, short* res_file, int permissions), noErr);
STUB6(NewMovieFromFile, OSErr(Movie* movie, short res_file, short* id,
                       const unsigned char* name, int, bool* was_changed), noErr);
STUB1(CloseMovieFile, OSErr(short res_file), noErr);

STUB1(StartMovie, void(Movie movie));
STUB1(StopMovie, void(Movie movie));
STUB1(DisposeMovie, void(Movie movie));
STUB0(GetMoviesError, OSErr(), noErr);

STUB2(GetMovieBox, void(Movie movie, Rect* box));
STUB2(SetMovieBox, void(Movie movie, Rect* box));

STUB1(GetMovieColorTable, ColorTable*(Movie movie), NULL);
STUB2(SetMovieColorTable, void(Movie movie, const ColorTable& clut));

STUB3(SetMovieGWorld, void(Movie movie, CGrafPtr window, void*));
STUB2(SetMovieVolume, void(Movie movie, double volume));
STUB1(GetMovieRate, Fixed(Movie movie), 0);
STUB3(PrerollMovie, OSErr(Movie movie, TimeValue duration, Fixed rate), noErr);
STUB1(IsMovieDone, bool(Movie movie), true);
STUB2(MoviesTask, void(Movie movie, int do_the_right_thing));

#endif // ANTARES_STUB_MOVIES_H_
