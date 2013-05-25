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

#include "drawing/color.hpp"

#include <stdint.h>
#include <sfz/sfz.hpp>

using sfz::ReadSource;
using sfz::WriteTarget;
using sfz::format;
using sfz::read;
using sfz::write;

namespace antares {

static RgbColor colors[256] = {
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

static uint8_t diffuse[][3] = {
    {255,    255,    255},
    {255,    128,      0},
    {255,    255,      0},
    {  0,      0,    255},
    {  0,    255,      0},
    {128,      0,    255},
    {128,    128,    255},
    {255,    128,    128},
    {255,    255,    128},
    {  0,    255,    255},
    {255,      0,    128},
    {128,    255,    128},
    {255,    128,    255},
    {  0,    128,    255},
    {239,    233,    195},
    {255,      0,      0},
};

static uint8_t ambient[][3] = {
    { 0,  0,  0},
    { 0,  0,  0},
    { 0,  0,  0},
    { 0,  0,  0},
    { 0,  0,  0},
    { 0,  0,  0},
    { 0,  0,  0},
    { 0,  0,  0},
    { 0,  0,  0},
    { 0,  0,  0},
    { 0,  0,  0},
    { 0,  0,  0},
    { 0,  0,  0},
    { 0,  0,  0},
    {16, 15, 12},
    { 0,  0,  0},
};

RgbColor RgbColor::tint(uint8_t color, uint8_t value) {
    return {
        (diffuse[color][0] * value / 255) + ambient[color][0],
        (diffuse[color][1] * value / 255) + ambient[color][1],
        (diffuse[color][2] * value / 255) + ambient[color][2],
    };
}

const RgbColor& RgbColor::at(uint8_t index) {
    return colors[index];
}

void read_from(ReadSource in, RgbColor& color) {
    in.shift(2);
    read(in, color.red);
    in.shift(3);
    read(in, color.green);
    in.shift(3);
    read(in, color.blue);
    in.shift(1);
}

void write_to(WriteTarget out, const RgbColor& color) {
    out.push(2, '\0');
    write(out, color.red);
    out.push(3, '\0');
    write(out, color.green);
    out.push(3, '\0');
    write(out, color.blue);
    out.push(1, '\0');
}

void print_to(sfz::PrintTarget out, const RgbColor& color) {
    if (color.alpha == 0xff) {
        print(out, format("rgb({0}, {1}, {2})", color.red, color.green, color.blue));
    } else {
        print(out, format(
                    "rgba({0}, {1}, {2}, {3})",
                    color.red, color.green, color.blue, color.alpha));
    }
}

uint8_t GetRetroIndex(uint8_t which) {
    return which;
}

uint8_t GetTranslateIndex(uint8_t which) {
    return which;
}

uint8_t GetTranslateColorShade(uint8_t color, uint8_t shade) {
    return (17 - shade) + (color * 16);
}

void GetRGBTranslateColorShade(RgbColor *c, uint8_t color, uint8_t shade) {
    *c = RgbColor::at(GetTranslateColorShade(color, shade));
}

RgbColor GetRGBTranslateColorShade(uint8_t color, uint8_t shade) {
    RgbColor result;
    GetRGBTranslateColorShade(&result, color, shade);
    return result;
}

void GetRGBTranslateColor(RgbColor *c, uint8_t color) {
    *c = RgbColor::at(color);
}

RgbColor GetRGBTranslateColor(uint8_t color) {
    RgbColor result;
    GetRGBTranslateColor(&result, color);
    return result;
}

}  // namespace antares
