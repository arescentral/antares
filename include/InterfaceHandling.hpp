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

#ifndef ANTARES_INTERFACE_HANDLING_HPP_
#define ANTARES_INTERFACE_HANDLING_HPP_

// Interface Handling.h

#include "AnyChar.h"
#include "PlayerInterfaceItems.h"

#pragma options align=mac68k

int InterfaceHandlingInit( void);
void InterfaceHandlingCleanup( void);
int OpenInterface( short);
long AppendInterface( short, long, Boolean);
void ShortenInterface( long);
void CloseInterface( void);
void DrawEntireInterface( void);
void DrawInterfaceRange( long, long, long);
void DrawAllItemsOfKind( interfaceKindType, Boolean, Boolean, Boolean);
void OffsetItemRange( long, long, long, long);
void OffsetAllItems( long, long);
void CenterItemRangeInRect( Rect *, long, long);
void CenterAllItemsInRect( Rect *);
void DrawAnyInterfaceItemOffToOn( interfaceItemType *);
void DrawAnyInterfaceItemSaveToOffToOn( interfaceItemType *);
void InvalidateInterfaceFunctions( void);
void InterfaceDisposeAllEditableText( void);
void InterfaceIdle( void);
short PtInInterfaceItem( Point);
short InterfaceMouseDown( Point);
short InterfaceKeyDown( long);
Boolean InterfaceButtonHit( interfaceItemType *);
Boolean InterfaceCheckboxHit( interfaceItemType *);
Boolean InterfaceRadioButtonHit( interfaceItemType *);
Boolean InterfaceTabBoxButtonHit( interfaceItemType *);
void InterfaceListRectHit( interfaceItemType *, Point);
void DrawStringInInterfaceContent( short, anyCharType *);
interfaceItemType *GetAnyInterfaceItemPtr( long);
void SetStatusOfAnyInterfaceItem( short, interfaceItemStatusType, Boolean);
void SwitchAnyRadioOrCheckbox( short, Boolean);
Boolean GetAnyRadioOrCheckboxOn( short);
void RefreshInterfaceItem( short);
void RefreshInterfaceListEntry( short, short);
void InterfaceTextEditItemInit( short);
void InterfaceTextEditSetText( short, anyCharType *);
void InterfaceTextEditSelectAll( short);
void InterfaceTextEditActivate( short);
void InterfaceTextEditDeactivate( short);
void SuspendActiveTextEdit( void);
void ResumeActiveTextEdit( void);
void UpdateAllTextEdit( void);
void SetInterfaceTextEditColors( short);
void CopyInterfaceTextEditContents( short, anyCharType *, long *);
long GetInterfaceTextEditLength( short);
void SetInterfaceListCallback(  short       whichItem,
                                short       (*getListLength)( void),
                                void        (*getItemString)( short, anyCharType *),
                                Boolean     (*itemHilited)( short, Boolean));
void SetButtonKeyNum( short, short);
short GetButtonKeyNum( short);

#pragma options align=reset

#endif // ANTARES_INTERFACE_HANDLING_HPP_
