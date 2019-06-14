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
#include <pn/output>
#include <sfz/sfz.hpp>

#include "config/keys.hpp"
#include "config/preferences.hpp"
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

using sfz::bin;
using sfz::range;
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

const int32_t kMiniScreenCharHeight = 10;  // height of the screen in characters

const int32_t kButBoxLeft   = 16;
const int32_t kButBoxTop    = 450;
const int32_t kButBoxRight  = 114;
const int32_t kButBoxBottom = 475;

const int32_t kMiniScreenNoLineSelected = -1;

const Hue kMiniScreenColor = Hue::GREEN;
const Hue kMiniButColor    = Hue::AQUA;

const int32_t kNoLineButton  = -1;
const int32_t kInLineButton  = 0;
const int32_t kOutLineButton = 1;

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

void pad_to(pn::string& s, size_t width) {
    size_t length = pn::rune::count(s);
    if (length >= width) {
        return;
    }
    size_t     left_pad  = (width - length) / 2;
    size_t     right_pad = (width - length) - left_pad;
    pn::string t;
    for (size_t i = 0; i < left_pad; ++i) {
        t += pn::rune{' '};
    }
    t += s;
    for (size_t i = 0; i < right_pad; ++i) {
        t += pn::rune{' '};
    }
    s = std::move(t);
}

const int32_t MiniIconMacLineTop() { return sys.fonts.computer.height * 2; }

Rect mini_screen_line_bounds(int32_t mtop, int32_t mlinenum, int32_t mleft, int32_t mright) {
    Rect mbounds;
    mbounds.left   = kMiniScreenLeft + mleft;
    mbounds.top    = mtop + mlinenum * sys.fonts.computer.height;
    mbounds.right  = kMiniScreenLeft + mright;
    mbounds.bottom = mbounds.top + sys.fonts.computer.height;
    return mbounds;
}

inline int32_t mGetLineNumFromV(int32_t mV) {
    return (((mV) - (kMiniScreenTop + instrument_top())) / sys.fonts.computer.height);
}

inline void mCopyBlankLineString(MiniLine* mline, pn::string_view mstring) {
    if (pn::rune::count(mstring) > kMiniScreenCharWidth) {
        mstring = pn::rune::slice(mstring, 0, kMiniScreenCharWidth);
    }
    mline->string = mstring.copy();
}

}  // namespace

static void draw_mini_ship_data(
        Handle<SpaceObject> obj, Hue header_hue, int16_t screen_top, pn::string_view label);

void    MiniComputerSetStatusStrings(void);
int32_t MiniComputerGetStatusValue(int32_t);
void    MiniComputerMakeStatusString(int32_t which_line, pn::string& string);

void MiniScreenInit() {
    g.mini.selectLine    = kMiniScreenNoLineSelected;
    g.mini.currentScreen = Screen::MAIN;
    g.mini.clickLine     = kMiniScreenNoLineSelected;

    g.mini.lines.reset(new MiniLine[kMiniScreenCharHeight]);
    g.mini.accept.reset(new MiniButton);
    g.mini.cancel.reset(new MiniButton);

    ClearMiniScreenLines();
}

void MiniScreenCleanup() {
    g.mini.lines.reset();
    g.mini.accept.reset();
    g.mini.cancel.reset();
}

#pragma mark -

static void clear_line(MiniLine* line) {
    line->string.clear();
    line->kind       = MINI_NONE;
    line->underline  = false;
    line->sourceData = nullptr;
    line->callback   = nullptr;
}

static void clear_button(MiniButton* line) {
    line->string.clear();
    line->whichButton = kNoLineButton;
    line->kind        = MINI_BUTTON_NONE;
}

void ClearMiniScreenLines() {
    for (int32_t i = 0; i < kMiniScreenCharHeight; i++) {
        clear_line(&g.mini.lines[i]);
    }
    clear_button(g.mini.accept.get());
    clear_button(g.mini.cancel.get());
}

static void underline(const Rects& rects, int line) {
    Rect rect = Rect(kMiniScreenLeft, kMiniScreenTop, kMiniScreenRight, kMiniScreenBottom);
    rect.offset(0, instrument_top());
    const RgbColor color = GetRGBTranslateColorShade(kMiniScreenColor, MEDIUM);
    int32_t        y = rect.top + (line * sys.fonts.computer.height) + sys.fonts.computer.ascent;
    rects.fill({rect.left, y, rect.right - 1, y + 1}, color);
}

static void highlight(const Rects& rects, int line) {
    Rect rect = Rect(kMiniScreenLeft, kMiniScreenTop, kMiniScreenRight, kMiniScreenBottom);
    rect.offset(0, instrument_top());
    Rect hilite_rect;
    hilite_rect.left   = rect.left;
    hilite_rect.top    = rect.top + (line * sys.fonts.computer.height);
    hilite_rect.right  = rect.right;
    hilite_rect.bottom = hilite_rect.top + sys.fonts.computer.height;
    draw_shaded_rect(rects, hilite_rect, kMiniScreenColor, DARK, MEDIUM, DARKER);
}

