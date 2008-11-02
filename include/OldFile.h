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
|**| Old_File.h
\******************************************/

#ifndef kOld_File_h
#define kOld_File_h

#pragma mark ¥¥INCLUDES¥¥
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

#pragma mark ¥¥DEFINITIONS¥¥
/******************************************\
|**| #defines
\******************************************/

/* - definitions
*******************************************/

#pragma mark _macros_
/* - macros
*******************************************/

#pragma mark ¥¥TYPEDEFS¥¥
/******************************************\
|**| typedefs
\******************************************/

#pragma mark ¥¥PUBLIC PROTOTYPES¥¥
/******************************************\
|**| public function prototypes
\******************************************/

pascal Boolean  filterFunctionOpenDialog(CInfoPBPtr pbPtr,void *dataPtr);
pascal SInt16  hookFunctionOpenDialog(SInt16 item,DialogPtr theDialog,
	void *dataPtr);
OSErr  doDirectorySelectionDialog( StandardFileReply *);
pascal Boolean  filterFunctionDirSelect(CInfoPBPtr pbPtr,void *dataPtr);
pascal SInt16  hookFunctionDirSelect(SInt16 item,DialogPtr theDialog,
	void *dataPtr);


#endif kOld_File_h
