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
#include <pn/file>
#include <sfz/sfz.hpp>

#include "config/preferences.hpp"

using sfz::range;

namespace antares {

static const ticks kDoubleClickInterval = ticks(30);

static Key kGLFWKeyToUSB[GLFW_KEY_LAST + 1] = {
        [GLFW_KEY_SPACE]      = Key::SPACE,
        [GLFW_KEY_APOSTROPHE] = Key::QUOTE,
        [GLFW_KEY_COMMA]      = Key::COMMA,
        Key::MINUS,
        Key::PERIOD,
        Key::SLASH,
        [GLFW_KEY_0] = Key::K0,
        Key::K1,
        Key::K2,
        Key::K3,
        Key::K4,
        Key::K5,
        Key::K6,
        Key::K7,
        Key::K8,
        Key::K9,
        [GLFW_KEY_SEMICOLON] = Key::SEMICOLON,
        [GLFW_KEY_EQUAL]     = Key::EQUALS,
        [GLFW_KEY_A]         = Key::A,
        Key::B,
        Key::C,
        Key::D,
        Key::E,
        Key::F,
        Key::G,
        Key::H,
        Key::I,
        Key::J,
        Key::K,
        Key::L,
        Key::M,
        Key::N,
        Key::O,
        Key::P,
        Key::Q,
        Key::R,
        Key::S,
        Key::T,
        Key::U,
        Key::V,
        Key::W,
        Key::X,
        Key::Y,
        Key::Z,
        [GLFW_KEY_LEFT_BRACKET] = Key::L_BRACKET,
        Key::BACKSLASH,
        Key::R_BRACKET,
        [GLFW_KEY_GRAVE_ACCENT] = Key::BACKTICK,
        [GLFW_KEY_WORLD_1]      = Key::NONE,
        Key::NONE,
        [GLFW_KEY_ESCAPE] = Key::ESCAPE,
        Key::RETURN,
        Key::TAB,
        Key::BACKSPACE,
        Key::NONE /* Key::INSERT */,
        Key::DELETE,
        [GLFW_KEY_RIGHT] = Key::RIGHT_ARROW,
        Key::LEFT_ARROW,
        Key::DOWN_ARROW,
        Key::UP_ARROW,
        Key::PAGE_UP,
        Key::PAGE_DOWN,
        Key::HOME,
        Key::END,
        [GLFW_KEY_CAPS_LOCK] = Key::CAPS_LOCK,
        Key::NONE /* SCROLL_LOCK */,
        Key::NONE /* NUM_LOCK */,
        [GLFW_KEY_PRINT_SCREEN] = Key::NONE /* PRINT_SCREEN */,
        Key::NONE /* PAUSE */,
        [GLFW_KEY_F1] = Key::F1,
        Key::F2,
        Key::F3,
        Key::F4,
        Key::F5,
        Key::F6,
        Key::F7,
        Key::F8,
        Key::F9,
        Key::F10,
        Key::F11,
        Key::F12,
        Key::F13,
        Key::F14,
        Key::F15,
        [GLFW_KEY_KP_1] = Key::N1,
        Key::N2,
        Key::N3,
        Key::N4,
        Key::N5,
        Key::N6,
        Key::N7,
        Key::N8,
        Key::N9,
        [GLFW_KEY_KP_DECIMAL] = Key::N_PERIOD,
        Key::N_DIVIDE,
        Key::N_TIMES,
        Key::N_MINUS,
        Key::N_PLUS,
        Key::N_ENTER,
        Key::N_EQUALS,
        [GLFW_KEY_LEFT_SHIFT] = Key::L_SHIFT,
        Key::L_CONTROL,
        Key::L_OPTION,
        Key::L_COMMAND,
        [GLFW_KEY_RIGHT_SHIFT] = Key::R_SHIFT,
        Key::R_CONTROL,
        Key::R_OPTION,
        Key::R_COMMAND,
        [GLFW_KEY_MENU] = Key::NONE /* MENU */,
};

static void throw_error(int code, const char* message) {
    throw std::runtime_error(pn::format("{0}: {1}", code, message).c_str());
}

GLFWVideoDriver::GLFWVideoDriver() : _screen_size(640, 480), _last_click_count(0) {
    if (!glfwInit()) {
        throw std::runtime_error("glfwInit()");
    }
    glfwSetErrorCallback(throw_error);
}

GLFWVideoDriver::~GLFWVideoDriver() { glfwTerminate(); }

Point GLFWVideoDriver::get_mouse() {
    double x, y;
    glfwGetCursorPos(_window, &x, &y);
    return {int(x), int(y)};
}

InputMode GLFWVideoDriver::input_mode() const { return KEYBOARD_MOUSE; }

wall_time GLFWVideoDriver::now() const { return wall_time(usecs(int64_t(glfwGetTime() * 1e6))); }

void GLFWVideoDriver::key(int key, int scancode, int action, int mods) {
    if (key < 0) {
        return;
    }
    Key usb_key = kGLFWKeyToUSB[key];
    if (usb_key == Key::NONE) {
        return;
    }
    if (action == GLFW_PRESS) {
        KeyDownEvent(now(), usb_key).send(_loop->top());
    } else if (action == GLFW_RELEASE) {
        KeyUpEvent(now(), usb_key).send(_loop->top());
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
        throw std::runtime_error("glfwCreateWindow");
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
