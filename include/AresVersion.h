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

/******************************************\
|**| Ares_Version.h
\******************************************/

#ifndef kAres_Version_h
#define kAres_Version_h

#pragma mark **INCLUDES**
/******************************************\
|**| #includes
\******************************************/

#pragma mark _system includes_
/* - system
*******************************************/

#pragma mark _third party includes_
/* - third party libraries
*******************************************/

#pragma mark _bp libraries includes_
/* - bp libraries
*******************************************/

#pragma mark _this library includes_
/* - this project
*******************************************/

#pragma mark **DEFINITIONS**
/******************************************\
|**| #defines
\******************************************/

/* - definitions
*******************************************/

#define	kThis_Version_Is					0x01010100

#pragma mark _macros_
/* - macros
*******************************************/

#pragma mark **TYPEDEFS**
/******************************************\
|**| typedefs
\******************************************/

typedef unsigned long aresVersionType;

#pragma mark **PUBLIC PROTOTYPES**
/******************************************\
|**| public function prototypes
\******************************************/

aresVersionType AresVersion_Get_FromString( StringPtr s);

StringPtr String_Get_FromAresVersion( StringPtr s, aresVersionType t);

#endif kAres_Version_h
