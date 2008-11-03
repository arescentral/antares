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
|**| Old_File.c
\******************************************/

#include "OldFile.h"

#include "str_bp.h"

#pragma mark **DEFINITIONS**
/******************************************\
|**| #defines
\******************************************/

/* - definitions
*******************************************/

#define mApple                                  128
#define  iAbout                                 1
#define mFile                                   129
#define  iNew                                   1
#define  iOpen                                  2
#define  iClose                                 4
#define  iSave                                  5
#define  iSaveAs                                6
#define  iRevert                                7
#define  iQuit                                  12
#define mDemonstration                          131
#define  iTouchWindow                           1
#define  iSelectDirectoryDialog                 3
#define rNewWindow                              128
#define rMenubar                                128
#define rRevertAlert                            128
#define rCloseFileAlert                         129
#define rCustomOpenDialog                       130
#define  iPopupItem                             10
#define rSelectDirectoryDialog  131
#define  iSelectButton                          10
#define rErrorStrings                           128
#define  eInstallHandler                        1000
#define  eMaxWindows                            1001
#define  eFileIsOpen                            opWrErr
#define kMaxWindows                             10
#define kUserCancelled                          1002
#define MAXLONG                                 0x7FFFFFFF
#define MIN(a,b)                                ((a) < (b) ? (a) : (b))

#pragma mark _macros_
/* - macros
*******************************************/

#define doCopyPString( m_sourceString, m_destString) pstrcpy( m_destString, m_sourceString)
#define doConcatPStrings(  m_destString, m_sourceString) pstrcat( m_destString, m_sourceString)

#pragma mark **TYPEDEFS**
/******************************************\
|**| typedefs
\******************************************/

typedef struct
{
    TEHandle        editStrucHdl;
    PicHandle       pictureHdl;
    SInt16          fileRefNum;
    FSSpec          fileFSSpec;
    Boolean         windowTouched;
}   docStructure, *docStructurePointer, **docStructureHandle;

typedef StandardFileReply *standardFileReplyPtr;

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

// **************************************************************************************
// FiltersAndHooks.c
// **************************************************************************************

// ***************************************************************************** includes

//#include "Files1.h"

// ********************************************************************* global variables

SInt16  gCurrentType = 1;
Str255  gPrevSelectedName;
Boolean gDirectorySelectionFlag;

SFTypeList  gFileTypes;

// ************************************************************* filterFunctionOpenDialog

pascal Boolean  filterFunctionOpenDialog(CInfoPBPtr pbPtr,void *dataPtr)
{
#pragma unused ( dataPtr)

    if(pbPtr->hFileInfo.ioFlFndrInfo.fdType == gFileTypes[gCurrentType - 1])
        return false;
    else
        return true;
}

// *************************************************************** hookFunctionOpenDialog

pascal SInt16  hookFunctionOpenDialog(SInt16 item,DialogPtr theDialog,void *dataPtr)
{
    SInt16  theType;
    Handle  controlHdl;
    Rect        theRect;

#pragma unused ( dataPtr)

    switch(item)
    {
        case sfHookFirstCall:
            GetDialogItem(theDialog,iPopupItem,&theType,&controlHdl,&theRect);
            SetControlValue((ControlHandle) controlHdl,gCurrentType);
            return sfHookNullEvent;
            break;

        case iPopupItem:
            GetDialogItem(theDialog,iPopupItem,&theType,&controlHdl,&theRect);
            theType = GetControlValue((ControlHandle) controlHdl);
            if(theType != gCurrentType)
            {
                gCurrentType = theType;
                return sfHookRebuildList;
            }
            break;
    }

    return item;
}

// *********************************************************** doDirectorySelectionDialog

OSErr  doDirectorySelectionDialog( StandardFileReply *stdFileReplyStruct)
{
    SFTypeList          fileTypes;
    Point               dialogLocation;
    FileFilterYDUPP     filterFunctionDirSelectUPP;
    DlgHookYDUPP        hookFunctionDirSelectUPP;
    OSErr               error = noErr;

    if ( stdFileReplyStruct == nil) return paramErr;
    stdFileReplyStruct->sfGood = false;

    filterFunctionDirSelectUPP = NewFileFilterYDProc((FileFilterYDProcPtr) filterFunctionDirSelect);
    hookFunctionDirSelectUPP = NewDlgHookYDProc((DlgHookYDProcPtr) hookFunctionDirSelect);

    gPrevSelectedName[0] = 0;
    gDirectorySelectionFlag = true;
    dialogLocation.v = -1;
    dialogLocation.h = -1;

    CustomGetFile(filterFunctionDirSelectUPP,-1,fileTypes, stdFileReplyStruct,
                                131,//rSelectDirectoryDialog,
                                dialogLocation,
                                hookFunctionDirSelectUPP,NULL,NULL,
                                NULL, stdFileReplyStruct);

    DisposeRoutineDescriptor(filterFunctionDirSelectUPP);
    DisposeRoutineDescriptor(hookFunctionDirSelectUPP);

    return error;
}

// ************************************************************** filterFunctionDirSelect

pascal Boolean  filterFunctionDirSelect(CInfoPBPtr pbPtr,void *dataPtr)
{
    SInt32  attributes;
    Boolean result;

#pragma unused ( dataPtr)

    attributes = (SInt32) pbPtr->hFileInfo.ioFlAttrib;
    result = !(BitTst(&attributes,31 - 4));
    return result;
}

// **************************************************************** hookFunctionDirSelect

pascal SInt16  hookFunctionDirSelect(SInt16 item,DialogPtr theDialog,
    void *dataPtr)
{
    SInt16                              itemType, width;
    Handle                              itemHdl;
    Rect                                    itemRect;
    Str255                              theName, theString =  "\pSelect  '";
    standardFileReplyPtr    stdFileReplyPtr;

    stdFileReplyPtr = (standardFileReplyPtr) dataPtr;

    if(stdFileReplyPtr->sfIsFolder || stdFileReplyPtr->sfIsVolume)
    {
        doCopyPString(stdFileReplyPtr->sfFile.name,theName);

        if(IdenticalString(theName,gPrevSelectedName,NULL) != 0)
        {
            doCopyPString(theName,gPrevSelectedName);

            GetDialogItem(theDialog,iSelectButton,&itemType,&itemHdl,&itemRect);
            width = (itemRect.right - itemRect.left) - StringWidth("\pSelect  '    ");
            TruncString(width,theName,smTruncMiddle);
            doConcatPStrings(theString,theName);
            doConcatPStrings(theString,"\p'");

            SetControlTitle((ControlHandle) itemHdl,theString);
        }
    }

    if(item == iSelectButton)
        return sfItemCancelButton;
    else if(item == sfItemCancelButton)
        gDirectorySelectionFlag = false;

    return item;
}

// **************************************************************************************
