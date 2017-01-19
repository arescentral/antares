// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2015-2017 The Antares Authors
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

#ifndef ANTARES_GLFW_VIDEO_DRIVER_HPP_
#define ANTARES_GLFW_VIDEO_DRIVER_HPP_

#include <queue>
#include <sfz/sfz.hpp>
#include <stack>

#include "config/keys.hpp"
#include "drawing/color.hpp"
#include "math/geometry.hpp"
#include "video/opengl-driver.hpp"

struct GLFWwindow;

namespace antares {

class Event;

class GLFWVideoDriver : public OpenGlVideoDriver {
  public:
    GLFWVideoDriver();
    virtual ~GLFWVideoDriver();

    virtual Size viewport_size() const { return _viewport_size; }
    virtual Size screen_size() const { return _screen_size; }

    virtual Point     get_mouse();
    virtual InputMode input_mode() const;

    virtual wall_time now() const;

    void loop(Card* initial);

  private:
    void key(int key, int scancode, int action, int mods);
    void mouse_button(int button, int action, int mods);
    void mouse_move(double x, double y);
    void window_size(int width, int height);
    static void key_callback(GLFWwindow* w, int key, int scancode, int action, int mods);
    static void mouse_button_callback(GLFWwindow* w, int button, int action, int mods);
    static void mouse_move_callback(GLFWwindow* w, double x, double y);
    static void window_size_callback(GLFWwindow* w, int width, int height);

    Size        _screen_size;
    Size        _viewport_size;
    GLFWwindow* _window;
    MainLoop*   _loop;
    wall_time   _last_click_usecs;
    int         _last_click_count;

    DISALLOW_COPY_AND_ASSIGN(GLFWVideoDriver);
};

}  // namespace antares

#endif  // ANTARES_GLFW_VIDEO_DRIVER_HPP_
