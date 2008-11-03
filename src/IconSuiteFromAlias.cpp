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
|**| IconSuiteFromAlias.c
\******************************************/

#pragma mark **INCLUDES**
/******************************************\
|**| #includes
\******************************************/

#pragma mark _system includes_
/* - system
*******************************************/

#include <AEPackObject.h>

#pragma mark _third party includes_
/* - third party libraries
*******************************************/

#pragma mark _bp libraries includes_
/* - bp libraries
*******************************************/
//#include "BP_Error.h"

#pragma mark _this library includes_
/* - this project
*******************************************/

#include "IconSuiteFromAlias.h"

#pragma mark **DEFINITIONS**
/******************************************\
|**| #defines
\******************************************/

/* - definitions
*******************************************/

#pragma mark _macros_
/* - macros
*******************************************/
#define require(x,y,merrtitle,merrdesc) do { if (!(x))\
    {\
/*  BP_UserError_Literal(merrtitle, merrdesc); */ \
    goto y;\
    }\
 } while (0)
#define require_num(x,y,merrtitle,merrdesc,merrnum) do\
{ if (!(x))\
    {\
/*  BP_UserError_Literal(merrtitle, merrdesc, merrnum); */\
    goto y;\
    }\
 } while (0)

#define require_plain( x, y) do { if (!(x)) { goto y; } } while (0)

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
OSErr GetIconSuiteFromFSSpec(FSSpecPtr hfsObj, Handle *iconSuite);
Boolean HaveScriptableFinder( void);
OSErr SendAppleEvent(AppleEvent *ae, AppleEvent *reply, AESendMode sendMode);
OSErr MakeAppleEvent(AEEventClass aeClass, AEEventID aeID,
            AEDesc *target, AppleEvent *ae);
OSErr MakeSpecifierForFile(FSSpecPtr hfsObj, AEDesc *fileSpecifier);
OSErr MakePropertySpecifierForSpecifier(DescType property,
            AEDesc *ofSpecifier, AEDesc *propertySpecifier);
OSErr BuildIconSuiteFromAEDesc(Boolean largeIcons, Handle *iconSuite, AEDesc *iconFam);
Boolean FinderIsRunning( void);
static Size GetSizeFromIconType(DescType iconType);

pascal Boolean IdleHandle( EventRecord *theEvent, long *sleepTime, RgnHandle *mouseRgn);

#pragma mark **PRIVATE FUNCTIONS**
/******************************************\
|**| private functions
\******************************************/


pascal Boolean IdleHandle( EventRecord *theEvent, long *sleepTime, RgnHandle *mouseRgn)

{
#pragma unused (mouseRgn)

    CWindowPtr  whichWindow;
    long        menuChoice;

    if ( *sleepTime != 5) *sleepTime = 5;

    switch ( theEvent->what )
    {
        case osEvt:
            menuChoice = theEvent->message;
            menuChoice >>= 24L;
            menuChoice &= 0xff;

            switch (menuChoice)
            {
                case mouseMovedMessage:
//                          DoIdle(event); {mouse-moved same as idle for this app}
                    break;

                case suspendResumeMessage:
//                  DoSuspendResumeEvent( theEvent);// handle supend/resume event}
                    break;
            }
            break;

        case activateEvt:
//          if ( gTimerGlobal->window != nil)
//          {
//              whichWindow = (CWindowPtr)theEvent->message;
//              if ( whichWindow == gTimerGlobal->window)
//                  ActivateEvent((theEvent->modifiers & activeFlag)?(true):(false));
//          }
                whichWindow = (CWindowPtr)theEvent->message;
//              ActivateEvent((theEvent->modifiers & activeFlag)?(true):(false));
            break;

        case nullEvent:
            break;

        case updateEvt:
//          if ( gTimerGlobal->window != nil)
            {
                whichWindow = (CWindowPtr)theEvent->message;
//              HandleUpdateEvent( whichWindow);
        BeginUpdate( (WindowPtr)whichWindow);
        EndUpdate( (WindowPtr)whichWindow);
            }
            break;


        case kHighLevelEvent:
//          HandleHighLevelEvent( theEvent);
            break;

    }
    return( false);
}

