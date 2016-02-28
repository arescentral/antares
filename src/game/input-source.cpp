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
        _at(0) {
    EventReceiver receiver;
    advance(receiver);
}

bool ReplayInputSource::next(EventReceiver& receiver) {
    if (!advance(receiver)) {
        return false;
    }
    return true;
}

bool ReplayInputSource::advance(EventReceiver& receiver) {
    if (_at >= _data->duration) {
        return false;
    }
    while ((_data_index < _data->actions.size())
            && (_at >= _data->actions[_data_index].at)) {
        const ReplayData::Action& action = _data->actions[_data_index];
        if (_at == action.at) {
            for (uint8_t key: action.keys_down) {
                int code = Preferences::preferences()->key(key) - 1;
                receiver.key_down(KeyDownEvent(now_usecs().time_since_epoch().count(), code));
            }
            for (uint8_t key: action.keys_up) {
                int code = Preferences::preferences()->key(key) - 1;
                receiver.key_up(KeyUpEvent(now_usecs().time_since_epoch().count(), code));
            }
        }
        ++_data_index;
    }
    ++_at;
    return true;
}

}  // namespace antares
