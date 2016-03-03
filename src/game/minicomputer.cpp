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

#include "game/minicomputer.hpp"

#include <algorithm>
#include <sfz/sfz.hpp>

#include "config/keys.hpp"
#include "data/string-list.hpp"
#include "drawing/color.hpp"
#include "drawing/pix-table.hpp"
#include "drawing/shapes.hpp"
#include "drawing/sprite-handling.hpp"
#include "drawing/text.hpp"
#include "game/admiral.hpp"
#include "game/globals.hpp"
#include "game/instruments.hpp"
#include "game/messages.hpp"
#include "game/player-ship.hpp"
#include "game/scenario-maker.hpp"
#include "game/space-object.hpp"
#include "game/starfield.hpp"
#include "lang/defines.hpp"
#include "math/fixed.hpp"
#include "sound/fx.hpp"
#include "video/driver.hpp"

using sfz::Bytes;
using sfz::Rune;
using sfz::String;
using sfz::StringSlice;
using sfz::bin;
using sfz::range;
using sfz::string_to_int;
using std::max;

namespace antares {

namespace {

const int32_t kMiniScreenCharWidth = 25;

const int32_t kMiniScreenLeft       = 12;
const int32_t kMiniScreenTop        = 320;
const int32_t kMiniScreenRight      = 121;
const int32_t kMiniScreenBottom     = 440;
const int32_t kMiniScreenWidth      = kMiniScreenRight - kMiniScreenLeft;

const int32_t kMiniScreenLeftBuffer = 3;

const int32_t kMiniScreenCharHeight     = 10;  // height of the screen in characters
const int32_t kMiniScreenButtonNum      = 2;
const int32_t kMiniScreenTrueLineNum    = kMiniScreenCharHeight + kMiniScreenButtonNum;

const int32_t kButBoxLeft           = 16;
const int32_t kButBoxTop            = 450;
const int32_t kButBoxRight          = 114;
const int32_t kButBoxBottom         = 475;

const int32_t kMiniScreenNoLineSelected = -1;

const int16_t kMiniScreenStringID   = 3000;
const int16_t kMiniDataStringID     = 3001;

const uint8_t kMiniScreenColor      = GREEN;
const uint8_t kMiniButColor         = AQUA;

const Rune kMiniScreenSpecChar      = '\\';
const Rune kEndLineChar             = 'x';
const Rune kUnderlineEndLineChar    = 'u';
const Rune kIntoButtonChar          = 'I';
const Rune kOutOfButtonChar         = 'O';
const Rune kSelectableLineChar      = 'S';

const int32_t kNoLineButton         = -1;
const int32_t kInLineButton         = kCompAcceptKeyNum;
const int32_t kOutLineButton        = kCompCancelKeyNum;

static ANTARES_GLOBAL StringList* mini_data_strings;
static ANTARES_GLOBAL std::unique_ptr<StringList> gMissionStatusStrList;

enum {
    kMainMiniScreen     = 1,
    kBuildMiniScreen    = 2,
    kSpecialMiniScreen  = 3,
    kMessageMiniScreen  = 4,
    kStatusMiniScreen   = 5,
};

enum {
    kMainMiniBuild      = 1,
    kMainMiniSpecial    = 2,
    kMainMiniMessage    = 3,
    kMainMiniStatus     = 4,
};

const int32_t kBuildScreenFirstTypeLine = 2;
const int32_t kBuildScreenWhereNameLine = 1;

enum {
    kSpecialMiniTransfer        = 1,
    kSpecialMiniHold            = 2,
    kSpecialMiniGoToMe          = 3,
    kSpecialMiniFire1           = 4,
    kSpecialMiniFire2           = 5,
    kSpecialMiniFireSpecial     = 6,
};

enum {
    kMessageMiniNext        = 1,
    kMessageMiniPrevious    = 2,
    kMessageMiniLast        = 3,
};

const int32_t kStatusMiniScreenFirstLine = 1;

enum {
    kNoStatusData           = -1,  // no status for this line
    kPlainTextStatus        = 0,
    kTrueFalseCondition     = 1,   // 0 = F, 1 = T, use condition not score
    kIntegerValue           = 2,   // interpret score as int
    kSmallFixedValue        = 3,   // interpret score as fixed
    kIntegerMinusValue      = 4,   // value - designated score
    kSmallFixedMinusValue   = 5,   // small fixed - designated score
    kMaxStatusTypeValue     = kSmallFixedMinusValue,
};

const int32_t kMiniObjectDataNum    = 2;
const int32_t kMiniSelectObjectNum  = 0;
const int32_t kMiniSelectTop        = 180;

const int32_t kMiniIconHeight       = 22;
const int32_t kMiniIconWidth        = 22;
const int32_t kMiniIconLeft         = (kMiniScreenLeft + 2);

const int32_t kMiniHealthLeft       = (kMiniIconLeft + kMiniIconWidth + 4);
const int32_t kMiniBarWidth         = 11;
const int32_t kMiniBarHeight        = 18;

const int32_t kMiniEnergyLeft       = (kMiniHealthLeft + kMiniBarWidth + 2);

const int32_t kMiniRightColumnLeft  = 57;
const int32_t kMiniWeapon1LineNum   = 2;
const int32_t kMiniWeapon2LineNum   = 3;
const int32_t kMiniWeapon3LineNum   = 1;
const int32_t kMiniNameLineNum      = 1;

const int32_t kMiniDestLineNum      = 4;

const int32_t kMiniTargetObjectNum  = 1;
const int32_t kMiniTargetTop        = 252;

const int32_t kMiniAmmoTop          = 161;
const int32_t kMiniAmmoBottom       = 170;
const int32_t kMiniAmmoSingleWidth  = 21;
const int32_t kMiniAmmoLeftOne      = 27;
const int32_t kMiniAmmoLeftTwo      = 64;
const int32_t kMiniAmmoLeftSpecial  = 100;
const int32_t kMiniAmmoTextHBuffer  = 2;

inline void mPlayBeep3() {
    PlayVolumeSound(kComputerBeep3, kMediumVolume, kMediumPersistence, kLowPrioritySound);
}

inline void mPlayBeepBad() {
    PlayVolumeSound(kWarningTone, kMediumVolume, kMediumPersistence, kLowPrioritySound);
}

const int32_t kMaxShipBuffer = 40;

void pad_to(String& s, size_t width) {
    if (s.size() < width) {
        String result;
        result.append((width - s.size()) / 2, ' ');
        result.append(s);
        result.append((1 + width - s.size()) / 2, ' ');
        swap(result, s);
    }
}

const int32_t MiniIconMacLineTop() {
    return computer_font->height * 2;
}

Rect mini_screen_line_bounds(int32_t mtop, int32_t mlinenum, int32_t mleft, int32_t mright) {
    Rect mbounds;
    mbounds.left = kMiniScreenLeft + mleft;
    mbounds.top = mtop + mlinenum * computer_font->height;
    mbounds.right = kMiniScreenLeft + mright;
    mbounds.bottom = mbounds.top + computer_font->height;
    return mbounds;
}

inline int32_t mGetLineNumFromV(int32_t mV) {
    return (((mV) - (kMiniScreenTop + instrument_top())) / computer_font->height);
}

inline void mCopyBlankLineString(miniScreenLineType* mline, StringSlice mstring) {
    mline->string.assign(mstring);
    if (mline->string.size() > kMiniScreenCharWidth) {
        mline->string.resize(kMiniScreenCharWidth);
    }
}

}  // namespace

static void draw_mini_ship_data(
        Handle<SpaceObject> obj, uint8_t header_color, int16_t screen_top, StringSlice label);

void MiniComputerSetStatusStrings( void);
int32_t MiniComputerGetStatusValue( int32_t);
void MiniComputerMakeStatusString(int32_t which_line, String& string);

void MiniScreenInit() {
    globals()->gMiniScreenData.selectLine = kMiniScreenNoLineSelected;
    globals()->gMiniScreenData.currentScreen = kMainMiniScreen;
    globals()->gMiniScreenData.clickLine = kMiniScreenNoLineSelected;

    globals()->gMiniScreenData.lineData.reset(new miniScreenLineType[kMiniScreenTrueLineNum]);

    ClearMiniScreenLines();
    ClearMiniObjectData();

    mini_data_strings = new StringList(kMiniDataStringID);
}

void MiniScreenCleanup() {
    globals()->gMiniScreenData.lineData.reset();
    // globals()->gMiniScreenData.objectData.reset();
}

#pragma mark -

void SetMiniScreenStatusStrList(int16_t strID) {
    DisposeMiniScreenStatusStrList();
    if (strID > 0) {
        gMissionStatusStrList.reset(new StringList(strID));
    }
}

void DisposeMiniScreenStatusStrList( void) {
    gMissionStatusStrList.reset();
}

void ClearMiniScreenLines() {
    miniScreenLineType* c = globals()->gMiniScreenData.lineData.get();
    for (int32_t b = 0; b < kMiniScreenTrueLineNum; b++) {
        c->string.clear();
        c->hiliteLeft = c->hiliteRight = 0;
        c->whichButton = kNoLineButton;
        c->selectable = cannotSelect;
        c->underline = false;
        c->lineKind = plainLineKind;
        c->sourceData = BaseObject::none();
        c++;
    }
}

void ClearMiniObjectData() {
}

void draw_minicomputer_lines() {
    Rect mini_rect = Rect(kMiniScreenLeft, kMiniScreenTop, kMiniScreenRight, kMiniScreenBottom);
    mini_rect.offset(0, instrument_top());
    Rect but_box_rect = Rect(kButBoxLeft, kButBoxTop, kButBoxRight, kButBoxBottom);
    but_box_rect.offset(0, instrument_top());

    {
        Rects rects;
        rects.fill(mini_rect, GetRGBTranslateColorShade(kMiniScreenColor, DARKEST));
        for (int32_t count = 0; count < kMiniScreenCharHeight; count++) {
            auto c = &globals()->gMiniScreenData.lineData[count];
            int32_t line_offset = count * computer_font->height;

            if (c->underline) {
                const RgbColor color = GetRGBTranslateColorShade(kMiniScreenColor, MEDIUM);
                int32_t y = mini_rect.top + line_offset + computer_font->ascent;
                rects.fill({mini_rect.left, y, mini_rect.right - 1, y + 1}, color);
            }

            if (c->hiliteLeft < c->hiliteRight) {
                Rect hilite_rect;
                hilite_rect.left = c->hiliteLeft;
                hilite_rect.top = mini_rect.top + line_offset;
                hilite_rect.right = c->hiliteRight;
                hilite_rect.bottom = hilite_rect.top + computer_font->height;
                draw_shaded_rect(rects, hilite_rect, kMiniScreenColor, DARK, MEDIUM, DARKER);
            }
        }

        rects.fill(but_box_rect, GetRGBTranslateColorShade(kMiniButColor, DARKEST));
        for (int32_t count = 0; count < kMiniScreenButtonNum; count++) {
            auto c = &globals()->gMiniScreenData.lineData[count + kMiniScreenCharHeight];
            Rect hilite_rect;
            hilite_rect.left = c->hiliteLeft - 2;
            hilite_rect.top = but_box_rect.top + (count * computer_font->height);
            hilite_rect.right = c->hiliteRight + 2;
            hilite_rect.bottom = hilite_rect.top + computer_font->height;
            switch (c->lineKind) {
                case plainLineKind:
                    break;

                case buttonOffLineKind:
                    draw_shaded_rect(rects, hilite_rect, kMiniButColor, MEDIUM, LIGHT, DARK);
                    break;

                case buttonOnLineKind:
                    hilite_rect.right = but_box_rect.right;
                    draw_shaded_rect(rects, hilite_rect, kMiniButColor, LIGHT, VERY_LIGHT, MEDIUM);
                    break;
            }
        }
    }

    {
        Quads quads(computer_font->texture);
        for (int32_t count = 0; count < kMiniScreenCharHeight; count++) {
            auto c = &globals()->gMiniScreenData.lineData[count];
            Point origin = mini_rect.origin();
            origin.offset(kMiniScreenLeftBuffer, (count * computer_font->height) + computer_font->ascent);

            RgbColor textcolor = GetRGBTranslateColorShade(kMiniScreenColor, VERY_LIGHT);
            if (c->selectable == selectDim) {
                if (c->hiliteLeft < c->hiliteRight) {
                    textcolor = GetRGBTranslateColorShade(kMiniScreenColor, VERY_DARK);
                } else {
                    textcolor = GetRGBTranslateColorShade(kMiniScreenColor, MEDIUM);
                }
            }
            computer_font->draw(quads, origin, c->string, textcolor);
        }

        for (int32_t count = 0; count < kMiniScreenButtonNum; count++) {
            auto c = &globals()->gMiniScreenData.lineData[count + kMiniScreenCharHeight];
            Point origin = but_box_rect.origin();
            origin.offset(kMiniScreenLeftBuffer, (count * computer_font->height) + computer_font->ascent);

            RgbColor textcolor = GetRGBTranslateColorShade(kMiniButColor, VERY_LIGHT);
            if (c->lineKind == buttonOnLineKind) {
                textcolor = RgbColor::kBlack;
            }

            computer_font->draw(quads, origin, c->string, textcolor);
        }
    }
}

void draw_mini_screen() {
    switch (globals()->gMiniScreenData.currentScreen) {
        case kMainMiniScreen:
        case kBuildMiniScreen:
        case kSpecialMiniScreen:
        case kMessageMiniScreen:
        case kStatusMiniScreen:
            draw_minicomputer_lines();
            break;
    }
    draw_mini_ship_data(
            g.admiral->control(), YELLOW, kMiniSelectTop + instrument_top(),
            mini_data_strings->at(0));
    draw_mini_ship_data(
            g.admiral->target(), SKY_BLUE, kMiniTargetTop + instrument_top(),
            mini_data_strings->at(1));
}

void MakeMiniScreenFromIndString(int16_t whichString) {
    Rect mRect(kMiniScreenLeft, kMiniScreenTop, kMiniScreenRight, kMiniScreenBottom);
    mRect.offset(0, instrument_top());

    ClearMiniScreenLines();
    globals()->gMiniScreenData.currentScreen = whichString;
    globals()->gMiniScreenData.selectLine = kMiniScreenNoLineSelected;

    StringList string_list(kMiniScreenStringID);
    StringSlice string = string_list.at(whichString - 1);

    miniScreenLineType* const line_begin = globals()->gMiniScreenData.lineData.get();
    miniScreenLineType* const line_switch = line_begin + kMiniScreenCharHeight;
    miniScreenLineType* const line_end = line_begin + kMiniScreenTrueLineNum;
    miniScreenLineType* line = line_begin;

    bool escape = false;
    for (Rune r: string) {
        if (escape) {
            escape = false;
            switch (r) {
              case kUnderlineEndLineChar:
                line->underline = true;
                // fall through.
              case kEndLineChar:
                ++line;
                if (line == line_end) {
                    return;
                } else if (line == line_switch) {
                    mRect = Rect(kButBoxLeft, kButBoxTop, kButBoxRight, kButBoxBottom);
                    mRect.offset(0, instrument_top());
                }
                break;

              case kSelectableLineChar:
                line->selectable = selectable;
                if (globals()->gMiniScreenData.selectLine == kMiniScreenNoLineSelected) {
                    globals()->gMiniScreenData.selectLine = line - line_begin;
                    line->hiliteLeft = mRect.left;
                    line->hiliteRight = mRect.right;
                }
                break;

              case kIntoButtonChar:
                {
                    line->lineKind = buttonOffLineKind;
                    line->whichButton = kInLineButton;
                    line->hiliteLeft
                        = mRect.left + kMiniScreenLeftBuffer
                        + computer_font->logicalWidth * line->string.size();

                    sfz::String key_name;
                    GetKeyNumName(Preferences::preferences()->key(kCompAcceptKeyNum), &key_name);
                    pad_to(key_name, kKeyNameLength);
                    line->string.append(key_name);

                    line->hiliteRight
                        = mRect.left + kMiniScreenLeftBuffer
                        + computer_font->logicalWidth * line->string.size() - 1;
                }
                break;

              case kOutOfButtonChar:
                {
                    line->lineKind = buttonOffLineKind;
                    line->whichButton = kOutLineButton;
                    line->hiliteLeft
                        = mRect.left + kMiniScreenLeftBuffer
                        + computer_font->logicalWidth * line->string.size();

                    sfz::String key_name;
                    GetKeyNumName(
                            Preferences::preferences()->key(kCompCancelKeyNum), &key_name);
                    pad_to(key_name, kKeyNameLength);
                    line->string.append(key_name);

                    line->hiliteRight
                        = mRect.left + kMiniScreenLeftBuffer
                        + computer_font->logicalWidth * line->string.size() - 1;
                }
                break;

              case kMiniScreenSpecChar:
                line->string.append(1, kMiniScreenSpecChar);
                break;
            }
        } else if (r == kMiniScreenSpecChar) {
            escape = true;
        } else {
            line->string.append(1, r);
        }

        while (line->string.size() > kMiniScreenCharWidth) {
            String excess(line->string.slice(kMiniScreenCharWidth));
            line->string.resize(kMiniScreenCharWidth);

            ++line;
            if (line == line_end) {
                return;
            } else if (line == line_switch) {
                mRect = Rect(kButBoxLeft, kButBoxTop, kButBoxRight, kButBoxBottom);
                mRect.offset(0, instrument_top());
            }
            line->string.assign(excess);
        }
    }
}

static void minicomputer_handle_action(int32_t button, bool key_down, void (*action)()) {
    // find out which line, if any, contains this button
    for (size_t i: range(kMiniScreenTrueLineNum)) {
        miniScreenLineType& line = globals()->gMiniScreenData.lineData[i];
        if (line.whichButton != button) {
            continue;
        }
        // hilite/unhilite this button
        if (key_down) {
            if (line.lineKind != buttonOnLineKind) {
                line.lineKind = buttonOnLineKind;
                mPlayBeep3();
            }
        } else {
            if (line.lineKind != buttonOffLineKind) {
                line.lineKind = buttonOffLineKind;
                if (action) {
                    action();
                }
            }
        }
    }
}

static void minicomputer_handle_move(int direction) {
    if (globals()->gMiniScreenData.selectLine == kMiniScreenNoLineSelected) {
        return;
    }
    miniScreenLineType* line = globals()->gMiniScreenData.lineData.get() + globals()->gMiniScreenData.selectLine;
    line->hiliteLeft = line->hiliteRight = 0;
    do {
        line += direction;
        globals()->gMiniScreenData.selectLine += direction;
        if (globals()->gMiniScreenData.selectLine < 0) {
            globals()->gMiniScreenData.selectLine += kMiniScreenCharHeight;
            line += kMiniScreenCharHeight;
        } else if (globals()->gMiniScreenData.selectLine >= kMiniScreenCharHeight) {
            globals()->gMiniScreenData.selectLine -= kMiniScreenCharHeight;
            line -= kMiniScreenCharHeight;
        }
    } while (line->selectable == cannotSelect);

    line->hiliteLeft = kMiniScreenLeft;
    line->hiliteRight = kMiniScreenRight;
}

void minicomputer_handle_keys(uint32_t new_keys, uint32_t old_keys, bool cancel) {
    if ((new_keys ^ old_keys) & kCompAcceptKey) {
        minicomputer_handle_action(
                kInLineButton, new_keys & kCompAcceptKey, MiniComputerDoAccept);
    }

    if ((new_keys ^ old_keys) & kCompCancelKey) {
        minicomputer_handle_action(
                kOutLineButton, new_keys & kCompCancelKey, MiniComputerDoCancel);
    }

    if ((new_keys & ~old_keys) & kCompUpKey) {
        minicomputer_handle_move(-1);
    }

    if ((new_keys & ~old_keys) & kCompDownKey) {
        minicomputer_handle_move(+1);
    }
}

void minicomputer_cancel() {
    minicomputer_handle_action(kInLineButton, false, NULL);
    minicomputer_handle_action(kOutLineButton, false, NULL);
}

static void update_build_screen_lines() {
    Rect mRect = Rect(
            kMiniScreenLeft, kMiniScreenTop + instrument_top(), kMiniScreenRight,
            kMiniScreenBottom + instrument_top());
    auto& mini = globals()->gMiniScreenData;

    const auto& admiral = g.admiral;
    miniScreenLineType* line = &mini.lineData[kBuildScreenWhereNameLine];
    if (line->value != GetAdmiralBuildAtObject(admiral).number()) {
        if (mini.selectLine != kMiniScreenNoLineSelected) {
            line = &mini.lineData[mini.selectLine];
            line->hiliteLeft = line->hiliteRight = 0;
            mini.selectLine = kMiniScreenNoLineSelected;
        }
        MiniComputerSetBuildStrings();
    } else if (GetAdmiralBuildAtObject(admiral).get()) {
        line = mini.lineData.get() + kBuildScreenFirstTypeLine;
        int32_t lineNum = kBuildScreenFirstTypeLine;

        for (int32_t count = 0; count < kMaxShipCanBuild; count++) {
            auto buildObject = line->sourceData;
            if (buildObject.get()) {
                if (buildObject->price > mFixedToLong(admiral->cash())) {
                    if (line->selectable != selectDim) {
                        line->selectable = selectDim;
                    }
                } else {
                    if (line->selectable != selectable) {
                        if (mini.selectLine == kMiniScreenNoLineSelected) {
                            mini.selectLine = lineNum;
                            line->hiliteLeft = mRect.left;
                            line->hiliteRight = mRect.right;
                        }
                        line->selectable = selectable;
                    }
                }
            }
            line++;
            lineNum++;
        }
    }
}

static void update_status_screen_lines() {
    auto& mini = globals()->gMiniScreenData;
    for (int32_t count = kStatusMiniScreenFirstLine; count < kMiniScreenCharHeight; count++) {
        miniScreenLineType* line = &mini.lineData[count];
        int32_t lineNum = MiniComputerGetStatusValue( count);
        if (line->value != lineNum) {
            line->value = lineNum;
            MiniComputerMakeStatusString(count, line->string);
        }
    }
}

// only for updating volitile lines--doesn't draw whole screen!
void UpdateMiniScreenLines() {
    switch (globals()->gMiniScreenData.currentScreen) {
        case kBuildMiniScreen:
            update_build_screen_lines();
            break;

        case kStatusMiniScreen:
            update_status_screen_lines();
            break;
    }
}

static void draw_player_ammo_in_rect(int32_t value, int8_t hue, const Rect& rect) {
    if (value >= 0) {
        const RgbColor text_color = GetRGBTranslateColorShade(hue, VERY_LIGHT);
        const char digits[] = {
            char(((value % 1000) / 100) + '0'),
            char(((value % 100) / 10) + '0'),
            char((value % 10) + '0'),
            '\0',
        };
        Point origin(rect.left + kMiniAmmoTextHBuffer, rect.bottom - 1);
        computer_font->draw(origin, digits, text_color);
    }
}

void draw_player_ammo(int32_t ammo_one, int32_t ammo_two, int32_t ammo_special) {
    Rect clip(0, kMiniAmmoTop, kMiniAmmoSingleWidth, kMiniAmmoBottom);
    clip.offset(0, instrument_top());

    clip.offset(kMiniAmmoLeftOne - clip.left, 0);
    draw_player_ammo_in_rect(ammo_one, RED, clip);
    clip.offset(kMiniAmmoLeftTwo - clip.left, 0);
    draw_player_ammo_in_rect(ammo_two, PALE_GREEN, clip);
    clip.offset(kMiniAmmoLeftSpecial - clip.left, 0);
    draw_player_ammo_in_rect(ammo_special, ORANGE, clip);
}

static void draw_mini_ship_data(
        Handle<SpaceObject> obj, uint8_t header_color, int16_t screen_top, StringSlice label) {
    {
        // "CONTROL" or "TARGET" label.
        Rect bar = mini_screen_line_bounds(screen_top, 0, 0, kMiniScreenWidth);
        draw_shaded_rect(Rects(), bar, header_color, LIGHT, VERY_LIGHT, MEDIUM);
        computer_font->draw(
                Point(bar.left + kMiniScreenLeftBuffer, bar.top + computer_font->ascent),
                label, RgbColor::kBlack);
    }

    // Icon
    Rect icon_rect = {
        {kMiniIconLeft, screen_top + MiniIconMacLineTop()},
        {kMiniIconWidth, kMiniIconHeight},
    };
    if (!obj.get()) {
        draw_vbracket(Rects(), icon_rect, GetRGBTranslateColorShade(PALE_GREEN, MEDIUM));
        return;
    }

    {
        // Object name.
        if (obj->base.get()) {
            Rect lRect = mini_screen_line_bounds(screen_top, kMiniNameLineNum, 0, kMiniScreenWidth);
            computer_font->draw(
                    Point(lRect.left + kMiniScreenLeftBuffer, lRect.top + computer_font->ascent),
                    obj->short_name(), GetRGBTranslateColorShade(PALE_GREEN, VERY_LIGHT));
        }
    }

    if ((obj->base.get()) && (obj->pixResID >= 0)) {
        // Icon
        NatePixTable* pixTable = GetPixTable(obj->pixResID);

        if (pixTable != NULL) {
            int16_t whichShape;
            if (obj->attributes & kIsSelfAnimated) {
                whichShape = more_evil_fixed_to_long(obj->base->frame.animation.firstShape);
            } else {
                whichShape = 0;
            }

            // get the picture data
            const NatePixTable::Frame& frame = pixTable->at(whichShape);

            Rect rect(0, 0, frame.width(), frame.height());
            int32_t max_dimension = max(frame.width(), frame.height());
            if (max_dimension > kMiniIconHeight) {
                rect.right = (rect.right * (kMiniIconHeight - 4)) / max_dimension;
                rect.bottom = (rect.bottom * (kMiniIconHeight - 4)) / max_dimension;
            }
            rect.center_in(icon_rect);

            frame.texture().draw(rect);
        }
    }
    draw_vbracket(Rects(), icon_rect, GetRGBTranslateColorShade(PALE_GREEN, MEDIUM));

    {
        if ((obj->max_health() > 0) && (obj->_health > 0)) {
            Rects rects;
            Rect dRect = {
                Point(kMiniHealthLeft, screen_top + MiniIconMacLineTop()),
                Size(kMiniBarWidth, kMiniIconHeight)
            };

            uint32_t tlong = obj->_health * kMiniBarHeight;
            tlong /= obj->max_health();

            Rect lRect;
            lRect.left = dRect.left + 2;
            lRect.top = dRect.top + 2;
            lRect.right = dRect.right - 2;
            lRect.bottom = dRect.bottom - 2 - tlong;
            rects.fill(lRect, GetRGBTranslateColorShade(SKY_BLUE, DARK));

            lRect.top = dRect.bottom - 2 - tlong;
            lRect.bottom = dRect.bottom - 2;
            rects.fill(lRect, GetRGBTranslateColorShade(SKY_BLUE, LIGHT));

            draw_vbracket(rects, dRect, GetRGBTranslateColorShade(SKY_BLUE, MEDIUM));
        }
    }

    {
        if ((obj->max_energy() > 0) && (obj->_energy > 0)) {
            Rects rects;
            Rect dRect = {
                Point(kMiniEnergyLeft, screen_top + MiniIconMacLineTop()),
                Size(kMiniBarWidth, kMiniIconHeight)
            };

            uint32_t tlong = obj->_energy * kMiniBarHeight;
            tlong /= obj->max_energy();

            Rect lRect;
            lRect.left = dRect.left + 2;
            lRect.top = dRect.top + 2;
            lRect.right = dRect.right - 2;
            lRect.bottom = dRect.bottom - 2 - tlong;
            rects.fill(lRect, GetRGBTranslateColorShade(YELLOW, DARK));

            lRect.top = dRect.bottom - 2 - tlong;
            lRect.bottom = dRect.bottom - 2;
            rects.fill(lRect, GetRGBTranslateColorShade(YELLOW, LIGHT));

            draw_vbracket(rects, dRect, GetRGBTranslateColorShade(YELLOW, MEDIUM));
        }
    }

    {
        // Weapons
        RgbColor color = GetRGBTranslateColorShade(PALE_GREEN, VERY_LIGHT);

        if (obj->beam.base.get()) {
            Rect lRect = mini_screen_line_bounds(
                    screen_top, kMiniWeapon1LineNum, kMiniRightColumnLeft, kMiniScreenWidth);
            computer_font->draw(
                    Point(lRect.left, lRect.top + computer_font->ascent),
                    get_object_short_name(obj->beam.base), color);
        }

        if (obj->pulse.base.get()) {
            Rect lRect = mini_screen_line_bounds(
                    screen_top, kMiniWeapon2LineNum, kMiniRightColumnLeft, kMiniScreenWidth);
            computer_font->draw(
                    Point(lRect.left, lRect.top + computer_font->ascent),
                    get_object_short_name(obj->pulse.base), color);
        }

        // Don't show special weapons of destination objects.
        if (!(obj->attributes & kIsDestination)) {
            if (obj->special.base.get()) {
                Rect lRect = mini_screen_line_bounds(
                        screen_top, kMiniWeapon3LineNum, kMiniRightColumnLeft, kMiniScreenWidth);
                computer_font->draw(
                        Point(lRect.left, lRect.top + computer_font->ascent),
                        get_object_short_name(obj->special.base), color);
            }
        }
    }

    // write the name
    if (obj->destObject.get()) {
        auto dest = obj->destObject;
        bool friendly = (dest->owner == g.admiral);
        RgbColor color = GetRGBTranslateColorShade(friendly ? GREEN : RED, VERY_LIGHT);
        Rect lRect = mini_screen_line_bounds(screen_top, kMiniDestLineNum, 0, kMiniScreenWidth);
        computer_font->draw(Point(lRect.left, lRect.top + computer_font->ascent), dest->name(), color);
    }
}

void MiniComputerDoAccept() {
    MiniComputerExecute(
            globals()->gMiniScreenData.currentScreen,
            globals()->gMiniScreenData.selectLine,
            g.admiral);
}

void MiniComputerExecute(int32_t whichPage, int32_t whichLine, Handle<Admiral> whichAdmiral) {
    SpaceObject *anotherObject;

    switch ( whichPage)
    {
        case kMainMiniScreen:
            if (whichAdmiral == g.admiral) {
                switch ( whichLine)
                {
                    case kMainMiniBuild:
                        MakeMiniScreenFromIndString( kBuildMiniScreen);
                        MiniComputerSetBuildStrings();
                        break;

                    case kMainMiniSpecial:
                        MakeMiniScreenFromIndString( kSpecialMiniScreen);
                        break;

                    case kMainMiniMessage:
                        MakeMiniScreenFromIndString( kMessageMiniScreen);
                        break;

                    case kMainMiniStatus:
                        MakeMiniScreenFromIndString( kStatusMiniScreen);
                        MiniComputerSetStatusStrings();
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
                if (CountObjectsOfBaseType(BaseObject::none(), Admiral::none()) < (kMaxSpaceObject - kMaxShipBuffer)) {
                    if (whichAdmiral->build(whichLine - kBuildScreenFirstTypeLine) == false) {
                        if (whichAdmiral == g.admiral) {
                            mPlayBeepBad();
                        }
                    }
                } else
                {
                    if (whichAdmiral == g.admiral) {
                        Messages::set_status("Maximum number of ships built", ORANGE);
                    }
                }
            }
            break;

        case kSpecialMiniScreen:
            if ( globals()->keyMask & kComputerSpecialMenu) return;
            switch ( whichLine)
            {
                case kSpecialMiniTransfer: {
                    auto control = whichAdmiral->control();
                    auto flagship = whichAdmiral->flagship();
                    if (flagship.get()) {
                        if (control.get()) {
                            if (!(control->attributes & kCanThink)
                                    || (control->attributes & kStaticDestination)
                                    || (control->owner != flagship->owner)
                                    || !(control->attributes & kCanAcceptDestination)
                                    || !(control->attributes & kCanBeDestination)
                                    || (flagship->active != kObjectInUse)) {
                                if (whichAdmiral == g.admiral) {
                                    mPlayBeepBad();
                                }
                            } else {
                                ChangePlayerShipNumber(whichAdmiral, control);
                            }
                        }
                    }
                    break;
                }

                case kSpecialMiniFire1: {
                    auto control = whichAdmiral->control();
                    if (control.get()) {
                        if (control->attributes & kCanAcceptDestination) {
                            control->keysDown |= kOneKey | kManualOverrideFlag;
                        }
                    }
                    break;
                }

                case kSpecialMiniFire2: {
                    auto control = whichAdmiral->control();
                    if (control.get()) {
                        if (control->attributes & kCanAcceptDestination) {
                            control->keysDown |= kTwoKey | kManualOverrideFlag;
                        }
                    }
                    break;
                }

                case kSpecialMiniFireSpecial: {
                    auto control = whichAdmiral->control();
                    if (control.get()) {
                        if (control->attributes & kCanAcceptDestination) {
                            control->keysDown |= kEnterKey | kManualOverrideFlag;
                        }
                    }
                    break;
                }

                case kSpecialMiniHold: {
                    auto control = whichAdmiral->control();
                    if (control.get()) {
                        SetObjectLocationDestination(control, &control->location);
                    }
                    break;
                }

                case kSpecialMiniGoToMe: {
                    auto control = whichAdmiral->control();
                    if (control.get()) {
                        auto flagship = whichAdmiral->flagship();
                        SetObjectLocationDestination(control, &flagship->location);
                    }
                    break;
                }

                default:
                    break;
            }
            break;

        case kMessageMiniScreen:
            if ( globals()->keyMask & kComputerMessageMenu) return;
            if (whichAdmiral == g.admiral) {
                switch ( whichLine)
                {
                    case kMessageMiniNext:
                        Messages::advance();
                        break;

                    case kMessageMiniLast:
                        Messages::replay();
                        break;

                    case kMessageMiniPrevious:
                        Messages::previous();
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

            break;

        default:
            break;
    }
}

void MiniComputerSetBuildStrings( void) // sets the ship type strings for the build screen
// also sets up the values = base object num

{
    Handle<Destination> buildAtObject;
    miniScreenLineType  *line = NULL;
    int32_t                count, lineNum;
    Rect                mRect;

    mRect = Rect(kMiniScreenLeft, kMiniScreenTop + instrument_top(), kMiniScreenRight,
                kMiniScreenBottom + instrument_top());

    globals()->gMiniScreenData.selectLine = kMiniScreenNoLineSelected;
    if ( globals()->gMiniScreenData.currentScreen == kBuildMiniScreen)
    {
        const auto& admiral = g.admiral;
        line = globals()->gMiniScreenData.lineData.get() +
            kBuildScreenWhereNameLine;
        auto buildAtObject = GetAdmiralBuildAtObject(g.admiral);
        line->value = buildAtObject.number();

        if (buildAtObject.get()) {
            mCopyBlankLineString( line, buildAtObject->name);

            line = globals()->gMiniScreenData.lineData.get() + kBuildScreenFirstTypeLine;
            lineNum = kBuildScreenFirstTypeLine;

            for ( count = 0; count < kMaxShipCanBuild; count++)
            {
                auto buildObject = mGetBaseObjectFromClassRace(
                        buildAtObject->canBuildType[count], admiral->race());
                line->value = buildObject.number();
                line->sourceData = buildObject;
                if (buildObject.get()) {
                    mCopyBlankLineString(line, get_object_name(buildObject));
                    if ( buildObject->price > mFixedToLong(admiral->cash()))
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
                    line->string.clear();
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
                line->string.clear();
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

int32_t MiniComputerGetPriceOfCurrentSelection( void)
{
    miniScreenLineType  *line = NULL;

    if (( globals()->gMiniScreenData.currentScreen != kBuildMiniScreen) ||
            ( globals()->gMiniScreenData.selectLine == kMiniScreenNoLineSelected))
        return (0);

        line = globals()->gMiniScreenData.lineData.get() +
            globals()->gMiniScreenData.selectLine;

        if ( line->value < 0) return( 0);

        auto buildObject = Handle<BaseObject>(line->value);

        if ( buildObject->price < 0) return( 0);

        return( mLongToFixed(buildObject->price));
}

void MiniComputerSetStatusStrings() {
    // the strings must be in this format:
    //  type\number\player\negativevalue\falsestring\truestring\basestring\poststring
    //
    //  where type = 0...5
    //
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

    miniScreenLineType  *line;

    if (gMissionStatusStrList.get() == NULL) {
        for (int count = kStatusMiniScreenFirstLine; count < kMiniScreenCharHeight; count++) {
            line = globals()->gMiniScreenData.lineData.get() + count;
            line->statusType = kNoStatusData;
            line->value = -1;
            line->string.clear();
        }
        return;
    }

    for (int count = kStatusMiniScreenFirstLine; count < kMiniScreenCharHeight; count++) {
        line = globals()->gMiniScreenData.lineData.get() + count;

        if (implicit_cast<size_t>(count - kStatusMiniScreenFirstLine) <
                gMissionStatusStrList->size()) {
            // we have some data for this line to interpret

            StringSlice sourceString =
                gMissionStatusStrList->at(count - kStatusMiniScreenFirstLine);

            if (sourceString.at(0) == '_') {
                line->underline = true;
                sourceString = sourceString.slice(1);
            }

            if (sourceString.at(0) == '-') {
                // - = abbreviated string, just plain text
                line->statusType = kPlainTextStatus;
                line->value = 0;
                line->string.assign(sourceString.slice(1));
            } else {
                //////////////////////////////////////////////
                // get status type
                StringSlice status_type_string;
                if (partition(status_type_string, "\\", sourceString)) {
                    int32_t value;
                    if (string_to_int<int32_t>(status_type_string, value)) {
                        if ((0 <= value) && (value <= kMaxStatusTypeValue)) {
                            line->statusType = value;
                        }
                    }
                }

                //////////////////////////////////////////////
                // get score/condition number
                StringSlice score_condition_string;
                if (partition(score_condition_string, "\\", sourceString)) {
                    int32_t value;
                    if (string_to_int<int32_t>(score_condition_string, value)) {
                        line->whichStatus = value;
                    }
                }

                //////////////////////////////////////////////
                // get player number
                StringSlice player_number_string;
                if (partition(player_number_string, "\\", sourceString)) {
                    int32_t value;
                    if (string_to_int<int32_t>(player_number_string, value)) {
                        line->statusPlayer = Handle<Admiral>(value);
                    }
                }

                //////////////////////////////////////////////
                // get negative value
                StringSlice negative_value_string;
                if (partition(negative_value_string, "\\", sourceString)) {
                    int32_t value;
                    if (string_to_int<int32_t>(negative_value_string, value)) {
                        line->negativeValue = value;
                    }
                }

                //////////////////////////////////////////////
                // get falseString
                StringSlice status_false_string;
                if (partition(status_false_string, "\\", sourceString)) {
                    line->statusFalse.assign(status_false_string);
                }

                //////////////////////////////////////////////
                // get trueString
                StringSlice status_true_string;
                if (partition(status_true_string, "\\", sourceString)) {
                    line->statusTrue.assign(status_true_string);
                }

                //////////////////////////////////////////////
                // get statusString
                StringSlice status_string;
                if (partition(status_string, "\\", sourceString)) {
                    line->statusString.assign(status_string);
                }

                //////////////////////////////////////////////
                // get postString
                line->postString.assign(sourceString);

                line->value = MiniComputerGetStatusValue( count);
                MiniComputerMakeStatusString(count, line->string);
            }
        } else {
            line->statusType = kNoStatusData;
            line->value = -1;
            line->string.clear();
        }
    }
}

void MiniComputerMakeStatusString(int32_t which_line, String& string) {
    string.clear();

    const miniScreenLineType& line = globals()->gMiniScreenData.lineData[which_line];
    if (line.statusType == kNoStatusData) {
        return;
    }

    print(string, line.statusString);
    switch (line.statusType) {
        case kTrueFalseCondition:
            if (line.value == 1) {
                print(string, line.statusTrue);
            } else {
                print(string, line.statusFalse);
            }
            break;

        case kIntegerValue:
        case kIntegerMinusValue:
            print(string, line.value);
            break;

        case kSmallFixedValue:
        case kSmallFixedMinusValue:
            print(string, fixed(line.value));
            break;
    }
    if (line.statusType != kPlainTextStatus) {
        print(string, line.postString);
    }
}

int32_t MiniComputerGetStatusValue( int32_t whichLine)
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
            if (g.level->condition(line->whichStatus)->true_yet()) {
                return 1;
            } else {
                return 0;
            }
            break;

        case kIntegerValue:
        case kSmallFixedValue:
            return GetAdmiralScore(line->statusPlayer, line->whichStatus);
            break;

        case kIntegerMinusValue:
        case kSmallFixedMinusValue:
            return line->negativeValue
                - GetAdmiralScore(line->statusPlayer, line->whichStatus);
            break;

        default:
            return( 0);
            break;
    }
}

void MiniComputerHandleClick( Point where)

{
    Rect        mRect;
    int32_t        lineNum, scrap, inLineButtonLine = -1, outLineButtonLine = -1;
    miniScreenLineType  *line;

    line = globals()->gMiniScreenData.lineData.get();
    scrap = 0;
    while ( scrap < kMiniScreenTrueLineNum)
    {
        if ( line->whichButton == kInLineButton) inLineButtonLine = scrap;
        else if ( line->whichButton == kOutLineButton) outLineButtonLine = scrap;
        scrap++;
        line++;
    }

    mRect = Rect(kButBoxLeft, kButBoxTop + instrument_top(), kButBoxRight,
                kButBoxBottom + instrument_top());

    // if click is in button screen
    if (mRect.contains(where)) {
        lineNum = (( where.v - ( kButBoxTop + instrument_top())) / computer_font->height) + kMiniScreenCharHeight;
        globals()->gMiniScreenData.clickLine = lineNum;
        line = globals()->gMiniScreenData.lineData.get() + lineNum;
        if ( line->whichButton == kInLineButton)
        {
            if ( line->lineKind != buttonOnLineKind)
            {
                line->lineKind = buttonOnLineKind;
                mPlayBeep3();
            }
            if ( outLineButtonLine >= 0)
            {
                line = globals()->gMiniScreenData.lineData.get() +
                    outLineButtonLine;
                if ( line->lineKind != buttonOffLineKind)
                {
                    line->lineKind = buttonOffLineKind;
                }
            }
        } else if ( line->whichButton == kOutLineButton)
        {
            if ( line->lineKind != buttonOnLineKind)
            {
                line->lineKind = buttonOnLineKind;
                mPlayBeep3();
            }
            if ( inLineButtonLine >= 0)
            {
                line = globals()->gMiniScreenData.lineData.get() + inLineButtonLine;
                if ( line->lineKind != buttonOffLineKind)
                {
                    line->lineKind = buttonOffLineKind;
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
            }
        }
        if ( outLineButtonLine >= 0)
        {
            line = globals()->gMiniScreenData.lineData.get() + outLineButtonLine;
            if ( line->lineKind != buttonOffLineKind)
            {
                line->lineKind = buttonOffLineKind;
            }
        }

        mRect = Rect(kMiniScreenLeft, kMiniScreenTop + instrument_top(), kMiniScreenRight,
                kMiniScreenBottom + instrument_top());

        // if click is in main menu screen
        if (mRect.contains(where)) {
            if ( globals()->gMiniScreenData.selectLine !=
                kMiniScreenNoLineSelected)
            {
                line = globals()->gMiniScreenData.lineData.get() +
                    globals()->gMiniScreenData.selectLine;
                line->hiliteLeft = line->hiliteRight = 0;
            }

            lineNum = mGetLineNumFromV(where.v);
            globals()->gMiniScreenData.clickLine = lineNum;
            line = globals()->gMiniScreenData.lineData.get() + lineNum;
            if (( line->selectable == selectable) || (line->selectable == selectDim))
            {
                globals()->gMiniScreenData.selectLine = lineNum;

                line = globals()->gMiniScreenData.lineData.get() +
                    globals()->gMiniScreenData.selectLine;
                line->hiliteLeft = mRect.left;
                line->hiliteRight = mRect.right;
            }
        } else globals()->gMiniScreenData.clickLine = kMiniScreenNoLineSelected;
    }
}

void MiniComputerHandleDoubleClick( Point where)

{
    Rect        mRect;
    int32_t        lineNum, scrap, inLineButtonLine = -1, outLineButtonLine = -1;
    miniScreenLineType  *line;

    line = globals()->gMiniScreenData.lineData.get();
    scrap = 0;
    while ( scrap < kMiniScreenTrueLineNum)
    {
        if ( line->whichButton == kInLineButton) inLineButtonLine = scrap;
        else if ( line->whichButton == kOutLineButton) outLineButtonLine = scrap;
        scrap++;
        line++;
    }

    mRect = Rect(kButBoxLeft, kButBoxTop + instrument_top(), kButBoxRight,
                kButBoxBottom + instrument_top());

    // if click is in button screen
    if (mRect.contains(where)) {
        lineNum = (( where.v - ( kButBoxTop + instrument_top())) / computer_font->height) + kMiniScreenCharHeight;
        line = globals()->gMiniScreenData.lineData.get() + lineNum;
        if ( line->whichButton == kInLineButton)
        {
            if ( line->lineKind != buttonOnLineKind)
            {
                line->lineKind = buttonOnLineKind;
                mPlayBeep3();
            }
            if ( outLineButtonLine >= 0)
            {
                line = globals()->gMiniScreenData.lineData.get() + outLineButtonLine;
                if ( line->lineKind != buttonOffLineKind)
                {
                    line->lineKind = buttonOffLineKind;
                }
            }
        } else if ( line->whichButton == kOutLineButton)
        {
            if ( line->lineKind != buttonOnLineKind)
            {
                line->lineKind = buttonOnLineKind;
                mPlayBeep3();
            }
            if ( inLineButtonLine >= 0)
            {
                line = globals()->gMiniScreenData.lineData.get() + inLineButtonLine;
                if ( line->lineKind != buttonOffLineKind)
                {
                    line->lineKind = buttonOffLineKind;
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
            }
        }
        if ( outLineButtonLine >= 0)
        {
            line = globals()->gMiniScreenData.lineData.get() + outLineButtonLine;
            if ( line->lineKind != buttonOffLineKind)
            {
                line->lineKind = buttonOffLineKind;
            }
        }

        mRect = Rect(kMiniScreenLeft, kMiniScreenTop + instrument_top(), kMiniScreenRight,
                kMiniScreenBottom + instrument_top());

        // if click is in main menu screen
        if (mRect.contains(where)) {
            lineNum = mGetLineNumFromV(where.v);
            if ( lineNum == globals()->gMiniScreenData.selectLine)
            {
                mPlayBeep3();
                MiniComputerDoAccept();
            } else
            {
                if ( globals()->gMiniScreenData.selectLine !=
                    kMiniScreenNoLineSelected)
                {
                    line = globals()->gMiniScreenData.lineData.get() + globals()->gMiniScreenData.selectLine;
                    line->hiliteLeft = line->hiliteRight = 0;
                }

                lineNum = mGetLineNumFromV(where.v);
                line = globals()->gMiniScreenData.lineData.get() + lineNum;
                if (( line->selectable == selectable) || (line->selectable == selectDim))
                {
                    globals()->gMiniScreenData.selectLine = lineNum;

                    line = globals()->gMiniScreenData.lineData.get() + globals()->gMiniScreenData.selectLine;
                    line->hiliteLeft = mRect.left;
                    line->hiliteRight = mRect.right;
                }
            }
        }
    }
}

void MiniComputerHandleMouseUp( Point where)

{
    Rect        mRect;
    int32_t        lineNum;
    miniScreenLineType  *line;

    mRect = Rect(kButBoxLeft, kButBoxTop + instrument_top(), kButBoxRight,
                kButBoxBottom + instrument_top());

    // if click is in button screen
    if (mRect.contains(where)) {
        lineNum = (( where.v - ( kButBoxTop + instrument_top())) / computer_font->height) + kMiniScreenCharHeight;
        line = globals()->gMiniScreenData.lineData.get() + lineNum;
        if ( line->whichButton == kInLineButton)
        {
            if ( line->lineKind == buttonOnLineKind)
            {
                line->lineKind = buttonOffLineKind;
                MiniComputerDoAccept();
            }
        } else if ( line->whichButton == kOutLineButton)
        {
            if ( line->lineKind == buttonOnLineKind)
            {
                line->lineKind = buttonOffLineKind;
                MiniComputerDoCancel();
            }
        }
    }
}

void MiniComputerHandleMouseStillDown( Point where)

{
    Rect        mRect;
    int32_t        lineNum, scrap, inLineButtonLine = -1, outLineButtonLine = -1;
    miniScreenLineType  *line;

    line = globals()->gMiniScreenData.lineData.get();
    scrap = 0;
    while ( scrap < kMiniScreenTrueLineNum)
    {
        if ( line->whichButton == kInLineButton) inLineButtonLine = scrap;
        else if ( line->whichButton == kOutLineButton) outLineButtonLine = scrap;
        scrap++;
        line++;
    }

    mRect = Rect(kButBoxLeft, kButBoxTop + instrument_top(), kButBoxRight,
                kButBoxBottom + instrument_top());

    // if click is in button screen
    if (mRect.contains(where)) {
        lineNum = (( where.v - ( kButBoxTop + instrument_top())) / computer_font->height) + kMiniScreenCharHeight;
        line = globals()->gMiniScreenData.lineData.get() + lineNum;
        if (( line->whichButton == kInLineButton) &&
            ( lineNum == globals()->gMiniScreenData.clickLine))
        {
            if ( line->lineKind != buttonOnLineKind)
            {
                line->lineKind = buttonOnLineKind;
            }
        } else if (( line->whichButton == kOutLineButton) &&
            ( lineNum == globals()->gMiniScreenData.clickLine))
        {
            if ( line->lineKind != buttonOnLineKind)
            {
                line->lineKind = buttonOnLineKind;
            }
        } else ( lineNum = -1);
    } else lineNum = -1;

    if ( lineNum == -1)
    {
        line = globals()->gMiniScreenData.lineData.get() + inLineButtonLine;
        if ( line->lineKind == buttonOnLineKind)
        {
            line->lineKind = buttonOffLineKind;
        }
        line = globals()->gMiniScreenData.lineData.get() + outLineButtonLine;
        if ( line->lineKind == buttonOnLineKind)
        {
            line->lineKind = buttonOffLineKind;
        }
    }
}

// for ambrosia tutorial, a horrific hack
void MiniComputer_SetScreenAndLineHack( int32_t whichScreen, int32_t whichLine)
{
    Point   w;

    switch ( whichScreen)
    {
        case kBuildMiniScreen:
            MakeMiniScreenFromIndString( kBuildMiniScreen);
            MiniComputerSetBuildStrings();
            break;

        case kSpecialMiniScreen:
            MakeMiniScreenFromIndString( kSpecialMiniScreen);
            break;

        case kMessageMiniScreen:
            MakeMiniScreenFromIndString( kMessageMiniScreen);
            break;

        case kStatusMiniScreen:
            MakeMiniScreenFromIndString( kStatusMiniScreen);
            MiniComputerSetStatusStrings();
            break;

        default:
            MakeMiniScreenFromIndString( kMainMiniScreen);
            break;
    }

    w.v = (whichLine * computer_font->height) + ( kMiniScreenTop + instrument_top());
    w.h = kMiniScreenLeft + 5;
    MiniComputerHandleClick( w);    // what an atrocious hack! oh well
}

}  // namespace antares
