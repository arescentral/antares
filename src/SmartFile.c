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
|**| Smart_File.c
\******************************************/

#pragma mark **INCLUDES**
/******************************************\
|**| #includes
\******************************************/

#pragma mark _system includes_
/* - system
*******************************************/

#include <Navigation.h>

#pragma mark _third party includes_
/* - third party libraries
*******************************************/

#pragma mark _bp libraries includes_
/* - bp libraries
*******************************************/

#pragma mark _this library includes_
/* - this project
*******************************************/

#include "Old_File.h"
#include "Navigation_Utilities.h"

#include "Smart_File.h"

#pragma mark **DEFINITIONS**
/******************************************\
|**| #defines
\******************************************/

/* - definitions
*******************************************/

#pragma mark _macros_
/* - macros
*******************************************/

#ifdef powerc	
#define Has_Navigation_Services		NavServicesAvailable()
#else
#define	Has_Navigation_Services		false
#endif

#pragma mark **TYPEDEFS**
/******************************************\
|**| typedefs
\******************************************/

#pragma mark **EXTERNAL GLOBALS**
/******************************************\
|**| external globals
\******************************************/

extern Boolean gDirectorySelectionFlag;

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

OSErr SmartFile_SelectFolder( FSSpecPtr destFile, StringPtr windowName,
	StringPtr prompt)
{
	OSErr	error = noErr;
	
	if ( destFile == nil) return paramErr;
	if ( !Has_Navigation_Services) 	// this is a macro that's safe to call any
									// time
	{
		StandardFileReply	fileReply;
		
		error = doDirectorySelectionDialog( &fileReply);
		if (( fileReply.sfGood) || ( gDirectorySelectionFlag))
		{
			BlockMove( &fileReply.sfFile, destFile, sizeof( FSSpec));
			return noErr;
		} else return userCanceledErr;
	} else
	{
#ifdef powerc	
		return NS_SelectFolderObject( destFile, windowName, prompt);
#endif
	}
	return noErr;	
}

OSErr SmartFile_SaveAs( FSSpecPtr destFile, StringPtr fileName,
	StringPtr appName, OSType fileTypeToSave, OSType fileCreator)
{
	OSErr				error = noErr;
	
	if ( destFile == nil) return paramErr;
	
	if ( !Has_Navigation_Services) 	// this is a macro that's safe to call any
									// time
	{
		StandardFileReply	fileReply;
		unsigned char		prompt[] = "\pSave as:";

		StandardPutFile( prompt, fileName, &fileReply);
		if ( fileReply.sfGood)
		{
			BlockMove( &fileReply.sfFile, destFile, sizeof( FSSpec));
			if ( !fileReply.sfReplacing)
			{
				FSpCreateResFile( destFile, fileCreator, fileTypeToSave,
					fileReply.sfScript);
				error = ResError();
				if ( error != noErr) return error;
			}
		} else return userCanceledErr;
	} else
	{
#ifdef powerc	
		return NS_SaveAs( destFile, fileName, appName, fileTypeToSave,
							fileCreator);
#endif
	}
	
	return noErr;
}

OSErr SmartFile_SelectFile( FSSpecPtr destFile, short openListResID)
{
	OSErr				error = noErr;
	
	if ( !Has_Navigation_Services)
	{
		Handle				openFileTypeResource =
								GetResource('open', openListResID);
		SFTypeList			typeList;
		OSType				*osType = nil;
		long				typeNum, i;
		StandardFileReply	fileReply;
		
		if ( openFileTypeResource == nil) return resNotFound;
		
		typeNum = *((long *)((*openFileTypeResource) + 4));
		if ( typeNum > 4) typeNum = 4;
		for ( i = 0; i < typeNum; i++)
		{
			osType = (OSType *)((*openFileTypeResource) + 8 + ( 4 * i));
			typeList[i] = *osType;
		}
		
		ReleaseResource( openFileTypeResource);
		StandardGetFile( 0, typeNum, typeList, &fileReply);
		if ( fileReply.sfGood)
		{
			BlockMove( &fileReply.sfFile, destFile, sizeof( FSSpec));
			return noErr;
		} else return userCanceledErr;
	} else
	{
#ifdef powerc	
		return NS_SelectFileObject( destFile, openListResID);
#endif
	}
	
	return noErr;
}
