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

// Player Ship.c

#include "PlayerShip.hpp"

//#include <math routines.h>

#include "Admiral.hpp"
#include "AresCheat.hpp"
#include "AresGlobalType.hpp"
#include "AresNetworkSprocket.hpp"
#include "AresPreferences.hpp"
#include "ColorTranslation.hpp"
#include "ConditionalMacros.h"
#include "Debug.hpp"
#include "DirectText.hpp"
#include "Error.hpp"
#include "GXMath.h"
#include "KeyCodes.hpp"
#include "KeyMapTranslation.hpp"        // major hack for testing
#include "MathMacros.hpp"
#include "MessageScreen.hpp"
#include "Minicomputer.hpp"
#include "NonPlayerShip.hpp"
#include "Options.hpp"
#include "Randomize.hpp"
#include "Resources.h"
#include "Rotation.hpp"
#include "ScenarioMaker.hpp"
#include "ScreenLabel.hpp"
#include "ScrollStars.hpp"
#include "ShotsBeamsExplosions.hpp"
#include "SoundFX.hpp"
#include "SpaceObject.hpp"
#include "SpaceObjectHandling.hpp"
#include "StringHandling.hpp"
#include "ToolUtils.h"
#include "UniverseUnit.hpp"

#define kSendMessageVOffset     20


#define kCursorBoundsSize   16          // should be same in instruments.c

extern aresGlobalType   *gAresGlobal;
//extern KeyMap     gAresGlobal->gKeyControl[];
extern spaceObjectType**    gSpaceObjectData;
extern baseObjectType**     gBaseObjectData;
extern long         /*gAresGlobal->gGameTime,*/ gRandomSeed,
                    //gAresGlobal->gPlayerAdmiralNumber, gAresGlobal->gScenarioWinner,
                    gNetLatency, gRootObjectNumber;
//extern unsigned long gAresGlobal->gGameOver, gAresGlobal->gOptions;
extern smallFixedType** gRotTable;  // for getting weapon position
//extern short      gAresGlobal->gMouseActive;
extern scenarioType *gThisScenario; // for setting debrief message when we run out of ships
extern coordPointType           gGlobalCorner;
extern long         gAbsoluteScale; // HACK FOR TESTING
extern directTextType   *gDirectText;
extern long         CLIP_LEFT, CLIP_TOP, CLIP_RIGHT, CLIP_BOTTOM/*, gAresGlobal->gTrueClipBottom*/;
extern spaceObjectType  *gRootObject;

//KeyMap                gAresGlobal->gLastKeyMap;
//unsigned long     gAresGlobal->gLastKeys, gAresGlobal->gTheseKeys;
//long              gAresGlobal->gPlayerShipNumber = 0, gAresGlobal->gSelectionLabel = -1, gAresGlobal->gDestKeyTime = 0,
//                  gAresGlobal->gZoomMode = kNearestFoeZoom, gAresGlobal->gDestinationLabel = -1,
//                  gAresGlobal->gAlarmCount = -1, gAresGlobal->gSendMessageLabel = -1;
//Boolean               gAresGlobal->gDemoZoomOverride = false;

long HotKey_GetFromObject( spaceObjectType *object);
StringPtr HotKey_AppendString( spaceObjectType *object, StringPtr s);
void Update_LabelStrings_ForHotKeyChange( void);

void StartPlayerShip( long owner, short type)

{
#pragma unused( owner, type)
}

void ResetPlayerShip( long which)

{
    spaceObjectType *theShip = *gSpaceObjectData + which;
    anyCharType     nilLabel = 0;
    long            h;

    WriteDebugLine("\pPLAYER:");
    WriteDebugLong( which);
    gAresGlobal->gPlayerShipNumber = which;
    gAresGlobal->gSelectionLabel = AddScreenLabel( 0, 0, 0, 10, &nilLabel, nil, TRUE, YELLOW);
    gAresGlobal->gDestinationLabel = AddScreenLabel( 0, 0, 0, -20, &nilLabel, nil, TRUE, SKY_BLUE);
    gAresGlobal->gSendMessageLabel = AddScreenLabel( 200, 200, 0, 30, &nilLabel, nil, false, GREEN);
    ResetScrollStars( gAresGlobal->gPlayerShipNumber);
    gAresGlobal->gAlarmCount = -1;
    gAresGlobal->gDemoZoomOverride = false;
    gAresGlobal->gLastKeys = gAresGlobal->gTheseKeys = 0;
    gAresGlobal->gAutoPilotOff = true;
    gAresGlobal->keyMask = 0;
    for ( h = 0; h < 4; h++)
    {
#if TARGET_OS_MAC
        gAresGlobal->gLastKeyMap[h] = 0;
        gAresGlobal->gLastMessageKeyMap[h] = 0;
#else
        gAresGlobal->gLastKeyMap[h].bigEndianValue = 0;
        gAresGlobal->gLastMessageKeyMap[h].bigEndianValue = 0;
#endif TARGET_OS_MAC
    }
    gAresGlobal->gZoomMode = kNearestFoeZoom;
    gAresGlobal->gKeyMapBufferTop = gAresGlobal->gKeyMapBufferBottom = 0;

    for ( h = 0; h < kHotKeyNum; h++)
    {
        gAresGlobal->hotKey[h].objectNum = -1;
        gAresGlobal->hotKey[h].objectID = -1;
    }
    gAresGlobal->hotKeyDownTime = 0;
    gAresGlobal->lastHotKey = -1;
    gAresGlobal->destKeyUsedForSelection = false;
    gAresGlobal->hotKey_target = false;
}

