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

// Ares Preferences

#include "AresPreferences.hpp"

#include "AltPreferences.h"
#include "AmbrosiaSerial.h"
#include "AnyChar.hpp"
#include "AresGlobalType.hpp"
#include "AresNetworkSprocket.hpp"
#include "ConditionalMacros.h"
#include "CopyProtection.h" // is included in prefs
#include "Debug.hpp"
#include "Error.hpp"
#include "KeyMapTranslation.hpp"
#include "Options.hpp"
//#include "publicserial.h"
//#include "PublicSerialDialog.h"
#include "Resources.h"
#include "StdPrefsLib.hpp"
#include "StringHandling.hpp"

#define kPreferenceFileCreator      'ar12'  // normally 'ar12'
#define kPreferenceFileType         'pref'

#define kPreferenceDataType         'ArPr'

#define kSerialNumberDataType       'nlS2'
#define kPreferencesResID           500
#define kFactoryPreferenceResID     1000

#define kStringResID                700
#define kPlayerNameNum              1
#define kGameNameNum                2

#define kDefaultRace                100 // = Ishiman
#define kDefaultColor               1   // = orange

// Nathan Lamont = 4YQHHC6EJ7

#define kOldKeyNum                  34

#define kPreferenceFileError        "\pPREF"

#define kAresPreferencesFileName    "\pAres Preferences"// "\pAres Demo DR2 Preferences"


#define kHotKeyPreferencesVersion   0x00000013
#define kCurrentPreferencesVersion  kHotKeyPreferencesVersion   //0x00000012

#define kNoNetPrefsVersion          0x00000010

#define kKeyControlDataNum_Pre0x13  40

extern aresGlobalType   *gAresGlobal;
//extern long       gAresGlobal->gSoundVolume;

struct preferencesDataTypeVersion0x00000010 {
    long                version;
    short               keyMap[kKeyControlDataNum_Pre0x13];
    serialNumberType    serialNumber;
    unsigned long       options;
    long                startingLevel;
    short               volume;
    short               reservedShort1;
    long                reserved2;
    long                reserved3;
    long                reserved4;
};

struct preferencesDataTypeVersion0x00000012 {
    long                version;
    short               keyMap[kKeyControlDataNum_Pre0x13];
    serialNumberType    serialNumber;
    unsigned long       options;
    long                startingLevel;
    short               volume;
    unsigned short      minutesPlayed;
    unsigned short      kills;
    unsigned short      losses;
    short               race;
    short               enemyColor;
    long                reserved4;
    Str31               playerName;
    Str31               gameName;
    long                resendDelay;
    long                registeredSetting;
    unsigned long       registeredFlags;
    unsigned long       protocolFlags;
    short               netLevel;
    short               netLatency;
//  long                netLatency;
}; // pre hotkeys

struct publicSerialType {
    Str255              userName;
    Str255              serialString;
    unsigned long       reserved1;
    unsigned long       reserved2;
    unsigned long       reserved3;
    unsigned long       reserved4;
    unsigned long       reserved5;
};

//KeyMap            gAresGlobal->gKeyControl[kKeyControlNum];
//short         gAresGlobal->gPreferenceRefNum = 0;
//Handle            gAresGlobal->gPreferencesData = nil;//gKeyMapData = nil, gSerialNumber = nil;
//unsigned long gAresGlobal->gOptions = kDefaultOptions;

OSErr HardWireNewPrefsdata( void);
OSErr PrefsUpdate_PreHotKeys(preferencesDataType** prefsData);
Boolean IsResourceHandle(preferencesDataType** theHandle);
long GetFirstKeyConflict(preferencesDataType *prefs);

int InitPreferences( void)