//----------------------------------------------------------------------------
// GetIconSuiteFromFSSpec
//
// Send a GetData AE for the 'ifam'
//----------------------------------------------------------------------------
OSErr GetIconSuiteFromFSSpec(FSSpecPtr hfsObj, Handle *iconSuite)
{
    OSErr       err;
    AppleEvent  finderEvent, replyEvent;
    AEDesc      fileSpecifier, iconPropertySpecifier;
    DescType    returnType;
    Size        returnSize;
    long        returnLong;
    AEDesc      iconFamily;
    AEDesc      pFinderTarget;

    //
    // Set up our locals for easy cleanup
    //
    *iconSuite = NULL;

    pFinderTarget.dataHandle = nil;
    pFinderTarget.descriptorType = 0;
    //
    // Make sure the Finder is scriptable and is running.
    //
    err = paramErr;
    require(HaveScriptableFinder() == true, HaveScriptableFinder,
        "An error occured while getting a file's icon",
        "Could not confirm that the scriptable Finder was running.");

    //
    // Make a GetData Apple event to send to the Finder
    //
    err = MakeAppleEvent(kAECoreSuite, kAEGetData, &pFinderTarget,
                &finderEvent);
    require_num(err == noErr, MakeAppleEvent,
        "An error occured while getting a file's icon",
        "Couldn't make an AppleEvent because an error of type %d occured",
        err);

    //
    // Make an object specifier for the interesting file
    //
    err = MakeSpecifierForFile(hfsObj, &fileSpecifier);
    require_num(err == noErr, MakeSpecifierForFile,
        "An error occured while getting a file's icon",
        "Couldn't MakeSpecifierForFile because an error of type %d occured",
        err);

    //
    // Make an icon family property specifier for the file
    //
    err = MakePropertySpecifierForSpecifier(pIconBitmap, &fileSpecifier,
                &iconPropertySpecifier);
    require_num(err == noErr, MakePropertySpecifierForSpecifier,
        "An error occured while getting a file's icon",
        "Couldn't MakePropertySpecifierForSpecifier because an error of type %d occured",
        err);

    //
    // Stuff it in the Apple event and send it
    //
    err = AEPutParamDesc(&finderEvent, keyDirectObject, &iconPropertySpecifier);
    require_num(err == noErr, AEPutParamDesc,
        "An error occured while getting a file's icon",
        "Couldn't AEPutParamDesc because an error of type %d occured",
        err);

    err = SendAppleEvent(&finderEvent, &replyEvent,
                kAEWaitReply + kAENeverInteract /* + kAECanInteract + kAECanSwitchLayer*/);
    require_num(err == noErr, SendAppleEvent,
        "An error occured while getting a file's icon",
        "Couldn't send an AppleEvent because an error of type %d occured",
        err);

    //
    // Now the Finder may have sent us an error number
    //
    err = AEGetParamPtr(&replyEvent, keyErrorNumber, typeLongInteger,
                        &returnType, &returnLong, sizeof(long), &returnSize);
    if (err == noErr)
        err = (OSErr) returnLong;

    else {
        //
        // If not, get the icon family and build an icon suite
        //
        err = AEGetParamDesc(&replyEvent, keyDirectObject, typeWildCard, &iconFamily);
        require_num(err == noErr, AEGetParamDesc,
        "An error occured while getting a file's icon",
        "Couldn't AEGetParamDesc because an error of type %d occured",
        err);

        err = BuildIconSuiteFromAEDesc(false, iconSuite, &iconFamily);
        if ( err != noErr)
        {
//          BP_UserError_Literal("An error occured while getting a file's icon",
//          "Couldn't build the icon suite because an error of type %d occurred.",
//          err);
        }
    }

    //
    // Clean up and exit
    //
    (void) AEDisposeDesc(&iconFamily);

AEGetParamDesc:
    (void) AEDisposeDesc(&replyEvent);

SendAppleEvent:

AEPutParamDesc:
    (void) AEDisposeDesc(&iconPropertySpecifier);

MakePropertySpecifierForSpecifier:
    (void) AEDisposeDesc(&fileSpecifier);

MakeSpecifierForFile:
    (void) AEDisposeDesc(&finderEvent);

MakeAppleEvent:

HaveScriptableFinder:
    return err;
}

