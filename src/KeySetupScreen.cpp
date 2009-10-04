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

// Key_Setup_Screen.c

#include "KeySetupScreen.hpp"

#include "AresGlobalType.hpp"
#include "AresMain.hpp"
#include "AresPreferences.hpp"
#include "ColorTranslation.hpp"
#include "Debug.hpp"
#include "Error.hpp"
#include "InterfaceHandling.hpp"
#include "KeyMapTranslation.hpp"
#include "OffscreenGWorld.hpp"
#include "Options.hpp"
#include "PlayerInterface.hpp"
#include "SoundFX.hpp"
#include "StringHandling.hpp"

#define kKeyScreenID                5030
#define kKeyCancelButton            0
#define kKeyDoneButton              1
#define kKeyOptionButton            2
#define kKeySubstituteCheckbox      23//22
#define kKeyIllustrationBox         32//31
#define kKeyIllustrationPictID      520

#define kConflictText               10//9

#define kPreTabItemNum              15//14

#define kTabBoxNum                  8//7

#define kShipTabID                  5031
#define kShipTabNum                 3
#define kFirstKey                   (kPreTabItemNum + 0)
#define kShipKeyNum                 8
#define kShipKeyIndexOffset         0

#define kCommandTabID               5032
#define kCommandTabNum              4
#define kCommandKeyNum              11
#define kCommandKeyIndexOffset      (kShipKeyIndexOffset + kShipKeyNum)

#define kShortcutTabID              5033
#define kShortcutTabNum             5
#define kShortcutKeyNum             9
#define kShortcutKeyIndexOffset     (kCommandKeyIndexOffset + kCommandKeyNum)

#define kUtilityTabID               5034
#define kUtilityTabNum              6
#define kUtilityKeyNum              6
#define kUtilityKeyIndexOffset      (kShortcutKeyIndexOffset + kShortcutKeyNum)


#define kHotKeyTabID                5035
#define kHotKeyTabNum               7
#define kHotKeyKeyNum               10
#define kHotKeyKeyIndexOffset       (kUtilityKeyIndexOffset + kUtilityKeyNum)


#define mPlayScreenSound            PlayVolumeSound( kComputerBeep3, kMediumLowVolume, kShortPersistence, kMustPlaySound)

struct tempKeyControlType {
    short   keyNum;
    Boolean conflicts;
    Boolean didConflict;
};

extern CWindowPtr       gTheWindow;

static void ConflictText_Update( tempKeyControlType *keyControls);
static long KeyControlIndex_GetTabNum( long whichKeyControl);
static void KeyControlButton_SetFlash( long whichKey, long currentTab,
    long currentKeyOffset, long currentButton, Boolean flash);

Boolean IsKeyReserved( KeyMap, Boolean);
void BlackenOffscreen( void);
void Pause( long);

static void ConflictText_Update( tempKeyControlType *keyControls)
{
    long    i, j;
    Str255  textString, tString;
    Boolean conflictFound = false;

    textString[0] = 0;
    for ( i = 0; i < kKeyExtendedControlNum; i++)
    {
        keyControls[i].didConflict = keyControls[i].conflicts;
        keyControls[i].conflicts = false;
    }
    for ( i = 0; i < kKeyExtendedControlNum; i++)
    {
        for ( j = i + 1; j < kKeyExtendedControlNum; j++)
        {
            if ( keyControls[i].keyNum == keyControls[j].keyNum)
            {
                keyControls[i].conflicts = true;
                keyControls[j].conflicts = true;
                if ( !conflictFound)
                {
                    GetIndString( textString, 2009,
                        KeyControlIndex_GetTabNum( i) + 1);
                    ConcatenatePString( textString, "\p: ");
                    GetIndString( tString, 2005, i + 1);
                    ConcatenatePString( textString, tString);
                    ConcatenatePString( textString, "\p conflicts with ");
                    GetIndString( tString, 2009,
                        KeyControlIndex_GetTabNum( j) + 1);
                    ConcatenatePString( textString, tString);
                    ConcatenatePString( textString, "\p: ");
                    GetIndString( tString, 2005, j + 1);
                    ConcatenatePString( textString, tString);
                    DrawStringInInterfaceItem( kConflictText, textString);
                    conflictFound = true;
                }
            }
        }
    }
    if ( !conflictFound) DrawStringInInterfaceItem( kConflictText,
        "\pNo Key Conflicts");
}

