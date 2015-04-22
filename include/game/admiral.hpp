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

#include "data/handle.hpp"
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
    BaseObject*         base = nullptr;
    int32_t             baseNum = -1;
    Fixed               chanceRange = -1;
};

class Admiral {
  public:
    static void         init();
    static void         reset();
    static Admiral*     get(int i);
    static Handle<Admiral>  make(int index, uint32_t attributes, const Scenario::Player& player);
    static Handle<Admiral>  none() { return Handle<Admiral>(-1); }

    void                think();
    bool                build(int32_t buildWhichType);
    void                pay(Fixed howMuch);
    void                pay_absolute(Fixed howMuch);

    uint32_t&           attributes() { return _attributes; }
    int32_t&            destinationObject() { return _destinationObject; }
    int32_t&            destinationObjectID() { return _destinationObjectID; }
    int32_t&            flagship() { return _flagship; }
    int32_t&            flagshipID() { return _flagshipID; }
    int32_t&            considerShip() { return _considerShip; }
    int32_t&            considerShipID() { return _considerShipID; }
    int32_t&            considerDestination() { return _considerDestination; }
    int32_t&            buildAtObject() { return _buildAtObject; } // # of destination object to build at
    int32_t&            race() { return _race; }
    destinationType&    destType() { return _destType; }
    Fixed               cash() const { return _cash; }
    Fixed&              cash() { return _cash; }
    Fixed&              saveGoal() { return _saveGoal; }
    Fixed&              earningPower() { return _earningPower; }
    int32_t&            kills() { return _kills; }
    int32_t&            losses() { return _losses; }
    int32_t&            shipsLeft() { return _shipsLeft; }
    int32_t*            score() { return _score; }
    int32_t&            blitzkrieg() { return _blitzkrieg; }
    Fixed&              lastFreeEscortStrength() { return _lastFreeEscortStrength; }
    Fixed&              thisFreeEscortStrength() { return _thisFreeEscortStrength; }
    admiralBuildType*   canBuildType() { return _canBuildType; };
    Fixed&              totalBuildChance() { return _totalBuildChance; }
    int32_t&            hopeToBuild() { return _hopeToBuild; }
    uint8_t&            color() { return _color; }
    bool&               active() { return _active; }
    sfz::StringSlice    name() { return _name; }

  private:
    uint32_t            _attributes;
    int32_t             _destinationObject = kNoDestinationObject;
    int32_t             _destinationObjectID = -1;
    int32_t             _flagship = kNoShip;
    int32_t             _flagshipID = -1;
    int32_t             _considerShip = kNoShip;
    int32_t             _considerShipID = -1;
    int32_t             _considerDestination = kNoShip;
    int32_t             _buildAtObject = -1; // # of destination object to build at
    int32_t             _race = -1;
    destinationType     _destType = kNoDestinationType;
    Fixed               _cash = 0;
    Fixed               _saveGoal = 0;
    Fixed               _earningPower = 0;
    int32_t             _kills = 0;
    int32_t             _losses = 0;
    int32_t             _shipsLeft = 0;
    int32_t             _score[kAdmiralScoreNum] = {};
    int32_t             _blitzkrieg = 1200;
    Fixed               _lastFreeEscortStrength = 0;
    Fixed               _thisFreeEscortStrength = 0;
    admiralBuildType    _canBuildType[kMaxNumAdmiralCanBuild];
    Fixed               _totalBuildChance = 0;
    int32_t             _hopeToBuild = -1;
    uint8_t             _color = 0;
    bool                _active = false;
    sfz::String         _name;

  private:
    Admiral() = default;
};

void ResetAllDestObjectData();

destBalanceType* mGetDestObjectBalancePtr(int32_t whichObject);

int32_t MakeNewDestination(
        int32_t whichObject, int32_t* canBuildType, Fixed earn, int16_t nameResID,
        int16_t nameStrNum);
void RemoveDestination(int32_t whichDestination);
void RecalcAllAdmiralBuildData();

uint8_t GetAdmiralColor(Handle<Admiral> whichAdmiral);
int32_t GetAdmiralRace(Handle<Admiral> whichAdmiral);
void SetAdmiralFlagship(Handle<Admiral> whichAdmiral, int32_t whichShip);
SpaceObject* GetAdmiralFlagship(Handle<Admiral> whichAdmiral);
void SetAdmiralEarningPower(Handle<Admiral> whichAdmiral, Fixed power);
Fixed GetAdmiralEarningPower(Handle<Admiral> whichAdmiral);

void SetAdmiralDestinationObject(Handle<Admiral> whichAdmiral, int32_t whichObject, destinationType dType);
int32_t GetAdmiralDestinationObject(Handle<Admiral> whichAdmiral);
void SetAdmiralConsiderObject(Handle<Admiral> whichAdmiral, int32_t whichObject);
int32_t GetAdmiralConsiderObject(Handle<Admiral> whichAdmiral);

bool BaseHasSomethingToBuild(int32_t whichObject);
int32_t GetAdmiralBuildAtObject(Handle<Admiral> whichAdmiral);
void SetAdmiralBuildAtObject(Handle<Admiral> whichAdmiral, int32_t whichObject);

void SetAdmiralBuildAtName(Handle<Admiral> whichAdmiral, sfz::StringSlice name);
sfz::StringSlice GetDestBalanceName(int32_t whichDestObject);
sfz::StringSlice GetAdmiralName(Handle<Admiral> whichAdmiral);

void SetObjectLocationDestination(SpaceObject* o, coordPointType* where);
void SetObjectDestination(SpaceObject* o, SpaceObject* overrideObject);
void RemoveObjectFromDestination(SpaceObject* o);

void AdmiralThink();
void StopBuilding(int32_t whichDestObject);

void AlterAdmiralScore(Handle<Admiral> whichAdmiral, int32_t whichScore, int32_t amount);
int32_t GetAdmiralScore(Handle<Admiral> whichAdmiral, int32_t whichScore);
int32_t GetAdmiralShipsLeft(Handle<Admiral> whichAdmiral);
int32_t AlterDestinationObjectOccupation(int32_t whichDestination, Handle<Admiral> whichAdmiral, int32_t amount);
void ClearAllOccupants(int32_t whichDestination, Handle<Admiral> whichAdmiral, int32_t fullAmount);
void AddKillToAdmiral(SpaceObject *anObject);

int32_t GetAdmiralLoss(Handle<Admiral> whichAdmiral);
int32_t GetAdmiralKill(Handle<Admiral> whichAdmiral);

}  // namespace antares

#endif // ANTARES_GAME_ADMIRAL_HPP_
