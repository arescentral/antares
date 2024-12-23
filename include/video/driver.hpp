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

#ifndef ANTARES_VIDEO_DRIVER_HPP_
#define ANTARES_VIDEO_DRIVER_HPP_

#include <stdint.h>
#include <memory>
#include <pn/string>

#include "drawing/color.hpp"
#include "math/geometry.hpp"
#include "math/units.hpp"
#include "ui/event.hpp"

namespace antares {

class Card;
class PixMap;
class Texture;
class TextReceiver;

class VideoDriver {
  public:
    VideoDriver();
    VideoDriver(const VideoDriver&)            = delete;
    VideoDriver& operator=(const VideoDriver&) = delete;

    virtual ~VideoDriver();
    virtual Point     get_mouse()         = 0;
    virtual InputMode input_mode() const  = 0;
    virtual int       scale() const       = 0;
    virtual Size      screen_size() const = 0;

    virtual bool start_editing(TextReceiver* text) = 0;
    virtual void stop_editing(TextReceiver* text)  = 0;

    virtual wall_time now() const = 0;

    virtual Texture texture(pn::string_view name, const PixMap& content, int scale) = 0;
    virtual void    dither_rect(const Rect& rect, const RgbColor& color)            = 0;
    virtual void    draw_triangle(const Rect& rect, const RgbColor& color)          = 0;
    virtual void    draw_diamond(const Rect& rect, const RgbColor& color)           = 0;
    virtual void    draw_plus(const Rect& rect, const RgbColor& color)              = 0;

  private:
    friend class Points;
    friend class Lines;
    friend class Rects;

    virtual void begin_points() {}
    virtual void end_points() {}
    virtual void batch_point(const Point& at, const RgbColor& color) = 0;

    virtual void begin_lines() {}
    virtual void end_lines() {}
    virtual void batch_line(const Point& from, const Point& to, const RgbColor& color) = 0;

    virtual void begin_rects() {}
    virtual void end_rects() {}
    virtual void batch_rect(const Rect& rect, const RgbColor& color) = 0;
};

class Texture {
  public:
    struct Impl {
        Impl() {}
        Impl(const Impl&)            = delete;
        Impl& operator=(const Impl&) = delete;
        virtual ~Impl();

        virtual pn::string_view name() const                      = 0;
        virtual void            draw(const Rect& draw_rect) const = 0;
        virtual void            draw_cropped(
                           const Rect& dest, const Rect& source, const RgbColor& tint) const = 0;
        virtual void draw_shaded(const Rect& draw_rect, const RgbColor& tint) const          = 0;
        virtual void draw_static(
                const Rect& draw_rect, const RgbColor& color, uint8_t frac) const = 0;
        virtual void draw_outlined(
                const Rect& draw_rect, const RgbColor& outline_color,
                const RgbColor& fill_color) const = 0;
        virtual const Size& size() const          = 0;

        virtual void begin_quads() const {}
        virtual void end_quads() const {}
        virtual void draw_quad(const Rect& dest, const Rect& source, const RgbColor& tint) const {
            draw_cropped(dest, source, tint);
        }
    };

    Texture(std::nullptr_t n = nullptr) {}
    Texture(std::unique_ptr<Impl> impl) : _impl(std::move(impl)) {}

    operator bool() const { return _impl != nullptr; }

    void draw(const Rect& draw_rect) const { _impl->draw(draw_rect); }
    void draw(int32_t x, int32_t y) const { _impl->draw(rect(x, y)); }

    void draw_cropped(const Rect& dest, const Rect& source) const {
        _impl->draw_cropped(dest, source, RgbColor::white());
    }

    void draw_static(const Rect& draw_rect, const RgbColor& color, uint8_t frac) const {
        _impl->draw_static(draw_rect, color, frac);
    }

    void draw_outlined(
            const Rect& draw_rect, const RgbColor& outline_color,
            const RgbColor& fill_color) const {
        _impl->draw_outlined(draw_rect, outline_color, fill_color);
    }

    void draw_shaded(const Rect& draw_rect, const RgbColor& tint) const {
        _impl->draw_shaded(draw_rect, tint);
    }
    void draw_shaded(int32_t x, int32_t y, const RgbColor& tint) const {
        _impl->draw_shaded(rect(x, y), tint);
    }

    void draw_static(int32_t x, int32_t y, const RgbColor& color, uint8_t frac) const {
        _impl->draw_static(rect(x, y), color, frac);
    }

    void draw_outlined(
            int32_t x, int32_t y, const RgbColor& outline_color,
            const RgbColor& fill_color) const {
        _impl->draw_outlined(rect(x, y), outline_color, fill_color);
    }

    const Size& size() const { return _impl->size(); }

  private:
    friend class Quads;

    Rect rect(int32_t x, int32_t y) const {
        return Rect(x, y, x + _impl->size().width, y + _impl->size().height);
    }

    std::unique_ptr<Impl> _impl;
};

class TextReceiver {
  public:
    template <typename T>
    struct range {
        T begin, end;
    };

    virtual ~TextReceiver();

    virtual void replace(
            range<int> replace, pn::string_view text) = 0;  // clears mark, selects end
    virtual void select(range<int> select)            = 0;  // clears mark
    virtual void mark(range<int> mark)                = 0;
    virtual void accept()                             = 0;  // might replace with \n
    virtual void newline()                            = 0;  // should replace with \n
    virtual void tab()                                = 0;  // may replace with \t
    virtual void escape()                             = 0;

    enum Offset {
        PREV_SAME  = -4,  // Previous equivalent position (line only)
        PREV_START = -3,  // Previous start of (glyph, word, line, paragraph)
        PREV_END   = -2,  // Previous end of (glyph, word, line, paragraph)
        THIS_START = -1,  // PREV_START, unless already at a start
        THIS_END   = +1,  // NEXT_END, unless already at a start
        NEXT_START = +2,  // Next start of (glyph, word, line, paragraph)
        NEXT_END   = +3,  // Next end of (glyph, word, line, paragraph)
        NEXT_SAME  = +4,  // Next equivalent position (line only)
    };
    enum OffsetUnit {
        GLYPHS     = 0,  // Composited character + modifiers.
        WORDS      = 1,  // Alphanumeric sequence, including internal [‘'’.]
        LINES      = 2,  // Visual line, including spaces at end.
        PARAGRAPHS = 3,  // Paragraph, excluding newline at end.
    };
    virtual int             offset(int origin, Offset offset, OffsetUnit unit) const = 0;
    virtual int             size() const                                             = 0;
    virtual range<int>      selection() const                                        = 0;
    virtual range<int>      mark() const                                             = 0;
    virtual pn::string_view text(range<int> range) const                             = 0;
};

class Points {
  public:
    Points();
    ~Points();
    void draw(const Point& at, const RgbColor& color) const;
};

class Lines {
  public:
    Lines();
    ~Lines();
    void draw(const Point& from, const Point& to, const RgbColor& color) const;
};

class Rects {
  public:
    Rects();
    ~Rects();
    void fill(const Rect& rect, const RgbColor& color) const;
};

class Quads {
  public:
    Quads(const Texture& sprite);
    ~Quads();
    void draw(const Rect& dest, const Rect& source, const RgbColor& tint) const;

  private:
    const Texture& _sprite;
};

}  // namespace antares

#endif  // ANTARES_VIDEO_DRIVER_HPP_
