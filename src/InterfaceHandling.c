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

// Interface Handling.c

/*  Liaison between the application and Interface Drawing.  Takes in events ( key events, mouse
    down events), hilights and scrolls as needed, and returns results.  Also handles editable text.
*/
#ifndef __CONDITIONALMACROS__
#include "ConditionalMacros.h"
#endif // __CONDITIONALMACROS__

#if TARGET_OS_WIN32

    #ifndef __QUICKTIMEVR__
    #include <QuickTimeVR.h>
    #endif

    #ifndef __QTUtilities__
    #include "QTUtilities.h"
    #endif

    #ifndef __QTVRUtilities__
    #include "QTVRUtilities.h"
    #endif

    #include <TextUtils.h>
    #include <Script.h>
    #include <string.h>
#endif // TARGET_OS_WIN32

#include "Resources.h"

#include "MixedMode.h"
#include <QDOffscreen.h>

#include "Error.h"
#include "Debug.h"
#include "Set Font By String.h"

#include "Key Map Translation.h"

#include "Player Interface Items.h"
#include "Player Interface Drawing.h"
#include "Handle Handling.h"

#ifndef kAnyChar
#include "AnyChar.h"
#endif

#include "Color Translation.h"

#include "Sound FX.h"               // for button on/off

#include "Interface Handling.h"
#include "Offscreen GWorld.h"

#include "Ares_ResFile.h"
#include "Ares Global Type.h"

#define kMakeInterfaceItem      20

#define kInterfaceError         "\pINTF"
#define kInterfaceResFileName   "\p:Ares Data Folder:Ares Interfaces"

#define kInterfaceResourceType  'intr'

#define kClickLoopInfo          ( kRegisterBased | RESULT_SIZE( SIZE_CODE ( sizeof ( Boolean))) | REGISTER_RESULT_LOCATION( kRegisterD0))

#define kTargetScreenWidth      640
#define kTargetScreenHeight     480

#define mPlayButtonDown         PlayVolumeSound( kComputerBeep1, kMediumLoudVolume, kShortPersistence, kMustPlaySound)
#define mPlayButtonUp           PlayVolumeSound( kComputerBeep2, kMediumLowVolume, kShortPersistence, kMustPlaySound)
#define mPlayScreenSound        PlayVolumeSound( kComputerBeep3, kMediumLowVolume, kShortPersistence, kVeryLowPrioritySound)

extern CWindowPtr       gTheWindow;     // we need the window for copying to the real world, a hack
extern long             WORLD_WIDTH, WORLD_HEIGHT;
extern GWorldPtr        gOffWorld, gSaveWorld;

Handle              gInterfaceItemData = nil;
short               gInterfaceFileRefID = -1, gCurrentTEItem = -1;
long                gInterfaceScreenHBuffer = 0, gInterfaceScreenVBuffer = 0;
extern aresGlobalType   *gAresGlobal;
//UniversalProcPtr  gCallBackTest = nil;

int InterfaceHandlingInit( void)

{
//  gCallBackTest = NewRoutineDescriptor( (ProcPtr)MyClipLoop, kClickLoopInfo, GetCurrentISA());    

/*  gInterfaceFileRefID = ARF_OpenResFile( kInterfaceResFileName);
    if ( gInterfaceFileRefID < 0)
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kInterfacesFileError, kDataFolderError, -1, -1, __FILE__, 1);
        return( RESOURCE_ERROR);
    } else
        UseResFile ( gInterfaceFileRefID);
*/
    
//  if ( gAresGlobal->externalFileRefNum > 0)
//      UseResFile( gAresGlobal->externalFileRefNum);

    gInterfaceScreenHBuffer = ( WORLD_WIDTH / 2) - ( kTargetScreenWidth / 2);
    gInterfaceScreenVBuffer = ( WORLD_HEIGHT / 2) - ( kTargetScreenHeight / 2);

    return ( kNoError);
}

void InterfaceHandlingCleanup( void)

{
    if ( gInterfaceItemData != nil)
    {
//      DisposeHandle( gInterfaceItemData);
        mHandleDisposeAndDeregister( gInterfaceItemData)
    }
//  CloseResFile( gInterfaceFileRefID);         // not needed-done automatically when program quits
}

int OpenInterface( short resID)

{
    interfaceItemType   *item;
    long                count, number;
    
    if ( gInterfaceItemData != nil)
    {
//      DisposeHandle( gInterfaceItemData);
        mHandleDisposeAndDeregister( gInterfaceItemData)
    }   
    gInterfaceItemData = HHGetResource( kInterfaceResourceType, resID);
    if ( gInterfaceItemData == nil) return( RESOURCE_ERROR);
    
    DetachResource( gInterfaceItemData);
    /*
    MoveHHi( gInterfaceItemData);
    HLock( gInterfaceItemData);
    */
    mDataHandleLockAndRegister( gInterfaceItemData, nil, nil, nil, "\pgInterfaceItemData")
    InvalidateInterfaceFunctions(); // if they've been set, they shouldn't be yet
    
    number = GetHandleSize( gInterfaceItemData) / sizeof( interfaceItemType);
    item = ( interfaceItemType *)*gInterfaceItemData;
    for ( count = 0; count < number; count++)
    {
        item->bounds.left += gInterfaceScreenHBuffer;
        item->bounds.right += gInterfaceScreenHBuffer;
        item->bounds.top += gInterfaceScreenVBuffer;
        item->bounds.bottom += gInterfaceScreenVBuffer;
        
        item++;
    }
    
    return ( kNoError);
}

