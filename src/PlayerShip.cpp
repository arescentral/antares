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

#include "Admiral.hpp"
#include "AresCheat.hpp"
#include "AresGlobalType.hpp"
#include "AresNetworkSprocket.hpp"
#include "AresPreferences.hpp"
#include "ColorTranslation.hpp"
#include "Debug.hpp"
#include "DirectText.hpp"
#include "Error.hpp"
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
#include "SoundFX.hpp"
#include "SpaceObject.hpp"
#include "SpaceObjectHandling.hpp"
#include "StringHandling.hpp"
#include "UniverseUnit.hpp"

namespace antares {

#define kSendMessageVOffset     20


#define kCursorBoundsSize   16          // should be same in instruments.c

extern TypedHandle<spaceObjectType> gSpaceObjectData;
extern long         gRandomSeed, gNetLatency, gRootObjectNumber;
extern scenarioType *gThisScenario; // for setting debrief message when we run out of ships
extern coordPointType           gGlobalCorner;
extern long         gAbsoluteScale; // HACK FOR TESTING
extern directTextType   *gDirectText;
extern long         CLIP_LEFT, CLIP_TOP, CLIP_RIGHT, CLIP_BOTTOM;
extern spaceObjectType  *gRootObject;

long HotKey_GetFromObject( spaceObjectType *object);
unsigned char* HotKey_AppendString(spaceObjectType *object, unsigned char* s);
void Update_LabelStrings_ForHotKeyChange( void);

void StartPlayerShip( long owner, short type)

{
#pragma unused( owner, type)
}

void ResetPlayerShip( long which)

{
    unsigned char   nilLabel = 0;
    long            h;

    WriteDebugLine("\pPLAYER:");
    WriteDebugLong( which);
    globals()->gPlayerShipNumber = which;
    globals()->gSelectionLabel = AddScreenLabel( 0, 0, 0, 10, &nilLabel, nil, true, YELLOW);
    globals()->gDestinationLabel = AddScreenLabel( 0, 0, 0, -20, &nilLabel, nil, true, SKY_BLUE);
    globals()->gSendMessageLabel = AddScreenLabel( 200, 200, 0, 30, &nilLabel, nil, false, GREEN);
    ResetScrollStars( globals()->gPlayerShipNumber);
    globals()->gAlarmCount = -1;
    globals()->gDemoZoomOverride = false;
    globals()->gLastKeys = globals()->gTheseKeys = 0;
    globals()->gAutoPilotOff = true;
    globals()->keyMask = 0;
    for ( h = 0; h < 4; h++)
    {
#if TARGET_OS_MAC
        globals()->gLastKeyMap[h] = 0;
        globals()->gLastMessageKeyMap[h] = 0;
#else
        globals()->gLastKeyMap[h].bigEndianValue = 0;
        globals()->gLastMessageKeyMap[h].bigEndianValue = 0;
#endif TARGET_OS_MAC
    }
    globals()->gZoomMode = kNearestFoeZoom;
    globals()->gKeyMapBufferTop = globals()->gKeyMapBufferBottom = 0;

    for ( h = 0; h < kHotKeyNum; h++)
    {
        globals()->hotKey[h].objectNum = -1;
        globals()->hotKey[h].objectID = -1;
    }
    globals()->hotKeyDownTime = 0;
    globals()->lastHotKey = -1;
    globals()->destKeyUsedForSelection = false;
    globals()->hotKey_target = false;
}

bool PlayerShipGetKeys( long timePass, unsigned long theKeys,
    bool *enterMessage)
{
    KeyMap          keyMap, *bufMap;
    short           a, b, h, friendOrFoe;
    spaceObjectType *theShip = nil, *selectShip = nil;
    baseObjectType  *baseObject = nil;
    long            selectShipNum;
    unsigned long   distance, difference, dcalc, attributes, nonattributes;
    bool         everPaused = false, newKeys = false;
    uint64_t        hugeDistance;
    unsigned char   *message;
    long            width, height, strlen;
    Str255          s;

    if ( globals()->gPlayerShipNumber < 0) return ( everPaused);

    GetKeys( keyMap);
    for ( h = 0; h < 4; h++)
#if TARGET_OS_MAC
        if ( globals()->gLastKeyMap[h] != keyMap[h]) newKeys = true;
#else
        if ( globals()->gLastKeyMap[h].bigEndianValue !=
            keyMap[h].bigEndianValue) newKeys = true;
#endif TARGET_OS_MAC

    globals()->gLastKeys = globals()->gTheseKeys;

    if ( theKeys == 0xffffffff)
    {
        globals()->gTheseKeys = 0;

//#ifndef kNonPlayableDemo
        for ( b = 0; b < kKeyControlNum; b++)
        {
            for ( a = 0; a < 4; a++)
            {
#if TARGET_OS_MAC
                if ( keyMap[a] & globals()->gKeyControl[b][a])
#else
                if ( keyMap[a].bigEndianValue &
                    globals()->gKeyControl[b][a].bigEndianValue)
#endif TARGET_OS_MAC
                {
                    globals()->gTheseKeys |= (0x01 << b) & ~globals()->keyMask;
                }
            }
        }
//#endif
    } else
    {
        globals()->gTheseKeys = theKeys;
        distance = 0;
        for ( b = 0; b < kKeyControlNum; b++)
        {
            for ( a = 0; a < 4; a++)
            {
#if TARGET_OS_MAC
                if ( keyMap[a] & globals()->gKeyControl[b][a])
#else
                if ( keyMap[a].bigEndianValue &
                    globals()->gKeyControl[b][a].bigEndianValue)
#endif TARGET_OS_MAC
                {
                    distance |= 0x01 << b;
                }
            }
        }
        if ( distance & (kZoomOutKey | kZoomInKey))
            globals()->gDemoZoomOverride = true;
        if ( globals()->gDemoZoomOverride)
        {
            globals()->gTheseKeys &= ~(kZoomOutKey | kZoomInKey);
            globals()->gTheseKeys |= distance & (kZoomOutKey | kZoomInKey);
        }
    }

    if ( *enterMessage) globals()->gTheseKeys = 0;
    while (( globals()->gKeyMapBufferBottom != globals()->gKeyMapBufferTop))
        // && ( globals()->gOptions & kOptionNetworkOn))
    {
        bufMap = globals()->gKeyMapBuffer + globals()->gKeyMapBufferBottom;
        globals()->gKeyMapBufferBottom++;
        if ( globals()->gKeyMapBufferBottom >= kKeyMapBufferNum)
            globals()->gKeyMapBufferBottom = 0;
        if (( *enterMessage))// && ( globals()->gOptions & kOptionNetworkOn))
        {
            message = GetScreenLabelStringPtr( globals()->gSendMessageLabel);
            if ( mGetAnyCharPStringLength( message) == 0)
            {
                CopyAnyCharPString( message, "\p<>");
            }
//          globals()->gTheseKeys = 0;
            if (( mReturnKey( *bufMap)) && (!AnyKeyButThisOne( *bufMap, 1, 28)))
            {
                *enterMessage = false;
                CutCharsFromAnyCharPString( message, mGetAnyCharPStringLength( message) - 1, 1);
                CutCharsFromAnyCharPString( message, 0, 1);
                a = GetCheatNumFromString( message);
                if ( a > 0)
                {
                    if ( globals()->gOptions & kOptionNetworkOn)
                    {
#if NETSPROCKET_AVAILABLE
                        SendCheatMessage( a);
#endif NETSPROCKET_AVAILABLE
                    } else
                        ExecuteCheat( a, globals()->gPlayerAdmiralNumber);
                } else if ( message[0] > 0)
                {
                    if ( globals()->gActiveCheats[globals()->gPlayerAdmiralNumber] & kNameObjectBit)
                    {
                        SetAdmiralBuildAtName( globals()->gPlayerAdmiralNumber, message);
                        globals()->gActiveCheats[globals()->gPlayerAdmiralNumber] &= ~kNameObjectBit;
                    }
#if NETSPROCKET_AVAILABLE
                    SendInGameTextMessage( (Ptr)(message + 1), mGetAnyCharPStringLength( message));
#endif NETSPROCKET_AVAILABLE
                }
                message[0] = 0;
                SetScreenLabelPosition( globals()->gSendMessageLabel, CLIP_LEFT +
                    (((CLIP_RIGHT - CLIP_LEFT) / 2)), CLIP_TOP +
                    (((globals()->gTrueClipBottom - CLIP_TOP) / 2)) + kSendMessageVOffset);
                RecalcScreenLabelSize( globals()->gSendMessageLabel);
            } else
            {
                message = GetScreenLabelStringPtr( globals()->gSendMessageLabel);
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
                            globals()->gLastMessageKeyMap);
                        if (s[1])
                        {
                            InsertAnyCharPStringInPString( message, s,
                                mGetAnyCharPStringLength( message) - 1);
//                          PlayVolumeSound(  kTeletype, kMediumLowVolume, kShortPersistence, kLowPrioritySound);
                        }
                    }
                }
                mGetDirectStringDimensions( message, width, height);
                strlen = CLIP_LEFT + (((CLIP_RIGHT - CLIP_LEFT) / 2) - ( width / 2));
                if (( strlen + width) > (CLIP_RIGHT))
                {
                    strlen -= (strlen + width) - (CLIP_RIGHT);
                }
                RecalcScreenLabelSize( globals()->gSendMessageLabel);
                SetScreenLabelPosition( globals()->gSendMessageLabel, strlen, CLIP_TOP +
                    (((globals()->gTrueClipBottom - CLIP_TOP) / 2) + kSendMessageVOffset));
            }
        } else //if  ( globals()->gOptions & kOptionNetworkOn)
        {
            if ( (mReturnKey( *bufMap)) && ( !(globals()->keyMask & kReturnKeyMask)))
                *enterMessage = true;
        }
        for ( h = 0; h < 4; h++)
            globals()->gLastMessageKeyMap[h] = (*bufMap)[h];
    }
