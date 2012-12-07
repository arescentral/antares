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
using std::vector;
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

}  // namespace

class TextVideoDriver::Sprite : public antares::Sprite {
  public:
    Sprite(PrintItem name, TextVideoDriver& driver, Size size):
            _name(name),
            _driver(driver),
            _size(size) { }

    virtual StringSlice name() const { return _name; }

    virtual void draw(const Rect& draw_rect) const {
        if (!world.intersects(draw_rect)) {
            return;
        }
        PrintItem args[] = {
            draw_rect.left, draw_rect.top, draw_rect.right, draw_rect.bottom,
            _name,
        };
        _driver.log("draw", args);
    }

    virtual void draw_cropped(const Rect& draw_rect, Point origin) const {
        if (!world.intersects(draw_rect)) {
            return;
        }
        PrintItem args[] = {
            draw_rect.left, draw_rect.top, draw_rect.right, draw_rect.bottom,
            origin.h, origin.v, _name,
        };
        _driver.log("crop", args);
    }

    virtual void draw_shaded(const Rect& draw_rect, const RgbColor& tint) const {
        if (!world.intersects(draw_rect)) {
            return;
        }
        PrintItem args[] = {
            draw_rect.left, draw_rect.top, draw_rect.right, draw_rect.bottom,
            hex(tint), _name,
        };
        _driver.log("tint", args);
    }

    virtual void draw_static(const Rect& draw_rect, const RgbColor& color, uint8_t frac) const {
        if (!world.intersects(draw_rect)) {
            return;
        }
        PrintItem args[] = {
            draw_rect.left, draw_rect.top, draw_rect.right, draw_rect.bottom,
            hex(color), frac, _name,
        };
        _driver.log("static", args);
    }

    virtual void draw_outlined(
            const Rect& draw_rect, const RgbColor& outline_color,
            const RgbColor& fill_color) const {
        if (!world.intersects(draw_rect)) {
            return;
        }
        PrintItem args[] = {
            draw_rect.left, draw_rect.top, draw_rect.right, draw_rect.bottom,
            hex(outline_color), hex(fill_color), _name,
        };
        _driver.log("outline", args);
    }

    virtual const Size& size() const { return _size; }

  private:
    String _name;
    TextVideoDriver& _driver;
    Size _size;
};

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
    return new Sprite(name, *this, content.size());
}

void TextVideoDriver::fill_rect(const Rect& rect, const RgbColor& color) {
    if (!world.intersects(rect)) {
        return;
    }
    PrintItem args[] = {rect.left, rect.top, rect.right, rect.bottom, hex(color)};
    log("rect", args);
}

void TextVideoDriver::dither_rect(const Rect& rect, const RgbColor& color) {
    PrintItem args[] = {rect.left, rect.top, rect.right, rect.bottom, hex(color)};
    log("dither", args);
}

void TextVideoDriver::draw_point(const Point& at, const RgbColor& color) {
    PrintItem args[] = {at.h, at.v, hex(color)};
    log("point", args);
}

void TextVideoDriver::draw_line(const Point& from, const Point& to, const RgbColor& color) {
    PrintItem args[] = {from.h, from.v, to.h, to.v, hex(color)};
    log("line", args);
}

void TextVideoDriver::draw_triangle(const Rect& rect, const RgbColor& color) {
    if (!world.intersects(rect)) {
        return;
    }
    PrintItem args[] = {rect.left, rect.top, rect.right, rect.bottom, hex(color)};
    log("triangle", args);
}

void TextVideoDriver::draw_diamond(const Rect& rect, const RgbColor& color) {
    if (!world.intersects(rect)) {
        return;
    }
    PrintItem args[] = {rect.left, rect.top, rect.right, rect.bottom, hex(color)};
    log("diamond", args);
}

void TextVideoDriver::draw_plus(const Rect& rect, const RgbColor& color) {
    if (!world.intersects(rect)) {
        return;
    }
    PrintItem args[] = {rect.left, rect.top, rect.right, rect.bottom, hex(color)};
    log("plus", args);
}

void TextVideoDriver::loop(Card* initial) {
    MainLoop loop(*this, _output_dir, initial);
    _scheduler.loop(loop);
}

static void add_arg(String& log, StringSlice arg, vector<StringSlice>& args) {
    size_t start = log.size();
    log.push(arg);
    args.push_back(log.slice(start));
}

template <int size>
void TextVideoDriver::log(StringSlice command, PrintItem (&args)[size]) {
    vector<StringSlice> this_args;
    bool new_command = _last_args.empty() || (command != _last_args[0]);

    if (new_command) {
        add_arg(_log, command, this_args);
    } else {
        this_args.push_back(_last_args[0]);
    }
    for (size_t i = 0; i < size; ++i) {
        _log.push("\t");
        String s(args[i]);
        if (new_command || (s != _last_args[i + 1])) {
            add_arg(_log, s, this_args);
        } else {
            this_args.push_back(_last_args[i + 1]);
        }
    }
    _log.push("\n");

    using std::swap;
    swap(this_args, _last_args);
}

}  // namespace antares
