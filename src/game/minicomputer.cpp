// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2017 The Antares Authors
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
#include "game/level.hpp"
#include "game/messages.hpp"
#include "game/player-ship.hpp"
#include "game/space-object.hpp"
#include "game/starfield.hpp"
#include "game/sys.hpp"
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
using std::vector;

namespace antares {

namespace {

const int32_t kMiniScreenCharWidth = 25;

const int32_t kMiniScreenLeft   = 12;
const int32_t kMiniScreenTop    = 320;
const int32_t kMiniScreenRight  = 121;
const int32_t kMiniScreenBottom = 440;
const int32_t kMiniScreenWidth  = kMiniScreenRight - kMiniScreenLeft;

const int32_t kMiniScreenLeftBuffer = 3;

const int32_t kMiniScreenCharHeight  = 10;  // height of the screen in characters
const int32_t kMiniScreenButtonNum   = 2;
const int32_t kMiniScreenTrueLineNum = kMiniScreenCharHeight + kMiniScreenButtonNum;

const int32_t kButBoxLeft   = 16;
const int32_t kButBoxTop    = 450;
const int32_t kButBoxRight  = 114;
const int32_t kButBoxBottom = 475;

const int32_t kMiniScreenNoLineSelected = -1;

const uint8_t kMiniScreenColor = GREEN;
const uint8_t kMiniButColor    = AQUA;

const int32_t kNoLineButton  = -1;
const int32_t kInLineButton  = kCompAcceptKeyNum;
const int32_t kOutLineButton = kCompCancelKeyNum;

static ANTARES_GLOBAL std::unique_ptr<StringList> gMissionStatusStrList;

enum {
    kMainMiniScreen    = 1,
    kBuildMiniScreen   = 2,
    kSpecialMiniScreen = 3,
    kMessageMiniScreen = 4,
    kStatusMiniScreen  = 5,
};

enum {
    kMainMiniBuild   = 1,
    kMainMiniSpecial = 2,
    kMainMiniMessage = 3,
    kMainMiniStatus  = 4,
};

const int32_t kBuildScreenFirstTypeLine = 2;
const int32_t kBuildScreenWhereNameLine = 1;

enum {
    kSpecialMiniTransfer    = 1,
    kSpecialMiniHold        = 2,
    kSpecialMiniGoToMe      = 3,
    kSpecialMiniFire1       = 4,
    kSpecialMiniFire2       = 5,
    kSpecialMiniFireSpecial = 6,
};

enum {
    kMessageMiniNext     = 1,
    kMessageMiniPrevious = 2,
    kMessageMiniLast     = 3,
};

const int32_t kStatusMiniScreenFirstLine = 1;

enum {
    kNoStatusData         = -1,  // no status for this line
    kPlainTextStatus      = 0,
    kTrueFalseCondition   = 1,  // 0 = F, 1 = T, use condition not score
    kIntegerValue         = 2,  // interpret score as int
    kSmallFixedValue      = 3,  // interpret score as fixed
    kIntegerMinusValue    = 4,  // value - designated score
    kSmallFixedMinusValue = 5,  // small fixed - designated score
    kMaxStatusTypeValue   = kSmallFixedMinusValue,
};

const int32_t kMiniSelectTop = 180;

const int32_t kMiniIconHeight = 22;
const int32_t kMiniIconWidth  = 22;
const int32_t kMiniIconLeft   = (kMiniScreenLeft + 2);

const int32_t kMiniHealthLeft = (kMiniIconLeft + kMiniIconWidth + 4);
const int32_t kMiniBarWidth   = 11;
const int32_t kMiniBarHeight  = 18;

const int32_t kMiniEnergyLeft = (kMiniHealthLeft + kMiniBarWidth + 2);

const int32_t kMiniRightColumnLeft = 57;
const int32_t kMiniWeapon1LineNum  = 2;
const int32_t kMiniWeapon2LineNum  = 3;
const int32_t kMiniWeapon3LineNum  = 1;
const int32_t kMiniNameLineNum     = 1;

const int32_t kMiniDestLineNum = 4;

const int32_t kMiniTargetTop = 252;

const int32_t kMiniAmmoTop         = 161;
const int32_t kMiniAmmoBottom      = 170;
const int32_t kMiniAmmoSingleWidth = 21;
const int32_t kMiniAmmoLeftOne     = 27;
const int32_t kMiniAmmoLeftTwo     = 64;
const int32_t kMiniAmmoLeftSpecial = 100;
const int32_t kMiniAmmoTextHBuffer = 2;

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
    return sys.fonts.computer->height * 2;
}

Rect mini_screen_line_bounds(int32_t mtop, int32_t mlinenum, int32_t mleft, int32_t mright) {
    Rect mbounds;
    mbounds.left   = kMiniScreenLeft + mleft;
    mbounds.top    = mtop + mlinenum * sys.fonts.computer->height;
    mbounds.right  = kMiniScreenLeft + mright;
    mbounds.bottom = mbounds.top + sys.fonts.computer->height;
    return mbounds;
}

inline int32_t mGetLineNumFromV(int32_t mV) {
    return (((mV) - (kMiniScreenTop + instrument_top())) / sys.fonts.computer->height);
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
static void MiniComputerExecute(
        int32_t whichPage, int32_t whichLine, Handle<Admiral> whichAdmiral);

void    MiniComputerSetStatusStrings(void);
int32_t MiniComputerGetStatusValue(int32_t);
void MiniComputerMakeStatusString(int32_t which_line, String& string);

void MiniScreenInit() {
    g.mini.selectLine    = kMiniScreenNoLineSelected;
    g.mini.currentScreen = kMainMiniScreen;
    g.mini.clickLine     = kMiniScreenNoLineSelected;

    g.mini.lineData.reset(new miniScreenLineType[kMiniScreenTrueLineNum]);

    ClearMiniScreenLines();
}

void MiniScreenCleanup() {
    g.mini.lineData.reset();
    // g.mini.objectData.reset();
}

#pragma mark -

void SetMiniScreenStatusStrList(int16_t strID) {
    DisposeMiniScreenStatusStrList();
    if (strID > 0) {
        gMissionStatusStrList.reset(new StringList(strID));
    }
}

void DisposeMiniScreenStatusStrList() {
    gMissionStatusStrList.reset();
}

void ClearMiniScreenLines() {
    miniScreenLineType* c = g.mini.lineData.get();
    for (int32_t b = 0; b < kMiniScreenTrueLineNum; b++) {
        c->string.clear();
        c->whichButton = kNoLineButton;
        c->kind        = MINI_NONE;
        c->underline   = false;
        c->sourceData  = BaseObject::none();
        c->callback    = nullptr;
        c++;
    }
}

static void underline(const Rects& rects, int line) {
    Rect rect = Rect(kMiniScreenLeft, kMiniScreenTop, kMiniScreenRight, kMiniScreenBottom);
    rect.offset(0, instrument_top());
    const RgbColor color = GetRGBTranslateColorShade(kMiniScreenColor, MEDIUM);
    int32_t        y = rect.top + (line * sys.fonts.computer->height) + sys.fonts.computer->ascent;
    rects.fill({rect.left, y, rect.right - 1, y + 1}, color);
}

static void highlight(const Rects& rects, int line) {
    Rect rect = Rect(kMiniScreenLeft, kMiniScreenTop, kMiniScreenRight, kMiniScreenBottom);
    rect.offset(0, instrument_top());
    Rect hilite_rect;
    hilite_rect.left   = rect.left;
    hilite_rect.top    = rect.top + (line * sys.fonts.computer->height);
    hilite_rect.right  = rect.right;
    hilite_rect.bottom = hilite_rect.top + sys.fonts.computer->height;
    draw_shaded_rect(rects, hilite_rect, kMiniScreenColor, DARK, MEDIUM, DARKER);
}

static void item_text(const Quads& quads, int line, StringSlice string, bool dim) {
    Rect rect = Rect(kMiniScreenLeft, kMiniScreenTop, kMiniScreenRight, kMiniScreenBottom);
    rect.offset(0, instrument_top());
    Point origin = rect.origin();
    origin.offset(
            kMiniScreenLeftBuffer,
            (line * sys.fonts.computer->height) + sys.fonts.computer->ascent);
    RgbColor textcolor = GetRGBTranslateColorShade(kMiniScreenColor, VERY_LIGHT);
    if (dim) {
        if (line == g.mini.selectLine) {
            textcolor = GetRGBTranslateColorShade(kMiniScreenColor, VERY_DARK);
        } else {
            textcolor = GetRGBTranslateColorShade(kMiniScreenColor, MEDIUM);
        }
    }
    sys.fonts.computer->draw(quads, origin, string, textcolor);
}

static void button_on(const Rects& rects, int line) {
    Rect rect = Rect(kButBoxLeft, kButBoxTop, kButBoxRight, kButBoxBottom);
    rect.offset(0, instrument_top());
    Rect hilite_rect;
    hilite_rect.left   = rect.left + kMiniScreenLeftBuffer - 2;
    hilite_rect.top    = rect.top + (line * sys.fonts.computer->height);
    hilite_rect.right  = rect.right;
    hilite_rect.bottom = hilite_rect.top + sys.fonts.computer->height;
    draw_shaded_rect(rects, hilite_rect, kMiniButColor, LIGHT, VERY_LIGHT, MEDIUM);
}

static void button_off(const Rects& rects, int line) {
    Rect rect = Rect(kButBoxLeft, kButBoxTop, kButBoxRight, kButBoxBottom);
    rect.offset(0, instrument_top());
    Rect hilite_rect;
    hilite_rect.left = rect.left + kMiniScreenLeftBuffer - 2;
    hilite_rect.top  = rect.top + (line * sys.fonts.computer->height);
    hilite_rect.right =
            rect.left + kMiniScreenLeftBuffer + sys.fonts.computer->logicalWidth * 4 + 1;
    hilite_rect.bottom = hilite_rect.top + sys.fonts.computer->height;
    draw_shaded_rect(rects, hilite_rect, kMiniButColor, MEDIUM, LIGHT, DARK);
}

static void button_on_text(const Quads& quads, int line, StringSlice string) {
    Rect rect = Rect(kButBoxLeft, kButBoxTop, kButBoxRight, kButBoxBottom);
    rect.offset(0, instrument_top());
    Point origin = rect.origin();
    origin.offset(
            kMiniScreenLeftBuffer,
            (line * sys.fonts.computer->height) + sys.fonts.computer->ascent);
    RgbColor textcolor = RgbColor::black();
    sys.fonts.computer->draw(quads, origin, string, textcolor);
}

static void button_off_text(const Quads& quads, int line, StringSlice string) {
    Rect rect = Rect(kButBoxLeft, kButBoxTop, kButBoxRight, kButBoxBottom);
    rect.offset(0, instrument_top());
    Point origin = rect.origin();
    origin.offset(
            kMiniScreenLeftBuffer,
            (line * sys.fonts.computer->height) + sys.fonts.computer->ascent);
    RgbColor textcolor = GetRGBTranslateColorShade(kMiniButColor, VERY_LIGHT);
    sys.fonts.computer->draw(quads, origin, string, textcolor);
}

static void draw_minicomputer_lines() {
    Rect mini_rect = Rect(kMiniScreenLeft, kMiniScreenTop, kMiniScreenRight, kMiniScreenBottom);
    mini_rect.offset(0, instrument_top());
    Rect but_box_rect = Rect(kButBoxLeft, kButBoxTop, kButBoxRight, kButBoxBottom);
    but_box_rect.offset(0, instrument_top());

    {
        Rects rects;
        rects.fill(mini_rect, GetRGBTranslateColorShade(kMiniScreenColor, DARKEST));
        for (int32_t i = 0; i < 9; i++) {
            if (g.mini.lineData[i].underline) {
                underline(rects, i);
            }
        }
        if (g.mini.selectLine != kMiniScreenNoLineSelected) {
            highlight(rects, g.mini.selectLine);
        }
        underline(rects, 9);

        rects.fill(but_box_rect, GetRGBTranslateColorShade(kMiniButColor, DARKEST));
        switch (g.mini.lineData[kMiniScreenCharHeight].kind) {
            case MINI_BUTTON_OFF: button_off(rects, 0); break;
            case MINI_BUTTON_ON: button_on(rects, 0); break;
            default: break;
        }
        switch (g.mini.lineData[kMiniScreenCharHeight + 1].kind) {
            case MINI_BUTTON_OFF: button_off(rects, 1); break;
            case MINI_BUTTON_ON: button_on(rects, 1); break;
            default: break;
        }
    }

    {
        Quads       quads(sys.fonts.computer->texture);
        bool        dim[kMiniScreenCharHeight];
        StringSlice strings[kMiniScreenCharHeight];
        for (int32_t count = 0; count < kMiniScreenCharHeight; count++) {
            auto c         = &g.mini.lineData[count];
            dim[count]     = (c->kind == MINI_DIM);
            strings[count] = c->string;
        }

        for (int32_t count = 0; count < kMiniScreenCharHeight; count++) {
            item_text(quads, count, strings[count], dim[count]);
        }

        for (int32_t count = 0; count < kMiniScreenButtonNum; count++) {
            switch (g.mini.lineData[count + kMiniScreenCharHeight].kind) {
                case MINI_BUTTON_ON:
                    button_on_text(
                            quads, count, g.mini.lineData[count + kMiniScreenCharHeight].string);
                    break;
                case MINI_BUTTON_OFF:
                    button_off_text(
                            quads, count, g.mini.lineData[count + kMiniScreenCharHeight].string);
                    break;
                default: break;
            }
        }
    }
}

void draw_mini_screen() {
    switch (g.mini.currentScreen) {
        case kMainMiniScreen:
        case kBuildMiniScreen:
        case kSpecialMiniScreen:
        case kMessageMiniScreen:
        case kStatusMiniScreen: draw_minicomputer_lines(); break;
    }
    draw_mini_ship_data(
            g.admiral->control(), YELLOW, kMiniSelectTop + instrument_top(), "CONTROL");
    draw_mini_ship_data(
            g.admiral->target(), SKY_BLUE, kMiniTargetTop + instrument_top(), "TARGET");
}

static miniScreenLineType text(StringSlice name, bool underlined) {
    miniScreenLineType line;
    line.string.assign(name);
    line.kind      = MINI_NONE;
    line.underline = underlined;
    return line;
}

static miniScreenLineType selectable(
        StringSlice name, void (*callback)(Handle<Admiral> adm, int32_t line)) {
    miniScreenLineType line;
    line.kind = MINI_SELECTABLE;
    line.string.assign(name);
    line.callback = callback;
    return line;
}

static miniScreenLineType accept(StringSlice name) {
    miniScreenLineType line;
    GetKeyNumName(sys.prefs->key(kCompAcceptKeyNum), &line.string);
    pad_to(line.string, kKeyNameLength);
    line.string.append(" ");
    line.string.append(name);
    line.kind        = MINI_BUTTON_OFF;
    line.whichButton = kInLineButton;
    return line;
}

static miniScreenLineType cancel(StringSlice name) {
    miniScreenLineType line;
    GetKeyNumName(sys.prefs->key(kCompCancelKeyNum), &line.string);
    pad_to(line.string, kKeyNameLength);
    line.string.append(" ");
    line.string.append(name);
    line.kind        = MINI_BUTTON_OFF;
    line.whichButton = kOutLineButton;
    return line;
}

static void make_mini_screen(int16_t screen, vector<miniScreenLineType> lines) {
    auto* item           = g.mini.lineData.get();
    auto* button         = g.mini.lineData.get() + kMiniScreenCharHeight;
    g.mini.currentScreen = screen;
    g.mini.selectLine    = kMiniScreenNoLineSelected;

    ClearMiniScreenLines();
    for (const auto& src : lines) {
        miniScreenLineType* dst;
        switch (src.kind) {
            case MINI_SELECTABLE:
            case MINI_DIM:
                if (g.mini.selectLine == kMiniScreenNoLineSelected) {
                    g.mini.selectLine = item - g.mini.lineData.get();
                }
            // fall through

            case MINI_NONE: dst = item++; break;

            case MINI_BUTTON_ON:
            case MINI_BUTTON_OFF:
                if (src.whichButton == kInLineButton) {
                    dst = &button[0];
                } else {
                    dst = &button[1];
                }
                break;
        }
        dst->kind = src.kind;
        dst->string.assign(src.string);
        dst->underline   = src.underline;
        dst->callback    = src.callback;
        dst->whichButton = src.whichButton;
    }
}

static void minicomputer_handle_action(int32_t button, bool key_down, void (*action)()) {
    // find out which line, if any, contains this button
    for (size_t i : range(kMiniScreenTrueLineNum)) {
        miniScreenLineType& line = g.mini.lineData[i];
        if (line.whichButton != button) {
            continue;
        }
        // hilite/unhilite this button
        if (key_down) {
            if (line.kind != MINI_BUTTON_ON) {
                line.kind = MINI_BUTTON_ON;
                sys.sound.click();
            }
        } else {
            if (line.kind != MINI_BUTTON_OFF) {
                line.kind = MINI_BUTTON_OFF;
                if (action) {
                    action();
                }
            }
        }
    }
}

static void minicomputer_handle_move(int direction) {
    if (g.mini.selectLine == kMiniScreenNoLineSelected) {
        return;
    }
    miniScreenLineType* line = g.mini.lineData.get() + g.mini.selectLine;
    do {
        line += direction;
        g.mini.selectLine += direction;
        if (g.mini.selectLine < 0) {
            g.mini.selectLine += kMiniScreenCharHeight;
            line += kMiniScreenCharHeight;
        } else if (g.mini.selectLine >= kMiniScreenCharHeight) {
            g.mini.selectLine -= kMiniScreenCharHeight;
            line -= kMiniScreenCharHeight;
        }
    } while (line->kind == MINI_NONE);
}

void minicomputer_handle_keys(uint32_t key_presses, uint32_t key_releases) {
    if ((key_presses | key_releases) & kCompAcceptKey) {
        minicomputer_handle_action(
                kInLineButton, key_presses & kCompAcceptKey, MiniComputerDoAccept);
    }

    if ((key_presses | key_releases) & kCompCancelKey) {
        minicomputer_handle_action(
                kOutLineButton, key_presses & kCompCancelKey, MiniComputerDoCancel);
    }

    if (key_presses & kCompUpKey) {
        minicomputer_handle_move(-1);
    }

    if (key_presses & kCompDownKey) {
        minicomputer_handle_move(+1);
    }
}

void minicomputer_cancel() {
    minicomputer_handle_action(kInLineButton, false, NULL);
    minicomputer_handle_action(kOutLineButton, false, NULL);
}

static void update_build_screen_lines() {
    const auto&         admiral = g.admiral;
    miniScreenLineType* line    = &g.mini.lineData[kBuildScreenWhereNameLine];
    if (line->value != GetAdmiralBuildAtObject(admiral).number()) {
        if (g.mini.selectLine != kMiniScreenNoLineSelected) {
            line              = &g.mini.lineData[g.mini.selectLine];
            g.mini.selectLine = kMiniScreenNoLineSelected;
        }
        MiniComputerSetBuildStrings();
    } else if (GetAdmiralBuildAtObject(admiral).get()) {
        line            = g.mini.lineData.get() + kBuildScreenFirstTypeLine;
        int32_t lineNum = kBuildScreenFirstTypeLine;

        for (int32_t count = 0; count < kMaxShipCanBuild; count++) {
            auto buildObject = line->sourceData;
            if (buildObject.get()) {
                if (buildObject->price > mFixedToLong(admiral->cash())) {
                    if (line->kind != MINI_DIM) {
                        line->kind = MINI_DIM;
                    }
                } else {
                    if (line->kind != MINI_SELECTABLE) {
                        if (g.mini.selectLine == kMiniScreenNoLineSelected) {
                            g.mini.selectLine = lineNum;
                        }
                        line->kind = MINI_SELECTABLE;
                    }
                }
            }
            line++;
            lineNum++;
        }
    }
}

static void update_status_screen_lines() {
    for (int32_t count = kStatusMiniScreenFirstLine; count < kMiniScreenCharHeight; count++) {
        miniScreenLineType* line    = &g.mini.lineData[count];
        int32_t             lineNum = MiniComputerGetStatusValue(count);
        if (line->value != lineNum) {
            line->value = lineNum;
            MiniComputerMakeStatusString(count, line->string);
        }
    }
}

// only for updating volitile lines--doesn't draw whole screen!
void UpdateMiniScreenLines() {
    switch (g.mini.currentScreen) {
        case kBuildMiniScreen: update_build_screen_lines(); break;

        case kStatusMiniScreen: update_status_screen_lines(); break;
    }
}

static void draw_player_ammo_in_rect(int32_t value, int8_t hue, const Rect& rect) {
    if (value >= 0) {
        const RgbColor text_color = GetRGBTranslateColorShade(hue, VERY_LIGHT);
        const char     digits[]   = {
                char(((value % 1000) / 100) + '0'), char(((value % 100) / 10) + '0'),
                char((value % 10) + '0'), '\0',
        };
        Point origin(rect.left + kMiniAmmoTextHBuffer, rect.bottom - 1);
        sys.fonts.computer->draw(origin, digits, text_color);
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
        sys.fonts.computer->draw(
                Point(bar.left + kMiniScreenLeftBuffer, bar.top + sys.fonts.computer->ascent),
                label, RgbColor::black());
    }

    // Icon
    Rect icon_rect = {
            {kMiniIconLeft, screen_top + MiniIconMacLineTop()}, {kMiniIconWidth, kMiniIconHeight},
    };
    if (!obj.get()) {
        draw_vbracket(Rects(), icon_rect, GetRGBTranslateColorShade(PALE_GREEN, MEDIUM));
        return;
    }

    {
        // Object name.
        if (obj->base.get()) {
            Rect lRect =
                    mini_screen_line_bounds(screen_top, kMiniNameLineNum, 0, kMiniScreenWidth);
            sys.fonts.computer->draw(
                    Point(lRect.left + kMiniScreenLeftBuffer,
                          lRect.top + sys.fonts.computer->ascent),
                    obj->short_name(), GetRGBTranslateColorShade(PALE_GREEN, VERY_LIGHT));
        }
    }

    if ((obj->base.get()) && (obj->pixResID >= 0)) {
        // Icon
        NatePixTable* pixTable = sys.pix.get(obj->pixResID);

        if (pixTable != NULL) {
            int16_t whichShape;
            if (obj->attributes & kIsSelfAnimated) {
                whichShape = more_evil_fixed_to_long(obj->base->frame.animation.firstShape);
            } else {
                whichShape = 0;
            }

            // get the picture data
            const NatePixTable::Frame& frame = pixTable->at(whichShape);

            Rect    rect(0, 0, frame.width(), frame.height());
            int32_t max_dimension = max(frame.width(), frame.height());
            if (max_dimension > kMiniIconHeight) {
                rect.right  = (rect.right * (kMiniIconHeight - 4)) / max_dimension;
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
            Rect  dRect = {Point(kMiniHealthLeft, screen_top + MiniIconMacLineTop()),
                          Size(kMiniBarWidth, kMiniIconHeight)};

            uint32_t tlong = obj->_health * kMiniBarHeight;
            tlong /= obj->max_health();

            Rect lRect;
            lRect.left   = dRect.left + 2;
            lRect.top    = dRect.top + 2;
            lRect.right  = dRect.right - 2;
            lRect.bottom = dRect.bottom - 2 - tlong;
            rects.fill(lRect, GetRGBTranslateColorShade(SKY_BLUE, DARK));

            lRect.top    = dRect.bottom - 2 - tlong;
            lRect.bottom = dRect.bottom - 2;
            rects.fill(lRect, GetRGBTranslateColorShade(SKY_BLUE, LIGHT));

            draw_vbracket(rects, dRect, GetRGBTranslateColorShade(SKY_BLUE, MEDIUM));
        }
    }

    {
        if ((obj->max_energy() > 0) && (obj->_energy > 0)) {
            Rects rects;
            Rect  dRect = {Point(kMiniEnergyLeft, screen_top + MiniIconMacLineTop()),
                          Size(kMiniBarWidth, kMiniIconHeight)};

            uint32_t tlong = obj->_energy * kMiniBarHeight;
            tlong /= obj->max_energy();

            Rect lRect;
            lRect.left   = dRect.left + 2;
            lRect.top    = dRect.top + 2;
            lRect.right  = dRect.right - 2;
            lRect.bottom = dRect.bottom - 2 - tlong;
            rects.fill(lRect, GetRGBTranslateColorShade(YELLOW, DARK));

            lRect.top    = dRect.bottom - 2 - tlong;
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
            sys.fonts.computer->draw(
                    Point(lRect.left, lRect.top + sys.fonts.computer->ascent),
                    get_object_short_name(obj->beam.base), color);
        }

        if (obj->pulse.base.get()) {
            Rect lRect = mini_screen_line_bounds(
                    screen_top, kMiniWeapon2LineNum, kMiniRightColumnLeft, kMiniScreenWidth);
            sys.fonts.computer->draw(
                    Point(lRect.left, lRect.top + sys.fonts.computer->ascent),
                    get_object_short_name(obj->pulse.base), color);
        }

        // Don't show special weapons of destination objects.
        if (!(obj->attributes & kIsDestination)) {
            if (obj->special.base.get()) {
                Rect lRect = mini_screen_line_bounds(
                        screen_top, kMiniWeapon3LineNum, kMiniRightColumnLeft, kMiniScreenWidth);
                sys.fonts.computer->draw(
                        Point(lRect.left, lRect.top + sys.fonts.computer->ascent),
                        get_object_short_name(obj->special.base), color);
            }
        }
    }

    // write the name
    if (obj->destObject.get()) {
        auto     dest     = obj->destObject;
        bool     friendly = (dest->owner == g.admiral);
        RgbColor color    = GetRGBTranslateColorShade(friendly ? GREEN : RED, VERY_LIGHT);
        Rect lRect = mini_screen_line_bounds(screen_top, kMiniDestLineNum, 0, kMiniScreenWidth);
        sys.fonts.computer->draw(
                Point(lRect.left, lRect.top + sys.fonts.computer->ascent), dest->name(), color);
    }
}

void MiniComputerDoAccept() {
    MiniComputerExecute(g.mini.currentScreen, g.mini.selectLine, g.admiral);
}

void transfer_control(Handle<Admiral> adm, int32_t line) {
    auto control  = adm->control();
    auto flagship = adm->flagship();
    if (flagship.get() && control.get()) {
        if ((control->attributes & kCanThink) && !(control->attributes & kStaticDestination) &&
            (control->owner == flagship->owner) && (control->attributes & kCanAcceptDestination) &&
            (control->attributes & kCanBeDestination) && (flagship->active == kObjectInUse)) {
            ChangePlayerShipNumber(adm, control);
        } else if (adm == g.admiral) {
            sys.sound.warning();
        }
    }
}

static void build_ship(Handle<Admiral> adm, int32_t line) {
    if (g.key_mask & kComputerBuildMenu) {
        return;
    }
    if (CountObjectsOfBaseType(BaseObject::none(), Admiral::none()) <
        (kMaxSpaceObject - kMaxShipBuffer)) {
        if (adm->build(line - kBuildScreenFirstTypeLine) == false) {
            if (adm == g.admiral) {
                sys.sound.warning();
            }
        }
    } else {
        if (adm == g.admiral) {
            Messages::set_status("Maximum number of ships built", ORANGE);
        }
    }
}

static void fire(Handle<Admiral> adm, int key) {
    if (g.key_mask & kComputerSpecialMenu) {
        return;
    }
    auto control = adm->control();
    if (control.get()) {
        if (control->attributes & kCanAcceptDestination) {
            control->keysDown |= key | kManualOverrideFlag;
        }
    }
}

static void fire1(Handle<Admiral> adm, int32_t line) {
    fire(adm, kOneKey);
}
static void fire2(Handle<Admiral> adm, int32_t line) {
    fire(adm, kTwoKey);
}
static void fire_special(Handle<Admiral> adm, int32_t line) {
    fire(adm, kEnterKey);
}

static void hold_position(Handle<Admiral> adm, int32_t line) {
    if (g.key_mask & kComputerSpecialMenu) {
        return;
    }
    auto control = adm->control();
    if (control.get()) {
        SetObjectLocationDestination(control, &control->location);
    }
}

static void come_to_me(Handle<Admiral> adm, int32_t line) {
    if (g.key_mask & kComputerSpecialMenu) {
        return;
    }
    auto control = adm->control();
    if (control.get()) {
        auto flagship = adm->flagship();
        SetObjectLocationDestination(control, &flagship->location);
    }
}

static void next_message(Handle<Admiral> adm, int32_t line) {
    if (g.key_mask & kComputerMessageMenu) {
        return;
    }
    Messages::advance();
}

static void last_message(Handle<Admiral> adm, int32_t line) {
    if (g.key_mask & kComputerMessageMenu) {
        return;
    }
    Messages::replay();
}

static void prev_message(Handle<Admiral> adm, int32_t line) {
    if (g.key_mask & kComputerMessageMenu) {
        return;
    }
    Messages::previous();
}

static void show_build_screen(Handle<Admiral> adm, int32_t line) {
    if (adm != g.admiral) {
        return;
    }
    make_mini_screen(
            kBuildMiniScreen,
            {
                    text("BUILD SHIPS", false), text("", true), selectable("", build_ship),
                    selectable("", build_ship), selectable("", build_ship),
                    selectable("", build_ship), selectable("", build_ship),
                    selectable("", build_ship), accept("Build"), cancel("Main Menu"),
            });
    MiniComputerSetBuildStrings();
}

static void show_special_screen(Handle<Admiral> adm, int32_t line) {
    if (adm != g.admiral) {
        return;
    }
    make_mini_screen(
            kSpecialMiniScreen,
            {
                    text("SPECIAL ORDERS", true), selectable("Transfer Control", transfer_control),
                    selectable("Hold Position", hold_position),
                    selectable("Go To My Position", come_to_me),
                    selectable("Fire Weapon 1", fire1), selectable("Fire Weapon 2", fire2),
                    selectable("Fire Special", fire_special), accept("Execute"),
                    cancel("Main Menu"),
            });
}

static void show_message_screen(Handle<Admiral> adm, int32_t line) {
    if (adm != g.admiral) {
        return;
    }
    make_mini_screen(
            kMessageMiniScreen,
            {
                    text("MESSAGES", true), selectable("Next Page/Clear", next_message),
                    selectable("Previous Page", prev_message),
                    selectable("Last Message", last_message), accept("Execute"),
                    cancel("Main Menu"),
            });
}

static void show_status_screen(Handle<Admiral> adm, int32_t line) {
    if (adm != g.admiral) {
        return;
    }
    make_mini_screen(
            kStatusMiniScreen, {
                                       text("MISSION STATUS", true), cancel("Main Menu"),
                               });
    MiniComputerSetStatusStrings();
}

static void show_main_screen(Handle<Admiral> adm, int32_t line) {
    if (adm != g.admiral) {
        return;
    }
    make_mini_screen(
            kMainMiniScreen,
            {
                    text("MAIN MENU", true), selectable("<Build>", show_build_screen),
                    selectable("<Special Orders>", show_special_screen),
                    selectable("<Message>", show_message_screen),
                    selectable("<Mission Status>", show_status_screen), accept("Select"),
            });
}

static void MiniComputerExecute(
        int32_t whichPage, int32_t whichLine, Handle<Admiral> whichAdmiral) {
    if (whichLine != kMiniScreenNoLineSelected) {
        // TODO(sfiera): has to work for remote players, which means
        const miniScreenLineType* line = &g.mini.lineData[whichLine];
        if (line->callback) {
            line->callback(whichAdmiral, whichLine);
        }
    }
}

void MiniComputerDoCancel() {
    show_main_screen(g.admiral, 0);
}

void MiniComputerSetBuildStrings() {
    // sets the ship type strings for the build screen
    // also sets up the values = base object num
    if (g.mini.currentScreen != kBuildMiniScreen) {
        return;
    }

    // Clear header, selection, and all build entries.
    miniScreenLineType* header = &g.mini.lineData[kBuildScreenWhereNameLine];
    header->value              = -1;
    g.mini.selectLine          = kMiniScreenNoLineSelected;
    for (int32_t count = 0; count < kMaxShipCanBuild; count++) {
        miniScreenLineType* line = &g.mini.lineData[kBuildScreenFirstTypeLine + count];
        line->string.clear();
        line->kind  = MINI_NONE;
        line->value = -1;
    }

    auto buildAtObject = GetAdmiralBuildAtObject(g.admiral);
    if (!buildAtObject.get()) {
        return;
    }
    header->value = buildAtObject.number();
    mCopyBlankLineString(header, buildAtObject->name);

    for (int32_t count = 0; count < kMaxShipCanBuild; count++) {
        int32_t             lineNum = kBuildScreenFirstTypeLine + count;
        miniScreenLineType* line    = &g.mini.lineData[lineNum];
        auto                buildObject =
                mGetBaseObjectFromClassRace(buildAtObject->canBuildType[count], g.admiral->race());
        line->value      = buildObject.number();
        line->sourceData = buildObject;
        if (!buildObject.get()) {
            continue;
        }

        mCopyBlankLineString(line, get_object_name(buildObject));
        if (buildObject->price > mFixedToLong(g.admiral->cash())) {
            line->kind = MINI_DIM;
        } else {
            line->kind = MINI_SELECTABLE;
        }
        if (g.mini.selectLine == kMiniScreenNoLineSelected) {
            g.mini.selectLine = lineNum;
        }
    }
}

// MiniComputerGetPriceOfCurrentSelection:
//  If the Build Menu is up, returns the price of the currently selected
//  ship, regardless of whether or not it is affordable.
//
//  If the selection is not legal, or the current Menu is not the Build Menu,
//  returns 0

Fixed MiniComputerGetPriceOfCurrentSelection() {
    if ((g.mini.currentScreen != kBuildMiniScreen) ||
        (g.mini.selectLine == kMiniScreenNoLineSelected)) {
        return Fixed::zero();
    }

    miniScreenLineType* line = &g.mini.lineData[g.mini.selectLine];
    if (line->value < 0) {
        return Fixed::zero();
    }

    auto buildObject = Handle<BaseObject>(line->value);
    if (buildObject->price < 0) {
        return Fixed::zero();
    }

    return Fixed::from_long(buildObject->price);
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

    if (gMissionStatusStrList.get() == NULL) {
        for (int count = kStatusMiniScreenFirstLine; count < kMiniScreenCharHeight; count++) {
            miniScreenLineType* line = g.mini.lineData.get() + count;
            line->statusType         = kNoStatusData;
            line->value              = -1;
            line->string.clear();
        }
        return;
    }

    for (int count = kStatusMiniScreenFirstLine; count < kMiniScreenCharHeight; count++) {
        miniScreenLineType* line = g.mini.lineData.get() + count;

        if (implicit_cast<size_t>(count - kStatusMiniScreenFirstLine) <
            gMissionStatusStrList->size()) {
            // we have some data for this line to interpret

            StringSlice sourceString =
                    gMissionStatusStrList->at(count - kStatusMiniScreenFirstLine);

            if (sourceString.at(0) == '_') {
                line->underline = true;
                sourceString    = sourceString.slice(1);
            }

            if (sourceString.at(0) == '-') {
                // - = abbreviated string, just plain text
                line->statusType = kPlainTextStatus;
                line->value      = 0;
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

                line->value = MiniComputerGetStatusValue(count);
                MiniComputerMakeStatusString(count, line->string);
            }
        } else {
            line->statusType = kNoStatusData;
            line->value      = -1;
            line->string.clear();
        }
    }
}

void MiniComputerMakeStatusString(int32_t which_line, String& string) {
    string.clear();

    const miniScreenLineType& line = g.mini.lineData[which_line];
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
        case kIntegerMinusValue: print(string, line.value); break;

        case kSmallFixedValue:
        case kSmallFixedMinusValue: print(string, Fixed::from_val(line.value)); break;
    }
    if (line.statusType != kPlainTextStatus) {
        print(string, line.postString);
    }
}

int32_t MiniComputerGetStatusValue(int32_t whichLine) {
    miniScreenLineType* line = g.mini.lineData.get() + whichLine;

    if (line->statusType == kNoStatusData) {
        return -1;
    }

    switch (line->statusType) {
        case kPlainTextStatus: return 0; break;

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
            return line->negativeValue - GetAdmiralScore(line->statusPlayer, line->whichStatus);
            break;

        default: return 0; break;
    }
}

void MiniComputerHandleClick(Point where) {
    Rect                mRect;
    int32_t             lineNum, scrap, inLineButtonLine = -1, outLineButtonLine = -1;
    miniScreenLineType* line;

    line  = g.mini.lineData.get();
    scrap = 0;
    while (scrap < kMiniScreenTrueLineNum) {
        if (line->whichButton == kInLineButton) {
            inLineButtonLine = scrap;
        } else if (line->whichButton == kOutLineButton) {
            outLineButtonLine = scrap;
        }
        scrap++;
        line++;
    }

    mRect =
            Rect(kButBoxLeft, kButBoxTop + instrument_top(), kButBoxRight,
                 kButBoxBottom + instrument_top());

    // if click is in button screen
    if (mRect.contains(where)) {
        lineNum = ((where.v - (kButBoxTop + instrument_top())) / sys.fonts.computer->height) +
                  kMiniScreenCharHeight;
        g.mini.clickLine = lineNum;
        line             = g.mini.lineData.get() + lineNum;
        if (line->whichButton == kInLineButton) {
            if (line->kind != MINI_BUTTON_ON) {
                line->kind = MINI_BUTTON_ON;
                sys.sound.click();
            }
            if (outLineButtonLine >= 0) {
                line = g.mini.lineData.get() + outLineButtonLine;
                if (line->kind != MINI_BUTTON_OFF) {
                    line->kind = MINI_BUTTON_OFF;
                }
            }
        } else if (line->whichButton == kOutLineButton) {
            if (line->kind != MINI_BUTTON_ON) {
                line->kind = MINI_BUTTON_ON;
                sys.sound.click();
            }
            if (inLineButtonLine >= 0) {
                line = g.mini.lineData.get() + inLineButtonLine;
                if (line->kind != MINI_BUTTON_OFF) {
                    line->kind = MINI_BUTTON_OFF;
                }
            }
        }
    } else {
        // make sure both buttons are off
        if (inLineButtonLine >= 0) {
            line = g.mini.lineData.get() + inLineButtonLine;
            if (line->kind != MINI_BUTTON_OFF) {
                line->kind = MINI_BUTTON_OFF;
            }
        }
        if (outLineButtonLine >= 0) {
            line = g.mini.lineData.get() + outLineButtonLine;
            if (line->kind != MINI_BUTTON_OFF) {
                line->kind = MINI_BUTTON_OFF;
            }
        }

        mRect =
                Rect(kMiniScreenLeft, kMiniScreenTop + instrument_top(), kMiniScreenRight,
                     kMiniScreenBottom + instrument_top());

        // if click is in main menu screen
        if (mRect.contains(where)) {
            if (g.mini.selectLine != kMiniScreenNoLineSelected) {
                line = g.mini.lineData.get() + g.mini.selectLine;
            }

            lineNum          = mGetLineNumFromV(where.v);
            g.mini.clickLine = lineNum;
            line             = g.mini.lineData.get() + lineNum;
            if ((line->kind == MINI_SELECTABLE) || (line->kind == MINI_DIM)) {
                g.mini.selectLine = lineNum;
                line              = g.mini.lineData.get() + g.mini.selectLine;
            }
        } else {
            g.mini.clickLine = kMiniScreenNoLineSelected;
        }
    }
}

void MiniComputerHandleDoubleClick(Point where) {
    Rect                mRect;
    int32_t             lineNum, scrap, inLineButtonLine = -1, outLineButtonLine = -1;
    miniScreenLineType* line;

    line  = g.mini.lineData.get();
    scrap = 0;
    while (scrap < kMiniScreenTrueLineNum) {
        if (line->whichButton == kInLineButton) {
            inLineButtonLine = scrap;
        } else if (line->whichButton == kOutLineButton) {
            outLineButtonLine = scrap;
        }
        scrap++;
        line++;
    }

    mRect =
            Rect(kButBoxLeft, kButBoxTop + instrument_top(), kButBoxRight,
                 kButBoxBottom + instrument_top());

    // if click is in button screen
    if (mRect.contains(where)) {
        lineNum = ((where.v - (kButBoxTop + instrument_top())) / sys.fonts.computer->height) +
                  kMiniScreenCharHeight;
        line = g.mini.lineData.get() + lineNum;
        if (line->whichButton == kInLineButton) {
            if (line->kind != MINI_BUTTON_ON) {
                line->kind = MINI_BUTTON_ON;
                sys.sound.click();
            }
            if (outLineButtonLine >= 0) {
                line = g.mini.lineData.get() + outLineButtonLine;
                if (line->kind != MINI_BUTTON_OFF) {
                    line->kind = MINI_BUTTON_OFF;
                }
            }
        } else if (line->whichButton == kOutLineButton) {
            if (line->kind != MINI_BUTTON_ON) {
                line->kind = MINI_BUTTON_ON;
                sys.sound.click();
            }
            if (inLineButtonLine >= 0) {
                line = g.mini.lineData.get() + inLineButtonLine;
                if (line->kind != MINI_BUTTON_OFF) {
                    line->kind = MINI_BUTTON_OFF;
                }
            }
        }
    } else {
        // make sure both buttons are off
        if (inLineButtonLine >= 0) {
            line = g.mini.lineData.get() + inLineButtonLine;
            if (line->kind != MINI_BUTTON_OFF) {
                line->kind = MINI_BUTTON_OFF;
            }
        }
        if (outLineButtonLine >= 0) {
            line = g.mini.lineData.get() + outLineButtonLine;
            if (line->kind != MINI_BUTTON_OFF) {
                line->kind = MINI_BUTTON_OFF;
            }
        }

        mRect =
                Rect(kMiniScreenLeft, kMiniScreenTop + instrument_top(), kMiniScreenRight,
                     kMiniScreenBottom + instrument_top());

        // if click is in main menu screen
        if (mRect.contains(where)) {
            lineNum = mGetLineNumFromV(where.v);
            if (lineNum == g.mini.selectLine) {
                sys.sound.click();
                MiniComputerDoAccept();
            } else {
                if (g.mini.selectLine != kMiniScreenNoLineSelected) {
                    line = g.mini.lineData.get() + g.mini.selectLine;
                }

                lineNum = mGetLineNumFromV(where.v);
                line    = g.mini.lineData.get() + lineNum;
                if ((line->kind == MINI_SELECTABLE) || (line->kind == MINI_DIM)) {
                    g.mini.selectLine = lineNum;

                    line = g.mini.lineData.get() + g.mini.selectLine;
                }
            }
        }
    }
}

void MiniComputerHandleMouseUp(Point where) {
    Rect                mRect;
    int32_t             lineNum;
    miniScreenLineType* line;

    mRect =
            Rect(kButBoxLeft, kButBoxTop + instrument_top(), kButBoxRight,
                 kButBoxBottom + instrument_top());

    // if click is in button screen
    if (mRect.contains(where)) {
        lineNum = ((where.v - (kButBoxTop + instrument_top())) / sys.fonts.computer->height) +
                  kMiniScreenCharHeight;
        line = g.mini.lineData.get() + lineNum;
        if (line->whichButton == kInLineButton) {
            if (line->kind == MINI_BUTTON_ON) {
                line->kind = MINI_BUTTON_OFF;
                MiniComputerDoAccept();
            }
        } else if (line->whichButton == kOutLineButton) {
            if (line->kind == MINI_BUTTON_ON) {
                line->kind = MINI_BUTTON_OFF;
                MiniComputerDoCancel();
            }
        }
    }
}

void MiniComputerHandleMouseStillDown(Point where) {
    Rect                mRect;
    int32_t             lineNum, scrap, inLineButtonLine = -1, outLineButtonLine = -1;
    miniScreenLineType* line;

    line  = g.mini.lineData.get();
    scrap = 0;
    while (scrap < kMiniScreenTrueLineNum) {
        if (line->whichButton == kInLineButton) {
            inLineButtonLine = scrap;
        } else if (line->whichButton == kOutLineButton) {
            outLineButtonLine = scrap;
        }
        scrap++;
        line++;
    }

    mRect =
            Rect(kButBoxLeft, kButBoxTop + instrument_top(), kButBoxRight,
                 kButBoxBottom + instrument_top());

    // if click is in button screen
    if (mRect.contains(where)) {
        lineNum = ((where.v - (kButBoxTop + instrument_top())) / sys.fonts.computer->height) +
                  kMiniScreenCharHeight;
        line = g.mini.lineData.get() + lineNum;
        if ((line->whichButton == kInLineButton) && (lineNum == g.mini.clickLine)) {
            if (line->kind != MINI_BUTTON_ON) {
                line->kind = MINI_BUTTON_ON;
            }
        } else if ((line->whichButton == kOutLineButton) && (lineNum == g.mini.clickLine)) {
            if (line->kind != MINI_BUTTON_ON) {
                line->kind = MINI_BUTTON_ON;
            }
        } else {
            lineNum = -1;
        }
    } else {
        lineNum = -1;
    }

    if (lineNum == -1) {
        line = g.mini.lineData.get() + inLineButtonLine;
        if (line->kind == MINI_BUTTON_ON) {
            line->kind = MINI_BUTTON_OFF;
        }
        line = g.mini.lineData.get() + outLineButtonLine;
        if (line->kind == MINI_BUTTON_ON) {
            line->kind = MINI_BUTTON_OFF;
        }
    }
}

// for ambrosia tutorial, a horrific hack
void MiniComputer_SetScreenAndLineHack(int32_t whichScreen, int32_t whichLine) {
    Point w;

    switch (whichScreen) {
        case kBuildMiniScreen: show_build_screen(g.admiral, 0); break;
        case kSpecialMiniScreen: show_special_screen(g.admiral, 0); break;
        case kMessageMiniScreen: show_message_screen(g.admiral, 0); break;
        case kStatusMiniScreen: show_status_screen(g.admiral, 0); break;
        default: show_main_screen(g.admiral, 0); break;
    }

    w.v = (whichLine * sys.fonts.computer->height) + (kMiniScreenTop + instrument_top());
    w.h = kMiniScreenLeft + 5;
    MiniComputerHandleClick(w);  // what an atrocious hack! oh well
}

}  // namespace antares