long AppendInterface( short resID, long relativeNumber, Boolean center)
{
    Handle              appendData;
    long                count, originalNumber, number;
    interfaceItemType   *item, *relativeItem;
    
//#pragma unused( center)
    appendData = HHGetResource( kInterfaceResourceType, resID);
    if (( appendData != nil) && ( gInterfaceItemData != nil))
    {
        originalNumber = GetHandleSize( gInterfaceItemData) / sizeof( interfaceItemType);
        HHConcatenateHandle( gInterfaceItemData, appendData);
        
        number = GetHandleSize( gInterfaceItemData) / sizeof( interfaceItemType);
        item = ( interfaceItemType *)*gInterfaceItemData + originalNumber;
        
        if ( relativeNumber < 0)
        {
            for ( count = originalNumber; count < number; count++)
            {
                item->bounds.left += gInterfaceScreenHBuffer;
                item->bounds.right += gInterfaceScreenHBuffer;
                item->bounds.top += gInterfaceScreenVBuffer;
                item->bounds.bottom += gInterfaceScreenVBuffer;
                
                item++;
            }
        } else if ( !center)
        {
            relativeItem = ( interfaceItemType *)*gInterfaceItemData + relativeNumber;

            for ( count = originalNumber; count < number; count++)
            {
                item->bounds.left += relativeItem->bounds.left;
                item->bounds.right += relativeItem->bounds.left;
                item->bounds.top += relativeItem->bounds.top;
                item->bounds.bottom += relativeItem->bounds.top;
                
                item++;
            }
        }
    } else return( -1);
    
    if ( center)
    {
        Rect    tRect;
        
        relativeItem = ( interfaceItemType *)*gInterfaceItemData + relativeNumber;
        mCopyAnyRect( tRect, relativeItem->bounds);
        CenterItemRangeInRect( &tRect, originalNumber, number);
    }
    return( number - originalNumber);
}

void ShortenInterface( long howMany)
{
    long    number;
    
    if ( gInterfaceItemData != nil)
    {
        number = GetHandleSize( gInterfaceItemData) / sizeof( interfaceItemType);
        
        if ( howMany <= number)
            SetHandleSize( gInterfaceItemData, (number - howMany) * sizeof( interfaceItemType));
    }
}

void CloseInterface( void)

{
    if ( gCurrentTEItem != -1)
    {
        InterfaceTextEditDeactivate( gCurrentTEItem);
    }
    InterfaceDisposeAllEditableText();
    
    if ( gInterfaceItemData != nil)
    {
//      DisposeHandle( gInterfaceItemData);
        mHandleDisposeAndDeregister( gInterfaceItemData)
    }
    gInterfaceItemData = nil;
}

void DrawEntireInterface( void)

{
    long                number, count;
    interfaceItemType   *item;
    Rect                tRect;
    PixMapHandle        offMap = GetGWorldPixMap( gOffWorld);

    DrawInOffWorld();
    MacSetRect( &tRect, 0, 0, WORLD_WIDTH, WORLD_HEIGHT);
    SetTranslateColorFore( BLACK);
    PaintRect( &tRect);
    
    number = GetHandleSize( gInterfaceItemData) / sizeof( interfaceItemType);
    item = ( interfaceItemType *)*gInterfaceItemData;
    
    for ( count = 0; count < number; count++)
    {
        DrawAnyInterfaceItem( item, *offMap, 0, 0);
        item++;
    }
    DrawInRealWorld();
    CopyOffWorldToRealWorld( (WindowPtr) gTheWindow, &tRect);
}

void DrawInterfaceRange( long from, long to, long withinItem)

{
    long                number, count;
    interfaceItemType   *item;
    Rect                tRect;
    PixMapHandle        offMap = GetGWorldPixMap( gOffWorld);

    DrawInOffWorld();
    if ( withinItem >= 0)
    {
        item = GetAnyInterfaceItemPtr( withinItem);
        LongRectToRect( &item->bounds, &tRect);
        SetTranslateColorFore( BLACK);
        PaintRect( &tRect);
    }
    number = GetHandleSize( gInterfaceItemData) / sizeof( interfaceItemType);
    if ( from < number)
    {
        if ( to > number) to = number;
        item = ( interfaceItemType *)*gInterfaceItemData + from;
            
        for ( count = from; count < to; count++)
        {
            DrawAnyInterfaceItem( item, *offMap, 0, 0);
            item++;
        }
        DrawInRealWorld();
        if ( withinItem >= 0)
            CopyOffWorldToRealWorld( (WindowPtr) gTheWindow, &tRect);
    }
}

