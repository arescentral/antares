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

/******************************************\
|**| Ares_ResFile.c
\******************************************/

#include "AresResFile.h"

#include "Assert.h"

#pragma mark **DEFINITIONS**
/******************************************\
|**| #defines
\******************************************/

/* - definitions
*******************************************/

#pragma mark _macros_
/* - macros
*******************************************/

#pragma mark **TYPEDEFS**
/******************************************\
|**| typedefs
\******************************************/

#pragma mark **EXTERNAL GLOBALS**
/******************************************\
|**| external globals
\******************************************/

#pragma mark **PRIVATE GLOBALS**
/******************************************\
|**| private globals
\******************************************/

#pragma mark **PRIVATE PROTOTYPES**
/******************************************\
|**| private function prototypes
\******************************************/

#pragma mark **PRIVATE FUNCTIONS**
/******************************************\
|**| private functions
\******************************************/

#pragma mark **PUBLIC FUNCTIONS**
/******************************************\
|**| public functions
\******************************************/

short ARF_OpenResFile( StringPtr fileName)
{
    ProcessSerialNumber             myProcess;
    OSErr                           error;
    ProcessInfoRec                  info;
    FSSpec                          appFileSpec, fileFileSpec;

//  mAssert( fileName != nil);
    error = GetCurrentProcess( &myProcess);
    if ( error != noErr) return -1;

    info.processName = nil;
    info.processAppSpec = &appFileSpec;
    info.processInfoLength = sizeof( ProcessInfoRec);
    error = GetProcessInformation( &myProcess, &info);
    if ( error != noErr) return -1;

    error = FSMakeFSSpec( appFileSpec.vRefNum, appFileSpec.parID, fileName,
        &fileFileSpec);

    return( FSpOpenResFile( &fileFileSpec, fsRdPerm));
}

/* ARF_OpenResFile_External
    makes the FSSpec of a file from a file name, relative to ares folder,
    so you can make it *the* external data file
*/

OSErr ARF_OpenResFile_External( StringPtr fileName, FSSpecPtr fileSpec)
{
    ProcessSerialNumber             myProcess;
    OSErr                           error;
    ProcessInfoRec                  info;
    FSSpec                          appFileSpec;

//  mAssert( fileName != nil);
    if ( fileSpec == nil) return paramErr;

    error = GetCurrentProcess( &myProcess);
    if ( error != noErr) return error;
    info.processName = nil;
    info.processAppSpec = &appFileSpec;
    info.processInfoLength = sizeof( ProcessInfoRec);
    error = GetProcessInformation( &myProcess, &info);
    if ( error != noErr) return error;

    error = FSMakeFSSpec( appFileSpec.vRefNum, appFileSpec.parID, fileName,
        fileSpec);
    if ( error != noErr) return error;

    return noErr;
}

