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

#ifndef ANTARES_HANDLE_HANDLING_HPP_
#define ANTARES_HANDLE_HANDLING_HPP_

// Handle Handling.h

#include <Base.h>
#include "Handle.hpp"

#pragma options align=mac68k

typedef void (UnlockFunction)(Handle);
typedef void (LockFunction)(Handle);
typedef void (ResolveFunction)(Handle);

short HandleHandlerInit( void);
void HandleHandlerCleanup( void);
void ResetAllHandleData( void);
short HHRegisterHandle( Handle *newHandle,
            void            (*unlockData)( Handle),
            void            (*lockData)( Handle),
            void            (*resolveData)( Handle),
            Boolean, const unsigned char*);
void HHDeregisterHandle( Handle *);
void HHMaxMem( void);
Handle HHNewHandle( long);
Handle HHGetResource( ResType, short);
void HHConcatenateHandle( Handle, Handle);
void HHClearHandle( Handle);
void HHCheckHandle( Handle, Handle);
void HHCheckAllHandles( void);

inline void mHandleLockAndRegister(
        Handle& mhandle, UnlockFunction munlockProc, LockFunction mlockPrc,
        ResolveFunction mresolveProc, const unsigned char* mhandlename) {
    MoveHHi( (mhandle));
    HLock( (mhandle));
    HHClearHandle( mhandle);
    HHRegisterHandle( &(mhandle), munlockProc, mlockPrc, mresolveProc, false, mhandlename);
}

// TypedHandle<>s can no longer be registered using mHandleLockAndRegister, but simply deleting the
// call thereto would prevent HHClearHandle() from being called.  HHClearHandle() makes a call to
// Random() for each byte contained in the handle, so this function ensures that the stream of
// random values is unaltered by the removal of the call to HHClearHandle().
template <typename T>
inline void TypedHandleClearHack(TypedHandle<T> handle) {
    for (size_t i = 0; i < handle.size(); ++i) {
        Random();
    }
}

inline void mDataHandleLockAndRegister(
        Handle& mhandle, UnlockFunction munlockProc, LockFunction mlockPrc,
        ResolveFunction mresolveProc, const unsigned char* mhandlename) {
    MoveHHi( (mhandle));
    HLock( (mhandle));
    HHRegisterHandle( &(mhandle), munlockProc, mlockPrc, mresolveProc, false, mhandlename);
}

inline void mCheckDataHandleLockAndRegister(
        Handle& mhandle, UnlockFunction munlockProc, LockFunction mlockPrc,
        ResolveFunction mresolveProc, const unsigned char* mhandlename) {
    MoveHHi( (mhandle));
    HLock( (mhandle));
    HHRegisterHandle( &(mhandle), munlockProc, mlockPrc, mresolveProc, true, mhandlename);
}

inline void mHandleDisposeAndDeregister(Handle mhandle) {
    HHDeregisterHandle( &(mhandle));
    DisposeHandle( (mhandle));
}

#pragma options align=reset

#endif // ANTARES_HANDLE_HANDLING_HPP_
