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

#ifndef ANTARES_ARES_PREFERENCES_HPP_
#define ANTARES_ARES_PREFERENCES_HPP_

#include "Base.h"

namespace sfz { class BinaryReader; }

namespace antares {

#define kUsePublicCopyProtection

#define kKeyControlDataNum  80

static const int kDigitNumber = 8;
struct serialNumberType {
    unsigned char name[76];
    char number[kDigitNumber];

    void read(sfz::BinaryReader* bin);
};

struct Preferences {
    Preferences();
    ~Preferences();

    int32_t             version;
    int16_t             keyMap[kKeyControlDataNum];
    serialNumberType    serialNumber;
    uint32_t            options;
    int16_t             volume;
    uint16_t            minutesPlayed;
    uint16_t            kills;
    uint16_t            losses;
    int16_t             race;
    int16_t             enemyColor;
    int32_t             reserved4;
    Str31               playerName;
    Str31               gameName;
    int32_t             resendDelay;
    int32_t             registeredSetting;
    uint32_t            registeredFlags;
    uint32_t            protocolFlags;
    int16_t             netLevel;
    int16_t             netLatency;

    size_t load_data(const char* data, size_t len);
};

struct startingLevelPreferenceType {
    long                version;
    long                startingLevel;
    unsigned long       levelDone_bit_0_31;
    unsigned long       levelDone_bit_32_63;
    unsigned long       scenarioVersion;
    unsigned long       unused_1;
    unsigned long       unused_2;
    unsigned long       unused_3;
    unsigned long       unused_4;
    unsigned long       unused_5;
    unsigned long       unused_6;
    unsigned long       unused_7;
    unsigned long       unused_8;
};

int SaveKeyControlPreferences( void);
short SaveOptionsPreferences( void);
short SaveAllPreferences( void);
void GetNetPreferences(unsigned char*, unsigned char*, unsigned long *, long *, long *, unsigned long *,
    short *, unsigned short *, unsigned short *, unsigned short *, short *, short *, short *);
OSErr SaveNetPreferences(unsigned char*, unsigned char*, unsigned long, long, long, unsigned long,
    short, unsigned short, unsigned short, unsigned short, short, short, short);

}  // namespace antares

#endif // ANTARES_ARES_PREFERENCES_HPP_
