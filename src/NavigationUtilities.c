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
|**| Navigation_Utilities.c
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
//#include "Window Dispatch.h"
#include "str_bp.h"

#pragma mark _this library includes_
/* - this project
*******************************************/
#include "Ares Global Type.h"
#include "Offscreen GWorld.h"

#include "Navigation_Utilities.h"

#pragma mark **DEFINITIONS**
/******************************************\
|**| #defines
\******************************************/

/* - definitions
*******************************************/
#define kMaxDocumentCount       100     // maximum number of documents allowed
#define kSelectObjectPrefKey    7

#pragma mark _macros_
/* - macros
*******************************************/

#pragma mark **TYPEDEFS**
/******************************************\
|**| typedefs
\******************************************/

/*typedef struct Document
{
    WindowPtr       theWindow;
    TEHandle        theTE;
    short           docTop;
    RgnHandle       hiliteRgn;
    
    ControlHandle   vScroll;
    ControlHandle   hScroll;
    short           vScrollPos;
    
    short           fRefNum;
    short           dirty;
    
    Handle          undoDragText;
    short           undoSelStart;
    short           undoSelEnd;
    
    short           lastSelStart;
    short           lastSelEnd;

    ParamBlockRec   fioParamBlock;          // param block for file I/O operations
    Handle          fPict;
    Handle          fHeader;
    long            fPictLength;

} Document;
*/
#pragma mark **EXTERNAL GLOBALS**
/******************************************\
|**| external globals
\******************************************/

extern CWindowPtr       gTheWindow;
extern aresGlobalType   *gAresGlobal;

#pragma mark **PRIVATE GLOBALS**
/******************************************\
|**| private globals
\******************************************/
//Document* gDocumentList[kMaxDocumentCount];

#pragma mark **PRIVATE PROTOTYPES**
/******************************************\
|**| private function prototypes
\******************************************/
pascal void myEventProc(const NavEventCallbackMessage callBackSelector, 
                        NavCBRecPtr callBackParms, 
                        NavCallBackUserData callBackUD);


OSErr AEGetDescData(const AEDesc *desc, DescType *typeCode, void *dataBuffer, ByteCount maximumSize, ByteCount *actualSize);
#pragma mark **PRIVATE FUNCTIONS**
/******************************************\
|**| private functions
\******************************************/

#pragma mark **PUBLIC FUNCTIONS**
/******************************************\
|**| public functions
\******************************************/

OSErr NS_SelectFileObject( FSSpecPtr destFile, short openListResID)
{   
    NavReplyRecord      theReply;
    NavDialogOptions    dialogOptions;
    OSErr               theErr = noErr;
    NavEventUPP         eventUPP = NewNavEventProc(myEventProc);
    NavTypeListHandle   openList = nil;
    
    theErr = NavGetDefaultDialogOptions(&dialogOptions);
    
//  GetIndString(dialogOptions.message,rAppStringsID,sChooseObject);
    
    if ( openListResID > 0)
    {
        openList = (NavTypeListHandle)GetResource( 'open', openListResID);
    }
    dialogOptions.preferenceKey = kSelectObjectPrefKey;
    dialogOptions.dialogOptionFlags |= kNavSelectAllReadableItem;//kNavNoTypePopup;
    theErr = //NavChooseObject
            NavGetFile
                        (   NULL,
                                &theReply,
                                &dialogOptions,
                                eventUPP,
                                nil,
                                nil,
                                (NavTypeListHandle)openList,
                                (NavCallBackUserData)nil/*&gDocumentList*/);
    
    DisposeRoutineDescriptor(eventUPP);

    if ((theReply.validRecord)&&(theErr == noErr))
        {
        // grab the target FSSpec from the AEDesc:  
//      FSSpec      finalFSSpec;    
        AEDesc      resultDesc;
//      AliasHandle alias;
        
        if ((theErr = AECoerceDesc(&(theReply.selection),typeFSS,&resultDesc)) ==
            noErr)
            if ((theErr = AEGetDescData ( &resultDesc, NULL, destFile,
                    sizeof ( FSSpec ), NULL )) == noErr)
                {
                // 'finalFSSpec' is the selected directory...
                
//              theErr = NewAlias( &finalFSSpec, destFile, &alias);
//              if ( theErr != noErr) return theErr;
//              DisposeHandle( (Handle)alias);
                }
        AEDisposeDesc(&resultDesc);
        
        theErr = NavDisposeReply(&theReply);
        }
    
    if ( openList != nil) ReleaseResource( (Handle)openList);   
    return theErr;
}