{
    OSErr               oserr;
    short               i, resID = kPreferencesResID;
    preferencesDataType *prefsData;
    preferencesDataType**   tempData;

    // first open preferences file
    if (!EasyOpenPreferenceFile( kAresPreferencesFileName, kPreferenceFileCreator,
                            kPreferenceFileType, &gAresGlobal->gPreferenceRefNum))
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, PREFERENCES_ERROR, -1, -1, -1, __FILE__, 2);
        return ( PREFERENCES_ERROR);
    }

    // try to get existing prefs
    resID = kPreferencesResID;
    oserr = ReadPreference( gAresGlobal->gPreferenceRefNum, kPreferenceDataType, &resID, nil,
        reinterpret_cast<Handle*>(&gAresGlobal->gPreferencesData));

    // if we have an old version and there's a key conflict (since we added new keys
    // but forgot to change pref version) then get factory keys
    if (( oserr == noErr) && ( gAresGlobal->gPreferencesData != nil))
    {
        long    keyConflict = -1, lastKeyConflict = -1;

        if ((*gAresGlobal->gPreferencesData)->version > kCurrentPreferencesVersion)
        {
            ShowErrorAny( eQuitErr, kErrorStrID, "\pCould use the current preferences file "
                                                "because it was created by a more recent version "
                                                "of Ares.", nil, nil, nil, -1, -1, -1, -1, __FILE__, 2);

            return ( PREFERENCES_ERROR);
        }

        if ((*gAresGlobal->gPreferencesData)->version < kHotKeyPreferencesVersion)
        {
            PrefsUpdate_PreHotKeys( gAresGlobal->gPreferencesData);
        }

//      if ((*gAresGlobal->gPreferencesData)->version <= 0x00000011)
        {
            (*gAresGlobal->gPreferencesData)->version = kCurrentPreferencesVersion;
            do
            {
                lastKeyConflict = keyConflict;

                keyConflict = GetFirstKeyConflict(*gAresGlobal->gPreferencesData);

                if ( keyConflict >= 0)
                {
                    tempData = reinterpret_cast<preferencesDataType**>(GetResource( kPreferenceDataType, kFactoryPreferenceResID));
                    oserr = ResError();

                    // check to see if we got factory prefs
                    if (( tempData != nil) && ( oserr == noErr))
                    {
                        i = (*tempData)->keyMap[keyConflict];
                        (*gAresGlobal->gPreferencesData)->keyMap[keyConflict] = i;

                        while ((GetFirstKeyConflict(*gAresGlobal->gPreferencesData) == keyConflict)
                                && (i < 128))
                        {
                            i++;
                            (*gAresGlobal->gPreferencesData)->keyMap[keyConflict] = i;
                        }
                        ReleaseResource( reinterpret_cast<Handle>(tempData));
                    }

                }
            } while (( keyConflict >= 0) && ( lastKeyConflict != keyConflict));
        }
    }

    // check to make sure we got existing prefs
    if (( oserr != noErr) || ( gAresGlobal->gPreferencesData == nil))
    {
        // if we got an error, scrap the prefs and try anew
        if ( gAresGlobal->gPreferencesData != nil)
        {
            if ( IsResourceHandle( gAresGlobal->gPreferencesData))
                ReleaseResource( reinterpret_cast<Handle>(gAresGlobal->gPreferencesData));
            else
                DisposeHandle( reinterpret_cast<Handle>(gAresGlobal->gPreferencesData));
            gAresGlobal->gPreferencesData = nil;
            DeletePreference( gAresGlobal->gPreferenceRefNum, kPreferenceDataType, kPreferencesResID);
        }

        // try to get factory prefs
        gAresGlobal->gPreferencesData = reinterpret_cast<preferencesDataType**>(GetResource( kPreferenceDataType, kFactoryPreferenceResID));
        oserr = ResError();

        // check to see if we got factory prefs
        if (( gAresGlobal->gPreferencesData == nil) || ( oserr != noErr))
        {
            // if we didn't, make 'em from scratch
            ShowErrorAny( eContinueOnlyErr, kErrorStrID, nil, nil, nil, nil, kFactoryPrefsError, -1, -1, -1, __FILE__, 11);
            if ( HardWireNewPrefsdata() != noErr)
            {
                ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, PREFERENCES_ERROR, -1, -1, -1, __FILE__, 17);
                return( -1);
            }

        } else // we got factory prefs
        {
            if ( IsResourceHandle( gAresGlobal->gPreferencesData))
                DetachResource( reinterpret_cast<Handle>(gAresGlobal->gPreferencesData));
            MoveHHi( reinterpret_cast<Handle>(gAresGlobal->gPreferencesData));
            HLock( reinterpret_cast<Handle>(gAresGlobal->gPreferencesData));
        }

        // write the new/factory prefs to the prefs file
        resID = kPreferencesResID;
        oserr = WritePreference( gAresGlobal->gPreferenceRefNum, kPreferenceDataType, &resID, nil,
            reinterpret_cast<Handle>(gAresGlobal->gPreferencesData));
        if ( oserr != noErr)
        {
            ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, PREFERENCES_ERROR, -1, -1, -1, __FILE__, 12);
            return ( PREFERENCES_ERROR);
        }

    } else // we loaded existing prefs just fine
    {
        if ( IsResourceHandle( gAresGlobal->gPreferencesData))
            DetachResource( reinterpret_cast<Handle>(gAresGlobal->gPreferencesData));
        MoveHHi( reinterpret_cast<Handle>(gAresGlobal->gPreferencesData));
        HLock( reinterpret_cast<Handle>(gAresGlobal->gPreferencesData));

        prefsData = *gAresGlobal->gPreferencesData;
        // if the version is newer than the what we think is current, freak out
        if ( prefsData->version > kCurrentPreferencesVersion)
        {
            ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kNewerPrefsError, -1, -1, -1, __FILE__, 15);
            return ( PREFERENCES_ERROR);
        } else if ( prefsData->version < kCurrentPreferencesVersion)
        {
            // if we have an older prefs, delete it and start from scratch
            if ( IsResourceHandle( gAresGlobal->gPreferencesData))
                ReleaseResource( reinterpret_cast<Handle>(gAresGlobal->gPreferencesData));
            else
                DisposeHandle( reinterpret_cast<Handle>(gAresGlobal->gPreferencesData));
            gAresGlobal->gPreferencesData = nil;
            DeletePreference( gAresGlobal->gPreferenceRefNum, kPreferenceDataType, kPreferencesResID);

            gAresGlobal->gPreferencesData = reinterpret_cast<preferencesDataType**>(GetResource( kPreferenceDataType, kFactoryPreferenceResID));
            oserr = ResError();

            // check to see if we got factory prefs
            if (( gAresGlobal->gPreferencesData == nil) || ( oserr != noErr))
            {
                ShowErrorAny( eContinueOnlyErr, kErrorStrID, nil, nil, nil, nil, kFactoryPrefsError, -1, -1, -1, __FILE__, 16);
                if ( HardWireNewPrefsdata() != noErr)
                {
                    ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, PREFERENCES_ERROR, -1, -1, -1, __FILE__, 17);
                    return( -1);
                }

            } else // we got factory prefs
            {
                if ( IsResourceHandle( gAresGlobal->gPreferencesData))
                    DetachResource( reinterpret_cast<Handle>(gAresGlobal->gPreferencesData));
                MoveHHi( reinterpret_cast<Handle>(gAresGlobal->gPreferencesData));
                HLock( reinterpret_cast<Handle>(gAresGlobal->gPreferencesData));
            }
            // write the new/factory prefs to the prefs file
            resID = kPreferencesResID;
            oserr = WritePreference( gAresGlobal->gPreferenceRefNum, kPreferenceDataType, &resID,
                    nil, reinterpret_cast<Handle>(gAresGlobal->gPreferencesData));
            if ( oserr != noErr)
            {

                ShowErrorOfTypeOccurred( eQuitErr, kErrorStrID, kWritePrefsError, oserr, __FILE__, 12);
                return ( PREFERENCES_ERROR);
            }
        }
    }
    // we must have existing prefs by now
    // translate key data to be more readable
    prefsData = *gAresGlobal->gPreferencesData;
    for ( i = 0; i < kKeyExtendedControlNum; i++)
    {
        GetKeyMapFromKeyNum( prefsData->keyMap[i], gAresGlobal->gKeyControl[i]);
    }
    gAresGlobal->gOptions = ((prefsData->options & ~(kCarryOverOptionMask)) |
                ( gAresGlobal->gOptions & kCarryOverOptionMask));
    gAresGlobal->gSoundVolume = prefsData->volume;

