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
#include "BinaryStream.hpp"
#include "ConditionalMacros.h"
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

//KeyMap            globals()->gKeyControl[kKeyControlNum];
//short         globals()->gPreferenceRefNum = 0;
//Handle            globals()->gPreferencesData = nil;//gKeyMapData = nil, gSerialNumber = nil;
//unsigned long globals()->gOptions = kDefaultOptions;

OSErr HardWireNewPrefsdata( void);
OSErr PrefsUpdate_PreHotKeys(TypedHandle<preferencesDataType> prefsData);
long GetFirstKeyConflict(preferencesDataType *prefs);
short SaveAnyResourceInPreferences(
        ResType, short, unsigned char*, TypedHandle<startingLevelPreferenceType>, Boolean);
short GetAnyResourceFromPreferences(
        ResType, short, unsigned char*, TypedHandle<startingLevelPreferenceType>*, Boolean);

int InitPreferences( void)

{
    OSErr               oserr;
    short               i, resID = kPreferencesResID;
    preferencesDataType *prefsData;
    TypedHandle<preferencesDataType> tempData;

    // first open preferences file
    if (!EasyOpenPreferenceFile( kAresPreferencesFileName, kPreferenceFileCreator,
                            kPreferenceFileType, &globals()->gPreferenceRefNum))
    {
        ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, PREFERENCES_ERROR, -1, -1, -1, __FILE__, 2);
        return ( PREFERENCES_ERROR);
    }

    // try to get existing prefs
    resID = kPreferencesResID;
    oserr = ReadPreference( globals()->gPreferenceRefNum, kPreferenceDataType, &resID, nil,
        &globals()->gPreferencesData);

    // if we have an old version and there's a key conflict (since we added new keys
    // but forgot to change pref version) then get factory keys
    if ((oserr == noErr) && (globals()->gPreferencesData.get() != nil))
    {
        long    keyConflict = -1, lastKeyConflict = -1;

        if ((*globals()->gPreferencesData)->version > kCurrentPreferencesVersion)
        {
            ShowErrorAny( eQuitErr, kErrorStrID, "\pCould use the current preferences file "
                                                "because it was created by a more recent version "
                                                "of Ares.", nil, nil, nil, -1, -1, -1, -1, __FILE__, 2);

            return ( PREFERENCES_ERROR);
        }

        if ((*globals()->gPreferencesData)->version < kHotKeyPreferencesVersion)
        {
            PrefsUpdate_PreHotKeys( globals()->gPreferencesData);
        }

//      if ((*globals()->gPreferencesData)->version <= 0x00000011)
        {
            (*globals()->gPreferencesData)->version = kCurrentPreferencesVersion;
            do
            {
                lastKeyConflict = keyConflict;

                keyConflict = GetFirstKeyConflict(*globals()->gPreferencesData);

                if ( keyConflict >= 0)
                {
                    tempData.load_resource(kPreferenceDataType, kFactoryPreferenceResID);
                    oserr = ResError();

                    // check to see if we got factory prefs
                    if ((tempData.get() != nil) && (oserr == noErr)) {
                        i = (*tempData)->keyMap[keyConflict];
                        (*globals()->gPreferencesData)->keyMap[keyConflict] = i;

                        while ((GetFirstKeyConflict(*globals()->gPreferencesData) == keyConflict)
                                && (i < 128))
                        {
                            i++;
                            (*globals()->gPreferencesData)->keyMap[keyConflict] = i;
                        }
                        tempData.destroy();
                    }

                }
            } while (( keyConflict >= 0) && ( lastKeyConflict != keyConflict));
        }
    }

    // check to make sure we got existing prefs
    if ((oserr != noErr) || (globals()->gPreferencesData.get() == nil))
    {
        // if we got an error, scrap the prefs and try anew
        if (globals()->gPreferencesData.get() != nil)
        {
            globals()->gPreferencesData.destroy();
            DeletePreference( globals()->gPreferenceRefNum, kPreferenceDataType, kPreferencesResID);
        }

        // try to get factory prefs
        globals()->gPreferencesData.load_resource(kPreferenceDataType, kFactoryPreferenceResID);
        oserr = ResError();

        // check to see if we got factory prefs
        if ((globals()->gPreferencesData.get() == nil) || (oserr != noErr))
        {
            // if we didn't, make 'em from scratch
            ShowErrorAny( eContinueOnlyErr, kErrorStrID, nil, nil, nil, nil, kFactoryPrefsError, -1, -1, -1, __FILE__, 11);
            if ( HardWireNewPrefsdata() != noErr)
            {
                ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, PREFERENCES_ERROR, -1, -1, -1, __FILE__, 17);
                return( -1);
            }

        }

        // write the new/factory prefs to the prefs file
        resID = kPreferencesResID;
        oserr = WritePreference( globals()->gPreferenceRefNum, kPreferenceDataType, &resID, nil,
            globals()->gPreferencesData);
        if ( oserr != noErr)
        {
            ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, PREFERENCES_ERROR, -1, -1, -1, __FILE__, 12);
            return ( PREFERENCES_ERROR);
        }

    } else // we loaded existing prefs just fine
    {
        prefsData = *globals()->gPreferencesData;
        // if the version is newer than the what we think is current, freak out
        if ( prefsData->version > kCurrentPreferencesVersion)
        {
            ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, kNewerPrefsError, -1, -1, -1, __FILE__, 15);
            return ( PREFERENCES_ERROR);
        } else if ( prefsData->version < kCurrentPreferencesVersion)
        {
            // if we have an older prefs, delete it and start from scratch
            globals()->gPreferencesData.destroy();
            DeletePreference( globals()->gPreferenceRefNum, kPreferenceDataType, kPreferencesResID);

            globals()->gPreferencesData.load_resource(kPreferenceDataType, kFactoryPreferenceResID);
            oserr = ResError();

            // check to see if we got factory prefs
            if ((globals()->gPreferencesData.get() == nil) || (oserr != noErr))
            {
                ShowErrorAny( eContinueOnlyErr, kErrorStrID, nil, nil, nil, nil, kFactoryPrefsError, -1, -1, -1, __FILE__, 16);
                if ( HardWireNewPrefsdata() != noErr)
                {
                    ShowErrorAny( eQuitErr, kErrorStrID, nil, nil, nil, nil, PREFERENCES_ERROR, -1, -1, -1, __FILE__, 17);
                    return( -1);
                }

            }
            // write the new/factory prefs to the prefs file
            resID = kPreferencesResID;
            oserr = WritePreference(globals()->gPreferenceRefNum, kPreferenceDataType, &resID,
                    nil, globals()->gPreferencesData);
            if ( oserr != noErr)
            {

                ShowErrorOfTypeOccurred( eQuitErr, kErrorStrID, kWritePrefsError, oserr, __FILE__, 12);
                return ( PREFERENCES_ERROR);
            }
        }
    }
    // we must have existing prefs by now
    // translate key data to be more readable
    prefsData = *globals()->gPreferencesData;
    for ( i = 0; i < kKeyExtendedControlNum; i++)
    {
        GetKeyMapFromKeyNum( prefsData->keyMap[i], globals()->gKeyControl[i]);
    }
    globals()->gOptions = ((prefsData->options & ~(kCarryOverOptionMask)) |
                ( globals()->gOptions & kCarryOverOptionMask));
    globals()->gSoundVolume = prefsData->volume;

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
        oserr = WritePreference( globals()->gPreferenceRefNum, kPreferenceDataType, &resID, globals()->gPreferencesData);
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
    globals()->gOptions &= ~kOptionNoSinglePlayer;
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
            if ( netType != 0) globals()->gOptions |= kOptionNoSinglePlayer;
            GetSerialNumberStringElements( publicSerial->serialString,
                &globals()->gSerialNumerator, &globals()->gSerialDenominator,
                &netType);
            CopyPString( globals()->gUserName, publicSerial->userName);
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
                if ( netType != 0) globals()->gOptions |= kOptionNoSinglePlayer;
                tempData = NewHandle( sizeof( publicSerialType));
                HLock( tempData);
                publicSerial = (publicSerialType *)*tempData;
                CopyPString( publicSerial->userName, userName);
                CopyPString( publicSerial->serialString, serialString);
                oserr = SaveAnyResourceInPreferences(  'tmp1',
                    500,  tempData, false);

                GetSerialNumberStringElements( publicSerial->serialString,
                    &globals()->gSerialNumerator, &globals()->gSerialDenominator,
                    &netType);
                CopyPString( globals()->gUserName, publicSerial->userName);

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

    ClosePreferencesFile( globals()->gPreferenceRefNum);

    if ( oserr != noErr) return kInvalidPublicSerial1;
    return ( kNoError );
}

