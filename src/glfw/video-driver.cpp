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

#include <game/sys.hpp>
#include <pn/output>
#include <sfz/sfz.hpp>

#include "config/preferences.hpp"

using sfz::range;

namespace antares {

static const ticks kDoubleClickInterval = ticks(30);

static Key glfw_key_to_usb(int key) {
    switch (key) {
        case GLFW_KEY_SPACE: return Key::SPACE;
        case GLFW_KEY_APOSTROPHE: return Key::QUOTE;
        case GLFW_KEY_COMMA: return Key::COMMA;
        case GLFW_KEY_MINUS: return Key::MINUS;
        case GLFW_KEY_PERIOD: return Key::PERIOD;
        case GLFW_KEY_SLASH: return Key::SLASH;

        case GLFW_KEY_0: return Key::K0;
        case GLFW_KEY_1: return Key::K1;
        case GLFW_KEY_2: return Key::K2;
        case GLFW_KEY_3: return Key::K3;
        case GLFW_KEY_4: return Key::K4;
        case GLFW_KEY_5: return Key::K5;
        case GLFW_KEY_6: return Key::K6;
        case GLFW_KEY_7: return Key::K7;
        case GLFW_KEY_8: return Key::K8;
        case GLFW_KEY_9: return Key::K9;

        case GLFW_KEY_SEMICOLON: return Key::SEMICOLON;
        case GLFW_KEY_EQUAL: return Key::EQUALS;

        case GLFW_KEY_A: return Key::A;
        case GLFW_KEY_B: return Key::B;
        case GLFW_KEY_C: return Key::C;
        case GLFW_KEY_D: return Key::D;
        case GLFW_KEY_E: return Key::E;
        case GLFW_KEY_F: return Key::F;
        case GLFW_KEY_G: return Key::G;
        case GLFW_KEY_H: return Key::H;
        case GLFW_KEY_I: return Key::I;
        case GLFW_KEY_J: return Key::J;
        case GLFW_KEY_K: return Key::K;
        case GLFW_KEY_L: return Key::L;
        case GLFW_KEY_M: return Key::M;
        case GLFW_KEY_N: return Key::N;
        case GLFW_KEY_O: return Key::O;
        case GLFW_KEY_P: return Key::P;
        case GLFW_KEY_Q: return Key::Q;
        case GLFW_KEY_R: return Key::R;
        case GLFW_KEY_S: return Key::S;
        case GLFW_KEY_T: return Key::T;
        case GLFW_KEY_U: return Key::U;
        case GLFW_KEY_V: return Key::V;
        case GLFW_KEY_W: return Key::W;
        case GLFW_KEY_X: return Key::X;
        case GLFW_KEY_Y: return Key::Y;
        case GLFW_KEY_Z: return Key::Z;

        case GLFW_KEY_LEFT_BRACKET: return Key::L_BRACKET;
        case GLFW_KEY_BACKSLASH: return Key::BACKSLASH;
        case GLFW_KEY_RIGHT_BRACKET: return Key::R_BRACKET;
        case GLFW_KEY_GRAVE_ACCENT: return Key::BACKTICK;
        case GLFW_KEY_WORLD_1: return Key::NONE;
        case GLFW_KEY_WORLD_2: return Key::NONE;
        case GLFW_KEY_ESCAPE: return Key::ESCAPE;
        case GLFW_KEY_ENTER: return Key::RETURN;
        case GLFW_KEY_TAB: return Key::TAB;
        case GLFW_KEY_BACKSPACE: return Key::BACKSPACE;
        case GLFW_KEY_INSERT: return Key::NONE;
        case GLFW_KEY_DELETE: return Key::DEL;
        case GLFW_KEY_RIGHT: return Key::RIGHT_ARROW;
        case GLFW_KEY_LEFT: return Key::LEFT_ARROW;
        case GLFW_KEY_DOWN: return Key::DOWN_ARROW;
        case GLFW_KEY_UP: return Key::UP_ARROW;
        case GLFW_KEY_PAGE_UP: return Key::PAGE_UP;
        case GLFW_KEY_PAGE_DOWN: return Key::PAGE_DOWN;
        case GLFW_KEY_HOME: return Key::HOME;
        case GLFW_KEY_END: return Key::END;
        case GLFW_KEY_CAPS_LOCK: return Key::CAPS_LOCK;
        case GLFW_KEY_SCROLL_LOCK: return Key::NONE;
        case GLFW_KEY_NUM_LOCK: return Key::NONE;
        case GLFW_KEY_PRINT_SCREEN: return Key::NONE;
        case GLFW_KEY_PAUSE: return Key::NONE;

        case GLFW_KEY_F1: return Key::F1;
        case GLFW_KEY_F2: return Key::F2;
        case GLFW_KEY_F3: return Key::F3;
        case GLFW_KEY_F4: return Key::F4;
        case GLFW_KEY_F5: return Key::F5;
        case GLFW_KEY_F6: return Key::F6;
        case GLFW_KEY_F7: return Key::F7;
        case GLFW_KEY_F8: return Key::F8;
        case GLFW_KEY_F9: return Key::F9;
        case GLFW_KEY_F10: return Key::F10;
        case GLFW_KEY_F11: return Key::F11;
        case GLFW_KEY_F12: return Key::F12;
        case GLFW_KEY_F13: return Key::F13;
        case GLFW_KEY_F14: return Key::F14;
        case GLFW_KEY_F15: return Key::F15;

        case GLFW_KEY_KP_1: return Key::N1;
        case GLFW_KEY_KP_2: return Key::N2;
        case GLFW_KEY_KP_3: return Key::N3;
        case GLFW_KEY_KP_4: return Key::N4;
        case GLFW_KEY_KP_5: return Key::N5;
        case GLFW_KEY_KP_6: return Key::N6;
        case GLFW_KEY_KP_7: return Key::N7;
        case GLFW_KEY_KP_8: return Key::N8;
        case GLFW_KEY_KP_9: return Key::N9;

        case GLFW_KEY_KP_DECIMAL: return Key::N_PERIOD;
        case GLFW_KEY_KP_DIVIDE: return Key::N_DIVIDE;
        case GLFW_KEY_KP_MULTIPLY: return Key::N_TIMES;
        case GLFW_KEY_KP_SUBTRACT: return Key::N_MINUS;
        case GLFW_KEY_KP_ADD: return Key::N_PLUS;
        case GLFW_KEY_KP_ENTER: return Key::N_ENTER;
        case GLFW_KEY_KP_EQUAL: return Key::N_EQUALS;

        case GLFW_KEY_LEFT_SHIFT: return Key::SHIFT;
        case GLFW_KEY_LEFT_CONTROL: return Key::CONTROL;
        case GLFW_KEY_LEFT_ALT: return Key::OPTION;
        case GLFW_KEY_LEFT_SUPER: return Key::COMMAND;
        case GLFW_KEY_RIGHT_SHIFT: return Key::R_SHIFT;
        case GLFW_KEY_RIGHT_CONTROL: return Key::R_CONTROL;
        case GLFW_KEY_RIGHT_ALT: return Key::R_OPTION;
        case GLFW_KEY_RIGHT_SUPER: return Key::R_COMMAND;
        case GLFW_KEY_MENU: return Key::NONE;

        default: return Key::NONE;
    }
}

static void throw_error(int code, const char* message) {
    throw std::runtime_error(pn::format("{0}: {1}", code, message).c_str());
}

GLFWVideoDriver::GLFWVideoDriver()
        : OpenGlVideoDriver("330 core"),
          _fullscreen(sys.prefs->fullscreen()),
          _screen_size(sys.prefs->window_size()),
          _last_click_count(0),
          _text(nullptr) {
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
#if GLFW_VERSION_MINOR >= 2
    if ((key == GLFW_KEY_ENTER) && (mods == GLFW_MOD_ALT)) {
        window_maximize(!_fullscreen);
        return;
    }
#endif

    if (_text) {
        edit(key, action, mods);
        return;
    }

    if (key < 0) {
        return;
    }
    Key usb_key = glfw_key_to_usb(key);
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
    if (!_fullscreen) {
        sys.prefs->set_window_size(_screen_size);
    }
    glfwGetFramebufferSize(_window, &_viewport_size.width, &_viewport_size.height);
}

void GLFWVideoDriver::window_maximize(bool maximized) {
#if GLFW_VERSION_MINOR >= 2
    if (maximized == _fullscreen) {
        return;
    }
    _fullscreen = maximized;
    sys.prefs->set_fullscreen(maximized);

    GLFWmonitor*       monitor = nullptr;
    const GLFWvidmode* mode;
    if (_fullscreen) {
        monitor      = glfwGetPrimaryMonitor();
        mode         = glfwGetVideoMode(monitor);
        _screen_size = {mode->width, mode->height};
    } else {
        mode         = glfwGetVideoMode(glfwGetWindowMonitor(_window));
        _screen_size = sys.prefs->window_size();
    }

    Rect screen_rect{Point{0, 0}, Size{mode->width, mode->height}};
    Rect window_rect{Point{0, 0}, _screen_size};
    window_rect.center_in(screen_rect);
    glfwSetWindowMonitor(
            _window, monitor, window_rect.left, window_rect.top, window_rect.width(),
            window_rect.height(), mode->refreshRate);
#endif
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

void GLFWVideoDriver::window_maximize_callback(GLFWwindow* w, int maximized) {
    GLFWVideoDriver* driver = reinterpret_cast<GLFWVideoDriver*>(glfwGetWindowUserPointer(w));
    driver->window_maximize(maximized);
}

void GLFWVideoDriver::loop(Card* initial) {
    /* Create a windowed mode window and its OpenGL context */
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    if (_fullscreen) {
        GLFWmonitor*       monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode    = glfwGetVideoMode(monitor);

        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

        _screen_size = {mode->width, mode->height};
        _window = glfwCreateWindow(_screen_size.width, _screen_size.height, "", monitor, NULL);
    } else {
        _window = glfwCreateWindow(_screen_size.width, _screen_size.height, "", NULL, NULL);
    }
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
#if GLFW_VERSION_MINOR >= 3
    glfwSetWindowMaximizeCallback(_window, window_maximize_callback);
#endif

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
