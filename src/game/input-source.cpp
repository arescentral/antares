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
#include "game/time.hpp"

using sfz::BytesSlice;
using sfz::read;

namespace antares {

InputSource::~InputSource() { }

ReplayInputSource::ReplayInputSource(ReplayData* data):
        _data(data),
        _data_index(0),
        _wait_ticks(0) {
    EventReceiver receiver;
    advance(receiver);
}

bool ReplayInputSource::next(EventReceiver& receiver) {
    if (_wait_ticks == 0) {
        if (!advance(receiver)) {
            return false;
        }
    }
    --_wait_ticks;
    return true;
}

bool ReplayInputSource::advance(EventReceiver& receiver) {
    int key;
    while (_data_index < _data->items.size()) {
        const ReplayData::Item& item = _data->items[_data_index++];
        switch (item.type) {
          case ReplayData::Item::WAIT:
            _wait_ticks += item.data.wait;
            return true;
          case ReplayData::Item::KEY_DOWN:
            key = Preferences::preferences()->key(item.data.key_down) - 1;
            receiver.key_down(KeyDownEvent(now_usecs(), key));
            break;
          case ReplayData::Item::KEY_UP:
            key = Preferences::preferences()->key(item.data.key_up) - 1;
            receiver.key_up(KeyUpEvent(now_usecs(), key));
            break;
        }
    }
    return false;
}

}  // namespace antares
