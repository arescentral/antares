// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2018 The Antares Authors
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

#include <gmock/gmock.h>

using testing::Eq;

namespace antares {
namespace {

using ColorTest = testing::Test;

MATCHER_P(Almost, color, "") {
    RgbColor x          = arg;
    RgbColor y          = color;
    int      diff_red   = std::abs(x.red - y.red);
    int      diff_green = std::abs(x.green - y.green);
    int      diff_blue  = std::abs(x.blue - y.blue);
    int      diff       = diff_red + diff_green + diff_blue;
    return (diff == 1);
}

// Orange: {1.0, 0.5, 0.0}
TEST_F(ColorTest, Orange) {
    EXPECT_THAT(RgbColor::tint(Hue::ORANGE, 255), Eq(GetRGBTranslateColorShade(Hue::ORANGE, 16)));
    EXPECT_THAT(RgbColor::tint(Hue::ORANGE, 240), Eq(GetRGBTranslateColorShade(Hue::ORANGE, 15)));
    EXPECT_THAT(RgbColor::tint(Hue::ORANGE, 224), Eq(GetRGBTranslateColorShade(Hue::ORANGE, 14)));
    EXPECT_THAT(RgbColor::tint(Hue::ORANGE, 208), Eq(GetRGBTranslateColorShade(Hue::ORANGE, 13)));
    EXPECT_THAT(RgbColor::tint(Hue::ORANGE, 192), Eq(GetRGBTranslateColorShade(Hue::ORANGE, 12)));
    EXPECT_THAT(RgbColor::tint(Hue::ORANGE, 176), Eq(GetRGBTranslateColorShade(Hue::ORANGE, 11)));
    EXPECT_THAT(RgbColor::tint(Hue::ORANGE, 160), Eq(GetRGBTranslateColorShade(Hue::ORANGE, 10)));
    EXPECT_THAT(RgbColor::tint(Hue::ORANGE, 144), Eq(GetRGBTranslateColorShade(Hue::ORANGE, 9)));
    EXPECT_THAT(RgbColor::tint(Hue::ORANGE, 128), Eq(GetRGBTranslateColorShade(Hue::ORANGE, 8)));
    EXPECT_THAT(RgbColor::tint(Hue::ORANGE, 112), Eq(GetRGBTranslateColorShade(Hue::ORANGE, 7)));
    EXPECT_THAT(RgbColor::tint(Hue::ORANGE, 96), Eq(GetRGBTranslateColorShade(Hue::ORANGE, 6)));
    EXPECT_THAT(RgbColor::tint(Hue::ORANGE, 80), Eq(GetRGBTranslateColorShade(Hue::ORANGE, 5)));
    EXPECT_THAT(RgbColor::tint(Hue::ORANGE, 64), Eq(GetRGBTranslateColorShade(Hue::ORANGE, 4)));
    EXPECT_THAT(RgbColor::tint(Hue::ORANGE, 48), Eq(GetRGBTranslateColorShade(Hue::ORANGE, 3)));
    EXPECT_THAT(RgbColor::tint(Hue::ORANGE, 32), Eq(GetRGBTranslateColorShade(Hue::ORANGE, 2)));
    EXPECT_THAT(RgbColor::tint(Hue::ORANGE, 16), Eq(GetRGBTranslateColorShade(Hue::ORANGE, 1)));
}

// Indigo: {0.5, 0.5, 1.0}
TEST_F(ColorTest, Indigo) {
    EXPECT_THAT(RgbColor::tint(Hue::INDIGO, 255), Eq(GetRGBTranslateColorShade(Hue::INDIGO, 16)));
    EXPECT_THAT(RgbColor::tint(Hue::INDIGO, 240), Eq(GetRGBTranslateColorShade(Hue::INDIGO, 15)));
    EXPECT_THAT(RgbColor::tint(Hue::INDIGO, 224), Eq(GetRGBTranslateColorShade(Hue::INDIGO, 14)));
    EXPECT_THAT(RgbColor::tint(Hue::INDIGO, 208), Eq(GetRGBTranslateColorShade(Hue::INDIGO, 13)));
    EXPECT_THAT(RgbColor::tint(Hue::INDIGO, 192), Eq(GetRGBTranslateColorShade(Hue::INDIGO, 12)));
    EXPECT_THAT(RgbColor::tint(Hue::INDIGO, 176), Eq(GetRGBTranslateColorShade(Hue::INDIGO, 11)));
    EXPECT_THAT(RgbColor::tint(Hue::INDIGO, 160), Eq(GetRGBTranslateColorShade(Hue::INDIGO, 10)));
    EXPECT_THAT(RgbColor::tint(Hue::INDIGO, 144), Eq(GetRGBTranslateColorShade(Hue::INDIGO, 9)));
    EXPECT_THAT(RgbColor::tint(Hue::INDIGO, 128), Eq(GetRGBTranslateColorShade(Hue::INDIGO, 8)));
    EXPECT_THAT(RgbColor::tint(Hue::INDIGO, 112), Eq(GetRGBTranslateColorShade(Hue::INDIGO, 7)));
    EXPECT_THAT(RgbColor::tint(Hue::INDIGO, 96), Eq(GetRGBTranslateColorShade(Hue::INDIGO, 6)));
    EXPECT_THAT(RgbColor::tint(Hue::INDIGO, 80), Eq(GetRGBTranslateColorShade(Hue::INDIGO, 5)));
    EXPECT_THAT(RgbColor::tint(Hue::INDIGO, 64), Eq(GetRGBTranslateColorShade(Hue::INDIGO, 4)));
    EXPECT_THAT(RgbColor::tint(Hue::INDIGO, 48), Eq(GetRGBTranslateColorShade(Hue::INDIGO, 3)));
    EXPECT_THAT(RgbColor::tint(Hue::INDIGO, 32), Eq(GetRGBTranslateColorShade(Hue::INDIGO, 2)));
    EXPECT_THAT(RgbColor::tint(Hue::INDIGO, 16), Eq(GetRGBTranslateColorShade(Hue::INDIGO, 1)));
}

// Tan: complicated shades
TEST_F(ColorTest, Tan) {
    EXPECT_THAT(RgbColor::tint(Hue::TAN, 255), Almost(GetRGBTranslateColorShade(Hue::TAN, 16)));
    EXPECT_THAT(RgbColor::tint(Hue::TAN, 240), Eq(GetRGBTranslateColorShade(Hue::TAN, 15)));
    EXPECT_THAT(RgbColor::tint(Hue::TAN, 224), Eq(GetRGBTranslateColorShade(Hue::TAN, 14)));
    EXPECT_THAT(RgbColor::tint(Hue::TAN, 208), Eq(GetRGBTranslateColorShade(Hue::TAN, 13)));
    EXPECT_THAT(RgbColor::tint(Hue::TAN, 192), Eq(GetRGBTranslateColorShade(Hue::TAN, 12)));
    EXPECT_THAT(RgbColor::tint(Hue::TAN, 176), Eq(GetRGBTranslateColorShade(Hue::TAN, 11)));
    EXPECT_THAT(RgbColor::tint(Hue::TAN, 160), Eq(GetRGBTranslateColorShade(Hue::TAN, 10)));
    EXPECT_THAT(RgbColor::tint(Hue::TAN, 144), Eq(GetRGBTranslateColorShade(Hue::TAN, 9)));
    EXPECT_THAT(RgbColor::tint(Hue::TAN, 128), Eq(GetRGBTranslateColorShade(Hue::TAN, 8)));
    EXPECT_THAT(RgbColor::tint(Hue::TAN, 112), Eq(GetRGBTranslateColorShade(Hue::TAN, 7)));
    EXPECT_THAT(RgbColor::tint(Hue::TAN, 96), Eq(GetRGBTranslateColorShade(Hue::TAN, 6)));
    EXPECT_THAT(RgbColor::tint(Hue::TAN, 80), Eq(GetRGBTranslateColorShade(Hue::TAN, 5)));
    EXPECT_THAT(RgbColor::tint(Hue::TAN, 64), Eq(GetRGBTranslateColorShade(Hue::TAN, 4)));
    EXPECT_THAT(RgbColor::tint(Hue::TAN, 48), Eq(GetRGBTranslateColorShade(Hue::TAN, 3)));
    EXPECT_THAT(RgbColor::tint(Hue::TAN, 32), Eq(GetRGBTranslateColorShade(Hue::TAN, 2)));
    EXPECT_THAT(RgbColor::tint(Hue::TAN, 16), Eq(GetRGBTranslateColorShade(Hue::TAN, 1)));
}

// Red: {1.0, 0.0, 0.0}
// Shade 14 is a little off for some unknown reason.
// Shades 1 & 2 are totally off
TEST_F(ColorTest, Red) {
    EXPECT_THAT(RgbColor::tint(Hue::RED, 255), Eq(GetRGBTranslateColorShade(Hue::RED, 16)));
    EXPECT_THAT(RgbColor::tint(Hue::RED, 240), Eq(GetRGBTranslateColorShade(Hue::RED, 15)));
    EXPECT_THAT(RgbColor::tint(Hue::RED, 224), Almost(GetRGBTranslateColorShade(Hue::RED, 14)));
    EXPECT_THAT(RgbColor::tint(Hue::RED, 208), Eq(GetRGBTranslateColorShade(Hue::RED, 13)));
    EXPECT_THAT(RgbColor::tint(Hue::RED, 192), Eq(GetRGBTranslateColorShade(Hue::RED, 12)));
    EXPECT_THAT(RgbColor::tint(Hue::RED, 176), Eq(GetRGBTranslateColorShade(Hue::RED, 11)));
    EXPECT_THAT(RgbColor::tint(Hue::RED, 160), Eq(GetRGBTranslateColorShade(Hue::RED, 10)));
    EXPECT_THAT(RgbColor::tint(Hue::RED, 144), Eq(GetRGBTranslateColorShade(Hue::RED, 9)));
    EXPECT_THAT(RgbColor::tint(Hue::RED, 128), Eq(GetRGBTranslateColorShade(Hue::RED, 8)));
    EXPECT_THAT(RgbColor::tint(Hue::RED, 112), Eq(GetRGBTranslateColorShade(Hue::RED, 7)));
    EXPECT_THAT(RgbColor::tint(Hue::RED, 96), Eq(GetRGBTranslateColorShade(Hue::RED, 6)));
    EXPECT_THAT(RgbColor::tint(Hue::RED, 80), Eq(GetRGBTranslateColorShade(Hue::RED, 5)));
    EXPECT_THAT(RgbColor::tint(Hue::RED, 64), Eq(GetRGBTranslateColorShade(Hue::RED, 4)));
    EXPECT_THAT(RgbColor::tint(Hue::RED, 48), Eq(GetRGBTranslateColorShade(Hue::RED, 3)));
    EXPECT_THAT(RgbColor::black(), Eq(GetRGBTranslateColorShade(Hue::RED, 2)));
    EXPECT_THAT(RgbColor::white(), Eq(GetRGBTranslateColorShade(Hue::RED, 1)));
}

// Gray: {1.0, 1.0, 1.0}
// Shade 16 is darkest red for some unknown reason.
// Everything else is just a shade darker than expected.
TEST_F(ColorTest, Gray) {
    EXPECT_THAT(RgbColor::tint(Hue::RED, 32), Eq(GetRGBTranslateColorShade(Hue::GRAY, 16)));
    EXPECT_THAT(RgbColor::tint(Hue::GRAY, 224), Eq(GetRGBTranslateColorShade(Hue::GRAY, 15)));
    EXPECT_THAT(RgbColor::tint(Hue::GRAY, 208), Eq(GetRGBTranslateColorShade(Hue::GRAY, 14)));
    EXPECT_THAT(RgbColor::tint(Hue::GRAY, 192), Eq(GetRGBTranslateColorShade(Hue::GRAY, 13)));
    EXPECT_THAT(RgbColor::tint(Hue::GRAY, 176), Eq(GetRGBTranslateColorShade(Hue::GRAY, 12)));
    EXPECT_THAT(RgbColor::tint(Hue::GRAY, 160), Eq(GetRGBTranslateColorShade(Hue::GRAY, 11)));
    EXPECT_THAT(RgbColor::tint(Hue::GRAY, 144), Eq(GetRGBTranslateColorShade(Hue::GRAY, 10)));
    EXPECT_THAT(RgbColor::tint(Hue::GRAY, 128), Eq(GetRGBTranslateColorShade(Hue::GRAY, 9)));
    EXPECT_THAT(RgbColor::tint(Hue::GRAY, 112), Eq(GetRGBTranslateColorShade(Hue::GRAY, 8)));
    EXPECT_THAT(RgbColor::tint(Hue::GRAY, 96), Eq(GetRGBTranslateColorShade(Hue::GRAY, 7)));
    EXPECT_THAT(RgbColor::tint(Hue::GRAY, 80), Eq(GetRGBTranslateColorShade(Hue::GRAY, 6)));
    EXPECT_THAT(RgbColor::tint(Hue::GRAY, 64), Eq(GetRGBTranslateColorShade(Hue::GRAY, 5)));
    EXPECT_THAT(RgbColor::tint(Hue::GRAY, 48), Eq(GetRGBTranslateColorShade(Hue::GRAY, 4)));
    EXPECT_THAT(RgbColor::tint(Hue::GRAY, 32), Eq(GetRGBTranslateColorShade(Hue::GRAY, 3)));
    EXPECT_THAT(RgbColor::tint(Hue::GRAY, 16), Eq(GetRGBTranslateColorShade(Hue::GRAY, 2)));
    EXPECT_THAT(RgbColor::tint(Hue::GRAY, 8), Eq(GetRGBTranslateColorShade(Hue::GRAY, 1)));
}

}  // namespace
}  // namespace antares
