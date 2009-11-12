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

#include "InterfaceScreen.hpp"

#include "BinaryStream.hpp"
#include "ColorTranslation.hpp"
#include "Error.hpp"
#include "FakeDrawing.hpp"
#include "InterfaceHandling.hpp"
#include "OffscreenGWorld.hpp"
#include "PlayerInterface.hpp"
#include "Resource.hpp"
#include "Time.hpp"

namespace antares {

extern long WORLD_WIDTH;
extern long WORLD_HEIGHT;
extern PixMap* gRealWorld;

InterfaceScreen::InterfaceScreen(int id, const Rect& bounds, bool full_screen)
        : _state(NORMAL),
          _id(id),
          _bounds(bounds),
          _full_screen(full_screen),
          _last_event(now_secs()),
          _hit_item(0),
          _pix(new ArrayPixMap(bounds.width(), bounds.height())) {
    Resource rsrc('intr', id);
    BufferBinaryReader bin(rsrc.data(), rsrc.size());
    const int offset_x = (bounds.width() / 2) - 320;
    const int offset_y = (bounds.height() / 2) - 240;
    while (bin.bytes_read() < rsrc.size()) {
        _items.push_back(interfaceItemType());
        interfaceItemType* const item = &_items.back();
        bin.read(item);
        item->bounds.offset(offset_x, offset_y);
    }
}

InterfaceScreen::~InterfaceScreen() { }

void InterfaceScreen::become_front() {
    this->adjust_interface();
    draw();
    _last_event = now_secs();
    // half-second fade from black.
}

void InterfaceScreen::draw() const {
    Rect copy_area;
    if (_full_screen) {
        copy_area = _pix->bounds();
    } else {
        GetAnyInterfaceItemGraphicBounds(_items[0], &copy_area);
        for (size_t i = 1; i < _items.size(); ++i) {
            Rect r;
            GetAnyInterfaceItemGraphicBounds(_items[i], &r);
            copy_area.enlarge_to(r);
        }
    }
    _pix->fill(RgbColor::kBlack);
    for (std::vector<interfaceItemType>::const_iterator it = _items.begin(); it != _items.end();
            ++it) {
        DrawAnyInterfaceItem(*it, _pix.get());
    }
    gRealWorld->view(_bounds).view(copy_area).copy(_pix->view(copy_area));
}

bool InterfaceScreen::mouse_down(int button, const Point& where) {
    if (button != 0) {
        return true;
    }
    for (size_t i = 0; i < _items.size(); ++i) {
        interfaceItemType* const item = &_items[i];
        Rect bounds;
        GetAnyInterfaceItemGraphicBounds(*item, &bounds);
        if (item->status() != kDimmed && bounds.contains(where)) {
            switch (item->kind) {
              case kPlainButton:
              case kCheckboxButton:
              case kRadioButton:
              case kTabBoxButton:
                _state = MOUSE_DOWN;
                item->set_status(kIH_Hilite);
                draw();
                // play kComputerBeep1, kMediumLoudVolume, kShortPersistence, kMustPlaySound.
                _hit_item = i;
                return true;

              case kLabeledRect:
                return true;

              case kListRect:
                fail("kListRect not yet handled");

              default:
                break;
            }
        }
    }
    return true;
}

bool InterfaceScreen::mouse_up(int button, const Point& where) {
    if (button != 0) {
        return true;
    }
    if (_state == MOUSE_DOWN) {
        // Save _hit_item and set it to 0 before calling handle_button(), as calling
        // handle_button() can result in the deletion of `this`.
        int hit_item = _hit_item;
        _hit_item = 0;

        _state = NORMAL;
        interfaceItemType* const item = &_items[hit_item];
        Rect bounds;
        GetAnyInterfaceItemGraphicBounds(*item, &bounds);
        item->set_status(kActive);
        draw();
        if (bounds.contains(where)) {
            if (item->kind == kTabBoxButton) {
                item->item.radioButton.on = true;
            }
            handle_button(hit_item);
        }
    }
    return true;
}

bool InterfaceScreen::mouse_moved(int button, const Point& where) {
    (void)button;
    (void)where;
    return true;
}

bool InterfaceScreen::key_down(int key) {
    const int32_t key_code = ((key & keyCodeMask) >> 8) + 1;
    if (key_code > 0) {
        for (size_t i = 0; i < _items.size(); ++i) {
            interfaceItemType* const item = &_items[i];
            if (item->status() != kDimmed && item->key() == key_code) {
                _state = KEY_DOWN;
                item->set_status(kIH_Hilite);
                draw();
                // play kComputerBeep1, kMediumLoudVolume, kShortPersistence, kMustPlaySound.
                _hit_item = i;
                return true;
            }
        }
    }
    return true;
}

bool InterfaceScreen::key_up(int key) {
    // TODO(sfiera): verify that the same key that was pressed was released.
    (void)key;
    if (_state == KEY_DOWN) {
        // Save _hit_item and set it to 0 before calling handle_button(), as calling
        // handle_button() can result in the deletion of `this`.
        int hit_item = _hit_item;
        _hit_item = 0;

        _state = NORMAL;
        interfaceItemType* const item = &_items[hit_item];
        item->set_status(kActive);
        if (item->kind == kTabBoxButton) {
            item->item.radioButton.on = true;
        }
        draw();
        handle_button(hit_item);
    }
    return true;
}

double InterfaceScreen::last_event() const {
    return _last_event;
}

void InterfaceScreen::adjust_interface() { }

const interfaceItemType& InterfaceScreen::item(int i) const {
    return _items[i];
}

interfaceItemType* InterfaceScreen::mutable_item(int i) {
    return &_items[i];
}

PixMap* InterfaceScreen::pix() const {
    return _pix.get();
}

}  // namespace antares