Boolean PlayerShipGetKeys( long timePass, unsigned long theKeys,
    Boolean *enterMessage)
{
    KeyMap          keyMap, *bufMap;
    short           a, b, h, friendOrFoe;
    spaceObjectType *theShip = nil, *selectShip = nil;
    baseObjectType  *baseObject = nil, *weaponObject = nil;
    long            selectShipNum;
    unsigned long   distance, difference, dcalc, attributes, nonattributes;
    Boolean         everPaused = FALSE, newKeys = false;
    UnsignedWide    hugeDistance;
    anyCharType     *message;
    unsigned char   *getwidchar, *getwidwid;
    long            width, height, strlen, shortcutKey = -1;
    Str255          s;

    if ( gAresGlobal->gPlayerShipNumber < 0) return ( everPaused);

    GetKeys( keyMap);
    for ( h = 0; h < 4; h++)
#if TARGET_OS_MAC
        if ( gAresGlobal->gLastKeyMap[h] != keyMap[h]) newKeys = true;
#else
        if ( gAresGlobal->gLastKeyMap[h].bigEndianValue !=
            keyMap[h].bigEndianValue) newKeys = true;
#endif TARGET_OS_MAC

    gAresGlobal->gLastKeys = gAresGlobal->gTheseKeys;

    if ( theKeys == 0xffffffff)
    {
        gAresGlobal->gTheseKeys = 0;

//#ifndef kNonPlayableDemo
        for ( b = 0; b < kKeyControlNum; b++)
        {
            for ( a = 0; a < 4; a++)
            {
#if TARGET_OS_MAC
                if ( keyMap[a] & gAresGlobal->gKeyControl[b][a])
#else
                if ( keyMap[a].bigEndianValue &
                    gAresGlobal->gKeyControl[b][a].bigEndianValue)
#endif TARGET_OS_MAC
                {
                    gAresGlobal->gTheseKeys |= (0x01 << b) & ~gAresGlobal->keyMask;
                }
            }
        }
//#endif
    } else
    {
        gAresGlobal->gTheseKeys = theKeys;
        distance = 0;
        for ( b = 0; b < kKeyControlNum; b++)
        {
            for ( a = 0; a < 4; a++)
            {
#if TARGET_OS_MAC
                if ( keyMap[a] & gAresGlobal->gKeyControl[b][a])
#else
                if ( keyMap[a].bigEndianValue &
                    gAresGlobal->gKeyControl[b][a].bigEndianValue)
#endif TARGET_OS_MAC
                {
                    distance |= 0x01 << b;
                }
            }
        }
        if ( distance & (kZoomOutKey | kZoomInKey))
            gAresGlobal->gDemoZoomOverride = true;
        if ( gAresGlobal->gDemoZoomOverride)
        {
            gAresGlobal->gTheseKeys &= ~(kZoomOutKey | kZoomInKey);
            gAresGlobal->gTheseKeys |= distance & (kZoomOutKey | kZoomInKey);
        }
    }

    if ( *enterMessage) gAresGlobal->gTheseKeys = 0;
    while (( gAresGlobal->gKeyMapBufferBottom != gAresGlobal->gKeyMapBufferTop))
        // && ( gAresGlobal->gOptions & kOptionNetworkOn))
    {
        bufMap = reinterpret_cast<KeyMap*>(gAresGlobal->gKeyMapBuffer) +
            gAresGlobal->gKeyMapBufferBottom;
        gAresGlobal->gKeyMapBufferBottom++;
        if ( gAresGlobal->gKeyMapBufferBottom >= kKeyMapBufferNum)
            gAresGlobal->gKeyMapBufferBottom = 0;
        if (( *enterMessage))// && ( gAresGlobal->gOptions & kOptionNetworkOn))
        {
            message = GetScreenLabelStringPtr( gAresGlobal->gSendMessageLabel);
            if ( mGetAnyCharPStringLength( message) == 0)
            {
                CopyAnyCharPString( message, "\p<>");
            }
//          gAresGlobal->gTheseKeys = 0;
            if (( mReturnKey( *bufMap)) && (!AnyKeyButThisOne( *bufMap, 1, 28)))
            {
                *enterMessage = false;
                CutCharsFromAnyCharPString( message, mGetAnyCharPStringLength( message) - 1, 1);
                CutCharsFromAnyCharPString( message, 0, 1);
                a = GetCheatNumFromString( message);
                if ( a > 0)
                {
                    if ( gAresGlobal->gOptions & kOptionNetworkOn)
                    {
#if NETSPROCKET_AVAILABLE
                        SendCheatMessage( a);
#endif NETSPROCKET_AVAILABLE
                    } else
                        ExecuteCheat( a, gAresGlobal->gPlayerAdmiralNumber);
                } else if ( message[0] > 0)
                {
                    if ( gAresGlobal->gActiveCheats[gAresGlobal->gPlayerAdmiralNumber] & kNameObjectBit)
                    {
                        SetAdmiralBuildAtName( gAresGlobal->gPlayerAdmiralNumber, message);
                        gAresGlobal->gActiveCheats[gAresGlobal->gPlayerAdmiralNumber] &= ~kNameObjectBit;
                    }
#if NETSPROCKET_AVAILABLE
                    SendInGameTextMessage( (Ptr)(message + 1), mGetAnyCharPStringLength( message));
#endif NETSPROCKET_AVAILABLE
                }
                message[0] = 0;
                SetScreenLabelPosition( gAresGlobal->gSendMessageLabel, CLIP_LEFT +
                    (((CLIP_RIGHT - CLIP_LEFT) / 2)), CLIP_TOP +
                    (((gAresGlobal->gTrueClipBottom - CLIP_TOP) / 2)) + kSendMessageVOffset);
                RecalcScreenLabelSize( gAresGlobal->gSendMessageLabel);
            } else
            {
                message = GetScreenLabelStringPtr( gAresGlobal->gSendMessageLabel);
                if (( mDeleteKey( *bufMap)) || (mLeftArrowKey(*bufMap)))
                {
                    if ( mGetAnyCharPStringLength( message) > 2)
                    {
                        if ( mCommandKey( *bufMap)) // delete whole message
                        {
                            CutCharsFromAnyCharPString( message,
                                1, mGetAnyCharPStringLength( message) - 2);
                        } else
                        {
                            CutCharsFromAnyCharPString( message,
                                mGetAnyCharPStringLength( message) - 2, 1);
                        }
                    }
                } else
                {
                    if ( message[0] < 120)
                    {
                        s[0] = 1;
                        s[1] = GetAsciiFromKeyMap( *bufMap,
                            gAresGlobal->gLastMessageKeyMap);
                        if (s[1])
                        {
                            InsertAnyCharPStringInPString( message, s,
                                mGetAnyCharPStringLength( message) - 1);
//                          PlayVolumeSound(  kTeletype, kMediumLowVolume, kShortPersistence, kLowPrioritySound);
                        }
                    }
                }
                mGetDirectStringDimensions( message, width, height, strlen, getwidchar, getwidwid);
                strlen = CLIP_LEFT + (((CLIP_RIGHT - CLIP_LEFT) / 2) - ( width / 2));
                if (( strlen + width) > (CLIP_RIGHT))
                {
                    strlen -= (strlen + width) - (CLIP_RIGHT);
                }
                RecalcScreenLabelSize( gAresGlobal->gSendMessageLabel);
                SetScreenLabelPosition( gAresGlobal->gSendMessageLabel, strlen, CLIP_TOP +
                    (((gAresGlobal->gTrueClipBottom - CLIP_TOP) / 2) + kSendMessageVOffset));
            }
        } else //if  ( gAresGlobal->gOptions & kOptionNetworkOn)
        {
            if ( (mReturnKey( *bufMap)) && ( !(gAresGlobal->keyMask & kReturnKeyMask)))
                *enterMessage = true;
        }
        for ( h = 0; h < 4; h++)
            gAresGlobal->gLastMessageKeyMap[h] = (*bufMap)[h];
    }
/*  if (( *enterMessage) && ( gAresGlobal->gOptions & kOptionNetworkOn))
    {
        message = GetScreenLabelStringPtr( gAresGlobal->gSendMessageLabel);
        if ( mGetAnyCharPStringLength( message) == 0)
        {
            CopyAnyCharPString( message, (anyCharType *)"\p<>");
        }
        gAresGlobal->gTheseKeys = 0;
        if (( mReturnKey( keyMap)) && ((!mReturnKey( gAresGlobal->gLastKeyMap))))
        {
            *enterMessage = false;
            CutCharsFromAnyCharPString( message, mGetAnyCharPStringLength( message) - 1, 1);
            CutCharsFromAnyCharPString( message, 0, 1);
            a = GetCheatNumFromString( message);
            if ( a > 0)
            {
                SendCheatMessage( a);
            } else if ( message[0] > 0)
            {
                if ( gAresGlobal->gActiveCheats[gAresGlobal->gPlayerAdmiralNumber] & kNameObjectBit)
                {
                    SetAdmiralBuildAtName( gAresGlobal->gPlayerAdmiralNumber, message);
                    gAresGlobal->gActiveCheats[gAresGlobal->gPlayerAdmiralNumber] &= ~kNameObjectBit;
                }
                SendInGameTextMessage( (Ptr)(message + 1), mGetAnyCharPStringLength( message));
            }
            message[0] = 0;
            SetScreenLabelPosition( gAresGlobal->gSendMessageLabel, CLIP_LEFT +
                (((CLIP_RIGHT - CLIP_LEFT) / 2)), CLIP_TOP +
                (((gAresGlobal->gTrueClipBottom - CLIP_TOP) / 2)) + kSendMessageVOffset);
            RecalcScreenLabelSize( gAresGlobal->gSendMessageLabel);
        } else if ( newKeys)
        {
            message = GetScreenLabelStringPtr( gAresGlobal->gSendMessageLabel);
            if (( mDeleteKey( keyMap)) || (mLeftArrowKey(keyMap)))
            {
                if ( mGetAnyCharPStringLength( message) > 2)
                {
                    if ( mCommandKey( keyMap)) // delete whole message
                    {
                        CutCharsFromAnyCharPString( message,
                            1, mGetAnyCharPStringLength( message) - 2);
                    } else
                    {
                        CutCharsFromAnyCharPString( message,
                            mGetAnyCharPStringLength( message) - 2, 1);
                    }
                }
            } else
            {
                if ( message[0] < 120)
                {
                    s[0] = 1;
                    s[1] = GetAsciiFromKeyMap( keyMap, gAresGlobal->gLastKeyMap);
                    if (s[1])
                    {
                        InsertAnyCharPStringInPString( message, (anyCharType *)s,
                            mGetAnyCharPStringLength( message) - 1);
                        PlayVolumeSound(  kTeletype, kMediumLowVolume, kShortPersistence, kLowPrioritySound);
                    }
                }
            }
            mGetDirectStringDimensions( message, width, height, strlen, getwidchar, getwidwid)
            strlen = CLIP_LEFT + (((CLIP_RIGHT - CLIP_LEFT) / 2) - ( width / 2));
            if (( strlen + width) > (CLIP_RIGHT))
            {
                strlen -= (strlen + width) - (CLIP_RIGHT);
            }
            RecalcScreenLabelSize( gAresGlobal->gSendMessageLabel);
            SetScreenLabelPosition( gAresGlobal->gSendMessageLabel, strlen, CLIP_TOP +
                (((gAresGlobal->gTrueClipBottom - CLIP_TOP) / 2) + kSendMessageVOffset));
        }
    } else if  ( gAresGlobal->gOptions & kOptionNetworkOn)
    {
        if (( mReturnKey( keyMap)) && ((!mReturnKey( gAresGlobal->gLastKeyMap)))) *enterMessage = true;
    }
*/
    // TERRIBLE HACK:
    //  this implements the often requested feature of having a shortcut for
    //  transfering control.

    a = gAresGlobal->gZoomMode;

    if (( gAresGlobal->gTheseKeys & kZoomOutKey) && ( !(gAresGlobal->gLastKeys & kZoomOutKey)))
    {
        gAresGlobal->gZoomMode++;
        if ( gAresGlobal->gZoomMode == kZoomLevelNum)
        {
             gAresGlobal->gZoomMode = kZoomLevelNum - 1;
        }
    }
    if (( gAresGlobal->gTheseKeys & kZoomInKey) && ( !(gAresGlobal->gLastKeys & kZoomInKey)))
    {
        gAresGlobal->gZoomMode--;
        if ( gAresGlobal->gZoomMode < 0)
        {
            gAresGlobal->gZoomMode = 0;
        }
    }

//  if ((( gAresGlobal->gOptions & kOptionSubstituteFKeys) ?
//      ((!*enterMessage) && ( mNOFScale221Key( keyMap))):( mScale221Key( keyMap))))

/*
    if ( mScale221Key( keyMap))
    {
        gAresGlobal->gZoomMode = 1;
    } else if ( mScale121Key( keyMap))
    {
        gAresGlobal->gZoomMode = 2;
    } else if ( mScale122Key( keyMap))
    {
        gAresGlobal->gZoomMode = 3;
    } else if ( mScale124Key( keyMap))
    {
        gAresGlobal->gZoomMode = 4;
    } else if ( mScaleHostileKey( keyMap))
    {
        gAresGlobal->gZoomMode = 5;
    } else if ( mScaleObjectKey( keyMap))
    {
        gAresGlobal->gZoomMode = 6;
    } else if ( mScaleAllKey( keyMap))
    {
        gAresGlobal->gZoomMode = 7;
    }
*/
//  if ((( gAresGlobal->gOptions & kOptionSubstituteFKeys) ?
//      ((!*enterMessage) && ( mNOFScale221Key( keyMap))):( mScale221Key( keyMap))))
//  {
//      gAresGlobal->gZoomMode = 1;
//  }

    if ( !*enterMessage)
    {
        if ((!(gAresGlobal->gOptions & kOptionSubstituteFKeys)) &&
            ( mTransferKey( keyMap)) && ( !(mTransferKey( gAresGlobal->gLastKeyMap))))
        {
            if (!(gAresGlobal->gOptions & kOptionNetworkOn))
            {
                MiniComputerExecute( 3,
                    1, gAresGlobal->gPlayerAdmiralNumber);
            } else
            {
    #if NETSPROCKET_AVAILABLE
                SendMenuMessage( gAresGlobal->gGameTime + gNetLatency,
                    3,  // the special screen
                    1   // kSpecialMiniTransfer
                    );
    #endif NETSPROCKET_AVAILABLE
            }
        }
        if (( ( mScale121Key( keyMap))))
        {
            gAresGlobal->gZoomMode = 1;
        }

        if ((  ( mScale122Key( keyMap))))
        {
            gAresGlobal->gZoomMode = 2;
        }

        if ((  ( mScale124Key( keyMap))))
        {
            gAresGlobal->gZoomMode = 3;
        }

        if ((  ( mScale1216Key( keyMap))))
        {
            gAresGlobal->gZoomMode = 4;
        }

        if ((  ( mScaleHostileKey( keyMap))))
        {
            gAresGlobal->gZoomMode = 5;
        }

        if ((  ( mScaleObjectKey( keyMap))))
        {
            gAresGlobal->gZoomMode = 6;
        }

        if ((  ( mScaleAllKey( keyMap))))
        {
            gAresGlobal->gZoomMode = 7;
        }
    }

    if ( gAresGlobal->gZoomMode != a)
    {
        PlayVolumeSound(  kComputerBeep3, kMediumVolume, kMediumPersistence, kLowPrioritySound);
        GetIndString( s, kMessageStringID, gAresGlobal->gZoomMode + kZoomStringOffset);
        SetStatusString( s, TRUE, kStatusLabelColor);
    }

    theShip = *gSpaceObjectData + gAresGlobal->gPlayerShipNumber;
//  theShip->attributes &= ~kIsHumanControlled;
    if ( !theShip->active) return ( everPaused);

    if ( theShip->health < ( theShip->baseType->health >> 2L))
    {
         if ( gAresGlobal->gAlarmCount < 0)
         {
            PlayVolumeSound( kKlaxon, kMaxSoundVolume, kLongPersistence, kMustPlaySound);
            gAresGlobal->gAlarmCount = 0;
            SetStatusString( "\pWARNING: Shields Low", TRUE, kStatusWarnColor);
         } else
         {
            gAresGlobal->gAlarmCount += timePass;
            if ( gAresGlobal->gAlarmCount > 125)
            {
                PlayVolumeSound( kKlaxon, kMediumVolume, kMediumLongPersistence, kPrioritySound);
                gAresGlobal->gAlarmCount = 0;
                SetStatusString( "\pWARNING: Shields Low", TRUE, kStatusWarnColor);
            }
        }
    } else gAresGlobal->gAlarmCount = -1;

    if ( !(theShip->attributes & kIsHumanControlled)) return( everPaused);

    baseObject = theShip->baseType;

    MiniComputerHandleKeys( gAresGlobal->gTheseKeys, gAresGlobal->gLastKeys);
    if ( (mMessageNextKey( keyMap)) && (!(mMessageNextKey(gAresGlobal->gLastKeyMap))) && (!*enterMessage))
        AdvanceCurrentLongMessage();
    dcalc = kSelectFriendKey | kSelectFoeKey | kSelectBaseKey;
    attributes = gAresGlobal->gTheseKeys & dcalc;

    if ( gAresGlobal->gTheseKeys & kDestinationKey)
    {
        if ( gAresGlobal->gDestKeyTime >= 0)
            gAresGlobal->gDestKeyTime += timePass;
    } else
    {
        if ( gAresGlobal->gDestKeyTime > 45)
        {
            if (( theShip->attributes & kCanBeDestination) &&
                (!gAresGlobal->destKeyUsedForSelection))
            {
/*              SetScreenLabelObject( gAresGlobal->gDestinationLabel, theShip);
                SetScreenLabelAge( gAresGlobal->gDestinationLabel, kLabelOffVisibleTime);
                GetIndString( s, kSpaceObjectNameResID, theShip->whichBaseObject + 1);
                SetScreenLabelString( gAresGlobal->gDestinationLabel, (anyCharType *)s);
                PlayVolumeSound(  kComputerBeep1, kMediumLoudVolume, kMediumPersistence, kLowPrioritySound);
                SetAdmiralDestinationObject( gAresGlobal->gPlayerAdmiralNumber, gAresGlobal->gPlayerShipNumber, kObjectDestinationType);
*/              if ( !(gAresGlobal->gOptions & kOptionNetworkOn))
                {
                    SetPlayerSelectShip( gAresGlobal->gPlayerShipNumber, true,
                        gAresGlobal->gPlayerAdmiralNumber);
                } else
                {
#if NETSPROCKET_AVAILABLE
                    if ( !SendSelectMessage( gAresGlobal->gGameTime + gNetLatency,
                        gAresGlobal->gPlayerShipNumber, true))
                        StopNetworking();
#endif NETSPROCKET_AVAILABLE
                }
            }
        }
        gAresGlobal->gDestKeyTime = 0;
        gAresGlobal->destKeyUsedForSelection = false;
    }

// NEW -- do hot key selection
    b = -1;
    for ( a = 0; a < kHotKeyNum; a++)
    {
        if ( mCheckKeyMap( keyMap, a + kFirstHotKeyNum))
        {
            b = a;
        }
    }

    if ( b >= 0)
    {
        if ( b != gAresGlobal->lastHotKey)
        {
            gAresGlobal->lastHotKey = b;
            gAresGlobal->hotKeyDownTime = 0;
            gAresGlobal->hotKey_target = false;
            if ( gAresGlobal->gTheseKeys & kDestinationKey)
                gAresGlobal->hotKey_target = true;

        } else
        {
            gAresGlobal->hotKeyDownTime += timePass;
        }
    } else if ( gAresGlobal->lastHotKey >= 0)
    {
        b = gAresGlobal->lastHotKey;
        gAresGlobal->lastHotKey = -1;

        if ( gAresGlobal->hotKeyDownTime > 45)
        {
            if ( gAresGlobal->lastSelectedObject >= 0)
            {
                selectShip = *gSpaceObjectData + gAresGlobal->lastSelectedObject;

                if ( selectShip->active)
                {
                    gAresGlobal->hotKey[b].objectNum =
                        gAresGlobal->lastSelectedObject;

                    gAresGlobal->hotKey[b].objectID =
                        gAresGlobal->lastSelectedObjectID;
                    Update_LabelStrings_ForHotKeyChange();
                    PlayVolumeSound(  kComputerBeep1, kMediumLoudVolume,
                        kMediumPersistence, kLowPrioritySound);
                }
            }
        } else
        {
            gAresGlobal->destKeyUsedForSelection = true;
            if ( gAresGlobal->hotKey[b].objectNum >= 0)
            {
                selectShip = *gSpaceObjectData + gAresGlobal->hotKey[b].objectNum;
                if ( (selectShip->active) && ( selectShip->id ==
                        gAresGlobal->hotKey[b].objectID))
                {
                    if (( gAresGlobal->gTheseKeys & kDestinationKey) ||
                        ( selectShip->owner !=
                            gAresGlobal->gPlayerAdmiralNumber) ||
                        ( gAresGlobal->hotKey_target))
                    {
                        a = 1;  // is target
                    } else
                    {
                        a = 0; // is not target
                    }
                    if ( !(gAresGlobal->gOptions & kOptionNetworkOn))
                    {
                        SetPlayerSelectShip( gAresGlobal->hotKey[b].objectNum,
                            (a != 0)?(true):(false),
                            gAresGlobal->gPlayerAdmiralNumber);
                    } else
                    {
    #if NETSPROCKET_AVAILABLE
                        if ( !SendSelectMessage(
                                gAresGlobal->gGameTime + gNetLatency,
                                gAresGlobal->hotKey[b].objectNum,
                                (a != 0)?(true):(false)))
                            StopNetworking();
    #endif NETSPROCKET_AVAILABLE
                    }
                } else
                {
                    gAresGlobal->hotKey[b].objectNum = -1;
                }
            }
            gAresGlobal->hotKeyDownTime = 0;

        }
    }
// end new hotkey selection

    // for this we check lastKeys against theseKeys & relevent keys now being pressed
    if (( attributes) && ( !(gAresGlobal->gLastKeys & attributes)) &&
            (!gAresGlobal->gMouseActive))
    {
        gAresGlobal->gDestKeyTime = -1;
        nonattributes = 0;
        if ( gAresGlobal->gTheseKeys & kSelectFriendKey)
        {
            if ( !(gAresGlobal->gTheseKeys & kDestinationKey))
            {
                selectShipNum = GetAdmiralConsiderObject( gAresGlobal->gPlayerAdmiralNumber);
                attributes = kCanBeDestination;
                nonattributes = kIsDestination;
            } else
            {
                selectShipNum = GetAdmiralDestinationObject( gAresGlobal->gPlayerAdmiralNumber);
                attributes = kCanBeDestination;
                nonattributes = kIsDestination;
            }
//          attributes = kCanAcceptDestination;
            friendOrFoe = 1;
        } else if ( gAresGlobal->gTheseKeys & kSelectFoeKey)
        {
            selectShipNum = GetAdmiralDestinationObject( gAresGlobal->gPlayerAdmiralNumber);
            attributes = kCanBeDestination;
            nonattributes = kIsDestination;
            friendOrFoe = -1;
        } else
        {
            if ( !(gAresGlobal->gTheseKeys & kDestinationKey))
            {
                selectShipNum = GetAdmiralConsiderObject( gAresGlobal->gPlayerAdmiralNumber);
                attributes = kCanAcceptBuild;
                friendOrFoe = 1;
            } else
            {
                selectShipNum = GetAdmiralDestinationObject( gAresGlobal->gPlayerAdmiralNumber);
                attributes = kIsDestination;
                friendOrFoe = 0;
            }
        }
        if ( selectShipNum >= 0)
        {
            selectShip = *gSpaceObjectData + selectShipNum;
/*          if (( selectShip->attributes & kCanThink) || ( selectShip->attributes & kIsHumanControlled))
            {
                distance = selectShip->distanceFromPlayer;
            } else
*/          {
                difference = ABS<int>( theShip->location.h - selectShip->location.h);
                dcalc = difference;
                difference =  ABS<int>( theShip->location.v - selectShip->location.v);
                distance = difference;

                if (( dcalc > kMaximumRelevantDistance) ||
                    ( distance > kMaximumRelevantDistance))
                {
                    hugeDistance.hi = 0;
                    hugeDistance.lo = dcalc;    // must be positive
                    MyWideMul( hugeDistance.lo, hugeDistance.lo, &hugeDistance);    // ppc automatically generates WideMultiply
                    selectShip->distanceFromPlayer.hi = 0;
                    selectShip->distanceFromPlayer.lo = distance;
                    MyWideMul( selectShip->distanceFromPlayer.lo, selectShip->distanceFromPlayer.lo, &selectShip->distanceFromPlayer);
                    MyWideAdd( &selectShip->distanceFromPlayer, &hugeDistance);
                }
                else
                {
                    selectShip->distanceFromPlayer.lo = distance * distance + dcalc * dcalc;
                    selectShip->distanceFromPlayer.hi = 0;
                }
                /*
                selectShip->distanceFromPlayer = (double long)distance * (double long)distance +
                                    (double long)dcalc * (double long)dcalc;
                */
                hugeDistance = selectShip->distanceFromPlayer;
            }
        } else
        {
            hugeDistance.hi = hugeDistance.lo = 0;
        }

/*      WriteDebugDivider();
        WriteDebugLong( hugeDistance.lo);
        WriteDebugLong( selectShipNum);
        WriteDebugLong( theShip->location.h);
        WriteDebugLong( theShip->location.v);
        WriteDebugLong( theShip->direction);
*/      selectShipNum = GetManualSelectObject( theShip, 0, attributes,
                                                nonattributes, &hugeDistance, selectShipNum, friendOrFoe);

        if ( selectShipNum >= 0)
        {
            if ( (gAresGlobal->gTheseKeys & kDestinationKey) || ( gAresGlobal->gTheseKeys & kSelectFoeKey))
            {
                /*
                // set new destination object
                selectShip = *gSpaceObjectData + selectShipNum;
                SetScreenLabelObject( gAresGlobal->gDestinationLabel, selectShip);
                if ( selectShipNum == gAresGlobal->gPlayerShipNumber)
                {
                    SetScreenLabelAge( gAresGlobal->gDestinationLabel, kLabelOffVisibleTime);
                }
                PlayVolumeSound(  kComputerBeep1, kMediumLoudVolume, kMediumPersistence, kLowPrioritySound);
                SetAdmiralDestinationObject( gAresGlobal->gPlayerAdmiralNumber, selectShipNum, kObjectDestinationType);
                if ( selectShip->attributes & kIsDestination)
                {
                    SetScreenLabelString( gAresGlobal->gDestinationLabel, GetDestBalanceName( selectShip->destinationObject));
//                  SetDestinationString( GetDestBalanceName( selectShip->destinationObject), TRUE);
                } else
                {
                    GetIndString( s, kSpaceObjectNameResID, selectShip->whichBaseObject + 1);
                    SetScreenLabelString( gAresGlobal->gDestinationLabel, (anyCharType *)s);
//                  SetDestinationString( s, TRUE);
                }

                SetObjectDestination( theShip, nil);
                */
                if ( !(gAresGlobal->gOptions & kOptionNetworkOn))
                {
                    SetPlayerSelectShip( selectShipNum, true, gAresGlobal->gPlayerAdmiralNumber);
                } else
                {
#if NETSPROCKET_AVAILABLE
                    if ( !SendSelectMessage( gAresGlobal->gGameTime + gNetLatency, selectShipNum, true))
                        StopNetworking();
#endif
                }
            }
            else
            {
                /*
//              ResetScrollStars( selectShipNum);
                selectShip = *gSpaceObjectData + selectShipNum;
                SetScreenLabelObject( gAresGlobal->gSelectionLabel, selectShip);
                if ( selectShipNum == gAresGlobal->gPlayerShipNumber)
                {
                    SetScreenLabelAge( gAresGlobal->gSelectionLabel, kLabelOffVisibleTime);
                }
                PlayVolumeSound(  kComputerBeep1, kMediumLoudVolume, kMediumPersistence, kLowPrioritySound);
                SetAdmiralConsiderObject( gAresGlobal->gPlayerAdmiralNumber, selectShipNum);
                if ( selectShip->attributes & kIsDestination)
                {
                    SetScreenLabelString( gAresGlobal->gSelectionLabel, GetAdmiralBuildAtName( gAresGlobal->gPlayerAdmiralNumber));
                } else
                {
                    GetIndString( s, kSpaceObjectNameResID, selectShip->whichBaseObject + 1);
                    SetScreenLabelString( gAresGlobal->gSelectionLabel, (anyCharType *)s);
                }
    //          NumToString(  selectShip->entryNumber, s);
        //      SetAdmiralDestinationObject( gAresGlobal->gPlayerAdmiralNumber, gAresGlobal->gPlayerShipNumber, kCoordinateDestinationType);
        //      SetObjectDestination( selectShip);
        //      SetDestinationString( s, TRUE);
                */
                if ( !(gAresGlobal->gOptions & kOptionNetworkOn))
                {
                    SetPlayerSelectShip( selectShipNum, false, gAresGlobal->gPlayerAdmiralNumber);
                } else
                {
#if NETSPROCKET_AVAILABLE
                    if ( !SendSelectMessage( gAresGlobal->gGameTime + gNetLatency, selectShipNum, false))
                        StopNetworking();
#endif NETSPROCKET_AVAILABLE
                }
            }
    //      WriteDebugLong( selectShipNum);
        }
    }

    if ( theShip->attributes & kOnAutoPilot)
    {
        if ((gAresGlobal->gAutoPilotOff) && // no off request pending
            ( gAresGlobal->gTheseKeys & ( kUpKey | kDownKey | kLeftKey | kRightKey)))
        {
            theShip->keysDown = gAresGlobal->gTheseKeys | kAutoPilotKey;
            gAresGlobal->gAutoPilotOff = false;
        } else
        {
            theShip->keysDown = (theShip->keysDown & ( ~kMiscKeyMask)) |
                            (gAresGlobal->gTheseKeys & ( kMiscKeyMask));
        }
    } else
    {
        theShip->keysDown = gAresGlobal->gTheseKeys;
        gAresGlobal->gAutoPilotOff = true;
    }

    if (( gAresGlobal->gTheseKeys & kOrderKey) && ( !(gAresGlobal->gLastKeys & kOrderKey)))
    {
        theShip->keysDown |= kGiveCommandKey;
        /*
        selectShipNum = GetAdmiralConsiderObject( gAresGlobal->gPlayerAdmiralNumber);
        if ( selectShipNum >= 0)
        {
            selectShip = *gSpaceObjectData + selectShipNum;
            SetObjectDestination( selectShip, nil);
            PlayVolumeSound(  kMorseBeepSound, kMediumVolume, kMediumPersistence, kLowPrioritySound);
        }
        */
    }

    if (( gAresGlobal->gTheseKeys & kWarpKey) &&
        ( gAresGlobal->gTheseKeys & kDestinationKey))
    {
        gAresGlobal->gDestKeyTime = -1;
        if ( !(gAresGlobal->gLastKeys & kWarpKey))
        {
            if ( !(theShip->attributes & kOnAutoPilot))
                theShip->keysDown |= kAutoPilotKey;
            theShip->keysDown |= kAdoptTargetKey;
        }
        theShip->keysDown &= ~kWarpKey;
    }

    for ( h = 0; h < 4; h++)
        gAresGlobal->gLastKeyMap[h] = keyMap[h];
    return ( everPaused);
}

