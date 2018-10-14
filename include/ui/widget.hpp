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
#include "video/driver.hpp"

namespace antares {

class TabBox;

class Widget {
  public:
    static std::unique_ptr<Widget> from(const WidgetData& data);

    virtual ~Widget();

    virtual sfz::optional<int64_t> id() const = 0;

    virtual Widget* accept_click(Point where);
    virtual Widget* accept_key(Key which);
    virtual Widget* accept_button(Gamepad::Button which);
    virtual void    action();

    virtual void activate();
    virtual void deactivate();
    virtual void draw(Point origin, InputMode mode) const = 0;
    virtual Rect inner_bounds() const                     = 0;
    virtual Rect outer_bounds() const                     = 0;

    virtual std::vector<const Widget*> children() const;
    virtual std::vector<Widget*>       children();
};

class BoxRect : public Widget {
  public:
    BoxRect(const BoxRectData& data);

    sfz::optional<int64_t> id() const override { return _id; }

    Hue            hue() const { return _hue; }
    InterfaceStyle style() const { return _style; }

    void draw(Point origin, InputMode mode) const override;
    Rect inner_bounds() const override;
    Rect outer_bounds() const override;

  private:
    void draw_labeled_box(Point origin) const;
    void draw_plain_rect(Point origin) const;

    Rect                      _inner_bounds;
    sfz::optional<int64_t>    _id;
    sfz::optional<pn::string> _label;
    Hue                       _hue   = Hue::GRAY;
    InterfaceStyle            _style = InterfaceStyle::LARGE;
};

class TextRect : public Widget {
  public:
    TextRect(const TextRectData& data);

    sfz::optional<int64_t> id() const override { return _id; }

    Hue            hue() const { return _hue; }
    InterfaceStyle style() const { return _style; }

    void draw(Point origin, InputMode mode) const override;
    Rect inner_bounds() const override;
    Rect outer_bounds() const override;

  private:
    Rect                      _inner_bounds;
    sfz::optional<int64_t>    _id;
    sfz::optional<pn::string> _text;
    Hue                       _hue   = Hue::GRAY;
    InterfaceStyle            _style = InterfaceStyle::LARGE;
};

class PictureRect : public Widget {
  public:
    PictureRect(const PictureRectData& data);

    sfz::optional<int64_t> id() const override { return _id; }

    void draw(Point origin, InputMode mode) const override;
    Rect inner_bounds() const override;
    Rect outer_bounds() const override;

  private:
    Rect                   _inner_bounds;
    sfz::optional<int64_t> _id;
    Texture                _texture;
};

class Button : public Widget {
  public:
    sfz::optional<int64_t> id() const override { return _id; }

    Widget* accept_click(Point where) override;
    Widget* accept_key(Key which) override;
    Widget* accept_button(Gamepad::Button which) override;

    void activate() override { _active = true; }
    void deactivate() override { _active = false; }

    pn::string_view label() const { return _label; }
    Key             key() const { return _key; }
    Gamepad::Button gamepad() const { return _gamepad; }
    Hue             hue() const { return _hue; }
    InterfaceStyle  style() const { return _style; }
    bool            active() const { return _active; }
    virtual bool    enabled() const = 0;

    Key& key() { return _key; }
    Hue& hue() { return _hue; }

  protected:
    Button(const ButtonData& data);

  private:
    sfz::optional<int64_t> _id;
    pn::string             _label;
    Key                    _key;
    Gamepad::Button        _gamepad;
    Hue                    _hue    = Hue::GRAY;
    InterfaceStyle         _style  = InterfaceStyle::LARGE;
    bool                   _active = false;
};

class PlainButton : public Button {
  public:
    struct Action {
        std::function<void()> perform;
        std::function<bool()> possible;
    };

    PlainButton(const PlainButtonData& data);

    void bind(Action a);
    void action() override;
    bool enabled() const override;

    void draw(Point origin, InputMode mode) const override;
    Rect inner_bounds() const override;
    Rect outer_bounds() const override;

  private:
    Rect   _inner_bounds;
    Action _action;
};

class CheckboxButton : public Button {
  public:
    struct Value {
        std::function<bool()>     get;
        std::function<void(bool)> set;
        std::function<bool()>     modifiable;
    };

    CheckboxButton(const CheckboxButtonData& data);

    void         bind(Value v);
    bool         get() const;
    void         set(bool on);
    bool         enabled() const override;
    virtual void action() override;

    void draw(Point origin, InputMode mode) const override;
    Rect inner_bounds() const override;
    Rect outer_bounds() const override;

  private:
    Rect  _inner_bounds;
    Value _value;
};

class RadioButton : public Button {
  public:
    RadioButton(const RadioButtonData& data);

    bool  on() const { return _on; }
    bool& on() { return _on; }
    bool  enabled() const override { return false; }

    void draw(Point origin, InputMode mode) const override;
    Rect inner_bounds() const override;
    Rect outer_bounds() const override;

  private:
    Rect _inner_bounds;
    bool _on = false;
};

class TabButton : public Button {
  public:
    TabButton(TabBox* box, const TabBoxData::Tab& data, Rect bounds);

    TabBox*                                     parent() const { return _parent; }
    const std::vector<std::unique_ptr<Widget>>& content() const { return _content; }
    bool                                        on() const { return _on; }
    bool&                                       on() { return _on; }
    bool                                        enabled() const override { return true; }
    virtual void                                action() override;

    void draw(Point origin, InputMode mode) const override;
    Rect inner_bounds() const override;
    Rect outer_bounds() const override;

  private:
    TabBox*                              _parent = nullptr;
    Rect                                 _inner_bounds;
    std::vector<std::unique_ptr<Widget>> _content;
    bool                                 _on = false;
};

class TabBox : public Widget {
  public:
    TabBox(const TabBoxData& data);

    sfz::optional<int64_t> id() const override { return _id; }

    Widget* accept_click(Point where) override;
    Widget* accept_key(Key which) override;
    Widget* accept_button(Gamepad::Button which) override;

    void draw(Point origin, InputMode mode) const override;
    Rect inner_bounds() const override;
    Rect outer_bounds() const override;

    std::vector<const Widget*> children() const override;
    std::vector<Widget*>       children() override;

    int            current_tab() const { return _current_tab; }
    Hue            hue() const { return _hue; }
    InterfaceStyle style() const { return _style; }

    void select(const TabButton& tab);

  private:
    template <typename MaybeConstWidget>
    std::vector<MaybeConstWidget*> children() const;

    Rect                                    _inner_bounds;
    sfz::optional<int64_t>                  _id;
    int                                     _current_tab           = 0;
    int64_t                                 _top_right_border_size = 0;
    Hue                                     _hue                   = Hue::GRAY;
    InterfaceStyle                          _style                 = InterfaceStyle::LARGE;
    std::vector<std::unique_ptr<TabButton>> _tabs;
};

}  // namespace antares

#endif  // ANTARES_UI_WIDGET_HPP_