static long KeyControlIndex_GetTabNum( long whichKeyControl)
{
    if ( whichKeyControl < kCommandKeyIndexOffset) return 0;
    if ( whichKeyControl < kShortcutKeyIndexOffset) return 1;
    if ( whichKeyControl < kUtilityKeyIndexOffset) return 2;
    if ( whichKeyControl < kHotKeyKeyIndexOffset) return 3;
    return 4;
}

static void KeyControlButton_SetFlash( long whichKey, long currentTab,
    long currentKeyOffset, long currentButton, Boolean flash)
{
    long                    whichTab =
                                    KeyControlIndex_GetTabNum( whichKey),
                                    whichButton = -1;
    interfaceItemType       *anItem;
    Boolean                 hilite = false;

    if ( whichTab != currentTab) whichButton = kShipTabNum + whichTab;
    else
    {
        if ( currentButton == (whichKey - currentKeyOffset))
        {
            flash = false;
            hilite = true;
        }
        whichButton = whichKey - currentKeyOffset + kFirstKey;
    }

    anItem = GetAnyInterfaceItemPtr( whichButton);
    if ( anItem == nil) return;

    if ( !flash)
    {
        anItem->color = AQUA;
//      SetStatusOfAnyInterfaceItem( whichButton, (hilite)?(kIH_Hilite):
//          (kActive), true);
        DrawAnyInterfaceItemOffToOn( anItem);
    } else
    {
        anItem->color = GOLD;
//      SetStatusOfAnyInterfaceItem( whichButton, kIH_Hilite, true);
        DrawAnyInterfaceItemOffToOn( anItem);
    }

}

Boolean Key_Setup_Screen_Do( void)

