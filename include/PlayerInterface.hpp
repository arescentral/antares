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

#ifndef ANTARES_PLAYER_INTERFACE_HPP_
#define ANTARES_PLAYER_INTERFACE_HPP_

// Player Interface.h

#include "AresNetwork.hpp"
#include "PlayerInterfaceDrawing.hpp"
#include "PlayerInterfaceItems.hpp"

#pragma options align=mac68k

typedef enum
{
    kMainPlay = 1,
    kMainQuit = 2,
    kMainAbout = 3,
    kMainOptions = 4,
    kMainNetwork = 5,
    kMainDemo = 6,
    kMainTimeoutDemo = 7,
    kMainTrain = 8,
    kNullResult = 0
} mainScreenResultType;

typedef enum
{
    kClient = 1,
    kHost = 2,
    kCancel = 0
} netResultType;

mainScreenResultType DoMainScreenInterface( long *);
void DoAboutAresInterface( void);
void DoLoadingInterface( Rect *, StringPtr);
void UpdateLoadingInterface( long, long, Rect *);
short DoPlayAgain( Boolean, Boolean);
void DoNetSettings( void);
void DoHelpScreen( void);
void StartPauseIndicator(StringPtr, unsigned char);
void StopPauseIndicator( StringPtr);
void DrawInterfaceOneAtATime( void);
void DoOptionsInterface( void);
void SetOptionCheckboxes( unsigned long);
void DrawOptionVolumeLevel( Rect *, long);
Boolean DoKeyInterface( void);
void DrawKeyControlPicture( long);
Boolean BothCommandAndQ( void);
netResultType StartNetworkGameSetup( void);
netResultType HostAcceptClientInterface( void);
netResultType ClientNetworkGameSetup( void);
void DrawStringInInterfaceItem( long, const unsigned char*);
netResultType ClientWaitInterface( void);
netResultType HostBeginGame( void);
netResultType ClientBeginGame( void);
void BlackenWindow( void);
short GetClientListLength( void);
void GetClientListName( short, anyCharType *);
Boolean IsThisClientHilited( short, Boolean);
netClientEntity *GetSelectClient( void);
short GetInGameListLength( void);
void GetInGameListName( short, anyCharType *);
Boolean IsThisInGameHilited( short, Boolean);
long DoNetLevelInterface( void);
long DoSelectLevelInterface( long);
void DrawLevelNameInBox( StringPtr, long, short, long);
Boolean DoMissionInterface( long);
long UpdateMissionBriefPoint( interfaceItemType *, long, long, coordPointType *, long, long,
        Rect *, Rect *, inlinePictType *);
void ShowObjectData( Point, short, Rect *);
Handle CreateWeaponDataText( long, StringPtr);
void ShowSuccessAnimation( WindowPtr);
void DoMissionDebriefing( WindowPtr, Rect *, long, long, long, long, long, long, long);
void DoMissionDebriefingText( WindowPtr, long, long, long, long, long, long, long, long);
void DoScrollText( WindowPtr, long, long, long, long, long);
void HandleOSEvent( EventRecord *);
Boolean Ares_WaitNextEvent( short eventMask, EventRecord *theEvent,
    unsigned long sleep, RgnHandle mouseRgn);
void Replace_KeyCode_Strings_With_Actual_Key_Names( Handle text, short resID, short padTo);

#pragma options align=reset

#endif // ANTARES_PLAYER_INTERFACE_HPP_
