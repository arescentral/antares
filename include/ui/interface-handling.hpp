// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2011 Ares Central
//
// This file is part of Antares, a tactical space combat game.
//
// Antares is free software: you can redistribute it and/or modify it
// under the terms of the Lesser GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Antares is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program.  If not, see
// <http://www.gnu.org/licenses/>.

#ifndef ANTARES_UI_INTERFACE_HANDLING_HPP_
#define ANTARES_UI_INTERFACE_HANDLING_HPP_

#include <vector>
#include <sfz/sfz.hpp>

#include "data/interface.hpp"
#include "drawing/interface.hpp"

namespace antares {

struct Scenario;

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

void InterfaceHandlingInit( void);
void InterfaceHandlingCleanup( void);
void OpenInterface( short);
long AppendInterface( short, long, bool);
void ShortenInterface( long);
void CloseInterface( void);
void DrawEntireInterface( void);
void DrawInterfaceRange( long, long, long);
void DrawAllItemsOfKind( interfaceKindType, bool, bool, bool);
void OffsetItemRange( long, long, long, long);
void OffsetAllItems( long, long);
void CenterItemRangeInRect( Rect *, long, long);
void CenterAllItemsInRect( Rect *);
void DrawAnyInterfaceItemOffToOn(const interfaceItemType& item);
void InvalidateInterfaceFunctions( void);
short PtInInterfaceItem( Point);
short InterfaceMouseDown( Point);
short InterfaceKeyDown( long);
bool InterfaceButtonHit(interfaceItemType* button);
bool InterfaceCheckboxHit(interfaceItemType* button);
bool InterfaceRadioButtonHit(interfaceItemType* button);
bool InterfaceTabBoxButtonHit(interfaceItemType* button);
void InterfaceListRectHit(const interfaceItemType&, Point);
void DrawStringInInterfaceContent(short, unsigned char*);
interfaceItemType *GetAnyInterfaceItemPtr( long);
void SetStatusOfAnyInterfaceItem( short, interfaceItemStatusType, bool);
void SwitchAnyRadioOrCheckbox( short, bool);
bool GetAnyRadioOrCheckboxOn( short);
void RefreshInterfaceItem( short);
void RefreshInterfaceListEntry( short, short);
void SetButtonKeyNum( short, short);
short GetButtonKeyNum( short);

void DoLoadingInterface(Rect* contentRect, sfz::StringSlice level_name);
void UpdateLoadingInterface( long, long, Rect *);
void DoNetSettings( void);
bool BothCommandAndQ( void);
netResultType StartNetworkGameSetup( void);
netResultType HostAcceptClientInterface( void);
netResultType ClientNetworkGameSetup( void);
netResultType ClientWaitInterface( void);
netResultType HostBeginGame( void);
netResultType ClientBeginGame( void);
short GetClientListLength( void);
void GetClientListName(short, unsigned char*);
bool IsThisClientHilited( short, bool);
short GetInGameListLength( void);
void GetInGameListName(short, unsigned char*);
long DoNetLevelInterface( void);
void UpdateMissionBriefPoint(
        interfaceItemType *dataItem, long whichBriefPoint, const Scenario* scenario,
        coordPointType *corner, long scale, Rect *bounds, std::vector<inlinePictType>& inlinePict,
        PixMap* pix);
void ShowObjectData( Point, short, Rect *);
void CreateObjectDataText(sfz::String* text, short id);
void CreateWeaponDataText(sfz::String* text, long whichWeapon, const sfz::StringSlice& weaponName);
void Replace_KeyCode_Strings_With_Actual_Key_Names(sfz::String* text, short resID, size_t padTo);

}  // namespace antares

#endif // ANTARES_UI_INTERFACE_HANDLING_HPP_