void DrawAllItemsOfKind( interfaceKindType kind, Boolean sound, Boolean clearFirst,
                            Boolean showAtEnd)

{
    long                number, count;
    interfaceItemType   *item;
    Rect                tRect;
    PixMapHandle        offMap = GetGWorldPixMap( gOffWorld);

    DrawInOffWorld();
    MacSetRect( &tRect, 0, 0, WORLD_WIDTH, WORLD_HEIGHT);
    SetTranslateColorFore( BLACK);
    if ( clearFirst)
        PaintRect( &tRect);
    
    number = GetHandleSize( gInterfaceItemData) / sizeof( interfaceItemType);
    item = ( interfaceItemType *)*gInterfaceItemData;
    
    for ( count = 0; count < number; count++)
    {
        if ( sound)
            mPlayScreenSound;
        if ( item->kind == kind)
        {
            if ( showAtEnd)
                DrawAnyInterfaceItem( item, *offMap, 0, 0);
            else
                DrawAnyInterfaceItemOffToOn( item);
        }
        item++;
    }
    DrawInRealWorld();
    if ( showAtEnd)
        CopyOffWorldToRealWorld( (WindowPtr) gTheWindow, &tRect);
}

void DrawAnyInterfaceItemOffToOn( interfaceItemType *item)

{
    Rect            bounds;
    PixMapHandle        offMap = GetGWorldPixMap( gOffWorld);

    GetAnyInterfaceItemGraphicBounds( item, &bounds);
    DrawInOffWorld();
    DrawAnyInterfaceItem( item, *offMap, 0, 0);
    DrawInRealWorld();
    CopyOffWorldToRealWorld( (WindowPtr) gTheWindow, &bounds);
}

void DrawAnyInterfaceItemSaveToOffToOn( interfaceItemType   *item)

{
    Rect            bounds;
    PixMapHandle    saveMap = GetGWorldPixMap( gSaveWorld);

    GetAnyInterfaceItemGraphicBounds( item, &bounds);
    DrawInSaveWorld();
    DrawAnyInterfaceItem( item, *saveMap, 0, 0);
    DrawInOffWorld();
    CopySaveWorldToOffWorld( &bounds);
    DrawInRealWorld();
    CopyOffWorldToRealWorld( (WindowPtr) gTheWindow, &bounds);
}

void OffsetAllItems( long hoffset, long voffset)
{
    OffsetItemRange( hoffset, voffset, 0,
        GetHandleSize( gInterfaceItemData) / sizeof( interfaceItemType));
}

void OffsetItemRange( long hoffset, long voffset, long from, long to)
{
    long                number, count;
    interfaceItemType   *item;

    number = to - from;//GetHandleSize( gInterfaceItemData) / sizeof( interfaceItemType);
    item = ( interfaceItemType *)*gInterfaceItemData + from;
    
    for ( count = 0; count < number; count++)
    {
        item->bounds.left += hoffset;
        item->bounds.right += hoffset;
        item->bounds.top += voffset;
        item->bounds.bottom += voffset;
        item++;
    }
}

void CenterAllItemsInRect( Rect *destRect)
{
    CenterItemRangeInRect( destRect, 0,
        GetHandleSize( gInterfaceItemData) / sizeof( interfaceItemType));
}

void CenterItemRangeInRect( Rect *destRect, long from, long to)
{
    long                number, count, hoffset, voffset;
    interfaceItemType   *item;
    longRect            itemsBounds = { 0x7fffffff, 0x7fffffff, 0, 0};
    
    number = to - from;//GetHandleSize( gInterfaceItemData) / sizeof( interfaceItemType);
    item = ( interfaceItemType *)*gInterfaceItemData + from;
    
    // first calc the rect that encloses all the interface items
    for ( count = 0; count < number; count++)
    {
        if ( item->bounds.left < itemsBounds.left) itemsBounds.left = item->bounds.left;
        if ( item->bounds.right > itemsBounds.right) itemsBounds.right = item->bounds.right;
        if ( item->bounds.top < itemsBounds.top) itemsBounds.top = item->bounds.top;
        if (item->bounds.bottom > itemsBounds.bottom) itemsBounds.bottom = item->bounds.bottom;
        item++;
    }
    
    // then center it in the destRect
    
    hoffset =   (
                    (
                        ( 
                            (long)(
                                destRect->right - destRect->left
                            )
                            / (long)2
                        )
                        + (long)destRect->left
                    ) -
                    (
                        ( 
                            itemsBounds.right - itemsBounds.left
                        )
                        / (long)2
                    )
                ) - itemsBounds.left;
    voffset =   (
                    (
                        ( 
                            (long)(
                                destRect->bottom - destRect->top
                            )
                            / (long)2
                        )
                        + (long)destRect->top
                    ) -
                    (
                        ( 
                            itemsBounds.bottom - itemsBounds.top
                        )
                        / (long)2
                    )
                ) - itemsBounds.top;
//  OffsetAllItems( hoffset, voffset);
    OffsetItemRange( hoffset, voffset, from, to);
}

void InvalidateInterfaceFunctions( void)

