// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2019 The Antares Authors
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

#include "ui/editable-text.hpp"

#include <gmock/gmock.h>

#include "video/text-driver.hpp"

namespace antares {

bool operator==(TextReceiver::range<int> x, TextReceiver::range<int> y) {
    return (x.begin == y.begin) && (x.end == y.end);
}

namespace {

using ::testing::Eq;

using range = TextReceiver::range<int>;

class EditableTextTest : public testing::Test {
  public:
    EditableTextTest() : video({640, 480}, sfz::nullopt) {}
    TextVideoDriver video;
};

class DummyEditableText : public EditableText {
  public:
    virtual void update() {}
};

TEST_F(EditableTextTest, Replace) {
    DummyEditableText text;
    EXPECT_THAT(text.text(), Eq(""));
    EXPECT_THAT(text.selection(), Eq(range{0, 0}));
    EXPECT_THAT(text.mark(), Eq(range{-1, -1}));

    text.replace({0, 0}, "initial text");
    EXPECT_THAT(text.text(), Eq("initial text"));
    EXPECT_THAT(text.selection(), Eq(range{12, 12}));
    EXPECT_THAT(text.mark(), Eq(range{-1, -1}));

    text.replace({8, 12}, "few words");
    EXPECT_THAT(text.text(), Eq("initial few words"));
    EXPECT_THAT(text.selection(), Eq(range{17, 17}));
    EXPECT_THAT(text.mark(), Eq(range{-1, -1}));

    text.replace({0, 7}, "final");
    EXPECT_THAT(text.text(), Eq("final few words"));
    EXPECT_THAT(text.selection(), Eq(range{5, 5}));
    EXPECT_THAT(text.mark(), Eq(range{-1, -1}));
}

TEST_F(EditableTextTest, OffsetGlyphs) {
    DummyEditableText text;
    text.replace(
            {0, 0},
            "\xc3\xa0"             // à
            "b\xcc\x86"            // b̆
            "\xc3\xa7\xcc\xa7"     // ç̧
            "?\xcc\x84\xcc\xb1");  // ?̱̄
    EXPECT_THAT(text.offset(0, -1, TextReceiver::GLYPHS), Eq(0));
    EXPECT_THAT(text.offset(0, +1, TextReceiver::GLYPHS), Eq(2));
    EXPECT_THAT(text.offset(2, -1, TextReceiver::GLYPHS), Eq(0));
    EXPECT_THAT(text.offset(2, +1, TextReceiver::GLYPHS), Eq(5));
    EXPECT_THAT(text.offset(5, -1, TextReceiver::GLYPHS), Eq(2));
    EXPECT_THAT(text.offset(5, +1, TextReceiver::GLYPHS), Eq(9));
    EXPECT_THAT(text.offset(9, -1, TextReceiver::GLYPHS), Eq(5));
    EXPECT_THAT(text.offset(9, +1, TextReceiver::GLYPHS), Eq(14));
    EXPECT_THAT(text.offset(14, -1, TextReceiver::GLYPHS), Eq(9));
    EXPECT_THAT(text.offset(14, +1, TextReceiver::GLYPHS), Eq(14));
}

TEST_F(EditableTextTest, OffsetWords) {
    DummyEditableText text;
    text.replace({0, 0}, "1 + 1.5 isn't two");
    EXPECT_THAT(text.offset(0, -1, TextReceiver::WORDS), Eq(0));
    EXPECT_THAT(text.offset(0, +1, TextReceiver::WORDS), Eq(1));

    EXPECT_THAT(text.offset(1, -1, TextReceiver::WORDS), Eq(0));
    EXPECT_THAT(text.offset(2, -1, TextReceiver::WORDS), Eq(0));
    EXPECT_THAT(text.offset(3, -1, TextReceiver::WORDS), Eq(0));
    EXPECT_THAT(text.offset(4, -1, TextReceiver::WORDS), Eq(0));
    EXPECT_THAT(text.offset(1, +1, TextReceiver::WORDS), Eq(7));
    EXPECT_THAT(text.offset(2, +1, TextReceiver::WORDS), Eq(7));
    EXPECT_THAT(text.offset(3, +1, TextReceiver::WORDS), Eq(7));
    EXPECT_THAT(text.offset(4, +1, TextReceiver::WORDS), Eq(7));

    EXPECT_THAT(text.offset(5, -1, TextReceiver::WORDS), Eq(4));
    EXPECT_THAT(text.offset(6, -1, TextReceiver::WORDS), Eq(4));
    EXPECT_THAT(text.offset(5, +1, TextReceiver::WORDS), Eq(7));
    EXPECT_THAT(text.offset(6, +1, TextReceiver::WORDS), Eq(7));

    EXPECT_THAT(text.offset(7, -1, TextReceiver::WORDS), Eq(4));
    EXPECT_THAT(text.offset(8, -1, TextReceiver::WORDS), Eq(4));
    EXPECT_THAT(text.offset(7, +1, TextReceiver::WORDS), Eq(13));
    EXPECT_THAT(text.offset(8, +1, TextReceiver::WORDS), Eq(13));

    EXPECT_THAT(text.offset(9, -1, TextReceiver::WORDS), Eq(8));
    EXPECT_THAT(text.offset(10, -1, TextReceiver::WORDS), Eq(8));
    EXPECT_THAT(text.offset(11, -1, TextReceiver::WORDS), Eq(8));
    EXPECT_THAT(text.offset(12, -1, TextReceiver::WORDS), Eq(8));
    EXPECT_THAT(text.offset(9, +1, TextReceiver::WORDS), Eq(13));
    EXPECT_THAT(text.offset(10, +1, TextReceiver::WORDS), Eq(13));
    EXPECT_THAT(text.offset(11, +1, TextReceiver::WORDS), Eq(13));
    EXPECT_THAT(text.offset(12, +1, TextReceiver::WORDS), Eq(13));

    EXPECT_THAT(text.offset(13, -1, TextReceiver::WORDS), Eq(8));
    EXPECT_THAT(text.offset(14, -1, TextReceiver::WORDS), Eq(8));
    EXPECT_THAT(text.offset(13, +1, TextReceiver::WORDS), Eq(17));
    EXPECT_THAT(text.offset(14, +1, TextReceiver::WORDS), Eq(17));

    EXPECT_THAT(text.offset(15, -1, TextReceiver::WORDS), Eq(14));
    EXPECT_THAT(text.offset(16, -1, TextReceiver::WORDS), Eq(14));
    EXPECT_THAT(text.offset(17, -1, TextReceiver::WORDS), Eq(14));
    EXPECT_THAT(text.offset(15, +1, TextReceiver::WORDS), Eq(17));
    EXPECT_THAT(text.offset(16, +1, TextReceiver::WORDS), Eq(17));
    EXPECT_THAT(text.offset(17, +1, TextReceiver::WORDS), Eq(17));
}

TEST_F(EditableTextTest, OffsetParagraphs) {
    DummyEditableText text;
    text.replace(
            {0, 0},
            "012345678\n"
            "012345678\n"
            "\n"
            "123456789");

    EXPECT_THAT(text.offset(0, -1, TextReceiver::PARA_ENDS), Eq(0));
    EXPECT_THAT(text.offset(0, -1, TextReceiver::PARA_BEGINNINGS), Eq(0));
    EXPECT_THAT(text.offset(0, +1, TextReceiver::PARA_ENDS), Eq(9));
    EXPECT_THAT(text.offset(0, +1, TextReceiver::PARA_BEGINNINGS), Eq(10));

    EXPECT_THAT(text.offset(15, -1, TextReceiver::PARA_ENDS), Eq(9));
    EXPECT_THAT(text.offset(15, -1, TextReceiver::PARA_BEGINNINGS), Eq(10));
    EXPECT_THAT(text.offset(15, +1, TextReceiver::PARA_ENDS), Eq(19));
    EXPECT_THAT(text.offset(15, +1, TextReceiver::PARA_BEGINNINGS), Eq(20));

    EXPECT_THAT(text.offset(21, -1, TextReceiver::PARA_ENDS), Eq(20));
    EXPECT_THAT(text.offset(21, -1, TextReceiver::PARA_BEGINNINGS), Eq(20));
    EXPECT_THAT(text.offset(21, +1, TextReceiver::PARA_ENDS), Eq(30));
    EXPECT_THAT(text.offset(21, +1, TextReceiver::PARA_BEGINNINGS), Eq(30));
}

}  // namespace
}  // namespace antares
