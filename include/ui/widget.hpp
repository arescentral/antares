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
    virtual void                     draw(Point origin, InputMode mode) const = 0;
    virtual Rect                     inner_bounds() const                     = 0;
    virtual Rect                     outer_bounds() const                     = 0;
    virtual InterfaceItemData*       item()                                   = 0;
    virtual const InterfaceItemData* item() const                             = 0;
};

class BoxRect : public Widget {
  public:
    BoxRect(BoxRectData data) : data{std::move(data)} {}
    BoxRectData data;

    void               draw(Point origin, InputMode mode) const override;
    Rect               inner_bounds() const override;
    Rect               outer_bounds() const override;
    BoxRectData*       item() override;
    const BoxRectData* item() const override;
};

class TextRect : public Widget {
  public:
    TextRect(TextRectData data) : data{std::move(data)} {}
    TextRectData data;

    Hue            hue() const { return item()->hue; }
    InterfaceStyle style() const { return item()->style; }

    void                draw(Point origin, InputMode mode) const override;
    Rect                inner_bounds() const override;
    Rect                outer_bounds() const override;
    TextRectData*       item() override;
    const TextRectData* item() const override;
};

class PictureRect : public Widget {
  public:
    PictureRect(PictureRectData data) : data{std::move(data)} {}
    PictureRectData data;
    Texture         texture;

    void                   draw(Point origin, InputMode mode) const override;
    Rect                   inner_bounds() const override;
    Rect                   outer_bounds() const override;
    PictureRectData*       item() override;
    const PictureRectData* item() const override;
};

class Button : public Widget {
  public:
    ButtonState state = ButtonState::ENABLED;

    int64_t        id() const { return item()->id; }
    int16_t        key() const { return item()->key; }
    int16_t        gamepad() const { return item()->gamepad; }
    Hue            hue() const { return item()->hue; }
    InterfaceStyle style() const { return item()->style; }
    int16_t&       key() { return item()->key; }
    Hue&           hue() { return item()->hue; }

    ButtonData*       item() override       = 0;
    const ButtonData* item() const override = 0;
};

class PlainButton : public Button {
  public:
    PlainButton(PlainButtonData data) : data{std::move(data)} {}
    PlainButtonData data;

    void                   draw(Point origin, InputMode mode) const override;
    Rect                   inner_bounds() const override;
    Rect                   outer_bounds() const override;
    PlainButtonData*       item() override;
    const PlainButtonData* item() const override;
};

class CheckboxButton : public Button {
  public:
    CheckboxButton(CheckboxButtonData data) : data{std::move(data)} {}
    CheckboxButtonData data;
    bool               on = false;

    void                      draw(Point origin, InputMode mode) const override;
    Rect                      inner_bounds() const override;
    Rect                      outer_bounds() const override;
    CheckboxButtonData*       item() override;
    const CheckboxButtonData* item() const override;
};

class RadioButton : public Button {
  public:
    RadioButton(RadioButtonData data) : data{std::move(data)} {}
    RadioButtonData data;
    bool            on = false;

    void                   draw(Point origin, InputMode mode) const override;
    Rect                   inner_bounds() const override;
    Rect                   outer_bounds() const override;
    RadioButtonData*       item() override;
    const RadioButtonData* item() const override;
};

class TabBoxButton : public Button {
  public:
    TabBoxButton(TabBoxButtonData data) : data{std::move(data)} {}
    TabBoxButtonData data;
    bool             on = false;

    const std::vector<std::unique_ptr<InterfaceItemData>>& content() const {
        return item()->content;
    }

    void                    draw(Point origin, InputMode mode) const override;
    Rect                    inner_bounds() const override;
    Rect                    outer_bounds() const override;
    TabBoxButtonData*       item() override;
    const TabBoxButtonData* item() const override;
};

class TabBox : public Widget {
  public:
    TabBox(TabBoxData data) : data{std::move(data)} {}
    TabBoxData data;

    void              draw(Point origin, InputMode mode) const override;
    Rect              inner_bounds() const override;
    Rect              outer_bounds() const override;
    TabBoxData*       item() override;
    const TabBoxData* item() const override;
};

}  // namespace antares

#endif  // ANTARES_UI_WIDGET_HPP_