static void item_text(const Quads& quads, int line, pn::string_view string, bool dim) {
    Rect rect = Rect(kMiniScreenLeft, kMiniScreenTop, kMiniScreenRight, kMiniScreenBottom);
    rect.offset(0, instrument_top());
    Point origin = rect.origin();
    origin.offset(
            kMiniScreenLeftBuffer, (line * sys.fonts.computer.height) + sys.fonts.computer.ascent);
    RgbColor textcolor = GetRGBTranslateColorShade(kMiniScreenColor, LIGHTEST);
    if (dim) {
        if (line == g.mini.selectLine) {
            textcolor = GetRGBTranslateColorShade(kMiniScreenColor, VERY_DARK);
        } else {
            textcolor = GetRGBTranslateColorShade(kMiniScreenColor, MEDIUM);
        }
    }
    sys.fonts.computer.draw(quads, origin, string, textcolor);
}

static void button_on(const Rects& rects, const MiniButton& button) {
    Rect rect = Rect(kButBoxLeft, kButBoxTop, kButBoxRight, kButBoxBottom);
    rect.offset(0, instrument_top());
    Rect hilite_rect;
    hilite_rect.left   = rect.left + kMiniScreenLeftBuffer - 2;
    hilite_rect.top    = rect.top + (button.whichButton * sys.fonts.computer.height);
    hilite_rect.right  = rect.right;
    hilite_rect.bottom = hilite_rect.top + sys.fonts.computer.height;
    draw_shaded_rect(rects, hilite_rect, kMiniButColor, LIGHT, LIGHTEST, MEDIUM);
}

static void button_off(const Rects& rects, const MiniButton& button) {
    Rect rect = Rect(kButBoxLeft, kButBoxTop, kButBoxRight, kButBoxBottom);
    rect.offset(0, instrument_top());
    Rect hilite_rect;
    hilite_rect.left = rect.left + kMiniScreenLeftBuffer - 2;
    hilite_rect.top  = rect.top + (button.whichButton * sys.fonts.computer.height);
    hilite_rect.right =
            rect.left + kMiniScreenLeftBuffer + sys.fonts.computer.logicalWidth * 4 + 1;
    hilite_rect.bottom = hilite_rect.top + sys.fonts.computer.height;
    draw_shaded_rect(rects, hilite_rect, kMiniButColor, MEDIUM, LIGHT, DARK);
}

static void button_on_text(const Quads& quads, const MiniButton& button) {
    Rect rect = Rect(kButBoxLeft, kButBoxTop, kButBoxRight, kButBoxBottom);
    rect.offset(0, instrument_top());
    Point origin = rect.origin();
    origin.offset(
            kMiniScreenLeftBuffer,
            (button.whichButton * sys.fonts.computer.height) + sys.fonts.computer.ascent);
    RgbColor textcolor = RgbColor::black();
    sys.fonts.computer.draw(quads, origin, button.string, textcolor);
}

static void button_off_text(const Quads& quads, const MiniButton& button) {
    Rect rect = Rect(kButBoxLeft, kButBoxTop, kButBoxRight, kButBoxBottom);
    rect.offset(0, instrument_top());
    Point origin = rect.origin();
    origin.offset(
            kMiniScreenLeftBuffer,
            (button.whichButton * sys.fonts.computer.height) + sys.fonts.computer.ascent);
    RgbColor textcolor = GetRGBTranslateColorShade(kMiniButColor, LIGHTEST);
    sys.fonts.computer.draw(quads, origin, button.string, textcolor);
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
            if (g.mini.lines[i].underline) {
                underline(rects, i);
            }
        }
        if (g.mini.selectLine != kMiniScreenNoLineSelected) {
            highlight(rects, g.mini.selectLine);
        }
        underline(rects, 9);

        rects.fill(but_box_rect, GetRGBTranslateColorShade(kMiniButColor, DARKEST));
        switch (g.mini.accept->kind) {
            case MINI_BUTTON_OFF: button_off(rects, *g.mini.accept); break;
            case MINI_BUTTON_ON: button_on(rects, *g.mini.accept); break;
            default: break;
        }
        switch (g.mini.cancel->kind) {
            case MINI_BUTTON_OFF: button_off(rects, *g.mini.cancel); break;
            case MINI_BUTTON_ON: button_on(rects, *g.mini.cancel); break;
            default: break;
        }
    }

    {
        Quads           quads(sys.fonts.computer.texture);
        bool            dim[kMiniScreenCharHeight];
        pn::string_view strings[kMiniScreenCharHeight];
        for (int32_t count = 0; count < kMiniScreenCharHeight; count++) {
            auto c         = &g.mini.lines[count];
            dim[count]     = (c->kind == MINI_DIM);
            strings[count] = c->string;
        }

        for (int32_t count = 0; count < kMiniScreenCharHeight; count++) {
            item_text(quads, count, strings[count], dim[count]);
        }

        switch (g.mini.accept->kind) {
            case MINI_BUTTON_ON: button_on_text(quads, *g.mini.accept); break;
            case MINI_BUTTON_OFF: button_off_text(quads, *g.mini.accept); break;
            default: break;
        }
        switch (g.mini.cancel->kind) {
            case MINI_BUTTON_ON: button_on_text(quads, *g.mini.cancel); break;
            case MINI_BUTTON_OFF: button_off_text(quads, *g.mini.cancel); break;
            default: break;
        }
    }
}

