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

bool RealInputSource::next(EventReceiver& receiver) {
    for (const auto& event: _queue) {
        event->send(&receiver);
    }
    _queue.clear();
    return true;
}

void RealInputSource::key_down(const KeyDownEvent& event) {
    _queue.emplace_back(new KeyDownEvent(event.at(), event.key()));
}

void RealInputSource::key_up(const KeyUpEvent& event) {
    _queue.emplace_back(new KeyUpEvent(event.at(), event.key()));
}

void RealInputSource::gamepad_button_down(const GamepadButtonDownEvent& event) {
    _queue.emplace_back(new GamepadButtonDownEvent(event.at(), event.button));
}

void RealInputSource::gamepad_button_up(const GamepadButtonUpEvent& event) {
    _queue.emplace_back(new GamepadButtonUpEvent(event.at(), event.button));
}

void RealInputSource::gamepad_stick(const GamepadStickEvent& event) {
    _queue.emplace_back(new GamepadStickEvent(event.at(), event.stick, event.x, event.y));
}

void RealInputSource::mouse_down(const MouseDownEvent& event) {
    _queue.emplace_back(new MouseDownEvent(event.at(), event.button(), event.count(), event.where()));
}

void RealInputSource::mouse_up(const MouseUpEvent& event) {
    _queue.emplace_back(new MouseUpEvent(event.at(), event.button(), event.where()));
}

void RealInputSource::mouse_move(const MouseMoveEvent& event) {
    _queue.emplace_back(new MouseMoveEvent(event.at(), event.where()));
}

ReplayInputSource::ReplayInputSource(ReplayData* data):
        _exit(false),
        _data(data),
        _data_index(0),
        _at(0) {
    EventReceiver receiver;
    advance(receiver);
}

bool ReplayInputSource::next(EventReceiver& receiver) {
    if (_exit || !advance(receiver)) {
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
                int code = sys.prefs->key(key) - 1;
                receiver.key_down(KeyDownEvent(now(), code));
            }
            for (uint8_t key: action.keys_up) {
                int code = sys.prefs->key(key) - 1;
                receiver.key_up(KeyUpEvent(now(), code));
            }
        }
        ++_data_index;
    }
    ++_at;
    return true;
}

void ReplayInputSource::key_down(const KeyDownEvent& event) {
    _exit = true;
}

void ReplayInputSource::gamepad_button_down(const GamepadButtonDownEvent& event) {
    _exit = true;
}

void ReplayInputSource::mouse_down(const MouseDownEvent& event) {
    _exit = true;
}

}  // namespace antares