{
    long                number, count;
    interfaceItemType   *item;
    
    number = GetHandleSize( gInterfaceItemData) / sizeof( interfaceItemType);
    item = ( interfaceItemType *)*gInterfaceItemData;
    
    for ( count = 0; count < number; count++)
    {
        if ( item->kind == kListRect)
        {
            item->item.listRect.getListLength = nil;
            item->item.listRect.getItemString = nil;
            item->item.listRect.itemHilited = nil;
        }
        item++;
    }
}

void InterfaceDisposeAllEditableText( void)

{
    long                number, count;
    interfaceItemType   *item;
    
    if ( gInterfaceItemData != nil)
    {
        number = GetHandleSize( gInterfaceItemData) / sizeof( interfaceItemType);
        item = ( interfaceItemType *)*gInterfaceItemData;
        
        for ( count = 0; count < number; count++)
        {
            if (( item->kind == kLabeledRect) && ( item->item.labeledRect.editable) &&
                ( item->item.labeledRect.teData != nil))
            {
                TEDispose( item->item.labeledRect.teData);
            }
            item++;
        }
    }
}

void InterfaceIdle( void)

{
    interfaceItemType   *item;

    if ( gCurrentTEItem != -1)
    {
        item = ( interfaceItemType *)*gInterfaceItemData + (long)gCurrentTEItem;
        TEIdle( item->item.labeledRect.teData);
    }
}

short PtInInterfaceItem( Point where)
{
    short   result = -1;
    long                number, count;
    interfaceItemType   *item;
    Rect                tRect;
    
    number = GetHandleSize( gInterfaceItemData) / sizeof( interfaceItemType);
    item = ( interfaceItemType *)*gInterfaceItemData;
    
    for ( count = 0; count < number; count++)
    {
        GetAnyInterfaceItemGraphicBounds( item, &tRect);
        
        if ( MacPtInRect( where, &tRect))
        {
            if (( item->kind != kTabBox) && ( item->kind != kPictureRect))
            {
                return( count);
            }
        }
        item++;
    }
    
    return( -1);
}

short InterfaceMouseDown( Point where)

{
    long                number, count;
    interfaceItemType   *item;
    Rect                tRect;
    short               result = -1;
    
    number = GetHandleSize( gInterfaceItemData) / sizeof( interfaceItemType);
    item = ( interfaceItemType *)*gInterfaceItemData;
    
    for ( count = 0; count < number; count++)
    {
//      LongRectToRect( &(item->bounds), &tRect);
        GetAnyInterfaceItemGraphicBounds( item, &tRect);
        
        if ( MacPtInRect( where, &tRect))
        {
            switch ( item->kind)
            {
                case kPlainButton:
                    if ( InterfaceButtonHit( item))
                        result = count;
                    return( result);
                    break;
                case kCheckboxButton:
                    if ( InterfaceCheckboxHit( item))
                        result = count;
                    return( result);
                    break;
                case kRadioButton:
                    if ( InterfaceRadioButtonHit( item))
                        result = count;
                    return( result);
                    break;
                case kTabBoxButton:
                    if ( InterfaceTabBoxButtonHit( item))
                        result = count;
                    return( result);
                    break;
                    
                case kLabeledRect:
                    if (( item->item.labeledRect.editable) && ( item->item.labeledRect.teData != nil))
                    {
                        if ( count != gCurrentTEItem)
                        {
                            InterfaceTextEditDeactivate( (short)gCurrentTEItem);
                            gCurrentTEItem = count;
                            InterfaceTextEditActivate( gCurrentTEItem);
                        }
                        SetInterfaceTextEditColors( gCurrentTEItem);
                        TEClick( where, ShiftKey(), item->item.labeledRect.teData);
                        DefaultColors();
                    }
                    return( result);
                    break;
                case kListRect:
                    InterfaceListRectHit( item, where);
                    result = count;
                    return( result);
                    break;
                default:
                    break;
            }
            
        }
        item++;
    }
    return ( -1);
}

short InterfaceKeyDown( long message)

