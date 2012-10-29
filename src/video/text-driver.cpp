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

#include "video/text-driver.hpp"

#include <fcntl.h>
#include <stdlib.h>
#include <strings.h>
#include <algorithm>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <sfz/sfz.hpp>

#include "cocoa/core-opengl.hpp"
#include "config/preferences.hpp"
#include "drawing/pix-map.hpp"
#include "game/globals.hpp"
#include "game/time.hpp"
#include "math/geometry.hpp"
#include "ui/card.hpp"
#include "ui/event.hpp"

using sfz::Optional;
using sfz::PrintItem;
using sfz::PrintTarget;
using sfz::ScopedFd;
using sfz::String;
using sfz::StringSlice;
using sfz::dec;
using sfz::format;
using sfz::print;
using sfz::write;
namespace utf8 = sfz::utf8;

namespace antares {

namespace {

struct HexColor {
    RgbColor color;
};
HexColor hex(RgbColor color) {
    HexColor result = {color};
    return result;
}
void print_to(PrintTarget target, HexColor color) {
    using sfz::hex;
    print(target, hex(color.color.red, 2));
    print(target, hex(color.color.green, 2));
    print(target, hex(color.color.blue, 2));
    if (color.color.alpha != 255) {
        print(target, hex(color.color.alpha, 2));
    }
}

class TextSprite : public Sprite {
  public:
    TextSprite(PrintItem name, String& log, Size size):
            _name(name),
            _log(log),
            _size(size) { }

    virtual StringSlice name() const { return _name; }
    virtual void draw(const Rect& draw_rect) const {
        if (!world.intersects(draw_rect)) {
            return;
        }
        print(_log, format(
                    "draw\t{0}\t{1}\t{2}\t{3}\t{4}\n",
                    draw_rect.left, draw_rect.top, draw_rect.right, draw_rect.bottom, _name));
    }
    virtual void draw_shaded(const Rect& draw_rect, const RgbColor& tint) const {
        if (!world.intersects(draw_rect)) {
            return;
        }
        print(_log, format(
                    "tint\t{0}\t{1}\t{2}\t{3}\t{4}\t{5}\n",
                    draw_rect.left, draw_rect.top, draw_rect.right, draw_rect.bottom,
                    _name, hex(tint)));
    }
    virtual void draw_static(const Rect& draw_rect, const RgbColor& color, uint8_t frac) const {
        if (!world.intersects(draw_rect)) {
            return;
        }
        print(_log, format(
                    "static\t{0}\t{1}\t{2}\t{3}\t{4}\t{5}\t{6}\n",
                    draw_rect.left, draw_rect.top, draw_rect.right, draw_rect.bottom,
                    _name, frac, hex(color)));
    }
    virtual const Size& size() const { return _size; }

  private:
    String _name;
    String& _log;
    Size _size;
};

}  // namespace

class TextVideoDriver::MainLoop : public EventScheduler::MainLoop {
  public:
    MainLoop(TextVideoDriver& driver, const Optional<String>& output_dir, Card* initial):
            _driver(driver),
            _output_dir(output_dir),
            _stack(initial) { }

    bool takes_snapshots() {
        return _output_dir.has();
    }

    void snapshot(int64_t ticks) {
        String dir(format("{0}/screens", *_output_dir));
        makedirs(dir, 0755);
        String path(format("{0}/{1}.txt", dir, dec(ticks, 6)));
        ScopedFd file(open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644));
        write(file, utf8::encode(_driver._log));
    }

    void draw() {
        _driver._log.clear();
        _stack.top()->draw();
    }
    bool done() const { return _stack.empty(); }
    Card* top() const { return _stack.top(); }

  private:
    TextVideoDriver& _driver;
    Optional<String> _output_dir;
    CardStack _stack;
};

TextVideoDriver::TextVideoDriver(
        Size screen_size, EventScheduler& scheduler, const Optional<String>& output_dir):
        _size(screen_size),
        _scheduler(scheduler),
        _output_dir(output_dir) { }

Sprite* TextVideoDriver::new_sprite(sfz::PrintItem name, const PixMap& content) {
    return new TextSprite(name, _log, content.size());
}

void TextVideoDriver::fill_rect(const Rect& rect, const RgbColor& color) {
    print(_log, format(
                "rect\t{0}\t{1}\t{2}\t{3}\t{4}\n",
                rect.left, rect.top, rect.right, rect.bottom, hex(color)));
}

void TextVideoDriver::dither_rect(const Rect& rect, const RgbColor& color) {
    print(_log, format(
                "dither\t{0}\t{1}\t{2}\t{3}\t{4}\n",
                rect.left, rect.top, rect.right, rect.bottom, hex(color)));
}

void TextVideoDriver::draw_point(const Point& at, const RgbColor& color) {
    print(_log, format(
                "point\t{0}\t{1}\t{2}\n",
                at.h, at.v, hex(color)));
}

void TextVideoDriver::draw_line(const Point& from, const Point& to, const RgbColor& color) {
    print(_log, format(
                "line\t{0}\t{1}\t{2}\t{3}\t{4}\n",
                from.h, from.v, to.h, to.v, hex(color)));
}

void TextVideoDriver::loop(Card* initial) {
    MainLoop loop(*this, _output_dir, initial);
    _scheduler.loop(loop);
}

}  // namespace antares