/*  if (( *enterMessage) && ( globals()->gOptions & kOptionNetworkOn))
    {
        message = GetScreenLabelStringPtr( globals()->gSendMessageLabel);
        if ( mGetAnyCharPStringLength( message) == 0)
        {
            CopyAnyCharPString( message, (anyCharType *)"\p<>");
        }
        globals()->gTheseKeys = 0;
        if (( mReturnKey( keyMap)) && ((!mReturnKey( globals()->gLastKeyMap))))
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
                if ( globals()->gActiveCheats[globals()->gPlayerAdmiralNumber] & kNameObjectBit)
                {
                    SetAdmiralBuildAtName( globals()->gPlayerAdmiralNumber, message);
                    globals()->gActiveCheats[globals()->gPlayerAdmiralNumber] &= ~kNameObjectBit;
                }
                SendInGameTextMessage( (Ptr)(message + 1), mGetAnyCharPStringLength( message));
            }
            message[0] = 0;
            SetScreenLabelPosition( globals()->gSendMessageLabel, CLIP_LEFT +
                (((CLIP_RIGHT - CLIP_LEFT) / 2)), CLIP_TOP +
                (((globals()->gTrueClipBottom - CLIP_TOP) / 2)) + kSendMessageVOffset);
            RecalcScreenLabelSize( globals()->gSendMessageLabel);
        } else if ( newKeys)
        {
            message = GetScreenLabelStringPtr( globals()->gSendMessageLabel);
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
                    s[1] = GetAsciiFromKeyMap( keyMap, globals()->gLastKeyMap);
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
            RecalcScreenLabelSize( globals()->gSendMessageLabel);
            SetScreenLabelPosition( globals()->gSendMessageLabel, strlen, CLIP_TOP +
                (((globals()->gTrueClipBottom - CLIP_TOP) / 2) + kSendMessageVOffset));
        }
    } else if  ( globals()->gOptions & kOptionNetworkOn)
    {
        if (( mReturnKey( keyMap)) && ((!mReturnKey( globals()->gLastKeyMap)))) *enterMessage = true;
    }
*/
    // TERRIBLE HACK:
    //  this implements the often requested feature of having a shortcut for
    //  transfering control.

    a = globals()->gZoomMode;

    if (( globals()->gTheseKeys & kZoomOutKey) && ( !(globals()->gLastKeys & kZoomOutKey)))
    {
        reinterpret_cast<int&>(globals()->gZoomMode)++;
        if ( globals()->gZoomMode > kSmallestZoom)
        {
             globals()->gZoomMode = kSmallestZoom;
        }
    }
    if (( globals()->gTheseKeys & kZoomInKey) && ( !(globals()->gLastKeys & kZoomInKey)))
    {
        reinterpret_cast<int&>(globals()->gZoomMode)--;
        if ( globals()->gZoomMode < kTimesTwoZoom)
        {
            globals()->gZoomMode = kTimesTwoZoom;
        }
    }