void PreferencesCleanup( void)

{
    if (globals()->gPreferencesData.get() != nil) {
        globals()->gPreferencesData.destroy();
    }
}

OSErr HardWireNewPrefsdata( void)
{
    preferencesDataType *prefsData;
    short               i;

    globals()->gPreferencesData.create(1);
    if (globals()->gPreferencesData.get() == nil) {
        return -1;
    }
    prefsData = *globals()->gPreferencesData;
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

OSErr PrefsUpdate_PreHotKeys(TypedHandle<preferencesDataType> prefsData) {
    OSErr                                   error;
    long                                    i;
    preferencesDataType*                    newPrefs;
    preferencesDataTypeVersion0x00000012    oldPrefs;
    TypedHandle<preferencesDataType>        tempData;

    if (prefsData.get() == nil) {
        return paramErr;
    }
    BlockMove( *prefsData, &oldPrefs, sizeof( preferencesDataTypeVersion0x00000012));
    prefsData.destroy();
    prefsData.create(1);
    if (prefsData.get() == nil) {
        return memFullErr;
    }
    error = MemError();
    if ( error != noErr) return error;

    newPrefs = *prefsData;
    newPrefs->version = kHotKeyPreferencesVersion;
    for ( i = 0; i < kOldKeyNum; i++)
    {
        newPrefs->keyMap[i] = oldPrefs.keyMap[i];
    }
    {
        tempData.load_resource(kPreferenceDataType, kFactoryPreferenceResID);
        error = ResError();

        if ((tempData.get() != nil) && (error == noErr)) {
            for ( i = kOldKeyNum; i < kKeyControlDataNum; i++)
            {
                (*prefsData)->keyMap[i] = (*tempData)->keyMap[i];;
            }
            tempData.destroy();
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
    preferencesDataType *prefsData = *globals()->gPreferencesData;

    if (!EasyOpenPreferenceFile( kAresPreferencesFileName, kPreferenceFileCreator,
                            kPreferenceFileType, &globals()->gPreferenceRefNum))
    {
        ShowErrorAny( eContinueOnlyErr, kErrorStrID, nil, nil, nil, nil, PREFERENCES_ERROR, -1, -1, -1, __FILE__, 15);
        return ( PREFERENCES_ERROR);
    }

    for ( i = 0; i < kKeyExtendedControlNum; i++)
    {
         prefsData->keyMap[i] = GetKeyNumFromKeyMap( globals()->gKeyControl[i]);
    }

    oserr = WritePreference( globals()->gPreferenceRefNum, kPreferenceDataType, &resID, nil,
        globals()->gPreferencesData);
    if ( oserr != noErr)
    {
        ShowErrorOfTypeOccurred( eContinueOnlyErr, kErrorStrID, kWritePrefsError, oserr, __FILE__, 15);
        ClosePreferencesFile( globals()->gPreferenceRefNum);
        return ( PREFERENCES_ERROR);
    }

    ClosePreferencesFile( globals()->gPreferenceRefNum);
    return ( kNoError );
}

short SaveOptionsPreferences( void)

{
    OSErr               oserr;
    short               resID = kPreferencesResID;
    preferencesDataType *prefsData = *globals()->gPreferencesData;

    if (!EasyOpenPreferenceFile( kAresPreferencesFileName, kPreferenceFileCreator,
                            kPreferenceFileType, &globals()->gPreferenceRefNum))
    {
        ShowErrorAny( eContinueOnlyErr, kErrorStrID, nil, nil, nil, nil, PREFERENCES_ERROR, -1, -1, -1, __FILE__, 25);
        return ( PREFERENCES_ERROR);
    }

    prefsData->options = globals()->gOptions;

    oserr = WritePreference( globals()->gPreferenceRefNum, kPreferenceDataType, &resID, nil,
        globals()->gPreferencesData);
    if ( oserr != noErr)
    {
        ShowErrorOfTypeOccurred( eContinueOnlyErr, kErrorStrID, kWritePrefsError, oserr, __FILE__, 19);
        return ( PREFERENCES_ERROR);
    }

    ClosePreferencesFile( globals()->gPreferenceRefNum);
    return ( kNoError );
}

short SaveStartingLevelPreferences( short whatLevel)

{
    OSErr               oserr;
    short               resID = kPreferencesResID;
    preferencesDataType *prefsData = *globals()->gPreferencesData;

    if ( whatLevel <= 0)
    {
        ShowErrorAny( eContinueErr, -1, "\pAn attempt was made to save an "
            "invalid starting level number. Save aborted.",
            "\p", nil, nil, -1, -1, -1, -1, __FILE__, 1);
        return paramErr;
    }
    if ( globals()->externalFileRefNum < 0)
    {
        if ( whatLevel != prefsData->startingLevel)
        {
            if (!EasyOpenPreferenceFile( kAresPreferencesFileName, kPreferenceFileCreator,
                                    kPreferenceFileType, &globals()->gPreferenceRefNum))
            {
                ShowErrorAny( eContinueOnlyErr, kErrorStrID, nil, nil, nil, nil, PREFERENCES_ERROR, -1, -1, -1, __FILE__, 20);
                return ( PREFERENCES_ERROR);
            }

            prefsData->startingLevel = whatLevel;

            oserr = WritePreference( globals()->gPreferenceRefNum, kPreferenceDataType, &resID,
                nil, globals()->gPreferencesData);
            if ( oserr != noErr)
            {
                ShowErrorOfTypeOccurred( eContinueOnlyErr, kErrorStrID, kWritePrefsError, oserr, __FILE__, 33);
                return ( PREFERENCES_ERROR);
            }

            ClosePreferencesFile( globals()->gPreferenceRefNum);
        }
        return ( kNoError );
    } else
    {
        TypedHandle<startingLevelPreferenceType> data;
        short   currentLevel = GetStartingLevelPreference();

        if ( whatLevel <= currentLevel) return kNoError;
        data.create(1);
        if (data.get() != nil) {
            (*data)->version = kCurrentPreferencesVersion;
            (*data)->startingLevel = whatLevel;
            (*data)->scenarioVersion = globals()->scenarioFileInfo.version;
            oserr = SaveAnyResourceInPreferences( 'aefL', 0, globals()->externalFileSpec.name,
                    data, true);
            data.destroy();
            return kNoError;
        } else return memFullErr;
    }
}

short GetStartingLevelPreference( void)
{
    preferencesDataType *prefsData = *globals()->gPreferencesData;

    if ( !Ambrosia_Is_Registered())
    {
        if (( globals()->externalFileSpec.name[0] == 0) ||
            ( globals()->externalFileRefNum == -1))
        {
            if ( prefsData->startingLevel >= 9) return 9;
            else return prefsData->startingLevel;
        } else return 1;
    }


    if (( globals()->externalFileSpec.name[0] != 0) &&
        (globals()->externalFileRefNum != -1))
    {
        TypedHandle<startingLevelPreferenceType> data;
        OSErr   error;
        long    result;

        error = GetAnyResourceFromPreferences( 'aefL', 0,
            globals()->externalFileSpec.name, &data, true);

        if ((error == noErr) && (data.get() != nil)) {
            result = (*data)->startingLevel;
            data.destroy();
            return result;
        }

        if (data.get() != nil) {
            data.destroy();
        }

        return 1;
    }

//  if ( (!(Ambrosia_Is_Registered())) && ( prefsData->startingLevel >= 9)) return 9;
    return ( prefsData->startingLevel);
}

short SaveAllPreferences( void)

{
    OSErr               oserr;
    short               resID = kPreferencesResID;
    preferencesDataType *prefsData = *globals()->gPreferencesData;

    if (!EasyOpenPreferenceFile( kAresPreferencesFileName, kPreferenceFileCreator,
                            kPreferenceFileType, &globals()->gPreferenceRefNum))
    {
        ShowErrorAny( eContinueOnlyErr, kErrorStrID, nil, nil, nil, nil, PREFERENCES_ERROR, -1, -1, -1, __FILE__, 27);
        return ( PREFERENCES_ERROR);
    }

    oserr = WritePreference( globals()->gPreferenceRefNum, kPreferenceDataType, &resID,
        nil, globals()->gPreferencesData);
    if ( oserr != noErr)
    {
        ShowErrorOfTypeOccurred( eContinueOnlyErr, kErrorStrID, kWritePrefsError, oserr, __FILE__, 34);
        return ( PREFERENCES_ERROR);
    }

    ClosePreferencesFile( globals()->gPreferenceRefNum);

    globals()->gOptions = ((prefsData->options & ~(kCarryOverOptionMask)) |
                ( globals()->gOptions & kCarryOverOptionMask));
    globals()->gSoundVolume = prefsData->volume;
    return ( kNoError );
}

short SaveAnyResourceInPreferences(
        ResType resType, short resID, unsigned char* name,
        TypedHandle<startingLevelPreferenceType> data, Boolean openAndClose)
{
    OSErr           oserr;

    if ( openAndClose)
    {
        if (!EasyOpenPreferenceFile( kAresPreferencesFileName, kPreferenceFileCreator,
                                kPreferenceFileType, &globals()->gPreferenceRefNum))
        {
            ShowErrorAny( eContinueOnlyErr, kErrorStrID, nil, nil, nil, nil, PREFERENCES_ERROR, -1, -1, -1, __FILE__, 29);
            return ( PREFERENCES_ERROR);
        }
    }

    oserr = WritePreference( globals()->gPreferenceRefNum, resType, &resID, name, data);
    if ( oserr != noErr)
    {
        ShowErrorOfTypeOccurred( eContinueOnlyErr, kErrorStrID, kWritePrefsError, oserr, __FILE__, 35);
        ClosePreferencesFile( globals()->gPreferenceRefNum);
        return ( PREFERENCES_ERROR);
    }

    if ( openAndClose)
    {
        ClosePreferencesFile( globals()->gPreferenceRefNum);
    }

    return ( kNoError );
}

short GetAnyResourceFromPreferences(
        ResType resType, short resID, unsigned char* name,
        TypedHandle<startingLevelPreferenceType>* data, Boolean openAndClose)
{
    OSErr           oserr;

    if (data->get() != nil) {
        data->destroy();
    }

    if ( openAndClose)
    {
        if (!EasyOpenPreferenceFile( kAresPreferencesFileName, kPreferenceFileCreator,
                                kPreferenceFileType, &globals()->gPreferenceRefNum))
        {
            ShowErrorAny( eContinueOnlyErr, kErrorStrID, nil, nil, nil, nil, PREFERENCES_ERROR, -1, -1, -1, __FILE__, 29);
            return ( PREFERENCES_ERROR);
        }
    }

    oserr = ReadPreference( globals()->gPreferenceRefNum, resType, &resID, name, data);
    if ( oserr != noErr)
    {
        ShowErrorOfTypeOccurred( eContinueOnlyErr, kErrorStrID, PREFERENCES_ERROR, oserr, __FILE__, 35);
        ClosePreferencesFile( globals()->gPreferenceRefNum);
        return ( PREFERENCES_ERROR);
    }

    if ( openAndClose)
    {
        ClosePreferencesFile( globals()->gPreferenceRefNum);
    }

    return ( kNoError );
}

void GetNetPreferences(unsigned char* playerName, unsigned char* gameName,
    unsigned long *protocolFlags, long *resendDelay, long *registeredSetting,
    unsigned long *registeredFlags, short *netLatency,
    unsigned short *minutesPlayed, unsigned short *kills, unsigned short *losses,
    short *race, short *enemyColor, short *netLevel)

{
    preferencesDataType *prefsData = *globals()->gPreferencesData;

    CopyAnyCharPString(playerName, prefsData->playerName);
    CopyAnyCharPString(gameName, prefsData->gameName);
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

OSErr SaveNetPreferences(unsigned char* playerName, unsigned char* gameName,
    unsigned long protocolFlags, long resendDelay, long registeredSetting,
    unsigned long registeredFlags, short netLatency, unsigned short minutesPlayed,
    unsigned short kills, unsigned short losses, short netRace, short enemyColor,
    short netLevel)

{
    OSErr               oserr;
    short               resID = kPreferencesResID;
    preferencesDataType *prefsData = *globals()->gPreferencesData;

    if (!EasyOpenPreferenceFile( kAresPreferencesFileName, kPreferenceFileCreator,
                            kPreferenceFileType, &globals()->gPreferenceRefNum))
    {
        ShowErrorAny( eContinueOnlyErr, kErrorStrID, nil, nil, nil, nil, PREFERENCES_ERROR, -1, -1, -1, __FILE__, 31);
        return ( PREFERENCES_ERROR);
    }

    CopyAnyCharPString(prefsData->playerName, playerName);
    CopyAnyCharPString(prefsData->gameName, gameName);
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
    oserr = WritePreference( globals()->gPreferenceRefNum, kPreferenceDataType, &resID, nil,
        globals()->gPreferencesData);
    if ( oserr != noErr)
    {
        ShowErrorOfTypeOccurred( eContinueOnlyErr, kErrorStrID, kWritePrefsError, oserr, __FILE__, 37);
        return ( oserr);
    }

    ClosePreferencesFile( globals()->gPreferenceRefNum);
    return ( noErr );

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

size_t preferencesDataType::load_data(const char* data, size_t len) {
    BufferBinaryReader bin(data, len);

    bin.read(&version);
    bin.read(keyMap, kKeyControlDataNum);
    bin.read(&serialNumber);
    bin.read(&options);
    bin.read(&startingLevel);
    bin.read(&volume);
    bin.read(&minutesPlayed);
    bin.read(&kills);
    bin.read(&losses);
    bin.read(&race);
    bin.read(&enemyColor);
    bin.discard(4);
    bin.read(playerName, 32);
    bin.read(gameName, 32);
    bin.read(&resendDelay);
    bin.read(&registeredSetting);
    bin.read(&registeredFlags);
    bin.read(&protocolFlags);
    bin.read(&netLevel);
    bin.read(&netLatency);

    return bin.bytes_read();
}

void serialNumberType::read(BinaryReader* bin) {
    bin->read(name, 76);
    bin->read(number, kDigitNumber);
}
