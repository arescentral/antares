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

#include "config/dirs.hpp"
#include "config/preferences.hpp"
#include "drawing/styled-text.hpp"
#include "game/sys.hpp"
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
    EditableTextTest() : video({640, 480}, sfz::nullopt) {
        sys.prefs->set_scenario_identifier(kFactoryScenarioIdentifier);
        sys_init();
    }
    TextVideoDriver video;
    NullPrefsDriver prefs;
};

class DummyEditableText : public EditableText {
    using TextReceiver::range;

  public:
    DummyEditableText(pn::string_view prefix, pn::string_view suffix)
            : EditableText{prefix, suffix} {
        _styled_text = StyledText::plain(pn::format("{}{}", prefix, suffix), sys.fonts.tactical);
        _styled_text.select(prefix.size(), prefix.size());
        _styled_text.mark(-1, -1);
    }

    virtual void update(pn::string_view text, range<int> selection, range<int> mark) {
        _styled_text = StyledText::plain(text, sys.fonts.tactical);
        _styled_text.select(selection.begin, selection.end);
        _styled_text.mark(mark.begin, mark.end);
    }

    virtual StyledText&       styled_text() { return _styled_text; };
    virtual const StyledText& styled_text() const { return _styled_text; };

  private:
    StyledText _styled_text;
};

TEST_F(EditableTextTest, Replace) {
    DummyEditableText text{"", ""};
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
    DummyEditableText text{"", ""};
    text.replace(
            {0, 0},
            "\xc3\xa0"             // à
            "b\xcc\x86"            // b̆
            "\xc3\xa7\xcc\xa7"     // ç̧
            "?\xcc\x84\xcc\xb1");  // ?̱̄
    EXPECT_THAT(text.offset(0, TextReceiver::PREV_START, TextReceiver::GLYPHS), Eq(0));
    EXPECT_THAT(text.offset(0, TextReceiver::NEXT_START, TextReceiver::GLYPHS), Eq(2));
    EXPECT_THAT(text.offset(2, TextReceiver::PREV_START, TextReceiver::GLYPHS), Eq(0));
    EXPECT_THAT(text.offset(2, TextReceiver::NEXT_START, TextReceiver::GLYPHS), Eq(5));
    EXPECT_THAT(text.offset(5, TextReceiver::PREV_START, TextReceiver::GLYPHS), Eq(2));
    EXPECT_THAT(text.offset(5, TextReceiver::NEXT_START, TextReceiver::GLYPHS), Eq(9));
    EXPECT_THAT(text.offset(9, TextReceiver::PREV_START, TextReceiver::GLYPHS), Eq(5));
    EXPECT_THAT(text.offset(9, TextReceiver::NEXT_START, TextReceiver::GLYPHS), Eq(14));
    EXPECT_THAT(text.offset(14, TextReceiver::PREV_START, TextReceiver::GLYPHS), Eq(9));
    EXPECT_THAT(text.offset(14, TextReceiver::NEXT_START, TextReceiver::GLYPHS), Eq(14));
}

