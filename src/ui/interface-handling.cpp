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

//
// Liaison between the application and Interface Drawing.  Takes in events ( key events, mouse
// down events), hilights and scrolls as needed, and returns results.  Also handles editable text.
//

#include "ui/interface-handling.hpp"

#include <vector>
#include <sfz/sfz.hpp>

#include "config/keys.hpp"
#include "config/preferences.hpp"
#include "data/interface.hpp"
#include "data/picture.hpp"
#include "data/races.hpp"
#include "data/resource.hpp"
#include "data/string-list.hpp"
#include "drawing/briefing.hpp"
#include "drawing/color.hpp"
#include "drawing/interface.hpp"
#include "drawing/offscreen-gworld.hpp"
#include "drawing/pix-map.hpp"
#include "drawing/pix-table.hpp"
#include "drawing/text.hpp"
#include "game/globals.hpp"
#include "game/instruments.hpp"
#include "game/main.hpp"
#include "game/messages.hpp"
#include "game/scenario-maker.hpp"
#include "game/space-object.hpp"
#include "game/starfield.hpp"
#include "math/fixed.hpp"
#include "math/special.hpp"
#include "sound/fx.hpp"
#include "sound/music.hpp"
#include "ui/interface-handling.hpp"
#include "video/driver.hpp"
#include "video/transitions.hpp"

using sfz::BytesSlice;
using sfz::Exception;
using sfz::PrintItem;
using sfz::ReadSource;
using sfz::String;
using sfz::StringSlice;
using sfz::read;
using sfz::scoped_array;
using sfz::scoped_ptr;
using std::vector;

namespace macroman = sfz::macroman;

