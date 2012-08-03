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

#include "game/input-source.hpp"

#include <sfz/sfz.hpp>

#include "config/keys.hpp"
#include "config/preferences.hpp"
#include "data/replay.hpp"
#include "game/globals.hpp"
#include "video/driver.hpp"

using sfz::BytesSlice;
using sfz::read;

namespace antares {

InputSource::~InputSource() { }

UserInputSource::UserInputSource() { }

bool UserInputSource::next(KeyMap& key_map) {
    VideoDriver::driver()->get_keys(&key_map);
    return true;
}

ReplayInputSource::ReplayInputSource(ReplayData* data):
        _data(data),
        _data_index(0),
        _wait_ticks(0) {
    advance();
}

bool ReplayInputSource::next(KeyMap& key_map) {
    if (_wait_ticks == 0) {
        if (!advance()) {
            return false;
        }
    }
    --_wait_ticks;

    key_map.copy(_key_map);
    return true;
}

bool ReplayInputSource::advance() {
    while (_data_index < _data->items.size()) {
        const ReplayData::Item& item = _data->items[_data_index++];
        switch (item.type) {
          case ReplayData::Item::WAIT:
            _wait_ticks += item.data.wait;
            return true;
          case ReplayData::Item::KEY_DOWN:
            _key_map.set(Preferences::preferences()->key(item.data.key_down) - 1, true);
            break;
          case ReplayData::Item::KEY_UP:
            _key_map.set(Preferences::preferences()->key(item.data.key_up) - 1, false);
            break;
        }
    }
    return false;
}

}  // namespace antares