////////////////////////////////////////////////////////////// WARNING!
/// The block below is for copy protection.
/// It is now disabled!
#ifdef kUseAlphaCopyProtection
/*
    // make sure we have a legal serial number
    i =  ConfirmSerialNumber( &(prefsData->serialNumber));
    if ( i != GOAL_SUM)
    {
        GetSerialNumberDialog( &(prefsData->serialNumber));
        resID = kPreferencesResID;
        oserr = WritePreference( gAresGlobal->gPreferenceRefNum, kPreferenceDataType, &resID, gAresGlobal->gPreferencesData);
        if ( oserr != noErr)
        {
            ShowErrorOfTypeOccurred( eQuitErr, kErrorStrID, kWritePrefsError, oserr, __FILE__, 13);
            return ( PREFERENCES_ERROR);
        }
        i =  ConfirmSerialNumber( &(prefsData->serialNumber));
        if ( i != GOAL_SUM)
        {
            ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kNameIDIncorrect, -1, -1, -1, __FILE__, 14);
            return ( PREFERENCES_ERROR);
        }
    }
*/  if ( HasGameExpired())
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, EXPIRED_ERROR, -1, -1, -1, __FILE__, 9);
        return( EXPIRED_ERROR);
    }
#endif
// public copy protect enabled!
#ifdef kUsePublicCopyProtection
/*
    tempData = nil;
    gAresGlobal->gOptions &= ~kOptionNoSinglePlayer;
    oserr = GetAnyResourceFromPreferences( 'tmp1', 500, &tempData , false);
    if (( tempData != nil) && ( oserr == noErr))
    {
        HLock( tempData);
        publicSerial = (publicSerialType *)*tempData;
        netType = SerialNumberStringIsValid( publicSerial->serialString);
        if ( netType < 0)
        {
            HUnlock( tempData);
            DisposeHandle( tempData);
            tempData = nil;
        } else
        {
            if ( netType != 0) gAresGlobal->gOptions |= kOptionNoSinglePlayer;
            GetSerialNumberStringElements( publicSerial->serialString,
                &gAresGlobal->gSerialNumerator, &gAresGlobal->gSerialDenominator,
                &netType);
            CopyPString( gAresGlobal->gUserName, publicSerial->userName);
            HUnlock( tempData);
            DisposeHandle( tempData);
            tempData = nil;
        }
    } else oserr = -1;
    if ( oserr != noErr)
    {
        if ( DoPublicSerialDialog( userName, serialString))
        {
            netType = SerialNumberStringIsValid( serialString);
            if ( netType >= 0)
            {
                if ( netType != 0) gAresGlobal->gOptions |= kOptionNoSinglePlayer;
                tempData = NewHandle( sizeof( publicSerialType));
                HLock( tempData);
                publicSerial = (publicSerialType *)*tempData;
                CopyPString( publicSerial->userName, userName);
                CopyPString( publicSerial->serialString, serialString);
                oserr = SaveAnyResourceInPreferences(  'tmp1',
                    500,  tempData, false);

                GetSerialNumberStringElements( publicSerial->serialString,
                    &gAresGlobal->gSerialNumerator, &gAresGlobal->gSerialDenominator,
                    &netType);
                CopyPString( gAresGlobal->gUserName, publicSerial->userName);

                HUnlock( tempData);
                DisposeHandle( tempData);
            } else
            {
                ShowErrorAny( eContinueOnlyErr, kErrorStrID, nil, serialString, nil, nil,
                    kInvalidPublicSerial1, -1, kInvalidPublicSerial2, -1, __FILE__, 14);
                oserr = -1;
            }
        } else oserr = -1;
    }
*/
#endif
////////////////////////////////////////////////////////////// END OF COPY PROTECTION

    ClosePreferencesFile( gAresGlobal->gPreferenceRefNum);

    if ( oserr != noErr) return kInvalidPublicSerial1;
    return ( kNoError );
}