//----------------------------------------------------------------------------
// HaveScriptableFinder
//
// We have it if the Gestalt bit is set and the Finder is running
//----------------------------------------------------------------------------
Boolean HaveScriptableFinder( void)
{
    long        response;
    Boolean     haveScriptableFinder;
    OSErr       err;

    haveScriptableFinder = false;

    err = Gestalt(gestaltFinderAttr, &response);
    require_num(err == noErr, Gestalt,
        "An error occured while checking for the Scriptable Finder",
        "An error of type %d occured while checking the gestalt.",
        err);

    if ((response & (1 << gestaltOSLCompliantFinder)) && (FinderIsRunning()))
        haveScriptableFinder = true;

Gestalt:
    return haveScriptableFinder;
}

//----------------------------------------------------------------------------
// SendAppleEvent
//----------------------------------------------------------------------------
OSErr SendAppleEvent(AppleEvent *ae, AppleEvent *reply, AESendMode sendMode)
{
    AppleEvent  throwAwayReply;
    OSErr       err;
    AEIdleUPP           idleFunction = nil;

    idleFunction = NewAEIdleProc( (AEIdleProcPtr)IdleHandle);


    if (reply == NULL) {
        err = AESend(ae, &throwAwayReply, sendMode,
                    kAENormalPriority, kAEDefaultTimeout, idleFunction, NULL);
        if (err == noErr)
            AEDisposeDesc(&throwAwayReply);
    }
    else
        err = AESend(ae, reply, sendMode,
                    kAENormalPriority, kAEDefaultTimeout, idleFunction, NULL);

    return err;
}


//----------------------------------------------------------------------------
// MakeAppleEvent
//----------------------------------------------------------------------------
OSErr MakeAppleEvent(AEEventClass aeClass, AEEventID aeID,
            AEDesc *target, AppleEvent *ae)
{
    OSErr err = noErr;

    if (target->dataHandle == nil)
    {
        DescType finderSig = 'MACS';

        err = AECreateDesc(typeApplSignature, (Ptr) &finderSig,
            sizeof(DescType), target);
    }

    if (!err)
        err = AECreateAppleEvent(aeClass, aeID, target,
            kAutoGenerateReturnID, kAnyTransactionID, ae);

    return err;
}

//----------------------------------------------------------------------------
// MakeSpecifierForFile
//----------------------------------------------------------------------------
OSErr MakeSpecifierForFile(FSSpecPtr hfsObj, AEDesc *fileSpecifier)
{
    OSErr       err;
    AEDesc      nullDesc, hfsData;
    AliasHandle fileAlias;

    //
    // Create the file descriptor with the FSSpec passed in.
    //
    err = NewAlias(NULL, hfsObj, &fileAlias);
    require_plain(err == noErr, NewAlias);

    HLock((Handle) fileAlias);
    err = AECreateDesc(typeAlias, (Ptr) *fileAlias,
                GetHandleSize((Handle) fileAlias), &hfsData);
    HUnlock((Handle) fileAlias);
    DisposeHandle((Handle) fileAlias);
    require_plain(err == noErr, AECreateDesc);

    //
    // Make the object specifier with a null container
    // (i.e., "file of <null>", or just "file")
    //
    nullDesc.descriptorType = typeNull;
    nullDesc.dataHandle = NULL;
    err = CreateObjSpecifier(typeWildCard, &nullDesc,
                formAlias, &hfsData, false, fileSpecifier);

AECreateDesc:
NewAlias:
    return err;
}


