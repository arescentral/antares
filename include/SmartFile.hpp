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

#ifndef ANTARES_SMART_FILE_HPP_
#define ANTARES_SMART_FILE_HPP_

/******************************************\
|**| Smart_File.h
\******************************************/

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

#pragma mark _macros_
/* - macros
*******************************************/

#pragma mark **TYPEDEFS**
/******************************************\
|**| typedefs
\******************************************/

#pragma mark **PUBLIC PROTOTYPES**
/******************************************\
|**| public function prototypes
\******************************************/

OSErr SmartFile_SelectFolder( FSSpecPtr destFile, StringPtr windowName,
    StringPtr prompt);

OSErr SmartFile_SaveAs( FSSpecPtr destFile, StringPtr fileName,
    StringPtr appName, OSType fileTypeToSave, OSType fileCreator);

OSErr SmartFile_SelectFile( FSSpecPtr destFile, short openListResID);

#endif // ANTARES_SMART_FILE_HPP_