void PlayerShipHandleClick( Point where)

{
    spaceObjectType *theShip = nil, *selectShip = nil;
    long            selectShipNum;
    Rect            bounds;

    if ( gAresGlobal->keyMask & kMouseMask) return;

    gAresGlobal->gDestKeyTime = -1;
    if ( gAresGlobal->gPlayerShipNumber >= 0)
    {
        theShip = *gSpaceObjectData + gAresGlobal->gPlayerShipNumber;
        if (( theShip->active) && ( theShip->attributes & kIsHumanControlled))
        {
            bounds.left = where.h - kCursorBoundsSize;
            bounds.top = where.v - kCursorBoundsSize;
            bounds.right = where.h + kCursorBoundsSize;
            bounds.bottom = where.v + kCursorBoundsSize;

            if ( theShip->keysDown & kDestinationKey)
            {
                selectShipNum = GetAdmiralDestinationObject( gAresGlobal->gPlayerAdmiralNumber);

                selectShipNum = GetSpritePointSelectObject( &bounds, theShip, 0,
                                                        kCanBeDestination | kIsDestination,//kCanThink | kIsDestination,
                                                        0, selectShipNum, 0);
                if ( selectShipNum >= 0)
                {
                    /*
                    selectShip = *gSpaceObjectData + selectShipNum;
                    SetScreenLabelObject( gAresGlobal->gDestinationLabel, selectShip);
                    if ( selectShipNum == gAresGlobal->gPlayerShipNumber)
                    {
                        SetScreenLabelAge( gAresGlobal->gDestinationLabel, kLabelOffVisibleTime);
                    }
                    PlayVolumeSound(  kComputerBeep1, kMediumLoudVolume, kMediumPersistence, kLowPrioritySound);
                    SetAdmiralDestinationObject( gAresGlobal->gPlayerAdmiralNumber, selectShipNum, kObjectDestinationType);
                    if ( selectShip->attributes & kIsDestination)
                    {
                        SetScreenLabelString( gAresGlobal->gDestinationLabel, GetDestBalanceName( selectShip->destinationObject));
    //                  SetDestinationString( GetDestBalanceName( selectShip->destinationObject), TRUE);
                    } else
                    {
                        GetIndString( s, kSpaceObjectNameResID, selectShip->whichBaseObject + 1);
                        SetScreenLabelString( gAresGlobal->gDestinationLabel, (anyCharType *)s);
    //                  SetDestinationString( s, TRUE);
                    }

                    SetObjectDestination( theShip, nil);
                    */
                    if ( !(gAresGlobal->gOptions & kOptionNetworkOn))
                    {
                        SetPlayerSelectShip( selectShipNum, true, gAresGlobal->gPlayerAdmiralNumber);
                    } else
                    {
#if NETSPROCKET_AVAILABLE
                        if ( !SendSelectMessage( gAresGlobal->gGameTime + gNetLatency, selectShipNum, true))
                            StopNetworking();
#endif NETSPROCKET_AVAILABLE
                    }
                }
            } else
            {
                selectShipNum = GetAdmiralConsiderObject( gAresGlobal->gPlayerAdmiralNumber);
                selectShipNum = GetSpritePointSelectObject( &bounds, theShip, 0,
                                                        kCanThink | kCanAcceptBuild,
                                                        0, selectShipNum, 1);
                if ( selectShipNum >= 0)
                {
                    /*
                    selectShip = *gSpaceObjectData + selectShipNum;
                    SetScreenLabelObject( gAresGlobal->gSelectionLabel, selectShip);
                    if ( selectShipNum == gAresGlobal->gPlayerShipNumber)
                    {
                        SetScreenLabelAge( gAresGlobal->gSelectionLabel, kLabelOffVisibleTime);
                    }
                    PlayVolumeSound(  kComputerBeep1, kMediumLoudVolume, kMediumPersistence, kLowPrioritySound);
                    SetAdmiralConsiderObject( gAresGlobal->gPlayerAdmiralNumber, selectShipNum);
                    if ( selectShip->attributes & kIsDestination)
                    {
                        SetScreenLabelString( gAresGlobal->gSelectionLabel, GetAdmiralBuildAtName( gAresGlobal->gPlayerAdmiralNumber));
                    } else
                    {
                        GetIndString( s, kSpaceObjectNameResID, selectShip->whichBaseObject + 1);
                        SetScreenLabelString( gAresGlobal->gSelectionLabel, (anyCharType *)s);
                    }
                    */
                    if ( !(gAresGlobal->gOptions & kOptionNetworkOn))
                    {
                        SetPlayerSelectShip( selectShipNum, false, gAresGlobal->gPlayerAdmiralNumber);
                    } else
                    {
#if NETSPROCKET_AVAILABLE
                        if ( !SendSelectMessage( gAresGlobal->gGameTime + gNetLatency, selectShipNum, false))
                            StopNetworking();
#endif NETSPROCKET_AVAILABLE
                    }
                }
            }
        }
    }
}