void draw_mini_screen() {
    switch (g.mini.currentScreen) {
        case Screen::MAIN:
        case Screen::BUILD:
        case Screen::SPECIAL:
        case Screen::MESSAGE:
        case Screen::STATUS: draw_minicomputer_lines(); break;
    }
    draw_mini_ship_data(
            g.admiral->control(), Hue::YELLOW, kMiniSelectTop + instrument_top(), "CONTROL");
    draw_mini_ship_data(
            g.admiral->target(), Hue::SKY_BLUE, kMiniTargetTop + instrument_top(), "TARGET");
}

static MiniLine text(pn::string_view name, bool underlined) {
    MiniLine line;
    line.string    = name.copy();
    line.kind      = MINI_NONE;
    line.underline = underlined;
    return line;
}

static MiniLine selectable(pn::string_view name, void (*callback)(Handle<Admiral> adm)) {
    MiniLine line;
    line.kind     = MINI_SELECTABLE;
    line.string   = name.copy();
    line.callback = callback;
    return line;
}

static MiniButton no_button() {
    MiniButton button;
    clear_button(&button);
    return button;
}

static MiniButton accept(pn::string_view name) {
    MiniButton button;
    pn::string line_string;
    GetKeyNumName(sys.prefs->key(kCompAcceptKeyNum), line_string);
    button.string = line_string.copy();
    pad_to(button.string, kKeyNameLength);
    button.string += " ";
    button.string += name;
    button.kind        = MINI_BUTTON_OFF;
    button.whichButton = kInLineButton;
    return button;
}

static MiniButton cancel(pn::string_view name) {
    MiniButton button;
    pn::string line_string;
    GetKeyNumName(sys.prefs->key(kCompCancelKeyNum), line_string);
    button.string = line_string.copy();
    pad_to(button.string, kKeyNameLength);
    button.string += " ";
    button.string += name;
    button.kind        = MINI_BUTTON_OFF;
    button.whichButton = kOutLineButton;
    return button;
}

template <size_t size>
static void make_mini_screen(
        Screen screen, const MiniLine (&lines)[size], const MiniButton& accept,
        const MiniButton& cancel) {
    auto* item           = g.mini.lines.get();
    g.mini.currentScreen = screen;
    g.mini.selectLine    = kMiniScreenNoLineSelected;

    ClearMiniScreenLines();
    for (const auto& src : lines) {
        MiniLine* dst;
        switch (src.kind) {
            case MINI_SELECTABLE:
            case MINI_DIM:
                if (g.mini.selectLine == kMiniScreenNoLineSelected) {
                    g.mini.selectLine = item - g.mini.lines.get();
                }
                // fall through

            case MINI_NONE: dst = item++; break;
        }
        dst->kind      = src.kind;
        dst->string    = src.string.copy();
        dst->underline = src.underline;
        dst->callback  = src.callback;
    }

    if (accept.kind != MINI_BUTTON_NONE) {
        g.mini.accept->kind        = accept.kind;
        g.mini.accept->string      = accept.string.copy();
        g.mini.accept->whichButton = accept.whichButton;
    }

    if (cancel.kind != MINI_BUTTON_NONE) {
        g.mini.cancel->kind        = cancel.kind;
        g.mini.cancel->string      = cancel.string.copy();
        g.mini.cancel->whichButton = cancel.whichButton;
    }
}

static void minicomputer_down(MiniButton* line) {
    if (line->kind == MINI_BUTTON_OFF) {
        line->kind = MINI_BUTTON_ON;
        sys.sound.click();
    }
}

static void minicomputer_up(MiniButton* line, void (*action)()) {
    if (line->kind == MINI_BUTTON_ON) {
        line->kind = MINI_BUTTON_OFF;
        if (action) {
            action();
        }
    }
}