//  if ((( globals()->gOptions & kOptionSubstituteFKeys) ?
//      ((!*enterMessage) && ( mNOFScale221Key( keyMap))):( mScale221Key( keyMap))))

/*
    if ( mScale221Key( keyMap))
    {
        globals()->gZoomMode = 1;
    } else if ( mScale121Key( keyMap))
    {
        globals()->gZoomMode = 2;
    } else if ( mScale122Key( keyMap))
    {
        globals()->gZoomMode = 3;
    } else if ( mScale124Key( keyMap))
    {
        globals()->gZoomMode = 4;
    } else if ( mScaleHostileKey( keyMap))
    {
        globals()->gZoomMode = 5;
    } else if ( mScaleObjectKey( keyMap))
    {
        globals()->gZoomMode = 6;
    } else if ( mScaleAllKey( keyMap))
    {
        globals()->gZoomMode = 7;
    }
*/
//  if ((( globals()->gOptions & kOptionSubstituteFKeys) ?
//      ((!*enterMessage) && ( mNOFScale221Key( keyMap))):( mScale221Key( keyMap))))
//  {
//      globals()->gZoomMode = 1;
//  }

    if ( !*enterMessage)
    {
        if ((!(globals()->gOptions & kOptionSubstituteFKeys)) &&
            ( mTransferKey( keyMap)) && ( !(mTransferKey( globals()->gLastKeyMap))))
        {
            if (!(globals()->gOptions & kOptionNetworkOn))
            {
                MiniComputerExecute( 3,
                    1, globals()->gPlayerAdmiralNumber);
            } else
            {
    #if NETSPROCKET_AVAILABLE
                SendMenuMessage( globals()->gGameTime + gNetLatency,
                    3,  // the special screen
                    1   // kSpecialMiniTransfer
                    );
    #endif NETSPROCKET_AVAILABLE
            }
        }
        if (( ( mScale121Key( keyMap))))
        {
            globals()->gZoomMode = kActualSizeZoom;
        }

        if ((  ( mScale122Key( keyMap))))
        {
            globals()->gZoomMode = kHalfSizeZoom;
        }

        if ((  ( mScale124Key( keyMap))))
        {
            globals()->gZoomMode = kQuarterSizeZoom;
        }

        if ((  ( mScale1216Key( keyMap))))
        {
            globals()->gZoomMode = kEighthSizeZoom;
        }

        if ((  ( mScaleHostileKey( keyMap))))
        {
            globals()->gZoomMode = kNearestFoeZoom;
        }

        if ((  ( mScaleObjectKey( keyMap))))
        {
            globals()->gZoomMode = kNearestAnythingZoom;
        }

        if ((  ( mScaleAllKey( keyMap))))
        {
            globals()->gZoomMode = kSmallestZoom;
        }
    }

    if ( globals()->gZoomMode != a)
    {
        PlayVolumeSound(  kComputerBeep3, kMediumVolume, kMediumPersistence, kLowPrioritySound);
        GetIndString( s, kMessageStringID, globals()->gZoomMode + kZoomStringOffset);
        SetStatusString( s, true, kStatusLabelColor);
    }

    theShip = *gSpaceObjectData + globals()->gPlayerShipNumber;
