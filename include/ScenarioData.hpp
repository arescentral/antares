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

#ifndef ANTARES_SCENARIO_DATA_HPP_
#define ANTARES_SCENARIO_DATA_HPP_

#include "Base.h"

namespace antares {

#define kScenario_Data_ResType          'nlAG'
#define kScenario_Data_ResID            128

#define kScenario_Data_Flag_IsNetworkable       0x00000001
#define kScenario_Data_Flag_HasCustomObjects    0x00000002
#define kScenario_Data_Flag_HasCustomRaces      0x00000004
#define kScenario_Data_Flag_HasCustomScenarios  0x00000008
#define kScenario_Data_Flag_NotOptimized        0x00000010

struct scenarioInfoType {
    int32_t         warpInFlareID;
    int32_t         warpOutFlareID;
    int32_t         playerBodyID;
    int32_t         energyBlobID;
    Str255          downloadURLString;
    Str255          titleString;
    Str255          authorNameString;
    Str255          authorURLString;
    uint32_t        version;
    uint32_t        requiresAresVersion;
    uint32_t        flags;
    uint32_t        checkSum;

    void read(sfz::BinaryReader* bin);
};

}  // namespace antares

#endif // ANTARES_SCENARIO_DATA_HPP_
