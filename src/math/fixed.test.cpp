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

#include "math/fixed.hpp"

#include <gmock/gmock.h>

using testing::Eq;
using testing::Ge;
using testing::Gt;
using testing::Le;
using testing::Lt;
using testing::Ne;

namespace antares {
namespace {

using FixedTest = testing::Test;

TEST_F(FixedTest, Print) {
    EXPECT_EQ("0.0", stringify(Fixed::from_val(0)));

    EXPECT_EQ("1.0", stringify(Fixed::from_val(256)));
    EXPECT_EQ("-1.0", stringify(Fixed::from_val(-256)));
    EXPECT_EQ("1.125", stringify(Fixed::from_val(288)));
    EXPECT_EQ("-1.125", stringify(Fixed::from_val(-288)));

    EXPECT_EQ("8388607.996", stringify(Fixed::from_val(std::numeric_limits<int32_t>::max())));
    EXPECT_EQ("-8388607.996", stringify(Fixed::from_val(-std::numeric_limits<int32_t>::max())));
    EXPECT_EQ("-8388608.0", stringify(Fixed::from_val(std::numeric_limits<int32_t>::min())));

    EXPECT_EQ("1.38", stringify(Fixed::from_val(353)));
    EXPECT_EQ("1.383", stringify(Fixed::from_val(354)));
    EXPECT_EQ("1.387", stringify(Fixed::from_val(355)));
    EXPECT_EQ("1.39", stringify(Fixed::from_val(356)));
    EXPECT_EQ("1.395", stringify(Fixed::from_val(357)));
    EXPECT_EQ("1.4", stringify(Fixed::from_val(358)));
    EXPECT_EQ("1.402", stringify(Fixed::from_val(359)));
    EXPECT_EQ("1.406", stringify(Fixed::from_val(360)));
    EXPECT_EQ("1.41", stringify(Fixed::from_val(361)));
    EXPECT_EQ("1.414", stringify(Fixed::from_val(362)));

    // All 2.x values should be printed with 1 digit of precision.
    for (int i = 0; i < 10; ++i) {
        char expected[4];
        sprintf(expected, "2.%d", i);
        EXPECT_EQ(pn::string_view{expected}, stringify(Fixed::from_float(2.0 + (i / 10.0))));
    }

    // All 3.xy values should be printed with 2 digits of precision
    // (except when y is 0).
    for (int i = 0; i < 100; ++i) {
        if ((i % 10) == 0) {
            continue;
        }
        char expected[5];
        sprintf(expected, "3.%02d", i);
        EXPECT_EQ(pn::string_view{expected}, stringify(Fixed::from_float(3.0 + (i / 100.0))));
    }
}

TEST_F(FixedTest, FloatToFixed) {
    EXPECT_EQ(-256, Fixed::from_float(-1.0).val());
    EXPECT_EQ(-128, Fixed::from_float(-0.5).val());
    EXPECT_EQ(-64, Fixed::from_float(-0.25).val());
    EXPECT_EQ(0, Fixed::from_float(0.0).val());
    EXPECT_EQ(64, Fixed::from_float(0.25).val());
    EXPECT_EQ(128, Fixed::from_float(0.5).val());
    EXPECT_EQ(256, Fixed::from_float(1.0).val());

    EXPECT_EQ(353, Fixed::from_float(1.378).val());
    EXPECT_EQ(353, Fixed::from_float(1.38).val());
    EXPECT_EQ(354, Fixed::from_float(1.382).val());
    EXPECT_EQ(355, Fixed::from_float(1.386).val());
    EXPECT_EQ(356, Fixed::from_float(1.390).val());
    EXPECT_EQ(356, Fixed::from_float(1.39).val());
    EXPECT_EQ(357, Fixed::from_float(1.394).val());
    EXPECT_EQ(358, Fixed::from_float(1.398).val());
    EXPECT_EQ(358, Fixed::from_float(1.4).val());
    EXPECT_EQ(358, Fixed::from_float(1.40).val());
    EXPECT_EQ(359, Fixed::from_float(1.402).val());
    EXPECT_EQ(360, Fixed::from_float(1.406).val());
    EXPECT_EQ(361, Fixed::from_float(1.41).val());
    EXPECT_EQ(361, Fixed::from_float(1.410).val());
    EXPECT_EQ(362, Fixed::from_float(1.414).val());

    EXPECT_EQ(2147483647, Fixed::from_float(8388607.996).val());
    EXPECT_EQ(-2147483647, Fixed::from_float(-8388607.996).val());
    EXPECT_EQ(-2147483648, Fixed::from_float(-8388608.0).val());
}

TEST_F(FixedTest, FixedToFloat) {
    EXPECT_EQ(-1.0, mFixedToFloat(Fixed::from_val(-256)));
    EXPECT_EQ(-0.5, mFixedToFloat(Fixed::from_val(-128)));
    EXPECT_EQ(-0.25, mFixedToFloat(Fixed::from_val(-64)));
    EXPECT_EQ(0.0, mFixedToFloat(Fixed::from_val(0)));
    EXPECT_EQ(0.25, mFixedToFloat(Fixed::from_val(64)));
    EXPECT_EQ(0.5, mFixedToFloat(Fixed::from_val(128)));
    EXPECT_EQ(1.0, mFixedToFloat(Fixed::from_val(256)));

    EXPECT_FLOAT_EQ(1.378, mFixedToFloat(Fixed::from_val(353)));
    EXPECT_FLOAT_EQ(1.382, mFixedToFloat(Fixed::from_val(354)));
    EXPECT_FLOAT_EQ(1.386, mFixedToFloat(Fixed::from_val(355)));
    EXPECT_FLOAT_EQ(1.390, mFixedToFloat(Fixed::from_val(356)));
    EXPECT_FLOAT_EQ(1.394, mFixedToFloat(Fixed::from_val(357)));
    EXPECT_FLOAT_EQ(1.398, mFixedToFloat(Fixed::from_val(358)));
    EXPECT_FLOAT_EQ(1.402, mFixedToFloat(Fixed::from_val(359)));
    EXPECT_FLOAT_EQ(1.406, mFixedToFloat(Fixed::from_val(360)));
    EXPECT_FLOAT_EQ(1.410, mFixedToFloat(Fixed::from_val(361)));
    EXPECT_FLOAT_EQ(1.414, mFixedToFloat(Fixed::from_val(362)));

    EXPECT_FLOAT_EQ(8388607.996, mFixedToFloat(Fixed::from_val(2147483647)));
    EXPECT_FLOAT_EQ(-8388607.996, mFixedToFloat(Fixed::from_val(-2147483647)));
    EXPECT_FLOAT_EQ(-8388608.0, mFixedToFloat(Fixed::from_val(-2147483648)));
}

TEST_F(FixedTest, Comparison) {
    EXPECT_THAT(Fixed::from_long(1), Eq(Fixed::from_long(1)));
    EXPECT_THAT(Fixed::from_long(1), Ne(Fixed::from_long(2)));
    EXPECT_THAT(Fixed::from_long(1), Lt(Fixed::from_long(2)));
    EXPECT_THAT(Fixed::from_long(1), Le(Fixed::from_long(2)));
    EXPECT_THAT(Fixed::from_long(2), Gt(Fixed::from_long(1)));
    EXPECT_THAT(Fixed::from_long(2), Ge(Fixed::from_long(1)));
}

TEST_F(FixedTest, Math) {
    EXPECT_THAT(Fixed::from_long(1) + Fixed::from_long(2), Eq(Fixed::from_long(3)));
    EXPECT_THAT(Fixed::from_long(3) - Fixed::from_long(2), Eq(Fixed::from_long(1)));

    EXPECT_THAT(Fixed::from_long(1) * 2, Eq(Fixed::from_long(2)));
    EXPECT_THAT(2 * Fixed::from_long(1), Eq(Fixed::from_long(2)));
    EXPECT_THAT(Fixed::from_long(1) * 2, Eq(Fixed::from_long(2)));
    EXPECT_THAT(Fixed::from_long(1) * Fixed::from_long(2), Eq(Fixed::from_long(2)));

    EXPECT_THAT(Fixed::from_long(3) / 2, Eq(Fixed::from_float(1.5)));
    EXPECT_THAT(Fixed::from_long(3) / Fixed::from_long(2), Eq(Fixed::from_float(1.5)));

    EXPECT_THAT(Fixed::from_long(5) % 2, Eq(Fixed::from_long(1)));
    EXPECT_THAT(Fixed::from_long(5) % Fixed::from_long(2), Eq(Fixed::from_long(1)));

    EXPECT_THAT(Fixed::from_long(2) >> 1, Eq(Fixed::from_long(1)));
    EXPECT_THAT(Fixed::from_long(2) << 1, Eq(Fixed::from_long(4)));
}

}  // namespace
}  // namespace antares