//  theShip->attributes &= ~kIsHumanControlled;
    if ( !theShip->active) return ( everPaused);

    if ( theShip->health < ( theShip->baseType->health >> 2L))
    {
         if ( globals()->gAlarmCount < 0)
         {
            PlayVolumeSound( kKlaxon, kMaxSoundVolume, kLongPersistence, kMustPlaySound);
            globals()->gAlarmCount = 0;
            SetStatusString( "\pWARNING: Shields Low", true, kStatusWarnColor);
         } else
         {
            globals()->gAlarmCount += timePass;
            if ( globals()->gAlarmCount > 125)
            {
                PlayVolumeSound( kKlaxon, kMediumVolume, kMediumLongPersistence, kPrioritySound);
                globals()->gAlarmCount = 0;
                SetStatusString( "\pWARNING: Shields Low", true, kStatusWarnColor);
            }
        }
    } else globals()->gAlarmCount = -1;

    if ( !(theShip->attributes & kIsHumanControlled)) return( everPaused);

    baseObject = theShip->baseType;

    MiniComputerHandleKeys( globals()->gTheseKeys, globals()->gLastKeys);
    if ( (mMessageNextKey( keyMap)) && (!(mMessageNextKey(globals()->gLastKeyMap))) && (!*enterMessage))
        AdvanceCurrentLongMessage();
    dcalc = kSelectFriendKey | kSelectFoeKey | kSelectBaseKey;
    attributes = globals()->gTheseKeys & dcalc;

    if ( globals()->gTheseKeys & kDestinationKey)
    {
        if ( globals()->gDestKeyTime >= 0)
            globals()->gDestKeyTime += timePass;
    } else
    {
        if ( globals()->gDestKeyTime > 45)
        {
            if (( theShip->attributes & kCanBeDestination) &&
                (!globals()->destKeyUsedForSelection))
            {
/*              SetScreenLabelObject( globals()->gDestinationLabel, theShip);
                SetScreenLabelAge( globals()->gDestinationLabel, kLabelOffVisibleTime);
                GetIndString( s, kSpaceObjectNameResID, theShip->whichBaseObject + 1);
                SetScreenLabelString( globals()->gDestinationLabel, (anyCharType *)s);
                PlayVolumeSound(  kComputerBeep1, kMediumLoudVolume, kMediumPersistence, kLowPrioritySound);
                SetAdmiralDestinationObject( globals()->gPlayerAdmiralNumber, globals()->gPlayerShipNumber, kObjectDestinationType);
*/              if ( !(globals()->gOptions & kOptionNetworkOn))
                {
                    SetPlayerSelectShip( globals()->gPlayerShipNumber, true,
                        globals()->gPlayerAdmiralNumber);
                } else
                {
#if NETSPROCKET_AVAILABLE
                    if ( !SendSelectMessage( globals()->gGameTime + gNetLatency,
                        globals()->gPlayerShipNumber, true))
                        StopNetworking();
#endif NETSPROCKET_AVAILABLE
                }
            }
        }
        globals()->gDestKeyTime = 0;
        globals()->destKeyUsedForSelection = false;
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
        if ( b != globals()->lastHotKey)
        {
            globals()->lastHotKey = b;
            globals()->hotKeyDownTime = 0;
            globals()->hotKey_target = false;
            if ( globals()->gTheseKeys & kDestinationKey)
                globals()->hotKey_target = true;

        } else
        {
            globals()->hotKeyDownTime += timePass;
        }
    } else if ( globals()->lastHotKey >= 0)
    {
        b = globals()->lastHotKey;
        globals()->lastHotKey = -1;

        if ( globals()->hotKeyDownTime > 45)
        {
            if ( globals()->lastSelectedObject >= 0)
            {
                selectShip = *gSpaceObjectData + globals()->lastSelectedObject;

                if ( selectShip->active)
                {
                    globals()->hotKey[b].objectNum =
                        globals()->lastSelectedObject;

                    globals()->hotKey[b].objectID =
                        globals()->lastSelectedObjectID;
                    Update_LabelStrings_ForHotKeyChange();
                    PlayVolumeSound(  kComputerBeep1, kMediumLoudVolume,
                        kMediumPersistence, kLowPrioritySound);
                }
            }
        } else
        {
            globals()->destKeyUsedForSelection = true;
            if ( globals()->hotKey[b].objectNum >= 0)
            {
                selectShip = *gSpaceObjectData + globals()->hotKey[b].objectNum;
                if ( (selectShip->active) && ( selectShip->id ==
                        globals()->hotKey[b].objectID))
                {
                    if (( globals()->gTheseKeys & kDestinationKey) ||
                        ( selectShip->owner !=
                            globals()->gPlayerAdmiralNumber) ||
                        ( globals()->hotKey_target))
                    {
                        a = 1;  // is target
                    } else
                    {
                        a = 0; // is not target
                    }
                    if ( !(globals()->gOptions & kOptionNetworkOn))
                    {
                        SetPlayerSelectShip( globals()->hotKey[b].objectNum,
                            (a != 0)?(true):(false),
                            globals()->gPlayerAdmiralNumber);
                    } else
                    {
    #if NETSPROCKET_AVAILABLE
                        if ( !SendSelectMessage(
                                globals()->gGameTime + gNetLatency,
                                globals()->hotKey[b].objectNum,
                                (a != 0)?(true):(false)))
                            StopNetworking();
    #endif NETSPROCKET_AVAILABLE
                    }
                } else
                {
                    globals()->hotKey[b].objectNum = -1;
                }
            }
            globals()->hotKeyDownTime = 0;

        }
    }