void PreferencesCleanup( void)

{
    if ( gAresGlobal->gPreferencesData != nil) DisposeHandle( reinterpret_cast<Handle>(gAresGlobal->gPreferencesData));
}

OSErr HardWireNewPrefsdata( void)
{
    preferencesDataType *prefsData;
    short               i;

    gAresGlobal->gPreferencesData = reinterpret_cast<preferencesDataType**>(NewHandle( sizeof( preferencesDataType)));
    if ( gAresGlobal->gPreferencesData == nil) return( -1);
    MoveHHi( reinterpret_cast<Handle>(gAresGlobal->gPreferencesData));
    HLock( reinterpret_cast<Handle>(gAresGlobal->gPreferencesData));
    prefsData = *gAresGlobal->gPreferencesData;
    prefsData->version = kCurrentPreferencesVersion;
    for ( i = 0; i < kKeyControlDataNum; i++)
    {
        prefsData->keyMap[i] = 0;
    }
    prefsData->serialNumber.name[0] = 0;
    for ( i = 0; i < kDigitNumber; i++)
    {
        prefsData->serialNumber.number[i] = 0;
    }
    prefsData->options = kDefaultOptions;
    prefsData->startingLevel = 1;
    prefsData->volume = 7;
    prefsData->minutesPlayed = 0;
    prefsData->kills = 0;
    prefsData->losses = 0;
    prefsData->netLevel = 0;
    prefsData->reserved4 = 0;
    prefsData->race = kDefaultRace;
    prefsData->enemyColor = kDefaultColor;
    GetIndString( prefsData->playerName, kStringResID, kPlayerNameNum);
    GetIndString( prefsData->gameName, kStringResID, kGameNameNum);
    prefsData->resendDelay = 60;
    prefsData->registeredSetting = 1;
    prefsData->registeredFlags = kRegisterResendRequest | kRegisterResend;
    prefsData->netLatency = 6;
    prefsData->protocolFlags = 0;
    return( noErr);
}