//----------------------------------------------------------------------------
// MakePropertySpecifierForSpecifier
//----------------------------------------------------------------------------
OSErr MakePropertySpecifierForSpecifier(DescType property,
            AEDesc *ofSpecifier, AEDesc *propertySpecifier)
{
    OSErr       err;
    AEDesc      keyData;

    //
    // Create a 'type' AEDesc with the desired property
    //
    err = AECreateDesc(typeType, (Ptr) &property, sizeof(DescType), &keyData);
    require_plain(err == noErr, AECreateDesc);

    //
    // With it create a property specifier for the object specifier
    // passed to us.
    //
    err = CreateObjSpecifier(cProperty, ofSpecifier,
                formPropertyID, &keyData, false, propertySpecifier);

    (void) AEDisposeDesc(&keyData);
AECreateDesc:
    return err;
}

//----------------------------------------------------------------------------
// BuildIconSuiteFromAEDesc
//
// OK, this uses the Apple Event Manager to pick the icon data out of the
// 'ifam' AEDesc.
//----------------------------------------------------------------------------
OSErr BuildIconSuiteFromAEDesc(Boolean largeIcons, Handle *iconSuite, AEDesc *iconFam)
{
    OSErr       err;
    Handle      suite, icon;
    AERecord    rec;
    Ptr         buffer;
    DescType    large[3] = {large8BitData, large4BitData, large1BitMask};
    DescType    small[3] = {small8BitData, small4BitData, small1BitMask};
    DescType    *type, iconType, typeCode;
    long        count, i;
    Size        maxSize, size, iconSize;
    Boolean     maskAdded;
    DescType    maskType;

//  largeIcons = true;
    maskAdded = false;
    suite = NULL;
    maxSize = kLarge8BitIconSize;

    if (largeIcons == true) {
        type = large;
        maskType = large1BitMask;
    }
    else {
        type = small;
        maskType = small1BitMask;
    }

    buffer = NewPtr(maxSize);
    require(buffer != NULL, NewPtr,
        "Out of memory error",
        "Couldn't allocate memory to build an icon suite.");

    err = NewIconSuite(&suite);
    require_num(err == noErr, NewIconSuite,
        "An error occurred while building an icon suite",
        "Couldn't create a new suite because an error of type %d occurred",
        err);

    err = AECoerceDesc(iconFam, typeAERecord, (AEDesc *) &rec);
    require_num(err == noErr, AECoerceDesc,
        "An error occurred while building an icon suite",
        "Couldn't coerce the description because an error of type %d occurred",
        err);

    for ( i = 0; i < 2; i++)
    {
        if ( i == 0) {
            type = large;
            maskType = large1BitMask;
        } else
        {
            type = small;
            maskType = small1BitMask;
        }


        for (count = 0; count < 3; count ++) {
            //
            // loop through the icons and grab the data from the AERecord for
            // each type of icon we're interested in.
            //
            iconType = type[count];
            size = GetSizeFromIconType(iconType);
            err = AEGetKeyPtr(&rec, iconType, iconType, &typeCode,
                            buffer, maxSize, &iconSize);

            if (err == noErr) {
                //
                // We don't set the error code for this unless the NewHandle
                // call fails, because it's possible that the 'ifam' doesn't
                // have an icon for one that we're interested in.
                //
                icon = NewHandle(size);

                if (icon != NULL) {
                    //
                    // OK, the memory alloc succeeded and we have data. Copy
                    // it into the allocated icon and add it to the suite.
                    // Set atLeastOne to true, to indicate later that we did
                    // in fact add at least one icon to this suite.
                    //
                    BlockMoveData(buffer, *icon, size);
                    err = AddIconToSuite(icon, suite, iconType);
                    if ((err == noErr) && (iconType == maskType))
                        maskAdded = true;
                }
                else
                    err = memFullErr;
            } else
            {
                err = noErr;
            }
        }
    }

    (void) AEDisposeDesc(&rec);

AECoerceDesc:
    if ((err != noErr) || (maskAdded == false)) {
        //
        // There was either an error in a memory allocation,  or something
        // else went wrong (like no mask was added to the suite).  Get
        // rid of the partially created suite.
        //
        DisposeIconSuite(suite, true);
        suite = NULL;
    }

NewIconSuite:
    DisposePtr(buffer);

NewPtr:
    *iconSuite = suite;
    return err;
}