{
    long                number = 0, count = 0;
    interfaceItemType   *item;
    short               result = -1, buttonKey;
    KeyMap              keyMap;
    Boolean             caughtKey = false;
    char                whichChar;
    long                keyCode;
    
    keyCode = message & keyCodeMask;
    keyCode >>= 8;
    keyCode += 1;
    whichChar = message & charCodeMask;
    if ( keyCode > 0)
    {
        number = GetHandleSize( gInterfaceItemData) / sizeof( interfaceItemType);
        item = ( interfaceItemType *)*gInterfaceItemData;
        
/*      while ( !(( item->kind == kPlainButton) &&
                ( item->item.plainButton.key == keyNum)) && ( count < number))
*/      
        // check plain buttons
        do
        {
            buttonKey = 0;
            switch( item->kind)
            {
                case kPlainButton:
                    if ( item->item.plainButton.status != kDimmed)
                        buttonKey = item->item.plainButton.key;
                    break;
                
                case kTabBoxButton:
                    if ( item->item.radioButton.status != kDimmed)
                        buttonKey = item->item.radioButton.key;
                    break;
                
                default:
                    buttonKey = 0;
                    break;
            }
            if ( keyCode == buttonKey)
                caughtKey = true;
            else
            {
                count++;
                item++;
            }
        } while ((!caughtKey) && ( count < number));

/*      while ( !(( item->kind == kPlainButton)
                && ( keyCode == item->item.plainButton.key)) && ( count < number))

        {
            count++;
            item++;
        }
*/      
        
        if ( caughtKey)
        {
                
//          item->item.plainButton.status = kIH_Hilite;
            SetStatusOfAnyInterfaceItem( count, kIH_Hilite, false);
            DrawAnyInterfaceItemOffToOn( item);
            mPlayButtonDown;
            do
            {
                GetKeys( keyMap);
            } while ( GetKeyNumFromKeyMap( keyMap) == buttonKey);
//              item->item.plainButton.key);
//          item->item.plainButton.status = kActive;
            SetStatusOfAnyInterfaceItem( count, kActive, false);

            switch ( item->kind)
            {
                case kTabBoxButton:
                    item->item.radioButton.on = TRUE;
                    break;
                
                default:
                    break;
            }

            DrawAnyInterfaceItemOffToOn( item);
            return( (short)count);
        }   

    }
    
    if (( gCurrentTEItem != -1) && ( !caughtKey))
    {
        if ( whichChar == 0x09) // TAB KEY
        {
            number = GetHandleSize( gInterfaceItemData) / sizeof( interfaceItemType);
            count = gCurrentTEItem + 1;
            item = ( interfaceItemType *)*gInterfaceItemData + (long)count;
            
            while ( !(( item->kind == kLabeledRect) && ( item->item.labeledRect.editable) &&
                    ( item->item.labeledRect.teData != nil)))
            {
                if ( count >= number)
                {
                    count = 0;
                    item = ( interfaceItemType *)*gInterfaceItemData;
                }
                item++;
                count++;
            }
            
            if ( count != gCurrentTEItem)
            {
                InterfaceTextEditDeactivate( gCurrentTEItem);
                gCurrentTEItem = count;
            }
//          InterfaceTextEditSelectAll( gCurrentTEItem);
            InterfaceTextEditActivate( gCurrentTEItem);
        } else
        {
            SetInterfaceTextEditColors( gCurrentTEItem);
            item = ( interfaceItemType *)*gInterfaceItemData + (long)gCurrentTEItem;
            TEKey( whichChar, item->item.labeledRect.teData);
            DefaultColors();
        }
        return( gCurrentTEItem);
    }
    return( -1);
}

Boolean InterfaceButtonHit( interfaceItemType *button)

{
    Rect    tRect;
    Point   where;
    
//  LongRectToRect( &(button->bounds), &tRect);
    GetAnyInterfaceItemGraphicBounds( button, &tRect);
    
    if ( button->item.plainButton.status == kDimmed) return( FALSE);
    
    while ( Button())
    {
        GetMouse( &where);
        if ( MacPtInRect( where, &tRect))
        {
            if ( button->item.plainButton.status != kIH_Hilite)
            {
                mPlayButtonDown;
                button->item.plainButton.status = kIH_Hilite;
                DrawAnyInterfaceItemOffToOn( button);
            }
        } else
        {
            if ( button->item.plainButton.status != kActive)
            {
                mPlayButtonUp;
                button->item.plainButton.status = kActive;
                DrawAnyInterfaceItemOffToOn( button);
            }
        }
    }
    if ( button->item.plainButton.status == kIH_Hilite)
    {
        button->item.plainButton.status = kActive;
        DrawAnyInterfaceItemOffToOn( button);
    }
    return( MacPtInRect( where, &tRect));
}

Boolean InterfaceCheckboxHit( interfaceItemType *button)

{
    Rect    tRect;
    Point   where;
    
//  LongRectToRect( &(button->bounds), &tRect);
    GetAnyInterfaceItemGraphicBounds( button, &tRect);
    
    if ( button->item.checkboxButton.status == kDimmed) return( FALSE);
    
    do
    {
        GetMouse( &where);
        if ( MacPtInRect( where, &tRect))
        {
            if ( button->item.checkboxButton.status != kIH_Hilite)
            {
                mPlayButtonDown;
                button->item.checkboxButton.status = kIH_Hilite;
                DrawAnyInterfaceItemOffToOn( button);
            }
        } else
        {
            if ( button->item.checkboxButton.status != kActive)
            {
                button->item.checkboxButton.status = kActive;
                DrawAnyInterfaceItemOffToOn( button);
            }
        }
    } while ( Button());
    if ( button->item.checkboxButton.status == kIH_Hilite)
    {
        button->item.checkboxButton.status = kActive;
    }
    GetMouse( &where);
    if ( MacPtInRect( where, &tRect))
    {
        if ( button->item.checkboxButton.on)
            button->item.checkboxButton.on = FALSE;
        else button->item.checkboxButton.on = TRUE; 
        DrawAnyInterfaceItemOffToOn( button);
        return( true);
    } else
    {
        DrawAnyInterfaceItemOffToOn( button);
        return( false);
    }
}