OSErr PrefsUpdate_PreHotKeys( preferencesDataType** prefsData)
{
    OSErr                                   error;
    long                                    i;
    preferencesDataType*                    newPrefs;
    preferencesDataTypeVersion0x00000012    oldPrefs;
    preferencesDataType**                   tempData;

    if ( prefsData == nil) return paramErr;
    BlockMove( *prefsData, &oldPrefs, sizeof( preferencesDataTypeVersion0x00000012));
    if ( IsResourceHandle( prefsData))
        ReleaseResource( reinterpret_cast<Handle>(prefsData));
    else
        DisposeHandle( reinterpret_cast<Handle>(prefsData));
    prefsData = reinterpret_cast<preferencesDataType**>(NewHandle( sizeof( preferencesDataType)));
    if ( prefsData == nil) return memFullErr;
    error = MemError();
    if ( error != noErr) return error;

    newPrefs = *prefsData;
    newPrefs->version = kHotKeyPreferencesVersion;
    for ( i = 0; i < kOldKeyNum; i++)
    {
        newPrefs->keyMap[i] = oldPrefs.keyMap[i];
    }
    {
        tempData = reinterpret_cast<preferencesDataType**>(GetResource( kPreferenceDataType, kFactoryPreferenceResID));
        error = ResError();

        if (( tempData != nil) && ( error == noErr))
        {
            for ( i = kOldKeyNum; i < kKeyControlDataNum; i++)
            {
                (*prefsData)->keyMap[i] = (*tempData)->keyMap[i];;
            }
            ReleaseResource( reinterpret_cast<Handle>(tempData));
        }

    }
    newPrefs = *prefsData;

    newPrefs->serialNumber = oldPrefs.serialNumber;
    newPrefs->options = oldPrefs.options;
    newPrefs->startingLevel = oldPrefs.startingLevel;
    newPrefs->volume = oldPrefs.volume;
    newPrefs->minutesPlayed = oldPrefs.minutesPlayed;
    newPrefs->kills = oldPrefs.kills;
    newPrefs->losses = oldPrefs.losses;
    newPrefs->race = oldPrefs.race;
    newPrefs->enemyColor = oldPrefs.enemyColor;
    newPrefs->reserved4 = oldPrefs.reserved4;
    CopyPString( newPrefs->playerName, oldPrefs.playerName);
    CopyPString( newPrefs->gameName, oldPrefs.gameName);
    newPrefs->resendDelay = oldPrefs.resendDelay;
    newPrefs->registeredSetting = oldPrefs.registeredSetting;
    newPrefs->registeredFlags = oldPrefs.registeredFlags;
    newPrefs->protocolFlags = oldPrefs.protocolFlags;
    newPrefs->netLevel = oldPrefs.netLevel;
    newPrefs->netLatency = oldPrefs.netLatency;

    return noErr;
}

int SaveKeyControlPreferences( void)

{
    OSErr           oserr;
    short           i, resID = kPreferencesResID;
    preferencesDataType *prefsData = *gAresGlobal->gPreferencesData;

    if (!EasyOpenPreferenceFile( kAresPreferencesFileName, kPreferenceFileCreator,
                            kPreferenceFileType, &gAresGlobal->gPreferenceRefNum))
    {
        ShowErrorAny( eContinueOnlyErr, kErrorStrID, nil, nil, nil, nil, PREFERENCES_ERROR, -1, -1, -1, __FILE__, 15);
        return ( PREFERENCES_ERROR);
    }

    for ( i = 0; i < kKeyExtendedControlNum; i++)
    {
         prefsData->keyMap[i] = GetKeyNumFromKeyMap( gAresGlobal->gKeyControl[i]);
    }

    oserr = WritePreference( gAresGlobal->gPreferenceRefNum, kPreferenceDataType, &resID, nil,
        reinterpret_cast<Handle>(gAresGlobal->gPreferencesData));
    if ( oserr != noErr)
    {
        ShowErrorOfTypeOccurred( eContinueOnlyErr, kErrorStrID, kWritePrefsError, oserr, __FILE__, 15);
        ClosePreferencesFile( gAresGlobal->gPreferenceRefNum);
        return ( PREFERENCES_ERROR);
    }

    ClosePreferencesFile( gAresGlobal->gPreferenceRefNum);
    return ( kNoError );
}

