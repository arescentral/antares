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

#include <Base.h>

#include "CopyProtection.h"

#pragma options align=mac68k

//#define   kUseAlphaCopyProtection
#define kUsePublicCopyProtection

// Ares Preferences.h

#define kKeyControlDataNum  80//40

/*
#define kMusicDuringGame            0x00000001
#define kMusicDuringIdle            0x00000002
#define kQuickDrawOnly              0x00000004
#define kRowSkip                    0x00000008
#define kBlackground                0x00000010
#define kNoScaleUp                  0x00000020
#define kPBit7                      0x00000040
#define kPBit8                      0x00000080
#define kPBit9                      0x00000100
#define kPBit10                     0x00000200
#define kPBit11                     0x00000400
#define kPBit12                     0x00000800
#define kPBit13                     0x00001000
#define kPBit14                     0x00002000
#define kPBit15                     0x00004000
#define kPBit16                     0x00008000
#define kPBit17                     0x00010000
#define kPBit18                     0x00020000
#define kPBit19                     0x00040000
#define kPBit20                     0x00080000
#define kPBit21                     0x00100000
#define kPBit22                     0x00200000
#define kPBit23                     0x00400000
#define kPBit24                     0x00800000
#define kPBit25                     0x01000000
#define kPBit26                     0x02000000
#define kPBit27                     0x04000000
#define kPBit28                     0x08000000
#define kPBit29                     0x10000000
#define kPBit30                     0x20000000
#define kPBit31                     0x40000000
#define kPBit32                     0x80000000
*/

/*
typedef struct
{
    long                version;
    short               keyMap[kKeyControlDataNum];
    serialNumberType    serialNumber;
    unsigned long       options;
    long                startingLevel;
    short               volume;
    short               reservedShort1;
    long                reserved2;
    long                reserved3;
    long                reserved4;
    Str31               playerName;
    Str31               gameName;
    long                resendDelay;
    long                registeredSetting;
    unsigned long       registeredFlags;
    unsigned long       protocolFlags;
    long                netLatency;
} preferencesDataType;
*/

struct preferencesDataType {
    long                version;
    short               keyMap[kKeyControlDataNum];
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

int InitPreferences( void);
void PreferencesCleanup( void);
int SaveKeyControlPreferences( void);
short SaveOptionsPreferences( void);
short SaveStartingLevelPreferences( short);
short GetStartingLevelPreference( void);
short SaveAllPreferences( void);
short SaveAnyResourceInPreferences( ResType, short, StringPtr, Handle, Boolean);
short GetAnyResourceFromPreferences( ResType, short, StringPtr, Handle*, Boolean);
void GetNetPreferences( StringPtr, StringPtr, unsigned long *, long *, long *, unsigned long *,
    short *, unsigned short *, unsigned short *, unsigned short *, short *, short *, short *);
OSErr SaveNetPreferences( StringPtr, StringPtr, unsigned long, long, long, unsigned long,
    short, unsigned short, unsigned short, unsigned short, short, short, short);

#pragma options align=reset

#endif // ANTARES_ARES_PREFERENCES_HPP_