TEST_F(EditableTextTest, OffsetWords) {
    DummyEditableText text{"", ""};
    text.replace({0, 0}, "1 + 1.5 isn't two");
    EXPECT_THAT(text.offset(0, TextReceiver::PREV_START, TextReceiver::WORDS), Eq(0));
    EXPECT_THAT(text.offset(0, TextReceiver::NEXT_END, TextReceiver::WORDS), Eq(1));

    EXPECT_THAT(text.offset(1, TextReceiver::PREV_START, TextReceiver::WORDS), Eq(0));
    EXPECT_THAT(text.offset(2, TextReceiver::PREV_START, TextReceiver::WORDS), Eq(0));
    EXPECT_THAT(text.offset(3, TextReceiver::PREV_START, TextReceiver::WORDS), Eq(0));
    EXPECT_THAT(text.offset(4, TextReceiver::PREV_START, TextReceiver::WORDS), Eq(0));
    EXPECT_THAT(text.offset(1, TextReceiver::NEXT_END, TextReceiver::WORDS), Eq(7));
    EXPECT_THAT(text.offset(2, TextReceiver::NEXT_END, TextReceiver::WORDS), Eq(7));
    EXPECT_THAT(text.offset(3, TextReceiver::NEXT_END, TextReceiver::WORDS), Eq(7));
    EXPECT_THAT(text.offset(4, TextReceiver::NEXT_END, TextReceiver::WORDS), Eq(7));

    EXPECT_THAT(text.offset(5, TextReceiver::PREV_START, TextReceiver::WORDS), Eq(4));
    EXPECT_THAT(text.offset(6, TextReceiver::PREV_START, TextReceiver::WORDS), Eq(4));
    EXPECT_THAT(text.offset(5, TextReceiver::NEXT_END, TextReceiver::WORDS), Eq(7));
    EXPECT_THAT(text.offset(6, TextReceiver::NEXT_END, TextReceiver::WORDS), Eq(7));

    EXPECT_THAT(text.offset(7, TextReceiver::PREV_START, TextReceiver::WORDS), Eq(4));
    EXPECT_THAT(text.offset(8, TextReceiver::PREV_START, TextReceiver::WORDS), Eq(4));
    EXPECT_THAT(text.offset(7, TextReceiver::NEXT_END, TextReceiver::WORDS), Eq(13));
    EXPECT_THAT(text.offset(8, TextReceiver::NEXT_END, TextReceiver::WORDS), Eq(13));

    EXPECT_THAT(text.offset(9, TextReceiver::PREV_START, TextReceiver::WORDS), Eq(8));
    EXPECT_THAT(text.offset(10, TextReceiver::PREV_START, TextReceiver::WORDS), Eq(8));
    EXPECT_THAT(text.offset(11, TextReceiver::PREV_START, TextReceiver::WORDS), Eq(8));
    EXPECT_THAT(text.offset(12, TextReceiver::PREV_START, TextReceiver::WORDS), Eq(8));
    EXPECT_THAT(text.offset(9, TextReceiver::NEXT_END, TextReceiver::WORDS), Eq(13));
    EXPECT_THAT(text.offset(10, TextReceiver::NEXT_END, TextReceiver::WORDS), Eq(13));
    EXPECT_THAT(text.offset(11, TextReceiver::NEXT_END, TextReceiver::WORDS), Eq(13));
    EXPECT_THAT(text.offset(12, TextReceiver::NEXT_END, TextReceiver::WORDS), Eq(13));

    EXPECT_THAT(text.offset(13, TextReceiver::PREV_START, TextReceiver::WORDS), Eq(8));
    EXPECT_THAT(text.offset(14, TextReceiver::PREV_START, TextReceiver::WORDS), Eq(8));
    EXPECT_THAT(text.offset(13, TextReceiver::NEXT_END, TextReceiver::WORDS), Eq(17));
    EXPECT_THAT(text.offset(14, TextReceiver::NEXT_END, TextReceiver::WORDS), Eq(17));

    EXPECT_THAT(text.offset(15, TextReceiver::PREV_START, TextReceiver::WORDS), Eq(14));
    EXPECT_THAT(text.offset(16, TextReceiver::PREV_START, TextReceiver::WORDS), Eq(14));
    EXPECT_THAT(text.offset(17, TextReceiver::PREV_START, TextReceiver::WORDS), Eq(14));
    EXPECT_THAT(text.offset(15, TextReceiver::NEXT_END, TextReceiver::WORDS), Eq(17));
    EXPECT_THAT(text.offset(16, TextReceiver::NEXT_END, TextReceiver::WORDS), Eq(17));
    EXPECT_THAT(text.offset(17, TextReceiver::NEXT_END, TextReceiver::WORDS), Eq(17));
}

TEST_F(EditableTextTest, OffsetParagraphs) {
    DummyEditableText text{"", ""};
    text.replace(
            {0, 0},
            "012345678\n"
            "012345678\n"
            "\n"
            "123456789");

    EXPECT_THAT(text.offset(0, TextReceiver::PREV_END, TextReceiver::PARAGRAPHS), Eq(0));
    EXPECT_THAT(text.offset(0, TextReceiver::PREV_START, TextReceiver::PARAGRAPHS), Eq(0));
    EXPECT_THAT(text.offset(0, TextReceiver::THIS_START, TextReceiver::PARAGRAPHS), Eq(0));
    EXPECT_THAT(text.offset(0, TextReceiver::THIS_END, TextReceiver::PARAGRAPHS), Eq(9));
    EXPECT_THAT(text.offset(0, TextReceiver::NEXT_END, TextReceiver::PARAGRAPHS), Eq(9));
    EXPECT_THAT(text.offset(0, TextReceiver::NEXT_START, TextReceiver::PARAGRAPHS), Eq(10));

    EXPECT_THAT(text.offset(15, TextReceiver::PREV_END, TextReceiver::PARAGRAPHS), Eq(9));
    EXPECT_THAT(text.offset(15, TextReceiver::PREV_START, TextReceiver::PARAGRAPHS), Eq(10));
    EXPECT_THAT(text.offset(15, TextReceiver::THIS_START, TextReceiver::PARAGRAPHS), Eq(10));
    EXPECT_THAT(text.offset(15, TextReceiver::THIS_END, TextReceiver::PARAGRAPHS), Eq(19));
    EXPECT_THAT(text.offset(15, TextReceiver::NEXT_END, TextReceiver::PARAGRAPHS), Eq(19));
    EXPECT_THAT(text.offset(15, TextReceiver::NEXT_START, TextReceiver::PARAGRAPHS), Eq(20));

    EXPECT_THAT(text.offset(21, TextReceiver::PREV_END, TextReceiver::PARAGRAPHS), Eq(20));
    EXPECT_THAT(text.offset(21, TextReceiver::PREV_START, TextReceiver::PARAGRAPHS), Eq(20));
    EXPECT_THAT(text.offset(21, TextReceiver::THIS_START, TextReceiver::PARAGRAPHS), Eq(21));
    EXPECT_THAT(text.offset(21, TextReceiver::THIS_END, TextReceiver::PARAGRAPHS), Eq(30));
    EXPECT_THAT(text.offset(21, TextReceiver::NEXT_END, TextReceiver::PARAGRAPHS), Eq(30));
    EXPECT_THAT(text.offset(21, TextReceiver::NEXT_START, TextReceiver::PARAGRAPHS), Eq(30));
}

}  // namespace
}  // namespace antares