Boolean InterfaceRadioButtonHit( interfaceItemType *button)

{
    Rect    tRect;
    Point   where;
    
//  LongRectToRect( &(button->bounds), &tRect);
    GetAnyInterfaceItemGraphicBounds( button, &tRect);
    
    if ( button->item.radioButton.status == kDimmed) return( FALSE);
    
    if ( button->item.radioButton.on == FALSE)
        button->item.radioButton.on = TRUE;
    
    do
    {
        GetMouse( &where);
        if ( MacPtInRect( where, &tRect))
        {
            if ( button->item.radioButton.status != kIH_Hilite)
            {
                mPlayButtonDown;
                button->item.radioButton.status = kIH_Hilite;
                DrawAnyInterfaceItemOffToOn( button);
            }
        } else
        {
            if ( button->item.radioButton.status != kActive)
            {
                button->item.radioButton.status = kActive;
                DrawAnyInterfaceItemOffToOn( button);
            }
        }
    } while ( Button());
    if ( button->item.radioButton.status == kIH_Hilite)
    {
        button->item.radioButton.status = kActive;
    }
    DrawAnyInterfaceItemOffToOn( button);
//  return( MacPtInRect( where, &tRect));
    return(TRUE);
}

Boolean InterfaceTabBoxButtonHit( interfaceItemType *button)

{
    Rect    tRect;
    Point   where;
    
    GetAnyInterfaceItemGraphicBounds( button, &tRect);
    
    if ( button->item.radioButton.status == kDimmed) return( FALSE);
    
    if ( button->item.radioButton.on != FALSE) return( false);
    
    do
    {
        GetMouse( &where);
        if ( MacPtInRect( where, &tRect))
        {
            if ( button->item.radioButton.status != kIH_Hilite)
            {
                mPlayButtonDown;
                button->item.radioButton.status = kIH_Hilite;
                DrawAnyInterfaceItemOffToOn( button);
            }
        } else
        {
            if ( button->item.radioButton.status != kActive)
            {
                button->item.radioButton.status = kActive;
                DrawAnyInterfaceItemOffToOn( button);
            }
        }
    } while ( Button());
    if ( button->item.radioButton.status == kIH_Hilite)
    {
        button->item.radioButton.status = kActive;
    }
    button->item.radioButton.on = TRUE;
    DrawAnyInterfaceItemOffToOn( button);

    return(TRUE);
}

void InterfaceListRectHit( interfaceItemType *listRect, Point where)

{
    Rect    tRect;
    short   lineHeight, whichHit;
    
    if ( listRect->item.listRect.getListLength != nil)
    {
        LongRectToRect( &(listRect->bounds), &tRect);
        lineHeight = GetInterfaceFontHeight(listRect->style) + kInterfaceTextVBuffer;
        where.v -= tRect.top;
        whichHit = where.v / lineHeight + listRect->item.listRect.topItem;
        if ( whichHit >= (*(listRect->item.listRect.getListLength))())
            whichHit = -1;
        (*(listRect->item.listRect.itemHilited))( whichHit, TRUE);
    }
}

/*
void DrawStringInInterfaceContent( short whichItem, anyCharType *s)

{
    interfaceItemType   *item;
    Rect                tRect;
        
    item = ( interfaceItemType *)*gInterfaceItemData + (long)whichItem;

    LongRectToRect( &(item->bounds), &tRect);
    DefaultColors();
    PaintRect( &tRect);
    MoveTo( tRect.left + kInterfaceTextHBuffer, tRect.top + GetInterfaceFontHeight( item->style) +
                kInterfaceTextVBuffer);
    SetTranslateColorShadeFore( item->color, VERY_LIGHT);   
    DrawInterfaceString( s, item->style);
}   
*/

interfaceItemType *GetAnyInterfaceItemPtr( long whichItem)

{
    return ( ( interfaceItemType *)*gInterfaceItemData + (long)whichItem);
}

void SetStatusOfAnyInterfaceItem( short whichItem, interfaceItemStatusType status, Boolean drawNow)

{
    interfaceItemType       *item;
    
    item = ( interfaceItemType *)*gInterfaceItemData + (long)whichItem;

    switch ( item->kind)
    {
        case kPlainButton:
            item->item.plainButton.status = status;
            break;
        case kRadioButton:
        case kTabBoxButton:
            item->item.radioButton.status = status;
            break;
        case kCheckboxButton:
            item->item.checkboxButton.status = status;
            break;
        case kTextRect:
            item->item.textRect.visibleBounds = ( status == kActive);
            break;
        case kPictureRect:
            item->item.pictureRect.visibleBounds = ( status == kActive);
            break;
    }
    if ( drawNow)
        RefreshInterfaceItem( whichItem);
}


void SwitchAnyRadioOrCheckbox( short whichItem, Boolean turnOn)

{
    interfaceItemType       *item;

    item = ( interfaceItemType *)*gInterfaceItemData + (long)whichItem;
    
    
    
    if ( item->kind == kCheckboxButton)
        item->item.checkboxButton.on = turnOn;
    else if (( item->kind == kRadioButton) || ( item->kind == kTabBoxButton))
        item->item.radioButton.on = turnOn;
            
}

