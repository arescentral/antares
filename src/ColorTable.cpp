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

#include "ColorTable.hpp"

#include <stdint.h>
#include "sfz/BinaryReader.hpp"
#include "sfz/BinaryWriter.hpp"
#include "Base.h"

using sfz::BinaryReader;
using sfz::BinaryWriter;

namespace antares {

RgbColor colors[256] = {
    RgbColor(255, 255, 255),
    RgbColor(32, 0, 0),
    RgbColor(224, 224, 224),
    RgbColor(208, 208, 208),
    RgbColor(192, 192, 192),
    RgbColor(176, 176, 176),
    RgbColor(160, 160, 160),
    RgbColor(144, 144, 144),
    RgbColor(128, 128, 128),
    RgbColor(112, 112, 112),
    RgbColor(96, 96, 96),
    RgbColor(80, 80, 80),
    RgbColor(64, 64, 64),
    RgbColor(48, 48, 48),
    RgbColor(32, 32, 32),
    RgbColor(16, 16, 16),
    RgbColor(8, 8, 8),
    RgbColor(255, 127, 0),
    RgbColor(240, 120, 0),
    RgbColor(224, 112, 0),
    RgbColor(208, 104, 0),
    RgbColor(192, 96, 0),
    RgbColor(176, 88, 0),
    RgbColor(160, 80, 0),
    RgbColor(144, 72, 0),
    RgbColor(128, 64, 0),
    RgbColor(112, 56, 0),
    RgbColor(96, 48, 0),
    RgbColor(80, 40, 0),
    RgbColor(64, 32, 0),
    RgbColor(48, 24, 0),
    RgbColor(32, 16, 0),
    RgbColor(16, 8, 0),
    RgbColor(255, 255, 0),
    RgbColor(240, 240, 0),
    RgbColor(224, 224, 0),
    RgbColor(208, 208, 0),
    RgbColor(192, 192, 0),
    RgbColor(176, 176, 0),
    RgbColor(160, 160, 0),
    RgbColor(144, 144, 0),
    RgbColor(128, 128, 0),
    RgbColor(112, 112, 0),
    RgbColor(96, 96, 0),
    RgbColor(80, 80, 0),
    RgbColor(64, 64, 0),
    RgbColor(48, 48, 0),
    RgbColor(32, 32, 0),
    RgbColor(16, 16, 0),
    RgbColor(0, 0, 255),
    RgbColor(0, 0, 240),
    RgbColor(0, 0, 224),
    RgbColor(0, 0, 208),
    RgbColor(0, 0, 192),
    RgbColor(0, 0, 176),
    RgbColor(0, 0, 160),
    RgbColor(0, 0, 144),
    RgbColor(0, 0, 128),
    RgbColor(0, 0, 112),
    RgbColor(0, 0, 96),
    RgbColor(0, 0, 80),
    RgbColor(0, 0, 64),
    RgbColor(0, 0, 48),
    RgbColor(0, 0, 32),
    RgbColor(0, 0, 16),
    RgbColor(0, 255, 0),
    RgbColor(0, 240, 0),
    RgbColor(0, 224, 0),
    RgbColor(0, 208, 0),
    RgbColor(0, 192, 0),
    RgbColor(0, 176, 0),
    RgbColor(0, 160, 0),
    RgbColor(0, 144, 0),
    RgbColor(0, 128, 0),
    RgbColor(0, 112, 0),
    RgbColor(0, 96, 0),
    RgbColor(0, 80, 0),
    RgbColor(0, 64, 0),
    RgbColor(0, 48, 0),
    RgbColor(0, 32, 0),
    RgbColor(0, 16, 0),
    RgbColor(127, 0, 255),
    RgbColor(120, 0, 240),
    RgbColor(112, 0, 224),
    RgbColor(104, 0, 208),
    RgbColor(96, 0, 192),
    RgbColor(88, 0, 176),
    RgbColor(80, 0, 160),
    RgbColor(72, 0, 144),
    RgbColor(64, 0, 128),
    RgbColor(56, 0, 112),
    RgbColor(48, 0, 96),
    RgbColor(40, 0, 80),
    RgbColor(32, 0, 64),
    RgbColor(24, 0, 48),
    RgbColor(16, 0, 32),
    RgbColor(8, 0, 16),
    RgbColor(127, 127, 255),
    RgbColor(120, 120, 240),
    RgbColor(112, 112, 224),
    RgbColor(104, 104, 208),
    RgbColor(96, 96, 192),
    RgbColor(88, 88, 176),
    RgbColor(80, 80, 160),
    RgbColor(72, 72, 144),
    RgbColor(64, 64, 128),
    RgbColor(56, 56, 112),
    RgbColor(48, 48, 96),
    RgbColor(40, 40, 80),
    RgbColor(32, 32, 64),
    RgbColor(24, 24, 48),
    RgbColor(16, 16, 32),
    RgbColor(8, 8, 16),
    RgbColor(255, 127, 127),
    RgbColor(240, 120, 120),
    RgbColor(224, 112, 112),
    RgbColor(208, 104, 104),
    RgbColor(192, 96, 96),
    RgbColor(176, 88, 88),
    RgbColor(160, 80, 80),
    RgbColor(144, 72, 72),
    RgbColor(128, 64, 64),
    RgbColor(112, 56, 56),
    RgbColor(96, 48, 48),
    RgbColor(80, 40, 40),
    RgbColor(64, 32, 32),
    RgbColor(48, 24, 24),
    RgbColor(32, 16, 16),
    RgbColor(16, 8, 8),
    RgbColor(255, 255, 127),
    RgbColor(240, 240, 120),
    RgbColor(224, 224, 112),
    RgbColor(208, 208, 104),
    RgbColor(192, 192, 96),
    RgbColor(176, 176, 88),
    RgbColor(160, 160, 80),
    RgbColor(144, 144, 72),
    RgbColor(128, 128, 64),
    RgbColor(112, 112, 56),
    RgbColor(96, 96, 48),
    RgbColor(80, 80, 40),
    RgbColor(64, 64, 32),
    RgbColor(48, 48, 24),
    RgbColor(32, 32, 16),
    RgbColor(16, 16, 8),
    RgbColor(0, 255, 255),
    RgbColor(0, 240, 240),
    RgbColor(0, 224, 224),
    RgbColor(0, 208, 208),
    RgbColor(0, 192, 192),
    RgbColor(0, 176, 176),
    RgbColor(0, 160, 160),
    RgbColor(0, 144, 144),
    RgbColor(0, 128, 128),
    RgbColor(0, 112, 112),
    RgbColor(0, 96, 96),
    RgbColor(0, 80, 80),
    RgbColor(0, 64, 64),
    RgbColor(0, 48, 48),
    RgbColor(0, 32, 32),
    RgbColor(0, 16, 16),
    RgbColor(255, 0, 127),
    RgbColor(240, 0, 120),
    RgbColor(224, 0, 112),
    RgbColor(208, 0, 104),
    RgbColor(192, 0, 96),
    RgbColor(176, 0, 88),
    RgbColor(160, 0, 80),
    RgbColor(144, 0, 72),
    RgbColor(128, 0, 64),
    RgbColor(112, 0, 56),
    RgbColor(96, 0, 48),
    RgbColor(80, 0, 40),
    RgbColor(64, 0, 32),
    RgbColor(48, 0, 24),
    RgbColor(32, 0, 16),
    RgbColor(16, 0, 8),
    RgbColor(127, 255, 127),
    RgbColor(120, 240, 120),
    RgbColor(112, 224, 112),
    RgbColor(104, 208, 104),
    RgbColor(96, 192, 96),
    RgbColor(88, 176, 88),
    RgbColor(80, 160, 80),
    RgbColor(72, 144, 72),
    RgbColor(64, 128, 64),
    RgbColor(56, 112, 56),
    RgbColor(48, 96, 48),
    RgbColor(40, 80, 40),
    RgbColor(32, 64, 32),
    RgbColor(24, 48, 24),
    RgbColor(16, 32, 16),
    RgbColor(8, 16, 8),
    RgbColor(255, 127, 255),
    RgbColor(240, 120, 240),
    RgbColor(224, 112, 224),
    RgbColor(208, 104, 208),
    RgbColor(192, 96, 192),
    RgbColor(176, 88, 176),
    RgbColor(160, 80, 160),
    RgbColor(144, 72, 143),
    RgbColor(128, 64, 128),
    RgbColor(112, 56, 112),
    RgbColor(96, 48, 96),
    RgbColor(80, 40, 80),
    RgbColor(64, 32, 64),
    RgbColor(48, 24, 48),
    RgbColor(32, 16, 32),
    RgbColor(16, 8, 16),
    RgbColor(0, 127, 255),
    RgbColor(0, 120, 240),
    RgbColor(0, 112, 224),
    RgbColor(0, 104, 208),
    RgbColor(0, 96, 192),
    RgbColor(0, 88, 176),
    RgbColor(0, 80, 160),
    RgbColor(0, 72, 143),
    RgbColor(0, 64, 128),
    RgbColor(0, 56, 112),
    RgbColor(0, 48, 96),
    RgbColor(0, 40, 80),
    RgbColor(0, 32, 64),
    RgbColor(0, 24, 48),
    RgbColor(0, 16, 32),
    RgbColor(0, 8, 16),
    RgbColor(255, 249, 207),
    RgbColor(240, 234, 195),
    RgbColor(225, 220, 183),
    RgbColor(210, 205, 171),
    RgbColor(195, 190, 159),
    RgbColor(180, 176, 146),
    RgbColor(165, 161, 134),
    RgbColor(150, 146, 122),
    RgbColor(135, 132, 110),
    RgbColor(120, 117, 97),
    RgbColor(105, 102, 85),
    RgbColor(90, 88, 73),
    RgbColor(75, 73, 61),
    RgbColor(60, 58, 48),
    RgbColor(45, 44, 36),
    RgbColor(30, 29, 24),
    RgbColor(255, 0, 0),
    RgbColor(240, 0, 0),
    RgbColor(225, 0, 0),
    RgbColor(208, 0, 0),
    RgbColor(192, 0, 0),
    RgbColor(176, 0, 0),
    RgbColor(160, 0, 0),
    RgbColor(144, 0, 0),
    RgbColor(128, 0, 0),
    RgbColor(112, 0, 0),
    RgbColor(96, 0, 0),
    RgbColor(80, 0, 0),
    RgbColor(64, 0, 0),
    RgbColor(48, 0, 0),
    RgbColor(0, 0, 0),
};

const RgbColor RgbColor::kBlack(0xFF, 0x00, 0x00, 0x00);
const RgbColor RgbColor::kWhite(0xFF, 0xFF, 0xFF, 0xFF);
const RgbColor RgbColor::kClear(0x00, 0x00, 0x00, 0x00);

RgbColor::RgbColor()
        : alpha(0xFF),
          red(0x00),
          green(0x00),
          blue(0x00) { }

RgbColor::RgbColor(uint8_t red, uint8_t green, uint8_t blue)
        : alpha(0xFF),
          red(red),
          green(green),
          blue(blue) { }

RgbColor::RgbColor(uint8_t alpha, uint8_t red, uint8_t green, uint8_t blue)
        : alpha(alpha),
          red(red),
          green(green),
          blue(blue) { }

void RgbColor::read(BinaryReader* bin) {
    bin->discard(2);
    bin->read(&red);
    bin->discard(3);
    bin->read(&green);
    bin->discard(3);
    bin->read(&blue);
    bin->discard(1);
}

void RgbColor::write(BinaryWriter* bin) const {
    bin->pad(2);
    bin->write(red);
    bin->pad(3);
    bin->write(green);
    bin->pad(3);
    bin->write(blue);
    bin->pad(1);
}

ColorTable::ColorTable(int32_t id) {
    static_cast<void>(id);
    for (int i = 0; i < 256; ++i) {
        _colors.push_back(colors[i]);
    }
}

ColorTable* ColorTable::clone() const {
    return new ColorTable(256);
}

size_t ColorTable::size() const {
    return _colors.size();
}

const RgbColor& ColorTable::color(size_t index) const {
    return _colors[index];
}

void ColorTable::set_color(size_t index, const RgbColor& color) {
    _colors[index] = color;
}

void ColorTable::transition_between(
        const ColorTable& source, const RgbColor& dest, double fraction) {
    double source_fraction = 1 - fraction;
    double dest_fraction = fraction;
    for (int i = 0; i < 256; ++i) {
        RgbColor out(
            source.color(i).red * source_fraction + dest.red * dest_fraction,
            source.color(i).green * source_fraction + dest.green * dest_fraction,
            source.color(i).blue * source_fraction + dest.blue * dest_fraction);
        _colors[i] = out;
    }
}

void ColorTable::read(BinaryReader* bin) {
    for (int i = 0; i < 256; ++i) {
        uint32_t index;
        bin->read(&index);
        bin->read(&_colors[i]);
    }
}

void ColorTable::write(BinaryWriter* bin) const {
    for (int i = 0; i < 256; ++i) {
        uint32_t index = i;
        bin->write(index);
        bin->write(_colors[i]);
    }
}

}  // namespace antares
