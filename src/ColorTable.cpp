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
#include "Base.h"
#include "BinaryStream.hpp"
#include "SmartPtr.hpp"

namespace antares {

struct Color24Bit {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

Color24Bit colors_24_bit[256] = {
    {255, 255, 255},
    {32, 0, 0},
    {224, 224, 224},
    {208, 208, 208},
    {192, 192, 192},
    {176, 176, 176},
    {160, 160, 160},
    {144, 144, 144},
    {128, 128, 128},
    {112, 112, 112},
    {96, 96, 96},
    {80, 80, 80},
    {64, 64, 64},
    {48, 48, 48},
    {32, 32, 32},
    {16, 16, 16},
    {8, 8, 8},
    {255, 127, 0},
    {240, 120, 0},
    {224, 112, 0},
    {208, 104, 0},
    {192, 96, 0},
    {176, 88, 0},
    {160, 80, 0},
    {144, 72, 0},
    {128, 64, 0},
    {112, 56, 0},
    {96, 48, 0},
    {80, 40, 0},
    {64, 32, 0},
    {48, 24, 0},
    {32, 16, 0},
    {16, 8, 0},
    {255, 255, 0},
    {240, 240, 0},
    {224, 224, 0},
    {208, 208, 0},
    {192, 192, 0},
    {176, 176, 0},
    {160, 160, 0},
    {144, 144, 0},
    {128, 128, 0},
    {112, 112, 0},
    {96, 96, 0},
    {80, 80, 0},
    {64, 64, 0},
    {48, 48, 0},
    {32, 32, 0},
    {16, 16, 0},
    {0, 0, 255},
    {0, 0, 240},
    {0, 0, 224},
    {0, 0, 208},
    {0, 0, 192},
    {0, 0, 176},
    {0, 0, 160},
    {0, 0, 144},
    {0, 0, 128},
    {0, 0, 112},
    {0, 0, 96},
    {0, 0, 80},
    {0, 0, 64},
    {0, 0, 48},
    {0, 0, 32},
    {0, 0, 16},
    {0, 255, 0},
    {0, 240, 0},
    {0, 224, 0},
    {0, 208, 0},
    {0, 192, 0},
    {0, 176, 0},
    {0, 160, 0},
    {0, 144, 0},
    {0, 128, 0},
    {0, 112, 0},
    {0, 96, 0},
    {0, 80, 0},
    {0, 64, 0},
    {0, 48, 0},
    {0, 32, 0},
    {0, 16, 0},
    {127, 0, 255},
    {120, 0, 240},
    {112, 0, 224},
    {104, 0, 208},
    {96, 0, 192},
    {88, 0, 176},
    {80, 0, 160},
    {72, 0, 144},
    {64, 0, 128},
    {56, 0, 112},
    {48, 0, 96},
    {40, 0, 80},
    {32, 0, 64},
    {24, 0, 48},
    {16, 0, 32},
    {8, 0, 16},
    {127, 127, 255},
    {120, 120, 240},
    {112, 112, 224},
    {104, 104, 208},
    {96, 96, 192},
    {88, 88, 176},
    {80, 80, 160},
    {72, 72, 144},
    {64, 64, 128},
    {56, 56, 112},
    {48, 48, 96},
    {40, 40, 80},
    {32, 32, 64},
    {24, 24, 48},
    {16, 16, 32},
    {8, 8, 16},
    {255, 127, 127},
    {240, 120, 120},
    {224, 112, 112},
    {208, 104, 104},
    {192, 96, 96},
    {176, 88, 88},
    {160, 80, 80},
    {144, 72, 72},
    {128, 64, 64},
    {112, 56, 56},
    {96, 48, 48},
    {80, 40, 40},
    {64, 32, 32},
    {48, 24, 24},
    {32, 16, 16},
    {16, 8, 8},
    {255, 255, 127},
    {240, 240, 120},
    {224, 224, 112},
    {208, 208, 104},
    {192, 192, 96},
    {176, 176, 88},
    {160, 160, 80},
    {144, 144, 72},
    {128, 128, 64},
    {112, 112, 56},
    {96, 96, 48},
    {80, 80, 40},
    {64, 64, 32},
    {48, 48, 24},
    {32, 32, 16},
    {16, 16, 8},
    {0, 255, 255},
    {0, 240, 240},
    {0, 224, 224},
    {0, 208, 208},
    {0, 192, 192},
    {0, 176, 176},
    {0, 160, 160},
    {0, 144, 144},
    {0, 128, 128},
    {0, 112, 112},
    {0, 96, 96},
    {0, 80, 80},
    {0, 64, 64},
    {0, 48, 48},
    {0, 32, 32},
    {0, 16, 16},
    {255, 0, 127},
    {240, 0, 120},
    {224, 0, 112},
    {208, 0, 104},
    {192, 0, 96},
    {176, 0, 88},
    {160, 0, 80},
    {144, 0, 72},
    {128, 0, 64},
    {112, 0, 56},
    {96, 0, 48},
    {80, 0, 40},
    {64, 0, 32},
    {48, 0, 24},
    {32, 0, 16},
    {16, 0, 8},
    {127, 255, 127},
    {120, 240, 120},
    {112, 224, 112},
    {104, 208, 104},
    {96, 192, 96},
    {88, 176, 88},
    {80, 160, 80},
    {72, 144, 72},
    {64, 128, 64},
    {56, 112, 56},
    {48, 96, 48},
    {40, 80, 40},
    {32, 64, 32},
    {24, 48, 24},
    {16, 32, 16},
    {8, 16, 8},
    {255, 127, 255},
    {240, 120, 240},
    {224, 112, 224},
    {208, 104, 208},
    {192, 96, 192},
    {176, 88, 176},
    {160, 80, 160},
    {144, 72, 143},
    {128, 64, 128},
    {112, 56, 112},
    {96, 48, 96},
    {80, 40, 80},
    {64, 32, 64},
    {48, 24, 48},
    {32, 16, 32},
    {16, 8, 16},
    {0, 127, 255},
    {0, 120, 240},
    {0, 112, 224},
    {0, 104, 208},
    {0, 96, 192},
    {0, 88, 176},
    {0, 80, 160},
    {0, 72, 143},
    {0, 64, 128},
    {0, 56, 112},
    {0, 48, 96},
    {0, 40, 80},
    {0, 32, 64},
    {0, 24, 48},
    {0, 16, 32},
    {0, 8, 16},
    {255, 249, 207},
    {240, 234, 195},
    {225, 220, 183},
    {210, 205, 171},
    {195, 190, 159},
    {180, 176, 146},
    {165, 161, 134},
    {150, 146, 122},
    {135, 132, 110},
    {120, 117, 97},
    {105, 102, 85},
    {90, 88, 73},
    {75, 73, 61},
    {60, 58, 48},
    {45, 44, 36},
    {30, 29, 24},
    {255, 0, 0},
    {240, 0, 0},
    {225, 0, 0},
    {208, 0, 0},
    {192, 0, 0},
    {176, 0, 0},
    {160, 0, 0},
    {144, 0, 0},
    {128, 0, 0},
    {112, 0, 0},
    {96, 0, 0},
    {80, 0, 0},
    {64, 0, 0},
    {48, 0, 0},
    {0, 0, 0},
};

ColorTable::ColorTable(int32_t id) {
    static_cast<void>(id);
    for (int i = 0; i < 256; ++i) {
        RGBColor color = {
            colors_24_bit[i].red * 0x101,
            colors_24_bit[i].green * 0x101,
            colors_24_bit[i].blue * 0x101,
        };
        _colors.push_back(color);
    }
}

ColorTable* ColorTable::clone() const {
    return new ColorTable(256);
}

size_t ColorTable::size() const {
    return _colors.size();
}

const RGBColor& ColorTable::color(size_t index) const {
    return _colors[index];
}

void ColorTable::set_color(size_t index, const RGBColor& color) {
    _colors[index] = color;
}

void ColorTable::transition_between(
        const ColorTable& source, const RGBColor& dest, double fraction) {
    double source_fraction = 1 - fraction;
    double dest_fraction = fraction;
    for (int i = 0; i < 256; ++i) {
        RGBColor out = {
            source.color(i).red * source_fraction + dest.red * dest_fraction,
            source.color(i).green * source_fraction + dest.green * dest_fraction,
            source.color(i).blue * source_fraction + dest.blue * dest_fraction,
        };
        _colors[i] = out;
    }
}

void ColorTable::read(BinaryReader* bin) {
    for (int i = 0; i < 256; ++i) {
        uint32_t index;
        bin->read(&index);
        bin->read(&_colors[i].red);
        bin->read(&_colors[i].green);
        bin->read(&_colors[i].blue);
    }
}

void ColorTable::write(BinaryWriter* bin) const {
    for (int i = 0; i < 256; ++i) {
        uint32_t index = i;
        bin->write(index);
        bin->write(_colors[i].red);
        bin->write(_colors[i].green);
        bin->write(_colors[i].blue);
    }
}

}  // namespace antares
