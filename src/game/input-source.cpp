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

#include "game/input-source.hpp"

#include <sfz/sfz.hpp>

#include "config/keys.hpp"
#include "config/preferences.hpp"
#include "data/replay.hpp"
#include "game/globals.hpp"
#include "game/time.hpp"

using sfz::BytesSlice;
using sfz::range;
using sfz::read;
using std::make_pair;

namespace antares {

InputSource::~InputSource() {}

void RealInputSource::start() {
    _events.clear();
}

bool RealInputSource::get(Handle<Admiral> admiral, game_ticks at, EventReceiver& receiver) {
    auto events = _events.equal_range(make_pair(admiral.number(), at));
    for (auto it : range(events.first, events.second)) {
        it->second->send(&receiver);
    }
    return true;
}

void RealInputSource::key_down(const KeyDownEvent& event) {
    _events.emplace(
            make_pair(g.admiral.number(), at()),
            std::unique_ptr<Event>(new KeyDownEvent(event.at(), event.key())));
}

void RealInputSource::key_up(const KeyUpEvent& event) {
    _events.emplace(
            make_pair(g.admiral.number(), at()),
            std::unique_ptr<Event>(new KeyUpEvent(event.at(), event.key())));
}

void RealInputSource::gamepad_button_down(const GamepadButtonDownEvent& event) {
    _events.emplace(
            make_pair(g.admiral.number(), at()),
            std::unique_ptr<Event>(new GamepadButtonDownEvent(event.at(), event.button)));
}

void RealInputSource::gamepad_button_up(const GamepadButtonUpEvent& event) {
    _events.emplace(
            make_pair(g.admiral.number(), at()),
            std::unique_ptr<Event>(new GamepadButtonUpEvent(event.at(), event.button)));
}

void RealInputSource::gamepad_stick(const GamepadStickEvent& event) {
    _events.emplace(
            make_pair(g.admiral.number(), at()),
            std::unique_ptr<Event>(
                    new GamepadStickEvent(event.at(), event.stick, event.x, event.y)));
}

void RealInputSource::mouse_down(const MouseDownEvent& event) {
    _events.emplace(
            make_pair(g.admiral.number(), at()),
            std::unique_ptr<Event>(
                    new MouseDownEvent(event.at(), event.button(), event.count(), event.where())));
}

void RealInputSource::mouse_up(const MouseUpEvent& event) {
    _events.emplace(
            make_pair(g.admiral.number(), at()),
            std::unique_ptr<Event>(new MouseUpEvent(event.at(), event.button(), event.where())));
}

void RealInputSource::mouse_move(const MouseMoveEvent& event) {
    _events.emplace(
            make_pair(g.admiral.number(), at()),
            std::unique_ptr<Event>(new MouseMoveEvent(event.at(), event.where())));
}

game_ticks RealInputSource::at() {
    game_ticks result = g.time + ticks(1);
    while ((result.time_since_epoch() % kMajorTick).count()) {
        result += ticks(1);
    }
    return result;
}

ReplayInputSource::ReplayInputSource(ReplayData* data)
        : _duration(game_ticks(ticks(data->duration * 3))), _exit(false) {
    for (auto action : data->actions) {
        game_ticks at = game_ticks(ticks(action.at * 3));
        for (auto key : action.keys_down) {
            int code = sys.prefs->key(key) - 1;
            _events.emplace(
                    make_pair(0, at), unique_ptr<Event>(new KeyDownEvent(wall_time(), code)));
        }
        for (auto key : action.keys_up) {
            int code = sys.prefs->key(key) - 1;
            _events.emplace(
                    make_pair(0, at), unique_ptr<Event>(new KeyUpEvent(wall_time(), code)));
        }
    }
}

void ReplayInputSource::start() {}

bool ReplayInputSource::get(Handle<Admiral> admiral, game_ticks at, EventReceiver& receiver) {
    if (_exit || (at >= _duration)) {
        return false;
    }
    auto events = _events.equal_range(make_pair(admiral.number(), at));
    for (auto it : range(events.first, events.second)) {
        it->second->send(&receiver);
    }
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
