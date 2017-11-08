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

#include <fcntl.h>
#include <getopt.h>
#include <sfz/sfz.hpp>

#include "config/preferences.hpp"
#include "drawing/build-pix.hpp"
#include "drawing/color.hpp"
#include "drawing/pix-map.hpp"
#include "drawing/text.hpp"
#include "video/offscreen-driver.hpp"
#include "video/text-driver.hpp"

using sfz::Optional;
using sfz::ScopedFd;
using sfz::String;
using sfz::StringSlice;
using sfz::args::help;
using sfz::args::store;
using sfz::args::store_const;
using sfz::dec;
using sfz::format;
using sfz::write;
using std::pair;
using std::unique_ptr;
using std::vector;

namespace io   = sfz::io;
namespace utf8 = sfz::utf8;
namespace args = sfz::args;

namespace antares {
namespace {

class DrawPix : public Card {
  public:
    DrawPix(OffscreenVideoDriver* driver, int16_t id, int32_t width)
            : _driver(driver), _id(id), _width(width) {}

    virtual void draw() const {
        BuildPix pix(_id, _width);
        pix.draw({0, 0});
        if (_driver) {
            _driver->set_capture_rect({0, 0, _width, pix.size().height});
        }
    }

  private:
    OffscreenVideoDriver* _driver;
    const int16_t         _id;
    const int32_t         _width;
};

int main(int argc, char* const* argv) {
    args::Parser parser(argv[0], "Builds all of the scrolling text images in the game");

    Optional<String> output_dir;
    bool             text = false;
    parser.add_argument("-o", "--output", store(output_dir))
            .help("place output in this directory");
    parser.add_argument("-h", "--help", help(parser, 0)).help("display this help screen");
    parser.add_argument("-t", "--text", store_const(text, true)).help("produce text output");

    String error;
    if (!parser.parse_args(argc - 1, argv + 1, error)) {
        print(io::err, format("{0}: {1}\n", parser.name(), error));
        exit(1);
    }

    if (output_dir.has()) {
        makedirs(*output_dir, 0755);
    }

    NullPrefsDriver prefs;

    vector<pair<int, int>> specs = {
            {3020, 450},   // Gaitori prologue
            {3025, 450},   // Tutorial prologue
            {3080, 450},   // Cantharan prologue
            {3081, 450},   // Cantharan epilogue
            {3120, 450},   // Salrilian prologue
            {3211, 450},   // Game epilogue
            {4063, 450},   // Bazidanese prologue
            {4509, 450},   // Elejeetian prologue
            {4606, 450},   // Audemedon prologue
            {5600, 450},   // Story introduction
            {6500, 540},   // Credits text
            {6501, 450},   // Please register
            {10199, 450},  // Unused Gaitori prologue
    };

    vector<pair<unique_ptr<Card>, String>> pix;
    if (text) {
        TextVideoDriver video({540, 2000}, output_dir);
        for (auto spec : specs) {
            pix.emplace_back(
                    unique_ptr<Card>(new DrawPix(nullptr, spec.first, spec.second)),
                    String(format("{0}.txt", dec(spec.first, 5))));
        }
        video.capture(pix);
    } else {
        OffscreenVideoDriver video({540, 2000}, output_dir);
        for (auto spec : specs) {
            pix.emplace_back(
                    unique_ptr<Card>(new DrawPix(&video, spec.first, spec.second)),
                    String(format("{0}.png", dec(spec.first, 5))));
        }
        video.capture(pix);
    }

    return 0;
}

}  // namespace
}  // namespace antares

int main(int argc, char** argv) {
    return antares::main(argc, argv);
}