{
    Point                   where;
    int                     error;
    short                   whichItem, i, whichKeyButton = -1, keyNum = 0,
                            currentKey = 0, checkKey, currentKeyOffset = -1,
                            currentMaxKey = -1, tabItemNum, whichTab;
    Boolean                 done = FALSE, result = TRUE, cancel = FALSE,
                            flashOn = false;
    EventRecord             theEvent;
    KeyMap                  keyMap;
    Preferences     *prefsData = nil;
    unsigned long           options = globals()->gOptions;
    tempKeyControlType      *tempKeyControls;
    long                    lastFlashTime = 0;
    interfaceItemType       *anItem;

    BlackenOffscreen();

    FlushEvents(everyEvent, 0);
    tempKeyControls = new tempKeyControlType[kKeyExtendedControlNum];
    if ( tempKeyControls == nil)
    {
        SysBeep(20);
        return false;
    }

    for ( i = 0; i < kKeyExtendedControlNum; i++)
    {
        tempKeyControls[i].keyNum = GetKeyNumFromKeyMap( globals()->gKeyControl[i]);
        tempKeyControls[i].conflicts = false;
    }

    error = OpenInterface( kKeyScreenID);
    if ( error == kNoError)
    {
        prefsData = globals()->gPreferencesData.get();

//      SwitchAnyRadioOrCheckbox( kKeySubstituteCheckbox,
//          ((options & kOptionSubstituteFKeys) ? (true):(false)));

        tabItemNum = AppendInterface( kShipTabID, kTabBoxNum, true);
        whichTab = kShipTabNum;
        currentMaxKey = kShipKeyNum;
        currentKeyOffset = kShipKeyIndexOffset;

        for ( i = 0; i < currentMaxKey; i++)
        {
            SetButtonKeyNum( kFirstKey + i,
                tempKeyControls[i + currentKeyOffset].keyNum);
        }

        DrawInterfaceOneAtATime();
        ConflictText_Update( tempKeyControls);

        DrawStringInInterfaceItem( kConflictText, nil);

        while ( !done)
        {
            InterfaceIdle();
            if (( AnyEvent()) && ( !( globals()->gOptions & kOptionInBackground)))
            {
                GetKeys( keyMap);
                keyNum = GetKeyNumFromKeyMap( keyMap);
                if ( currentKey > 0) keyNum = currentKey;

                // make sure it's not a reserved key
                if ( IsKeyReserved( keyMap, ((options & kOptionSubstituteFKeys) ?
                    ( true):(false))))
                {
                    PlayVolumeSound( kWarningTone, kMediumLowVolume,
                        kShortPersistence, kMustPlaySound);
                    keyNum = -1;
                }

                // make sure it's not a key that's already in use
                checkKey = 0;

                if (( keyNum > 0) && (whichKeyButton >= 0) &&
                        (whichKeyButton < currentMaxKey))
                {
                    mPlayScreenSound;
                    SetButtonKeyNum( whichKeyButton + kFirstKey, keyNum);
                    tempKeyControls[whichKeyButton + currentKeyOffset].keyNum =
                        keyNum;
                    SetStatusOfAnyInterfaceItem( whichKeyButton + kFirstKey,
                        kIH_Hilite, TRUE);
                    do
                    {
                        GetKeys( keyMap);
                        currentKey = GetKeyNumFromKeyMap( keyMap);
                    } while ( currentKey > 0);

                    SetStatusOfAnyInterfaceItem( whichKeyButton + kFirstKey,
                        kActive, TRUE);
                    whichKeyButton++;
                    if ( whichKeyButton >= (currentMaxKey))
                        whichKeyButton = 0;
                    SetStatusOfAnyInterfaceItem( whichKeyButton + kFirstKey,
                        kIH_Hilite, TRUE);
                    ConflictText_Update( tempKeyControls);
                    for ( i = 0; i < kKeyExtendedControlNum; i++)
                    {
                        if (( tempKeyControls[i].didConflict) &&
                            (!tempKeyControls[i].conflicts))
                        {
                            KeyControlButton_SetFlash( i, whichTab -
                                kShipTabNum, currentKeyOffset, whichKeyButton,
                                false);
                            tempKeyControls[i].didConflict = false;
                        }
                    }
                }
                keyNum = currentKey = 0;
            }
            WaitNextEvent (everyEvent, &theEvent, 3, nil);
            {
                whichItem = -1;
                switch ( theEvent.what )
                {
                    case nullEvent:
                        if ( (TickCount() - lastFlashTime) > 12)
                        {
                            interfaceItemStatusType     doneButtonStatus = kActive;
                            for ( i = 0; i < kKeyExtendedControlNum; i++)
                            {


                                if ( tempKeyControls[i].conflicts)
                                {
                                    doneButtonStatus = kDimmed;
                                    KeyControlButton_SetFlash( i, whichTab -
                                        kShipTabNum, currentKeyOffset, whichKeyButton,
                                        flashOn);
                                }
                            }
                            if ( flashOn) flashOn = false;
                            else flashOn = true;
                            SetStatusOfAnyInterfaceItem( kKeyDoneButton, doneButtonStatus,
                                true);
                            SetStatusOfAnyInterfaceItem( kKeyOptionButton, doneButtonStatus,
                                true);
                            lastFlashTime = TickCount();
                        }
                        InterfaceIdle();
                        if ( globals()->returnToMain)
                        {
                            done = true;
                            result = false;
                        }
                        break;
                    case osEvt:
//                      HandleOSEvent( &theEvent);
                        break;

                    case mouseDown:
                        where = theEvent.where;
                        GlobalToLocal( &where);
                        HandleMouseDown( &theEvent);
                        whichItem = InterfaceMouseDown( where);
                        break;
                    case mouseUp:
                        break;
                    case keyDown:
                    case autoKey:
//                      whichChar = theEvent.message & charCodeMask;
//                      whichItem = InterfaceKeyDown( theEvent.message);
                        break;
                }

                if (( whichItem >= kFirstKey) &&
                    (whichItem < (kFirstKey + currentMaxKey)))
                {
                    if (( whichKeyButton >= 0) &&  (whichKeyButton < currentMaxKey))
                        SetStatusOfAnyInterfaceItem(
                            whichKeyButton + kFirstKey, kActive, TRUE);
                    SetStatusOfAnyInterfaceItem( whichItem,
                            kIH_Hilite, TRUE);
                    whichKeyButton = whichItem - kFirstKey;
//                  DrawKeyControlPicture( whichKeyButton);
                }

                switch ( whichItem)
                {
                    case kKeyCancelButton:
                        cancel = TRUE;
                        done = TRUE;
                        break;
                    case kKeyDoneButton:
                        done = TRUE;
                        break;
                    case kKeyOptionButton:
                        result = FALSE;
                        done = TRUE;
                        break;

                    case kShipTabNum:
                    case kCommandTabNum:
                    case kShortcutTabNum:
                    case kUtilityTabNum:
                    case kHotKeyTabNum:
                        ShortenInterface( tabItemNum);
                        anItem = GetAnyInterfaceItemPtr( whichTab);
                        if ( anItem != nil)
                        {
                            anItem->color = AQUA;
                        }
                        SwitchAnyRadioOrCheckbox( whichTab, false);
                        DrawAnyInterfaceItemOffToOn( GetAnyInterfaceItemPtr( whichTab));
                        whichTab = whichItem;

                        anItem = GetAnyInterfaceItemPtr( whichTab);
                        if ( anItem != nil)
                        {
                            anItem->color = AQUA;
                        }
                        SwitchAnyRadioOrCheckbox( whichTab, true);
                        DrawAnyInterfaceItemOffToOn( GetAnyInterfaceItemPtr( whichTab));

                        whichKeyButton = -1;
                        keyNum = 0;
                        currentKey = 0;
                        switch( whichTab)
                        {
                            case kShipTabNum:
                                tabItemNum = AppendInterface( kShipTabID,
                                    kTabBoxNum, true);
                                currentMaxKey = kShipKeyNum;
                                currentKeyOffset = kShipKeyIndexOffset;
                                break;

                            case kCommandTabNum:
                                tabItemNum = AppendInterface( kCommandTabID,
                                    kTabBoxNum, true);
                                currentMaxKey = kCommandKeyNum;
                                currentKeyOffset = kCommandKeyIndexOffset;
                                break;

                            case kShortcutTabNum:
                                tabItemNum = AppendInterface( kShortcutTabID,
                                    kTabBoxNum, true);
                                currentMaxKey = kShortcutKeyNum;
                                currentKeyOffset = kShortcutKeyIndexOffset;
                                break;

                            case kUtilityTabNum:
                                tabItemNum = AppendInterface( kUtilityTabID,
                                    kTabBoxNum, true);
                                currentMaxKey = kUtilityKeyNum;
                                currentKeyOffset = kUtilityKeyIndexOffset;
                                break;

                            case kHotKeyTabNum:
                                tabItemNum = AppendInterface( kHotKeyTabID,
                                    kTabBoxNum, true);
                                currentMaxKey = kHotKeyKeyNum;
                                currentKeyOffset = kHotKeyKeyIndexOffset;
                                break;
                        }

                        for ( i = 0; i < currentMaxKey; i++)
                        {
                            SetButtonKeyNum( kFirstKey + i,
                                tempKeyControls[i + currentKeyOffset].keyNum);
                        }

                        DrawInterfaceRange( kPreTabItemNum,
                            kPreTabItemNum + tabItemNum, kTabBoxNum);
                        break;
                }

            }
        }
        if ( !cancel)
        {
            for ( i = 0; i < kKeyExtendedControlNum; i++)
            {
                GetKeyMapFromKeyNum( tempKeyControls[i].keyNum,
                    globals()->gKeyControl[i]);
            }
            prefsData->options = ( prefsData->options & ~kOptionSubstituteFKeys) |
                                ( options & kOptionSubstituteFKeys);
            SaveKeyControlPreferences();
            SaveAllPreferences();
        }
        CloseInterface();
    }
    WriteDebugLine("\pRESULT:");
    WriteDebugLong( result);
    delete[] tempKeyControls;
    return( result);
}

