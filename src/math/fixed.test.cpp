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

#include "math/fixed.hpp"

#include <gmock/gmock.h>
#include <sfz/sfz.hpp>

using sfz::String;

namespace antares {
namespace {

typedef testing::Test FixedTest;

TEST_F(FixedTest, Print) {
    EXPECT_EQ("0.000", String(fixed(0)));

    EXPECT_EQ("1.000", String(fixed(256)));
    EXPECT_EQ("-1.000", String(fixed(-256)));
    EXPECT_EQ("1.125", String(fixed(288)));
    EXPECT_EQ("-1.125", String(fixed(-288)));

    EXPECT_EQ("8388607.996", String(fixed(std::numeric_limits<Fixed>::max())));
    EXPECT_EQ("-8388607.996", String(fixed(-std::numeric_limits<Fixed>::max())));
    EXPECT_EQ("-8388608.000", String(fixed(std::numeric_limits<Fixed>::min())));

    EXPECT_EQ("1.378", String(fixed(353)));
    EXPECT_EQ("1.382", String(fixed(354)));
    EXPECT_EQ("1.386", String(fixed(355)));
    EXPECT_EQ("1.390", String(fixed(356)));
    EXPECT_EQ("1.394", String(fixed(357)));
    EXPECT_EQ("1.398", String(fixed(358)));
    EXPECT_EQ("1.402", String(fixed(359)));
    EXPECT_EQ("1.406", String(fixed(360)));
    EXPECT_EQ("1.410", String(fixed(361)));
    EXPECT_EQ("1.414", String(fixed(362)));
}

TEST_F(FixedTest, FloatToFixed) {
    EXPECT_EQ(-256, mFloatToFixed(-1.0));
    EXPECT_EQ(-128, mFloatToFixed(-0.5));
    EXPECT_EQ(-64, mFloatToFixed(-0.25));
    EXPECT_EQ(0, mFloatToFixed(0.0));
    EXPECT_EQ(64, mFloatToFixed(0.25));
    EXPECT_EQ(128, mFloatToFixed(0.5));
    EXPECT_EQ(256, mFloatToFixed(1.0));

    EXPECT_EQ(353, mFloatToFixed(1.378));
    EXPECT_EQ(354, mFloatToFixed(1.382));
    EXPECT_EQ(355, mFloatToFixed(1.386));
    EXPECT_EQ(356, mFloatToFixed(1.390));
    EXPECT_EQ(357, mFloatToFixed(1.394));
    EXPECT_EQ(358, mFloatToFixed(1.398));
    EXPECT_EQ(359, mFloatToFixed(1.402));
    EXPECT_EQ(360, mFloatToFixed(1.406));
    EXPECT_EQ(361, mFloatToFixed(1.410));
    EXPECT_EQ(362, mFloatToFixed(1.414));
}

TEST_F(FixedTest, FixedToFloat) {
    EXPECT_EQ(-1.0, mFixedToFloat(-256));
    EXPECT_EQ(-0.5, mFixedToFloat(-128));
    EXPECT_EQ(-0.25, mFixedToFloat(-64));
    EXPECT_EQ(0.0, mFixedToFloat(0));
    EXPECT_EQ(0.25, mFixedToFloat(64));
    EXPECT_EQ(0.5, mFixedToFloat(128));
    EXPECT_EQ(1.0, mFixedToFloat(256));

    EXPECT_FLOAT_EQ(1.378, mFixedToFloat(353));
    EXPECT_FLOAT_EQ(1.382, mFixedToFloat(354));
    EXPECT_FLOAT_EQ(1.386, mFixedToFloat(355));
    EXPECT_FLOAT_EQ(1.390, mFixedToFloat(356));
    EXPECT_FLOAT_EQ(1.394, mFixedToFloat(357));
    EXPECT_FLOAT_EQ(1.398, mFixedToFloat(358));
    EXPECT_FLOAT_EQ(1.402, mFixedToFloat(359));
    EXPECT_FLOAT_EQ(1.406, mFixedToFloat(360));
    EXPECT_FLOAT_EQ(1.410, mFixedToFloat(361));
    EXPECT_FLOAT_EQ(1.414, mFixedToFloat(362));
}

}  // namespace
}  // namespace antares
