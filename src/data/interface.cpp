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

}  // namespace antares