Boolean GetAnyRadioOrCheckboxOn( short whichItem)

{
    interfaceItemType       *item;

    item = ( interfaceItemType *)*gInterfaceItemData + (long)whichItem;
    
    if ( item->kind == kCheckboxButton)
        return( item->item.checkboxButton.on);
    else if ( item->kind == kRadioButton)
        return (item->item.radioButton.on);
            
            
    return( FALSE);
}


void RefreshInterfaceItem( short whichItem)

{
    interfaceItemType   *item;
    Rect                tRect;
    
    item = ( interfaceItemType *)*gInterfaceItemData + (long)whichItem;
    GetAnyInterfaceItemGraphicBounds( item, &tRect);
    DrawInOffWorld();
    DefaultColors();
    PaintRect( &tRect);
    DrawAnyInterfaceItemOffToOn( item);
}

/*
void RefreshInterfaceListEntry( short whichItem, short whichEntry)

{
    interfaceItemType   *item;
    
    item = ( interfaceItemType *)*gInterfaceItemData + (long)whichItem;
    
    DrawPlayerInterfaceListEntry( item, whichEntry);
}
*/
    
void InterfaceTextEditItemInit( short whichItem)

{
    interfaceItemType   *item;
    Rect                tRect;
    
    item = ( interfaceItemType *)*gInterfaceItemData + (long)whichItem;
    
    if (( item->kind == kLabeledRect) && ( item->item.labeledRect.editable))
    {
        if ( item->item.labeledRect.teData == nil)
        {
            LongRectToRect( &(item->bounds), &tRect);
            SetInterfaceTextEditColors( whichItem);

            item->item.labeledRect.teData = TENew( &tRect, &tRect);
//          SetClikLoop( (UniversalProcPtr)gCallBackTest, item->item.labeledRect.teData);
            DefaultColors();
        }
    }
}

void InterfaceTextEditSetText( short whichItem, anyCharType *s)

{
    interfaceItemType   *item;
    
    item = ( interfaceItemType *)*gInterfaceItemData + (long)whichItem;
    
    if (( item->kind == kLabeledRect) && ( item->item.labeledRect.editable) &&
            ( item->item.labeledRect.teData != nil))
    {
        TESetText( (Ptr)( s + 1), (long)*s, item->item.labeledRect.teData);
    }
}

void InterfaceTextEditSelectAll( short whichItem)

{
    interfaceItemType   *item;
    
    item = ( interfaceItemType *)*gInterfaceItemData + (long)whichItem;
    
    if (( item->kind == kLabeledRect) && ( item->item.labeledRect.editable) &&
            ( item->item.labeledRect.teData != nil))
    {
        SetInterfaceTextEditColors( whichItem);
        TESetSelect( 0, 32767, item->item.labeledRect.teData);
        DefaultColors();
    }
}

void InterfaceTextEditActivate( short whichItem)

{
    interfaceItemType   *item;
    Rect                tRect;
    
    item = ( interfaceItemType *)*gInterfaceItemData + (long)whichItem;
    
    if (( item->kind == kLabeledRect) && ( item->item.labeledRect.editable) &&
            ( item->item.labeledRect.teData != nil))
    {
        SetInterfaceTextEditColors( whichItem);
        
        LongRectToRect( &(item->bounds), &tRect);
        TEActivate( item->item.labeledRect.teData);
        EraseRect( &tRect);
        TEUpdate( &tRect, item->item.labeledRect.teData);
        gCurrentTEItem = whichItem;
        
    }
}

void InterfaceTextEditDeactivate( short whichItem)

{
    interfaceItemType   *item;
    Rect                tRect;
    
    item = ( interfaceItemType *)*gInterfaceItemData + (long)whichItem;
    
    if (( item->kind == kLabeledRect) && ( item->item.labeledRect.editable) &&
            ( item->item.labeledRect.teData != nil))
    {
        LongRectToRect( &(item->bounds), &tRect);

        SetInterfaceTextEditColors( whichItem);
        
        TEDeactivate( item->item.labeledRect.teData);
        EraseRect( &tRect);
        TEUpdate( &tRect, item->item.labeledRect.teData);
        gCurrentTEItem = -1;

        DefaultColors();
    }
}

void SuspendActiveTextEdit( void)
{
    interfaceItemType   *item;
    Rect                tRect;

    if ( gInterfaceItemData != nil)
    {
        if ( gCurrentTEItem != -1)
        {
            item = ( interfaceItemType *)*gInterfaceItemData + (long)gCurrentTEItem;
            
            if (( item->kind == kLabeledRect) && ( item->item.labeledRect.editable) &&
                    ( item->item.labeledRect.teData != nil))
            {
                SetInterfaceTextEditColors( gCurrentTEItem);
                
                LongRectToRect( &(item->bounds), &tRect);
                TEDeactivate( item->item.labeledRect.teData);
                EraseRect( &tRect);
                TEUpdate( &tRect, item->item.labeledRect.teData);               
            }
        }
    }
}