// end new hotkey selection

    // for this we check lastKeys against theseKeys & relevent keys now being pressed
    if (( attributes) && ( !(globals()->gLastKeys & attributes)) &&
            (!globals()->gMouseActive))
    {
        globals()->gDestKeyTime = -1;
        nonattributes = 0;
        if ( globals()->gTheseKeys & kSelectFriendKey)
        {
            if ( !(globals()->gTheseKeys & kDestinationKey))
            {
                selectShipNum = GetAdmiralConsiderObject( globals()->gPlayerAdmiralNumber);
                attributes = kCanBeDestination;
                nonattributes = kIsDestination;
            } else
            {
                selectShipNum = GetAdmiralDestinationObject( globals()->gPlayerAdmiralNumber);
                attributes = kCanBeDestination;
                nonattributes = kIsDestination;
            }
//          attributes = kCanAcceptDestination;
            friendOrFoe = 1;
        } else if ( globals()->gTheseKeys & kSelectFoeKey)
        {
            selectShipNum = GetAdmiralDestinationObject( globals()->gPlayerAdmiralNumber);
            attributes = kCanBeDestination;
            nonattributes = kIsDestination;
            friendOrFoe = -1;
        } else
        {
            if ( !(globals()->gTheseKeys & kDestinationKey))
            {
                selectShipNum = GetAdmiralConsiderObject( globals()->gPlayerAdmiralNumber);
                attributes = kCanAcceptBuild;
                friendOrFoe = 1;
            } else
            {
                selectShipNum = GetAdmiralDestinationObject( globals()->gPlayerAdmiralNumber);
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
                    hugeDistance = dcalc;    // must be positive
                    MyWideMul(hugeDistance, hugeDistance, &hugeDistance);    // ppc automatically generates WideMultiply
                    selectShip->distanceFromPlayer = distance;
                    MyWideMul(selectShip->distanceFromPlayer, selectShip->distanceFromPlayer, &selectShip->distanceFromPlayer);
                    selectShip->distanceFromPlayer += hugeDistance;
                }
                else
                {
                    selectShip->distanceFromPlayer = distance * distance + dcalc * dcalc;
                }
                /*
                selectShip->distanceFromPlayer = (double long)distance * (double long)distance +
                                    (double long)dcalc * (double long)dcalc;
                */
                hugeDistance = selectShip->distanceFromPlayer;
            }
        } else {
            hugeDistance = 0;
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
            if ( (globals()->gTheseKeys & kDestinationKey) || ( globals()->gTheseKeys & kSelectFoeKey))
            {
                /*
                // set new destination object
                selectShip = *gSpaceObjectData + selectShipNum;
                SetScreenLabelObject( globals()->gDestinationLabel, selectShip);
                if ( selectShipNum == globals()->gPlayerShipNumber)
                {
                    SetScreenLabelAge( globals()->gDestinationLabel, kLabelOffVisibleTime);
                }
                PlayVolumeSound(  kComputerBeep1, kMediumLoudVolume, kMediumPersistence, kLowPrioritySound);
                SetAdmiralDestinationObject( globals()->gPlayerAdmiralNumber, selectShipNum, kObjectDestinationType);
                if ( selectShip->attributes & kIsDestination)
                {
                    SetScreenLabelString( globals()->gDestinationLabel, GetDestBalanceName( selectShip->destinationObject));
//                  SetDestinationString( GetDestBalanceName( selectShip->destinationObject), true);
                } else
                {
                    GetIndString( s, kSpaceObjectNameResID, selectShip->whichBaseObject + 1);
                    SetScreenLabelString( globals()->gDestinationLabel, (anyCharType *)s);
//                  SetDestinationString( s, true);
                }

                SetObjectDestination( theShip, nil);
                */
                if ( !(globals()->gOptions & kOptionNetworkOn))
                {
                    SetPlayerSelectShip( selectShipNum, true, globals()->gPlayerAdmiralNumber);
                } else
                {
#if NETSPROCKET_AVAILABLE
                    if ( !SendSelectMessage( globals()->gGameTime + gNetLatency, selectShipNum, true))
                        StopNetworking();
#endif
                }
            }
            else
            {
                /*
//              ResetScrollStars( selectShipNum);
                selectShip = *gSpaceObjectData + selectShipNum;
                SetScreenLabelObject( globals()->gSelectionLabel, selectShip);
                if ( selectShipNum == globals()->gPlayerShipNumber)
                {
                    SetScreenLabelAge( globals()->gSelectionLabel, kLabelOffVisibleTime);
                }
                PlayVolumeSound(  kComputerBeep1, kMediumLoudVolume, kMediumPersistence, kLowPrioritySound);
                SetAdmiralConsiderObject( globals()->gPlayerAdmiralNumber, selectShipNum);
                if ( selectShip->attributes & kIsDestination)
                {
                    SetScreenLabelString( globals()->gSelectionLabel, GetAdmiralBuildAtName( globals()->gPlayerAdmiralNumber));
                } else
                {
                    GetIndString( s, kSpaceObjectNameResID, selectShip->whichBaseObject + 1);
                    SetScreenLabelString( globals()->gSelectionLabel, (anyCharType *)s);
                }
    //          NumToString(  selectShip->entryNumber, s);
        //      SetAdmiralDestinationObject( globals()->gPlayerAdmiralNumber, globals()->gPlayerShipNumber, kCoordinateDestinationType);
        //      SetObjectDestination( selectShip);
        //      SetDestinationString( s, true);
                */
                if ( !(globals()->gOptions & kOptionNetworkOn))
                {
                    SetPlayerSelectShip( selectShipNum, false, globals()->gPlayerAdmiralNumber);
                } else
                {
#if NETSPROCKET_AVAILABLE
                    if ( !SendSelectMessage( globals()->gGameTime + gNetLatency, selectShipNum, false))
                        StopNetworking();
#endif NETSPROCKET_AVAILABLE
                }
            }
    //      WriteDebugLong( selectShipNum);
        }
    }

    if ( theShip->attributes & kOnAutoPilot)
    {
        if ((globals()->gAutoPilotOff) && // no off request pending
            ( globals()->gTheseKeys & ( kUpKey | kDownKey | kLeftKey | kRightKey)))
        {
            theShip->keysDown = globals()->gTheseKeys | kAutoPilotKey;
            globals()->gAutoPilotOff = false;
        } else
        {
            theShip->keysDown = (theShip->keysDown & ( ~kMiscKeyMask)) |
                            (globals()->gTheseKeys & ( kMiscKeyMask));
        }
    } else
    {
        theShip->keysDown = globals()->gTheseKeys;
        globals()->gAutoPilotOff = true;
    }

    if (( globals()->gTheseKeys & kOrderKey) && ( !(globals()->gLastKeys & kOrderKey)))
    {
        theShip->keysDown |= kGiveCommandKey;
        /*
        selectShipNum = GetAdmiralConsiderObject( globals()->gPlayerAdmiralNumber);
        if ( selectShipNum >= 0)
        {
            selectShip = *gSpaceObjectData + selectShipNum;
            SetObjectDestination( selectShip, nil);
            PlayVolumeSound(  kMorseBeepSound, kMediumVolume, kMediumPersistence, kLowPrioritySound);
        }
        */
    }

    if (( globals()->gTheseKeys & kWarpKey) &&
        ( globals()->gTheseKeys & kDestinationKey))
    {
        globals()->gDestKeyTime = -1;
        if ( !(globals()->gLastKeys & kWarpKey))
        {
            if ( !(theShip->attributes & kOnAutoPilot))
                theShip->keysDown |= kAutoPilotKey;
            theShip->keysDown |= kAdoptTargetKey;
        }
        theShip->keysDown &= ~kWarpKey;
    }

    for ( h = 0; h < 4; h++)
        globals()->gLastKeyMap[h] = keyMap[h];
    return ( everPaused);
}