namespace antares {

namespace {

const int32_t kTargetScreenWidth = 640;
const int32_t kTargetScreenHeight = 480;

inline void mPlayButtonDown() {
    PlayVolumeSound(kComputerBeep1, kMediumLoudVolume, kShortPersistence, kMustPlaySound);
}
inline void mPlayButtonUp() {
    PlayVolumeSound(kComputerBeep2, kMediumLowVolume, kShortPersistence, kMustPlaySound);
}
inline void mPlayScreenSound() {
    PlayVolumeSound(kComputerBeep3, kMediumLowVolume, kShortPersistence, kVeryLowPrioritySound);
}

const int32_t kMissionDataWidth         = 200;
const int32_t kMissionDataVBuffer       = 40;
const int32_t kMissionDataTopBuffer     = 30;
const int32_t kMissionDataBottomBuffer  = 15;
const int32_t kMissionDataHBuffer       = 41;
const int32_t kMissionDataHiliteColor   = GOLD;
const int32_t kMissionLineHJog          = 10;
const int32_t kMissionBriefPointOffset  = 2;

const int16_t kLoadingScreenID      = 6001;
const uint8_t kLoadingScreenColor   = PALE_GREEN;

const int16_t kShipDataTextID       = 6001;
const int16_t kShipDataKeyStringID  = 6001;
const int16_t kShipDataNameID       = 6002;
const int16_t kWeaponDataTextID     = 6003;

enum {
    kShipOrObjectStringNum      = 0,
    kShipTypeStringNum          = 1,
    kMassStringNum              = 2,
    kShieldStringNum            = 3,
    kHasLightStringNum          = 4,
    kMaxSpeedStringNum          = 5,
    kThrustStringNum            = 6,
    kTurnStringNum              = 7,
    kWeaponNumberStringNum      = 8,
    kWeaponNameStringNum        = 9,
    kWeaponGuidedStringNum      = 10,
    kWeaponRangeStringNum       = 11,
    kWeaponDamageStringNum      = 12,
    kWeaponAutoTargetStringNum  = 13,
};

enum {
    kShipDataDashStringNum      = 2,
    kShipDataYesStringNum       = 3,
    kShipDataNoStringNum        = 4,
    kShipDataPulseStringNum     = 5,
    kShipDataBeamStringNum      = 6,
    kShipDataSpecialStringNum   = 7,
};

const int32_t kShipDataWidth = 240;

const int16_t kHelpScreenKeyStringID = 6003;

inline void mDoubleDigitize(unsigned char* mstring) {
    if (mstring[0] == 1) {
        mstring[0] = 2;
        mstring[2] = mstring[1];
        mstring[1] = '0';
    }
}

int find_replace(String& data, int pos, const StringSlice& search, const PrintItem& replace) {
    size_t at = data.find(search, pos);
    if (at != String::npos) {
        String replace_string;
        replace.print_to(replace_string);
        data.replace(at, search.size(), replace_string);
    }
    return at;
}

}  // namespace

std::vector<interfaceItemType> gInterfaceItemData;
uint32_t gInterfaceScreenHBuffer = 0;
uint32_t gInterfaceScreenVBuffer = 0;

void InterfaceHandlingInit() {
    gInterfaceScreenHBuffer = (world.width() / 2) - (kTargetScreenWidth / 2);
    gInterfaceScreenVBuffer = (world.height() / 2) - (kTargetScreenHeight / 2);
}

void InterfaceHandlingCleanup() {
    gInterfaceItemData.clear();
}

void OpenInterface(short resID) {
    Resource rsrc("interfaces", "intr", resID);
    BytesSlice in(rsrc.data());

    gInterfaceItemData.clear();
    while (!in.empty()) {
        gInterfaceItemData.push_back(interfaceItemType());
        interfaceItemType& item = gInterfaceItemData.back();
        read(in, item);
        item.bounds.left += gInterfaceScreenHBuffer;
        item.bounds.right += gInterfaceScreenHBuffer;
        item.bounds.top += gInterfaceScreenVBuffer;
        item.bounds.bottom += gInterfaceScreenVBuffer;
    }

    InvalidateInterfaceFunctions(); // if they've been set, they shouldn't be yet
}

long AppendInterface(short resID, long relative_number, bool center) {
    if (relative_number < 0) {
        throw Exception("interfaces must be appended within existing elements");
    }
    const int32_t original_number = gInterfaceItemData.size();

    Resource rsrc("interfaces", "intr", resID);
    BytesSlice in(rsrc.data());
    while (!in.empty()) {
        gInterfaceItemData.push_back(interfaceItemType());
        read(in, gInterfaceItemData.back());
    }

    Rect bounds = gInterfaceItemData[relative_number].bounds;
    if (center) {
        CenterItemRangeInRect(&bounds, original_number, gInterfaceItemData.size());
    } else {
        for (size_t i = original_number; i < gInterfaceItemData.size(); ++i) {
            interfaceItemType* const item = &gInterfaceItemData[i];
            item->bounds.left += bounds.left;
            item->bounds.right += bounds.left;
            item->bounds.top += bounds.top;
            item->bounds.bottom += bounds.top;
        }
    }

    return gInterfaceItemData.size() - original_number;
}

void ShortenInterface(long howMany) {
    gInterfaceItemData.resize(gInterfaceItemData.size() - howMany);
}

void CloseInterface() {
    gInterfaceItemData.clear();
}

void DrawEntireInterface() {
    Rect tRect = world;
    gOffWorld->view(tRect).fill(RgbColor::kBlack);

    for (size_t i = 0; i < gInterfaceItemData.size(); ++i) {
        DrawAnyInterfaceItem(gInterfaceItemData[i], gOffWorld);
    }
    copy_world(*gRealWorld, *gOffWorld, tRect);
}

void DrawInterfaceRange(long from, long to, long withinItem) {
    interfaceItemType* const item = GetAnyInterfaceItemPtr(withinItem);
    Rect tRect = item->bounds;
    if (withinItem >= 0) {
        gOffWorld->view(tRect).fill(RgbColor::kBlack);
    }
    from = std::max<int>(from, 0);
    to = std::min<int>(to, gInterfaceItemData.size());
    if (from < to) {
        for (int i = from; i < to; ++i) {
            DrawAnyInterfaceItem(gInterfaceItemData[i], gOffWorld);
        }
        if (withinItem >= 0) {
            copy_world(*gRealWorld, *gOffWorld, tRect);
        }
    }
}

void DrawAllItemsOfKind(interfaceKindType kind, bool sound, bool clearFirst, bool showAtEnd) {
    Rect tRect = world;
    if (clearFirst) {
        gOffWorld->view(tRect).fill(RgbColor::kBlack);
    }

    for (size_t i = 0; i < gInterfaceItemData.size(); ++i) {
        const interfaceItemType& item = gInterfaceItemData[i];
        if (sound) {
            mPlayScreenSound();
        }
        if (item.kind == kind) {
            if (showAtEnd) {
                DrawAnyInterfaceItem(item, gOffWorld);
            } else {
                DrawAnyInterfaceItemOffToOn(item);
            }
        }
    }
    if (showAtEnd) {
        copy_world(*gRealWorld, *gOffWorld, tRect);
    }
}

void DrawAnyInterfaceItemOffToOn(const interfaceItemType& item) {
    Rect bounds;

    GetAnyInterfaceItemGraphicBounds(item, &bounds);
    DrawAnyInterfaceItem(item, gOffWorld);
    copy_world(*gRealWorld, *gOffWorld, bounds);
}

void OffsetAllItems(long hoffset, long voffset) {
    OffsetItemRange(hoffset, voffset, 0, gInterfaceItemData.size());
}

void OffsetItemRange(long hoffset, long voffset, long from, long to) {
    from = std::max<int>(from, 0);
    to = std::min<int>(to, gInterfaceItemData.size());
    for (int i = from; i < to; ++i) {
        interfaceItemType* const item = &gInterfaceItemData[i];
        item->bounds.left += hoffset;
        item->bounds.right += hoffset;
        item->bounds.top += voffset;
        item->bounds.bottom += voffset;
    }
}

void CenterAllItemsInRect(Rect* destRect) {
    CenterItemRangeInRect(destRect, 0, gInterfaceItemData.size());
}

void CenterItemRangeInRect(Rect* dest, long from, long to) {
    from = std::max<int>(from, 0);
    to = std::min<int>(to, gInterfaceItemData.size());
    if (to <= from) {
        return;
    }

    Rect src = gInterfaceItemData[from].bounds;

    // first calc the rect that encloses all the interface items
    for (int i = from; i < to; ++i) {
        src.enlarge_to(gInterfaceItemData[i].bounds);
    }

    int32_t offset_x = ((dest->left - src.left) + (dest->right - src.right)) / 2;
    int32_t offset_y = ((dest->top - src.top) + (dest->bottom - src.bottom)) / 2;
    OffsetItemRange(offset_x, offset_y, from, to);
}

void InvalidateInterfaceFunctions() {
    for (size_t i = 0; i < gInterfaceItemData.size(); ++i) {
        interfaceItemType* const item = &gInterfaceItemData[i];
        if (item->kind == kListRect) {
            item->item.listRect.getListLength = NULL;
            item->item.listRect.getItemString = NULL;
            item->item.listRect.itemHilited = NULL;
        }
    }
}

short PtInInterfaceItem(Point where) {
    for (size_t i = 0; i < gInterfaceItemData.size(); ++i) {
        const interfaceItemType& item = gInterfaceItemData[i];
        Rect tRect;
        GetAnyInterfaceItemGraphicBounds(item, &tRect);

        if (tRect.contains(where)) {
            if ((item.kind != kTabBox) && (item.kind != kPictureRect)) {
                return i;
            }
        }
    }

    return -1;
}

short InterfaceMouseDown(Point where) {
    for (size_t i = 0; i < gInterfaceItemData.size(); ++i) {
        interfaceItemType* const item = &gInterfaceItemData[i];
        Rect tRect;
        GetAnyInterfaceItemGraphicBounds(*item, &tRect);

        if (tRect.contains(where)) {
            switch (item->kind) {
              case kPlainButton:
                if (InterfaceButtonHit(item)) {
                    return i;
                } else {
                    return -1;
                }

              case kCheckboxButton:
                if (InterfaceCheckboxHit(item)) {
                    return i;
                } else {
                    return -1;
                }

              case kRadioButton:
                if (InterfaceRadioButtonHit(item)) {
                    return i;
                } else {
                    return -1;
                }

              case kTabBoxButton:
                if (InterfaceTabBoxButtonHit(item)) {
                    return i;
                } else {
                    return -1;
                }

              case kLabeledRect:
                return -1;

              case kListRect:
                InterfaceListRectHit(*item, where);
                return i;

              default:
                break;
            }
        }
    }
    return -1;
}

short InterfaceKeyDown(long message) {
    int32_t key_code = message & 0xFF00;
    key_code >>= 8;
    key_code += 1;
    if (key_code > 0) {
        interfaceItemType* hit = NULL;
        size_t hit_number = 0;
        int button_key = 0;
        // check plain buttons
        for (size_t i = 0; i < gInterfaceItemData.size(); ++i) {
            interfaceItemType* const item = &gInterfaceItemData[i];
            switch(item->kind) {
              case kPlainButton:
                if (item->item.plainButton.status != kDimmed) {
                    button_key = item->item.plainButton.key;
                }
                break;

              case kTabBoxButton:
                if (item->item.radioButton.status != kDimmed) {
                    button_key = item->item.radioButton.key;
                }
                break;

              default:
                button_key = 0;
                break;
            }
            if (key_code == button_key) {
                hit = item;
                hit_number = i;
                break;
            }
        }

        if (hit) {
            SetStatusOfAnyInterfaceItem(hit_number, kIH_Hilite, false);
            DrawAnyInterfaceItemOffToOn(*hit);
            mPlayButtonDown();
            KeyMap key_map;
            do {
                VideoDriver::driver()->get_keys(&key_map);
            } while (GetKeyNumFromKeyMap(key_map) == button_key);
            SetStatusOfAnyInterfaceItem(hit_number, kActive, false);

            switch (hit->kind) {
              case kTabBoxButton:
                hit->item.radioButton.on = true;
                break;

              default:
                break;
            }

            DrawAnyInterfaceItemOffToOn(*hit);
            return hit_number;
        }
    }
    return -1;
}

bool InterfaceButtonHit(interfaceItemType* button) {
    if (button->item.plainButton.status == kDimmed) {
        return false;
    }

    Rect tRect;
    GetAnyInterfaceItemGraphicBounds(*button, &tRect);

    Point where;
    while (VideoDriver::driver()->button()) {
        where = VideoDriver::driver()->get_mouse();
        if (tRect.contains(where)) {
            if (button->item.plainButton.status != kIH_Hilite) {
                mPlayButtonDown();
                button->item.plainButton.status = kIH_Hilite;
                DrawAnyInterfaceItemOffToOn(*button);
            }
        } else {
            if (button->item.plainButton.status != kActive) {
                mPlayButtonUp();
                button->item.plainButton.status = kActive;
                DrawAnyInterfaceItemOffToOn(*button);
            }
        }
    }
    if (button->item.plainButton.status == kIH_Hilite) {
        button->item.plainButton.status = kActive;
        DrawAnyInterfaceItemOffToOn(*button);
    }
    return tRect.contains(where);
}

bool InterfaceCheckboxHit(interfaceItemType* button) {
    if (button->item.checkboxButton.status == kDimmed) {
        return false;
    }

    Rect tRect;
    GetAnyInterfaceItemGraphicBounds(*button, &tRect);

    Point where;
    do {
        where = VideoDriver::driver()->get_mouse();
        if (tRect.contains(where)) {
            if (button->item.checkboxButton.status != kIH_Hilite) {
                mPlayButtonDown();
                button->item.checkboxButton.status = kIH_Hilite;
                DrawAnyInterfaceItemOffToOn(*button);
            }
        } else {
            if (button->item.checkboxButton.status != kActive) {
                button->item.checkboxButton.status = kActive;
                DrawAnyInterfaceItemOffToOn(*button);
            }
        }
    } while (VideoDriver::driver()->button());
    if ( button->item.checkboxButton.status == kIH_Hilite) {
        button->item.checkboxButton.status = kActive;
    }
    where = VideoDriver::driver()->get_mouse();
    if (tRect.contains(where)) {
        if ( button->item.checkboxButton.on) {
            button->item.checkboxButton.on = false;
        } else {
            button->item.checkboxButton.on = true;
        }
        DrawAnyInterfaceItemOffToOn(*button);
        return true;
    } else {
        DrawAnyInterfaceItemOffToOn(*button);
        return false;
    }
}

bool InterfaceRadioButtonHit(interfaceItemType* button) {
    Rect tRect;
    GetAnyInterfaceItemGraphicBounds(*button, &tRect);

    if (button->item.radioButton.status == kDimmed) {
        return( false);
    }

    if (button->item.radioButton.on == false) {
        button->item.radioButton.on = true;
    }

    Point where;
    do {
        where = VideoDriver::driver()->get_mouse();
        if (tRect.contains(where)) {
            if (button->item.radioButton.status != kIH_Hilite) {
                mPlayButtonDown();
                button->item.radioButton.status = kIH_Hilite;
                DrawAnyInterfaceItemOffToOn(*button);
            }
        } else {
            if (button->item.radioButton.status != kActive) {
                button->item.radioButton.status = kActive;
                DrawAnyInterfaceItemOffToOn(*button);
            }
        }
    } while (VideoDriver::driver()->button());
    if (button->item.radioButton.status == kIH_Hilite) {
        button->item.radioButton.status = kActive;
    }
    DrawAnyInterfaceItemOffToOn(*button);
    return true;
}

bool InterfaceTabBoxButtonHit(interfaceItemType* button) {
    Rect tRect;
    GetAnyInterfaceItemGraphicBounds(*button, &tRect);

    if (button->item.radioButton.status == kDimmed) {
        return false;
    }

    if (button->item.radioButton.on != false) {
        return false;
    }

    Point where;
    do {
        where = VideoDriver::driver()->get_mouse();
        if (tRect.contains(where)) {
            if (button->item.radioButton.status != kIH_Hilite) {
                mPlayButtonDown();
                button->item.radioButton.status = kIH_Hilite;
                DrawAnyInterfaceItemOffToOn(*button);
            }
        } else {
            if (button->item.radioButton.status != kActive) {
                button->item.radioButton.status = kActive;
                DrawAnyInterfaceItemOffToOn(*button);
            }
        }
    } while ( VideoDriver::driver()->button());
    if ( button->item.radioButton.status == kIH_Hilite) {
        button->item.radioButton.status = kActive;
    }
    button->item.radioButton.on = true;
    DrawAnyInterfaceItemOffToOn(*button);
    return true;
}

void InterfaceListRectHit(const interfaceItemType& listRect, Point where) {
    Rect    tRect;
    short   lineHeight, whichHit;

    if (listRect.item.listRect.getListLength != NULL) {
        tRect = listRect.bounds;
        lineHeight = GetInterfaceFontHeight(listRect.style) + kInterfaceTextVBuffer;
        where.v -= tRect.top;
        whichHit = where.v / lineHeight + listRect.item.listRect.topItem;
        if ( whichHit >= (*(listRect.item.listRect.getListLength))())
            whichHit = -1;
        (*(listRect.item.listRect.itemHilited))( whichHit, true);
    }
}

interfaceItemType *GetAnyInterfaceItemPtr(long whichItem) {
    return &gInterfaceItemData[whichItem];
}

void SetStatusOfAnyInterfaceItem(short whichItem, interfaceItemStatusType status, bool drawNow) {
    interfaceItemType* const item = &gInterfaceItemData[whichItem];

    switch (item->kind) {
      case kPlainButton:
        item->item.plainButton.status = status;
        break;

      case kRadioButton:
      case kTabBoxButton:
        item->item.radioButton.status = status;
        break;

      case kCheckboxButton:
        item->item.checkboxButton.status = status;
        break;

      case kTextRect:
        item->item.textRect.visibleBounds = (status == kActive);
        break;

      case kPictureRect:
        item->item.pictureRect.visibleBounds = (status == kActive);
        break;
    }
    if (drawNow) {
        RefreshInterfaceItem(whichItem);
    }
}


void SwitchAnyRadioOrCheckbox(short whichItem, bool turnOn) {
    interfaceItemType* const item = &gInterfaceItemData[whichItem];
    if (item->kind == kCheckboxButton) {
        item->item.checkboxButton.on = turnOn;
    } else if ((item->kind == kRadioButton) || (item->kind == kTabBoxButton)) {
        item->item.radioButton.on = turnOn;
    }

}

bool GetAnyRadioOrCheckboxOn(short whichItem) {
    interfaceItemType* const item = &gInterfaceItemData[whichItem];
    if (item->kind == kCheckboxButton) {
        return item->item.checkboxButton.on;
    } else if (item->kind == kRadioButton) {
        return item->item.radioButton.on;
    } else {
        return false;
    }
}


void RefreshInterfaceItem(short whichItem) {
    const interfaceItemType& item = gInterfaceItemData[whichItem];
    Rect tRect;
    GetAnyInterfaceItemGraphicBounds(item, &tRect);
    gOffWorld->view(tRect).fill(RgbColor::kBlack);
    DrawAnyInterfaceItemOffToOn(item);
}

void SetButtonKeyNum(short whichItem, short whichKey) {
    interfaceItemType* const item = &gInterfaceItemData[whichItem];
    if (item->kind == kPlainButton) {
        item->item.plainButton.key = whichKey;
    }
}

short GetButtonKeyNum(short whichItem) {
    interfaceItemType* item = &gInterfaceItemData[whichItem];
    if (item->kind == kPlainButton) {
        return item->item.plainButton.key;
    } else {
        return 0;
    }
}

void SetInterfaceTextBoxText( short resID) {
    static_cast<void>(resID);
}

void BlackenWindow();

void DoLoadingInterface(Rect *contentRect, StringSlice level_name) {
    RgbColor color;
    Rect                lRect, clipRect, boundsRect;
    Rect                    tRect;
    retroTextSpecType       retroTextSpec;
    long                    height;

    BlackenWindow();

    OpenInterface( kLoadingScreenID);
    DrawEntireInterface();

    GetAnyInterfaceItemContentBounds(*GetAnyInterfaceItemPtr(0), contentRect); // item 0 = loading rect
    CloseInterface();

// it is assumed that we're "recovering" from a fade-out
    // AutoFadeFrom( 10, false);

    mSetDirectFont( kTitleFontNum);
    color = GetRGBTranslateColorShade(PALE_GREEN, LIGHT);
    lRect = world;
//      MoveTo( contentRect->left + (( contentRect->right - contentRect->left) / 2) - (stringWidth / 2),
//              contentRect->top);

    retroTextSpec.textLength = level_name.size();
    retroTextSpec.text.reset(new String(level_name));

    retroTextSpec.thisPosition = retroTextSpec.linePosition = retroTextSpec.lineCount = 0;
    retroTextSpec.tabSize =220;
    retroTextSpec.color = GetRGBTranslateColorShade(PALE_GREEN, VERY_LIGHT);
    retroTextSpec.backColor = GetRGBTranslateColorShade(SKY_BLUE, DARKEST);
    retroTextSpec.backColor = RgbColor::kBlack;
    retroTextSpec.originalColor = retroTextSpec.nextColor = retroTextSpec.color;
    retroTextSpec.originalBackColor = retroTextSpec.nextBackColor = retroTextSpec.backColor;

    retroTextSpec.topBuffer = 2;
    retroTextSpec.bottomBuffer = 0;
    height = DetermineDirectTextHeightInWidth( &retroTextSpec, kSmallScreenWidth);

    boundsRect.left = (world.width() / 2) - ( retroTextSpec.autoWidth / 2);
    boundsRect.right = boundsRect.left + retroTextSpec.autoWidth;
    boundsRect.top = (contentRect->top / 2) - ( retroTextSpec.autoHeight / 2);
    boundsRect.bottom = boundsRect.top + retroTextSpec.autoHeight;
    retroTextSpec.xpos = boundsRect.left;
    retroTextSpec.ypos = boundsRect.top + mDirectFontAscent();

    clipRect.left = 0;
    clipRect.right = clipRect.left + world.width();
    clipRect.top = 0;
    clipRect.bottom = clipRect.top + world.height();
    while ( retroTextSpec.thisPosition < retroTextSpec.textLength)
    {
        PlayVolumeSound(  kTeletype, kMediumLowVolume, kShortPersistence, kLowPrioritySound);
        DrawRetroTextCharInRect( &retroTextSpec, 3, boundsRect, clipRect, gRealWorld);
    }

    retroTextSpec.text.reset();

    tRect = boundsRect;
}

void UpdateLoadingInterface( long value, long total, Rect *contentRect)

{
    RgbColor        color;
    long            width, height, temp;
    Rect        clipRect;
    Rect            tRect;

    VideoDriver::driver()->button();  // Hack to get it to update.
    if ( total < 0)
    {
        gOffWorld->view(*contentRect).fill(RgbColor::kBlack);
        StringList strings(2004);
        StringSlice text = strings.at(32);

        mSetDirectFont( kButtonFontNum);
        mGetDirectStringDimensions(text, width, height);

        clipRect = *contentRect;
        tRect = Rect(0, 0, width, height);
        tRect.center_in(*contentRect);

        color = GetRGBTranslateColorShade(kLoadingScreenColor, LIGHTER);
        DrawDirectTextStringClipped(
                Point(tRect.left, tRect.top + mDirectFontAscent()), text, color, gOffWorld,
                clipRect);

        copy_world(*gRealWorld, *gOffWorld, *contentRect);
    } else {
        width = contentRect->right - contentRect->left;

        temp = (value * width);
        temp /= total;

        tRect = Rect(contentRect->left, contentRect->top, contentRect->left + temp, contentRect->bottom);
        color = GetRGBTranslateColorShade(kLoadingScreenColor, LIGHT);
        gOffWorld->view(tRect).fill(color);

        tRect = Rect(contentRect->left + temp, contentRect->top, contentRect->right, contentRect->bottom);
        color = GetRGBTranslateColorShade(kLoadingScreenColor, DARK);
        gOffWorld->view(tRect).fill(color);
        tRect = Rect(contentRect->left, contentRect->top, contentRect->right, contentRect->bottom);
        copy_world(*gRealWorld, *gOffWorld, tRect);
        if (tRect.left >= tRect.right - 2) {
            // AutoFadeTo( 10, RgbColor::kBlack, false);
        }
    }
}

void DoNetSettings( void)

{
#ifdef NETSPROCKET_AVAILABLE
    int                     error = kNoError;
    Rect                    tRect;
    interfaceItemType       *item;
    bool                 done = false, cancel = false, currentBandwidth = GetBandwidth();
    Point                   where;
    short                   whichItem, result = 0, currentRegistered = 0,
                            currentDelay = 0, i;
    EventRecord             theEvent;
    char                    whichChar;

//  BlackenWindow();

    FlushEvents(everyEvent, 0);
    OpenInterface( kNetSettingsID);
    if ( error == kNoError)
    {
        tRect = viewport;
        CenterAllItemsInRect( &tRect);
        item = GetAnyInterfaceItemPtr( kNetSettingsBox);
        DrawInOffWorld();
        GetAnyInterfaceItemGraphicBounds( item, &tRect);
        copy_world(*gSaveWorld, *gOffWorld, tRect);
        DrawInRealWorld();

        currentRegistered = GetRegisteredSetting();
        currentDelay = GetResendDelay();
        if ( currentDelay <= 60) currentDelay = 0;
        else if ( currentDelay <= 120) currentDelay = 1;
        else if ( currentDelay <= 240) currentDelay = 2;
        else currentDelay = 3;
        for ( i = kNetSettingsFirstRegisteredRadio;
                i <= kNetSettingsLastRegisteredRadio; i++)
        {
            if (( i - kNetSettingsFirstRegisteredRadio) == currentRegistered)
            {
                SwitchAnyRadioOrCheckbox( i, true);
            } else
            {
                SwitchAnyRadioOrCheckbox( i, false);
            }
        }
        for ( i = kNetSettingsFirstDelayRadio;
                i <= kNetSettingsLastDelayRadio; i++)
        {
            if (( i - kNetSettingsFirstDelayRadio) == currentDelay)
            {
                SwitchAnyRadioOrCheckbox( i, true);
            } else
            {
                SwitchAnyRadioOrCheckbox( i, false);
            }
        }

        SwitchAnyRadioOrCheckbox( kNetSettingsLowerBandwidthCheck, currentBandwidth);

        DrawAllItemsOfKind( kPictureRect, false, false, false);
        DrawAllItemsOfKind( kLabeledRect, true, false, false);
        DrawAllItemsOfKind( kPlainRect, true, false, false);
        DrawAllItemsOfKind( kCheckboxButton, true, false, false);
        DrawAllItemsOfKind( kRadioButton, true, false, false);
        DrawAllItemsOfKind( kPlainButton, true, false, false);
        DrawAllItemsOfKind( kTextRect, true, false, false);
        while ( AnyRealKeyDown()) {
            // DO NOTHING
        };

        while (!done) {
            WaitNextEvent (everyEvent, &theEvent, 3, NULL);
            {
                whichItem = -1;
                switch ( theEvent.what )
                {
                    case osEvt:
//                      HandleOSEvent( &theEvent);
                        break;

                    case mouseDown:
                        where = theEvent.where;
                        GlobalToLocal( &where);
                        whichItem = InterfaceMouseDown( where);
                        break;
                    case mouseUp:
                        break;
                    case keyDown:
                    case autoKey:
                        whichChar = theEvent.message & charCodeMask;

                        whichItem = InterfaceKeyDown( theEvent.message);
                        break;
                }
                switch ( whichItem)
                {
                    case kNetSettingsOKButton:
                        done = true;
                        break;

                    case kNetSettingsCancelButton:
                        done = true;
                        cancel = true;
                        break;

                    case kNetSettingsLowerBandwidthCheck:
                        currentBandwidth = !currentBandwidth;
                        break;

                    default:
                        if (( whichItem >= kNetSettingsFirstRegisteredRadio) &&
                            ( whichItem <= kNetSettingsLastRegisteredRadio))
                        {
                            SwitchAnyRadioOrCheckbox( currentRegistered +
                                kNetSettingsFirstRegisteredRadio, false);
                            RefreshInterfaceItem( currentRegistered +
                                kNetSettingsFirstRegisteredRadio);
                            currentRegistered = whichItem - kNetSettingsFirstRegisteredRadio;
                            SwitchAnyRadioOrCheckbox( currentRegistered +
                                kNetSettingsFirstRegisteredRadio, true);
                        } else if (( whichItem >= kNetSettingsFirstDelayRadio) &&
                            ( whichItem <= kNetSettingsLastDelayRadio))
                        {
                            SwitchAnyRadioOrCheckbox( currentDelay +
                                kNetSettingsFirstDelayRadio, false);
                            RefreshInterfaceItem( currentDelay +
                                kNetSettingsFirstDelayRadio);
                            currentDelay = whichItem - kNetSettingsFirstDelayRadio;
                            SwitchAnyRadioOrCheckbox( currentDelay +
                                kNetSettingsFirstDelayRadio, true);
                        }
                        break;
                }

            }
        }
        DrawInOffWorld();
        GetAnyInterfaceItemGraphicBounds( item, &tRect);
        copy_world(*gOffWorld, *gSaveWorld, &tRect);
        DrawInRealWorld();
        CloseInterface();
    }

    if ( !cancel)
    {
        SetRegisteredSetting( currentRegistered);
        SetBandwidth( currentBandwidth);
        SendInGameMiscLongMessage( 0, eSetRegisteredStateMessage, currentRegistered, true, false);
        SendInGameMiscLongMessage( 0, eSetBandwidthMessage, currentBandwidth, true, false);
        switch( currentDelay)
        {
            case 0:
                SetResendDelay( 60);
                SendInGameMiscLongMessage( 0, eSetResendDelayMessage, 60, true, false);
                break;

            case 1:
                SetResendDelay( 120);
                SendInGameMiscLongMessage( 0, eSetResendDelayMessage, 120, true, false);
                break;

            case 2:
                SetResendDelay( 240);
                SendInGameMiscLongMessage( 0, eSetResendDelayMessage, 240, true, false);
                break;
        }
    }
#endif NETSPROCKET_AVAILABLE
}


//
// BothCommandAndQ:
//  returns true if both the command and q keys are set by player. If this is
//  true, then command-q for quit during a game should be disabled.
//

bool BothCommandAndQ() {
    bool command = false;
    bool q = false;

    for (int i = 0; i < kKeyExtendedControlNum; i++) {
        uint32_t key = Preferences::preferences()->key(i);
        q |= (key == Keys::Q);
        command |= (key == Keys::COMMAND);
    }

    return command && q;
}

netResultType StartNetworkGameSetup( void)

{
#ifdef NETSPROCKET_AVAILABLE
    Point                   where;
    int                     error;
    short                   whichItem = -1;
    bool                 done = false;
    EventRecord             theEvent;
    char                    whichChar;
    netResultType           result = kCancel;

    if ( globals()->gameRangerPending)
    {
        if ( Wrap_GRIsHostCmd())
        {
            whichItem = kNetSetupHostButton;
        } else if ( Wrap_GRIsJoinCmd())
        {
            whichItem = kNetSetupJoinButton;
        }
    }

    FlushEvents(everyEvent, 0);
    if ( whichItem == -1)
    {
        error = OpenInterface( kStartNetworkGameID);
        if ( error == kNoError)
        {
            DrawInterfaceOneAtATime();
            InvalRect(&gRealWorld->bounds);
            while (!done) {
                WaitNextEvent (everyEvent, &theEvent, 3, NULL);
                {
                    whichItem = -1;
                    switch ( theEvent.what )
                    {
                        case osEvt:
//                          HandleOSEvent( &theEvent);
                            break;

                        case mouseDown:
                            where = theEvent.where;
                            GlobalToLocal( &where);
                            whichItem = InterfaceMouseDown( where);
                            break;
                        case mouseUp:
                            break;
                        case keyDown:
                        case autoKey:
                            whichChar = theEvent.message & charCodeMask;
                            whichItem = InterfaceKeyDown( theEvent.message);
                            break;

                    }

                    switch ( whichItem)
                    {
                        case kNetSetupCancelButton:
                            done = true;
                            result = kCancel;
                            break;

                        case kNetSetupHostButton:
//                          BlackenWindow();
//                          if ( DoHostGame()) result = kHost;
                            done = true;
                            break;

                        case kNetSetupJoinButton:
//                          BlackenWindow();
//                          if ( DoJoinGameModalDialog()) result = kClient;
                            done = true;
                            break;
                    }

                }
            }
            CloseInterface();
        }
    }
    switch( whichItem)
    {
        case kNetSetupHostButton:
            BlackenWindow();
            if ( DoHostGame()) result = kHost;
            break;

        case kNetSetupJoinButton:
            BlackenWindow();
            if ( DoJoinGameModalDialog()) result = kClient;
            break;
    }
    return ( result);
#else
    return( kCancel);
#endif NETSPROCKET_AVAILABLE
}

netResultType ClientWaitInterface( void)

{
#ifdef NETSPROCKET_AVAILABLE
    Point                   where;
    int                     error;
    short                   whichItem;
    bool                 done = false;
    EventRecord             theEvent;
    char                    whichChar;
    netResultType           result = kCancel;
    long                    theMessage, roundTripTime, version, serialNumerator,
                            serialDenominator;
    Str255                  s;

    if ( globals()->gameRangerPending)
    {
        globals()->gameRangerPending = false;
        globals()->gameRangerInProgress = true;

    }

    FlushEvents(everyEvent, 0);
    error = OpenInterface( kClientWaitID);
    if ( error == kNoError)
    {
        SetStatusOfAnyInterfaceItem( kClientWaitCancelButton, kDimmed, false);
        DrawInterfaceOneAtATime();
        GetIndString( s, kClientWaitStrID, 18);
        DrawStringInInterfaceItem( kClientWaitStatusRect, s);
        if ( DoJoinGame())
        {
            GetIndString( s, kClientWaitStrID, kClientWaitHostWaitingStrNum);
            DrawStringInInterfaceItem( kClientWaitStatusRect, s);
        } else
        {
            GetIndString( s, kClientWaitStrID, 19);
            DrawStringInInterfaceItem( kClientWaitStatusRect, s);
        }

        SetStatusOfAnyInterfaceItem( kClientWaitCancelButton, kActive, true);

        while (!done) {
            WaitNextEvent (everyEvent, &theEvent, 0, NULL);
            {
                whichItem = -1;
                switch ( theEvent.what )
                {
                    case osEvt:
//                      HandleOSEvent( &theEvent);
                        break;

                    case mouseDown:
                        where = theEvent.where;
                        GlobalToLocal( &where);
                        whichItem = InterfaceMouseDown( where);
                        break;
                    case mouseUp:
                        break;
                    case keyDown:
                    case autoKey:
                        whichChar = theEvent.message & charCodeMask;
                        whichItem = InterfaceKeyDown( theEvent.message);
                        break;
                }

                switch ( whichItem)
                {
                    case kClientWaitCancelButton:
                        done = true;
                        result = kCancel;
                        StopNetworking();
                        break;
                }
                do
                {
                    theMessage = ProcessPreGameMessages( NULL, &version, &serialNumerator,
                        &serialDenominator, NULL,
                        &roundTripTime, 0, NULL, NULL, NULL, NULL);

                    switch( theMessage)
                    {
                        case eHostAcceptsMessage:
                            if (( version == kThisVersion)
                                && (
                                    (
                                        ( serialNumerator != globals()->gSerialNumerator)
                                        ||
                                        ( serialDenominator !=
                                            globals()->gSerialDenominator)
                                        )
                                        ||
                                        (
                                            ( serialNumerator == 0)
                                            &&
                                            ( serialDenominator == 0)
                                        )
                                    )
                                )
                            {
                                GetIndString( s, kClientWaitStrID, kClientWaitHostAcceptedStrNum);
                                DrawStringInInterfaceItem( kClientWaitStatusRect, s);
                                done = true;
                                result = kClient;
                                SendPreGameVerboseMessage( eClientReadyMessage,
                                    kThisVersion, globals()->gSerialNumerator,
                                    globals()->gSerialDenominator, 0);
                                if ( ( serialNumerator == 0) &&
                                            ( serialDenominator == 0))
                                    SetOpponentIsUnregistered( true);
                                else
                                    SetOpponentIsUnregistered( false);
                            } else
                            {
                                SendPreGameVerboseMessage( eClientReadyMessage,
                                    kThisVersion, globals()->gSerialNumerator,
                                    globals()->gSerialDenominator, 0);

                                if ( version < kThisVersion)
                                    ShowErrorAny( eContinueOnlyErr, kErrorStrID,
                                        NULL, NULL, NULL, NULL, kOlderVersionError, -1, -1, -1, __FILE__, 0);
                                else if ( version > kThisVersion)
                                    ShowErrorAny( eContinueOnlyErr, kErrorStrID,
                                        NULL, NULL, NULL, NULL, kNewerVersionError, -1, -1, -1, __FILE__, 0);
                                else
                                    ShowErrorAny( eContinueOnlyErr, kErrorStrID,
                                        NULL, NULL, NULL, NULL, kSameSerialNumberError, -1, -1, -1, __FILE__, 0);

                            }
                            break;

                        case eHostDeclinesMessage:
                            GetIndString( s, kClientWaitStrID, kClientWaitHostDeclinedStrNum);
                            DrawStringInInterfaceItem( kClientWaitStatusRect, s);
                            StopNetworking();
                            break;

                    }
                } while ( theMessage != eNoMessage);

            }
        }
        CloseInterface();
    }
    return ( result);
#else
    return ( kCancel);
#endif NETSPROCKET_AVAILABLE
}

netResultType HostAcceptClientInterface( void)

{
#ifdef NETSPROCKET_AVAILABLE
    Point                   where;
    int                     error;
    short                   whichItem;
    bool                 done = false;
    EventRecord             theEvent;
    char                    whichChar;
    netResultType           result = kCancel;
    long                    theMessage, roundTripTime, version, serialNumerator,
                            serialDenominator;
    Str31                   s;
    unsigned char*          name;

    if ( globals()->gameRangerPending)
    {
        globals()->gameRangerPending = false;
        globals()->gameRangerInProgress = true;

        Wrap_GRHostReady();
    }


    FlushEvents(everyEvent, 0);
    error = OpenInterface( kNetHostID);
    if ( error == kNoError)
    {
        if ( GetNumberOfPlayers() == 2)
        {
            SetStatusOfAnyInterfaceItem( kHostAcceptButton, kActive, false);
            SetStatusOfAnyInterfaceItem( kHostDeclineButton, kActive, false);
        } else
        {
            SetStatusOfAnyInterfaceItem( kHostAcceptButton, kDimmed, false);
            SetStatusOfAnyInterfaceItem( kHostDeclineButton, kDimmed, false);
        }
        DrawInterfaceOneAtATime();
        while (!done) {
            WaitNextEvent (everyEvent, &theEvent, 0, NULL);
            {
                whichItem = -1;
                switch ( theEvent.what )
                {
                    case osEvt:
//                      HandleOSEvent( &theEvent);
                        break;
                    case mouseDown:
                        where = theEvent.where;
                        GlobalToLocal( &where);
                        whichItem = InterfaceMouseDown( where);
                        break;
                    case mouseUp:
                        break;
                    case keyDown:
                    case autoKey:
                        whichChar = theEvent.message & charCodeMask;
                        whichItem = InterfaceKeyDown( theEvent.message);
                        break;
                }

                switch ( whichItem)
                {
                    case kHostCancelButton:
                        done = true;
                        result = kCancel;
                        StopNetworking();
                        break;

                    case kHostAcceptButton:
                        SendPreGameVerboseMessage( eHostAcceptsMessage, kThisVersion,
                            globals()->gSerialNumerator,
                            globals()->gSerialDenominator,
                            0);
                        break;

                    case kHostDeclineButton:
                        SendPreGameBasicMessage( eHostDeclinesMessage);
                        break;


                }
            }
//          SendPreGameBasicMessage( eRoundTripGetReadyMessage);
            do
            {
                theMessage = ProcessPreGameMessages( NULL, &version, &serialNumerator,
                    &serialDenominator, NULL, &roundTripTime, 0,
                    NULL, NULL, NULL, NULL);
                switch( theMessage)
                {
                    case kNSpPlayerJoined:
                    case kNSpPlayerLeft:
                        if ( GetNumberOfPlayers() == 2)
                        {
                            SetStatusOfAnyInterfaceItem( kHostAcceptButton, kActive, true);
                            SetStatusOfAnyInterfaceItem( kHostDeclineButton, kActive, true);
                            GetOtherPlayerName( &name);
                            DrawStringInInterfaceItem( kHostHostNameRect, name);
                            PlayVolumeSound( kComputerBeep2, kMediumLowVolume, kShortPersistence, kMustPlaySound);
                        } else
                        {
                            SetStatusOfAnyInterfaceItem( kHostAcceptButton, kDimmed, true);
                            SetStatusOfAnyInterfaceItem( kHostDeclineButton, kDimmed, true);
                            s[0] = 0;
                            DrawStringInInterfaceItem( kHostHostNameRect, s);
                        }
                        break;

                    case eClientReadyMessage:
                        if ( version < kThisVersion)
                        {
                            ShowErrorAny( eContinueOnlyErr, kErrorStrID,
                                NULL, NULL, NULL, NULL, kOlderVersionError, -1, -1, -1, __FILE__, 0);
                        } else if ( version > kThisVersion)
                        {
                            ShowErrorAny( eContinueOnlyErr, kErrorStrID,
                                NULL, NULL, NULL, NULL, kNewerVersionError, -1, -1, -1, __FILE__, 0);
                        } else if (( serialNumerator == globals()->gSerialNumerator)
                            && ( serialDenominator == globals()->gSerialDenominator)
                            && (( serialNumerator != 0) || ( serialDenominator != 0)))
                        {
                            ShowErrorAny( eContinueOnlyErr, kErrorStrID,
                                NULL, NULL, NULL, NULL, kSameSerialNumberError, -1, -1, -1, __FILE__, 0);
                        } else
                        {
                            if ( ( serialNumerator == 0) &&
                                        ( serialDenominator == 0))
                                SetOpponentIsUnregistered( true);
                            else
                                SetOpponentIsUnregistered( false);
                            done = true;
                            result = kHost;
                        }
                        break;
                }
            } while ( theMessage != eNoMessage);
        }
        CloseInterface();
    }
    return ( result);
#else
    return( kCancel);
#endif
}

void BlackenWindow( void)

{
    Rect    tRect;

    tRect = world;
    gOffWorld->view(tRect).fill(RgbColor::kBlack);
    gRealWorld->view(tRect).fill(RgbColor::kBlack);
}

void UpdateMissionBriefPoint(
        interfaceItemType *dataItem, long whichBriefPoint, const Scenario* scenario,
        coordPointType *corner, long scale, Rect *bounds, vector<inlinePictType>& inlinePict,
        PixMap* pix) {
    if (whichBriefPoint < kMissionBriefPointOffset) {
        // No longer handled here.
        return;
    }

    whichBriefPoint -= kMissionBriefPointOffset;

    Rect hiliteBounds;
    long            headerID, headerNumber, contentID;
    BriefPoint_Data_Get(whichBriefPoint, scenario, &headerID, &headerNumber, &contentID,
            &hiliteBounds, corner, scale, 16, 32, bounds);
    hiliteBounds.offset(bounds->left, bounds->top);

    // TODO(sfiera): catch exception.
    Resource rsrc("text", "txt", contentID);
    String text(macroman::decode(rsrc.data()));
    short textHeight = GetInterfaceTextHeightFromWidth(text, dataItem->style, kMissionDataWidth);
    if (hiliteBounds.left == hiliteBounds.right) {
        dataItem->bounds.left = (bounds->right - bounds->left) / 2 - (kMissionDataWidth / 2) + bounds->left;
        dataItem->bounds.right = dataItem->bounds.left + kMissionDataWidth;
        dataItem->bounds.top = (bounds->bottom - bounds->top) / 2 - (textHeight / 2) + bounds->top;
        dataItem->bounds.bottom = dataItem->bounds.top + textHeight;
    } else {
        if ((hiliteBounds.left + (hiliteBounds.right - hiliteBounds.left) / 2) >
                (bounds->left + (bounds->right - bounds->left) / 2)) {
            dataItem->bounds.right = hiliteBounds.left - kMissionDataHBuffer;
            dataItem->bounds.left = dataItem->bounds.right - kMissionDataWidth;
        } else {
            dataItem->bounds.left = hiliteBounds.right + kMissionDataHBuffer;
            dataItem->bounds.right = dataItem->bounds.left + kMissionDataWidth;
        }

        dataItem->bounds.top = hiliteBounds.top + (hiliteBounds.bottom - hiliteBounds.top) / 2 -
                                textHeight / 2;
        dataItem->bounds.bottom = dataItem->bounds.top + textHeight;
        if (dataItem->bounds.top < (bounds->top + kMissionDataTopBuffer)) {
            dataItem->bounds.top = bounds->top + kMissionDataTopBuffer;
            dataItem->bounds.bottom = dataItem->bounds.top + textHeight;
        }
        if (dataItem->bounds.bottom > (bounds->bottom - kMissionDataBottomBuffer)) {
            dataItem->bounds.bottom = bounds->bottom - kMissionDataBottomBuffer;
            dataItem->bounds.top = dataItem->bounds.bottom - textHeight;
        }

        if (dataItem->bounds.left < (bounds->left + kMissionDataVBuffer)) {
            dataItem->bounds.left = bounds->left + kMissionDataVBuffer;
            dataItem->bounds.right = dataItem->bounds.left + kMissionDataWidth;
        }
        if (dataItem->bounds.right > (bounds->right - kMissionDataVBuffer)) {
            dataItem->bounds.right = bounds->right - kMissionDataVBuffer;
            dataItem->bounds.left = dataItem->bounds.right - kMissionDataWidth;
        }

        const RgbColor very_light = GetRGBTranslateColorShade(kMissionDataHiliteColor, VERY_LIGHT);
        hiliteBounds.right++;
        hiliteBounds.bottom++;
        FrameRect(pix, hiliteBounds, very_light);
        const RgbColor color = GetRGBTranslateColorShade(kMissionDataHiliteColor, MEDIUM);
        Rect newRect;
        GetAnyInterfaceItemGraphicBounds(*dataItem, &newRect);
        if (dataItem->bounds.right < hiliteBounds.left) {
            MoveTo(hiliteBounds.left, hiliteBounds.top);
            MacLineTo(pix, newRect.right + kMissionLineHJog, hiliteBounds.top, color);
            MacLineTo(pix, newRect.right + kMissionLineHJog, newRect.top, color);
            MacLineTo(pix, newRect.right + 1, newRect.top, color);
            MoveTo(hiliteBounds.left, hiliteBounds.bottom - 1);
            MacLineTo(pix, newRect.right + kMissionLineHJog, hiliteBounds.bottom - 1, color);
            MacLineTo(pix, newRect.right + kMissionLineHJog, newRect.bottom - 1, color);
            MacLineTo(pix, newRect.right + 1, newRect.bottom - 1, color);
        } else {
            MoveTo(hiliteBounds.right, hiliteBounds.top);
            MacLineTo(pix, newRect.left - kMissionLineHJog, hiliteBounds.top, color);
            MacLineTo(pix, newRect.left - kMissionLineHJog, newRect.top, color);
            MacLineTo(pix, newRect.left - 2, newRect.top, color);
            MoveTo(hiliteBounds.right, hiliteBounds.bottom - 1);
            MacLineTo(pix, newRect.left - kMissionLineHJog, hiliteBounds.bottom - 1, color);
            MacLineTo(pix, newRect.left - kMissionLineHJog, newRect.bottom - 1, color);
            MacLineTo(pix, newRect.left - 2, newRect.bottom - 1, color);
        }
    }
    dataItem->item.labeledRect.label.stringID = headerID;
    dataItem->item.labeledRect.label.stringNumber = headerNumber;
    Rect newRect;
    GetAnyInterfaceItemGraphicBounds(*dataItem, &newRect);
    pix->view(newRect).fill(RgbColor::kBlack);
    DrawAnyInterfaceItem(*dataItem, pix);

    newRect = dataItem->bounds;
    DrawInterfaceTextInRect(newRect, text, dataItem->style, dataItem->color, pix, inlinePict);
}

void ShowObjectData( Point where, short pictID, Rect *clipRect)
{
    Rect            dataRect;
    Rect        lRect, longClipRect;
    baseObjectType  *baseObject = gBaseObjectData.get();// + (pictID - kFirstShipDataPictID);
    retroTextSpecType   retroTextSpec;
    long            height, waitTime, i;

    // find object who belongs to this pict id
    i = 0;
    while (( i < globals()->maxBaseObject) && ( baseObject->pictPortraitResID != pictID))
    {
        i++;
        baseObject++;
    }

    if ( i >= globals()->maxBaseObject) return;

    {
        // HideCursor();

        retroTextSpec.text.reset(new String);
        if ( retroTextSpec.text.get() != NULL) {
            CreateObjectDataText(retroTextSpec.text.get(), i);
            retroTextSpec.textLength = retroTextSpec.text->size();

            mSetDirectFont( kButtonFontNum);
            retroTextSpec.thisPosition = retroTextSpec.linePosition = retroTextSpec.lineCount = 0;
            retroTextSpec.tabSize = 100;
            retroTextSpec.color = GetRGBTranslateColorShade(GREEN, VERY_LIGHT);
            retroTextSpec.backColor = GetRGBTranslateColorShade(GREEN, DARKEST);
            retroTextSpec.originalColor = retroTextSpec.nextColor = retroTextSpec.color;
            retroTextSpec.originalBackColor = retroTextSpec.nextBackColor = retroTextSpec.backColor;
            retroTextSpec.topBuffer = 1;
            retroTextSpec.bottomBuffer = 1;

            height = DetermineDirectTextHeightInWidth( &retroTextSpec, kShipDataWidth);

            dataRect.left = (where.h) - ( retroTextSpec.autoWidth / 2);
            dataRect.right = dataRect.left + retroTextSpec.autoWidth;
            dataRect.top = (where.v) - ( retroTextSpec.autoHeight / 2);
            dataRect.bottom = dataRect.top + retroTextSpec.autoHeight;

            if ( dataRect.left < clipRect->left)
            {
                dataRect.offset(clipRect->left - dataRect.left + 1, 0);
            } else if ( dataRect.right > clipRect->right)
            {
                dataRect.offset(clipRect->right - dataRect.right - 1, 0);
            }

            if ( dataRect.top < clipRect->top)
            {
                dataRect.offset(0, clipRect->top - dataRect.top + 1);
            } else if ( dataRect.bottom > clipRect->bottom)
            {
                dataRect.offset(0, clipRect->bottom - dataRect.bottom - 1);
            }
            retroTextSpec.xpos = dataRect.left;
            retroTextSpec.ypos = dataRect.top + mDirectFontAscent();

    //      clipRect.left = dataRect.left;
    //      clipRect.right = dataRect.right;
    //      clipRect.top = dataRect.top;
    //      clipRect.bottom = dataRect.bottom;
            longClipRect = *clipRect;
            lRect = dataRect;
            dataRect.inset(-8, -4);
            gRealWorld->view(dataRect).fill(RgbColor::kBlack);
            const RgbColor light_green = GetRGBTranslateColorShade(GREEN, VERY_LIGHT);
            FrameRect(gRealWorld, dataRect, light_green);


            while (( retroTextSpec.thisPosition < retroTextSpec.textLength) && (( VideoDriver::driver()->button()) || (AnyRealKeyDown())))
            {
                PlayVolumeSound(  kComputerBeep3, kMediumLowVolume, kShortPersistence, kLowPrioritySound);
                DrawRetroTextCharInRect( &retroTextSpec, 24, lRect, lRect, gRealWorld);

                waitTime = VideoDriver::driver()->ticks();
                while (( VideoDriver::driver()->ticks() - waitTime) < 3) {
                    // DO NOTHING
                };
            }

            retroTextSpec.text.reset();
        }

        // MacShowCursor();
        while (( VideoDriver::driver()->button()) || (AnyRealKeyDown())) {
            // DO NOTHING
        };

        copy_world(*gRealWorld, *gOffWorld, dataRect);
    }
}

void CreateObjectDataText(String* text, short id) {
    Resource rsrc("text", "txt", kShipDataTextID);
    String data(macroman::decode(rsrc.data()));

    const baseObjectType& baseObject = gBaseObjectData.get()[id];

    StringList keys(kShipDataKeyStringID);
    StringList values(kShipDataNameID);

    // *** Replace place-holders in text with real data, using the fabulous find_replace routine
    // an object or a ship?
    if ( baseObject.attributes & kCanThink) {
        const StringSlice& name = values.at(0);
        find_replace(data, 0, keys.at(kShipOrObjectStringNum), name);
    } else {
        const StringSlice& name = values.at(1);
        find_replace(data, 0, keys.at(kShipOrObjectStringNum), name);
    }

    // ship name
    {
        StringList names(5000);
        const StringSlice& name = names.at(id);
        find_replace(data, 0, keys.at(kShipTypeStringNum), name);
    }

    // ship mass
    find_replace(data, 0, keys.at(kMassStringNum), fixed(baseObject.mass));

    // ship shields
    find_replace(data, 0, keys.at(kShieldStringNum), baseObject.health);

    // light speed
    find_replace(data, 0, keys.at(kHasLightStringNum), baseObject.warpSpeed);

    // max velocity
    find_replace(data, 0, keys.at(kMaxSpeedStringNum), fixed(baseObject.maxVelocity));

    // thrust
    find_replace(data, 0, keys.at(kThrustStringNum), fixed(baseObject.maxThrust));

    // par turn
    find_replace(data, 0, keys.at(kTurnStringNum),
            fixed(baseObject.frame.rotation.turnAcceleration));

    // now, check for weapons!
    CreateWeaponDataText(&data, baseObject.pulse, values.at(kShipDataPulseStringNum));
    CreateWeaponDataText(&data, baseObject.beam, values.at(kShipDataBeamStringNum));
    CreateWeaponDataText(&data, baseObject.special, values.at(kShipDataSpecialStringNum));

    print(*text, data);
}

void CreateWeaponDataText(String* text, long whichWeapon, const StringSlice& weaponName) {
    baseObjectType      *weaponObject, *missileObject;
    long                mostDamage, actionNum;
    objectActionType    *action;
    bool             isGuided = false;

    if (whichWeapon == kNoShip) {
        return;
    }

    weaponObject = gBaseObjectData.get() + whichWeapon;

    // TODO(sfiera): catch exception.
    Resource rsrc("text", "txt", kWeaponDataTextID);
    String data(macroman::decode(rsrc.data()));
    // damage; this is tricky--we have to guess by walking through activate actions,
    //  and for all the createObject actions, see which creates the most damaging
    //  object.  We calc this first so we can use isGuided

    mostDamage = 0;
    isGuided = false;
    if ( weaponObject->activateActionNum > 0)
    {
        action = gObjectActionData.get() + weaponObject->activateAction;
        for ( actionNum = 0; actionNum < weaponObject->activateActionNum; actionNum++)
        {
            if (( action->verb == kCreateObject) || ( action->verb == kCreateObjectSetDest))
            {
                missileObject = gBaseObjectData.get() +
                    action->argument.createObject.whichBaseType;
                if ( missileObject->attributes & kIsGuided) isGuided = true;
                if ( missileObject->damage > mostDamage) mostDamage = missileObject->damage;
            }
            action++;
        }
    }

    StringList keys(kShipDataKeyStringID);
    StringList values(kShipDataNameID);

    // weapon name #
    find_replace(data, 0, keys.at(kWeaponNumberStringNum), weaponName);

    // weapon name
    {
        StringList names(5000);
        const StringSlice& name = names.at(whichWeapon);
        find_replace(data, 0, keys.at(kWeaponNameStringNum), name);
    }

    const StringSlice& yes = values.at(kShipDataYesStringNum);
    const StringSlice& no = values.at(kShipDataNoStringNum);
    const StringSlice& dash = values.at(kShipDataDashStringNum);

    // is guided
    if (isGuided) {
        find_replace(data, 0, keys.at(kWeaponGuidedStringNum), yes);
    } else {
        find_replace(data, 0, keys.at(kWeaponGuidedStringNum), no);
    }

    // is autotarget
    if (weaponObject->attributes & kAutoTarget) {
        find_replace(data, 0, keys.at(kWeaponAutoTargetStringNum), yes);
    } else {
        find_replace(data, 0, keys.at(kWeaponAutoTargetStringNum), no);
    }

    // range
    find_replace(data, 0, keys.at(kWeaponRangeStringNum),
            lsqrt(weaponObject->frame.weapon.range));

    if (mostDamage > 0) {
        find_replace(data, 0, keys.at(kWeaponDamageStringNum), mostDamage);
    } else {
        find_replace(data, 0, keys.at(kWeaponDamageStringNum), dash);
    }
    print(*text, data);
}

void Replace_KeyCode_Strings_With_Actual_Key_Names(String* text, short resID, size_t padTo) {
    StringList keys(kHelpScreenKeyStringID);
    StringList values(resID);

    for (int i = 0; i < kKeyExtendedControlNum; ++i) {
        const StringSlice& search = keys.at(i);
        String replace(values.at(Preferences::preferences()->key(i) - 1));
        if (replace.size() < padTo) {
            replace.resize(padTo, ' ');
        }
        while (find_replace(*text, 0, search, replace) > 0) {
            // DO NOTHING
        };
    }
}

}  // namespace antares
