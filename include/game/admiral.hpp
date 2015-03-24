// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2012 The Antares Authors
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
// License along with Antares.  If not, see http://www.gnu.org/licenses/

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
    int32_t             whichObject;
    int32_t             canBuildType[kMaxTypeBaseCanBuild];
    int32_t             occupied[kMaxPlayerNum];
    Fixed               earn;
    int32_t             buildTime;
    int32_t             totalBuildTime;
    int32_t             buildObjectBaseNum;
    sfz::String         name;
};

struct admiralBuildType {
    baseObjectType*     base;
    int32_t             baseNum;
    Fixed               chanceRange;
};

struct admiralType {
    uint32_t            attributes;
    int32_t             destinationObject;
    int32_t             destinationObjectID;
    int32_t             flagship;
    int32_t             flagshipID;
    int32_t             considerShip;
    int32_t             considerShipID;
    int32_t             considerDestination;
    int32_t             buildAtObject; // # of destination object to build at
    int32_t             race;
    destinationType     destType;
    Fixed               cash;
    Fixed               saveGoal;
    Fixed               earningPower;
    int32_t             kills;
    int32_t             losses;
    int32_t             shipsLeft;
    int32_t             score[kAdmiralScoreNum];
    int32_t             blitzkrieg;
    Fixed               lastFreeEscortStrength;
    Fixed               thisFreeEscortStrength;
    admiralBuildType    canBuildType[kMaxNumAdmiralCanBuild];
    Fixed               totalBuildChance;
    int32_t             hopeToBuild;
    uint8_t             color;
    bool                active;
    sfz::String         name;
};


void AdmiralInit();
void AdmiralCleanup();
void ResetAllAdmirals();
void ResetAllDestObjectData();

destBalanceType* mGetDestObjectBalancePtr(int32_t whichObject);
admiralType* mGetAdmiralPtr(int32_t mwhichAdmiral);

int32_t MakeNewAdmiral(uint32_t attributes, const Scenario::Player& player);
int32_t MakeNewDestination(
        int32_t whichObject, int32_t* canBuildType, Fixed earn, int16_t nameResID,
        int16_t nameStrNum);
void RemoveDestination(int32_t whichDestination);
void RecalcAllAdmiralBuildData();

void SetAdmiralAttributes(int32_t whichAdmiral, uint32_t attributes);
void SetAdmiralColor(int32_t whichAdmiral, uint8_t color);
uint8_t GetAdmiralColor(int32_t whichAdmiral);
int32_t GetAdmiralRace(int32_t whichAdmiral);
void SetAdmiralFlagship(int32_t whichAdmiral, int32_t whichShip);
spaceObjectType* GetAdmiralFlagship(int32_t whichAdmiral);
void SetAdmiralEarningPower(int32_t whichAdmiral, Fixed power);
Fixed GetAdmiralEarningPower(int32_t whichAdmiral);

void SetAdmiralDestinationObject(int32_t whichAdmiral, int32_t whichObject, destinationType dType);
int32_t GetAdmiralDestinationObject(int32_t whichAdmiral);
void SetAdmiralConsiderObject(int32_t whichAdmiral, int32_t whichObject);
int32_t GetAdmiralConsiderObject(int32_t whichAdmiral);

bool BaseHasSomethingToBuild(int32_t whichObject);
int32_t GetAdmiralBuildAtObject(int32_t whichAdmiral);
void SetAdmiralBuildAtObject(int32_t whichAdmiral, int32_t whichObject);

sfz::StringSlice GetAdmiralBuildAtName(int32_t whichAdmiral);
void SetAdmiralBuildAtName(int32_t whichAdmiral, sfz::StringSlice name);
sfz::StringSlice GetDestBalanceName(int32_t whichDestObject);
sfz::StringSlice GetAdmiralName(int32_t whichAdmiral);
void SetAdmiralName(int32_t whichAdmiral, sfz::StringSlice name);

void SetObjectLocationDestination(spaceObjectType* o, coordPointType* where);
void SetObjectDestination(spaceObjectType* o, spaceObjectType* overrideObject);
void RemoveObjectFromDestination(spaceObjectType* o);

void AdmiralThink();
void AdmiralBuildAtObject(int32_t whichAdmiral, int32_t baseTypeNum, int32_t whichDestObject);
bool AdmiralScheduleBuild(int32_t whichAdmiral, int32_t buildWhichType);
void StopBuilding(int32_t whichDestObject);

void PayAdmiral(int32_t whichAdmiral, Fixed howMuch);
void PayAdmiralAbsolute(int32_t whichAdmiral, Fixed howMuch);
void AlterAdmiralScore(int32_t whichAdmiral, int32_t whichScore, int32_t amount);
int32_t GetAdmiralScore(int32_t whichAdmiral, int32_t whichScore);
int32_t GetAdmiralShipsLeft(int32_t whichAdmiral);
int32_t AlterDestinationObjectOccupation(int32_t whichDestination, int32_t whichAdmiral, int32_t amount);
void ClearAllOccupants(int32_t whichDestination, int32_t whichAdmiral, int32_t fullAmount);
void AddKillToAdmiral(spaceObjectType *anObject);

int32_t GetAdmiralLoss(int32_t whichAdmiral);
int32_t GetAdmiralKill(int32_t whichAdmiral);

}  // namespace antares

#endif // ANTARES_GAME_ADMIRAL_HPP_