OSErr NS_SelectFolderObject( FSSpecPtr destFile, StringPtr windowName,
    StringPtr prompt)
{   
    NavReplyRecord      theReply;
    NavDialogOptions    dialogOptions;
    OSErr               theErr = noErr;
    NavEventUPP         eventUPP = NewNavEventProc(myEventProc);
    
    theErr = NavGetDefaultDialogOptions(&dialogOptions);
    
//  GetIndString(dialogOptions.message,rAppStringsID,sChooseObject);
    
    if ( windowName != nil)
        pstrcpy( dialogOptions.windowTitle, windowName);
    
    if ( prompt != nil)
        pstrcpy( dialogOptions.message, prompt);
        
    dialogOptions.preferenceKey = kSelectObjectPrefKey;
    
    theErr = NavChooseFolder(   NULL,
                                &theReply,
                                &dialogOptions,
                                eventUPP,
                                NULL,
                                (NavCallBackUserData)nil/*&gDocumentList*/);
    
    DisposeRoutineDescriptor(eventUPP);

    if ((theReply.validRecord)&&(theErr == noErr))
        {
        // grab the target FSSpec from the AEDesc:  
//      FSSpec      finalFSSpec;    
        AEDesc      resultDesc;
//      AliasHandle alias;
        
        if ((theErr = AECoerceDesc(&(theReply.selection),typeFSS,&resultDesc)) ==
            noErr)
            if ((theErr = AEGetDescData ( &resultDesc, NULL, destFile,
                    sizeof ( FSSpec ), NULL )) == noErr)
                {
                // 'finalFSSpec' is the selected directory...
                
//              theErr = NewAlias( &finalFSSpec, destFile, &alias);
//              if ( theErr != noErr) return theErr;
//              DisposeHandle( (Handle)alias);
                }
        AEDisposeDesc(&resultDesc);
        
        theErr = NavDisposeReply(&theReply);
        }
        
    return theErr;
}

OSErr NS_SaveAs( FSSpecPtr destFile, StringPtr fileName, StringPtr appName,
    OSType fileTypeToSave, OSType fileCreator)
{
    OSErr               theErr = noErr;
    short               result = true;
    NavReplyRecord      theReply;
    NavDialogOptions    dialogOptions;
    NavEventUPP         eventUPP = NewNavEventProc(myEventProc);
//  OSType              fileTypeToSave = 'rsrc';

    // default behavior for browser and dialog:
    NavGetDefaultDialogOptions(&dialogOptions);

    // user might want to translate the saveed doc into another format
//  dialogOptions.dialogOptionFlags -= kNavDontAddTranslateItems;

//  GetWTitle(theDocument->theWindow,dialogOptions.savedFileName);
//  GetIndString((unsigned char*)&dialogOptions.clientName,rAppStringsID,sApplicationName);

    pstrcpy( dialogOptions.savedFileName, fileName);
    pstrcpy( dialogOptions.clientName, appName);
    
//  if (theDocument->theTE != NULL) // which document type is it?
//      fileTypeToSave = kFileType;
//  else
//      fileTypeToSave = kFileTypePICT;

    dialogOptions.preferenceKey = 2; // save button

    theErr = NavPutFile(NULL,   // use system's default location
                        &theReply,
                        &dialogOptions,
                        eventUPP,
                        fileTypeToSave,
                        fileCreator,
                        (NavCallBackUserData)nil/*&gDocumentList*/);
    DisposeRoutineDescriptor(eventUPP);

    if (theReply.validRecord && theErr == noErr)
        {
//      FSSpec  finalFSSpec;    
        AEDesc  resultDesc; 
        resultDesc.dataHandle = 0L;
        
        // retrieve the returned selection:
        if ((theErr = AEGetNthDesc(&(theReply.selection),1,typeFSS,NULL,&resultDesc)) == noErr)
            {
            BlockMoveData(*resultDesc.dataHandle,destFile,sizeof(FSSpec));

            if (!theReply.replacing)
                {
//              result = FSpCreate(destFile,fileCreator,fileTypeToSave,
//                      theReply.keyScript);
                FSpCreateResFile( destFile, fileCreator, fileTypeToSave,
                    theReply.keyScript);
                result = ResError();
                
                if (result != noErr)
                    {
                    SysBeep(5);
                    return result;
                    }
                }
                
//          if (theDocument->fRefNum)
//              result = FSClose(theDocument->fRefNum);
            
//          result = FSpOpenDF(destFile,fsRdWrPerm,&theDocument->fRefNum);
//          if (result)
//              {
//              SysBeep(5);
//              return result;
//              }

//          if (result = WriteFile(theDocument))
//              return result;

            AEDisposeDesc(&resultDesc);

            theErr = NavCompleteSave(&theReply,kNavTranslateInPlace);

//          SetWTitle(theDocument->theWindow,(unsigned char*)finalFSSpec.name);
//          theDocument->dirty = false;
            }

        NavDisposeReply(&theReply);
        }
    else
        result = theErr;

    return result;
}


