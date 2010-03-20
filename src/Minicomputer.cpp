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

// Minicomputer.c
// shouldn't be used without initing instruments 1st

#include "Minicomputer.hpp"

#include "Admiral.hpp"
#include "AnyChar.hpp"
#include "AresGlobalType.hpp"
#include "ColorTranslation.hpp"
#include "Debug.hpp"
#include "DirectText.hpp"
#include "Error.hpp"
#include "Instruments.hpp"
#include "KeyCodes.hpp"
#include "KeyMapTranslation.hpp"
#include "MessageScreen.hpp"
#include "NatePixTable.hpp"
#include "OffscreenGWorld.hpp"
#include "Options.hpp"
#include "PlayerShip.hpp"
#include "ScenarioMaker.hpp"
#include "ScrollStars.hpp"
#include "SoundFX.hpp"
#include "SpaceObjectHandling.hpp"
#include "SpriteHandling.hpp"
#include "StringHandling.hpp"
#include "StringList.hpp"
#include "StringNumerics.hpp"

using sfz::scoped_array;

namespace antares {

#define kMiniScreenLeft         12
#define kMiniScreenTop          320 //191
#define kMiniScreenRight        121
#define kMiniScreenBottom       440
#define kMiniScreenWidth        108
#define kMiniScreenHeight       120//252

#define kMiniScreenLeftBuffer   3

#define kMiniScreenCharHeight   10//20 // height of the screen in characters
#define kMiniScreenTrueLineNum  12// = kMiniScreenCharHeight + the 2 button lines

#define kButBoxLeft             16
#define kButBoxTop              450
#define kButBoxRight            114
#define kButBoxBottom           475
#define kButBoxWidth            99
#define kButBoxHeight           25

#define kMiniScreenNoLineSelected   -1

#define kMiniScreenStringID     3000
#define kMiniDataStringID       3001

#define kMiniScreenColor        GREEN
#define kMiniButColor           AQUA

#define kMiniScreenError        "\pMNCM"

#define kMiniScreenSpecChar     '\\'
#define kEndLineChar            'x'
#define kUnderlineEndLineChar   'u'
#define kStartHiliteChar        'i'
#define kEndHiliteChar          'n'
#define kIntoButtonChar         'I'
#define kOutOfButtonChar        'O'
#define kSelectableLineChar     'S'

#define kNoLineButton           -1
#define kPreviousLineButton     kCompUpKeyNum
#define kNextLineButton         kCompDownKeyNum
#define kInLineButton           kCompAcceptKeyNum
#define kOutLineButton          kCompCancelKeyNum

#define kMainMiniScreen         1
#define kMainMiniBuild          1
#define kMainMiniSpecial        2
#define kMainMiniMessage        3
#define kMainMiniStatus         4

#define kBuildMiniScreen            2
#define kBuildScreenFirstTypeLine   2
#define kBuildScreenWhereNameLine   1

#define kSpecialMiniScreen      3
#define kSpecialMiniTransfer    1
#define kSpecialMiniHold        2
#define kSpecialMiniGoToMe      3
#define kSpecialMiniFire1       4
#define kSpecialMiniFire2       5
#define kSpecialMiniFireSpecial 6

#define kMessageMiniScreen      4
#define kMessageMiniNext        1
#define kMessageMiniPrevious    2
#define kMessageMiniLast        3

#define kStatusMiniScreen           5
#define kStatusMiniScreenFirstLine  1

#define kNoStatusData               -1
#define kPlainTextStatus            0
#define kTrueFalseCondition         1   // 0 = F, 1 = T
#define kIntegerValue               2
#define kSmallFixedValue            3
#define kIntegerMinusValue          4   // value - designated score
#define kSmallFixedMinusValue       5   // small fixed - designated score
#define kMaxStatusTypeValue         5

#define kMiniComputerPollTime   60

#define kMiniObjectDataNum      2
#define kMiniSelectObjectNum    0
#define kMiniSelectTop          180

#define kMiniIconMacLineTop     ( gDirectText->height * 2)
#define kMiniIconLineBottom     ( gDirectText->height * 4)
#define kMiniIconHeight         22//19
#define kMiniIconWidth          24
#define kMiniIconLeft           ( kMiniScreenLeft + 2)

#define kMiniHealthLeft         ( kMiniIconLeft + kMiniIconWidth + 2)
#define kMiniBarWidth           11
#define kMiniBarHeight          18//15

#define kMiniEnergyLeft         ( kMiniHealthLeft + kMiniBarWidth + 2)

#define kMiniRightColumnLeft    57
#define kMiniWeapon1LineNum     2
#define kMiniWeapon2LineNum     3
#define kMiniWeapon3LineNum     1
#define kMiniNameLineNum        1

#define kMiniDestLineNum        4

#define kMiniTargetObjectNum    1
#define kMiniTargetTop          252

#define kMiniAmmoTop            161//161//158
#define kMiniAmmoBottom         170//171
#define kMiniAmmoSingleWidth    21//19
#define kMiniAmmoLeftOne        27
#define kMiniAmmoLeftTwo        64
#define kMiniAmmoLeftSpecial    100
#define kMiniAmmoTextHBuffer    2

#define mPlayBeep3              PlayVolumeSound(  kComputerBeep3, kMediumVolume, kMediumPersistence, kLowPrioritySound)
#define mPlayBeepBad            PlayVolumeSound(  kWarningTone, kMediumVolume, kMediumPersistence, kLowPrioritySound)

#define kMaxShipBuffer          40

inline void mBlackMiniScreenLine(
        long mtop, long mlinenum, long mleft, long mright, Rect& mbounds, PixMap* mpixbase) {
    mbounds.left = kMiniScreenLeft + mleft;
    mbounds.top = mtop + mlinenum * mDirectFontHeight();
    mbounds.right = kMiniScreenLeft + mright;
    mbounds.bottom = mbounds.top + mDirectFontHeight();
    DrawNateRect(mpixbase, &mbounds, 0, 0, RgbColor::kBlack);
}

inline long mGetLineNumFromV(long mV) {
    return (((mV) - (kMiniScreenTop + globals()->gInstrumentTop)) / mDirectFontHeight());
}

// for copying the fields of a space object relevant to the miniscreens:
inline void mCopyMiniSpaceObject(
        spaceObjectType& mdestobject, const spaceObjectType& msourceobject) {
    (mdestobject).id = (msourceobject).id;
    (mdestobject).beamType = (msourceobject).beamType;
    (mdestobject).pulseType = (msourceobject).pulseType;
    (mdestobject).specialType = (msourceobject).specialType;
    (mdestobject).destinationLocation.h = (msourceobject).destinationLocation.h;
    (mdestobject).destinationLocation.v = (msourceobject).destinationLocation.v;
    (mdestobject).destinationObject = (msourceobject).destinationObject;
    (mdestobject).destObjectPtr = (msourceobject).destObjectPtr;
    (mdestobject).health = (msourceobject).health;
    (mdestobject).energy = (msourceobject).energy;
    (mdestobject).whichBaseObject = (msourceobject).whichBaseObject;
    (mdestobject).pixResID = (msourceobject).pixResID;
    (mdestobject).attributes = (msourceobject).attributes;
    (mdestobject).location = (msourceobject).location;
    (mdestobject).owner = (msourceobject).owner;
    (mdestobject).nextFarObject = (msourceobject).nextFarObject;
    (mdestobject).distanceGrid = (msourceobject).distanceGrid;
    (mdestobject).nextNearObject = (msourceobject).nextNearObject;
    (mdestobject).collisionGrid = (msourceobject).collisionGrid;
    (mdestobject).remoteFriendStrength = (msourceobject).remoteFriendStrength;
    (mdestobject).remoteFoeStrength = (msourceobject).remoteFoeStrength;
    (mdestobject).escortStrength = (msourceobject).escortStrength;
    (mdestobject).baseType = (msourceobject).baseType;
}

inline void mCopyBlankLineString(
        miniScreenLineType* mline, unsigned char*& mchar, unsigned char* mstring, short& mslen,
        short& mlinelen) {
    mchar = mstring;
    mslen = *mchar;
    mchar++;
    mlinelen = 1;
    while (( mslen > 0) && ( mlinelen <= kMiniScreenCharWidth))
    {
        mline->string[mlinelen] = *mchar;
        mchar++;
        mlinelen++;
        mslen--;
    }
    while ( mlinelen <= kMiniScreenCharWidth)
    {
        mline->string[mlinelen] = kAnyCharSpace;
        mlinelen++;
    }
}

inline spaceObjectType* mGetMiniObjectPtr(long mwhich) {
    return globals()->gMiniScreenData.objectData.get() + mwhich;
}

extern long             gNatePortLeft, gNatePortTop, gNetLatency;
extern directTextType*  gDirectText;
extern scoped_array<spaceObjectType> gSpaceObjectData;
extern PixMap*          gActiveWorld;
extern PixMap*          gRealWorld;
extern PixMap*          gOffWorld;

miniComputerDataType    *gMiniScreenData = nil;

void MiniComputerSetStatusStrings( void);
long MiniComputerGetStatusValue( long);
void MiniComputerMakeStatusString(long, unsigned char*);

int MiniScreenInit() {
    globals()->gMiniScreenData.selectLine = kMiniScreenNoLineSelected;
    globals()->gMiniScreenData.currentScreen = kMainMiniScreen;
    globals()->gMiniScreenData.pollTime = 0;
    globals()->gMiniScreenData.buildTimeBarValue = 0;
    globals()->gMiniScreenData.clickLine = kMiniScreenNoLineSelected;

    globals()->gMiniScreenData.lineData.reset(new miniScreenLineType[kMiniScreenTrueLineNum]);
    globals()->gMiniScreenData.objectData.reset(new spaceObjectType[kMiniObjectDataNum]);

    ClearMiniScreenLines();
    ClearMiniObjectData();

    return kNoError;
}

void MiniScreenCleanup() {
    globals()->gMiniScreenData.lineData.reset();
    globals()->gMiniScreenData.objectData.reset();
}

#pragma mark -

void SetMiniScreenStatusStrList(short strID) {
    DisposeMiniScreenStatusStrList();
    if (strID > 0) {
        globals()->gMissionStatusStrList.reset(new StringList(strID));
    }
}

void DisposeMiniScreenStatusStrList( void) {
    globals()->gMissionStatusStrList.reset();
}

void ClearMiniScreenLines( void)

{
    long                a, b;
    miniScreenLineType  *c;

    c = globals()->gMiniScreenData.lineData.get();

    for ( b = 0; b < kMiniScreenTrueLineNum; b++)
    {
        c->string[0] = kMiniScreenCharWidth;
        for ( a = 1; a <= kMiniScreenCharWidth; a++)
        {
            c->string[a] = ' ';
        }
        c->hiliteLeft = c->hiliteRight = 0;
        c->whichButton = kNoLineButton;
        c->selectable = cannotSelect;
        c->underline = false;
        c->lineKind = plainLineKind;
        c->sourceData = nil;
        c++;
    }
}

void ClearMiniObjectData( void)

{
    spaceObjectType *o;

    o = mGetMiniObjectPtr( kMiniSelectObjectNum);
    o->id = -1;
    o->beamType = -1;
    o->pulseType = -1;
    o->specialType = -1;
    o->destinationLocation.h = o->destinationLocation.v = -1;
    o->destinationObject = -1;
    o->destObjectPtr = nil;
    o->health = 0;
    o->energy = 0;
    o->whichBaseObject = -1;
    o->pixResID = -1;
    o->attributes = 0;
    o->baseType = nil;

    o = mGetMiniObjectPtr( kMiniTargetObjectNum);
    o->id = -1;
    o->beamType = -1;
    o->pulseType = -1;
    o->specialType = -1;
    o->destinationLocation.h = o->destinationLocation.v = -1;
    o->destinationObject = -1;
    o->destObjectPtr = nil;
    o->health = 0;
    o->energy = 0;
    o->whichBaseObject = -1;
    o->pixResID = -1;
    o->attributes = 0;
    o->baseType = nil;

    globals()->gMiniScreenData.buildTimeBarValue = 0;
    globals()->gMiniScreenData.pollTime = 0;
}

void DrawMiniScreen( void)

{
    Rect                mRect;
    Rect            lRect, cRect;
    miniScreenLineType  *c;
    RgbColor            color, lightcolor, darkcolor, textcolor;
    unsigned char lineColor = kMiniScreenColor;
    long                count, lineCorrect = 0;

    mSetDirectFont( kComputerFontNum);

    DrawInOffWorld();
    lRect = Rect(kMiniScreenLeft, kMiniScreenTop + globals()->gInstrumentTop, kMiniScreenRight,
                kMiniScreenBottom + globals()->gInstrumentTop);
    GetRGBTranslateColorShade(&color, kMiniScreenColor, DARKEST);
    cRect = lRect;
    DrawNateRect(gOffWorld, &cRect, 0, 0, color);

    mRect.left = kMiniScreenLeft;
    mRect.top = kMiniScreenTop + globals()->gInstrumentTop;
    mRect.right = kMiniScreenRight;
    mRect.bottom = kMiniScreenBottom + globals()->gInstrumentTop;

    c = globals()->gMiniScreenData.lineData.get();

    for ( count = 0; count < kMiniScreenTrueLineNum; count++)
    {
        if ( count == kMiniScreenCharHeight)
        {
            lRect.left = mRect.left = kButBoxLeft;
            lRect.top = mRect.top = kButBoxTop + globals()->gInstrumentTop;
            lRect.right = mRect.right = kButBoxRight;
            lRect.bottom = mRect.bottom = kButBoxBottom + globals()->gInstrumentTop;
            GetRGBTranslateColorShade(&color, kMiniScreenColor, DARKEST);
            cRect = lRect;
            DrawNateRect(gOffWorld, &cRect, 0, 0, color);
            lineCorrect = -kMiniScreenCharHeight;
            lineColor = kMiniButColor;
        }

        if ( c->underline)
        {
            MoveTo( mRect.left, mRect.top + (count + lineCorrect) * ((
                gDirectText->height)/* * 2*/) + gDirectText->ascent/* * 2*/);
            SetTranslateColorShadeFore( lineColor, MEDIUM);
            MacLineTo(gActiveWorld, mRect.right - 1, mRect.top + (count + lineCorrect) * ((
                gDirectText->height) /* * 2 */) + gDirectText->ascent /* * 2 */);
        }

        if ( c->hiliteLeft < c->hiliteRight)
        {
            if ( c->selectable == selectDim)
                GetRGBTranslateColorShade(&textcolor, lineColor, VERY_DARK);
            else
                GetRGBTranslateColorShade(&textcolor, lineColor, VERY_LIGHT);
            switch( c->lineKind)
            {
                case plainLineKind:
                    if ( c->hiliteRight > c->hiliteLeft)
                    {
                        cRect.left = c->hiliteLeft;
                        cRect.top = mRect.top + (( count + lineCorrect) * ( gDirectText->height /* * 2 */));
                        cRect.right = c->hiliteRight;
                        cRect.bottom = cRect.top + gDirectText->height /* * 2 */;
//                      color = GetTranslateColorShade( lineColor, DARK);
                        GetRGBTranslateColorShade(&color, lineColor, DARK);
                        GetRGBTranslateColorShade(&lightcolor, lineColor, MEDIUM);
                        GetRGBTranslateColorShade(&darkcolor, lineColor, DARKER);
                        DrawNateShadedRect(gOffWorld, &cRect, lRect, 0, 0, color, lightcolor, darkcolor);
                    }
                    break;

                case buttonOffLineKind:
                    cRect.left = c->hiliteLeft - 2;
                    cRect.top = lRect.top + (( count + lineCorrect) * ( gDirectText->height /* * 2 */));
                    cRect.right = c->hiliteRight + 2;
                    cRect.bottom = cRect.top + gDirectText->height /* * 2 */;

                    GetRGBTranslateColorShade(&color, lineColor, MEDIUM);
                    GetRGBTranslateColorShade(&lightcolor, lineColor, LIGHT);
                    GetRGBTranslateColorShade(&darkcolor, lineColor, DARK);
                    DrawNateShadedRect(gOffWorld, &cRect, lRect, 0, 0, color, lightcolor, darkcolor);
                    break;

                case buttonOnLineKind:
                    cRect.left = c->hiliteLeft - 2;
                    cRect.top = lRect.top + (( count + lineCorrect) * ( gDirectText->height /* * 2 */));
                    cRect.right = lRect.right; //c->hiliteRight + 2;
                    cRect.bottom = cRect.top + gDirectText->height /* * 2 */;

                    GetRGBTranslateColorShade(&color, lineColor, LIGHT);
                    GetRGBTranslateColorShade(&lightcolor, lineColor, VERY_LIGHT);
                    GetRGBTranslateColorShade(&darkcolor, lineColor, MEDIUM);
                    DrawNateShadedRect(gOffWorld, &cRect, lRect, 0, 0, color, lightcolor, darkcolor);
                    textcolor = RgbColor::kBlack;
                    break;

            }
        } else
        {
            if ( c->selectable == selectDim)
                GetRGBTranslateColorShade(&textcolor, lineColor, MEDIUM);
            else
                GetRGBTranslateColorShade(&textcolor, lineColor, VERY_LIGHT);
        }
        MoveTo( mRect.left + kMiniScreenLeftBuffer, mRect.top + (count + lineCorrect) * ((
            gDirectText->height) /* * 2 */) + gDirectText->ascent /* * 2 */);
        DrawDirectTextStringClipped(PStringPiece(c->string), textcolor, gOffWorld, lRect, 0, 0);
        c++;
    }

    NormalizeColors();
    DrawInRealWorld();
    NormalizeColors();
}

void DrawAndShowMiniScreenLine( long whichLine)

{
    Rect                tRect;
    Rect            lRect, cRect;
    miniScreenLineType  *c;
    RgbColor            color, lightcolor, darkcolor, textcolor;
    unsigned char lineColor = kMiniScreenColor;
    long                lineCorrect = 0;

    if ( whichLine < 0) return;

    mSetDirectFont( kComputerFontNum);

    DrawInOffWorld();
    if ( whichLine < kMiniScreenCharHeight)
    {
        lRect = Rect(kMiniScreenLeft, kMiniScreenTop + globals()->gInstrumentTop, kMiniScreenRight,
                    kMiniScreenBottom + globals()->gInstrumentTop);
        cRect = lRect;
        cRect.top = lRect.top + whichLine * gDirectText->height;
        cRect.bottom = cRect.top + gDirectText->height;
    } else
    {
        lRect = Rect(kButBoxLeft, kButBoxTop + globals()->gInstrumentTop, kButBoxRight,
                    kButBoxBottom + globals()->gInstrumentTop);
        lineCorrect = -kMiniScreenCharHeight;
        lineColor = kMiniButColor;
        cRect = lRect;
        cRect.top = lRect.top + (whichLine - kMiniScreenCharHeight) * gDirectText->height;
        cRect.bottom = cRect.top + gDirectText->height;
    }

    GetRGBTranslateColorShade(&color, lineColor, DARKEST);
    DrawNateRect(gOffWorld, &cRect, 0, 0, color);

    c = globals()->gMiniScreenData.lineData.get() + whichLine;

    if ( c->underline)
    {
        MoveTo( lRect.left, lRect.top + (whichLine + lineCorrect) * ((
            gDirectText->height) /* * 2 */) + gDirectText->ascent /* * 2 */);
        SetTranslateColorShadeFore( lineColor, MEDIUM);
        MacLineTo(gActiveWorld, lRect.right - 1, lRect.top + (whichLine + lineCorrect) * ((
            gDirectText->height) /* * 2 */) + gDirectText->ascent /* * 2 */);
    }

    if ( c->hiliteLeft < c->hiliteRight)
    {
        if ( c->selectable == selectDim)
            GetRGBTranslateColorShade(&textcolor, lineColor, VERY_DARK);
        else
            GetRGBTranslateColorShade(&textcolor, lineColor, VERY_LIGHT);
        switch( c->lineKind)
        {
            case plainLineKind:
                    if ( c->hiliteRight > c->hiliteLeft)
                    {
                        cRect.left = c->hiliteLeft;
                        cRect.top = lRect.top + (( whichLine + lineCorrect) * ( gDirectText->height /* * 2 */));
                        cRect.right = c->hiliteRight;
                        cRect.bottom = cRect.top + gDirectText->height /* * 2 */;
//                      color = GetTranslateColorShade( lineColor, DARK);
                        GetRGBTranslateColorShade(&color, lineColor, DARK);
                        GetRGBTranslateColorShade(&lightcolor, lineColor, MEDIUM);
                        GetRGBTranslateColorShade(&darkcolor, lineColor, DARKER);
                        DrawNateShadedRect(gOffWorld, &cRect, lRect, 0, 0, color, lightcolor, darkcolor);
                    }
                break;

            case buttonOffLineKind:
                cRect.left = c->hiliteLeft - 2;
                cRect.top = lRect.top + (( whichLine + lineCorrect) * ( gDirectText->height /* * 2 */));
                cRect.right = c->hiliteRight + 2;
                cRect.bottom = cRect.top + gDirectText->height /* * 2 */;

                GetRGBTranslateColorShade(&color, lineColor, MEDIUM);
                GetRGBTranslateColorShade(&lightcolor, lineColor, LIGHT);
                GetRGBTranslateColorShade(&darkcolor, lineColor, DARK);
                DrawNateShadedRect(gOffWorld, &cRect, lRect, 0, 0, color, lightcolor, darkcolor);
                break;

            case buttonOnLineKind:
                cRect.left = c->hiliteLeft - 2;
                cRect.top = lRect.top + (( whichLine + lineCorrect) * ( gDirectText->height /* * 2 */));
                cRect.right = lRect.right; //c->hiliteRight + 2;
                cRect.bottom = cRect.top + gDirectText->height /* * 2 */;

                GetRGBTranslateColorShade(&color, lineColor, LIGHT);
                GetRGBTranslateColorShade(&lightcolor, lineColor, VERY_LIGHT);
                GetRGBTranslateColorShade(&darkcolor, lineColor, MEDIUM);
                DrawNateShadedRect(gOffWorld, &cRect, lRect, 0, 0, color, lightcolor, darkcolor);
                textcolor = RgbColor::kBlack;
                break;
        }
    } else
    {
        if ( c->selectable == selectDim)
            GetRGBTranslateColorShade(&textcolor, lineColor, MEDIUM);
        else
            GetRGBTranslateColorShade(&textcolor, lineColor, VERY_LIGHT);
    }
    MoveTo( lRect.left + kMiniScreenLeftBuffer, lRect.top + (whichLine + lineCorrect) * ((
        gDirectText->height) /* * 2 */) + gDirectText->ascent /* * 2 */);

    DrawDirectTextStringClipped(PStringPiece(c->string), textcolor, gOffWorld, lRect, 0, 0);
    NormalizeColors();
    DrawInRealWorld();
    NormalizeColors();

    tRect.left = lRect.left;
    tRect.right = kMiniScreenRight;
    tRect.top = lRect.top + (( whichLine + lineCorrect) * ( gDirectText->height /* * 2 */));
    tRect.bottom = tRect.top + gDirectText->height/* * 2 */;
    CopyBits(gOffWorld, gActiveWorld, tRect, tRect);
}

void ShowWholeMiniScreen( void)

{
    Rect                tRect;

    tRect = Rect(kMiniScreenLeft, kMiniScreenTop + globals()->gInstrumentTop, kMiniScreenRight,
                kMiniScreenBottom + globals()->gInstrumentTop);
    CopyBits(gOffWorld, gActiveWorld, tRect, tRect);
    tRect = Rect(kButBoxLeft, kButBoxTop + globals()->gInstrumentTop, kButBoxRight,
                kButBoxBottom + globals()->gInstrumentTop);
    CopyBits(gOffWorld, gActiveWorld, tRect, tRect);
}

void MakeMiniScreenFromIndString( short whichString)

{
    Str255              s, keyname;
    unsigned char       *c, *keyc, len, keyName[kKeyNameLength], keyNameLen;
    miniScreenLineType  *line;
    short               lineNum = 0, charNum, count;
    Rect                mRect;

    mSetDirectFont( kComputerFontNum);

    mRect = Rect(kMiniScreenLeft, kMiniScreenTop + globals()->gInstrumentTop, kMiniScreenRight,
                kMiniScreenBottom + globals()->gInstrumentTop);
    ClearMiniScreenLines();
    globals()->gMiniScreenData.currentScreen = whichString;

    GetIndString( s, kMiniScreenStringID, whichString);
    c = s;
    len = *c;
    c++;
    charNum = 1;
    line = globals()->gMiniScreenData.lineData.get();
    globals()->gMiniScreenData.selectLine = kMiniScreenNoLineSelected;

    while (( len > 0) && ( lineNum < kMiniScreenTrueLineNum))
    {
        while ( *c == kMiniScreenSpecChar)
        {
            c++;
            len--;

            switch( *c)
            {
                case kUnderlineEndLineChar:
                    line->underline = true;

                    // ||| NO BREAK
                    // VVV fall through to kEndLineChar
                case kEndLineChar:
                    charNum = 1;
                    lineNum++;
                    if ( lineNum == kMiniScreenCharHeight)
                    {
                        mRect = Rect(kButBoxLeft, kButBoxTop + globals()->gInstrumentTop, kButBoxRight,
                                    kButBoxBottom + globals()->gInstrumentTop);
                    }

                    line++;
                    c++;
                    len--;
                    break;

                case kSelectableLineChar:
                    line->selectable = selectable;
                    if ( globals()->gMiniScreenData.selectLine == kMiniScreenNoLineSelected)
                    {
                        globals()->gMiniScreenData.selectLine = lineNum;
                        line->hiliteLeft = mRect.left;
                        line->hiliteRight = mRect.right;
                    }
                    c++;
                    len--;
                    break;

                case kStartHiliteChar:
                    c++;
                    len--;
                    line->hiliteLeft = mRect.left + kMiniScreenLeftBuffer + gDirectText->logicalWidth * (charNum - 1);
                    break;

                case kEndHiliteChar:
                    c++;
                    len--;
                    line->hiliteRight = mRect.left + kMiniScreenLeftBuffer + gDirectText->logicalWidth * (charNum - 1) - 1;
                    break;

                case kIntoButtonChar:
                    line->lineKind = buttonOffLineKind;
                    line->whichButton = kInLineButton;
                    line->hiliteLeft = mRect.left + kMiniScreenLeftBuffer + gDirectText->logicalWidth * (charNum - 1);

                    {
                        sfz::String key_name;
                        GetKeyNumName(
                                Preferences::preferences()->key(kCompAcceptKeyNum), &key_name);
                        string_to_pstring(key_name, keyname);
                    }
                    keyc = keyname;
                    keyNameLen = *keyc;
                    keyc++;

                    for ( count = 0; count < kKeyNameLength; count++)
                    {
                        keyName[count] = ' ';
                    }

                    for ( count = 0; count < keyNameLen; count++)
                    {
                        keyName[count + ((kKeyNameLength  - keyNameLen) / 2)] =
                            *keyc;
                        keyc++;
                    }

                    for ( count = 0; count < kKeyNameLength; count++)
                    {
                        if ( charNum > kMiniScreenCharWidth)
                        {
                            charNum = 1;
                            lineNum++;
                            if ( lineNum == kMiniScreenCharHeight)
                            {
                                mRect = Rect(kButBoxLeft,
                                    kButBoxTop + globals()->gInstrumentTop,
                                    kButBoxRight,
                                    kButBoxBottom + globals()->gInstrumentTop);
                            }

                            line++;
                        }
                        line->string[charNum] = keyName[count];
                        charNum++;
                    }

                    line->hiliteRight = mRect.left + kMiniScreenLeftBuffer + gDirectText->logicalWidth * (charNum - 1) - 1;

                    c++;
                    len--;
                    break;

                case kOutOfButtonChar:
                    line->lineKind = buttonOffLineKind;
                    line->whichButton = kOutLineButton;
                    line->hiliteLeft = mRect.left + kMiniScreenLeftBuffer + gDirectText->logicalWidth * (charNum - 1);

                    {
                        sfz::String key_name;
                        GetKeyNumName(
                                Preferences::preferences()->key(kCompCancelKeyNum), &key_name);
                        string_to_pstring(key_name, keyname);
                    }
                    keyc = keyname;
                    keyNameLen = *keyc;
                    keyc++;

                    for ( count = 0; count < kKeyNameLength; count++)
                    {
                        keyName[count] = ' ';
                    }

                    for ( count = 0; count < keyNameLen; count++)
                    {
                        keyName[count + ((kKeyNameLength  - keyNameLen) / 2)] =
                            *keyc;
                        keyc++;
                    }

                    for ( count = 0; count < kKeyNameLength; count++)
                    {
                        if ( charNum > kMiniScreenCharWidth)
                        {
                            charNum = 1;
                            lineNum++;
                            if ( lineNum == kMiniScreenCharHeight)
                            {
                                mRect = Rect(kButBoxLeft, kButBoxTop + globals()->gInstrumentTop, kButBoxRight,
                                            kButBoxBottom + globals()->gInstrumentTop);
                            }

                            line++;
                        }
                        line->string[charNum] = keyName[count];
                        charNum++;
                    }

                    line->hiliteRight = mRect.left + kMiniScreenLeftBuffer + gDirectText->logicalWidth * (charNum - 1) - 1;

                    c++;
                    len--;
                    break;

                case kMiniScreenSpecChar:
                    if ( charNum > kMiniScreenCharWidth)
                    {
                        charNum = 1;
                        lineNum++;
                        if ( lineNum == kMiniScreenCharHeight)
                        {
                            mRect = Rect(kButBoxLeft, kButBoxTop + globals()->gInstrumentTop, kButBoxRight,
                                        kButBoxBottom + globals()->gInstrumentTop);
                        }

                        line++;
                    }
                    line->string[charNum] = *c;
                    charNum++;
                    c++;
                    len--;
                    break;

                default:
                    break;
            }
        }

        if (( len > 0) && ( lineNum < kMiniScreenTrueLineNum))
        {
            if ( charNum > kMiniScreenCharWidth)
            {
                charNum = 1;
                lineNum++;
                if ( lineNum == kMiniScreenCharHeight)
                {
                    mRect = Rect(kButBoxLeft, kButBoxTop + globals()->gInstrumentTop, kButBoxRight,
                                kButBoxBottom + globals()->gInstrumentTop);
                }

                line++;
            }
            line->string[charNum] = *c;
            charNum++;

            len--;
            c++;
        }
    }
}

void MiniComputerHandleKeys( unsigned long theseKeys, unsigned long lastKeys)

{
    miniScreenLineType  *line;
    long                count, scrap;
    Rect                mRect;

    if (( theseKeys | lastKeys) & kCompAcceptKey)
    {
        // find out which line, if any, contains this button
        line = globals()->gMiniScreenData.lineData.get();
        count = 0;
        while (( line->whichButton !=kInLineButton) && ( count < kMiniScreenTrueLineNum))
        {
            count++;
            line++;
        }

        // hilite/unhilite this button
        if ( count < kMiniScreenTrueLineNum)
        {
            if (( theseKeys & kCompAcceptKey) && ( line->lineKind != buttonOnLineKind))
            {
                line->lineKind = buttonOnLineKind;
                DrawAndShowMiniScreenLine( count);
                mPlayBeep3;
            } else if ((!( theseKeys & kCompAcceptKey)) && ( line->lineKind != buttonOffLineKind))
            {
                line->lineKind = buttonOffLineKind;
                DrawAndShowMiniScreenLine( count);

                MiniComputerDoAccept();
            }
        }
    }

    if (( theseKeys | lastKeys) & kCompCancelKey)
    {
        // find out which line, if any, contains this button
        line = globals()->gMiniScreenData.lineData.get();
        count = 0;
        while (( line->whichButton !=kOutLineButton) && ( count < kMiniScreenTrueLineNum))
        {
            count++;
            line++;
        }

        if ( count < kMiniScreenCharHeight)
        {
            mRect = Rect(kMiniScreenLeft, kMiniScreenTop + globals()->gInstrumentTop, kMiniScreenRight,
                        kMiniScreenBottom + globals()->gInstrumentTop);
        } else
        {
            mRect = Rect(kButBoxLeft, kButBoxTop + globals()->gInstrumentTop, kButBoxRight,
                        kButBoxBottom + globals()->gInstrumentTop);
        }

        // hilite/unhilite this button
        if ( count < kMiniScreenTrueLineNum)
        {
            if (( theseKeys & kCompCancelKey) && ( line->lineKind != buttonOnLineKind))
            {
                line->lineKind = buttonOnLineKind;
                DrawAndShowMiniScreenLine( count);
                mPlayBeep3;
            } else if ((!( theseKeys & kCompCancelKey)) && ( line->lineKind != buttonOffLineKind))
            {
                line->lineKind = buttonOffLineKind;
                DrawAndShowMiniScreenLine( count);

                MiniComputerDoCancel();
            }
        }
    }
    if (( theseKeys & kCompUpKey) && ( !(lastKeys & kCompUpKey)) && ( globals()->gMiniScreenData.selectLine !=
            kMiniScreenNoLineSelected))
    {
        scrap = globals()->gMiniScreenData.selectLine;
        line = globals()->gMiniScreenData.lineData.get() + globals()->gMiniScreenData.selectLine;
        line->hiliteLeft = line->hiliteRight = 0;
        do
        {
            line--;
            globals()->gMiniScreenData.selectLine--;
            if ( globals()->gMiniScreenData.selectLine < 0)
            {
                globals()->gMiniScreenData.selectLine = kMiniScreenTrueLineNum - 1;
                line = globals()->gMiniScreenData.lineData.get() + kMiniScreenTrueLineNum - 1L;
            }
        } while ( line->selectable == cannotSelect);

        if ( globals()->gMiniScreenData.selectLine < kMiniScreenCharHeight)
        {
            mRect = Rect(kMiniScreenLeft, kMiniScreenTop + globals()->gInstrumentTop, kMiniScreenRight,
                        kMiniScreenBottom + globals()->gInstrumentTop);
        } else
        {
            mRect = Rect(kButBoxLeft, kButBoxTop + globals()->gInstrumentTop, kButBoxRight,
                        kButBoxBottom + globals()->gInstrumentTop);
        }

        line->hiliteLeft = mRect.left;
        line->hiliteRight = mRect.right;
        if ( scrap != globals()->gMiniScreenData.selectLine)
        {
            DrawAndShowMiniScreenLine( globals()->gMiniScreenData.selectLine);
            DrawAndShowMiniScreenLine( scrap);
        }
    }

    if (( theseKeys & kCompDownKey) && ( !(lastKeys & kCompDownKey)) && ( globals()->gMiniScreenData.selectLine !=
            kMiniScreenNoLineSelected))
    {
        scrap = globals()->gMiniScreenData.selectLine;
        line = globals()->gMiniScreenData.lineData.get() + globals()->gMiniScreenData.selectLine;
        line->hiliteLeft = line->hiliteRight = 0;
        do
        {
            line++;
            globals()->gMiniScreenData.selectLine++;
            if ( globals()->gMiniScreenData.selectLine >= kMiniScreenTrueLineNum)
            {
                globals()->gMiniScreenData.selectLine = 0;
                line = globals()->gMiniScreenData.lineData.get();
            }
        } while ( line->selectable == cannotSelect);

        if ( globals()->gMiniScreenData.selectLine < kMiniScreenCharHeight)
        {
            mRect = Rect(kMiniScreenLeft, kMiniScreenTop + globals()->gInstrumentTop, kMiniScreenRight,
                        kMiniScreenBottom + globals()->gInstrumentTop);
        } else
        {
            mRect = Rect(kButBoxLeft, kButBoxTop + globals()->gInstrumentTop, kButBoxRight,
                        kButBoxBottom + globals()->gInstrumentTop);
        }

        line->hiliteLeft = mRect.left;
        line->hiliteRight = mRect.right;
        if ( scrap != globals()->gMiniScreenData.selectLine)
        {
            DrawAndShowMiniScreenLine( globals()->gMiniScreenData.selectLine);
            DrawAndShowMiniScreenLine( scrap);
        }
    }


}

void MiniComputerHandleNull( long unitsToDo)

{
    destBalanceType     *buildAtObject = nil;
    long                count;
    spaceObjectType     *realObject = nil, *myObject = nil, newObject;

    globals()->gMiniScreenData.pollTime += unitsToDo;
    if ( globals()->gMiniScreenData.pollTime > kMiniComputerPollTime)
    {
        globals()->gMiniScreenData.pollTime = 0;
        UpdateMiniScreenLines();
        /*
        switch( globals()->gMiniScreenData.currentScreen)
        {
            case kBuildMiniScreen:
                admiral = ( admiralType *)*globals()->gAdmiralData + globals()->gPlayerAdmiralNumber;
                line = globals()->gMiniScreenData.lineData.get() + kBuildScreenWhereNameLine;
                if ( line->value != admiral->buildAtObject)
                {
                    MiniComputerSetBuildStrings();
                    DrawMiniScreen();
                    ShowWholeMiniScreen();
                } else
                {
                    line = globals()->gMiniScreenData.lineData.get() + kBuildScreenFirstTypeLine;
                    lineNum = kBuildScreenFirstTypeLine;

                    for ( count = 0; count < kMaxShipCanBuild; count++)
                    {
                        buildObject = (baseObjectType *)line->sourceData;
                        if ( buildObject != nil)
                        {
                            if ( buildObject->price > admiral->cash)
                            {
                                if ( line->selectable != selectDim)
                                {
                                    line->selectable = selectDim;
                                    DrawAndShowMiniScreenLine( lineNum);
                                }
                            } else
                            {
                                if (line->selectable != selectable)
                                {
                                    if ( globals()->gMiniScreenData.selectLine == kMiniScreenNoLineSelected)
                                    {
                                        globals()->gMiniScreenData.selectLine = lineNum;
                                        line->hiliteLeft = mRect.left;
                                        line->hiliteRight = mRect.right;
                                    }
                                    line->selectable = selectable;
                                    DrawAndShowMiniScreenLine( lineNum);
                                }
                            }
                        }
                        line++;
                        lineNum++;
                    }
                }

                break;
        }
        */


        // handle control/command/selected object

        myObject = mGetMiniObjectPtr( kMiniSelectObjectNum);
        count = GetAdmiralConsiderObject( globals()->gPlayerAdmiralNumber);
        if ( count >= 0)
        {
            realObject = gSpaceObjectData.get() + count;
            mCopyMiniSpaceObject( newObject, *realObject);
        } else
        {
            newObject.id = -1;
            newObject.beamType = -1;
            newObject.pulseType = -1;
            newObject.specialType = -1;
            newObject.destinationLocation.h = newObject.destinationLocation.v = -1;
            newObject.destinationObject = -1;
            newObject.destObjectPtr = nil;
            newObject.health = 0;
            newObject.energy = 0;
            newObject.whichBaseObject = -1;
            newObject.pixResID = -1;
            newObject.attributes = 0;
            newObject.baseType = nil;
        }
        UpdateMiniShipData( myObject, &newObject, YELLOW, kMiniSelectTop, kMiniSelectObjectNum + 1);

        myObject = mGetMiniObjectPtr( kMiniTargetObjectNum);
        count = GetAdmiralDestinationObject( globals()->gPlayerAdmiralNumber);
        if ( count >= 0)
        {
            realObject = gSpaceObjectData.get() + count;
            mCopyMiniSpaceObject( newObject, *realObject);
        } else
        {
            newObject.id = -1;
            newObject.beamType = -1;
            newObject.pulseType = -1;
            newObject.specialType = -1;
            newObject.destinationLocation.h = newObject.destinationLocation.v = -1;
            newObject.destinationObject = -1;
            newObject.destObjectPtr = nil;
            newObject.health = 0;
            newObject.energy = 0;
            newObject.whichBaseObject = -1;
            newObject.pixResID = -1;
            newObject.attributes = 0;
            newObject.baseType = nil;
        }
        UpdateMiniShipData( myObject, &newObject, SKY_BLUE, kMiniTargetTop, kMiniTargetObjectNum + 1);

        count = GetAdmiralBuildAtObject( globals()->gPlayerAdmiralNumber);
        if ( count >= 0)
        {
            buildAtObject = mGetDestObjectBalancePtr( GetAdmiralBuildAtObject( globals()->gPlayerAdmiralNumber));
            count = buildAtObject->buildTime * kMiniBuildTimeHeight;
            if ( buildAtObject->totalBuildTime > 0)
            {
                count /= buildAtObject->totalBuildTime;
            } else count = 0;
        } else count = 0;
        if ( count != globals()->gMiniScreenData.buildTimeBarValue)
        {
            globals()->gMiniScreenData.buildTimeBarValue = count;
            DrawBuildTimeBar( globals()->gMiniScreenData.buildTimeBarValue);
        }
    }
    if ( globals()->gPlayerShipNumber >= 0)
    {
        myObject = gSpaceObjectData.get() + globals()->gPlayerShipNumber;
        if ( myObject->active)
        {
            UpdatePlayerAmmo(
                (myObject->pulseType >= 0) ?
                    (( myObject->pulseBase->frame.weapon.ammo > 0) ?
                        ( myObject->pulseAmmo):(-1)):
                    (-1),
                (myObject->beamType >= 0) ?
                    (( myObject->beamBase->frame.weapon.ammo > 0) ?
                        ( myObject->beamAmmo):(-1)):
                    (-1),
                (myObject->specialType >= 0) ?
                    (( myObject->specialBase->frame.weapon.ammo > 0) ?
                        ( myObject->specialAmmo):(-1)):
                    (-1)
                );
        }
    }
}


// only for updating volitile lines--doesn't draw whole screen!
void UpdateMiniScreenLines( void)

{
    admiralType         *admiral = nil;
    miniScreenLineType  *line = nil;
    baseObjectType      *buildObject = nil;
    long                lineNum, count;
    Rect                mRect;

    mRect = Rect(kMiniScreenLeft, kMiniScreenTop + globals()->gInstrumentTop, kMiniScreenRight,
                        kMiniScreenBottom + globals()->gInstrumentTop);
    switch( globals()->gMiniScreenData.currentScreen)
    {
        case kBuildMiniScreen:
            admiral = globals()->gAdmiralData.get() + globals()->gPlayerAdmiralNumber;
            line = globals()->gMiniScreenData.lineData.get() +
                kBuildScreenWhereNameLine;
            if ( line->value !=
                GetAdmiralBuildAtObject( globals()->gPlayerAdmiralNumber))
            {
                if ( globals()->gMiniScreenData.selectLine !=
                        kMiniScreenNoLineSelected)
                {
                    line = globals()->gMiniScreenData.lineData.get()
                        + globals()->gMiniScreenData.selectLine;
                    line->hiliteLeft = line->hiliteRight = 0;
                    globals()->gMiniScreenData.selectLine =
                        kMiniScreenNoLineSelected;
                }
                MiniComputerSetBuildStrings();
                DrawMiniScreen();
                ShowWholeMiniScreen();
            } else if ( GetAdmiralBuildAtObject( globals()->gPlayerAdmiralNumber)
                >= 0)
            {
                line = globals()->gMiniScreenData.lineData.get() + kBuildScreenFirstTypeLine;
                lineNum = kBuildScreenFirstTypeLine;

                for ( count = 0; count < kMaxShipCanBuild; count++)
                {
                    buildObject = line->sourceData;
                    if ( buildObject != nil)
                    {
                        if ( buildObject->price > mFixedToLong(admiral->cash))
                        {
                            if ( line->selectable != selectDim)
                            {
                                line->selectable = selectDim;
                                DrawAndShowMiniScreenLine( lineNum);
                            }
                        } else
                        {
                            if (line->selectable != selectable)
                            {
                                if ( globals()->gMiniScreenData.selectLine ==
                                    kMiniScreenNoLineSelected)
                                {
                                    globals()->gMiniScreenData.selectLine =
                                        lineNum;
                                    line->hiliteLeft = mRect.left;
                                    line->hiliteRight = mRect.right;
                                }
                                line->selectable = selectable;
                                DrawAndShowMiniScreenLine( lineNum);
                            }
                        }
                    }
                    line++;
                    lineNum++;
                }
            }

            break;

        case kStatusMiniScreen:
            for ( count = kStatusMiniScreenFirstLine; count <
                kMiniScreenCharHeight; count++)
            {
                line =
                    globals()->gMiniScreenData.lineData.get() +
                        count;
                lineNum = MiniComputerGetStatusValue( count);
                if ( line->value != lineNum)
                {
                    line->value = lineNum;
                    MiniComputerMakeStatusString( count, line->string);
                    DrawAndShowMiniScreenLine( count);
                }

            }
            break;
    }
}

void UpdatePlayerAmmo( long thisOne, long thisTwo, long thisSpecial)

{
    static long         lastOne = -1, lastTwo = -1, lastSpecial = -1;
    RgbColor            lightcolor;
    Rect            lRect, clipRect;
    Rect                mRect;
    bool             update = false;
    unsigned char       digit[3], *digitp;
    long                scratch;

    mSetDirectFont( kComputerFontNum);

    clipRect.left = kMiniScreenLeft;
    lRect.top = clipRect.top = kMiniAmmoTop + globals()->gInstrumentTop;
    clipRect.right = kMiniScreenRight;
    lRect.bottom = clipRect.bottom = kMiniAmmoBottom + globals()->gInstrumentTop;

    if ( thisOne != lastOne)
    {
        GetRGBTranslateColorShade(&lightcolor, RED, VERY_LIGHT);

        lRect.left = kMiniAmmoLeftOne;
        lRect.right = lRect.left + kMiniAmmoSingleWidth;

        DrawNateRect(gOffWorld, &lRect, 0, 0, RgbColor::kBlack);

        if ( thisOne >= 0)
        {
            digitp = digit;
            *digitp = 3;
            digitp++;
            scratch = (thisOne % 1000) / 100;
            *digitp = '0' + scratch;

            digitp++;
            scratch = (thisOne % 100) / 10;
            *digitp = '0' + scratch;

            digitp++;
            scratch = thisOne % 10;
            *digitp = '0' + scratch;

            MoveTo( lRect.left + kMiniAmmoTextHBuffer, lRect.bottom-1/*lRect.top + gDirectText->ascent*/);

            DrawDirectTextStringClipped(
                    PStringPiece(digit), lightcolor, gOffWorld, clipRect, 0, 0);
        }

        update = true;
    }

    if ( thisTwo != lastTwo)
    {
        GetRGBTranslateColorShade(&lightcolor, PALE_GREEN, VERY_LIGHT);

        lRect.left = kMiniAmmoLeftTwo;
        lRect.right = lRect.left + kMiniAmmoSingleWidth;

        DrawNateRect(gOffWorld, &lRect, 0, 0, RgbColor::kBlack);

        if ( thisTwo >= 0)
        {
            digitp = digit;
            *digitp = 3;
            digitp++;
            scratch = (thisTwo % 1000) / 100;
            *digitp = '0' + scratch;

            digitp++;
            scratch = (thisTwo % 100) / 10;
            *digitp = '0' + scratch;

            digitp++;
            scratch = thisTwo % 10;
            *digitp = '0' + scratch;

            MoveTo( lRect.left + kMiniAmmoTextHBuffer, lRect.bottom-1/*lRect.top + gDirectText->ascent*/);

            DrawDirectTextStringClipped(
                    PStringPiece(digit), lightcolor, gOffWorld, clipRect, 0, 0);
        }
        update = true;
    }

    if ( thisSpecial != lastSpecial)
    {
        GetRGBTranslateColorShade(&lightcolor, ORANGE, VERY_LIGHT);

        lRect.left = kMiniAmmoLeftSpecial;
        lRect.right = lRect.left + kMiniAmmoSingleWidth;

        DrawNateRect(gOffWorld, &lRect, 0, 0, RgbColor::kBlack);

        if ( thisSpecial >= 0)
        {
            digitp = digit;
            *digitp = 3;
            digitp++;
            scratch = (thisSpecial % 1000) / 100;
            *digitp = '0' + scratch;

            digitp++;
            scratch = (thisSpecial % 100) / 10;
            *digitp = '0' + scratch;

            digitp++;
            scratch = thisSpecial % 10;
            *digitp = '0' + scratch;

            MoveTo( lRect.left + kMiniAmmoTextHBuffer, lRect.bottom-1/*lRect.top + gDirectText->ascent*/);

            DrawDirectTextStringClipped(
                    PStringPiece(digit), lightcolor, gOffWorld, clipRect, 0, 0);
        }
        update = true;
    }

    if ( update)
    {
        mRect.left = clipRect.left;
        mRect.right = clipRect.right;
        mRect.top = clipRect.top;
        mRect.bottom = clipRect.bottom;

        // copy the dirty rect
        CopyBits(gOffWorld, gActiveWorld, mRect, mRect);
    }

    lastOne = thisOne;
    lastTwo = thisTwo;
    lastSpecial = thisSpecial;
}


void UpdateMiniShipData( spaceObjectType *oldObject, spaceObjectType *newObject, unsigned char headerColor,
                    short screenTop, short whichString)

{
    RgbColor            color, lightcolor, darkcolor;
    Str255              s;
    spritePix           aSpritePix;
    coordPointType      coord;
    Point               where;
    short               whichShape;
    spaceObjectType     *dObject = nil;
    long                tlong, thisScale;
    Rect                mRect;
    Rect            lRect, dRect, spriteRect, uRect, clipRect;

    // get ready to draw in offworld
    DrawInOffWorld();

    clipRect.left = kMiniScreenLeft;
    clipRect.top = screenTop + globals()->gInstrumentTop;
    clipRect.right = kMiniScreenRight;
    clipRect.bottom = clipRect.top + 64;

    mSetDirectFont( kComputerFontNum);

    uRect.left = uRect.top = uRect.bottom = -1;

    if ( oldObject->id != newObject->id)
    {

        mBlackMiniScreenLine( screenTop + globals()->gInstrumentTop, 0, 0, kMiniScreenWidth, lRect, gOffWorld);
        GetRGBTranslateColorShade(&color, headerColor, LIGHT);
        GetRGBTranslateColorShade(&lightcolor, headerColor, VERY_LIGHT);
        GetRGBTranslateColorShade(&darkcolor, headerColor, MEDIUM);

        DrawNateShadedRect(gOffWorld, &lRect, clipRect, 0, 0, color, lightcolor, darkcolor);

        MoveTo( lRect.left + kMiniScreenLeftBuffer, lRect.top + gDirectText->ascent);
        GetIndString( s, kMiniDataStringID, whichString);

        DrawDirectTextStringClipped(PStringPiece(s), RgbColor::kBlack, gOffWorld, clipRect, 0, 0);
        uRect = lRect;
        uRect = clipRect;

        if ( newObject->attributes & kIsDestination)
        {
            // blacken the line for the object type name
            mBlackMiniScreenLine( screenTop + globals()->gInstrumentTop, kMiniNameLineNum, 0, kMiniScreenWidth, lRect, gOffWorld);

            // get the color for writing the name
            GetRGBTranslateColorShade(&color, PALE_GREEN, VERY_LIGHT);

            // move to the 1st line in the selection miniscreen
            MoveTo( lRect.left + kMiniScreenLeftBuffer, lRect.top + gDirectText->ascent);

            DrawDirectTextStringClipped(
                    PStringPiece(GetDestBalanceName(newObject->destinationObject)), color,
                    gOffWorld, clipRect, 0, 0);
            if ( uRect.left == -1)
            {
                uRect = lRect;
            } else
            {
                uRect.enlarge_to(lRect);
            }
        } else if ( oldObject->whichBaseObject != newObject->whichBaseObject)
        {

            // blacken the line for the object type name
            mBlackMiniScreenLine( screenTop + globals()->gInstrumentTop, kMiniNameLineNum, 0, kMiniScreenWidth, lRect, gOffWorld);

            if ( newObject->whichBaseObject >= 0)
            {

                // get the color for writing the name
                GetRGBTranslateColorShade(&color, PALE_GREEN, VERY_LIGHT);
                GetIndString( s, kSpaceObjectShortNameResID, newObject->whichBaseObject + 1);
//              SmallFixedToString( HackGetObjectStrength( newObject), s);

                // move to the 1st line in the selection miniscreen
                MoveTo( lRect.left + kMiniScreenLeftBuffer, lRect.top + gDirectText->ascent);

                // write the name
                DrawDirectTextStringClipped(PStringPiece(s), color, gOffWorld, clipRect, 0, 0);
            }

            if ( uRect.left == -1)
            {
                uRect = lRect;
            } else
            {
                uRect.enlarge_to(lRect);
            }
        }
    }
        // set the rect for drawing the "icon" of the object type

    if ( oldObject->pixResID != newObject->pixResID)
    {
        dRect.left = kMiniIconLeft;
        dRect.top = screenTop + globals()->gInstrumentTop + kMiniIconMacLineTop;
        dRect.right = kMiniScreenLeft + kMiniIconWidth;
        dRect.bottom = dRect.top + kMiniIconHeight;

        // erase the area

        DrawNateRect(gOffWorld, &dRect, 0, 0, RgbColor::kBlack);

        if (( newObject->whichBaseObject >= 0) && ( newObject->pixResID >= 0))
        {
            natePixType* pixTable = GetPixTable( newObject->pixResID);

            if (pixTable != nil) {
                if ( newObject->attributes & kIsSelfAnimated)
                    whichShape = newObject->baseType->frame.animation.firstShape >> kFixedBitShiftNumber;
                else
                    whichShape = 0;

                // get the picture data
                aSpritePix.data = GetNatePixTableNatePixData(*pixTable, whichShape);
                aSpritePix.center.h = GetNatePixTableNatePixHRef(*pixTable, whichShape);
                aSpritePix.center.v = GetNatePixTableNatePixVRef(*pixTable, whichShape);
                aSpritePix.width = GetNatePixTableNatePixWidth(*pixTable, whichShape);
                aSpritePix.height = GetNatePixTableNatePixHeight(*pixTable, whichShape);

                // calculate the correct size

                tlong = (kMiniIconHeight - 2) * SCALE_SCALE;
                tlong /= aSpritePix.height;
                thisScale = (kMiniIconWidth - 2) * SCALE_SCALE;
                thisScale /= aSpritePix.width;

                if ( tlong < thisScale) thisScale = tlong;
                if ( thisScale > SCALE_SCALE) thisScale = SCALE_SCALE;

                // calculate the correct position

                coord.h = aSpritePix.center.h;
                coord.h *= thisScale;
                coord.h >>= SHIFT_SCALE;
                tlong = aSpritePix.width;
                tlong *= thisScale;
                tlong >>= SHIFT_SCALE;
                where.h = ( kMiniIconWidth / 2) - ( tlong / 2);
                where.h += dRect.left + coord.h;

                coord.v = aSpritePix.center.v;
                coord.v *= thisScale;
                coord.v >>= SHIFT_SCALE;
                tlong = aSpritePix.height;
                tlong *= thisScale;
                tlong >>= SHIFT_SCALE;
                where.v = ( kMiniIconHeight / 2) - ( tlong / 2);
                where.v += dRect.top + coord.v;


                // draw the sprite

                OptScaleSpritePixInPixMap( &aSpritePix, where, thisScale,
                        &spriteRect, &dRect, gOffWorld);
            }
        }

        GetRGBTranslateColorShade(&color, PALE_GREEN, MEDIUM);
        DrawNateVBracket(gOffWorld, dRect, clipRect, 0, 0, color);

        if ( uRect.left == -1)
        {
            uRect = dRect;
        }
        else
        {
            uRect.enlarge_to(dRect);
        }
    }

    if ( oldObject->health != newObject->health)
    {
        dRect.left = kMiniHealthLeft;
        dRect.top = screenTop + globals()->gInstrumentTop + kMiniIconMacLineTop;
        dRect.right = dRect.left + kMiniBarWidth;
        dRect.bottom = dRect.top + kMiniIconHeight;

        // erase the area

        DrawNateRect(gOffWorld, &dRect, 0, 0, RgbColor::kBlack);

        if ( newObject->baseType != nil)
        {
            if (( newObject->baseType->health > 0) && ( newObject->health > 0))
            {
                tlong = newObject->health * kMiniBarHeight;
                tlong /= newObject->baseType->health;

                GetRGBTranslateColorShade(&color, SKY_BLUE, DARK);

                lRect.left = dRect.left + 2;
                lRect.top = dRect.top + 2;
                lRect.right = dRect.right - 2;
                lRect.bottom = dRect.bottom - 2 - tlong;
                DrawNateRect(gOffWorld, &lRect, 0, 0, color);

                GetRGBTranslateColorShade(&color, SKY_BLUE, LIGHT);
                lRect.top = dRect.bottom - 2 - tlong;
                lRect.bottom = dRect.bottom - 2;
                DrawNateRect(gOffWorld, &lRect, 0, 0, color);

                GetRGBTranslateColorShade(&color, SKY_BLUE, MEDIUM);
                DrawNateVBracket(gOffWorld, dRect, clipRect, 0, 0, color);
            }
        }


        if ( uRect.left == -1)
        {
            uRect = dRect;
        }
        else
        {
            uRect.enlarge_to(dRect);
        }

    }

    if (oldObject->energy != newObject->energy)
    {
        dRect.left = kMiniEnergyLeft;
        dRect.top = screenTop + globals()->gInstrumentTop + kMiniIconMacLineTop;
        dRect.right = dRect.left + kMiniBarWidth;
        dRect.bottom = dRect.top + kMiniIconHeight;

        // erase the area

        DrawNateRect(gOffWorld, &dRect, 0, 0, RgbColor::kBlack);

        if ( newObject->baseType != nil)
        {
            if (( newObject->baseType->energy > 0) && ( newObject->energy > 0))
            {
                tlong = newObject->energy * kMiniBarHeight;
                tlong /= newObject->baseType->energy;

                GetRGBTranslateColorShade(&color, YELLOW, DARK);

                lRect.left = dRect.left + 2;
                lRect.top = dRect.top + 2;
                lRect.right = dRect.right - 2;
                lRect.bottom = dRect.bottom - 2 - tlong;
                DrawNateRect(gOffWorld, &lRect, 0, 0, color);

                GetRGBTranslateColorShade(&color, YELLOW, LIGHT);
                lRect.top = dRect.bottom - 2 - tlong;
                lRect.bottom = dRect.bottom - 2;
                DrawNateRect(gOffWorld, &lRect, 0, 0, color);

                GetRGBTranslateColorShade(&color, YELLOW, MEDIUM);
                DrawNateVBracket(gOffWorld, dRect, clipRect, 0, 0, color);
            }
        }

        if ( uRect.left == -1)
        {
            uRect = dRect;
        }
        else
        {
            uRect.enlarge_to(dRect);
        }
    }

    if ( oldObject->beamType != newObject->beamType)
    {
        // blacken the line for the weapon1 name
        mBlackMiniScreenLine( screenTop + globals()->gInstrumentTop, kMiniWeapon1LineNum, kMiniRightColumnLeft, kMiniScreenWidth, lRect, gOffWorld);

        // get the color for writing the name
        GetRGBTranslateColorShade(&color, PALE_GREEN, VERY_LIGHT);

        // move to the 1st line in the selection miniscreen
        MoveTo( lRect.left, lRect.top + gDirectText->ascent);

        // write the name
        if ( newObject->beamType >= 0)
        {
            GetIndString(s, kSpaceObjectShortNameResID, newObject->beamType + 1);
            DrawDirectTextStringClipped(PStringPiece(s), color, gOffWorld, clipRect, 0, 0);
        }

        if ( uRect.left == -1)
        {
            uRect = lRect;
        }
        else
        {
            uRect.enlarge_to(lRect);
        }
    }

    if ( oldObject->pulseType != newObject->pulseType)
    {
        // blacken the line for the weapon1 name
        mBlackMiniScreenLine( screenTop + globals()->gInstrumentTop, kMiniWeapon2LineNum, kMiniRightColumnLeft, kMiniScreenWidth, lRect, gOffWorld);

        // get the color for writing the name
        GetRGBTranslateColorShade(&color, PALE_GREEN, VERY_LIGHT);

        // move to the 1st line in the selection miniscreen
        MoveTo( lRect.left, lRect.top + gDirectText->ascent);

        // write the name
        if ( newObject->pulseType >= 0)
        {
            GetIndString(s, kSpaceObjectShortNameResID, newObject->pulseType + 1);
            DrawDirectTextStringClipped(PStringPiece(s), color, gOffWorld, clipRect, 0, 0);
        }

        if ( uRect.left == -1)
        {
            uRect = lRect;
        }
        else
        {
            uRect.enlarge_to(lRect);
        }
    }

    if (( oldObject->specialType != newObject->specialType) && ( ! (newObject->attributes & kIsDestination)))
    {
        // blacken the line for the weapon1 name
        mBlackMiniScreenLine( screenTop + globals()->gInstrumentTop, kMiniWeapon3LineNum, kMiniRightColumnLeft, kMiniScreenWidth, lRect, gOffWorld);

        // get the color for writing the name
        GetRGBTranslateColorShade(&color, PALE_GREEN, VERY_LIGHT);

        // move to the 1st line in the selection miniscreen
        MoveTo( lRect.left, lRect.top + gDirectText->ascent);

        // write the name
        if ( newObject->specialType >= 0)
        {
            GetIndString(s, kSpaceObjectShortNameResID, newObject->specialType + 1);
            DrawDirectTextStringClipped(PStringPiece(s), color, gOffWorld, clipRect, 0, 0);
        }

        if ( uRect.left == -1)
        {
            uRect = lRect;
        }
        else
        {
            uRect.enlarge_to(lRect);
        }
    }

    if ( oldObject->destinationObject != newObject->destinationObject)
    {
        // blacken the line for the weapon1 name
        mBlackMiniScreenLine( screenTop + globals()->gInstrumentTop, kMiniDestLineNum, 0, kMiniScreenWidth, lRect, gOffWorld);

        // move to the 1st line in the selection miniscreen
        MoveTo( lRect.left, lRect.top + gDirectText->ascent);

        // write the name
        if ( newObject->destinationObject >= 0)
        {
            if ( newObject->destObjectPtr != nil)
            {
                dObject = newObject->destObjectPtr;

                // get the color for writing the name
                if ( dObject->owner == globals()->gPlayerAdmiralNumber)
                {
                    GetRGBTranslateColorShade(&color, GREEN, VERY_LIGHT);
                } else
                {
                    GetRGBTranslateColorShade(&color, RED, VERY_LIGHT);
                }

                if ( dObject->attributes & kIsDestination)
                {
                    DrawDirectTextStringClipped(
                            PStringPiece(GetDestBalanceName(dObject->destinationObject)), color,
                            gOffWorld, clipRect, 0, 0);
                } else
                {
                    GetIndString( s, kSpaceObjectNameResID, dObject->whichBaseObject + 1);
                    DrawDirectTextStringClipped(PStringPiece(s), color, gOffWorld, clipRect, 0, 0);
                }
            }
        }

        if ( uRect.left == -1)
        {
            uRect = lRect;
        }
        else
        {
            uRect.enlarge_to(lRect);
        }
    }

    NormalizeColors();
    DrawInRealWorld();
    NormalizeColors();
    mRect.left = uRect.left;
    mRect.right = uRect.right;
    mRect.top = uRect.top;
    mRect.bottom = uRect.bottom;

    // copy the dirty rect
    CopyBits(gOffWorld, gActiveWorld, mRect, mRect);

    mCopyMiniSpaceObject( *oldObject, *newObject);
}

void MiniComputerDoAccept( void)

{
    if (true) {  // TODO(sfiera): if non-networked.
        MiniComputerExecute( globals()->gMiniScreenData.currentScreen,
            globals()->gMiniScreenData.selectLine, globals()->gPlayerAdmiralNumber);
    } else {
#ifdef NETSPROCKET_AVAILABLE
        if ( !SendMenuMessage( globals()->gGameTime + gNetLatency, globals()->gMiniScreenData.currentScreen,
            globals()->gMiniScreenData.selectLine))
            StopNetworking();
#endif NETSPROCKET_AVAILABLE
    }
/*  spaceObjectType *anObject, *anotherObject;
    long            l;

    switch ( globals()->gMiniScreenData.currentScreen)
    {
        case kMainMiniScreen:
            switch ( globals()->gMiniScreenData.selectLine)
            {
                case kMainMiniBuild:
                    MakeMiniScreenFromIndString( kBuildMiniScreen);
                    MiniComputerSetBuildStrings();
                    DrawMiniScreen();
                    ShowWholeMiniScreen();
                    break;

                case kMainMiniSpecial:
                    MakeMiniScreenFromIndString( kSpecialMiniScreen);
                    DrawMiniScreen();
                    ShowWholeMiniScreen();
                    break;

                case kMainMiniMessage:
                    MakeMiniScreenFromIndString( kMessageMiniScreen);
                    DrawMiniScreen();
                    ShowWholeMiniScreen();
                    break;

                default:
                    break;
            }

            break;

        case kBuildMiniScreen:
            if ( globals()->gMiniScreenData.selectLine != kMiniScreenNoLineSelected)
                if (AdmiralScheduleBuild( globals()->gPlayerAdmiralNumber,
                    globals()->gMiniScreenData.selectLine - kBuildScreenFirstTypeLine) == false)
                {
                    mPlayBeepBad;
                }
            break;

        case kSpecialMiniScreen:
            switch ( globals()->gMiniScreenData.selectLine)
            {
                case kSpecialMiniTransfer:
                    l = GetAdmiralConsiderObject( globals()->gPlayerAdmiralNumber);
                    if (( globals()->gPlayerShipNumber != kNoShip) && ( l != kNoShip) && ( l != globals()->gPlayerShipNumber))
                    {
                        anObject = gSpaceObjectData.get() + globals()->gPlayerShipNumber;
                        anotherObject = gSpaceObjectData.get() + l;
                        if (( anObject->active) && (anObject->attributes & kIsHumanControlled) &&
                            (anotherObject->attributes & ( kCanAcceptDestination)))
                        {
                            ChangePlayerShipNumber( l);

                        }
                    }
                    break;

                case kSpecialMiniFire1:
                    l = GetAdmiralConsiderObject( globals()->gPlayerAdmiralNumber);
                    if (( l != kNoShip) && ( l != globals()->gPlayerShipNumber))
                    {
                        anObject = gSpaceObjectData.get() + l;
                        if (( anObject->active) &&
                            (anObject->attributes & ( kCanAcceptDestination)))
                        {
                            anObject->keysDown |= kOneKey | kManualOverrideFlag;
                        }
                    }
                    break;

                case kSpecialMiniFire2:
                    l = GetAdmiralConsiderObject( globals()->gPlayerAdmiralNumber);
                    if (( l != kNoShip) && ( l != globals()->gPlayerShipNumber))
                    {
                        anObject = gSpaceObjectData.get() + l;
                        if (( anObject->active) &&
                            (anObject->attributes & ( kCanAcceptDestination)))
                        {
                            anObject->keysDown |= kTwoKey | kManualOverrideFlag;
                        }
                    }
                    break;

                case kSpecialMiniFireSpecial:
                    l = GetAdmiralConsiderObject( globals()->gPlayerAdmiralNumber);
                    if (( l != kNoShip) && ( l != globals()->gPlayerShipNumber))
                    {
                        anObject = gSpaceObjectData.get() + l;
                        if (( anObject->active) &&
                            (anObject->attributes & ( kCanAcceptDestination)))
                        {
                            anObject->keysDown |= kEnterKey | kManualOverrideFlag;
                        }
                    }
                    break;

                case kSpecialMiniHold:
                    l = GetAdmiralConsiderObject( globals()->gPlayerAdmiralNumber);
                    if (( l != kNoShip) && ( l != globals()->gPlayerShipNumber))
                    {
                        anObject = gSpaceObjectData.get() + l;
                        SetObjectLocationDestination( anObject, &(anObject->location));
                    }
                    break;

                case kSpecialMiniGoToMe:
                    l = GetAdmiralConsiderObject( globals()->gPlayerAdmiralNumber);
                    if (( l != kNoShip) && ( l != globals()->gPlayerShipNumber) && ( globals()->gPlayerShipNumber != kNoShip))
                    {
                        anObject = gSpaceObjectData.get() + l;
                        anotherObject = gSpaceObjectData.get() + globals()->gPlayerShipNumber;
                        SetObjectLocationDestination( anObject, &(anotherObject->location));
                    }
                    break;

                default:
                    break;
            }
            break;

        case kMessageMiniScreen:
            switch ( globals()->gMiniScreenData.selectLine)
            {
                case kMessageMiniNext:
                    AdvanceCurrentLongMessage();
                    break;

                case kMessageMiniLast:
                    ReplayLastLongMessage();
                    break;

                case kMessageMiniPrevious:
                    PreviousCurrentLongMessage();
                    break;

                default:
                    break;
            }
            break;

        default:
            break;
    }
*/}

void MiniComputerExecute( long whichPage, long whichLine, long whichAdmiral)

{
    spaceObjectType *anObject, *anotherObject;
    long            l;

    switch ( whichPage)
    {
        case kMainMiniScreen:
            if ( whichAdmiral == globals()->gPlayerAdmiralNumber)
            {
                switch ( whichLine)
                {
                    case kMainMiniBuild:
                        MakeMiniScreenFromIndString( kBuildMiniScreen);
                        MiniComputerSetBuildStrings();
                        DrawMiniScreen();
                        ShowWholeMiniScreen();
                        break;

                    case kMainMiniSpecial:
                        MakeMiniScreenFromIndString( kSpecialMiniScreen);
                        DrawMiniScreen();
                        ShowWholeMiniScreen();
                        break;

                    case kMainMiniMessage:
                        MakeMiniScreenFromIndString( kMessageMiniScreen);
                        DrawMiniScreen();
                        ShowWholeMiniScreen();
                        break;

                    case kMainMiniStatus:
                        MakeMiniScreenFromIndString( kStatusMiniScreen);
                        MiniComputerSetStatusStrings();
                        DrawMiniScreen();
                        ShowWholeMiniScreen();
                        break;

                    default:
                        break;
                }
            }

            break;

        case kBuildMiniScreen:
            if ( globals()->keyMask & kComputerBuildMenu) return;
            if ( whichLine != kMiniScreenNoLineSelected)
            {
                if ( CountObjectsOfBaseType( -1, -1) <
                    (kMaxSpaceObject - kMaxShipBuffer))
                {
                    if (AdmiralScheduleBuild( whichAdmiral,
                        whichLine - kBuildScreenFirstTypeLine) == false)
                    {
                        if ( whichAdmiral == globals()->gPlayerAdmiralNumber)
                            mPlayBeepBad;
                    }
                } else
                {
                    if ( whichAdmiral == globals()->gPlayerAdmiralNumber)
                    {
                        SetStatusString( "\pMaximum number of ships built", true,
                            ORANGE);
                    }
                }
            }
            break;

        case kSpecialMiniScreen:
            if ( globals()->keyMask & kComputerSpecialMenu) return;
            switch ( whichLine)
            {
                case kSpecialMiniTransfer:
                    l = GetAdmiralConsiderObject( whichAdmiral);
                    anObject = GetAdmiralFlagship( whichAdmiral);
                    if ( anObject != nil)
                    {
                        if ( l != kNoShip)
                        {
                            anotherObject = gSpaceObjectData.get() + l;
                            if (( anotherObject->active != kObjectInUse) ||
                                ( !(anotherObject->attributes & kCanThink)) ||
                                ( anotherObject->attributes & kStaticDestination)
                                || ( anotherObject->owner != anObject->owner) ||
                                (!(anotherObject->attributes & kCanAcceptDestination))
                                || ( !(anotherObject->attributes & kCanBeDestination))
                                || ( anObject->active != kObjectInUse))
                            {
                                if ( whichAdmiral == globals()->gPlayerAdmiralNumber)
                                    mPlayBeepBad;
                            } else
                            {
                                ChangePlayerShipNumber( whichAdmiral, l);
                            }
/*
                            if (    ( anObject->active) &&
                                    ( anotherObject->active) &&
                                ( anotherObject->attributes &
                                        ( kCanAcceptDestination)) &&
                                    ( !(anotherObject->attributes & kStaticDestination))
                                )
                            {
                                ChangePlayerShipNumber( whichAdmiral, l);
                            } else
                            {
                                if ( whichAdmiral == globals()->gPlayerAdmiralNumber)
                                    mPlayBeepBad;
                            }
*/
                        } else
                        {
                            PlayerShipBodyExpire( anObject, false);
                        }
                    }
                    break;

                case kSpecialMiniFire1:
                    l = GetAdmiralConsiderObject( whichAdmiral);
                    if (( l != kNoShip))
                    {
                        anObject = gSpaceObjectData.get() + l;
                        if (( anObject->active) &&
                            (anObject->attributes & ( kCanAcceptDestination)))
                        {
                            anObject->keysDown |= kOneKey | kManualOverrideFlag;
                        }
                    }
                    break;

                case kSpecialMiniFire2:
                    l = GetAdmiralConsiderObject( whichAdmiral);
                    if (( l != kNoShip))
                    {
                        anObject = gSpaceObjectData.get() + l;
                        if (( anObject->active) &&
                            (anObject->attributes & ( kCanAcceptDestination)))
                        {
                            anObject->keysDown |= kTwoKey | kManualOverrideFlag;
                        }
                    }
                    break;

                case kSpecialMiniFireSpecial:
                    l = GetAdmiralConsiderObject( whichAdmiral);
                    if (( l != kNoShip))
                    {
                        anObject = gSpaceObjectData.get() + l;
                        if (( anObject->active) &&
                            (anObject->attributes & ( kCanAcceptDestination)))
                        {
                            anObject->keysDown |= kEnterKey | kManualOverrideFlag;
                        }
                    }
                    break;

                case kSpecialMiniHold:
                    l = GetAdmiralConsiderObject( whichAdmiral);
                    if (( l != kNoShip))
                    {
                        anObject = gSpaceObjectData.get() + l;
                        SetObjectLocationDestination( anObject, &(anObject->location));
                    }
                    break;

                case kSpecialMiniGoToMe:
                    l = GetAdmiralConsiderObject( whichAdmiral);
                    if (( l != kNoShip))
                    {
                        anObject = gSpaceObjectData.get() + l;
                        anotherObject = GetAdmiralFlagship( whichAdmiral);
                        SetObjectLocationDestination( anObject, &(anotherObject->location));
                    }
                    break;

                default:
                    break;
            }
            break;

        case kMessageMiniScreen:
            if ( globals()->keyMask & kComputerMessageMenu) return;
            if ( whichAdmiral == globals()->gPlayerAdmiralNumber)
            {
                switch ( whichLine)
                {
                    case kMessageMiniNext:
                        AdvanceCurrentLongMessage();
                        break;

                    case kMessageMiniLast:
                        ReplayLastLongMessage();
                        break;

                    case kMessageMiniPrevious:
                        PreviousCurrentLongMessage();
                        break;

                    default:
                        break;
                }
            }
            break;

        default:
            break;
    }
}

void MiniComputerDoCancel( void)

{
    switch ( globals()->gMiniScreenData.currentScreen)
    {
        case kBuildMiniScreen:
        case kSpecialMiniScreen:
        case kMessageMiniScreen:
        case kStatusMiniScreen:
            MakeMiniScreenFromIndString( kMainMiniScreen);
            DrawMiniScreen();
            ShowWholeMiniScreen();

            break;

        default:
            break;
    }
}

void MiniComputerSetBuildStrings( void) // sets the ship type strings for the build screen
// also sets up the values = base object num

{
    baseObjectType      *buildObject = nil;
    admiralType         *admiral = nil;
    destBalanceType     *buildAtObject = nil;
    miniScreenLineType  *line = nil;
    Str255              s;
    long                count, baseNum, lineNum, buildAtObjectNum;
    unsigned char       *namechar;
    short               namelen, linelen;
    Rect                mRect;

    mRect = Rect(kMiniScreenLeft, kMiniScreenTop + globals()->gInstrumentTop, kMiniScreenRight,
                kMiniScreenBottom + globals()->gInstrumentTop);

    globals()->gMiniScreenData.selectLine = kMiniScreenNoLineSelected;
    if ( globals()->gMiniScreenData.currentScreen == kBuildMiniScreen)
    {
        admiral = globals()->gAdmiralData.get() + globals()->gPlayerAdmiralNumber;
        line = globals()->gMiniScreenData.lineData.get() +
            kBuildScreenWhereNameLine;
        buildAtObjectNum =
            GetAdmiralBuildAtObject( globals()->gPlayerAdmiralNumber);
        line->value = buildAtObjectNum;

        if ( buildAtObjectNum >= 0)
        {
            buildAtObject = mGetDestObjectBalancePtr( buildAtObjectNum);
            mCopyBlankLineString( line, namechar, buildAtObject->name, namelen, linelen);

            line = globals()->gMiniScreenData.lineData.get() + kBuildScreenFirstTypeLine;
            lineNum = kBuildScreenFirstTypeLine;

            for ( count = 0; count < kMaxShipCanBuild; count++)
            {
                mGetBaseObjectFromClassRace( buildObject, baseNum, buildAtObject->canBuildType[count], admiral->race);
                line->value = baseNum;
                line->sourceData = buildObject;
                if ( buildObject != nil)
                {
                    GetIndString( s, kSpaceObjectNameResID, baseNum + 1);

                    mCopyBlankLineString( line, namechar, s, namelen, linelen);
                    if ( buildObject->price > mFixedToLong(admiral->cash))
                        line->selectable = selectDim;
                    else line->selectable = selectable;
                    if ( globals()->gMiniScreenData.selectLine == kMiniScreenNoLineSelected)
                    {
                        globals()->gMiniScreenData.selectLine = lineNum;
                        line->hiliteLeft = mRect.left;
                        line->hiliteRight = mRect.right;
                    }

                } else
                {
                    linelen = 1;
                    while ( linelen <= kMiniScreenCharWidth)
                    {
                        line->string[linelen] = kAnyCharSpace;
                        linelen++;
                    }
                    line->selectable = cannotSelect;
                    if ( globals()->gMiniScreenData.selectLine == (count + kBuildScreenFirstTypeLine))
                    {
                        line->hiliteLeft = line->hiliteRight = 0;
                        globals()->gMiniScreenData.selectLine++;
                    }
                    line->value = -1;
                }
                lineNum++;
                line++;
            }
            line = globals()->gMiniScreenData.lineData.get() + globals()->gMiniScreenData.selectLine;
            if ( line->selectable == cannotSelect)
                globals()->gMiniScreenData.selectLine =
                kMiniScreenNoLineSelected;
        } else
        {
            globals()->gMiniScreenData.selectLine = kMiniScreenNoLineSelected;

            line =
                globals()->gMiniScreenData.lineData.get() +
                kBuildScreenFirstTypeLine;
            for ( count = 0; count < kMaxShipCanBuild; count++)
            {
                linelen = 1;
                while ( linelen <= kMiniScreenCharWidth)
                {
                    line->string[linelen] = kAnyCharSpace;
                    linelen++;
                }
                line->selectable = cannotSelect;
                line->hiliteLeft = line->hiliteRight = 0;
                line++;
            }
        }
    }
}

// MiniComputerGetPriceOfCurrentSelection:
//  If the Build Menu is up, returns the price of the currently selected
//  ship, regardless of whether or not it is affordable.
//
//  If the selection is not legal, or the current Menu is not the Build Menu,
//  returns 0

long MiniComputerGetPriceOfCurrentSelection( void)
{
    miniScreenLineType  *line = nil;
    baseObjectType      *buildObject = nil;

    if (( globals()->gMiniScreenData.currentScreen != kBuildMiniScreen) ||
            ( globals()->gMiniScreenData.selectLine == kMiniScreenNoLineSelected))
        return (0);

        line = globals()->gMiniScreenData.lineData.get() +
            globals()->gMiniScreenData.selectLine;

        if ( line->value < 0) return( 0);

        buildObject = mGetBaseObjectPtr( line->value);

        if ( buildObject->price < 0) return( 0);

        return( mLongToFixed(buildObject->price));
}

void MiniComputerSetStatusStrings( void)
{
    // the strings must be in this format:
    //  type\number\player\negativevalue\falsestring\truestring\basestring\poststring
    //
    //  where type = 0...5
#define kNoStatusData               -1  // no status for this line
#define kPlainTextStatus            0
#define kTrueFalseCondition         1   // 0 = F, 1 = T, use condition not score
#define kIntegerValue               2   // interpret score as int
#define kSmallFixedValue            3   // interpret score as fixed
#define kIntegerMinusValue          4   // value - designated score
#define kSmallFixedMinusValue       5   // small fixed - designated score

    //  number = which score/condition #
    //
    //  player = which player score (if any); -1 = you, -2 = first not you
    //  ( 0 if you're player 1, 1 if you're player 0)
    //
    //  negative value = value to use for kIntegerMinusValue or kSmallFixedMinusValue
    //
    //  falsestring = string to use if false
    //
    //  truestring = string to use if true
    //
    //  basestring = first part of string
    //
    //  for example, the string 1\0\\0\0\N\Y\SHIP DESTROYED:
    //  would result in the status line SHIP DESTROYED, based on condition 0;
    //  if false, line reads SHIP DESTROYED: N, and if true SHIP DESTROYED: Y
    //
    //  example #2, string 2\1\0\10\\\Samples Left:
    //  would result in the status line "Samples Left: " + score 1 of player 0
    //  so if player 0's score 1 was 3, the line would read:
    //  Samples Left: 7
    //

    short               count, charNum, value;
    Str255              sourceString;
    miniScreenLineType  *line;

    if (globals()->gMissionStatusStrList.get() == nil) {
        for ( count = kStatusMiniScreenFirstLine; count < kMiniScreenCharHeight;
            count++)
        {
            line = globals()->gMiniScreenData.lineData.get() +
                count;
            line->statusType = kNoStatusData;
            line->value = -1;
            line->string[0] = 0;
        }
        return;
    }

    for ( count = kStatusMiniScreenFirstLine; count < kMiniScreenCharHeight;
        count++)
    {
        line = globals()->gMiniScreenData.lineData.get() +
            count;

        if (implicit_cast<size_t>(count - kStatusMiniScreenFirstLine) <
                globals()->gMissionStatusStrList->size()) {
            // we have some data for this line to interpret

            string_to_pstring(globals()->gMissionStatusStrList->at(count -
                            kStatusMiniScreenFirstLine), sourceString);

            charNum = 1;
            if ( sourceString[charNum] == '_')
            {
                line->underline = true;
                charNum++;
            }

            if ( sourceString[charNum] != '-') // - = abbreviated string, just plain
            {
                //////////////////////////////////////////////
                // get status type
                value = 0;
                while (( charNum <= sourceString[0]) && ( sourceString[charNum] !=
                    '\\'))
                {
                    value *= 10;
                    value += sourceString[charNum] - '0';
                    charNum++;
                }
                charNum++;

                if (( value >= 0) && ( value <= kMaxStatusTypeValue))
                    line->statusType = value;

                //////////////////////////////////////////////
                // get score/condition number
                value = 0;
                while (( charNum <= sourceString[0]) && ( sourceString[charNum] !=
                    '\\'))
                {
                    value *= 10;
                    value += sourceString[charNum] - '0';
                    charNum++;
                }
                charNum++;

                line->whichStatus = value;

                //////////////////////////////////////////////
                // get player number
                value = 0;
                while (( charNum <= sourceString[0]) && ( sourceString[charNum] !=
                    '\\'))
                {
                    value *= 10;
                    value += sourceString[charNum] - '0';
                    charNum++;
                }
                charNum++;

                line->statusPlayer = value;

                //////////////////////////////////////////////
                // get negative value
                value = 0;
                while (( charNum <= sourceString[0]) && ( sourceString[charNum] !=
                    '\\'))
                {
                    value *= 10;
                    value += sourceString[charNum] - '0';
                    charNum++;
                }
                charNum++;

                line->negativeValue = value;

                //////////////////////////////////////////////
                // get falseString
                line->statusFalse[0] = 0;
                while (( charNum <= sourceString[0]) && ( sourceString[charNum] !=
                    '\\'))
                {
                    line->statusFalse[0]++;
                    line->statusFalse[line->statusFalse[0]] = sourceString[charNum];
                    charNum++;
                }
                charNum++;

                //////////////////////////////////////////////
                // get trueString
                line->statusTrue[0] = 0;
                while (( charNum <= sourceString[0]) && ( sourceString[charNum] !=
                    '\\'))
                {
                    line->statusTrue[0]++;
                    line->statusTrue[line->statusTrue[0]] = sourceString[charNum];
                    charNum++;
                }
                charNum++;
            } else
            {
                charNum++;
                line->statusType = kPlainTextStatus;
            }

            //////////////////////////////////////////////
            // get statusString
            line->statusString[0] = 0;
            while (( charNum <= sourceString[0]) && ( sourceString[charNum] !=
                '\\'))
            {
                line->statusString[0]++;
                line->statusString[line->statusString[0]] = sourceString[charNum];
                charNum++;
            }
            charNum++;

            //////////////////////////////////////////////
            // get postString
            line->postString[0] = 0;
            while ( charNum <= sourceString[0])
            {
                line->postString[0]++;
                line->postString[line->postString[0]] = sourceString[charNum];
                charNum++;
            }

            line->value = MiniComputerGetStatusValue( count);
            MiniComputerMakeStatusString( count, line->string);
        } else
        {
            line->statusType = kNoStatusData;
            line->value = -1;
            line->string[0] = 0;
        }
    }
}

void MiniComputerMakeStatusString(long whichLine, unsigned char* destString) {
    miniScreenLineType  *line;
    Str255              tempString;

    line = globals()->gMiniScreenData.lineData.get() +
        whichLine;

    destString[0] = 0;

    if ( line->statusType != kNoStatusData)
        CopyPString( destString, line->statusString);
    else
    {
        destString[0] = 0;
        return;
    }

    switch ( line->statusType)
    {
        case kTrueFalseCondition:
            if ( line->value == 1)
            {
                ConcatenatePString( destString, line->statusTrue);
            } else
            {
                ConcatenatePString( destString, line->statusFalse);
            }
            break;

        case kIntegerValue:
        case kIntegerMinusValue:
            NumToString( line->value, tempString);
            ConcatenatePString( destString, tempString);
            break;

        case kSmallFixedValue:
        case kSmallFixedMinusValue:
            SmallFixedToString( line->value, tempString);
            ConcatenatePString( destString, tempString);
            break;
    }
    if ( line->statusType != kPlainTextStatus)
        ConcatenatePString( destString, line->postString);
}

long MiniComputerGetStatusValue( long whichLine)
{
    miniScreenLineType  *line;

    line = globals()->gMiniScreenData.lineData.get() +
        whichLine;

    if ( line->statusType == kNoStatusData)
        return( -1);

    switch ( line->statusType)
    {
        case kPlainTextStatus:
            return( 0);
            break;

        case kTrueFalseCondition:
            if ( GetScenarioConditionTrue( line->whichStatus))
            {
                return( 1);
            } else
            {
                return( 0);
            }
            break;

        case kIntegerValue:
        case kSmallFixedValue:
            return( GetAdmiralScore( GetRealAdmiralNumber( line->statusPlayer),
                line->whichStatus));
            break;

        case kIntegerMinusValue:
        case kSmallFixedMinusValue:
            return( line->negativeValue - GetAdmiralScore(
                GetRealAdmiralNumber( line->statusPlayer), line->whichStatus));
            break;

        default:
            return( 0);
            break;
    }
}

void MiniComputerHandleClick( Point where)

{
    Rect        mRect;
    long        lineNum, scrap, inLineButtonLine = -1, outLineButtonLine = -1;
    miniScreenLineType  *line;

    mSetDirectFont( kComputerFontNum);
    line = globals()->gMiniScreenData.lineData.get();
    scrap = 0;
    while ( scrap < kMiniScreenTrueLineNum)
    {
        if ( line->whichButton == kInLineButton) inLineButtonLine = scrap;
        else if ( line->whichButton == kOutLineButton) outLineButtonLine = scrap;
        scrap++;
        line++;
    }

    mRect = Rect(kButBoxLeft, kButBoxTop + globals()->gInstrumentTop, kButBoxRight,
                kButBoxBottom + globals()->gInstrumentTop);

    // if click is in button screen
    if (mRect.contains(where)) {
        lineNum = (( where.v - ( kButBoxTop + globals()->gInstrumentTop)) / gDirectText->height) + kMiniScreenCharHeight;
        globals()->gMiniScreenData.clickLine = lineNum;
        line = globals()->gMiniScreenData.lineData.get() + lineNum;
        if ( line->whichButton == kInLineButton)
        {
            if ( line->lineKind != buttonOnLineKind)
            {
                line->lineKind = buttonOnLineKind;
                DrawAndShowMiniScreenLine( lineNum);
                mPlayBeep3;
            }
            if ( outLineButtonLine >= 0)
            {
                line = globals()->gMiniScreenData.lineData.get() +
                    outLineButtonLine;
                if ( line->lineKind != buttonOffLineKind)
                {
                    line->lineKind = buttonOffLineKind;
                    DrawAndShowMiniScreenLine( outLineButtonLine);
                }
            }
        } else if ( line->whichButton == kOutLineButton)
        {
            if ( line->lineKind != buttonOnLineKind)
            {
                line->lineKind = buttonOnLineKind;
                DrawAndShowMiniScreenLine( lineNum);
                mPlayBeep3;
            }
            if ( inLineButtonLine >= 0)
            {
                line = globals()->gMiniScreenData.lineData.get() + inLineButtonLine;
                if ( line->lineKind != buttonOffLineKind)
                {
                    line->lineKind = buttonOffLineKind;
                    DrawAndShowMiniScreenLine( inLineButtonLine);
                }
            }
        }
    } else
    {

        // make sure both buttons are off
        if ( inLineButtonLine >= 0)
        {
            line = globals()->gMiniScreenData.lineData.get() + inLineButtonLine;
            if ( line->lineKind != buttonOffLineKind)
            {
                line->lineKind = buttonOffLineKind;
                DrawAndShowMiniScreenLine( inLineButtonLine);
            }
        }
        if ( outLineButtonLine >= 0)
        {
            line = globals()->gMiniScreenData.lineData.get() + outLineButtonLine;
            if ( line->lineKind != buttonOffLineKind)
            {
                line->lineKind = buttonOffLineKind;
                DrawAndShowMiniScreenLine( outLineButtonLine);
            }
        }

        mRect = Rect(kMiniScreenLeft, kMiniScreenTop + globals()->gInstrumentTop, kMiniScreenRight,
                kMiniScreenBottom + globals()->gInstrumentTop);

        // if click is in main menu screen
        if (mRect.contains(where)) {
            if ( globals()->gMiniScreenData.selectLine !=
                kMiniScreenNoLineSelected)
            {
                line = globals()->gMiniScreenData.lineData.get() +
                    globals()->gMiniScreenData.selectLine;
                line->hiliteLeft = line->hiliteRight = 0;
                DrawAndShowMiniScreenLine( globals()->gMiniScreenData.selectLine);
            }

            lineNum = mGetLineNumFromV( where.v);
            globals()->gMiniScreenData.clickLine = lineNum;
            line = globals()->gMiniScreenData.lineData.get() + lineNum;
            if (( line->selectable == selectable) || (line->selectable == selectDim))
            {
                globals()->gMiniScreenData.selectLine = lineNum;

                line = globals()->gMiniScreenData.lineData.get() +
                    globals()->gMiniScreenData.selectLine;
                line->hiliteLeft = mRect.left;
                line->hiliteRight = mRect.right;
                DrawAndShowMiniScreenLine( globals()->gMiniScreenData.selectLine);
            }
        } else globals()->gMiniScreenData.clickLine = kMiniScreenNoLineSelected;
    }
}

void MiniComputerHandleDoubleClick( Point where)

{
    Rect        mRect;
    long        lineNum, scrap, inLineButtonLine = -1, outLineButtonLine = -1;
    miniScreenLineType  *line;

    mSetDirectFont( kComputerFontNum);
    line = globals()->gMiniScreenData.lineData.get();
    scrap = 0;
    while ( scrap < kMiniScreenTrueLineNum)
    {
        if ( line->whichButton == kInLineButton) inLineButtonLine = scrap;
        else if ( line->whichButton == kOutLineButton) outLineButtonLine = scrap;
        scrap++;
        line++;
    }

    mRect = Rect(kButBoxLeft, kButBoxTop + globals()->gInstrumentTop, kButBoxRight,
                kButBoxBottom + globals()->gInstrumentTop);

    // if click is in button screen
    if (mRect.contains(where)) {
        lineNum = (( where.v - ( kButBoxTop + globals()->gInstrumentTop)) / gDirectText->height) + kMiniScreenCharHeight;
        line = globals()->gMiniScreenData.lineData.get() + lineNum;
        if ( line->whichButton == kInLineButton)
        {
            if ( line->lineKind != buttonOnLineKind)
            {
                line->lineKind = buttonOnLineKind;
                DrawAndShowMiniScreenLine( lineNum);
                mPlayBeep3;
            }
            if ( outLineButtonLine >= 0)
            {
                line = globals()->gMiniScreenData.lineData.get() + outLineButtonLine;
                if ( line->lineKind != buttonOffLineKind)
                {
                    line->lineKind = buttonOffLineKind;
                    DrawAndShowMiniScreenLine( outLineButtonLine);
                }
            }
        } else if ( line->whichButton == kOutLineButton)
        {
            if ( line->lineKind != buttonOnLineKind)
            {
                line->lineKind = buttonOnLineKind;
                DrawAndShowMiniScreenLine( lineNum);
                mPlayBeep3;
            }
            if ( inLineButtonLine >= 0)
            {
                line = globals()->gMiniScreenData.lineData.get() + inLineButtonLine;
                if ( line->lineKind != buttonOffLineKind)
                {
                    line->lineKind = buttonOffLineKind;
                    DrawAndShowMiniScreenLine( inLineButtonLine);
                }
            }
        }
    } else
    {

        // make sure both buttons are off
        if ( inLineButtonLine >= 0)
        {
            line = globals()->gMiniScreenData.lineData.get() + inLineButtonLine;
            if ( line->lineKind != buttonOffLineKind)
            {
                line->lineKind = buttonOffLineKind;
                DrawAndShowMiniScreenLine( inLineButtonLine);
            }
        }
        if ( outLineButtonLine >= 0)
        {
            line = globals()->gMiniScreenData.lineData.get() + outLineButtonLine;
            if ( line->lineKind != buttonOffLineKind)
            {
                line->lineKind = buttonOffLineKind;
                DrawAndShowMiniScreenLine( outLineButtonLine);
            }
        }

        mRect = Rect(kMiniScreenLeft, kMiniScreenTop + globals()->gInstrumentTop, kMiniScreenRight,
                kMiniScreenBottom + globals()->gInstrumentTop);

        // if click is in main menu screen
        if (mRect.contains(where)) {
            lineNum = mGetLineNumFromV( where.v);
            if ( lineNum == globals()->gMiniScreenData.selectLine)
            {
                mPlayBeep3;
                MiniComputerDoAccept();
            } else
            {
                if ( globals()->gMiniScreenData.selectLine !=
                    kMiniScreenNoLineSelected)
                {
                    line = globals()->gMiniScreenData.lineData.get() + globals()->gMiniScreenData.selectLine;
                    line->hiliteLeft = line->hiliteRight = 0;
                    DrawAndShowMiniScreenLine( globals()->gMiniScreenData.selectLine);
                }

                lineNum = mGetLineNumFromV( where.v);
                line = globals()->gMiniScreenData.lineData.get() + lineNum;
                if (( line->selectable == selectable) || (line->selectable == selectDim))
                {
                    globals()->gMiniScreenData.selectLine = lineNum;

                    line = globals()->gMiniScreenData.lineData.get() + globals()->gMiniScreenData.selectLine;
                    line->hiliteLeft = mRect.left;
                    line->hiliteRight = mRect.right;
                    DrawAndShowMiniScreenLine( globals()->gMiniScreenData.selectLine);
                }
            }
        }
    }
}

void MiniComputerHandleMouseUp( Point where)

{
    Rect        mRect;
    long        lineNum, scrap, inLineButtonLine = -1, outLineButtonLine = -1;
    miniScreenLineType  *line;

    mSetDirectFont( kComputerFontNum);
    line = globals()->gMiniScreenData.lineData.get();
    scrap = 0;
    while ( scrap < kMiniScreenTrueLineNum)
    {
        if ( line->whichButton == kInLineButton) inLineButtonLine = scrap;
        else if ( line->whichButton == kOutLineButton) outLineButtonLine = scrap;
        scrap++;
        line++;
    }

    mRect = Rect(kButBoxLeft, kButBoxTop + globals()->gInstrumentTop, kButBoxRight,
                kButBoxBottom + globals()->gInstrumentTop);

    // if click is in button screen
    if (mRect.contains(where)) {
        lineNum = (( where.v - ( kButBoxTop + globals()->gInstrumentTop)) / gDirectText->height) + kMiniScreenCharHeight;
        line = globals()->gMiniScreenData.lineData.get() + lineNum;
        if ( line->whichButton == kInLineButton)
        {
            if ( line->lineKind == buttonOnLineKind)
            {
                line->lineKind = buttonOffLineKind;
                DrawAndShowMiniScreenLine( lineNum);
                MiniComputerDoAccept();
            }
        } else if ( line->whichButton == kOutLineButton)
        {
            if ( line->lineKind == buttonOnLineKind)
            {
                line->lineKind = buttonOffLineKind;
                DrawAndShowMiniScreenLine( lineNum);
                MiniComputerDoCancel();
            }
        }
    }
}

void MiniComputerHandleMouseStillDown( Point where)

{
    Rect        mRect;
    long        lineNum, scrap, inLineButtonLine = -1, outLineButtonLine = -1;
    miniScreenLineType  *line;

    mSetDirectFont( kComputerFontNum);
    line = globals()->gMiniScreenData.lineData.get();
    scrap = 0;
    while ( scrap < kMiniScreenTrueLineNum)
    {
        if ( line->whichButton == kInLineButton) inLineButtonLine = scrap;
        else if ( line->whichButton == kOutLineButton) outLineButtonLine = scrap;
        scrap++;
        line++;
    }

    mRect = Rect(kButBoxLeft, kButBoxTop + globals()->gInstrumentTop, kButBoxRight,
                kButBoxBottom + globals()->gInstrumentTop);

    // if click is in button screen
    if (mRect.contains(where)) {
        lineNum = (( where.v - ( kButBoxTop + globals()->gInstrumentTop)) / gDirectText->height) + kMiniScreenCharHeight;
        line = globals()->gMiniScreenData.lineData.get() + lineNum;
        if (( line->whichButton == kInLineButton) &&
            ( lineNum == globals()->gMiniScreenData.clickLine))
        {
            if ( line->lineKind != buttonOnLineKind)
            {
                line->lineKind = buttonOnLineKind;
                DrawAndShowMiniScreenLine( lineNum);
            }
        } else if (( line->whichButton == kOutLineButton) &&
            ( lineNum == globals()->gMiniScreenData.clickLine))
        {
            if ( line->lineKind != buttonOnLineKind)
            {
                line->lineKind = buttonOnLineKind;
                DrawAndShowMiniScreenLine( lineNum);
            }
        } else ( lineNum = -1);
    } else lineNum = -1;

    if ( lineNum == -1)
    {
        line = globals()->gMiniScreenData.lineData.get() + inLineButtonLine;
        if ( line->lineKind == buttonOnLineKind)
        {
            line->lineKind = buttonOffLineKind;
            DrawAndShowMiniScreenLine( inLineButtonLine);
        }
        line = globals()->gMiniScreenData.lineData.get() + outLineButtonLine;
        if ( line->lineKind == buttonOnLineKind)
        {
            line->lineKind = buttonOffLineKind;
            DrawAndShowMiniScreenLine( outLineButtonLine);
        }
    }
}

// for ambrosia tutorial, a horrific hack
void MiniComputer_SetScreenAndLineHack( long whichScreen, long whichLine)
{
    Point   w;

    switch ( whichScreen)
    {
        case kBuildMiniScreen:
            MakeMiniScreenFromIndString( kBuildMiniScreen);
            MiniComputerSetBuildStrings();
            DrawMiniScreen();
            ShowWholeMiniScreen();
            break;

        case kSpecialMiniScreen:
            MakeMiniScreenFromIndString( kSpecialMiniScreen);
            DrawMiniScreen();
            ShowWholeMiniScreen();
            break;

        case kMessageMiniScreen:
            MakeMiniScreenFromIndString( kMessageMiniScreen);
            DrawMiniScreen();
            ShowWholeMiniScreen();
            break;

        case kStatusMiniScreen:
            MakeMiniScreenFromIndString( kStatusMiniScreen);
            MiniComputerSetStatusStrings();
            DrawMiniScreen();
            ShowWholeMiniScreen();
            break;

        default:
            MakeMiniScreenFromIndString( kMainMiniScreen);
            DrawMiniScreen();
            ShowWholeMiniScreen();
            break;
    }

    mSetDirectFont( kComputerFontNum);
    w.v = (whichLine * gDirectText->height) + ( kMiniScreenTop +
                    globals()->gInstrumentTop);
    w.h = kMiniScreenLeft + 5;
    MiniComputerHandleClick( w);    // what an atrocious hack! oh well
}

}  // namespace antares