void PlayerShipHandleClick( Point where)

{
    spaceObjectType *theShip = nil;
    long            selectShipNum;
    Rect            bounds;

    if ( globals()->keyMask & kMouseMask) return;

    globals()->gDestKeyTime = -1;
    if ( globals()->gPlayerShipNumber >= 0)
    {
        theShip = *gSpaceObjectData + globals()->gPlayerShipNumber;
        if (( theShip->active) && ( theShip->attributes & kIsHumanControlled))
        {
            bounds.left = where.h - kCursorBoundsSize;
            bounds.top = where.v - kCursorBoundsSize;
            bounds.right = where.h + kCursorBoundsSize;
            bounds.bottom = where.v + kCursorBoundsSize;

            if ( theShip->keysDown & kDestinationKey)
            {
                selectShipNum = GetAdmiralDestinationObject( globals()->gPlayerAdmiralNumber);

                selectShipNum = GetSpritePointSelectObject( &bounds, theShip, 0,
                                                        kCanBeDestination | kIsDestination,//kCanThink | kIsDestination,
                                                        0, selectShipNum, 0);
                if ( selectShipNum >= 0)
                {
                    /*
                    selectShip = *gSpaceObjectData + selectShipNum;
                    SetScreenLabelObject( globals()->gDestinationLabel, selectShip);
                    if ( selectShipNum == globals()->gPlayerShipNumber)
                    {
                        SetScreenLabelAge( globals()->gDestinationLabel, kLabelOffVisibleTime);
                    }
                    PlayVolumeSound(  kComputerBeep1, kMediumLoudVolume, kMediumPersistence, kLowPrioritySound);
                    SetAdmiralDestinationObject( globals()->gPlayerAdmiralNumber, selectShipNum, kObjectDestinationType);
                    if ( selectShip->attributes & kIsDestination)
                    {
                        SetScreenLabelString( globals()->gDestinationLabel, GetDestBalanceName( selectShip->destinationObject));
    //                  SetDestinationString( GetDestBalanceName( selectShip->destinationObject), true);
                    } else
                    {
                        GetIndString( s, kSpaceObjectNameResID, selectShip->whichBaseObject + 1);
                        SetScreenLabelString( globals()->gDestinationLabel, (anyCharType *)s);
    //                  SetDestinationString( s, true);
                    }

                    SetObjectDestination( theShip, nil);
                    */
                    if ( !(globals()->gOptions & kOptionNetworkOn))
                    {
                        SetPlayerSelectShip( selectShipNum, true, globals()->gPlayerAdmiralNumber);
                    } else
                    {
#if NETSPROCKET_AVAILABLE
                        if ( !SendSelectMessage( globals()->gGameTime + gNetLatency, selectShipNum, true))
                            StopNetworking();
#endif NETSPROCKET_AVAILABLE
                    }
                }
            } else
            {
                selectShipNum = GetAdmiralConsiderObject( globals()->gPlayerAdmiralNumber);
                selectShipNum = GetSpritePointSelectObject( &bounds, theShip, 0,
                                                        kCanThink | kCanAcceptBuild,
                                                        0, selectShipNum, 1);
                if ( selectShipNum >= 0)
                {
                    /*
                    selectShip = *gSpaceObjectData + selectShipNum;
                    SetScreenLabelObject( globals()->gSelectionLabel, selectShip);
                    if ( selectShipNum == globals()->gPlayerShipNumber)
                    {
                        SetScreenLabelAge( globals()->gSelectionLabel, kLabelOffVisibleTime);
                    }
                    PlayVolumeSound(  kComputerBeep1, kMediumLoudVolume, kMediumPersistence, kLowPrioritySound);
                    SetAdmiralConsiderObject( globals()->gPlayerAdmiralNumber, selectShipNum);
                    if ( selectShip->attributes & kIsDestination)
                    {
                        SetScreenLabelString( globals()->gSelectionLabel, GetAdmiralBuildAtName( globals()->gPlayerAdmiralNumber));
                    } else
                    {
                        GetIndString( s, kSpaceObjectNameResID, selectShip->whichBaseObject + 1);
                        SetScreenLabelString( globals()->gSelectionLabel, (anyCharType *)s);
                    }
                    */
                    if ( !(globals()->gOptions & kOptionNetworkOn))
                    {
                        SetPlayerSelectShip( selectShipNum, false, globals()->gPlayerAdmiralNumber);
                    } else
                    {
#if NETSPROCKET_AVAILABLE
                        if ( !SendSelectMessage( globals()->gGameTime + gNetLatency, selectShipNum, false))
                            StopNetworking();
#endif NETSPROCKET_AVAILABLE
                    }
                }
            }
        }
    }
}