//----------------------------------------------------------------------------
// FinderIsRunning
//
// Walk the Process Mgr list to check if the Finder is running
//----------------------------------------------------------------------------
Boolean FinderIsRunning( void)
{
    OSErr           err;
    ProcessInfoRec  pInfo;
    ProcessSerialNumber psn;
    Boolean         foundIt;

    foundIt = false;
    psn.highLongOfPSN = 0; psn.lowLongOfPSN = kNoProcess;

    while ((foundIt == false) && (GetNextProcess(&psn) == noErr)) {
        pInfo.processName       = NULL;
        pInfo.processAppSpec    = NULL;
        pInfo.processInfoLength = sizeof(ProcessInfoRec);

        err = GetProcessInformation(&psn, &pInfo);

        if ((err == noErr)
            && (pInfo.processSignature == 'MACS')
            && (pInfo.processType == 'FNDR'))

            foundIt = true;
    }

    return foundIt;
}

//----------------------------------------------------------------------------
// GetSizeFromIconType
//----------------------------------------------------------------------------
static Size GetSizeFromIconType(DescType iconType)
{
    Size    size = -1;

    switch (iconType) {
        case large8BitData:
            size = kLarge8BitIconSize;
            break;
        case large4BitData:
            size = kLarge4BitIconSize;
            break;
        case large1BitMask:
            size = kLargeIconSize;
            break;
        case small8BitData:
            size = kSmall8BitIconSize;
            break;
        case small4BitData:
            size = kSmall4BitIconSize;
            break;
        case small1BitMask:
            size = kSmallIconSize;
            break;
    }
    return size;
}


#pragma mark **PUBLIC FUNCTIONS**
/******************************************\
|**| public functions
\******************************************/

void DrawAliasSmallIcon( AliasHandle alias, short left, short top)
{
    Rect        iconRect;
    Handle      iconSuite = nil;
    FSSpec      fspec;
    OSStatus    error = noErr;
    Boolean     changed;
    Str255      s;

    error = ResolveAlias( nil, alias, &fspec, &changed);
    if ( error != noErr)
    {
        NumToString( error, s);
//              DebugStr( s);
    }

    error = GetIconSuiteFromFSSpec( &fspec, &iconSuite);
    if ( error != noErr)
    {
        NumToString( error, s);
//              DebugStr( s);
    }
    if ( iconSuite != nil)
    {
        iconRect.left = left;
        iconRect.top = top;
        iconRect.right = iconRect.left + 16;
        iconRect.bottom = iconRect.top + 16;
        error = PlotIconSuite (&iconRect,kAlignNone,kTransformNone, iconSuite);
        DisposeIconSuite( iconSuite, true);
    }

}

void DrawFSpecSmallIcon( FSSpec *fspec, short left, short top)
{
    Rect        iconRect;
    Handle      iconSuite = nil;
    OSStatus    error = noErr;
    Str255      s;

    error = GetIconSuiteFromFSSpec( fspec, &iconSuite);
    if ( error != noErr)
    {
        NumToString( error, s);
//              DebugStr( s);
    }
    if ( iconSuite != nil)
    {
        iconRect.left = left;
        iconRect.top = top;
        iconRect.right = iconRect.left + 16;
        iconRect.bottom = iconRect.top + 16;
        error = PlotIconSuite (&iconRect,kAlignNone,kTransformNone, iconSuite);
        DisposeIconSuite( iconSuite, true);
    }

}

Handle GetIconSuiteFromAlias( AliasHandle alias)
{
    Handle      iconSuite = nil;
    FSSpec      fspec;
    OSStatus    error = noErr;
    Boolean     changed;

    error = ResolveAlias( nil, alias, &fspec, &changed);
    if ( error != noErr)
    {
        return nil;
    }

    error = GetIconSuiteFromFSSpec( &fspec, &iconSuite);
    if ( error != noErr)
    {
        return nil;
    }

    return iconSuite;
}