// *****************************************************************************
// *
// *    myEventProc()   
// *
// *****************************************************************************
pascal void myEventProc(const NavEventCallbackMessage   callBackSelector, 
                        NavCBRecPtr                     callBackParms, 
                        NavCallBackUserData             callBackUD)
{
#pragma unused (callBackUD)

    CWindowPtr  whichWindow = NULL;
    short       index = 0;
    NavEventData    neData = callBackParms->eventData;
    
//  if (callBackUD != 0)
    switch (callBackSelector)
    {
        case kNavCBEvent:
        {
//              docList = (Document**)callBackUD;
//              if (docList != NULL)
            switch (callBackParms->eventData.eventDataParms.event->what)
            {
                case nullEvent:
                    break;
                    
                case updateEvt:
                    whichWindow = (CWindowPtr)callBackParms->eventData.eventDataParms.event->message;
                    if ( whichWindow == gTheWindow)
                    {
                        BeginUpdate( (WindowPtr)whichWindow);
                            MacSetPort( (WindowPtr)gTheWindow);
                            CopyOffWorldToRealWorld((WindowPtr)gTheWindow, &(gTheWindow->portRect));
                        EndUpdate( (WindowPtr)whichWindow);
                        break;
                        EndUpdate( (WindowPtr)whichWindow);
                    } else if ( whichWindow == gAresGlobal->gBackWindow)
                    {
                        BeginUpdate( (WindowPtr)whichWindow);
                            MacSetPort( (WindowPtr)gAresGlobal->gBackWindow);
                            MacFillRect(  &(gAresGlobal->gBackWindow->portRect), (Pattern *)&qd.black);
                        EndUpdate( (WindowPtr)whichWindow);
                    } else
                    {
                        BeginUpdate( (WindowPtr)whichWindow);
                        EndUpdate( (WindowPtr)whichWindow);
                    }
                    MacSetPort( (WindowPtr)gTheWindow);
                    break;

                case activateEvt:
                    break;

                default:
                    break;
            }
                break;
                }
            }
}

OSErr AEGetDescData(const AEDesc *desc, DescType *typeCode, void *dataBuffer, ByteCount maximumSize, ByteCount *actualSize)
{
    *typeCode = desc->descriptorType;
    Handle h = (Handle)desc->dataHandle;
    ByteCount dataSize = GetHandleSize(h);
    if (dataSize > maximumSize)
        *actualSize = maximumSize;
    else
        *actualSize = dataSize;
    BlockMoveData(*h, dataBuffer, *actualSize);
    return noErr;
}