void SetPlayerSelectShip( long whichShip, bool target, long admiralNumber)
{
    spaceObjectType *selectShip = *gSpaceObjectData + whichShip,
                    *theShip = GetAdmiralFlagship( admiralNumber);
    Str255          s;

    if ( admiralNumber == globals()->gPlayerAdmiralNumber)
    {
        globals()->lastSelectedObject = whichShip;
        globals()->lastSelectedObjectID = selectShip->id;
        globals()->destKeyUsedForSelection = true;
        WriteDebugLine("\plast sel");
    }
    if ( target)
    {
        SetAdmiralDestinationObject( admiralNumber, whichShip, kObjectDestinationType);
        if ( admiralNumber == globals()->gPlayerAdmiralNumber)
        {
            SetScreenLabelObject( globals()->gDestinationLabel, selectShip);
            if ( whichShip == globals()->gPlayerShipNumber)
            {
                SetScreenLabelAge( globals()->gDestinationLabel, kLabelOffVisibleTime);
            }
            PlayVolumeSound(  kComputerBeep1, kMediumLoudVolume, kMediumPersistence, kLowPrioritySound);
            if ( selectShip->attributes & kIsDestination)
            {
                CopyPString( s,
                    GetDestBalanceName( selectShip->destinationObject));

                HotKey_AppendString( selectShip, s);

                SetScreenLabelString( globals()->gDestinationLabel,
                    s );
            } else
            {
                GetIndString( s, kSpaceObjectNameResID,
                    selectShip->whichBaseObject + 1);

                HotKey_AppendString( selectShip, s);

                SetScreenLabelString( globals()->gDestinationLabel, s);
            }
        }

        if ( !( theShip->attributes & kOnAutoPilot))
            SetObjectDestination( theShip, nil);
    } else
    {
        SetAdmiralConsiderObject( admiralNumber, whichShip);
        if ( admiralNumber == globals()->gPlayerAdmiralNumber)
        {
            SetScreenLabelObject( globals()->gSelectionLabel, selectShip);
            if ( whichShip == globals()->gPlayerShipNumber)
            {
                SetScreenLabelAge( globals()->gSelectionLabel,
                    kLabelOffVisibleTime);
            }
            PlayVolumeSound(  kComputerBeep1, kMediumLoudVolume, kMediumPersistence, kLowPrioritySound);
            if ( selectShip->attributes & kIsDestination)
            {
                CopyPString( s,
                    GetDestBalanceName( selectShip->destinationObject));

                HotKey_AppendString( selectShip, s);

                SetScreenLabelString( globals()->gSelectionLabel, s
                    );
            } else
            {
                GetIndString( s, kSpaceObjectNameResID, selectShip->whichBaseObject + 1);

                HotKey_AppendString( selectShip, s);

                SetScreenLabelString( globals()->gSelectionLabel, s);
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

    if ( whichAdmiral == globals()->gPlayerAdmiralNumber)
    {
        anObject->attributes &= (~kIsHumanControlled) & (~kIsPlayerShip);
        if ( newShipNumber != globals()->gPlayerShipNumber)
        {
            globals()->gPlayerShipNumber = newShipNumber;
            ResetScrollStars( globals()->gPlayerShipNumber);
        }


        anObject = *gSpaceObjectData + globals()->gPlayerShipNumber;

//      if ( !(globals()->gActiveCheats[whichAdmiral] & kAutoPlayBit))
            anObject->attributes |= (kIsHumanControlled) | (kIsPlayerShip);
//      else
//          anObject->attributes |= kIsPlayerShip;

        if ( newShipNumber == GetAdmiralConsiderObject( globals()->gPlayerAdmiralNumber))
        {
            SetScreenLabelAge( globals()->gSelectionLabel, kLabelOffVisibleTime);
        }
        if ( newShipNumber == GetAdmiralDestinationObject( globals()->gPlayerAdmiralNumber))
        {
            SetScreenLabelAge( globals()->gDestinationLabel, kLabelOffVisibleTime);
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
        if ((theShip->owner == globals()->gPlayerAdmiralNumber) &&
            ( theShip->attributes & kIsHumanControlled))
        {
            GetIndString( s, kMessageStringID, kAutoPilotOffString);
            SetStatusString( s, true, kStatusLabelColor);
        }
    } else
    {
        SetObjectDestination( theShip, nil);
        theShip->attributes |= kOnAutoPilot;
        if ((theShip->owner == globals()->gPlayerAdmiralNumber) &&
            ( theShip->attributes & kIsHumanControlled))
        {
            GetIndString( s, kMessageStringID, kAutoPilotOnString);
            SetStatusString( s, true, kStatusLabelColor);
        }
    }
}

bool IsPlayerShipOnAutoPilot( void)
{
    spaceObjectType *theShip;

    if ( globals()->gPlayerShipNumber < 0) return false;
    theShip = *gSpaceObjectData + globals()->gPlayerShipNumber;
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
        if ( whichAdmiral == globals()->gPlayerAdmiralNumber)
            PlayVolumeSound(  kMorseBeepSound, kMediumVolume, kMediumPersistence, kLowPrioritySound);
    }
}

// bool sourceIsBody was hacked in to use this for xferring control
void PlayerShipBodyExpire( spaceObjectType *theShip, bool sourceIsBody)
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
        if ( globals()->gGameOver >= 0)
        {
            globals()->gGameOver = -180;
//          ShowErrorAny( eContinueOnlyErr, -1, "\pEnding the game because", "\p the player died.", nil, nil, -1, -1, -1, -1, __FILE__, 1);
        }
        if (theShip->owner == globals()->gPlayerAdmiralNumber) {
            globals()->gScenarioWinner.text = kScenarioNoShipTextID + gThisScenario->levelNameStrNum;
        } else {
            globals()->gScenarioWinner.text = 10050 + gThisScenario->levelNameStrNum;
        }
        SetAdmiralFlagship( theShip->owner, -1);
    } else if ( selectShip != nil)
    {
        ChangePlayerShipNumber( theShip->owner, selectShipNum);
    }
}

void HandleTextMessageKeys( KeyMap keyMap, KeyMap lastKeyMap,
    bool *enterMessage)
{
    bool         newKeys = false, anyKeys = false;
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
        bufferMap = globals()->gKeyMapBuffer + globals()->gKeyMapBufferTop;
        for ( h = 0; h < 4; h++)
            (*bufferMap)[h] = keyMap[h];
        if ( mReturnKey( keyMap))
        {
            if ( *enterMessage) *enterMessage = false;
            else *enterMessage = true;
        }
        globals()->gKeyMapBufferTop++;
        if ( globals()->gKeyMapBufferTop >= kKeyMapBufferNum)
            globals()->gKeyMapBufferTop = 0;
    }
}

