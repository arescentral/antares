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

#include <fcntl.h>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "sfz/sfz.hpp"
#include "BuildPix.hpp"
#include "ColorTable.hpp"
#include "ColorTranslation.hpp"
#include "DirectText.hpp"
#include "FakeDrawing.hpp"
#include "ImageDriver.hpp"
#include "LibpngImageDriver.hpp"
#include "PixMap.hpp"

using sfz::Bytes;
using sfz::BytesPiece;
using sfz::Exception;
using sfz::MappedFile;
using sfz::ScopedFd;
using sfz::String;
using sfz::dec;
using sfz::format;
using sfz::posix_strerror;
using sfz::range;
using sfz::scoped_ptr;
using std::make_pair;
using std::pair;
using sfz::write;
using testing::Eq;
using testing::ExplainMatchResult;
using testing::Ge;
using testing::NotNull;
using testing::TestWithParam;
using testing::Values;

namespace antares {

extern PixMap* gRealWorld;
extern PixMap* gActiveWorld;

namespace {

const char kBase64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string Base64(const BytesPiece& bytes, const char* mime_type) {
    std::string result("data:");
    result.append(mime_type);
    result.append(";base64,");
    foreach (i, range(bytes.size() / 3)) {
        const uint8_t a = bytes.at(i * 3);
        const uint8_t b = bytes.at(1 + (i * 3));
        const uint8_t c = bytes.at(2 + (i * 3));

        result.append(1, kBase64[63 & (a >> 2)]);
        result.append(1, kBase64[63 & ((a << 4) | (b >> 4))]);
        result.append(1, kBase64[63 & ((b << 2) | (c >> 6))]);
        result.append(1, kBase64[63 & c]);
    }
    switch (bytes.size() % 3) {
      case 0:
        // Fits evenly; no '=' required.
        break;

      case 1:
        {
            const uint8_t a = bytes.at(bytes.size() - 1);
            result.append(1, kBase64[63 & (a >> 2)]);
            result.append(1, kBase64[63 & (a << 4)]);
            result.append("==");
        }
        break;

      case 2:
        {
            const uint8_t a = bytes.at(bytes.size() - 2);
            const uint8_t b = bytes.at(bytes.size() - 1);
            result.append(1, kBase64[63 & (a >> 2)]);
            result.append(1, kBase64[63 & ((a << 4) | (b >> 4))]);
            result.append(1, kBase64[63 & (b << 2)]);
            result.append("=");
        }
        break;
    }
    return result;
}

class BuildPixTypedTest : public TestWithParam<pair<int, int> > {
  protected:
    BuildPixTypedTest()
            : _id(GetParam().first),
              _width(GetParam().second) {
        ImageDriver::set_driver(new LibpngImageDriver);
        ColorTable clut(256);
        FakeDrawingInit(640, 480);
        gActiveWorld = gRealWorld;
        ColorTranslatorInit(clut);
        InitDirectText();
    }

    const int _id;
    const int _width;
};

TEST_P(BuildPixTypedTest, BuildPix) {
    std::string expected;
    {
        String path(format("test/build-pix/{0}.png", dec(_id, 5)));
        MappedFile file(path);
        expected = Base64(file.data(), "image/png");
    }

    std::string actual;
    {
        scoped_ptr<PixMap> pix(build_pix(_id, _width));
        ASSERT_THAT(pix.get(), NotNull());
        Bytes bytes;
        write(&bytes, *pix);
        actual = Base64(bytes, "image/png");
    }

    ASSERT_THAT(actual, Eq(expected));
}

INSTANTIATE_TEST_CASE_P(FactoryScenario, BuildPixTypedTest, Values(
            make_pair(3020, 450),  // Gaitori prologue
            make_pair(3025, 450),  // Tutorial prologue
            make_pair(3080, 450),  // Cantharan prologue
            make_pair(3081, 450),  // Cantharan epilogue
            make_pair(3120, 450),  // Salrilian prologue
            make_pair(3211, 450),  // Game epilogue
            make_pair(4063, 450),  // Bazidanese prologue
            make_pair(4509, 450),  // Elejeetian prologue
            make_pair(4606, 450),  // Audemedon prologue
            make_pair(5600, 450),  // Story introduction
            make_pair(6500, 540),  // Credits text
            make_pair(6501, 450),  // Please register
            make_pair(10199, 450)  // Unused Gaitori prologue
));

}  // namespace
}  // namespace antares
