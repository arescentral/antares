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

// Interface Handling.c

//
// Liaison between the application and Interface Drawing.  Takes in events ( key events, mouse
// down events), hilights and scrolls as needed, and returns results.  Also handles editable text.
//

#include "InterfaceHandling.hpp"

#include <assert.h>
#include <vector>
#include "AnyChar.hpp"
#include "AresGlobalType.hpp"
#include "BinaryStream.hpp"
#include "ColorTranslation.hpp"
#include "Debug.hpp"
#include "Error.hpp"
#include "KeyMapTranslation.hpp"
#include "OffscreenGWorld.hpp"
#include "PlayerInterfaceDrawing.hpp"
#include "PlayerInterfaceItems.hpp"
#include "Resource.hpp"
#include "SoundFX.hpp"              // for button on/off

namespace antares {

#define kMakeInterfaceItem      20

#define kInterfaceError         "\pINTF"
#define kInterfaceResFileName   "\p:Ares Data Folder:Ares Interfaces"

#define kInterfaceResourceType  'intr'

#define kClickLoopInfo          ( kRegisterBased | RESULT_SIZE( SIZE_CODE ( sizeof ( bool))) | REGISTER_RESULT_LOCATION( kRegisterD0))

#define kTargetScreenWidth      640
#define kTargetScreenHeight     480

#define mPlayButtonDown         PlayVolumeSound( kComputerBeep1, kMediumLoudVolume, kShortPersistence, kMustPlaySound)
#define mPlayButtonUp           PlayVolumeSound( kComputerBeep2, kMediumLowVolume, kShortPersistence, kMustPlaySound)
#define mPlayScreenSound        PlayVolumeSound( kComputerBeep3, kMediumLowVolume, kShortPersistence, kVeryLowPrioritySound)

extern long             WORLD_WIDTH, WORLD_HEIGHT;
extern PixMap*          gActiveWorld;
extern PixMap*          gOffWorld;
extern PixMap*          gSaveWorld;

std::vector<interfaceItemType> gInterfaceItemData;
uint32_t gInterfaceScreenHBuffer = 0;
uint32_t gInterfaceScreenVBuffer = 0;

ScopedOpenInterface::ScopedOpenInterface(int id) {
    OpenInterface(id);
}

ScopedOpenInterface::~ScopedOpenInterface() {
    CloseInterface();
}

int InterfaceHandlingInit() {
    gInterfaceScreenHBuffer = (WORLD_WIDTH / 2) - (kTargetScreenWidth / 2);
    gInterfaceScreenVBuffer = (WORLD_HEIGHT / 2) - (kTargetScreenHeight / 2);

    return kNoError;
}

void InterfaceHandlingCleanup() {
    gInterfaceItemData.clear();
}

int OpenInterface(short resID) {
    Resource rsrc(kInterfaceResourceType, resID);
    BufferBinaryReader bin(rsrc.data(), rsrc.size());

    gInterfaceItemData.clear();
    while (bin.bytes_read() < rsrc.size()) {
        gInterfaceItemData.push_back(interfaceItemType());
        interfaceItemType* const item = &gInterfaceItemData.back();
        bin.read(item);
        item->bounds.left += gInterfaceScreenHBuffer;
        item->bounds.right += gInterfaceScreenHBuffer;
        item->bounds.top += gInterfaceScreenVBuffer;
        item->bounds.bottom += gInterfaceScreenVBuffer;
    }

    InvalidateInterfaceFunctions(); // if they've been set, they shouldn't be yet

    return kNoError;
}

long AppendInterface(short resID, long relative_number, bool center) {
    assert(relative_number >= 0);
    const int32_t original_number = gInterfaceItemData.size();

    Resource rsrc(kInterfaceResourceType, resID);
    BufferBinaryReader bin(rsrc.data(), rsrc.size());
    while (bin.bytes_read() < rsrc.size()) {
        gInterfaceItemData.push_back(interfaceItemType());
        bin.read(&gInterfaceItemData.back());
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
    DrawInOffWorld();
    Rect tRect(0, 0, WORLD_WIDTH, WORLD_HEIGHT);
    SetTranslateColorFore(BLACK);
    gActiveWorld->view(tRect).fill(BLACK);

    for (size_t i = 0; i < gInterfaceItemData.size(); ++i) {
        DrawAnyInterfaceItem(gInterfaceItemData[i], gOffWorld);
    }
    DrawInRealWorld();
    CopyOffWorldToRealWorld(tRect);
}

void DrawInterfaceRange(long from, long to, long withinItem) {
    DrawInOffWorld();
    interfaceItemType* const item = GetAnyInterfaceItemPtr(withinItem);
    Rect tRect = item->bounds;
    if (withinItem >= 0) {
        SetTranslateColorFore(BLACK);
        gActiveWorld->view(tRect).fill(BLACK);
    }
    from = std::max<int>(from, 0);
    to = std::min<int>(to, gInterfaceItemData.size());
    if (from < to) {
        for (int i = from; i < to; ++i) {
            DrawAnyInterfaceItem(gInterfaceItemData[i], gOffWorld);
        }
        DrawInRealWorld();
        if (withinItem >= 0) {
            CopyOffWorldToRealWorld(tRect);
        }
    }
}

void DrawAllItemsOfKind(interfaceKindType kind, bool sound, bool clearFirst, bool showAtEnd) {
    DrawInOffWorld();
    Rect tRect(0, 0, WORLD_WIDTH, WORLD_HEIGHT);
    SetTranslateColorFore(BLACK);
    if (clearFirst) {
        gActiveWorld->view(tRect).fill(BLACK);
    }

    for (size_t i = 0; i < gInterfaceItemData.size(); ++i) {
        const interfaceItemType& item = gInterfaceItemData[i];
        if (sound) {
            mPlayScreenSound;
        }
        if (item.kind == kind) {
            if (showAtEnd) {
                DrawAnyInterfaceItem(item, gOffWorld);
            } else {
                DrawAnyInterfaceItemOffToOn(item);
            }
        }
    }
    DrawInRealWorld();
    if (showAtEnd) {
        CopyOffWorldToRealWorld(tRect);
    }
}

void DrawAnyInterfaceItemOffToOn(const interfaceItemType& item) {
    Rect bounds;

    GetAnyInterfaceItemGraphicBounds(item, &bounds);
    DrawInOffWorld();
    DrawAnyInterfaceItem(item, gOffWorld);
    DrawInRealWorld();
    CopyOffWorldToRealWorld(bounds);
}

void DrawAnyInterfaceItemSaveToOffToOn(const interfaceItemType& item) {
    Rect bounds;

    GetAnyInterfaceItemGraphicBounds(item, &bounds);
    DrawInSaveWorld();
    DrawAnyInterfaceItem(item, gSaveWorld);
    DrawInOffWorld();
    CopySaveWorldToOffWorld(bounds);
    DrawInRealWorld();
    CopyOffWorldToRealWorld(bounds);
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
            item->item.listRect.getListLength = nil;
            item->item.listRect.getItemString = nil;
            item->item.listRect.itemHilited = nil;
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
    int32_t key_code = message & keyCodeMask;
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
            mPlayButtonDown;
            KeyMap key_map;
            do {
                GetKeys(key_map);
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
    while (Button()) {
        GetMouse(&where);
        if (tRect.contains(where)) {
            if (button->item.plainButton.status != kIH_Hilite) {
                mPlayButtonDown;
                button->item.plainButton.status = kIH_Hilite;
                DrawAnyInterfaceItemOffToOn(*button);
            }
        } else {
            if (button->item.plainButton.status != kActive) {
                mPlayButtonUp;
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
        GetMouse(&where);
        if (tRect.contains(where)) {
            if (button->item.checkboxButton.status != kIH_Hilite) {
                mPlayButtonDown;
                button->item.checkboxButton.status = kIH_Hilite;
                DrawAnyInterfaceItemOffToOn(*button);
            }
        } else {
            if (button->item.checkboxButton.status != kActive) {
                button->item.checkboxButton.status = kActive;
                DrawAnyInterfaceItemOffToOn(*button);
            }
        }
    } while (Button());
    if ( button->item.checkboxButton.status == kIH_Hilite) {
        button->item.checkboxButton.status = kActive;
    }
    GetMouse(&where);
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
        GetMouse(&where);
        if (tRect.contains(where)) {
            if (button->item.radioButton.status != kIH_Hilite) {
                mPlayButtonDown;
                button->item.radioButton.status = kIH_Hilite;
                DrawAnyInterfaceItemOffToOn(*button);
            }
        } else {
            if (button->item.radioButton.status != kActive) {
                button->item.radioButton.status = kActive;
                DrawAnyInterfaceItemOffToOn(*button);
            }
        }
    } while (Button());
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
        GetMouse(&where);
        if (tRect.contains(where)) {
            if (button->item.radioButton.status != kIH_Hilite) {
                mPlayButtonDown;
                button->item.radioButton.status = kIH_Hilite;
                DrawAnyInterfaceItemOffToOn(*button);
            }
        } else {
            if (button->item.radioButton.status != kActive) {
                button->item.radioButton.status = kActive;
                DrawAnyInterfaceItemOffToOn(*button);
            }
        }
    } while ( Button());
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

    if (listRect.item.listRect.getListLength != nil) {
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
    DrawInOffWorld();
    DefaultColors();
    gActiveWorld->view(tRect).fill(BLACK);
    DrawAnyInterfaceItemOffToOn(item);
}

void SetInterfaceListCallback(
        short whichItem,
        short (*getListLength)(void),
        void (*getItemString)(short, unsigned char*),
        bool (*itemHilited)(short, bool)) {
    interfaceItemType* const item = &gInterfaceItemData[whichItem];
    if (item->kind == kListRect) {
        item->item.listRect.getListLength = getListLength;
        item->item.listRect.getItemString = getItemString;
        item->item.listRect.itemHilited = itemHilited;
        item->item.listRect.topItem = 0;
    }
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

void interfaceItemType::read(BinaryReader* bin) {
    char section[22];

    bin->read(&bounds);
    bin->read(section, 22);
    bin->read(&color);
    bin->read(&kind);
    bin->read(&style);
    bin->discard(1);

    BufferBinaryReader sub(section, 22);
    switch (kind) {
      case kPlainRect:
      case kPictureRect:
        sub.read(&item.pictureRect);
        break;

      case kLabeledRect:
        sub.read(&item.labeledRect);
        break;

      case kListRect:
        sub.read(&item.listRect);
        break;

      case kTextRect:
        sub.read(&item.textRect);
        break;

      case kPlainButton:
        sub.read(&item.plainButton);
        break;

      case kRadioButton:
      case kTabBoxButton:
        sub.read(&item.radioButton);
        break;

      case kCheckboxButton:
        sub.read(&item.checkboxButton);
        break;

      case kTabBox:
        sub.read(&item.tabBox);
        break;

      case kTabBoxTop:
        break;
    }
}

void interfaceLabelType::read(BinaryReader* bin) {
    bin->read(&stringID);
    bin->read(&stringNumber);
}

void interfaceLabeledRectType::read(BinaryReader* bin) {
    bin->read(&label);
    bin->read(&color);
    bin->discard(5);
    bin->read(&editable);
}

void interfaceListType::read(BinaryReader* bin) {
    bin->read(&label);
    bin->discard(12);
    bin->read(&topItem);

    getListLength = NULL;
    getItemString = NULL;
    itemHilited = NULL;
}

void interfaceTextRectType::read(BinaryReader* bin) {
    bin->read(&textID);
    bin->read(&visibleBounds);
}

void interfaceButtonType::read(BinaryReader* bin) {
    bin->read(&label);
    bin->read(&key);
    bin->read(&defaultButton);
    bin->read(&status);
}

void interfaceRadioType::read(BinaryReader* bin) {
    bin->read(&label);
    bin->read(&key);
    bin->read(&on);
    bin->read(&status);
}

void interfaceCheckboxType::read(BinaryReader* bin) {
    bin->read(&label);
    bin->read(&key);
    bin->read(&on);
    bin->read(&status);
}

void interfacePictureRectType::read(BinaryReader* bin) {
    bin->read(&pictureID);
    bin->read(&visibleBounds);
}

void interfaceTabBoxType::read(BinaryReader* bin) {
    bin->read(&topRightBorderSize);
}

}  // namespace antares
