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

#include "glfw/video-driver.hpp"

#include <GLFW/glfw3.h>
#include <sys/time.h>
#include <unistd.h>
#include <sfz/sfz.hpp>

#include "config/preferences.hpp"

using sfz::Exception;
using sfz::String;
using sfz::format;
using sfz::range;

namespace utf8 = sfz::utf8;

namespace antares {

static const ticks kDoubleClickInterval = ticks(30);

static int kGLFWKeyToUSB[GLFW_KEY_LAST + 1] = {
                [GLFW_KEY_SPACE]      = Keys::SPACE,
                [GLFW_KEY_APOSTROPHE] = Keys::QUOTE,
                [GLFW_KEY_COMMA]      = Keys::COMMA,
                Keys::MINUS,
                Keys::PERIOD,
                Keys::SLASH,
                [GLFW_KEY_0] = Keys::K0,
                Keys::K1,
                Keys::K2,
                Keys::K3,
                Keys::K4,
                Keys::K5,
                Keys::K6,
                Keys::K7,
                Keys::K8,
                Keys::K9,
                [GLFW_KEY_SEMICOLON] = Keys::SEMICOLON,
                [GLFW_KEY_EQUAL]     = Keys::EQUALS,
                [GLFW_KEY_A]         = Keys::A,
                Keys::B,
                Keys::C,
                Keys::D,
                Keys::E,
                Keys::F,
                Keys::G,
                Keys::H,
                Keys::I,
                Keys::J,
                Keys::K,
                Keys::L,
                Keys::M,
                Keys::N,
                Keys::O,
                Keys::P,
                Keys::Q,
                Keys::R,
                Keys::S,
                Keys::T,
                Keys::U,
                Keys::V,
                Keys::W,
                Keys::X,
                Keys::Y,
                Keys::Z,
                [GLFW_KEY_LEFT_BRACKET] = Keys::L_BRACKET,
                Keys::BACKSLASH,
                Keys::R_BRACKET,
                [GLFW_KEY_GRAVE_ACCENT] = Keys::BACKTICK,
                [GLFW_KEY_WORLD_1]      = 0,
                0,
                [GLFW_KEY_ESCAPE] = Keys::ESCAPE,
                Keys::RETURN,
                Keys::TAB,
                Keys::BACKSPACE,
                0 /* Keys::INSERT */,
                Keys::DELETE,
                [GLFW_KEY_RIGHT] = Keys::RIGHT_ARROW,
                Keys::LEFT_ARROW,
                Keys::DOWN_ARROW,
                Keys::UP_ARROW,
                Keys::PAGE_UP,
                Keys::PAGE_DOWN,
                Keys::HOME,
                Keys::END,
                [GLFW_KEY_CAPS_LOCK] = Keys::CAPS_LOCK,
                0 /* SCROLL_LOCK */,
                0 /* NUM_LOCK */,
                [GLFW_KEY_PRINT_SCREEN] = 0 /* PRINT_SCREEN */,
                0 /* PAUSE */,
                [GLFW_KEY_F1] = Keys::F1,
                Keys::F2,
                Keys::F3,
                Keys::F4,
                Keys::F5,
                Keys::F6,
                Keys::F7,
                Keys::F8,
                Keys::F9,
                Keys::F10,
                Keys::F11,
                Keys::F12,
                Keys::F13,
                Keys::F14,
                Keys::F15,
                [GLFW_KEY_KP_1] = Keys::N1,
                Keys::N2,
                Keys::N3,
                Keys::N4,
                Keys::N5,
                Keys::N6,
                Keys::N7,
                Keys::N8,
                Keys::N9,
                [GLFW_KEY_KP_DECIMAL] = Keys::N_PERIOD,
                Keys::N_DIVIDE,
                Keys::N_TIMES,
                Keys::N_MINUS,
                Keys::N_PLUS,
                Keys::N_ENTER,
                Keys::N_EQUALS,
                [GLFW_KEY_LEFT_SHIFT] = Keys::L_SHIFT,
                Keys::L_CONTROL,
                Keys::L_OPTION,
                Keys::L_COMMAND,
                [GLFW_KEY_RIGHT_SHIFT] = Keys::R_SHIFT,
                Keys::R_CONTROL,
                Keys::R_OPTION,
                Keys::R_COMMAND,
                [GLFW_KEY_MENU] = 0 /* MENU */,
};

static void throw_error(int code, const char* message) {
    throw Exception(format("{0}: {1}", code, utf8::decode(message)));
}

GLFWVideoDriver::GLFWVideoDriver() : _screen_size(640, 480), _last_click_count(0) {
    if (!glfwInit()) {
        throw Exception("glfwInit()");
    }
    glfwSetErrorCallback(throw_error);
}

GLFWVideoDriver::~GLFWVideoDriver() {
    glfwTerminate();
}

Point GLFWVideoDriver::get_mouse() {
    double x, y;
    glfwGetCursorPos(_window, &x, &y);
    return {int(x), int(y)};
}

InputMode GLFWVideoDriver::input_mode() const {
    return KEYBOARD_MOUSE;
}

wall_time GLFWVideoDriver::now() const {
    return wall_time(usecs(int64_t(glfwGetTime() * 1e6)));
}

void GLFWVideoDriver::key(int key, int scancode, int action, int mods) {
    if (key < 0) {
        return;
    }
    key = kGLFWKeyToUSB[key];
    if (!key) {
        return;
    }
    String name;
    GetKeyNumName(key + 1, &name);
    if (action == GLFW_PRESS) {
        KeyDownEvent(now(), key).send(_loop->top());
    } else if (action == GLFW_RELEASE) {
        KeyUpEvent(now(), key).send(_loop->top());
    } else {
        return;
    }
}

void GLFWVideoDriver::mouse_button(int button, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (now() <= (_last_click_usecs + kDoubleClickInterval)) {
            _last_click_count += 1;
        } else {
            _last_click_count = 1;
        }
        MouseDownEvent(now(), button, _last_click_count, get_mouse()).send(_loop->top());
        _last_click_usecs = now();
    } else if (action == GLFW_RELEASE) {
        MouseUpEvent(now(), button, get_mouse()).send(_loop->top());
    } else {
        return;
    }
}