short SaveOptionsPreferences( void)

{
    OSErr               oserr;
    short               resID = kPreferencesResID;
    preferencesDataType *prefsData = *gAresGlobal->gPreferencesData;

    if (!EasyOpenPreferenceFile( kAresPreferencesFileName, kPreferenceFileCreator,
                            kPreferenceFileType, &gAresGlobal->gPreferenceRefNum))
    {
        ShowErrorAny( eContinueOnlyErr, kErrorStrID, nil, nil, nil, nil, PREFERENCES_ERROR, -1, -1, -1, __FILE__, 25);
        return ( PREFERENCES_ERROR);
    }

    prefsData->options = gAresGlobal->gOptions;

    oserr = WritePreference( gAresGlobal->gPreferenceRefNum, kPreferenceDataType, &resID, nil,
        reinterpret_cast<Handle>(gAresGlobal->gPreferencesData));
    if ( oserr != noErr)
    {
        ShowErrorOfTypeOccurred( eContinueOnlyErr, kErrorStrID, kWritePrefsError, oserr, __FILE__, 19);
        return ( PREFERENCES_ERROR);
    }

    ClosePreferencesFile( gAresGlobal->gPreferenceRefNum);
    return ( kNoError );
}

short SaveStartingLevelPreferences( short whatLevel)

{
    OSErr               oserr;
    short               resID = kPreferencesResID;
    preferencesDataType *prefsData = *gAresGlobal->gPreferencesData;

    if ( whatLevel <= 0)
    {
        ShowErrorAny( eContinueErr, -1, "\pAn attempt was made to save an "
            "invalid starting level number. Save aborted.",
            "\p", nil, nil, -1, -1, -1, -1, __FILE__, 1);
        return paramErr;
    }
    if ( gAresGlobal->externalFileRefNum < 0)
    {
        if ( whatLevel != prefsData->startingLevel)
        {
            if (!EasyOpenPreferenceFile( kAresPreferencesFileName, kPreferenceFileCreator,
                                    kPreferenceFileType, &gAresGlobal->gPreferenceRefNum))
            {
                ShowErrorAny( eContinueOnlyErr, kErrorStrID, nil, nil, nil, nil, PREFERENCES_ERROR, -1, -1, -1, __FILE__, 20);
                return ( PREFERENCES_ERROR);
            }

            prefsData->startingLevel = whatLevel;

            oserr = WritePreference( gAresGlobal->gPreferenceRefNum, kPreferenceDataType, &resID,
                nil, reinterpret_cast<Handle>(gAresGlobal->gPreferencesData));
            if ( oserr != noErr)
            {
                ShowErrorOfTypeOccurred( eContinueOnlyErr, kErrorStrID, kWritePrefsError, oserr, __FILE__, 33);
                return ( PREFERENCES_ERROR);
            }

            ClosePreferencesFile( gAresGlobal->gPreferenceRefNum);
        }
        return ( kNoError );
    } else
    {
        startingLevelPreferenceType** data = nil;
        short   currentLevel = GetStartingLevelPreference();

        if ( whatLevel <= currentLevel) return kNoError;
        data = reinterpret_cast<startingLevelPreferenceType**>(NewHandle( sizeof( startingLevelPreferenceType)));
        if ( data != nil)
        {
            (*data)->version = kCurrentPreferencesVersion;
            (*data)->startingLevel = whatLevel;
            (*data)->scenarioVersion = gAresGlobal->scenarioFileInfo.version;
            oserr = SaveAnyResourceInPreferences( 'aefL', 0, gAresGlobal->externalFileSpec.name,
                    reinterpret_cast<Handle>(data), true);
            DisposeHandle(reinterpret_cast<Handle>(data));
            return kNoError;
        } else return memFullErr;
    }
}

