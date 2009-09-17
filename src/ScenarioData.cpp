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

#include "ScenarioData.hpp"

#include "Scenario.hpp"

size_t scenarioInfoType::load_data(const char* data, size_t len) {
    assert(len >= sizeof(scenarioInfoType));
    memcpy(this, data, sizeof(scenarioInfoType));
    return sizeof(scenarioInfoType);
}

size_t scenarioType::load_data(const char* data, size_t len) {
    assert(len >= sizeof(scenarioType));
    memcpy(this, data, sizeof(scenarioType));
    return sizeof(scenarioType);
}

size_t scenarioConditionType::load_data(const char* data, size_t len) {
    assert(len >= sizeof(scenarioConditionType));
    memcpy(this, data, sizeof(scenarioConditionType));
    return sizeof(scenarioConditionType);
}

size_t briefPointType::load_data(const char* data, size_t len) {
    assert(len >= sizeof(briefPointType));
    memcpy(this, data, sizeof(briefPointType));
    return sizeof(briefPointType);
}

size_t scenarioInitialType::load_data(const char* data, size_t len) {
    assert(len >= sizeof(scenarioInitialType));
    memcpy(this, data, sizeof(scenarioInitialType));
    return sizeof(scenarioInitialType);
}
