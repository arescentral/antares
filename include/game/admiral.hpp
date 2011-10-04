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

#ifndef ANTARES_GAME_ADMIRAL_HPP_
#define ANTARES_GAME_ADMIRAL_HPP_

#include <sfz/sfz.hpp>

#include "data/scenario.hpp"
#include "data/space-object.hpp"
#include "math/fixed.hpp"

namespace antares {

enum {
    kAIsHuman       = 1 << 0,
    kAIsRemote      = 1 << 1,
    kAIsComputer    = 1 << 2,
    kABit4          = 1 << 3,
    kABit5          = 1 << 4,
    kABit6          = 1 << 5,
    kABit7          = 1 << 6,
    kABit8          = 1 << 7,
    kABit9          = 1 << 8,
    kABit10         = 1 << 9,
    kABit11         = 1 << 10,
    kABit12         = 1 << 11,
    kABit13         = 1 << 12,
    kABit14         = 1 << 13,
    kABit15         = 1 << 14,
    kABit16         = 1 << 15,
    kABit17         = 1 << 16,
    kABit18         = 1 << 17,
    kABit19         = 1 << 18,
    kABit20         = 1 << 19,
    kABit21         = 1 << 20,
    kABit22         = 1 << 21,
    kABit23         = 1 << 22,
    kABit24         = 1 << 23,
    kABit25         = 1 << 24,
    kABit26         = 1 << 25,
    kABit27         = 1 << 26,
    kABit28         = 1 << 27,
    kABit29         = 1 << 28,
    kABit30         = 1 << 29,
    kABit31         = 1 << 30,
    kABit32         = 1 << 31,
};

const int32_t kMaxDestObject = 10;  // we keep special track of dest objects for AI
const int32_t kMaxNumAdmiralCanBuild = kMaxDestObject * kMaxTypeBaseCanBuild;
const int32_t kAdmiralScoreNum = 3;

enum destinationType {
    kNoDestinationType,
    kObjectDestinationType,
    kCoordinateDestinationType
};

struct destBalanceType {
    long                whichObject;
    long                canBuildType[kMaxTypeBaseCanBuild];
    long                occupied[kMaxPlayerNum];
    Fixed               earn;
    long                buildTime;
    long                totalBuildTime;
    long                buildObjectBaseNum;
    sfz::String         name;
};

struct admiralBuildType {
    baseObjectType*     base;
    long                baseNum;
    Fixed               chanceRange;
};

struct admiralType {
    unsigned long       attributes;
    long                destinationObject;
    long                destinationObjectID;
    long                flagship;
    long                flagshipID;
    long                considerShip;
    long                considerShipID;
    long                considerDestination;
    long                buildAtObject; // # of destination object to build at
    long                race;
    destinationType     destType;
    Fixed               cash;
    Fixed               saveGoal;
    Fixed               earningPower;
    long                kills;
    long                losses;
    long                shipsLeft;
    long                score[kAdmiralScoreNum];
    long                blitzkrieg;
    Fixed               lastFreeEscortStrength;
    Fixed               thisFreeEscortStrength;
    admiralBuildType    canBuildType[kMaxNumAdmiralCanBuild];
    Fixed               totalBuildChance;
    long                hopeToBuild;
    unsigned char       color;
    bool                active;
    sfz::String         name;
};


void AdmiralInit();
void AdmiralCleanup();
void ResetAllAdmirals();
void ResetAllDestObjectData();

destBalanceType* mGetDestObjectBalancePtr(long whichObject);
admiralType* mGetAdmiralPtr(long mwhichAdmiral);

long MakeNewAdmiral(
        long flagship, long destinationObject, destinationType dType, unsigned long attributes,
        long race, short nameResID, short nameStrNum, Fixed earningPower);
long MakeNewDestination(
        long whichObject, int32_t* canBuildType, Fixed earn, short nameResID,
        short nameStrNum);
void RemoveDestination(long whichDestination);
void RecalcAllAdmiralBuildData();

void SetAdmiralAttributes(long whichAdmiral, unsigned long attributes);
void SetAdmiralColor(long whichAdmiral, unsigned char color);
unsigned char GetAdmiralColor(long whichAdmiral);
long GetAdmiralRace(long whichAdmiral);
void SetAdmiralFlagship(long whichAdmiral, long whichShip);
spaceObjectType* GetAdmiralFlagship(long whichAdmiral);
void SetAdmiralEarningPower(long whichAdmiral, Fixed power);
Fixed GetAdmiralEarningPower(long whichAdmiral);

void SetAdmiralDestinationObject(long whichAdmiral, long whichObject, destinationType dType);
long GetAdmiralDestinationObject(long whichAdmiral);
void SetAdmiralConsiderObject(long whichAdmiral, long whichObject);
long GetAdmiralConsiderObject(long whichAdmiral);

bool BaseHasSomethingToBuild(long whichObject);
long GetAdmiralBuildAtObject(long whichAdmiral);
void SetAdmiralBuildAtObject(long whichAdmiral, long whichObject);

sfz::StringSlice GetAdmiralBuildAtName(long whichAdmiral);
void SetAdmiralBuildAtName(long whichAdmiral, sfz::StringSlice name);
sfz::StringSlice GetDestBalanceName(long whichDestObject);
sfz::StringSlice GetAdmiralName(long whichAdmiral);
void SetAdmiralName(long whichAdmiral, sfz::StringSlice name);

void SetObjectLocationDestination(spaceObjectType* o, coordPointType* where);
void SetObjectDestination(spaceObjectType* o, spaceObjectType* overrideObject);
void RemoveObjectFromDestination(spaceObjectType* o);

void AdmiralThink();
void AdmiralBuildAtObject(long whichAdmiral, long baseTypeNum, long whichDestObject);
bool AdmiralScheduleBuild(long whichAdmiral, long buildWhichType);
void StopBuilding(long whichDestObject);

void PayAdmiral(long whichAdmiral, Fixed howMuch);
void PayAdmiralAbsolute(long whichAdmiral, Fixed howMuch);
void AlterAdmiralScore(long whichAdmiral, long whichScore, long amount);
long GetAdmiralScore(long whichAdmiral, long whichScore);
long GetAdmiralShipsLeft(long whichAdmiral);
long AlterDestinationObjectOccupation(long whichDestination, long whichAdmiral, long amount);
void ClearAllOccupants(long whichDestination, long whichAdmiral, long fullAmount);
void AddKillToAdmiral(spaceObjectType *anObject);

long GetAdmiralLoss(long whichAdmiral);
long GetAdmiralKill(long whichAdmiral);

}  // namespace antares

#endif // ANTARES_GAME_ADMIRAL_HPP_
