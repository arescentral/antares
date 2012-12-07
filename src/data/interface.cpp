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

#include "data/interface.hpp"

#include <sfz/sfz.hpp>

using sfz::BytesSlice;
using sfz::ReadSource;

namespace antares {

void read_from(ReadSource in, interfaceItemType& item) {
    uint8_t section[22];

    read(in, item.bounds);
    read(in, section, 22);
    read(in, item.color);
    read(in, item.kind);
    read(in, item.style);
    in.shift(1);

    BytesSlice sub(section, 22);
    switch (item.kind) {
      case kPlainRect:
      case kPictureRect:
        read(sub, item.item.pictureRect);
        break;

      case kLabeledRect:
        read(sub, item.item.labeledRect);
        break;

      case kListRect:
        read(sub, item.item.listRect);
        break;

      case kTextRect:
        read(sub, item.item.textRect);
        break;

      case kPlainButton:
        read(sub, item.item.plainButton);
        break;

      case kRadioButton:
      case kTabBoxButton:
        read(sub, item.item.radioButton);
        break;

      case kCheckboxButton:
        read(sub, item.item.checkboxButton);
        break;

      case kTabBox:
        read(sub, item.item.tabBox);
        break;

      case kTabBoxTop:
        break;
    }
}

interfaceItemStatusType interfaceItemType::status() const {
    switch (kind) {
      case kPlainButton:
        return item.plainButton.status;
      case kRadioButton:
      case kTabBoxButton:
        return item.radioButton.status;
      case kCheckboxButton:
        return item.checkboxButton.status;
      case kTextRect:
        return item.textRect.visibleBounds ? kActive : kDimmed;
      case kPictureRect:
        return item.pictureRect.visibleBounds ? kActive : kDimmed;
      default:
        return kDimmed;
    }
}

void interfaceItemType::set_status(interfaceItemStatusType status) {
    switch (kind) {
      case kPlainButton:
        item.plainButton.status = status;
        break;
      case kRadioButton:
      case kTabBoxButton:
        item.radioButton.status = status;
        break;
      case kCheckboxButton:
        item.checkboxButton.status = status;
        break;
      case kTextRect:
        item.textRect.visibleBounds = (status == kActive);
        break;
      case kPictureRect:
        item.pictureRect.visibleBounds = (status == kActive);
        break;
      default:
        break;
    }
}

int interfaceItemType::key() const {
    switch (kind) {
      case kPlainButton:
        return item.plainButton.key;
      case kTabBoxButton:
        return item.radioButton.key;
      default:
        return 0;
    }
}

void interfaceItemType::set_key(int key) {
    switch (kind) {
      case kPlainButton:
        item.plainButton.key = key;
        break;
      case kTabBoxButton:
        item.radioButton.key = key;
        break;
      default:
        break;
    }
}

void read_from(ReadSource in, interfaceLabelType& label) {
    read(in, label.stringID);
    read(in, label.stringNumber);
}

void read_from(ReadSource in, interfaceLabeledRectType& labeled_rect) {
    read(in, labeled_rect.label);
    read(in, labeled_rect.color);
    in.shift(5);
    read(in, labeled_rect.editable);
}

void read_from(ReadSource in, interfaceListType& list) {
    read(in, list.label);
    in.shift(12);
    read(in, list.topItem);

    list.getListLength = NULL;
    list.getItemString = NULL;
    list.itemHilited = NULL;
}

void read_from(ReadSource in, interfaceTextRectType& text_rect) {
    read(in, text_rect.textID);
    read(in, text_rect.visibleBounds);
}

void read_from(ReadSource in, interfaceButtonType& button) {
    read(in, button.label);
    read(in, button.key);
    read(in, button.defaultButton);
    read(in, button.status);
}

void read_from(ReadSource in, interfaceRadioType& radio) {
    read(in, radio.label);
    read(in, radio.key);
    read(in, radio.on);
    read(in, radio.status);
}

void read_from(ReadSource in, interfaceCheckboxType& checkbox) {
    read(in, checkbox.label);
    read(in, checkbox.key);
    read(in, checkbox.on);
    read(in, checkbox.status);
}

void read_from(ReadSource in, interfacePictureRectType& picture_rect) {
    read(in, picture_rect.pictureID);
    read(in, picture_rect.visibleBounds);
}

void read_from(sfz::ReadSource in, interfaceTabBoxType& tab_box) {
    read(in, tab_box.topRightBorderSize);
}

}  // namespace antares
