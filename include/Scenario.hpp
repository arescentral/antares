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

#ifndef ANTARES_SCENARIO_HPP_
#define ANTARES_SCENARIO_HPP_

// Scenario.h

#include "NateDraw.h"
#include "SpaceObject.h"

#pragma options align=mac68k

#define kScenarioInitialNum             12
#define kScenarioSpontaneousNum         2
#define kScenarioEndgameNum             12
#define kScenarioPlayerNum              4

#define kBriefPointNum                      24

#define kMaxScenarioBrief                   (gAresGlobal->maxScenarioBrief)//200
#define kMaxScenarioCondition           (gAresGlobal->maxScenarioCondition)//200
#define kMaxScenarioInitial             (gAresGlobal->maxScenarioInitial)//500

//#define   kMaxTypeBaseCanBuildOld     4
#define kMaxTypeBaseCanBuild                12
#define kMaxShipCanBuild                    6

#define kScenarioNum                        (gAresGlobal->scenarioNum)//31

#define kSingleHumanPlayer              0
#define kNetworkHumanPlayer             1
#define kComputerPlayer                 2

#define kRaceNum                                16
#define kRaceStrID                          4201
#define kRaceAdjective                      0
#define kRacePlural                         1
#define kRaceForce                          2
#define kRaceWorld                          3
#define kRaceStringNum                      4

#define kScenarioBriefMask              0x00ff
#define kScenarioAngleMask              0xff00
#define kScenarioAngleShift             8

#define kScenarioNoOwner                    -1

#define kScenarioTimeMultiple           20

#define kScenario_StartTimeMask         0x7fff
#define kScenario_IsTraining_Bit        0x8000

// condition flags
#define kTrueOnlyOnce                       0x00000001
#define kInitiallyTrue                      0x00000002
#define kHasBeenTrue                        0x00000004

#define kEndgameNum0                        0x00000001
#define kEndgameNum1                        0x00000002
#define kEndgameNum2                        0x00000004
#define kEndgameNum3                        0x00000008
#define kEndgameNum4                        0x00000010
#define kEndgameNum5                        0x00000020
#define kEndgameNum6                        0x00000040
#define kEndgameNum7                        0x00000080
#define kEndgameNum8                        0x00000100
#define kEndgameNum9                        0x00000200
#define kEndgameNum10                       0x00000400
#define kEndgameNum11                       0x00000800
#define kEndgameNum12                       0x00001000
#define kEndgameNum13                       0x00002000
#define kEndgameNum14                       0x00004000
#define kEndgameNum15                       0x00008000
#define kEndgameNum16                       0x00010000
#define kEndgameNum17                       0x00020000
#define kEndgameNum18                       0x00040000
#define kEndgameNum19                       0x00080000
#define kEndgameNum20                       0x00100000
#define kEndgameNum21                       0x00200000
#define kEndgameNum22                       0x00400000
#define kEndgameNum23                       0x00800000
#define kEndgameNum24                       0x01000000
#define kEndgameNum25                       0x02000000
#define kEndgameNum26                       0x04000000
#define kEndgameNum27                       0x08000000
#define kEndgameNum28                       0x10000000
#define kEndgameNum29                       0x20000000
#define kEndgameNum30                       0x40000000
#define kEndgameNum31                       0x80000000

typedef enum
{
    kNoCondition = 0,
    kLocationCondition = 1,
    kCounterCondition = 2,
    kProximityCondition = 3,
    kOwnerCondition = 4,
    kDestructionCondition = 5,
    kAgeCondition = 6,
    kTimeCondition = 7,
    kRandomCondition = 8,
    kHalfHealthCondition = 9,
    kIsAuxiliaryObject = 10,
    kIsTargetObject = 11,
    kCounterGreaterCondition = 12,
    kCounterNotCondition = 13,
    kDistanceGreaterCondition = 14,
    kVelocityLessThanEqualToCondition = 15,
    kNoShipsLeftCondition = 16,
    kCurrentMessageCondition = 17, // use location.h for res id, .v for page
    kCurrentComputerCondition = 18, // use location.h for screen #, .v for line #
    kZoomLevelCondition = 19,
    kAutopilotCondition = 20,
    kNotAutopilotCondition = 21,
    kObjectIsBeingBuilt = 22,       // for tutorial; is base building something?
    kDirectIsSubjectTarget = 23,
    kSubjectIsPlayerCondition = 24
} conditionType;

