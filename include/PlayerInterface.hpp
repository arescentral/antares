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

#include "PlayerInterfaceDrawing.hpp"
#include "PlayerInterfaceItems.hpp"

namespace sfz { class String; }
namespace sfz { class StringPiece; }

namespace antares {

enum mainScreenResultType {
    kMainPlay = 1,
    kMainQuit = 2,
    kMainAbout = 3,
    kMainOptions = 4,
    kMainNetwork = 5,
    kMainDemo = 6,
    kMainTimeoutDemo = 7,
    kMainTrain = 8,
};

enum netResultType {
    kClient = 1,
    kHost = 2,
    kCancel = 0
};

enum PlayAgainResult {
    PLAY_AGAIN_QUIT,
    PLAY_AGAIN_RESTART,
    PLAY_AGAIN_RESUME,
    PLAY_AGAIN_SKIP,
};

void DoLoadingInterface(Rect*, unsigned char*);
void UpdateLoadingInterface( long, long, Rect *);
PlayAgainResult DoPlayAgain(bool allow_resume, bool allow_skip);
void DoNetSettings( void);
void DoHelpScreen( void);
void StartPauseIndicator(unsigned char*, unsigned char);
void StopPauseIndicator(unsigned char*);
void DrawInterfaceOneAtATime( void);
void DoOptionsInterface( void);
void SetOptionCheckboxes( unsigned long);
void DrawOptionVolumeLevel( Rect *, long);
bool DoKeyInterface( void);
void DrawKeyControlPicture( long);
bool BothCommandAndQ( void);
netResultType StartNetworkGameSetup( void);
netResultType HostAcceptClientInterface( void);
netResultType ClientNetworkGameSetup( void);
void DrawStringInInterfaceItem(long which_item, const sfz::StringPiece& string);
netResultType ClientWaitInterface( void);
netResultType HostBeginGame( void);
netResultType ClientBeginGame( void);
void BlackenWindow( void);
short GetClientListLength( void);
void GetClientListName(short, unsigned char*);
bool IsThisClientHilited( short, bool);
short GetInGameListLength( void);
void GetInGameListName(short, unsigned char*);
bool IsThisInGameHilited( short, bool);
long DoNetLevelInterface( void);
void DrawLevelNameInBox(unsigned char*, long, short, long);
bool DoMissionInterface( long);
long UpdateMissionBriefPoint( interfaceItemType *, long, long, coordPointType *, long, long,
        Rect *, Rect *, inlinePictType *);
void ShowObjectData( Point, short, Rect *);
void CreateObjectDataText(sfz::String* text, short id);
void CreateWeaponDataText(sfz::String* text, long whichWeapon, const sfz::StringPiece& weaponName);
void HandleOSEvent( EventRecord *);
void Replace_KeyCode_Strings_With_Actual_Key_Names(sfz::String* text, short resID, size_t padTo);

}  // namespace antares

#endif // ANTARES_PLAYER_INTERFACE_HPP_