static void minicomputer_handle_move(int direction) {
    if (g.mini.selectLine == kMiniScreenNoLineSelected) {
        return;
    }
    MiniLine* line = g.mini.lines.get() + g.mini.selectLine;
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

void minicomputer_handle_event(PlayerEvent e) {
    switch (e.key) {
        case PlayerEventType::COMP_ACCEPT_ON: minicomputer_down(g.mini.accept.get()); break;

        case PlayerEventType::COMP_CANCEL_ON: minicomputer_down(g.mini.cancel.get()); break;

        case PlayerEventType::COMP_UP_ON: minicomputer_handle_move(-1); break;
        case PlayerEventType::COMP_DOWN_ON: minicomputer_handle_move(+1); break;

        case PlayerEventType::COMP_ACCEPT_OFF:
            minicomputer_up(g.mini.accept.get(), MiniComputerDoAccept);
            break;

        case PlayerEventType::COMP_CANCEL_OFF:
            minicomputer_up(g.mini.cancel.get(), MiniComputerDoCancel);
            break;

        default: break;
    }
}

void minicomputer_handle_keys(std::vector<PlayerEvent> player_events) {
    for (const auto& e : player_events) {
        minicomputer_handle_event(e);
    }
}

void minicomputer_cancel() {
    minicomputer_up(g.mini.accept.get(), NULL);
    minicomputer_up(g.mini.cancel.get(), NULL);
}

static void update_build_screen_lines() {
    const auto& admiral = g.admiral;
    MiniLine*   line    = &g.mini.lines[kBuildScreenWhereNameLine];
    if (line->value != GetAdmiralBuildAtObject(admiral).number()) {
        if (g.mini.selectLine != kMiniScreenNoLineSelected) {
            line              = &g.mini.lines[g.mini.selectLine];
            g.mini.selectLine = kMiniScreenNoLineSelected;
        }
        MiniComputerSetBuildStrings();
    } else if (GetAdmiralBuildAtObject(admiral).get()) {
        line            = g.mini.lines.get() + kBuildScreenFirstTypeLine;
        int32_t lineNum = kBuildScreenFirstTypeLine;

        for (int32_t count = 0; count < kMaxShipCanBuild; count++) {
            auto buildObject = line->sourceData;
            if (buildObject) {
                if (buildObject->price > admiral->cash()) {
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
        MiniLine* line    = &g.mini.lines[count];
        int32_t   lineNum = MiniComputerGetStatusValue(count);
        if (line->value != lineNum) {
            line->value = lineNum;
            MiniComputerMakeStatusString(count, line->string);
        }
    }
}

// only for updating volitile lines--doesn't draw whole screen!
void UpdateMiniScreenLines() {
    switch (g.mini.currentScreen) {
        case Screen::BUILD: update_build_screen_lines(); break;
        case Screen::STATUS: update_status_screen_lines(); break;

        case Screen::MAIN:
        case Screen::MESSAGE:
        case Screen::SPECIAL: break;  // nothing to do
    }
}

static void draw_player_ammo_in_rect(int32_t value, Hue hue, const Rect& rect) {
    if (value >= 0) {
        const RgbColor text_color = GetRGBTranslateColorShade(hue, LIGHTEST);
        const char     digits[]   = {
                char(((value % 1000) / 100) + '0'),
                char(((value % 100) / 10) + '0'),
                char((value % 10) + '0'),
                '\0',
        };
        Point origin(rect.left + kMiniAmmoTextHBuffer, rect.bottom - 1);
        sys.fonts.computer.draw(origin, digits, text_color);
    }
}

void draw_player_ammo(int32_t ammo_one, int32_t ammo_two, int32_t ammo_special) {
    Rect clip(0, kMiniAmmoTop, kMiniAmmoSingleWidth, kMiniAmmoBottom);
    clip.offset(0, instrument_top());

    clip.offset(kMiniAmmoLeftOne - clip.left, 0);
    draw_player_ammo_in_rect(ammo_one, Hue::RED, clip);
    clip.offset(kMiniAmmoLeftTwo - clip.left, 0);
    draw_player_ammo_in_rect(ammo_two, Hue::PALE_GREEN, clip);
    clip.offset(kMiniAmmoLeftSpecial - clip.left, 0);
    draw_player_ammo_in_rect(ammo_special, Hue::ORANGE, clip);
}

static void draw_mini_ship_data(
        Handle<SpaceObject> obj, Hue header_hue, int16_t screen_top, pn::string_view label) {
    {
        // "CONTROL" or "TARGET" label.
        Rect bar = mini_screen_line_bounds(screen_top, 0, 0, kMiniScreenWidth);
        draw_shaded_rect(Rects(), bar, header_hue, LIGHT, LIGHTEST, MEDIUM);
        sys.fonts.computer.draw(
                Point(bar.left + kMiniScreenLeftBuffer, bar.top + sys.fonts.computer.ascent),
                label, RgbColor::black());
    }

    // Icon
    Rect icon_rect = {
            {kMiniIconLeft, screen_top + MiniIconMacLineTop()},
            {kMiniIconWidth, kMiniIconHeight},
    };
    if (!obj.get()) {
        draw_vbracket(Rects(), icon_rect, GetRGBTranslateColorShade(Hue::PALE_GREEN, MEDIUM));
        return;
    }

    {
        // Object name.
        if (obj->base) {
            Rect lRect =
                    mini_screen_line_bounds(screen_top, kMiniNameLineNum, 0, kMiniScreenWidth);
            sys.fonts.computer.draw(
                    Point(lRect.left + kMiniScreenLeftBuffer,
                          lRect.top + sys.fonts.computer.ascent),
                    obj->short_name(), GetRGBTranslateColorShade(Hue::PALE_GREEN, LIGHTEST));
        }
    }

    if (obj->base && obj->pix_id.has_value()) {
        // Icon
        NatePixTable* pixTable = sys.pix.get(obj->pix_id->name, obj->pix_id->hue);

        if (pixTable != NULL) {
            int16_t whichShape;
            if (obj->attributes & kIsSelfAnimated) {
                whichShape = more_evil_fixed_to_long(obj->base->animation->frames.begin);
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
    draw_vbracket(Rects(), icon_rect, GetRGBTranslateColorShade(Hue::PALE_GREEN, MEDIUM));

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
            rects.fill(lRect, GetRGBTranslateColorShade(Hue::SKY_BLUE, DARK));

            lRect.top    = dRect.bottom - 2 - tlong;
            lRect.bottom = dRect.bottom - 2;
            rects.fill(lRect, GetRGBTranslateColorShade(Hue::SKY_BLUE, LIGHT));

            draw_vbracket(rects, dRect, GetRGBTranslateColorShade(Hue::SKY_BLUE, MEDIUM));
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
            rects.fill(lRect, GetRGBTranslateColorShade(Hue::YELLOW, DARK));

            lRect.top    = dRect.bottom - 2 - tlong;
            lRect.bottom = dRect.bottom - 2;
            rects.fill(lRect, GetRGBTranslateColorShade(Hue::YELLOW, LIGHT));

            draw_vbracket(rects, dRect, GetRGBTranslateColorShade(Hue::YELLOW, MEDIUM));
        }
    }

    {
        // Weapons
        RgbColor color = GetRGBTranslateColorShade(Hue::PALE_GREEN, LIGHTEST);

        if (obj->beam.base) {
            Rect lRect = mini_screen_line_bounds(
                    screen_top, kMiniWeapon1LineNum, kMiniRightColumnLeft, kMiniScreenWidth);
            sys.fonts.computer.draw(
                    Point(lRect.left, lRect.top + sys.fonts.computer.ascent),
                    obj->beam.base->short_name, color);
        }

        if (obj->pulse.base) {
            Rect lRect = mini_screen_line_bounds(
                    screen_top, kMiniWeapon2LineNum, kMiniRightColumnLeft, kMiniScreenWidth);
            sys.fonts.computer.draw(
                    Point(lRect.left, lRect.top + sys.fonts.computer.ascent),
                    obj->pulse.base->short_name, color);
        }

        // Don't show special weapons of destination objects.
        if (!(obj->attributes & kIsDestination)) {
            if (obj->special.base) {
                Rect lRect = mini_screen_line_bounds(
                        screen_top, kMiniWeapon3LineNum, kMiniRightColumnLeft, kMiniScreenWidth);
                sys.fonts.computer.draw(
                        Point(lRect.left, lRect.top + sys.fonts.computer.ascent),
                        obj->special.base->short_name, color);
            }
        }
    }

    // write the name
    if (obj->destObject.get()) {
        auto     dest     = obj->destObject;
        bool     friendly = (dest->owner == g.admiral);
        RgbColor color    = GetRGBTranslateColorShade(friendly ? Hue::GREEN : Hue::RED, LIGHTEST);
        Rect lRect = mini_screen_line_bounds(screen_top, kMiniDestLineNum, 0, kMiniScreenWidth);
        sys.fonts.computer.draw(
                Point(lRect.left, lRect.top + sys.fonts.computer.ascent), dest->long_name(),
                color);
    }
}

void MiniComputerDoAccept() {
    if (g.mini.selectLine != kMiniScreenNoLineSelected) {
        // TODO(sfiera): has to work for remote players, which means
        const MiniLine* line = &g.mini.lines[g.mini.selectLine];
        if (line->callback) {
            line->callback(g.admiral);
        }
    }
}

void transfer_control(Handle<Admiral> adm) {
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

static void build_ship(Handle<Admiral> adm, int32_t index) {
    if (g.key_mask & kComputerBuildMenu) {
        return;
    }
    if (CountObjectsOfBaseType(nullptr, Admiral::none()) < (kMaxSpaceObject - kMaxShipBuffer)) {
        if (adm->build(index) == false) {
            if (adm == g.admiral) {
                sys.sound.warning();
            }
        }
    } else {
        if (adm == g.admiral) {
            Messages::max_ships_built();
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

static void fire_pulse(Handle<Admiral> adm) { fire(adm, kPulseKey); }
static void fire_beam(Handle<Admiral> adm) { fire(adm, kBeamKey); }
static void fire_special(Handle<Admiral> adm) { fire(adm, kSpecialKey); }

static void hold_position(Handle<Admiral> adm) {
    if (g.key_mask & kComputerSpecialMenu) {
        return;
    }
    auto control = adm->control();
    if (control.get()) {
        SetObjectLocationDestination(control, &control->location);
    }
}

static void come_to_me(Handle<Admiral> adm) {
    if (g.key_mask & kComputerSpecialMenu) {
        return;
    }
    auto control = adm->control();
    if (control.get()) {
        auto flagship = adm->flagship();
        SetObjectLocationDestination(control, &flagship->location);
    }
}

static void next_message(Handle<Admiral> adm) {
    if (g.key_mask & kComputerMessageMenu) {
        return;
    }
    Messages::advance();
}

static void last_message(Handle<Admiral> adm) {
    if (g.key_mask & kComputerMessageMenu) {
        return;
    }
    Messages::replay();
}

static void prev_message(Handle<Admiral> adm) {
    if (g.key_mask & kComputerMessageMenu) {
        return;
    }
    Messages::previous();
}

static void show_build_screen(Handle<Admiral> adm) {
    if (adm != g.admiral) {
        return;
    }
    const MiniLine lines[] = {
            text("BUILD SHIPS", false),
            text("", true),
            selectable("", [](Handle<Admiral> adm) { build_ship(adm, 0); }),
            selectable("", [](Handle<Admiral> adm) { build_ship(adm, 1); }),
            selectable("", [](Handle<Admiral> adm) { build_ship(adm, 2); }),
            selectable("", [](Handle<Admiral> adm) { build_ship(adm, 3); }),
            selectable("", [](Handle<Admiral> adm) { build_ship(adm, 4); }),
            selectable("", [](Handle<Admiral> adm) { build_ship(adm, 5); }),
    };
    make_mini_screen(Screen::BUILD, lines, accept("Build"), cancel("Main Menu"));
    MiniComputerSetBuildStrings();
}

static void show_special_screen(Handle<Admiral> adm) {
    if (adm != g.admiral) {
        return;
    }
    const MiniLine lines[] = {
            text("SPECIAL ORDERS", true),
            selectable("Transfer Control", transfer_control),
            selectable("Hold Position", hold_position),
            selectable("Go To My Position", come_to_me),
            selectable("Fire Weapon 1", fire_pulse),
            selectable("Fire Weapon 2", fire_beam),
            selectable("Fire Special", fire_special),
    };
    make_mini_screen(Screen::SPECIAL, lines, accept("Execute"), cancel("Main Menu"));
}

static void show_message_screen(Handle<Admiral> adm) {
    if (adm != g.admiral) {
        return;
    }
    const MiniLine lines[] = {
            text("MESSAGES", true),
            selectable("Next Page/Clear", next_message),
            selectable("Previous Page", prev_message),
            selectable("Last Message", last_message),
    };
    make_mini_screen(Screen::MESSAGE, lines, accept("Execute"), cancel("Main Menu"));
}

static void show_status_screen(Handle<Admiral> adm) {
    if (adm != g.admiral) {
        return;
    }
    const MiniLine lines[] = {
            text("MISSION STATUS", true),
    };
    make_mini_screen(Screen::STATUS, lines, no_button(), cancel("Main Menu"));
    MiniComputerSetStatusStrings();
}

static void show_main_screen(Handle<Admiral> adm) {
    if (adm != g.admiral) {
        return;
    }
    const MiniLine lines[] = {
            text("MAIN MENU", true),
            selectable("<Build>", show_build_screen),
            selectable("<Special Orders>", show_special_screen),
            selectable("<Message>", show_message_screen),
            selectable("<Mission Status>", show_status_screen),
    };
    make_mini_screen(Screen::MAIN, lines, accept("Select"), no_button());
}

void MiniComputerDoCancel() { show_main_screen(g.admiral); }

void MiniComputerSetBuildStrings() {
    // sets the ship type strings for the build screen
    // also sets up the values = base object num
    if (g.mini.currentScreen != Screen::BUILD) {
        return;
    }

    // Clear header, selection, and all build entries.
    MiniLine* header  = &g.mini.lines[kBuildScreenWhereNameLine];
    header->value     = -1;
    g.mini.selectLine = kMiniScreenNoLineSelected;
    for (int32_t count = 0; count < kMaxShipCanBuild; count++) {
        MiniLine* line = &g.mini.lines[kBuildScreenFirstTypeLine + count];
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
        int32_t           lineNum     = kBuildScreenFirstTypeLine + count;
        MiniLine*         line        = &g.mini.lines[lineNum];
        const BaseObject* buildObject = nullptr;
        if (count < buildAtObject->canBuildType.size()) {
            buildObject =
                    get_buildable_object(buildAtObject->canBuildType[count], g.admiral->race());
        }
        line->value      = -1;
        line->sourceData = buildObject;
        if (!buildObject) {
            continue;
        }

        mCopyBlankLineString(line, buildObject->long_name);
        if (buildObject->price > g.admiral->cash()) {
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

Cash MiniComputerGetPriceOfCurrentSelection() {
    if ((g.mini.currentScreen != Screen::BUILD) ||
        (g.mini.selectLine == kMiniScreenNoLineSelected)) {
        return Cash{Fixed::zero()};
    }

    MiniLine* line        = &g.mini.lines[g.mini.selectLine];
    auto      buildObject = line->sourceData;
    if (!buildObject || (buildObject->price < Cash{Fixed::zero()})) {
        return Cash{Fixed::zero()};
    }

    return buildObject->price;
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

    for (int count = kStatusMiniScreenFirstLine; count < kMiniScreenCharHeight; count++) {
        MiniLine* line = g.mini.lines.get() + count;
        if (implicit_cast<size_t>(count - kStatusMiniScreenFirstLine) >=
            g.level->base.status.size()) {
            line->statusType = kNoStatusData;
            line->value      = -1;
            line->string.clear();
            continue;
        }

        // we have some data for this line to interpret
        const LevelBase::StatusLine& l =
                g.level->base.status.at(count - kStatusMiniScreenFirstLine);

        line->underline = l.underline.value_or(false);
        if (l.text.has_value()) {
            line->statusType = kPlainTextStatus;
            line->value      = 0;
            line->string     = l.text->copy();
            continue;
        }

        line->statusString = l.prefix.has_value() ? l.prefix->copy() : pn::string{};
        line->postString   = l.suffix.has_value() ? l.suffix->copy() : pn::string{};
        if (l.condition.has_value()) {
            line->statusType  = kTrueFalseCondition;
            line->condition   = *l.condition;
            line->statusTrue  = l.true_.has_value() ? l.true_->copy() : pn::string{};
            line->statusFalse = l.false_.has_value() ? l.false_->copy() : pn::string{};
        } else if (l.counter.has_value()) {
            line->counter = *l.counter;
            if (l.fixed.value_or(false)) {
                if (l.minuend.has_value()) {
                    line->statusType    = kSmallFixedMinusValue;
                    line->negativeValue = l.minuend->val();
                } else {
                    line->statusType = kSmallFixedValue;
                }
            } else {
                if (l.minuend.has_value()) {
                    line->statusType    = kIntegerMinusValue;
                    line->negativeValue = mFixedToLong(*l.minuend);
                } else {
                    line->statusType = kIntegerValue;
                }
            }
        } else {
            line->statusType = kPlainTextStatus;
            line->value      = 0;
            line->string.clear();
            continue;
        }

        line->value = MiniComputerGetStatusValue(count);
        MiniComputerMakeStatusString(count, line->string);
    }
}

void MiniComputerMakeStatusString(int32_t which_line, pn::string& string) {
    string.clear();

    const MiniLine& line = g.mini.lines[which_line];
    if (line.statusType == kNoStatusData) {
        return;
    }

    string += line.statusString;
    switch (line.statusType) {
        case kTrueFalseCondition:
            if (line.value == 1) {
                string += line.statusTrue;
            } else {
                string += line.statusFalse;
            }
            break;

        case kIntegerValue:
        case kIntegerMinusValue: string += pn::dump(line.value, pn::dump_short); break;

        case kSmallFixedValue:
        case kSmallFixedMinusValue: string += stringify(Fixed::from_val(line.value)); break;
    }
    if (line.statusType != kPlainTextStatus) {
        string += line.postString;
    }
}

int32_t MiniComputerGetStatusValue(int32_t whichLine) {
    MiniLine* line = g.mini.lines.get() + whichLine;

    if (line->statusType == kNoStatusData) {
        return -1;
    }

    switch (line->statusType) {
        case kPlainTextStatus: return 0; break;

        case kTrueFalseCondition:
            if (g.condition_enabled[line->condition.number()]) {
                return 0;
            } else {
                return 1;
            }
            break;

        case kIntegerValue:
        case kSmallFixedValue: return GetAdmiralScore(line->counter); break;

        case kIntegerMinusValue:
        case kSmallFixedMinusValue:
            return line->negativeValue - GetAdmiralScore(line->counter);
            break;

        default: return 0; break;
    }
}

void MiniComputerHandleClick(Point where) {
    // if click is in button screen
    if (Rect(kButBoxLeft, kButBoxTop + instrument_top(), kButBoxRight,
             kButBoxBottom + instrument_top())
                .contains(where)) {
        int lineNum = ((where.v - (kButBoxTop + instrument_top())) / sys.fonts.computer.height);
        g.mini.clickLine       = lineNum + kMiniScreenCharHeight;
        MiniButton* button     = (lineNum == 0) ? g.mini.accept.get() : g.mini.cancel.get();
        MiniButton* off_button = (lineNum == 0) ? g.mini.cancel.get() : g.mini.accept.get();
        if (button->kind) {
            if (button->kind != MINI_BUTTON_ON) {
                button->kind = MINI_BUTTON_ON;
                sys.sound.click();
            }
            if (off_button->kind) {
                off_button->kind = MINI_BUTTON_OFF;
            }
        }
    } else {
        // make sure both buttons are off
        if (g.mini.accept->kind) {
            g.mini.accept->kind = MINI_BUTTON_OFF;
        }
        if (g.mini.cancel->kind) {
            g.mini.cancel->kind = MINI_BUTTON_OFF;
        }

        // if click is in main menu screen
        if (Rect(kMiniScreenLeft, kMiniScreenTop + instrument_top(), kMiniScreenRight,
                 kMiniScreenBottom + instrument_top())
                    .contains(where)) {
            int lineNum      = mGetLineNumFromV(where.v);
            g.mini.clickLine = lineNum;
            MiniLine* line   = g.mini.lines.get() + lineNum;
            if ((line->kind == MINI_SELECTABLE) || (line->kind == MINI_DIM)) {
                g.mini.selectLine = lineNum;
            }
        } else {
            g.mini.clickLine = kMiniScreenNoLineSelected;
        }
    }
}

void MiniComputerHandleDoubleClick(Point where) {
    // if click is in button screen
    if (Rect(kButBoxLeft, kButBoxTop + instrument_top(), kButBoxRight,
             kButBoxBottom + instrument_top())
                .contains(where)) {
        int lineNum = ((where.v - (kButBoxTop + instrument_top())) / sys.fonts.computer.height);
        MiniButton* button     = (lineNum == 0) ? g.mini.accept.get() : g.mini.cancel.get();
        MiniButton* off_button = (lineNum == 0) ? g.mini.cancel.get() : g.mini.accept.get();
        if (button->kind) {
            if (button->kind != MINI_BUTTON_ON) {
                button->kind = MINI_BUTTON_ON;
                sys.sound.click();
            }
            if (off_button->kind) {
                off_button->kind = MINI_BUTTON_OFF;
            }
        }
    } else {
        // make sure both buttons are off
        if (g.mini.accept->kind) {
            g.mini.accept->kind = MINI_BUTTON_OFF;
        }
        if (g.mini.cancel->kind) {
            g.mini.cancel->kind = MINI_BUTTON_OFF;
        }

        // if click is in main menu screen
        if (Rect(kMiniScreenLeft, kMiniScreenTop + instrument_top(), kMiniScreenRight,
                 kMiniScreenBottom + instrument_top())
                    .contains(where)) {
            int lineNum = mGetLineNumFromV(where.v);
            if (lineNum == g.mini.selectLine) {
                sys.sound.click();
                MiniComputerDoAccept();
            } else {
                lineNum        = mGetLineNumFromV(where.v);
                MiniLine* line = g.mini.lines.get() + lineNum;
                if ((line->kind == MINI_SELECTABLE) || (line->kind == MINI_DIM)) {
                    g.mini.selectLine = lineNum;

                    line = g.mini.lines.get() + g.mini.selectLine;
                }
            }
        }
    }
}

void MiniComputerHandleMouseUp(Point where) {
    // if click is in button screen
    if (Rect(kButBoxLeft, kButBoxTop + instrument_top(), kButBoxRight,
             kButBoxBottom + instrument_top())
                .contains(where)) {
        int32_t lineNum =
                ((where.v - (kButBoxTop + instrument_top())) / sys.fonts.computer.height);
        MiniButton* button = (lineNum == 0) ? g.mini.accept.get() : g.mini.cancel.get();
        if (button->kind) {
            if (button->kind == MINI_BUTTON_ON) {
                button->kind = MINI_BUTTON_OFF;
                if (lineNum == 0) {
                    MiniComputerDoAccept();
                } else {
                    MiniComputerDoCancel();
                }
            }
        }
    }
}

void MiniComputerHandleMouseStillDown(Point where) {
    // if click is in button screen
    if (Rect(kButBoxLeft, kButBoxTop + instrument_top(), kButBoxRight,
             kButBoxBottom + instrument_top())
                .contains(where)) {
        int lineNum = ((where.v - (kButBoxTop + instrument_top())) / sys.fonts.computer.height);
        MiniButton* button = (lineNum == 0) ? g.mini.accept.get() : g.mini.cancel.get();
        if (button->kind && ((lineNum + kMiniScreenCharHeight) == g.mini.clickLine)) {
            button->kind = MINI_BUTTON_ON;
            return;
        }
    }

    if (g.mini.accept->kind) {
        g.mini.accept->kind = MINI_BUTTON_OFF;
    }
    if (g.mini.cancel->kind) {
        g.mini.cancel->kind = MINI_BUTTON_OFF;
    }
}

// for ambrosia tutorial, a horrific hack
void MiniComputer_SetScreenAndLineHack(Screen whichScreen, int32_t whichLine) {
    Point w;

    switch (whichScreen) {
        case Screen::BUILD: show_build_screen(g.admiral); break;
        case Screen::SPECIAL: show_special_screen(g.admiral); break;
        case Screen::MESSAGE: show_message_screen(g.admiral); break;
        case Screen::STATUS: show_status_screen(g.admiral); break;
        default: show_main_screen(g.admiral); break;
    }

    w.v = (whichLine * sys.fonts.computer.height) + (kMiniScreenTop + instrument_top());
    w.h = kMiniScreenLeft + 5;
    MiniComputerHandleClick(w);  // what an atrocious hack! oh well
}

}  // namespace antares
