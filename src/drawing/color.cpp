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

#include "drawing/color.hpp"

#include <stdint.h>
#include <pn/file>

#include "lang/casts.hpp"
#include "lang/defines.hpp"

namespace antares {

static const RgbColor kColors[256] = {
        rgb(255, 255, 255), rgb(32, 0, 0),      rgb(224, 224, 224), rgb(208, 208, 208),
        rgb(192, 192, 192), rgb(176, 176, 176), rgb(160, 160, 160), rgb(144, 144, 144),
        rgb(128, 128, 128), rgb(112, 112, 112), rgb(96, 96, 96),    rgb(80, 80, 80),
        rgb(64, 64, 64),    rgb(48, 48, 48),    rgb(32, 32, 32),    rgb(16, 16, 16),

        rgb(8, 8, 8),       rgb(255, 127, 0),   rgb(240, 120, 0),   rgb(224, 112, 0),
        rgb(208, 104, 0),   rgb(192, 96, 0),    rgb(176, 88, 0),    rgb(160, 80, 0),
        rgb(144, 72, 0),    rgb(128, 64, 0),    rgb(112, 56, 0),    rgb(96, 48, 0),
        rgb(80, 40, 0),     rgb(64, 32, 0),     rgb(48, 24, 0),     rgb(32, 16, 0),

        rgb(16, 8, 0),      rgb(255, 255, 0),   rgb(240, 240, 0),   rgb(224, 224, 0),
        rgb(208, 208, 0),   rgb(192, 192, 0),   rgb(176, 176, 0),   rgb(160, 160, 0),
        rgb(144, 144, 0),   rgb(128, 128, 0),   rgb(112, 112, 0),   rgb(96, 96, 0),
        rgb(80, 80, 0),     rgb(64, 64, 0),     rgb(48, 48, 0),     rgb(32, 32, 0),

        rgb(16, 16, 0),     rgb(0, 0, 255),     rgb(0, 0, 240),     rgb(0, 0, 224),
        rgb(0, 0, 208),     rgb(0, 0, 192),     rgb(0, 0, 176),     rgb(0, 0, 160),
        rgb(0, 0, 144),     rgb(0, 0, 128),     rgb(0, 0, 112),     rgb(0, 0, 96),
        rgb(0, 0, 80),      rgb(0, 0, 64),      rgb(0, 0, 48),      rgb(0, 0, 32),

        rgb(0, 0, 16),      rgb(0, 255, 0),     rgb(0, 240, 0),     rgb(0, 224, 0),
        rgb(0, 208, 0),     rgb(0, 192, 0),     rgb(0, 176, 0),     rgb(0, 160, 0),
        rgb(0, 144, 0),     rgb(0, 128, 0),     rgb(0, 112, 0),     rgb(0, 96, 0),
        rgb(0, 80, 0),      rgb(0, 64, 0),      rgb(0, 48, 0),      rgb(0, 32, 0),

        rgb(0, 16, 0),      rgb(127, 0, 255),   rgb(120, 0, 240),   rgb(112, 0, 224),
        rgb(104, 0, 208),   rgb(96, 0, 192),    rgb(88, 0, 176),    rgb(80, 0, 160),
        rgb(72, 0, 144),    rgb(64, 0, 128),    rgb(56, 0, 112),    rgb(48, 0, 96),
        rgb(40, 0, 80),     rgb(32, 0, 64),     rgb(24, 0, 48),     rgb(16, 0, 32),

        rgb(8, 0, 16),      rgb(127, 127, 255), rgb(120, 120, 240), rgb(112, 112, 224),
        rgb(104, 104, 208), rgb(96, 96, 192),   rgb(88, 88, 176),   rgb(80, 80, 160),
        rgb(72, 72, 144),   rgb(64, 64, 128),   rgb(56, 56, 112),   rgb(48, 48, 96),
        rgb(40, 40, 80),    rgb(32, 32, 64),    rgb(24, 24, 48),    rgb(16, 16, 32),

        rgb(8, 8, 16),      rgb(255, 127, 127), rgb(240, 120, 120), rgb(224, 112, 112),
        rgb(208, 104, 104), rgb(192, 96, 96),   rgb(176, 88, 88),   rgb(160, 80, 80),
        rgb(144, 72, 72),   rgb(128, 64, 64),   rgb(112, 56, 56),   rgb(96, 48, 48),
        rgb(80, 40, 40),    rgb(64, 32, 32),    rgb(48, 24, 24),    rgb(32, 16, 16),

        rgb(16, 8, 8),      rgb(255, 255, 127), rgb(240, 240, 120), rgb(224, 224, 112),
        rgb(208, 208, 104), rgb(192, 192, 96),  rgb(176, 176, 88),  rgb(160, 160, 80),
        rgb(144, 144, 72),  rgb(128, 128, 64),  rgb(112, 112, 56),  rgb(96, 96, 48),
        rgb(80, 80, 40),    rgb(64, 64, 32),    rgb(48, 48, 24),    rgb(32, 32, 16),

        rgb(16, 16, 8),     rgb(0, 255, 255),   rgb(0, 240, 240),   rgb(0, 224, 224),
        rgb(0, 208, 208),   rgb(0, 192, 192),   rgb(0, 176, 176),   rgb(0, 160, 160),
        rgb(0, 144, 144),   rgb(0, 128, 128),   rgb(0, 112, 112),   rgb(0, 96, 96),
        rgb(0, 80, 80),     rgb(0, 64, 64),     rgb(0, 48, 48),     rgb(0, 32, 32),

        rgb(0, 16, 16),     rgb(255, 0, 127),   rgb(240, 0, 120),   rgb(224, 0, 112),
        rgb(208, 0, 104),   rgb(192, 0, 96),    rgb(176, 0, 88),    rgb(160, 0, 80),
        rgb(144, 0, 72),    rgb(128, 0, 64),    rgb(112, 0, 56),    rgb(96, 0, 48),
        rgb(80, 0, 40),     rgb(64, 0, 32),     rgb(48, 0, 24),     rgb(32, 0, 16),

        rgb(16, 0, 8),      rgb(127, 255, 127), rgb(120, 240, 120), rgb(112, 224, 112),
        rgb(104, 208, 104), rgb(96, 192, 96),   rgb(88, 176, 88),   rgb(80, 160, 80),
        rgb(72, 144, 72),   rgb(64, 128, 64),   rgb(56, 112, 56),   rgb(48, 96, 48),
        rgb(40, 80, 40),    rgb(32, 64, 32),    rgb(24, 48, 24),    rgb(16, 32, 16),

        rgb(8, 16, 8),      rgb(255, 127, 255), rgb(240, 120, 240), rgb(224, 112, 224),
        rgb(208, 104, 208), rgb(192, 96, 192),  rgb(176, 88, 176),  rgb(160, 80, 160),
        rgb(144, 72, 143),  rgb(128, 64, 128),  rgb(112, 56, 112),  rgb(96, 48, 96),
        rgb(80, 40, 80),    rgb(64, 32, 64),    rgb(48, 24, 48),    rgb(32, 16, 32),

        rgb(16, 8, 16),     rgb(0, 127, 255),   rgb(0, 120, 240),   rgb(0, 112, 224),
        rgb(0, 104, 208),   rgb(0, 96, 192),    rgb(0, 88, 176),    rgb(0, 80, 160),
        rgb(0, 72, 143),    rgb(0, 64, 128),    rgb(0, 56, 112),    rgb(0, 48, 96),
        rgb(0, 40, 80),     rgb(0, 32, 64),     rgb(0, 24, 48),     rgb(0, 16, 32),

        rgb(0, 8, 16),      rgb(255, 249, 207), rgb(240, 234, 195), rgb(225, 220, 183),
        rgb(210, 205, 171), rgb(195, 190, 159), rgb(180, 176, 146), rgb(165, 161, 134),
        rgb(150, 146, 122), rgb(135, 132, 110), rgb(120, 117, 97),  rgb(105, 102, 85),
        rgb(90, 88, 73),    rgb(75, 73, 61),    rgb(60, 58, 48),    rgb(45, 44, 36),

        rgb(30, 29, 24),    rgb(255, 0, 0),     rgb(240, 0, 0),     rgb(225, 0, 0),
        rgb(208, 0, 0),     rgb(192, 0, 0),     rgb(176, 0, 0),     rgb(160, 0, 0),
        rgb(144, 0, 0),     rgb(128, 0, 0),     rgb(112, 0, 0),     rgb(96, 0, 0),
        rgb(80, 0, 0),      rgb(64, 0, 0),      rgb(48, 0, 0),      rgb(0, 0, 0),
};

static const int kDiffuse[][3] = {
        {256, 256, 256}, {256, 128, 0}, {256, 256, 0},   {0, 0, 256},
        {0, 256, 0},     {128, 0, 256}, {128, 128, 256}, {256, 128, 128},
        {256, 256, 128}, {0, 256, 256}, {256, 0, 128},   {128, 256, 128},
        {256, 128, 256}, {0, 128, 256}, {241, 235, 196}, {256, 0, 0},
};

static const int kAmbient[][3] = {
        {0, 0, 0}, {0, 0, 0}, {0, 0, 0},          {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
        {0, 0, 0}, {0, 0, 0}, {0, 0, 0},          {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
        {0, 0, 0}, {0, 0, 0}, {3840, 3744, 3072}, {0, 0, 0},
};

RgbColor RgbColor::tint(Hue hue, uint8_t shade) {
    int h = static_cast<int>(hue);
    return rgb(
            ((kDiffuse[h][0] * shade) + kAmbient[h][0]) / 256,
            ((kDiffuse[h][1] * shade) + kAmbient[h][1]) / 256,
            ((kDiffuse[h][2] * shade) + kAmbient[h][2]) / 256);
}

const RgbColor& RgbColor::at(uint8_t index) { return kColors[index]; }

pn::string stringify(const RgbColor& color) {
    if (color.alpha == 0xff) {
        return pn::format("rgb({0}, {1}, {2})", color.red, color.green, color.blue);
    } else {
        return pn::format(
                "rgba({0}, {1}, {2}, {3})", color.red, color.green, color.blue, color.alpha);
    }
}

uint8_t GetTranslateColorShade(Hue hue, uint8_t shade) {
    return (17 - shade) + (static_cast<int>(hue) * 16);
}

RgbColor GetRGBTranslateColorShade(Hue hue, uint8_t shade) {
    return RgbColor::at(GetTranslateColorShade(hue, shade));
}

RgbColor GetRGBTranslateColor(uint8_t color) { return RgbColor::at(color); }

}  // namespace antares