void SetPlayerSelectShip( long whichShip, Boolean target, long admiralNumber)
{
    spaceObjectType *selectShip = *gSpaceObjectData + whichShip,
                    *theShip = GetAdmiralFlagship( admiralNumber);
    Str255          s;

    if ( admiralNumber == gAresGlobal->gPlayerAdmiralNumber)
    {
        gAresGlobal->lastSelectedObject = whichShip;
        gAresGlobal->lastSelectedObjectID = selectShip->id;
        gAresGlobal->destKeyUsedForSelection = true;
        WriteDebugLine("\plast sel");
    }
    if ( target)
    {
        SetAdmiralDestinationObject( admiralNumber, whichShip, kObjectDestinationType);
        if ( admiralNumber == gAresGlobal->gPlayerAdmiralNumber)
        {
            SetScreenLabelObject( gAresGlobal->gDestinationLabel, selectShip);
            if ( whichShip == gAresGlobal->gPlayerShipNumber)
            {
                SetScreenLabelAge( gAresGlobal->gDestinationLabel, kLabelOffVisibleTime);
            }
            PlayVolumeSound(  kComputerBeep1, kMediumLoudVolume, kMediumPersistence, kLowPrioritySound);
            if ( selectShip->attributes & kIsDestination)
            {
                CopyPString( s,
                    GetDestBalanceName( selectShip->destinationObject));

                HotKey_AppendString( selectShip, s);

                SetScreenLabelString( gAresGlobal->gDestinationLabel,
                    s );
            } else
            {
                GetIndString( s, kSpaceObjectNameResID,
                    selectShip->whichBaseObject + 1);

                HotKey_AppendString( selectShip, s);

                SetScreenLabelString( gAresGlobal->gDestinationLabel, s);
            }
        }

        if ( !( theShip->attributes & kOnAutoPilot))
            SetObjectDestination( theShip, nil);
    } else
    {
        SetAdmiralConsiderObject( admiralNumber, whichShip);
        if ( admiralNumber == gAresGlobal->gPlayerAdmiralNumber)
        {
            SetScreenLabelObject( gAresGlobal->gSelectionLabel, selectShip);
            if ( whichShip == gAresGlobal->gPlayerShipNumber)
            {
                SetScreenLabelAge( gAresGlobal->gSelectionLabel,
                    kLabelOffVisibleTime);
            }
            PlayVolumeSound(  kComputerBeep1, kMediumLoudVolume, kMediumPersistence, kLowPrioritySound);
            if ( selectShip->attributes & kIsDestination)
            {
                CopyPString( s,
                    GetDestBalanceName( selectShip->destinationObject));

                HotKey_AppendString( selectShip, s);

                SetScreenLabelString( gAresGlobal->gSelectionLabel, s
                    );
            } else
            {
                GetIndString( s, kSpaceObjectNameResID, selectShip->whichBaseObject + 1);

                HotKey_AppendString( selectShip, s);

                SetScreenLabelString( gAresGlobal->gSelectionLabel, s);
            }
        }
    }
}