typedef enum
{
    kNoPointKind = 0,
    kBriefObjectKind = 1,
    kBriefAbsoluteKind = 2,
    kBriefFreestandingKind = 3
} briefingPointKindType;

//
// We need to know:
// type of tour point: object, absolute, or free-standing
// either scenario object # & visible --or-- location ((long & Boolean) or longPoint)
// range (longPoint)
// title ID, # (short, short)
// content ID, # (short, short)
//

typedef struct
{
    briefingPointKindType   briefPointKind;
    union
    {
        struct
        {
            long            objectNum;
            Boolean         objectVisible;
        } objectBriefType;

        struct
        {
            longPointType   location;
        } absoluteBriefType;
    } briefPointData;
    longPointType           range;
    short                   titleResID;
    short                   titleNum;
    short                   contentResID;
} briefPointType;

/*
typedef struct
{
    long            type;
    long            owner;
    long            realObjectNumber;
    union
    {
        long            realObjectID;
        unsigned long   attributes;
    } realData;
    longPointType   location;
    long            distanceMinimum;
    long            distanceRange;
    long            rotationMinimum;
    long            rotationRange;
    long            spriteIDOverride;               // <- ADDED 9/30
    long            canBuild[kMaxTypeBaseCanBuildOld];
    long            initialDestination;             // <- ADDED 9/27
    long            nameResID;
    long            nameStrNum;
    Boolean         isFlagShip;
} scenarioInitialTypeOld;
*/

typedef struct
{
    long            type;
    long            owner;
    long            realObjectNumber;
    long            realObjectID;
    longPointType   location;
    smallFixedType  earning;
    long            distanceRange;
    long            rotationMinimum;
    long            rotationRange;
    long            spriteIDOverride;               // <- ADDED 9/30
    long            canBuild[kMaxTypeBaseCanBuild];
    long            initialDestination;             // <- ADDED 9/27
    long            nameResID;
    long            nameStrNum;
    unsigned long   attributes;
} scenarioInitialType;

typedef struct
{
    long            whichPlayer;
    long            whichCounter;
    long            amount;
} counterArgumentType;

typedef struct
{
    conditionType   condition;
    union
    {
        longPointType       location;
        counterArgumentType counter;
        long                longValue;
        unsigned long       unsignedLongValue;
    } conditionArgument;
    long            subjectObject;      // initial object #
    long            directObject;       // initial object #
    long            startVerb;
    long            verbNum;
} scenarioConditionTypeOld;

typedef struct
{
    conditionType   condition;
    union
    {
        longPointType       location;
        counterArgumentType counter;
        long                longValue;
        unsigned long       unsignedLongValue;
    } conditionArgument;
    long            subjectObject;      // initial object #
    long            directObject;       // initial object #
    long            startVerb;
    long            verbNum;
    unsigned long   flags;
    long            direction;
} scenarioConditionType;

typedef struct
{
    short           playerType;
    short           playerRace;
    short           nameResID;
    short           nameStrNum;
    long            admiralNumber;
    smallFixedType  earningPower;
//  long            reserved1;
    short           netRaceFlags;
    short           reserved1;
} scenarioPlayerType;

typedef struct
{
    long                    playerNum;
    scenarioPlayerType      player[kScenarioPlayerNum];
    long                    initialFirst;
    long                    initialNum;
    long                    conditionFirst;
    long                    conditionNum;
    long                    briefPointFirst;
    long                    briefPointNum;
} scenarioTypeOld;

typedef struct
{
    short                       netRaceFlags;
    short                       playerNum;
    scenarioPlayerType  player[kScenarioPlayerNum];
    short                       scoreStringResID;
    short                       initialFirst;
    short                       prologueID;
    short                       initialNum;
    short                       songID;
    short                       conditionFirst;
    short                       epilogueID;
    short                       conditionNum;
    short                       starMapH;
    short                       briefPointFirst;
    short                       starMapV;
    short                       briefPointNum;  // use kScenarioBriefMask
    short                       parTime;
    short                       movieNameStrNum;
    short                       parKills;
    short                       levelNameStrNum;
    smallFixedType          parKillRatio;
    short                       parLosses;
    short                       startTime;      // use kScenario_StartTimeMask
} scenarioType;

typedef struct
{
    long                        id;
    unsigned char           apparentColor;
    unsigned long           illegalColors;
    long                        advantage;
} raceType;

#pragma options align=reset

#endif // ANTARES_SCENARIO_HPP_
