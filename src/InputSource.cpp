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

#include "InputSource.hpp"

#include <assert.h>

InputSource::~InputSource() { }

ReplayInputSource::ReplayInputSource(int32_t id)
        : _resource('NLRP', id),
          _position(reinterpret_cast<const uint32_t*>(_resource.data())),
          _end(reinterpret_cast<const uint32_t*>(_resource.data() + _resource.size())) {
    assert(_resource.size() >= 3 * sizeof(uint32_t));
    _random_seed = *(_position++);
    _turn_num = *(_position++);
    _keys = *(_position++);
}

uint32_t ReplayInputSource::random_seed() const {
    return _random_seed;
}

bool ReplayInputSource::next(uint32_t* key_map) {
    while (_turn_num == 0) {
        if (_end < _position + 2) {
            return false;
        }
        _turn_num = *(_position++) + 1;
        _keys = *(_position++);
    }
    --_turn_num;
    *key_map = _keys;
    return true;
}