long HotKey_GetFromObject( spaceObjectType *object)
{
    long    i = 0;

    if ( object == nil) return -1;
    if ( !object->active) return -1;
    while ( i < kHotKeyNum)
    {
        if ( globals()->hotKey[i].objectNum == object->entryNumber)
        {
            if ( globals()->hotKey[i].objectID == object->id)
            {
                return i;
            }
        }
        i++;
    }
    return -1;
}

unsigned char* HotKey_AppendString(spaceObjectType *object, unsigned char* s) {
    long    h = HotKey_GetFromObject( object), keyNum;
    Str255  keyName;

    if ( h < 0) return s;
    keyNum = GetKeyNumFromKeyMap(
        globals()->gKeyControl[h + kFirstHotKeyNum]);
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

    whichShip = GetAdmiralDestinationObject( globals()->gPlayerAdmiralNumber);
    if ( whichShip >= 0)

    {
        selectShip = *gSpaceObjectData + whichShip;

//      if ( admiralNumber == globals()->gPlayerAdmiralNumber)
        {
            SetScreenLabelObject( globals()->gDestinationLabel, selectShip);
            if ( whichShip == globals()->gPlayerShipNumber)
            {
                SetScreenLabelAge( globals()->gDestinationLabel,
                    kLabelOffVisibleTime);
            }
            if ( selectShip->attributes & kIsDestination)
            {
                CopyPString( s,
                    GetDestBalanceName( selectShip->destinationObject));

                HotKey_AppendString( selectShip, s);

                SetScreenLabelString( globals()->gDestinationLabel,
                    s );
            } else
            {
                GetIndString( s, kSpaceObjectNameResID,
                    selectShip->whichBaseObject + 1);

                HotKey_AppendString( selectShip, s);

                SetScreenLabelString( globals()->gDestinationLabel, s);
            }
        }

    }

    whichShip = GetAdmiralConsiderObject( globals()->gPlayerAdmiralNumber);
    if ( whichShip >= 0)
    {
        selectShip = *gSpaceObjectData + whichShip;
//      if ( admiralNumber == globals()->gPlayerAdmiralNumber)
        {
            SetScreenLabelObject( globals()->gSelectionLabel, selectShip);
            if ( whichShip == globals()->gPlayerShipNumber)
            {
                SetScreenLabelAge( globals()->gSelectionLabel,
                    kLabelOffVisibleTime);
            }
            PlayVolumeSound(  kComputerBeep1, kMediumLoudVolume, kMediumPersistence, kLowPrioritySound);
            if ( selectShip->attributes & kIsDestination)
            {
                CopyPString( s,
                    GetDestBalanceName( selectShip->destinationObject));

                HotKey_AppendString( selectShip, s);

                SetScreenLabelString( globals()->gSelectionLabel,
                    s);
            } else
            {
                GetIndString( s, kSpaceObjectNameResID, selectShip->whichBaseObject + 1);

                HotKey_AppendString( selectShip, s);

                SetScreenLabelString( globals()->gSelectionLabel, s);
            }
        }
    }
}

}  // namespace antares
