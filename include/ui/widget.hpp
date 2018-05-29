// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2018 The Antares Authors
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

#ifndef ANTARES_UI_WIDGET_HPP_
#define ANTARES_UI_WIDGET_HPP_

#include <vector>

#include "data/interface.hpp"
#include "drawing/interface.hpp"
#include "math/geometry.hpp"

namespace antares {

class Widget {
  public:
    virtual void draw(Point origin, InputMode mode) const = 0;
    virtual Rect inner_bounds() const                     = 0;
    virtual Rect outer_bounds() const                     = 0;
};

class BoxRect : public Widget {
  public:
    BoxRect(const BoxRectData& data);

    Hue            hue() const { return _hue; }
    InterfaceStyle style() const { return _style; }

    void draw(Point origin, InputMode mode) const override;
    Rect inner_bounds() const override;
    Rect outer_bounds() const override;

  private:
    void draw_labeled_box(Point origin) const;
    void draw_plain_rect(Point origin) const;

    Rect                      _inner_bounds;
    sfz::optional<pn::string> _label;
    Hue                       _hue   = Hue::GRAY;
    InterfaceStyle            _style = InterfaceStyle::LARGE;
};

class TextRect : public Widget {
  public:
    TextRect(const TextRectData& data);

    Hue            hue() const { return _hue; }
    InterfaceStyle style() const { return _style; }

    void draw(Point origin, InputMode mode) const override;
    Rect inner_bounds() const override;
    Rect outer_bounds() const override;

  private:
    Rect           _inner_bounds;
    pn::string     _text;
    Hue            _hue   = Hue::GRAY;
    InterfaceStyle _style = InterfaceStyle::LARGE;
};

class PictureRect : public Widget {
  public:
    PictureRect(const PictureRectData& data);

    void draw(Point origin, InputMode mode) const override;
    Rect inner_bounds() const override;
    Rect outer_bounds() const override;

  private:
    Rect    _inner_bounds;
    Texture _texture;
};

class Button : public Widget {
  public:
    int64_t         id() const { return _id; }
    pn::string_view label() const { return _label; }
    int16_t         key() const { return _key; }
    int16_t         gamepad() const { return _gamepad; }
    Hue             hue() const { return _hue; }
    InterfaceStyle  style() const { return _style; }
    bool            active() const { return _active; }
    bool            enabled() const { return _enabled; }

    int16_t& key() { return _key; }
    Hue&     hue() { return _hue; }
    bool&    active() { return _active; }
    bool&    enabled() { return _enabled; }

  protected:
    Button(const ButtonData& data);

  private:
    int64_t        _id;
    pn::string     _label;
    int16_t        _key;
    int16_t        _gamepad;
    Hue            _hue     = Hue::GRAY;
    InterfaceStyle _style   = InterfaceStyle::LARGE;
    bool           _enabled = true;
    bool           _active  = false;
};

class PlainButton : public Button {
  public:
    PlainButton(const PlainButtonData& data);

    void draw(Point origin, InputMode mode) const override;
    Rect inner_bounds() const override;
    Rect outer_bounds() const override;

  private:
    Rect _inner_bounds;
};

class CheckboxButton : public Button {
  public:
    CheckboxButton(const CheckboxButtonData& data);

    bool  on() const { return _on; }
    bool& on() { return _on; }

    void draw(Point origin, InputMode mode) const override;
    Rect inner_bounds() const override;
    Rect outer_bounds() const override;

  private:
    Rect _inner_bounds;
    bool _on = false;
};

class RadioButton : public Button {
  public:
    RadioButton(const RadioButtonData& data);

    bool  on() const { return _on; }
    bool& on() { return _on; }

    void draw(Point origin, InputMode mode) const override;
    Rect inner_bounds() const override;
    Rect outer_bounds() const override;

  private:
    Rect _inner_bounds;
    bool _on = false;
};

class TabBoxButton : public Button {
  public:
    TabBoxButton(const TabBoxButtonData& data);

    const std::vector<std::unique_ptr<InterfaceItemData>>& content() const { return _content; }
    bool                                                   on() const { return _on; }
    bool&                                                  on() { return _on; }

    void draw(Point origin, InputMode mode) const override;
    Rect inner_bounds() const override;
    Rect outer_bounds() const override;

  private:
    Rect                                            _inner_bounds;
    std::vector<std::unique_ptr<InterfaceItemData>> _content;
    bool                                            _on = false;
};

class TabBox : public Widget {
  public:
    TabBox(const TabBoxData& data);

    void draw(Point origin, InputMode mode) const override;
    Rect inner_bounds() const override;
    Rect outer_bounds() const override;

  private:
    Rect           _inner_bounds;
    int64_t        _top_right_border_size = 0;
    Hue            _hue                   = Hue::GRAY;
    InterfaceStyle _style                 = InterfaceStyle::LARGE;
};

}  // namespace antares

#endif  // ANTARES_UI_WIDGET_HPP_