void ResumeActiveTextEdit( void)
{
    interfaceItemType   *item;
    Rect                tRect;

    if ( gInterfaceItemData != nil)
    {
        if ( gCurrentTEItem != -1)
        {
            item = ( interfaceItemType *)*gInterfaceItemData + (long)gCurrentTEItem;
            
            if (( item->kind == kLabeledRect) && ( item->item.labeledRect.editable) &&
                    ( item->item.labeledRect.teData != nil))
            {
                SetInterfaceTextEditColors( gCurrentTEItem);
                
                LongRectToRect( &(item->bounds), &tRect);
                TEActivate( item->item.labeledRect.teData);
                EraseRect( &tRect);
                TEUpdate( &tRect, item->item.labeledRect.teData);               
            }
        }
    }
}

void UpdateAllTextEdit( void)
{
    long                number, count;
    interfaceItemType   *item;
    Rect                tRect;
    
    if ( gInterfaceItemData != nil)
    {

        number = GetHandleSize( gInterfaceItemData) / sizeof( interfaceItemType);
        item = ( interfaceItemType *)*gInterfaceItemData;
        
        for ( count = 0; count < number; count++)
        {
            SetInterfaceTextEditColors( count);
            if (( item->kind == kLabeledRect) && ( item->item.labeledRect.editable) &&
                ( item->item.labeledRect.teData != nil))
            {
                LongRectToRect( &(item->bounds), &tRect);

                EraseRect( &tRect);
                TEUpdate( &tRect, item->item.labeledRect.teData);
            }
            item++;
        }
        DefaultColors();
    }
}

void SetInterfaceTextEditColors( short whichItem)

{
    interfaceItemType   *item;
    RGBColor            color;
    
    item = ( interfaceItemType *)*gInterfaceItemData + (long)whichItem;
    DrawInRealWorld();
    MacSetPort( (WindowPtr)gTheWindow);
    PenNormal();
    DefaultColors();
    if (( item->kind == kLabeledRect) && ( item->item.labeledRect.editable))
    {
    
        GetRGBTranslateColor( &color, BLACK);
        RGBBackColor( &color);
        
        GetRGBTranslateColorShade( &color, item->color, VERY_LIGHT);
        RGBForeColor( &color);

        GetRGBTranslateColorShade( &color, item->color, MEDIUM);
        HiliteColor( &color);
    
        SetFontByString( "\pmonaco");
        TextSize( 9);
        TextFace(  0);
    } else WriteDebugLong( (long)whichItem);
}

void CopyInterfaceTextEditContents( short whichItem, anyCharType *d, long *maxlen)

{
    interfaceItemType   *item;
    long                slen;
    
    item = ( interfaceItemType *)*gInterfaceItemData + (long)whichItem;
    
    if (( item->kind == kLabeledRect) && ( item->item.labeledRect.editable) &&
            ( item->item.labeledRect.teData != nil))
    {
        slen = (*(item->item.labeledRect.teData))->teLength;
        if ( slen <= *maxlen)
        {
            BlockMove( *((*(item->item.labeledRect.teData))->hText),
                        (Ptr)d, slen);
            *maxlen = slen;
        } else
        {
            BlockMove( *((*(item->item.labeledRect.teData))->hText),
                        (Ptr)d, *maxlen);
        }
    }
}

long GetInterfaceTextEditLength( short whichItem)
{

    interfaceItemType   *item;
    
    item = ( interfaceItemType *)*gInterfaceItemData + (long)whichItem;
    
    if (( item->kind == kLabeledRect) && ( item->item.labeledRect.editable) &&
            ( item->item.labeledRect.teData != nil))
    {
        return ( (*(item->item.labeledRect.teData))->teLength);
    } else return( -1);
}

void SetInterfaceListCallback(  short       whichItem,
                                short       (*getListLength)( void),
                                void        (*getItemString)( short, anyCharType *),
                                Boolean     (*itemHilited)( short, Boolean))
{   
    interfaceItemType   *item;
    
    item = ( interfaceItemType *)*gInterfaceItemData + (long)whichItem;
    
    if ( item->kind == kListRect)
    {
        item->item.listRect.getListLength = getListLength;
        item->item.listRect.getItemString = getItemString;
        item->item.listRect.itemHilited = itemHilited;
//      item->item.listRect.hiliteItem = hiliteItem;
        item->item.listRect.topItem = 0;
    }
    
}

void SetButtonKeyNum( short whichItem, short whichKey)

{
    interfaceItemType   *item;
    
    item = ( interfaceItemType *)*gInterfaceItemData + (long)whichItem;
    
    if ( item->kind == kPlainButton)
    {
        item->item.plainButton.key = whichKey;
    }
}

short GetButtonKeyNum( short whichItem)

{
    interfaceItemType   *item;

    item = ( interfaceItemType *)*gInterfaceItemData + (long)whichItem;
    
    if ( item->kind == kPlainButton)
        return ( item->item.plainButton.key);
    else return( 0);
}

void SetInterfaceTextBoxText( short resID);
void SetInterfaceTextBoxText( short resID)
{
}