void GLFWVideoDriver::mouse_move(double x, double y) {
    MouseMoveEvent(now(), Point(x, y)).send(_loop->top());
}

void GLFWVideoDriver::window_size(int width, int height) {
    _screen_size = {width, height};
    glfwGetFramebufferSize(_window, &_viewport_size.width, &_viewport_size.height);
}

void GLFWVideoDriver::key_callback(GLFWwindow* w, int key, int scancode, int action, int mods) {
    GLFWVideoDriver* driver = reinterpret_cast<GLFWVideoDriver*>(glfwGetWindowUserPointer(w));
    driver->key(key, scancode, action, mods);
}

void GLFWVideoDriver::mouse_button_callback(GLFWwindow* w, int button, int action, int mods) {
    GLFWVideoDriver* driver = reinterpret_cast<GLFWVideoDriver*>(glfwGetWindowUserPointer(w));
    driver->mouse_button(button, action, mods);
}

void GLFWVideoDriver::mouse_move_callback(GLFWwindow* w, double x, double y) {
    GLFWVideoDriver* driver = reinterpret_cast<GLFWVideoDriver*>(glfwGetWindowUserPointer(w));
    driver->mouse_move(x, y);
}

void GLFWVideoDriver::window_size_callback(GLFWwindow* w, int width, int height) {
    GLFWVideoDriver* driver = reinterpret_cast<GLFWVideoDriver*>(glfwGetWindowUserPointer(w));
    driver->window_size(width, height);
}

void GLFWVideoDriver::loop(Card* initial) {
    /* Create a windowed mode window and its OpenGL context */
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    _window = glfwCreateWindow(_screen_size.width, _screen_size.height, "", NULL, NULL);
    if (!_window) {
        throw Exception("glfwCreateWindow");
    }
    glfwGetFramebufferSize(_window, &_viewport_size.width, &_viewport_size.height);
    glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    glfwSetWindowUserPointer(_window, this);
    glfwSetKeyCallback(_window, key_callback);
    glfwSetMouseButtonCallback(_window, mouse_button_callback);
    glfwSetCursorPosCallback(_window, mouse_move_callback);
    glfwSetWindowSizeCallback(_window, window_size_callback);

    /* Make the _window's context current */
    glfwMakeContextCurrent(_window);

    MainLoop main_loop(*this, initial);
    _loop = &main_loop;
    main_loop.draw();

    while (!main_loop.done() && !glfwWindowShouldClose(_window)) {
        glfwPollEvents();
        _loop->draw();
        glfwSwapBuffers(_window);
        wall_time at;
        if (main_loop.top()->next_timer(at) && (now() > at)) {
            main_loop.top()->fire_timer();
            main_loop.draw();
            glfwSwapBuffers(_window);
        } else {
            usleep(10);
        }
    }
}

}  // namespace antares
