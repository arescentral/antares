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

#include "AresGlobalType.hpp"
#include "BinaryStream.hpp"
#include "Error.hpp"
#include "KeyMapTranslation.hpp"
#include "Options.hpp"
#include "Resource.hpp"

namespace antares {

Preferences::Preferences() {
    Resource rsrc('ArPr', 1000);
    BufferBinaryReader bin(rsrc.data(), rsrc.size());

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

    // we must have existing prefs by now
    // translate key data to be more readable
    for (int i = 0; i < kKeyExtendedControlNum; i++) {
        GetKeyMapFromKeyNum(keyMap[i], globals()->gKeyControl[i]);
    }
    globals()->gOptions = (
            (options & ~(kCarryOverOptionMask)) |
            (globals()->gOptions & kCarryOverOptionMask));
    globals()->gSoundVolume = volume;
}

Preferences::~Preferences() { }

int SaveKeyControlPreferences() {
    Preferences *prefsData = globals()->gPreferencesData.get();
    for (int i = 0; i < kKeyExtendedControlNum; i++) {
         prefsData->keyMap[i] = GetKeyNumFromKeyMap(globals()->gKeyControl[i]);
    }
    return kNoError;
}

short SaveOptionsPreferences() {
    Preferences *prefsData = globals()->gPreferencesData.get();
    prefsData->options = globals()->gOptions;
    return kNoError;
}

short SaveStartingLevelPreferences(short whatLevel) {
    Preferences *prefsData = globals()->gPreferencesData.get();
    if (whatLevel <= 0) {
        fail("An attempt was made to save an invalid starting level number. Save aborted.");
    }
    if (whatLevel != prefsData->startingLevel) {
        prefsData->startingLevel = whatLevel;
    }
    return ( kNoError );
}

short GetStartingLevelPreference() {
    Preferences *prefsData = globals()->gPreferencesData.get();
    return prefsData->startingLevel;
}

short SaveAllPreferences() {
    Preferences *prefsData = globals()->gPreferencesData.get();

    globals()->gOptions = ((prefsData->options & ~(kCarryOverOptionMask)) |
                ( globals()->gOptions & kCarryOverOptionMask));
    globals()->gSoundVolume = prefsData->volume;
    return ( kNoError );
}

void serialNumberType::read(BinaryReader* bin) {
    bin->read(name, 76);
    bin->read(number, kDigitNumber);
}

}  // namespace antares