// ChangePlayerShipNumber()
// assumes that newShipNumber is the number of a valid (legal, living) ship and that
// gPlayerShip already points to the current, legal living ship

void ChangePlayerShipNumber( long whichAdmiral, long newShipNumber)
{
    spaceObjectType *anObject = GetAdmiralFlagship( whichAdmiral);

    mWriteDebugString("\pXFER");
    WriteDebugLong( whichAdmiral);
    WriteDebugLong( newShipNumber);

    if ( whichAdmiral == gAresGlobal->gPlayerAdmiralNumber)
    {
        anObject->attributes &= (~kIsHumanControlled) & (~kIsPlayerShip);
        if ( newShipNumber != gAresGlobal->gPlayerShipNumber)
        {
            gAresGlobal->gPlayerShipNumber = newShipNumber;
            ResetScrollStars( gAresGlobal->gPlayerShipNumber);
        }


        anObject = *gSpaceObjectData + gAresGlobal->gPlayerShipNumber;

//      if ( !(gAresGlobal->gActiveCheats[whichAdmiral] & kAutoPlayBit))
            anObject->attributes |= (kIsHumanControlled) | (kIsPlayerShip);
//      else
//          anObject->attributes |= kIsPlayerShip;

        if ( newShipNumber == GetAdmiralConsiderObject( gAresGlobal->gPlayerAdmiralNumber))
        {
            SetScreenLabelAge( gAresGlobal->gSelectionLabel, kLabelOffVisibleTime);
        }
        if ( newShipNumber == GetAdmiralDestinationObject( gAresGlobal->gPlayerAdmiralNumber))
        {
            SetScreenLabelAge( gAresGlobal->gDestinationLabel, kLabelOffVisibleTime);
        }
    } else
    {
        anObject->attributes &= ~(kIsRemote | kIsPlayerShip);
        anObject = *gSpaceObjectData + newShipNumber;
        anObject->attributes |= (kIsRemote | kIsPlayerShip);
    }
    SetAdmiralFlagship( whichAdmiral, newShipNumber);
}

