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
    BoxRect(BoxRectData data) : data{std::move(data)} {}

    Hue            hue() const { return data.hue; }
    InterfaceStyle style() const { return data.style; }

    void draw(Point origin, InputMode mode) const override;
    Rect inner_bounds() const override;
    Rect outer_bounds() const override;

  private:
    BoxRectData data;
};

class TextRect : public Widget {
  public:
    TextRect(TextRectData data) : data{std::move(data)} {}

    Hue            hue() const { return data.hue; }
    InterfaceStyle style() const { return data.style; }

    void draw(Point origin, InputMode mode) const override;
    Rect inner_bounds() const override;
    Rect outer_bounds() const override;

  private:
    TextRectData data;
};

class PictureRect : public Widget {
  public:
    PictureRect(PictureRectData data) : data{std::move(data)} {}
    Texture texture;

    void draw(Point origin, InputMode mode) const override;
    Rect inner_bounds() const override;
    Rect outer_bounds() const override;

  private:
    PictureRectData data;
};

class Button : public Widget {
  public:
    ButtonState state = ButtonState::ENABLED;

    virtual int64_t        id() const      = 0;
    virtual int16_t        key() const     = 0;
    virtual int16_t        gamepad() const = 0;
    virtual Hue            hue() const     = 0;
    virtual InterfaceStyle style() const   = 0;
};

class PlainButton : public Button {
  public:
    PlainButton(PlainButtonData data) : data{std::move(data)} {}

    void draw(Point origin, InputMode mode) const override;
    Rect inner_bounds() const override;
    Rect outer_bounds() const override;

    int64_t        id() const override;
    int16_t        key() const override;
    int16_t        gamepad() const override;
    Hue            hue() const override;
    InterfaceStyle style() const override;

    int16_t& key() { return data.key; }
    Hue&     hue() { return data.hue; }

  private:
    PlainButtonData data;
};

class CheckboxButton : public Button {
  public:
    CheckboxButton(CheckboxButtonData data) : data{std::move(data)} {}
    bool on = false;

    void draw(Point origin, InputMode mode) const override;
    Rect inner_bounds() const override;
    Rect outer_bounds() const override;

    int64_t        id() const override;
    int16_t        key() const override;
    int16_t        gamepad() const override;
    Hue            hue() const override;
    InterfaceStyle style() const override;

  private:
    CheckboxButtonData data;
};

class RadioButton : public Button {
  public:
    RadioButton(RadioButtonData data) : data{std::move(data)} {}
    bool on = false;

    void draw(Point origin, InputMode mode) const override;
    Rect inner_bounds() const override;
    Rect outer_bounds() const override;

    int64_t        id() const override;
    int16_t        key() const override;
    int16_t        gamepad() const override;
    Hue            hue() const override;
    InterfaceStyle style() const override;

  private:
    RadioButtonData data;
};

class TabBoxButton : public Button {
  public:
    TabBoxButton(TabBoxButtonData data) : data{std::move(data)} {}
    bool on = false;

    const std::vector<std::unique_ptr<InterfaceItemData>>& content() const { return data.content; }

    int16_t& key() { return data.key; }
    Hue&     hue() { return data.hue; }

    void draw(Point origin, InputMode mode) const override;
    Rect inner_bounds() const override;
    Rect outer_bounds() const override;

    int64_t        id() const override;
    int16_t        key() const override;
    int16_t        gamepad() const override;
    Hue            hue() const override;
    InterfaceStyle style() const override;

  private:
    TabBoxButtonData data;
};

class TabBox : public Widget {
  public:
    TabBox(TabBoxData data) : data{std::move(data)} {}

    void draw(Point origin, InputMode mode) const override;
    Rect inner_bounds() const override;
    Rect outer_bounds() const override;

  private:
    TabBoxData data;
};

}  // namespace antares

#endif  // ANTARES_UI_WIDGET_HPP_
