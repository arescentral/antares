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

#include <pn/output>
#include <sfz/sfz.hpp>

#include "config/preferences.hpp"

using sfz::range;

namespace antares {

static const ticks kDoubleClickInterval = ticks(30);

namespace {

struct GLFWKeyMap {
    Key map[GLFW_KEY_LAST + 1];
};

}  // namespace

static GLFWKeyMap glfw_key_to_usb() {
    GLFWKeyMap keys = {};

    keys.map[GLFW_KEY_SPACE]      = Key::SPACE;
    keys.map[GLFW_KEY_APOSTROPHE] = Key::QUOTE;
    keys.map[GLFW_KEY_COMMA]      = Key::COMMA;
    keys.map[GLFW_KEY_MINUS]      = Key::MINUS;
    keys.map[GLFW_KEY_PERIOD]     = Key::PERIOD;
    keys.map[GLFW_KEY_SLASH]      = Key::SLASH;

    keys.map[GLFW_KEY_0] = Key::K0;
    keys.map[GLFW_KEY_1] = Key::K1;
    keys.map[GLFW_KEY_2] = Key::K2;
    keys.map[GLFW_KEY_3] = Key::K3;
    keys.map[GLFW_KEY_4] = Key::K4;
    keys.map[GLFW_KEY_5] = Key::K5;
    keys.map[GLFW_KEY_6] = Key::K6;
    keys.map[GLFW_KEY_7] = Key::K7;
    keys.map[GLFW_KEY_8] = Key::K8;
    keys.map[GLFW_KEY_9] = Key::K9;

    keys.map[GLFW_KEY_SEMICOLON] = Key::SEMICOLON;
    keys.map[GLFW_KEY_EQUAL]     = Key::EQUALS;

    keys.map[GLFW_KEY_A] = Key::A;
    keys.map[GLFW_KEY_B] = Key::B;
    keys.map[GLFW_KEY_C] = Key::C;
    keys.map[GLFW_KEY_D] = Key::D;
    keys.map[GLFW_KEY_E] = Key::E;
    keys.map[GLFW_KEY_F] = Key::F;
    keys.map[GLFW_KEY_G] = Key::G;
    keys.map[GLFW_KEY_H] = Key::H;
    keys.map[GLFW_KEY_I] = Key::I;
    keys.map[GLFW_KEY_J] = Key::J;
    keys.map[GLFW_KEY_K] = Key::K;
    keys.map[GLFW_KEY_L] = Key::L;
    keys.map[GLFW_KEY_M] = Key::M;
    keys.map[GLFW_KEY_N] = Key::N;
    keys.map[GLFW_KEY_O] = Key::O;
    keys.map[GLFW_KEY_P] = Key::P;
    keys.map[GLFW_KEY_Q] = Key::Q;
    keys.map[GLFW_KEY_R] = Key::R;
    keys.map[GLFW_KEY_S] = Key::S;
    keys.map[GLFW_KEY_T] = Key::T;
    keys.map[GLFW_KEY_U] = Key::U;
    keys.map[GLFW_KEY_V] = Key::V;
    keys.map[GLFW_KEY_W] = Key::W;
    keys.map[GLFW_KEY_X] = Key::X;
    keys.map[GLFW_KEY_Y] = Key::Y;
    keys.map[GLFW_KEY_Z] = Key::Z;

    keys.map[GLFW_KEY_LEFT_BRACKET]  = Key::L_BRACKET;
    keys.map[GLFW_KEY_BACKSLASH]     = Key::BACKSLASH;
    keys.map[GLFW_KEY_RIGHT_BRACKET] = Key::R_BRACKET;
    keys.map[GLFW_KEY_GRAVE_ACCENT]  = Key::BACKTICK;
    keys.map[GLFW_KEY_WORLD_1]       = Key::NONE;
    keys.map[GLFW_KEY_WORLD_2]       = Key::NONE;
    keys.map[GLFW_KEY_ESCAPE]        = Key::ESCAPE;
    keys.map[GLFW_KEY_ENTER]         = Key::RETURN;
    keys.map[GLFW_KEY_TAB]           = Key::TAB;
    keys.map[GLFW_KEY_BACKSPACE]     = Key::BACKSPACE;
    keys.map[GLFW_KEY_INSERT]        = Key::NONE;
    keys.map[GLFW_KEY_DELETE]        = Key::DEL;
    keys.map[GLFW_KEY_RIGHT]         = Key::RIGHT_ARROW;
    keys.map[GLFW_KEY_LEFT]          = Key::LEFT_ARROW;
    keys.map[GLFW_KEY_DOWN]          = Key::DOWN_ARROW;
    keys.map[GLFW_KEY_UP]            = Key::UP_ARROW;
    keys.map[GLFW_KEY_PAGE_UP]       = Key::PAGE_UP;
    keys.map[GLFW_KEY_PAGE_DOWN]     = Key::PAGE_DOWN;
    keys.map[GLFW_KEY_HOME]          = Key::HOME;
    keys.map[GLFW_KEY_END]           = Key::END;
    keys.map[GLFW_KEY_CAPS_LOCK]     = Key::CAPS_LOCK;
    keys.map[GLFW_KEY_SCROLL_LOCK]   = Key::NONE;
    keys.map[GLFW_KEY_NUM_LOCK]      = Key::NONE;
    keys.map[GLFW_KEY_PRINT_SCREEN]  = Key::NONE;
    keys.map[GLFW_KEY_PAUSE]         = Key::NONE;

    keys.map[GLFW_KEY_F1]  = Key::F1;
    keys.map[GLFW_KEY_F2]  = Key::F2;
    keys.map[GLFW_KEY_F3]  = Key::F3;
    keys.map[GLFW_KEY_F4]  = Key::F4;
    keys.map[GLFW_KEY_F5]  = Key::F5;
    keys.map[GLFW_KEY_F6]  = Key::F6;
    keys.map[GLFW_KEY_F7]  = Key::F7;
    keys.map[GLFW_KEY_F8]  = Key::F8;
    keys.map[GLFW_KEY_F9]  = Key::F9;
    keys.map[GLFW_KEY_F10] = Key::F10;
    keys.map[GLFW_KEY_F11] = Key::F11;
    keys.map[GLFW_KEY_F12] = Key::F12;
    keys.map[GLFW_KEY_F13] = Key::F13;
    keys.map[GLFW_KEY_F14] = Key::F14;
    keys.map[GLFW_KEY_F15] = Key::F15;

    keys.map[GLFW_KEY_KP_1] = Key::N1;
    keys.map[GLFW_KEY_KP_2] = Key::N2;
    keys.map[GLFW_KEY_KP_3] = Key::N3;
    keys.map[GLFW_KEY_KP_4] = Key::N4;
    keys.map[GLFW_KEY_KP_5] = Key::N5;
    keys.map[GLFW_KEY_KP_6] = Key::N6;
    keys.map[GLFW_KEY_KP_7] = Key::N7;
    keys.map[GLFW_KEY_KP_8] = Key::N8;
    keys.map[GLFW_KEY_KP_9] = Key::N9;

    keys.map[GLFW_KEY_KP_DECIMAL]  = Key::N_PERIOD;
    keys.map[GLFW_KEY_KP_DIVIDE]   = Key::N_DIVIDE;
    keys.map[GLFW_KEY_KP_MULTIPLY] = Key::N_TIMES;
    keys.map[GLFW_KEY_KP_SUBTRACT] = Key::N_MINUS;
    keys.map[GLFW_KEY_KP_ADD]      = Key::N_PLUS;
    keys.map[GLFW_KEY_KP_ENTER]    = Key::N_ENTER;
    keys.map[GLFW_KEY_KP_EQUAL]    = Key::N_EQUALS;

    keys.map[GLFW_KEY_LEFT_SHIFT]    = Key::SHIFT;
    keys.map[GLFW_KEY_LEFT_CONTROL]  = Key::CONTROL;
    keys.map[GLFW_KEY_LEFT_ALT]      = Key::OPTION;
    keys.map[GLFW_KEY_LEFT_SUPER]    = Key::COMMAND;
    keys.map[GLFW_KEY_RIGHT_SHIFT]   = Key::R_SHIFT;
    keys.map[GLFW_KEY_RIGHT_CONTROL] = Key::R_CONTROL;
    keys.map[GLFW_KEY_RIGHT_ALT]     = Key::R_OPTION;
    keys.map[GLFW_KEY_RIGHT_SUPER]   = Key::R_COMMAND;
    keys.map[GLFW_KEY_MENU]          = Key::NONE;

    return keys;
}

static GLFWKeyMap kGLFWKeyToUSB = glfw_key_to_usb();

static void throw_error(int code, const char* message) {
    throw std::runtime_error(pn::format("{0}: {1}", code, message).c_str());
}

GLFWVideoDriver::GLFWVideoDriver() : _screen_size(640, 480), _last_click_count(0), _text(nullptr) {
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

bool GLFWVideoDriver::start_editing(TextReceiver* text) {
    _text = text;
    return true;
}

void GLFWVideoDriver::stop_editing(TextReceiver* text) {
    if (_text == text) {
        _text = nullptr;
    }
}

wall_time GLFWVideoDriver::now() const { return wall_time(usecs(int64_t(glfwGetTime() * 1e6))); }

void GLFWVideoDriver::key(int key, int scancode, int action, int mods) {
    if (_text) {
        edit(key, action, mods);
        return;
    }

    if (key < 0) {
        return;
    }
    Key usb_key = kGLFWKeyToUSB.map[key];
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

void GLFWVideoDriver::char_(unsigned int code_point) {
    if (!_text) {
        return;
    }
    _text->replace(_text->selection(), pn::rune{code_point});
}

static void backspace(TextReceiver* text, int mods) {
    auto s = text->selection();
    if (s.begin != s.end) {
        // delete selection
    } else if (mods & GLFW_MOD_SUPER) {
        s.begin = text->offset(s.begin, TextReceiver::THIS_START, TextReceiver::LINES);
    } else if (mods & (GLFW_MOD_CONTROL | GLFW_MOD_ALT)) {
        s.begin = text->offset(s.begin, TextReceiver::PREV_START, TextReceiver::WORDS);
    } else {
        s.begin = text->offset(s.begin, TextReceiver::PREV_START, TextReceiver::GLYPHS);
    }
    text->replace(s, "");
}

static void fwd_delete(TextReceiver* text, int mods) {
    auto s = text->selection();
    if (s.begin != s.end) {
        // delete selection
    } else if (mods & GLFW_MOD_SUPER) {
        s.end = text->offset(s.end, TextReceiver::THIS_END, TextReceiver::LINES);
    } else if (mods & (GLFW_MOD_CONTROL | GLFW_MOD_ALT)) {
        s.end = text->offset(s.end, TextReceiver::NEXT_END, TextReceiver::WORDS);
    } else {
        s.end = text->offset(s.end, TextReceiver::NEXT_END, TextReceiver::GLYPHS);
    }
    text->replace(s, "");
}

static void left(TextReceiver* text, int mods) {
    auto s = text->selection();
    if ((s.begin != s.end) && !(mods & GLFW_MOD_SHIFT)) {
        // exit selection
    } else if (mods & GLFW_MOD_SUPER) {
        s.begin = text->offset(s.begin, TextReceiver::THIS_START, TextReceiver::LINES);
    } else if (mods & (GLFW_MOD_CONTROL | GLFW_MOD_ALT)) {
        s.begin = text->offset(s.begin, TextReceiver::PREV_START, TextReceiver::WORDS);
    } else {
        s.begin = text->offset(s.begin, TextReceiver::PREV_START, TextReceiver::GLYPHS);
    }
    if (!(mods & GLFW_MOD_SHIFT)) {
        s.end = s.begin;
    }
    text->select(s);
}

static void right(TextReceiver* text, int mods) {
    auto s = text->selection();
    if ((s.begin != s.end) && !(mods & GLFW_MOD_SHIFT)) {
        // exit selection
    } else if (mods & GLFW_MOD_SUPER) {
        s.end = text->offset(s.end, TextReceiver::THIS_END, TextReceiver::LINES);
    } else if (mods & (GLFW_MOD_CONTROL | GLFW_MOD_ALT)) {
        s.end = text->offset(s.end, TextReceiver::NEXT_END, TextReceiver::WORDS);
    } else {
        s.end = text->offset(s.end, TextReceiver::NEXT_END, TextReceiver::GLYPHS);
    }
    if (!(mods & GLFW_MOD_SHIFT)) {
        s.begin = s.end;
    }
    text->select(s);
}

static void up(TextReceiver* text, int mods) {
    auto s = text->selection();
    if ((s.begin != s.end) && !(mods & GLFW_MOD_SHIFT)) {
        // exit selection
    } else {
        s.begin = text->offset(s.begin, TextReceiver::PREV_SAME, TextReceiver::LINES);
    }
    if (!(mods & GLFW_MOD_SHIFT)) {
        s.end = s.begin;
    }
    text->select(s);
}

static void down(TextReceiver* text, int mods) {
    auto s = text->selection();
    if ((s.begin != s.end) && !(mods & GLFW_MOD_SHIFT)) {
        // exit selection
    } else {
        s.end = text->offset(s.end, TextReceiver::NEXT_SAME, TextReceiver::LINES);
    }
    if (!(mods & GLFW_MOD_SHIFT)) {
        s.begin = s.end;
    }
    text->select(s);
}

void GLFWVideoDriver::edit(int key, int action, int mods) {
    if (action == GLFW_RELEASE) {
        return;
    }

    switch (key) {
        case GLFW_KEY_ESCAPE:
            if (action != GLFW_REPEAT) {
                _text->escape();
            }
            break;

        case GLFW_KEY_ENTER:
            if (action == GLFW_REPEAT) {
                return;
            } else if (mods & GLFW_MOD_SHIFT) {
                _text->newline();
            } else {
                _text->accept();
            }
            break;

        case GLFW_KEY_BACKSPACE: backspace(_text, mods); break;
        case GLFW_KEY_DELETE: fwd_delete(_text, mods); break;

        case GLFW_KEY_LEFT: left(_text, mods); break;
        case GLFW_KEY_RIGHT: right(_text, mods); break;
        case GLFW_KEY_UP: up(_text, mods); break;
        case GLFW_KEY_DOWN: down(_text, mods); break;
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

void GLFWVideoDriver::char_callback(GLFWwindow* w, unsigned int code_point) {
    GLFWVideoDriver* driver = reinterpret_cast<GLFWVideoDriver*>(glfwGetWindowUserPointer(w));
    driver->char_(code_point);
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
    glfwSetCharCallback(_window, char_callback);
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
