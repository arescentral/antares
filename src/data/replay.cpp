// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2011 Ares Central
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
// License along with this program.  If not, see
// <http://www.gnu.org/licenses/>.

#include "data/replay.hpp"

#include <sfz/sfz.hpp>

using sfz::Exception;
using sfz::ReadSource;
using sfz::WriteTarget;
using sfz::format;
using sfz::range;
using sfz::read;
using sfz::write;

namespace antares {

ReplayData::ReplayData() {
}

ReplayData::ReplayData(sfz::BytesSlice in) {
    read(in, *this);
}

void ReplayData::wait(uint32_t ticks) {
    Item item;
    item.type = Item::WAIT;
    item.data.wait = ticks;
    items.push_back(item);
}

void ReplayData::key_down(uint32_t key) {
    Item item;
    item.type = Item::KEY_DOWN;
    item.data.key_down = key;
    items.push_back(item);
}

void ReplayData::key_up(uint32_t key) {
    Item item;
    item.type = Item::KEY_UP;
    item.data.key_up = key;
    items.push_back(item);
}

void read_from(ReadSource in, ReplayData& replay) {
    read(in, replay.chapter_id);
    read(in, replay.global_seed);
    const uint32_t item_count = read<uint32_t>(in);
    SFZ_FOREACH(int i, range(item_count), {
        ReplayData::Item item;
        read(in, item);
        replay.items.push_back(item);
    });
}

void read_from(ReadSource in, ReplayData::Item& item) {
    const uint8_t type = read<uint8_t>(in);
    switch (type) {
      case ReplayData::Item::WAIT:
        item.type = ReplayData::Item::WAIT;
        read(in, item.data.wait);
        break;
      case ReplayData::Item::KEY_DOWN:
        item.type = ReplayData::Item::KEY_DOWN;
        read(in, item.data.key_down);
        break;
      case ReplayData::Item::KEY_UP:
        item.type = ReplayData::Item::KEY_UP;
        read(in, item.data.key_up);
        break;
      default:
        throw Exception(format("Illegal replay item type {0}", type));
    }
}

void write_to(WriteTarget out, const ReplayData& replay) {
    write(out, replay.chapter_id);
    write(out, replay.global_seed);
    write<uint32_t>(out, replay.items.size());
    SFZ_FOREACH(const ReplayData::Item& item, replay.items, {
        write(out, item);
    });
}

void write_to(WriteTarget out, const ReplayData::Item& item) {
    write<uint8_t>(out, item.type);
    switch (item.type) {
      case ReplayData::Item::WAIT:
        write(out, item.data.wait);
        break;
      case ReplayData::Item::KEY_DOWN:
        write(out, item.data.key_down);
        break;
      case ReplayData::Item::KEY_UP:
        write(out, item.data.key_up);
        break;
    }
}

}  // namespace antares