void TogglePlayerAutoPilot( spaceObjectType *theShip)
{
    Str255  s;

    if ( theShip->attributes & kOnAutoPilot)
    {
        theShip->attributes &= ~kOnAutoPilot;
        if ((theShip->owner == gAresGlobal->gPlayerAdmiralNumber) &&
            ( theShip->attributes & kIsHumanControlled))
        {
            GetIndString( s, kMessageStringID, kAutoPilotOffString);
            SetStatusString( s, TRUE, kStatusLabelColor);
        }
    } else
    {
        SetObjectDestination( theShip, nil);
        theShip->attributes |= kOnAutoPilot;
        if ((theShip->owner == gAresGlobal->gPlayerAdmiralNumber) &&
            ( theShip->attributes & kIsHumanControlled))
        {
            GetIndString( s, kMessageStringID, kAutoPilotOnString);
            SetStatusString( s, TRUE, kStatusLabelColor);
        }
    }
}

Boolean IsPlayerShipOnAutoPilot( void)
{
    spaceObjectType *theShip;

    if ( gAresGlobal->gPlayerShipNumber < 0) return false;
    theShip = *gSpaceObjectData + gAresGlobal->gPlayerShipNumber;
    if ( theShip->attributes & kOnAutoPilot) return true;
    else return false;
}