short GetStartingLevelPreference( void)
{
    preferencesDataType *prefsData = *gAresGlobal->gPreferencesData;

    if ( !Ambrosia_Is_Registered())
    {
        if (( gAresGlobal->externalFileSpec.name[0] == 0) ||
            ( gAresGlobal->externalFileRefNum == -1))
        {
            if ( prefsData->startingLevel >= 9) return 9;
            else return prefsData->startingLevel;
        } else return 1;
    }


    if (( gAresGlobal->externalFileSpec.name[0] != 0) &&
        (gAresGlobal->externalFileRefNum != -1))
    {
        startingLevelPreferenceType** data = NULL;
        OSErr   error;
        long    result;

        error = GetAnyResourceFromPreferences( 'aefL', 0,
            gAresGlobal->externalFileSpec.name, reinterpret_cast<Handle*>(&data), true);

        if (( error == noErr) && ( data != nil))
        {
            result = (*data)->startingLevel;
            DisposeHandle(reinterpret_cast<Handle>(data));
            return result;
        }

        if ( data != nil) DisposeHandle(reinterpret_cast<Handle>(data));

        return 1;
    }

//  if ( (!(Ambrosia_Is_Registered())) && ( prefsData->startingLevel >= 9)) return 9;
    return ( prefsData->startingLevel);
}

short SaveAllPreferences( void)

{
    OSErr               oserr;
    short               resID = kPreferencesResID;
    preferencesDataType *prefsData = *gAresGlobal->gPreferencesData;

    if (!EasyOpenPreferenceFile( kAresPreferencesFileName, kPreferenceFileCreator,
                            kPreferenceFileType, &gAresGlobal->gPreferenceRefNum))
    {
        ShowErrorAny( eContinueOnlyErr, kErrorStrID, nil, nil, nil, nil, PREFERENCES_ERROR, -1, -1, -1, __FILE__, 27);
        return ( PREFERENCES_ERROR);
    }

    oserr = WritePreference( gAresGlobal->gPreferenceRefNum, kPreferenceDataType, &resID,
        nil, reinterpret_cast<Handle>(gAresGlobal->gPreferencesData));
    if ( oserr != noErr)
    {
        ShowErrorOfTypeOccurred( eContinueOnlyErr, kErrorStrID, kWritePrefsError, oserr, __FILE__, 34);
        return ( PREFERENCES_ERROR);
    }

    ClosePreferencesFile( gAresGlobal->gPreferenceRefNum);

    gAresGlobal->gOptions = ((prefsData->options & ~(kCarryOverOptionMask)) |
                ( gAresGlobal->gOptions & kCarryOverOptionMask));
    gAresGlobal->gSoundVolume = prefsData->volume;
    return ( kNoError );
}

short SaveAnyResourceInPreferences( ResType resType, short resID, StringPtr name, Handle data,
        Boolean openAndClose)
{
    OSErr           oserr;
    SignedByte      handleState = HGetState(data);

    HLock(data);

    if ( openAndClose)
    {
        if (!EasyOpenPreferenceFile( kAresPreferencesFileName, kPreferenceFileCreator,
                                kPreferenceFileType, &gAresGlobal->gPreferenceRefNum))
        {
            ShowErrorAny( eContinueOnlyErr, kErrorStrID, nil, nil, nil, nil, PREFERENCES_ERROR, -1, -1, -1, __FILE__, 29);
            return ( PREFERENCES_ERROR);
        }
    }

    oserr = WritePreference( gAresGlobal->gPreferenceRefNum, resType, &resID, name, data);
    if ( oserr != noErr)
    {
        ShowErrorOfTypeOccurred( eContinueOnlyErr, kErrorStrID, kWritePrefsError, oserr, __FILE__, 35);
        ClosePreferencesFile( gAresGlobal->gPreferenceRefNum);
        return ( PREFERENCES_ERROR);
    }

    if ( openAndClose)
    {
        ClosePreferencesFile( gAresGlobal->gPreferenceRefNum);
    }

    HSetState(data, handleState);
    return ( kNoError );
}

short GetAnyResourceFromPreferences( ResType resType, short resID, StringPtr name, Handle* data,
        Boolean openAndClose)
{
    OSErr           oserr;

    if ( *data != nil)
    {
        HUnlock(*data);
        DisposeHandle(*data);
    }

    if ( openAndClose)
    {
        if (!EasyOpenPreferenceFile( kAresPreferencesFileName, kPreferenceFileCreator,
                                kPreferenceFileType, &gAresGlobal->gPreferenceRefNum))
        {
            ShowErrorAny( eContinueOnlyErr, kErrorStrID, nil, nil, nil, nil, PREFERENCES_ERROR, -1, -1, -1, __FILE__, 29);
            return ( PREFERENCES_ERROR);
        }
    }

    oserr = ReadPreference( gAresGlobal->gPreferenceRefNum, resType, &resID, name, data);
    if ( oserr != noErr)
    {
        ShowErrorOfTypeOccurred( eContinueOnlyErr, kErrorStrID, PREFERENCES_ERROR, oserr, __FILE__, 35);
        ClosePreferencesFile( gAresGlobal->gPreferenceRefNum);
        return ( PREFERENCES_ERROR);
    }

    if ( openAndClose)
    {
        ClosePreferencesFile( gAresGlobal->gPreferenceRefNum);
    }

    return ( kNoError );
}

