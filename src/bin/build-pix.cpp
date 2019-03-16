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
#include <pn/file>
#include <sfz/sfz.hpp>

#include "config/preferences.hpp"
#include "data/level.hpp"
#include "data/plugin.hpp"
#include "data/resource.hpp"
#include "drawing/build-pix.hpp"
#include "drawing/color.hpp"
#include "drawing/pix-map.hpp"
#include "drawing/text.hpp"
#include "lang/exception.hpp"
#include "video/offscreen-driver.hpp"
#include "video/text-driver.hpp"

using sfz::dec;
using std::pair;
using std::unique_ptr;
using std::vector;

namespace args = sfz::args;

namespace antares {
namespace {

class DrawPix : public Card {
  public:
    DrawPix(std::function<pn::string_view()> text, int32_t width,
            std::function<void(Rect)> set_capture_rect)
            : _set_capture_rect(set_capture_rect), _text(text), _width(width) {}

    virtual void draw() const {
        PluginInit();
        BuildPix pix(_text(), _width);
        pix.draw({0, 0});
        _set_capture_rect({0, 0, _width, pix.size().height});
    }

  private:
    const std::function<void(Rect)>        _set_capture_rect;
    const std::function<pn::string_view()> _text;
    const int32_t                          _width;
};

void usage(pn::file_view out, pn::string_view progname, int retcode) {
    out.format(
            "usage: {0} [OPTIONS]\n"
            "\n"
            "  Builds all of the scrolling text images in the game\n"
            "\n"
            "  options:\n"
            "    -o, --output=OUTPUT place output in this directory\n"
            "    -h, --help          display this help screen\n"
            "    -t, --text          produce text output\n",
            progname);
    exit(retcode);
}

std::function<pn::string_view()> prologue(pn::string_view chapter) {
    return [chapter]() -> pn::string_view {
        return *plug.levels.find(chapter.copy())->second.solo.prologue;
    };
}

std::function<pn::string_view()> epilogue(pn::string_view chapter) {
    return [chapter]() -> pn::string_view {
        return *plug.levels.find(chapter.copy())->second.solo.epilogue;
    };
}

template <typename VideoDriver>
void run(
        VideoDriver* video, pn::string_view extension,
        std::function<void(Rect)> set_capture_rect) {
    struct Spec {
        pn::string_view                  name;
        int                              width;
        std::function<pn::string_view()> text;
    };
    vector<Spec> specs{
            {"gai-prologue", 450, prologue("ch01")},
            {"tut-prologue", 450, prologue("tut1")},
            {"can-prologue", 450, prologue("ch07")},
            {"can-epilogue", 450, epilogue("ch07")},
            {"sal-prologue", 450, prologue("ch11")},
            {"outro", 450, epilogue("ch20")},
            {"baz-prologue", 450, prologue("ch14")},
            {"ele-prologue", 450, prologue("ch13")},
            {"aud-prologue", 450, prologue("ch16")},
            {"intro", 450, []() -> pn::string_view { return *plug.info.intro; }},
            {"about", 540, []() -> pn::string_view { return *plug.info.about; }},
    };

    vector<pair<unique_ptr<Card>, pn::string>> pix;
    for (const auto& spec : specs) {
        pix.emplace_back(
                unique_ptr<Card>(new DrawPix(spec.text, spec.width, set_capture_rect)),
                pn::format("{0}.{1}", spec.name, extension));
    }
    video->capture(pix);
}

void main(int argc, char* const* argv) {
    args::callbacks callbacks;

    callbacks.argument = [](pn::string_view arg) { return false; };

    sfz::optional<pn::string> output_dir;
    bool                      text = false;
    callbacks.short_option         = [&argv, &output_dir, &text](
                                     pn::rune opt, const args::callbacks::get_value_f& get_value) {
        switch (opt.value()) {
            case 'o': output_dir.emplace(get_value().copy()); return true;
            case 't': text = true; return true;
            case 'h': usage(stdout, sfz::path::basename(argv[0]), 0); return true;
            default: return false;
        }
    };
    callbacks.long_option =
            [&callbacks](pn::string_view opt, const args::callbacks::get_value_f& get_value) {
                if (opt == "output") {
                    return callbacks.short_option(pn::rune{'o'}, get_value);
                } else if (opt == "text") {
                    return callbacks.short_option(pn::rune{'t'}, get_value);
                } else if (opt == "help") {
                    return callbacks.short_option(pn::rune{'h'}, get_value);
                } else {
                    return false;
                }
            };

    args::parse(argc - 1, argv + 1, callbacks);

    if (output_dir.has_value()) {
        sfz::makedirs(*output_dir, 0755);
    }

    NullPrefsDriver prefs;
    if (text) {
        TextVideoDriver video({540, 2000}, output_dir);
        run(&video, "txt", [](Rect) {});
    } else {
        OffscreenVideoDriver video({540, 2000}, output_dir);
        run(&video, "png", [&video](Rect r) { video.set_capture_rect(r); });
    }
}

}  // namespace
}  // namespace antares

int main(int argc, char* const* argv) { return antares::wrap_main(antares::main, argc, argv); }
