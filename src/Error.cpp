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

// Error Handling

#include "Error.hpp"

#include "Quickdraw.h"

#include "StringHandling.hpp"
#include "StringNumerics.hpp"

namespace antares {

#define ERROR_STR_ID    800
#define ERROR_ALERT_ID  800

//#define   kDebugError // comment out to not include internal code

#ifdef kDebugError
#define kAnyAlertID         806
#define kAnyErrorDialogID   811
#else
#define kAnyAlertID         805
#define kAnyErrorDialogID   810
#endif

#define kContinueButton     1
#define kQuitButton         2

namespace {

class Failure : public std::exception {
  public:
    // Takes ownership of `what`.
    Failure(char* what) throw() : _what(what) { }
    virtual ~Failure() throw() { free(_what); }
    virtual const char* what() const throw() { return _what; }
  private:
    char* _what;
};

}  // namespace

void check(bool condition, const char* fmt, ...) {
    if (!condition) {
        char* error = NULL;
        va_list args;
        va_start(args, fmt);
        if (vasprintf(&error, fmt, args) < 0) {
            perror("vasprintf");
            abort();
        }
        va_end(args);
        throw Failure(error);
    }
}

void fail(const char* fmt, ...) {
    char* error = NULL;
    va_list args;
    va_start(args, fmt);
    if (vasprintf(&error, fmt, args) < 0) {
        perror("vasprintf");
        abort();
    }
    va_end(args);
    throw Failure(error);
}

}  // namespace antares