void PlayerShipGiveCommand( long whichAdmiral)
{
    spaceObjectType *selectShip;
    long selectShipNum = GetAdmiralConsiderObject( whichAdmiral);

    if ( selectShipNum >= 0)
    {
                    DebugFileAppendString( "\pGO\t");
                    DebugFileAppendLong( selectShipNum);
                    DebugFileAppendString( "\p\t");
                    DebugFileAppendLong( whichAdmiral);
                    DebugFileAppendString( "\p\r");

        selectShip = *gSpaceObjectData + selectShipNum;
        SetObjectDestination( selectShip, nil);
        if ( whichAdmiral == gAresGlobal->gPlayerAdmiralNumber)
            PlayVolumeSound(  kMorseBeepSound, kMediumVolume, kMediumPersistence, kLowPrioritySound);
    }
}

// Boolean sourceIsBody was hacked in to use this for xferring control
void PlayerShipBodyExpire( spaceObjectType *theShip, Boolean sourceIsBody)
{
    spaceObjectType *selectShip = nil;
    long            selectShipNum;

    mWriteDebugString("\pBody Expire");
    selectShipNum = GetAdmiralConsiderObject( theShip->owner);

    if ( selectShipNum >= 0)
    {
        selectShip = *gSpaceObjectData + selectShipNum;
        if (( selectShip->active != kObjectInUse) ||
            ( !(selectShip->attributes & kCanThink)) ||
            ( selectShip->attributes & kStaticDestination)
            || ( selectShip->owner != theShip->owner) ||
            (!(selectShip->attributes & kCanAcceptDestination))
            )
            selectShip = nil;
    }
    if ( selectShip == nil)
    {
//      selectShip = *gSpaceObjectData;
//      selectShipNum = 0;
        selectShip = gRootObject;
        selectShipNum = gRootObjectNumber;
        while ( ( selectShip != nil) &&
                (
                    ( selectShip->active != kObjectInUse)
                    ||
                    ( selectShip->attributes & kStaticDestination)
                    ||
                    (
                        !(
                            (selectShip->attributes & kCanThink) &&
                            (selectShip->attributes & kCanAcceptDestination)
                        )
                    )
                    ||
                    ( selectShip->owner != theShip->owner)
                )
            )
        {
            selectShipNum = selectShip->nextObjectNumber;
            selectShip = selectShip->nextObject;
        }
    }
    if (( selectShip == nil) && ( sourceIsBody))
    {
        if ( gAresGlobal->gGameOver >= 0)
        {
            gAresGlobal->gGameOver = -180;
//          ShowErrorAny( eContinueOnlyErr, -1, "\pEnding the game because", "\p the player died.", nil, nil, -1, -1, -1, -1, __FILE__, 1);
        }
        if ( theShip->owner == gAresGlobal->gPlayerAdmiralNumber)
        {
            gAresGlobal->gScenarioWinner = (gAresGlobal->gScenarioWinner & ~kScenarioWinnerTextMask) |
                (( kScenarioNoShipTextID + gThisScenario->levelNameStrNum) <<
                    kScenarioWinnerTextShift);
        } else
        {
            gAresGlobal->gScenarioWinner = (gAresGlobal->gScenarioWinner & ~kScenarioWinnerTextMask) |
                (( 10050 + gThisScenario->levelNameStrNum) <<
                    kScenarioWinnerTextShift);
        }
        SetAdmiralFlagship( theShip->owner, -1);
    } else if ( selectShip != nil)
    {
        ChangePlayerShipNumber( theShip->owner, selectShipNum);
    }
}

