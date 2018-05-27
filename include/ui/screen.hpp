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

#ifndef ANTARES_UI_SCREEN_HPP_
#define ANTARES_UI_SCREEN_HPP_

#include <vector>

#include "data/interface.hpp"
#include "drawing/interface.hpp"
#include "game/cursor.hpp"
#include "math/geometry.hpp"
#include "ui/card.hpp"

namespace antares {

class InterfaceItem;
class BoxRect;
class TextRect;
class PictureRect;
class Button;
class PlainButton;
class CheckboxButton;
class RadioButton;
class TabBoxButton;
class TabBox;

class InterfaceScreen : public Card {
  public:
    InterfaceScreen(pn::string_view name, const Rect& bounds, bool full_screen);
    ~InterfaceScreen();

    virtual void become_front();
    virtual void resign_front();

    virtual void draw() const;

    virtual void mouse_down(const MouseDownEvent& event);
    virtual void mouse_up(const MouseUpEvent& event);
    virtual void mouse_move(const MouseMoveEvent& event);
    virtual void key_down(const KeyDownEvent& event);
    virtual void key_up(const KeyUpEvent& event);
    virtual void gamepad_button_down(const GamepadButtonDownEvent& event);
    virtual void gamepad_button_up(const GamepadButtonUpEvent& event);

  protected:
    virtual void overlay() const;
    virtual void adjust_interface();
    virtual void handle_button(ButtonData& button) = 0;

    void truncate(size_t size);
    void extend(const std::vector<std::unique_ptr<InterfaceItemData>>& items);

    Point                    offset() const;
    size_t                   size() const;
    const InterfaceItemData& item(int index) const;
    InterfaceItemData&       mutable_item(int index);

  private:
    enum State {
        NORMAL,
        MOUSE_DOWN,
        KEY_DOWN,
        GAMEPAD_DOWN,
    };
    State _state;

    pn::value load_pn(pn::string_view id);
    void      become_normal();

    const Rect                                  _bounds;
    const bool                                  _full_screen;
    std::vector<std::unique_ptr<InterfaceItem>> _items;
    Button*                                     _hit_button;
    uint32_t                                    _pressed;
    Cursor                                      _cursor;
};

class InterfaceItem {
  public:
    virtual void                     draw(Point origin, InputMode mode) const = 0;
    virtual Rect                     bounds() const                           = 0;
    virtual InterfaceItemData*       item()                                   = 0;
    virtual const InterfaceItemData* item() const                             = 0;
};

class BoxRect : public InterfaceItem {
  public:
    BoxRect(BoxRectData data) : data{std::move(data)} {}
    BoxRectData data;

    void               draw(Point origin, InputMode mode) const override;
    Rect               bounds() const override;
    BoxRectData*       item() override;
    const BoxRectData* item() const override;
};

class TextRect : public InterfaceItem {
  public:
    TextRect(TextRectData data) : data{std::move(data)} {}
    TextRectData data;

    void                draw(Point origin, InputMode mode) const override;
    Rect                bounds() const override;
    TextRectData*       item() override;
    const TextRectData* item() const override;
};

class PictureRect : public InterfaceItem {
  public:
    PictureRect(PictureRectData data) : data{std::move(data)} {}
    PictureRectData data;

    void                   draw(Point origin, InputMode mode) const override;
    Rect                   bounds() const override;
    PictureRectData*       item() override;
    const PictureRectData* item() const override;
};

class Button : public InterfaceItem {
  public:
    ButtonData*       item() override       = 0;
    const ButtonData* item() const override = 0;
};

class PlainButton : public Button {
  public:
    PlainButton(PlainButtonData data) : data{std::move(data)} {}
    PlainButtonData data;

    void                   draw(Point origin, InputMode mode) const override;
    Rect                   bounds() const override;
    PlainButtonData*       item() override;
    const PlainButtonData* item() const override;
};

class CheckboxButton : public Button {
  public:
    CheckboxButton(CheckboxButtonData data) : data{std::move(data)} {}
    CheckboxButtonData data;

    void                      draw(Point origin, InputMode mode) const override;
    Rect                      bounds() const override;
    CheckboxButtonData*       item() override;
    const CheckboxButtonData* item() const override;
};

class RadioButton : public Button {
  public:
    RadioButton(RadioButtonData data) : data{std::move(data)} {}
    RadioButtonData data;

    void                   draw(Point origin, InputMode mode) const override;
    Rect                   bounds() const override;
    RadioButtonData*       item() override;
    const RadioButtonData* item() const override;
};

class TabBoxButton : public Button {
  public:
    TabBoxButton(TabBoxButtonData data) : data{std::move(data)} {}
    TabBoxButtonData data;

    void                    draw(Point origin, InputMode mode) const override;
    Rect                    bounds() const override;
    TabBoxButtonData*       item() override;
    const TabBoxButtonData* item() const override;
};

class TabBox : public InterfaceItem {
  public:
    TabBox(TabBoxData data) : data{std::move(data)} {}
    TabBoxData data;

    void              draw(Point origin, InputMode mode) const override;
    Rect              bounds() const override;
    TabBoxData*       item() override;
    const TabBoxData* item() const override;
};

}  // namespace antares

#endif  // ANTARES_UI_SCREEN_HPP_
