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

#ifndef ANTARES_PROCESSOR_HPP_
#define ANTARES_PROCESSOR_HPP_

/* PROCESSOR.H */

//#define   POWER_PC            FALSE
#include "ConditionalMacros.h"

#if TARGET_OS_WIN32
#define kDontDoLong68KAssem
#endif // TARGET_OS_WIN32

#ifndef kMyProcessor
#define kMyProcessor

#ifdef powerc
    #define powercc
    #define kDontDoLong68KAssem
#else
    #ifndef __CFM68K__
    #define kUseCFMGlue
    #endif
//#define   kDontDoLong68KAssem
#endif

#ifdef __CFM68K__
#define kDontDoLong68KAssem
#endif

//#define   powercc powerc

//#define powerc TRUE

#endif

#define kAllowAssem

#endif // ANTARES_PROCESSOR_HPP_
