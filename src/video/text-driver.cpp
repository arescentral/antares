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

#include "video/text-driver.hpp"

#include <fcntl.h>
#include <stdlib.h>
#include <strings.h>
#include <algorithm>
#include <pn/file>
#include <sfz/sfz.hpp>

#include "config/preferences.hpp"
#include "drawing/pix-map.hpp"
#include "game/globals.hpp"
#include "game/sys.hpp"
#include "game/time.hpp"
#include "math/geometry.hpp"
#include "ui/card.hpp"
#include "ui/event.hpp"

using sfz::dec;
using std::make_pair;
using std::max;
using std::min;
using std::pair;
using std::unique_ptr;
using std::vector;
namespace path = sfz::path;

namespace antares {

namespace {

pn::string hex(RgbColor color) {
    char s[9];
    if (color.alpha != 255) {
        sprintf(s, "%02x%02x%02x%02x", color.red, color.green, color.blue, color.alpha);
    } else {
        sprintf(s, "%02x%02x%02x", color.red, color.green, color.blue);
    }
    return s;
}

}  // namespace

class TextVideoDriver::TextureImpl : public Texture::Impl {
  public:
    TextureImpl(pn::string_view name, TextVideoDriver& driver, Size size)
            : _name(name.copy()), _driver(driver), _size(size) {}

    virtual pn::string_view name() const { return _name; }

    virtual void draw(const Rect& draw_rect) const {
        if (!world().intersects(draw_rect)) {
            return;
        }
        _driver.log(
                "draw", draw_rect.left, draw_rect.top, draw_rect.right, draw_rect.bottom, _name);
    }

    virtual void draw_cropped(const Rect& dest, const Rect& source, const RgbColor& tint) const {
        if (!world().intersects(dest)) {
            return;
        }
        if (source.size() == dest.size()) {
            _driver.log(
                    "crop", dest.left, dest.top, dest.right, dest.bottom, source.left, source.top,
                    hex(tint), _name);
        } else {
            _driver.log(
                    "crop", dest.left, dest.top, dest.right, dest.bottom, source.left, source.top,
                    source.right, source.bottom, hex(tint), _name);
        }
    }

    virtual void draw_shaded(const Rect& draw_rect, const RgbColor& tint) const {
        if (!world().intersects(draw_rect)) {
            return;
        }
        _driver.log(
                "tint", draw_rect.left, draw_rect.top, draw_rect.right, draw_rect.bottom,
                hex(tint), _name);
    }

    virtual void draw_static(const Rect& draw_rect, const RgbColor& color, uint8_t frac) const {
        if (!world().intersects(draw_rect)) {
            return;
        }
        _driver.log(
                "static", draw_rect.left, draw_rect.top, draw_rect.right, draw_rect.bottom,
                hex(color), frac, _name);
    }

    virtual void draw_outlined(
            const Rect& draw_rect, const RgbColor& outline_color,
            const RgbColor& fill_color) const {
        if (!world().intersects(draw_rect)) {
            return;
        }
        _driver.log(
                "outline", draw_rect.left, draw_rect.top, draw_rect.right, draw_rect.bottom,
                hex(outline_color), hex(fill_color), _name.copy());
    }

    virtual const Size& size() const { return _size; }

  private:
    pn::string       _name;
    TextVideoDriver& _driver;
    Size             _size;
};

class TextVideoDriver::MainLoop : public EventScheduler::MainLoop {
  public:
    MainLoop(TextVideoDriver& driver, const sfz::optional<pn::string>& output_dir, Card* initial)
            : _driver(driver), _stack(initial) {
        if (output_dir.has_value()) {
            _output_dir.emplace(output_dir->copy());
        }
    }

    bool takes_snapshots() { return _output_dir.has_value(); }

    void snapshot(wall_ticks ticks) {
        snapshot_to(pn::format("screens/{0}.txt", dec(ticks.time_since_epoch().count(), 6)));
    }

    void snapshot_to(pn::string_view relpath) {
        pn::string path = pn::format("{0}/{1}", *_output_dir, relpath);
        sfz::makedirs(path::dirname(path), 0755);
        pn::file file = pn::open(path, "w");
        file.write(_driver._log);
    }

    void draw() {
        _driver._log.clear();
        _driver._last_args.clear();
        _stack.top()->draw();
    }
    bool  done() const { return _stack.empty(); }
    Card* top() const { return _stack.top(); }

