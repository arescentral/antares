/*
Ares, a tactical space combat game.
Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

pragma options align=mac68k

#define kScenarioWinnerPlayerMask   0x000000ff
#define kScenarioWinnerNoPlayer     0x000000ff
#define kScenarioWinnerNextMask     0xff000000
#define kScenarioWinnerNextShift    (long)24
#define kScenarioWinnerTextMask     0x00ffff00
#define kScenarioWinnerTextShift    (long)8

#define kScenarioWinnerNoNext       0xff000000
#define kScenarioWinnerNoText       0x00ffff00

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


#define mGetRealAdmiralNum( mplayernum) gThisScenario->player[mplayernum].admiralNumber

#define mGetActionFromBaseTypeNum( mactPtr, mbaseObjPtr, mactionType, mactionNum)\
if ( (mactionType) == kDestroyActionType)\
{\
    if ( mactionNum >= ((mbaseObjPtr)->destroyActionNum & kDestroyActionNotMask)) mactPtr = nil;\
    else mactPtr = (objectActionType *)*gObjectActionData + (mbaseObjPtr)->destroyAction + (long)mactionNum;\
} else if ( (mactionType) == kExpireActionType) \
{\
    if ( mactionNum >= ((mbaseObjPtr)->expireActionNum  & kDestroyActionNotMask)) mactPtr = nil;\
    else mactPtr = (objectActionType *)*gObjectActionData + (mbaseObjPtr)->expireAction + (long)mactionNum;\
} else if ( (mactionType) == kCreateActionType)\
{\
    if ( mactionNum >= (mbaseObjPtr)->createActionNum) mactPtr = nil;\
    else mactPtr = (objectActionType *)*gObjectActionData + (mbaseObjPtr)->createAction + (long)mactionNum;\
} else if ( (mactionType) == kCollideActionType)\
{\
    if ( mactionNum >= (mbaseObjPtr)->collideActionNum) mactPtr = nil;\
    else mactPtr = (objectActionType *)*gObjectActionData + (mbaseObjPtr)->collideAction + (long)mactionNum;\
} else if ( (mactionType) == kActivateActionType)\
{\
    if ( mactionNum >= ((mbaseObjPtr)->activateActionNum & kPeriodicActionNotMask)) mactPtr = nil;\
    else mactPtr = (objectActionType *)*gObjectActionData + (mbaseObjPtr)->activateAction + (long)mactionNum;\
} else if ( (mactionType) == kArriveActionType)\
{\
        mWriteDebugString("\pArrive Action:");\
        WriteDebugLong( mactionNum);\
        WriteDebugLong( (mbaseObjPtr)->arriveActionNum);\
    if ( mactionNum >= (mbaseObjPtr)->arriveActionNum) mactPtr = nil;\
    else mactPtr = (objectActionType *)*gObjectActionData + (mbaseObjPtr)->arriveAction + (long)mactionNum;\
} else mactPtr = nil;

#define mGetScenarioInitial( mscenario, minitialnum) (scenarioInitialType *)*gAresGlobal->gScenarioInitialData + (mscenario)->initialFirst + (minitialnum)
#define mGetScenarioBrief( mscenario, mbriefnum) (briefPointType *)*gAresGlobal->gScenarioBriefData + ((mscenario)->briefPointFirst) + (mbriefnum)
#define mGetScenarioCondition( mscenario, mconditionnum) (scenarioConditionType *)*gAresGlobal->gScenarioConditionData + (mscenario)->conditionFirst + (mconditionnum)

#define mGetRealObjectFromInitial( mobject, minitialobject, minum)\
if ( minum >= 0)\
{\
    minitialobject = mGetScenarioInitial( gThisScenario, minum);\
    if ( minitialobject->realObjectNumber >= 0)\
    {\
        mobject = (spaceObjectType *)*gSpaceObjectData + (long)minitialobject->realObjectNumber;\
        if (( mobject->id != minitialobject->realObjectID) || ( mobject->active != kObjectInUse))\
            mobject = nil;\
    } else mobject = nil;\
} else if ( minum == -2)\
{\
    mobject = (spaceObjectType *)*gSpaceObjectData + gAresGlobal->gPlayerShipNumber;\
    if ((!(mobject->active)) || ( !(mobject->attributes & kCanThink)))\
    {\
        mobject = nil;\
    }\
} else mobject = nil;

short ScenarioMakerInit( void);
void ScenarioMakerCleanup( void);
Boolean ConstructScenario( long);
void CheckEndgame( void);
void DeclareWinner( long, long, long);
void GetInitialCoord( scenarioInitialType *, coordPointType *, long);
void HackWinLoseMessage( Boolean);
void CheckScenarioConditions( long);
void SetScenarioConditionTrueYet( long, Boolean);
Boolean GetScenarioConditionTrue( long);
long GetRealAdmiralNumber( long);
void UnhideInitialObject( long);
spaceObjectType *GetObjectFromInitialNumber( long);
void GetScenarioFullScaleAndCorner( long, long, coordPointType *, long *, Rect *);
/*void GetScenarioBriefPointData( long, long, long *, long *, long *, Rect *, coordPointType *,
                    long, long, long, Rect *);
void GetInitialObjectSpriteData( long whichScenario, long whichObject, long maxSize,
        Rect *bounds, coordPointType *corner,
        long scale, long *thisScale, spritePix *aSpritePix, Point *where, longRect *spriteRect);
void GetRealObjectSpriteData( coordPointType *, baseObjectType *, long, 
        long, long, Rect *, coordPointType *,
        long, long *, spritePix *, Point *,
        longRect *);
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
void GetScenarioName( long, StringPtr);
long GetScenarioNumber( void);
long GetScenarioPlayerNum( long);
long GetScenarioPrologueID( long);
long GetScenarioEpilogueID( long);
void GetScenarioMovieName( long, StringPtr);
long GetNextScenarioChapter( long);
long GetFirstNetworkScenario( void);
long GetNextNetworkScenario( long);
long GetPreviousNetworkScenario( long);
Boolean ThisChapterIsNetworkable( long);
void CorrectThisScenarioPtr( Handle);

#pragma options align=reset