void GetNetPreferences( StringPtr playerName, StringPtr gameName,
    unsigned long *protocolFlags, long *resendDelay, long *registeredSetting,
    unsigned long *registeredFlags, short *netLatency,
    unsigned short *minutesPlayed, unsigned short *kills, unsigned short *losses,
    short *race, short *enemyColor, short *netLevel)

{
    preferencesDataType *prefsData = *gAresGlobal->gPreferencesData;

    CopyAnyCharPString( reinterpret_cast<anyCharType *>(playerName), reinterpret_cast<anyCharType *>(prefsData->playerName));
    CopyAnyCharPString( reinterpret_cast<anyCharType *>(gameName), reinterpret_cast<anyCharType *>(prefsData->gameName));
    *protocolFlags = prefsData->protocolFlags;
    *registeredSetting = prefsData->registeredSetting;
    *registeredFlags = prefsData->registeredFlags;
    *netLatency = prefsData->netLatency;
    *resendDelay = prefsData->resendDelay;
    *minutesPlayed = prefsData->minutesPlayed;
    *kills = prefsData->kills;
    *losses = prefsData->losses;
    *race = prefsData->race;
    *enemyColor = prefsData->enemyColor;
    *netLevel = prefsData->netLevel;
}

OSErr SaveNetPreferences( StringPtr playerName, StringPtr gameName,
    unsigned long protocolFlags, long resendDelay, long registeredSetting,
    unsigned long registeredFlags, short netLatency, unsigned short minutesPlayed,
    unsigned short kills, unsigned short losses, short netRace, short enemyColor,
    short netLevel)

{
    OSErr               oserr;
    short               resID = kPreferencesResID;
    preferencesDataType *prefsData = *gAresGlobal->gPreferencesData;

    if (!EasyOpenPreferenceFile( kAresPreferencesFileName, kPreferenceFileCreator,
                            kPreferenceFileType, &gAresGlobal->gPreferenceRefNum))
    {
        ShowErrorAny( eContinueOnlyErr, kErrorStrID, nil, nil, nil, nil, PREFERENCES_ERROR, -1, -1, -1, __FILE__, 31);
        return ( PREFERENCES_ERROR);
    }

    CopyAnyCharPString( reinterpret_cast<anyCharType *>(prefsData->playerName), reinterpret_cast<anyCharType *>(playerName));
    CopyAnyCharPString( reinterpret_cast<anyCharType *>(prefsData->gameName), reinterpret_cast<anyCharType *>(gameName));
    prefsData->protocolFlags = protocolFlags;
    prefsData->registeredSetting = registeredSetting;
    prefsData->registeredFlags = registeredFlags;
    prefsData->netLatency = netLatency;
    prefsData->resendDelay = resendDelay;
    prefsData->minutesPlayed = minutesPlayed;
    prefsData->kills = kills;
    prefsData->losses = losses;
    prefsData->race = netRace;
    prefsData->enemyColor = enemyColor;
    prefsData->netLevel = netLevel;
    oserr = WritePreference( gAresGlobal->gPreferenceRefNum, kPreferenceDataType, &resID, nil,
        reinterpret_cast<Handle>(gAresGlobal->gPreferencesData));
    if ( oserr != noErr)
    {
        ShowErrorOfTypeOccurred( eContinueOnlyErr, kErrorStrID, kWritePrefsError, oserr, __FILE__, 37);
        return ( oserr);
    }

    ClosePreferencesFile( gAresGlobal->gPreferenceRefNum);
    return ( noErr );

}

Boolean
IsResourceHandle(preferencesDataType** theHandle)
{
SInt8   memState;

    memState = HGetState(reinterpret_cast<Handle>(theHandle));

    // *    Check the resource bit in the handle info
    if (memState & 0x20)
        return (true);

    return (false);
}

long GetFirstKeyConflict( preferencesDataType *prefs)
{
    long    i, j;

    for ( i = 0; i < kKeyExtendedControlNum; i++)
    {
        for ( j = i + 1; j < kKeyExtendedControlNum; j++)
        {
            if ( prefs->keyMap[i] == prefs->keyMap[j])
            {
                return i;
            }
        }
    }
    return -1;
}
