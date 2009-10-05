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

#ifndef ANTARES_SCENARIO_MAKER_HPP_
#define ANTARES_SCENARIO_MAKER_HPP_

#include <Base.h>

#include "AresGlobalType.hpp"
#include "Debug.hpp"
#include "NateDraw.hpp"
#include "Scenario.hpp"
#include "SpaceObject.hpp"

#define kScenarioNoShipTextID       10000

#define kScenarioResFileName    "\p:Ares Data Folder:Ares Scenarios"

#define kScenarioResType        'snro'
#define kScenarioResID          500

#define kScenarioInitialResType 'snit'
#define kScenarioInitialResID   500

#define kScenarioConditionResType   'sncd'
#define kScenarioConditionResID     500

#define kScenarioBriefResType   'snbf'
#define kScenarioBriefResID     500

extern scenarioType* gThisScenario;
extern TypedHandle<objectActionType>    gObjectActionData;
extern TypedHandle<spaceObjectType> gSpaceObjectData;

inline int mGetRealAdmiralNum(int mplayernum) {
    return gThisScenario->player[mplayernum].admiralNumber;
}

enum {
    kDestroyActionType = 1,
    kExpireActionType = 2,
    kCreateActionType = 3,
    kCollideActionType = 4,
    kActivateActionType = 5,
    kArriveActionType = 6
};

inline void mGetActionFromBaseTypeNum(
        objectActionType*& mactPtr, baseObjectType* mbaseObjPtr, long mactionType,
        long mactionNum) {
    if ( (mactionType) == kDestroyActionType)
    {
        if ( mactionNum >= ((mbaseObjPtr)->destroyActionNum & kDestroyActionNotMask)) mactPtr = nil;
        else mactPtr = *gObjectActionData + (mbaseObjPtr)->destroyAction + implicit_cast<long>(mactionNum);
    } else if ( (mactionType) == kExpireActionType) 
    {
        if ( mactionNum >= ((mbaseObjPtr)->expireActionNum  & kDestroyActionNotMask)) mactPtr = nil;
        else mactPtr = *gObjectActionData + (mbaseObjPtr)->expireAction + implicit_cast<long>(mactionNum);
    } else if ( (mactionType) == kCreateActionType)
    {
        if ( mactionNum >= (mbaseObjPtr)->createActionNum) mactPtr = nil;
        else mactPtr = *gObjectActionData + (mbaseObjPtr)->createAction + implicit_cast<long>(mactionNum);
    } else if ( (mactionType) == kCollideActionType)
    {
        if ( mactionNum >= (mbaseObjPtr)->collideActionNum) mactPtr = nil;
        else mactPtr = *gObjectActionData + (mbaseObjPtr)->collideAction + implicit_cast<long>(mactionNum);
    } else if ( (mactionType) == kActivateActionType)
    {
        if ( mactionNum >= ((mbaseObjPtr)->activateActionNum & kPeriodicActionNotMask)) mactPtr = nil;
        else mactPtr = *gObjectActionData + (mbaseObjPtr)->activateAction + implicit_cast<long>(mactionNum);
    } else if ( (mactionType) == kArriveActionType)
    {
        mWriteDebugString("\pArrive Action:");
        WriteDebugLong( mactionNum);
        WriteDebugLong( (mbaseObjPtr)->arriveActionNum);
        if ( mactionNum >= (mbaseObjPtr)->arriveActionNum) mactPtr = nil;
        else mactPtr = *gObjectActionData + (mbaseObjPtr)->arriveAction + implicit_cast<long>(mactionNum);
    } else mactPtr = nil;
}

inline scenarioInitialType* mGetScenarioInitial(scenarioType* mscenario, long minitialnum) {
    return *globals()->gScenarioInitialData
        + (mscenario)->initialFirst + (minitialnum);
}

inline briefPointType* mGetScenarioBrief(scenarioType* mscenario, long mbriefnum) {
    return *globals()->gScenarioBriefData
        + ((mscenario)->briefPointFirst) + (mbriefnum);
}

inline scenarioConditionType* mGetScenarioCondition(scenarioType* mscenario, long mconditionnum) {
    return *globals()->gScenarioConditionData
        + (mscenario)->conditionFirst + (mconditionnum);
}

inline void mGetRealObjectFromInitial(
        spaceObjectType*& mobject, scenarioInitialType*& minitialobject, long minum) {
    if ( minum >= 0)
    {
        minitialobject = mGetScenarioInitial( gThisScenario, minum);
        if ( minitialobject->realObjectNumber >= 0)
        {
            mobject = *gSpaceObjectData + minitialobject->realObjectNumber;
            if (( mobject->id != minitialobject->realObjectID) || ( mobject->active != kObjectInUse))
                mobject = nil;
        } else mobject = nil;
    } else if ( minum == -2)
    {
        mobject = *gSpaceObjectData + globals()->gPlayerShipNumber;
        if ((!(mobject->active)) || ( !(mobject->attributes & kCanThink)))
        {
            mobject = nil;
        }
    } else mobject = nil;
}

short ScenarioMakerInit( void);
void ScenarioMakerCleanup( void);
bool ConstructScenario( long);
void CheckEndgame( void);
void DeclareWinner( long, long, long);
void GetInitialCoord( scenarioInitialType *, coordPointType *, long);
void HackWinLoseMessage( bool);
void CheckScenarioConditions( long);
void SetScenarioConditionTrueYet( long, bool);
bool GetScenarioConditionTrue( long);
long GetRealAdmiralNumber( long);
void UnhideInitialObject( long);
spaceObjectType *GetObjectFromInitialNumber( long);
void GetScenarioFullScaleAndCorner( long, long, coordPointType *, long *, Rect *);
/*void GetScenarioBriefPointData( long, long, long *, long *, long *, Rect *, coordPointType *,
                    long, long, long, Rect *);
void GetInitialObjectSpriteData( long whichScenario, long whichObject, long maxSize,
        Rect *bounds, coordPointType *corner,
        long scale, long *thisScale, spritePix *aSpritePix, Point *where, Rect *spriteRect);
void GetRealObjectSpriteData( coordPointType *, baseObjectType *, long,
        long, long, Rect *, coordPointType *,
        long, long *, spritePix *, Point *,
        Rect *);
*/
long GetBriefPointNumber( long);
long GetScenarioAngle( long whichScenario);
void GetScenarioStarMapPoint( long, Point *);
void CheckBaseObjectActionMedia( long, unsigned char);
void CheckBaseObjectMedia( baseObjectType *, unsigned char);
void CheckActionMedia( long, long, unsigned char);
void SetAllBaseObjectsUnchecked( void);
void AddBaseObjectActionMedia( long, long, unsigned char);
void AddBaseObjectMedia( long, unsigned char);
void AddActionMedia( objectActionType *, unsigned char);
long GetScenarioNumberFromChapterNumber( long);
long GetChapterNumberFromScenarioNumber( long);
scenarioType *GetScenarioPtrFromChapter( long);
void GetScenarioName(long, unsigned char*);
long GetScenarioNumber( void);
long GetScenarioPlayerNum( long);
long GetScenarioPrologueID( long);
long GetScenarioEpilogueID( long);
void GetScenarioMovieName(long, unsigned char*);
long GetNextScenarioChapter( long);
long GetFirstNetworkScenario( void);
long GetNextNetworkScenario( long);
long GetPreviousNetworkScenario( long);
bool ThisChapterIsNetworkable( long);

#endif // ANTARES_SCENARIO_MAKER_HPP_
