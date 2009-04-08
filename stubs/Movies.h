#ifndef ANTARES_STUB_MOVIES_H_
#define ANTARES_STUB_MOVIES_H_

#include <Base.h>
#include <Quickdraw.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

enum {
    DoTheRightThing = 9000,

    newMovieActive = 9100,
};

typedef void* Movie;

typedef double TimeValue;

OSErr EnterMovies();
void ExitMovies();
OSErr OpenMovieFile(FSSpec* fsspec, short* res_file, int permissions);
OSErr NewMovieFromFile(Movie* movie, short res_file, short* id,
                       const unsigned char* name, int, bool* was_changed);
OSErr CloseMovieFile(short res_file);

void StartMovie(Movie movie);
void StopMovie(Movie movie);
void DisposeMovie(Movie movie);
OSErr GetMoviesError();

void GetMovieBox(Movie movie, Rect* box);
void SetMovieBox(Movie movie, Rect* box);

OSErr GetMovieColorTable(Movie movie, CTabHandle* clut);
OSErr SetMovieColorTable(Movie movie, CTabHandle clut);

void SetMovieGWorld(Movie movie, CGrafPtr window, void*);
void SetMovieVolume(Movie movie, double volume);
Fixed GetMovieRate(Movie movie);
OSErr PrerollMovie(Movie movie, TimeValue duration, Fixed rate);
bool IsMovieDone(Movie movie);
void MoviesTask(Movie movie, int do_the_right_thing);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ANTARES_STUB_MOVIES_H_
