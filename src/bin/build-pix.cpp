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

#include "drawing/build-pix.hpp"

#include <fcntl.h>

#include <pn/output>
#include <sfz/sfz.hpp>

#include "config/dirs.hpp"
#include "config/preferences.hpp"
#include "data/level.hpp"
#include "data/plugin.hpp"
#include "data/resource.hpp"
#include "drawing/color.hpp"
#include "drawing/pix-map.hpp"
#include "drawing/text.hpp"
#include "game/sys.hpp"
#include "lang/exception.hpp"
#include "video/offscreen-driver.hpp"
#include "video/text-driver.hpp"

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
        PluginInit(sfz::nullopt);
        BuildPix pix(sys.fonts.title, _text(), _width);
        pix.draw({0, 0});
        _set_capture_rect({0, 0, _width, pix.size().height});
    }

  private:
    const std::function<void(Rect)>        _set_capture_rect;
    const std::function<pn::string_view()> _text;
    const int32_t                          _width;
};

void usage(pn::output_view out, pn::string_view progname, int retcode) {
    out.format(
            "usage: {0} [OPTIONS]"
            "\n"
            "\n  Builds all of the scrolling text images in the game"
            "\n"
            "\n  options:"
            "\n    -o, --output=OUTPUT  place output in this directory"
            "\n    -t, --text           produce text output"
            "\n        --opengl=2.0|3.2 select OpenGL version (default: 3.2)"
            "\n    -h, --help           display this help screen"
            "\n",
            progname);
    exit(retcode);
}

pn::string_view intro() { return *plug.info.intro; }

pn::string_view about() { return *plug.info.about; }

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

pn::string_view charset() {
    return "The quick brown fox jumps over the lazy dog.\n"
           "\n"
           "Benjamín pidió una bebida de kiwi y fresa."
           " Noé, sin vergüenza, la más exquisita champaña del menú.\n"
           "\n"
           "Aa Åå Áá Àà Ââ Ää Ãã Ææ\n"
           "Cc Çç\n"
           "Ee Éé Èè Êê Ëë\n"
           "Ii Íí Ìì Îî Ïï İı\n"
           "Nn Ññ\n"
           "Oo Óó Òò Ôô Öö Õõ Øø Œœ\n"
           "Ss ß\n"
           "Uu Úú Ùù Ûû Üü\n"
           "Yy Ÿÿ\n"
           "\n"
           "\uf8ff\n";
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
            {"intro", 450, intro},
            {"about", 540, about},
            {"charset", 450, charset},
            {"pangram", 540,
             []() -> pn::string_view {
                 return "\\fdfLight years from Proxima, zerbilite shockwave jars quadrant\n\n";
             }},
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
    bool                      text         = false;
    int                       scale        = 1;
    std::pair<int, int>       gl_version   = {3, 2};
    pn::string_view           glsl_version = "330 core";
    callbacks.short_option = [&](pn::rune opt, const args::callbacks::get_value_f& get_value) {
        switch (opt.value()) {
            case 'o': output_dir.emplace(get_value().copy()); return true;
            case 't': text = true; return true;
            case 'h': usage(pn::out, sfz::path::basename(argv[0]), 0); return true;
            default: return false;
        }
    };
    callbacks.long_option = [&](pn::string_view                     opt,
                                const args::callbacks::get_value_f& get_value) {
        if (opt == "output") {
            return callbacks.short_option(pn::rune{'o'}, get_value);
        } else if (opt == "hidpi") {
            scale = 2;
            return true;
        } else if (opt == "text") {
            return callbacks.short_option(pn::rune{'t'}, get_value);
        } else if (opt == "opengl") {
            if (get_value() == "2.0") {
                gl_version   = {2, 0};
                glsl_version = "110";
            } else if (get_value() == "3.2") {
                gl_version   = {3, 2};
                glsl_version = "330 core";
            } else {
                throw std::runtime_error("invalid OpenGL version");
            }
            return true;
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
#ifndef _WIN32
        OffscreenVideoDriver video({540, 2000}, scale, gl_version, glsl_version, output_dir);
        run(&video, "png", [&video](Rect r) { video.set_capture_rect(r); });
#endif
    }
}

}  // namespace
}  // namespace antares

int main(int argc, char* const* argv) { return antares::wrap_main(antares::main, argc, argv); }