void HandleTextMessageKeys( KeyMap keyMap, KeyMap lastKeyMap,
    Boolean *enterMessage)
{
    Boolean         newKeys = false, anyKeys = false;
    short           h;
    KeyMap          *bufferMap;

    for ( h = 0; h < 4; h++)
    {
#if TARGET_OS_MAC
        if ( lastKeyMap[h] != keyMap[h]) newKeys = true;
        if ( keyMap[h] != 0) anyKeys = true;
#else
        if ( lastKeyMap[h].bigEndianValue != keyMap[h].bigEndianValue) newKeys = true;
        if ( keyMap[h].bigEndianValue != 0) anyKeys = true;
#endif TARGET_OS_MAC
    }
    if ( newKeys)
    {
        if (( *enterMessage) && anyKeys)
            PlayVolumeSound(  kTeletype, kMediumLowVolume, kShortPersistence, kLowPrioritySound);
        bufferMap = reinterpret_cast<KeyMap*>(gAresGlobal->gKeyMapBuffer) +
            gAresGlobal->gKeyMapBufferTop;
        for ( h = 0; h < 4; h++)
            (*bufferMap)[h] = keyMap[h];
        if ( mReturnKey( keyMap))
        {
            if ( *enterMessage) *enterMessage = false;
            else *enterMessage = true;
        }
        gAresGlobal->gKeyMapBufferTop++;
        if ( gAresGlobal->gKeyMapBufferTop >= kKeyMapBufferNum)
            gAresGlobal->gKeyMapBufferTop = 0;
    }
}

long HotKey_GetFromObject( spaceObjectType *object)
{
    long    i = 0;

    if ( object == nil) return -1;
    if ( !object->active) return -1;
    while ( i < kHotKeyNum)
    {
        if ( gAresGlobal->hotKey[i].objectNum == object->entryNumber)
        {
            if ( gAresGlobal->hotKey[i].objectID == object->id)
            {
                return i;
            }
        }
        i++;
    }
    return -1;
}

StringPtr HotKey_AppendString( spaceObjectType *object, StringPtr s)
{
    long    h = HotKey_GetFromObject( object), keyNum;
    Str255  keyName;

    if ( h < 0) return s;
    keyNum = GetKeyNumFromKeyMap(
        gAresGlobal->gKeyControl[h + kFirstHotKeyNum]);
    if ( keyNum < 0) return s;

    GetIndString( keyName, kKeyMapNameLongID, keyNum);
    ConcatenatePString( s, "\p < ");
    ConcatenatePString( s, keyName);
    ConcatenatePString( s, "\p >");
    return s;
}

void Update_LabelStrings_ForHotKeyChange( void)
{
    spaceObjectType *selectShip;
    long            whichShip;
    Str255          s;

    whichShip = GetAdmiralDestinationObject( gAresGlobal->gPlayerAdmiralNumber);
    if ( whichShip >= 0)

    {
        selectShip = *gSpaceObjectData + whichShip;

//      if ( admiralNumber == gAresGlobal->gPlayerAdmiralNumber)
        {
            SetScreenLabelObject( gAresGlobal->gDestinationLabel, selectShip);
            if ( whichShip == gAresGlobal->gPlayerShipNumber)
            {
                SetScreenLabelAge( gAresGlobal->gDestinationLabel,
                    kLabelOffVisibleTime);
            }
            if ( selectShip->attributes & kIsDestination)
            {
                CopyPString( s,
                    GetDestBalanceName( selectShip->destinationObject));

                HotKey_AppendString( selectShip, s);

                SetScreenLabelString( gAresGlobal->gDestinationLabel,
                    s );
            } else
            {
                GetIndString( s, kSpaceObjectNameResID,
                    selectShip->whichBaseObject + 1);

                HotKey_AppendString( selectShip, s);

                SetScreenLabelString( gAresGlobal->gDestinationLabel, s);
            }
        }

    }

    whichShip = GetAdmiralConsiderObject( gAresGlobal->gPlayerAdmiralNumber);
    if ( whichShip >= 0)
    {
        selectShip = *gSpaceObjectData + whichShip;
//      if ( admiralNumber == gAresGlobal->gPlayerAdmiralNumber)
        {
            SetScreenLabelObject( gAresGlobal->gSelectionLabel, selectShip);
            if ( whichShip == gAresGlobal->gPlayerShipNumber)
            {
                SetScreenLabelAge( gAresGlobal->gSelectionLabel,
                    kLabelOffVisibleTime);
            }
            PlayVolumeSound(  kComputerBeep1, kMediumLoudVolume, kMediumPersistence, kLowPrioritySound);
            if ( selectShip->attributes & kIsDestination)
            {
                CopyPString( s,
                    GetDestBalanceName( selectShip->destinationObject));

                HotKey_AppendString( selectShip, s);

                SetScreenLabelString( gAresGlobal->gSelectionLabel,
                    s);
            } else
            {
                GetIndString( s, kSpaceObjectNameResID, selectShip->whichBaseObject + 1);

                HotKey_AppendString( selectShip, s);

                SetScreenLabelString( gAresGlobal->gSelectionLabel, s);
            }
        }
    }
}