  private:
    TextVideoDriver&          _driver;
    sfz::optional<pn::string> _output_dir;
    CardStack                 _stack;
};

TextVideoDriver::TextVideoDriver(Size screen_size, const sfz::optional<pn::string>& output_dir)
        : _size(screen_size) {
    if (output_dir.has_value()) {
        _output_dir.emplace(output_dir->copy());
    }
}

int TextVideoDriver::scale() const { return 1; }

Texture TextVideoDriver::texture(pn::string_view name, const PixMap& content, int scale) {
    static_cast<void>(scale);  // TODO(sfiera): test HiDPI?
    return std::unique_ptr<Texture::Impl>(new TextureImpl(name, *this, content.size()));
}

void TextVideoDriver::batch_rect(const Rect& rect, const RgbColor& color) {
    if (!world().intersects(rect)) {
        return;
    }
    log("rect", rect.left, rect.top, rect.right, rect.bottom, hex(color));
}

void TextVideoDriver::dither_rect(const Rect& rect, const RgbColor& color) {
    log("dither", rect.left, rect.top, rect.right, rect.bottom, hex(color));
}

void TextVideoDriver::draw_point(const Point& at, const RgbColor& color) {
    log("point", at.h, at.v, hex(color));
}

void TextVideoDriver::draw_line(const Point& from, const Point& to, const RgbColor& color) {
    log("line", from.h, from.v, to.h, to.v, hex(color));
}

void TextVideoDriver::draw_triangle(const Rect& rect, const RgbColor& color) {
    if (!world().intersects(rect)) {
        return;
    }
    log("triangle", rect.left, rect.top, rect.right, rect.bottom, hex(color));
}

void TextVideoDriver::draw_diamond(const Rect& rect, const RgbColor& color) {
    if (!world().intersects(rect)) {
        return;
    }
    log("diamond", rect.left, rect.top, rect.right, rect.bottom, hex(color));
}

void TextVideoDriver::draw_plus(const Rect& rect, const RgbColor& color) {
    if (!world().intersects(rect)) {
        return;
    }
    log("plus", rect.left, rect.top, rect.right, rect.bottom, hex(color));
}

void TextVideoDriver::loop(Card* initial, EventScheduler& scheduler) {
    _scheduler = &scheduler;
    MainLoop loop(*this, _output_dir, initial);
    _scheduler->loop(loop);
    _scheduler = nullptr;
}

namespace {

class DummyCard : public Card {
  public:
    void become_front() {
        if (!_inited) {
            sys_init();
            _inited = true;
        }
    }

  private:
    bool _inited = false;
};

}  // namespace

void TextVideoDriver::capture(vector<pair<unique_ptr<Card>, pn::string>>& pix) {
    MainLoop loop(*this, _output_dir, new DummyCard);
    for (auto& p : pix) {
        loop.top()->stack()->push(p.first.release());
        loop.draw();
        loop.snapshot_to(p.second);
        loop.top()->stack()->pop(loop.top());
    }
}

void TextVideoDriver::add_arg(pn::string_view arg, std::vector<std::pair<size_t, size_t>>& args) {
    size_t start = _log.size();
    _log += arg;
    args.push_back(make_pair(start, _log.size() - start));
}

void TextVideoDriver::dup_arg(size_t index, std::vector<std::pair<size_t, size_t>>& args) {
    args.push_back(_last_args[index]);
}

pn::string_view TextVideoDriver::last_arg(size_t index) const {
    return _log.substr(_last_args[index].first, _last_args[index].second);
}

static pn::string log_string(int i) { return pn::dump(i, pn::dump_short); }
static pn::string log_string(pn::string_view s) { return s.copy(); }

template <typename... Args>
void TextVideoDriver::log(pn::string_view command, const Args&... args) {
    vector<pair<size_t, size_t>> this_args;
    pn::string                   str_args[]  = {log_string(args)...};
    bool                         new_command = _last_args.empty() || (command != last_arg(0));

    if (new_command) {
        add_arg(command, this_args);
    } else {
        dup_arg(0, this_args);
    }
    for (size_t i = 0; i < sizeof...(args); ++i) {
        _log += "\t";
        pn::string_view s(str_args[i]);
        if (new_command || (s != last_arg(i + 1))) {
            add_arg(s, this_args);
        } else {
            dup_arg(i + 1, this_args);
        }
    }
    _log += "\n";

    using std::swap;
    swap(this_args, _last_args);
}

}  // namespace antares
