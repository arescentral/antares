// Ares, a tactical space combat game.
// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <sys/socket.h>
#include <exception>

#include "BinaryStream.hpp"
#include "Casts.hpp"
#include "ColorTable.hpp"
#include "FakeDrawing.hpp"
#include "MappedFile.hpp"
#include "PosixException.hpp"
#include "Threading.hpp"
#include "VncServer.hpp"

namespace antares {

extern scoped_ptr<Window> fakeWindow;

namespace {

class SocketBinaryReader : public BinaryReader {
  public:
    SocketBinaryReader(int fd)
        : _fd(fd) { }

  protected:
    virtual void read_bytes(char* bytes, size_t len) {
        while (_buffer.size() < len) {
            char more[1024];
            ssize_t recd = recv(_fd, more, 1024, 0);
            if (recd <= 0) {
                throw PosixException();
            }
            _buffer.append(more, recd);
        }
        memcpy(bytes, _buffer.c_str(), len);
        _buffer = _buffer.substr(len);
    }

  private:
    int _fd;
    std::string _buffer;
};

class SocketBinaryWriter : public BinaryWriter {
  public:
    SocketBinaryWriter(int fd)
        : _fd(fd) { }

    void flush() {
        if (!_buffer.empty()) {
            size_t sent = send(_fd, _buffer.c_str(), _buffer.size(), 0);
            if (sent != _buffer.size()) {
                throw PosixException();
            }
            _buffer.clear();
        }
    }

  protected:
    virtual void write_bytes(const char* bytes, size_t len) {
        _buffer.append(bytes, len);
    }

  private:
    int _fd;
    std::string _buffer;
};

int64_t usecs() {
    timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000ll + tv.tv_usec;
}

int listen_on(int port) {
    int one = 1;
    int sock;
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        throw PosixException();
    }
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        throw PosixException();
    }
    if (listen(sock, 5) < 0) {
        throw PosixException();
    }

    return sock;
}

int accept_on(int sock) {
    int one = 1;
    sockaddr addr;
    socklen_t addrlen;
    int fd = accept(sock, &addr, &addrlen);
    if (fd < 0) {
        throw PosixException();
    }

    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
    return fd;
}

// Common Messages.

struct PixelFormat {
    uint8_t bits_per_pixel;
    uint8_t depth;
    uint8_t big_endian;
    uint8_t true_color;
    uint16_t red_max;
    uint16_t green_max;
    uint16_t blue_max;
    uint8_t red_shift;
    uint8_t green_shift;
    uint8_t blue_shift;
    uint8_t unused[3];

    void read(BinaryReader* bin) {
        bin->read(&bits_per_pixel);
        bin->read(&depth);
        bin->read(&big_endian);
        bin->read(&true_color);
        bin->read(&red_max);
        bin->read(&green_max);
        bin->read(&blue_max);
        bin->read(&red_shift);
        bin->read(&green_shift);
        bin->read(&blue_shift);
        bin->discard(3);
    }

    void write(BinaryWriter* bin) const {
        bin->write(bits_per_pixel);
        bin->write(depth);
        bin->write(big_endian);
        bin->write(true_color);
        bin->write(red_max);
        bin->write(green_max);
        bin->write(blue_max);
        bin->write(red_shift);
        bin->write(green_shift);
        bin->write(blue_shift);
        bin->pad(3);
    }
};

// 6.1. Handshaking Messages.

struct ProtocolVersion {
    char version[12];

    void read(BinaryReader* bin) {
        bin->read(version, 12);
    }

    void write(BinaryWriter* bin) const {
        bin->write(version, 12);
    }
};

struct SecurityMessage {
    uint8_t number_of_security_types;

    void read(BinaryReader* bin) {
        bin->read(&number_of_security_types);
    }

    void write(BinaryWriter* bin) const {
        bin->write(number_of_security_types);
    }
};

struct SecurityResultMessage {
    uint32_t status;

    void read(BinaryReader* bin) {
        bin->read(&status);
    }

    void write(BinaryWriter* bin) const {
        bin->write(status);
    }
};

// 6.3. Initialization Messages.

struct ClientInitMessage {
    uint8_t shared_flag;

    void read(BinaryReader* bin) {
        bin->read(&shared_flag);
    }

    void write(BinaryWriter* bin) const {
        bin->write(shared_flag);
    }
};

struct ServerInitMessage {
    uint16_t width;
    uint16_t height;
    PixelFormat format;
    uint32_t name_length;

    void read(BinaryReader* bin) {
        bin->read(&width);
        bin->read(&height);
        bin->read(&format);
        bin->read(&name_length);
    }

    void write(BinaryWriter* bin) const {
        bin->write(width);
        bin->write(height);
        bin->write(format);
        bin->write(name_length);
    }
};

// 6.4. Client-to-Server Messages.

enum ClientToServerMessageType {
    SET_PIXEL_FORMAT = 0,
    SET_ENCODINGS = 2,
    FRAMEBUFFER_UPDATE_REQUEST = 3,
    KEY_EVENT = 4,
    POINTER_EVENT = 5,
    CLIENT_CUT_TEXT = 6,
};

struct SetPixelFormatMessage {
    // uint8_t message_type;
    uint8_t unused[3];
    PixelFormat format;

    void read(BinaryReader* bin) {
        // bin->read(&message_type);
        bin->discard(3);
        bin->read(&format);
    }

    void write(BinaryWriter* bin) const {
        // bin->write(message_type);
        bin->pad(3);
        bin->write(format);
    }
};

struct SetEncodingsMessage {
    // uint8_t message_type;
    uint8_t unused;
    uint16_t number_of_encodings;

    void read(BinaryReader* bin) {
        // bin->read(&message_type);
        bin->discard(1);
        bin->read(&number_of_encodings);
    }

    void write(BinaryWriter* bin) const {
        // bin->write(message_type);
        bin->pad(1);
        bin->write(number_of_encodings);
    }
};

struct FramebufferUpdateRequestMessage {
    // uint8_t message_type;
    uint8_t incremental;
    uint16_t x;
    uint16_t y;
    uint16_t w;
    uint16_t h;

    void read(BinaryReader* bin) {
        // bin->read(&message_type);
        bin->read(&incremental);
        bin->read(&x);
        bin->read(&y);
        bin->read(&w);
        bin->read(&h);
    }

    void write(BinaryWriter* bin) const {
        // bin->write(message_type);
        bin->write(incremental);
        bin->write(x);
        bin->write(y);
        bin->write(w);
        bin->write(h);
    }
};

struct KeyEventMessage {
    // uint8_t message_type;
    uint8_t down_flag;
    uint8_t unused[2];
    uint32_t key;

    void read(BinaryReader* bin) {
        // bin->read(&message_type);
        bin->read(&down_flag);
        bin->discard(2);
        bin->read(&key);
    }

    void write(BinaryWriter* bin) const {
        // bin->write(message_type);
        bin->write(down_flag);
        bin->pad(2);
        bin->write(key);
    }
};

struct PointerEventMessage {
    // uint8_t message_type;
    uint8_t button_mask;
    uint16_t x_position;
    uint16_t y_position;

    void read(BinaryReader* bin) {
        // bin->read(&message_type);
        bin->read(&button_mask);
        bin->read(&x_position);
        bin->read(&y_position);
    }

    void write(BinaryWriter* bin) const {
        // bin->write(message_type);
        bin->write(button_mask);
        bin->write(x_position);
        bin->write(y_position);
    }
};

struct ClientCutTextMessage {
    // uint8_t message_type;
    uint8_t unused[3];
    uint32_t length;

    void read(BinaryReader* bin) {
        // bin->read(&message_type);
        bin->discard(3);
        bin->read(&length);
    }

    void write(BinaryWriter* bin) const {
        // bin->write(message_type);
        bin->pad(3);
        bin->write(length);
    }
};

// 6.5. Server-to-Client Messages.

enum ServerToClientMessageType {
    FRAMEBUFFER_UPDATE = 0,
    SET_COLOR_MAP_ENTRIES = 1,
    BELL = 2,
    SERVER_CUT_TEXT = 3,
};

struct FramebufferUpdateMessage {
    // uint8_t message_type;
    uint8_t padding;
    uint16_t number_of_rectangles;

    void read(BinaryReader* bin) {
        // bin->read(&message_type);
        bin->discard(1);
        bin->read(&number_of_rectangles);
    }

    void write(BinaryWriter* bin) const {
        // bin->write(message_type);
        bin->pad(1);
        bin->write(number_of_rectangles);
    }
};

struct FramebufferUpdateRectangle {
    uint16_t x_position;
    uint16_t y_position;
    uint16_t width;
    uint16_t height;
    int32_t encoding_type;

    void read(BinaryReader* bin) {
        bin->read(&x_position);
        bin->read(&y_position);
        bin->read(&width);
        bin->read(&height);
        bin->read(&encoding_type);
    }

    void write(BinaryWriter* bin) const {
        bin->write(x_position);
        bin->write(y_position);
        bin->write(width);
        bin->write(height);
        bin->write(encoding_type);
    }
};

struct SetColorMapEntriesMessage {
    // uint8_t message_type;
    uint8_t padding;
    uint16_t first_color;
    uint16_t number_of_colors;

    void read(BinaryReader* bin) {
        // bin->read(&message_type);
        bin->discard(1);
        bin->read(&first_color);
        bin->read(&number_of_colors);
    }

    void write(BinaryWriter* bin) const {
        // bin->write(message_type);
        bin->pad(1);
        bin->write(first_color);
        bin->write(number_of_colors);
    }
};

struct SetColorMapEntriesColor {
    uint16_t red;
    uint16_t green;
    uint16_t blue;

    void read(BinaryReader* bin) {
        bin->read(&red);
        bin->read(&green);
        bin->read(&blue);
    }

    void write(BinaryWriter* bin) const {
        bin->write(red);
        bin->write(green);
        bin->write(blue);
    }
};

struct ServerCutTextMessage {
    // uint8_t message_type;
    uint8_t unused[3];
    uint32_t length;

    void read(BinaryReader* bin) {
        // bin->read(&message_type);
        bin->discard(3);
        bin->read(&length);
    }

    void write(BinaryWriter* bin) const {
        // bin->write(message_type);
        bin->pad(3);
        bin->write(length);
    }
};

// 6.6 Encodings.

enum {
    RAW = 0,
};

struct FramebufferPixel {
    uint8_t unused;
    uint8_t red;
    uint8_t green;
    uint8_t blue;

    void read(BinaryReader* bin) {
        bin->read(&red);
        bin->read(&green);
        bin->read(&blue);
        bin->discard(1);
    }

    void write(BinaryWriter* bin) const {
        bin->pad(1);
        bin->write(red);
        bin->write(green);
        bin->write(blue);
    }
};

}  // namespace

bool VncVideoDriver::vnc_poll(int64_t timeout) {
    int width = fakeWindow->portRect.right;
    int height = fakeWindow->portRect.bottom;
    SocketBinaryReader in(_socket.fd());
    SocketBinaryWriter out(_socket.fd());
    int64_t stop_time = usecs() + timeout;
    bool more = false;

    do {
        scoped_ptr<EventRecord> evt(new EventRecord);

        more = false;
        timeout = std::max(0ll, stop_time - usecs());
        timeval tv;
        tv.tv_sec = timeout / 1000000ll;
        tv.tv_usec = timeout % 1000000ll;

        fd_set read;
        fd_set write;
        fd_set error;
        FD_ZERO(&read);
        FD_ZERO(&write);
        FD_ZERO(&error);
        FD_SET(_socket.fd(), &read);

        if (select(_socket.fd() + 1, &read, &write, &error, &tv) > 0) {
            uint8_t client_message_type;
            in.read(&client_message_type);
            switch (client_message_type) {
            case SET_PIXEL_FORMAT:
                {
                    SetPixelFormatMessage msg;
                    in.read(&msg);
                    more = true;
                }
                break;

            case SET_ENCODINGS:
                {
                    SetEncodingsMessage msg;
                    in.read(&msg);
                    for (int i = 0; i < msg.number_of_encodings; ++i) {
                        int32_t encoding_type;
                        in.read(&encoding_type);
                    }
                    more = true;
                }
                break;

            case FRAMEBUFFER_UPDATE_REQUEST:
                {
                    FramebufferUpdateRequestMessage request;
                    in.read(&request);

                        FramebufferUpdateMessage response;
                        uint8_t server_message_type = FRAMEBUFFER_UPDATE;
                        response.number_of_rectangles = 1;

                        FramebufferUpdateRectangle rect;
                        rect.x_position = 0;
                        rect.y_position = 0;
                        rect.width = width;
                        rect.height = height;
                        rect.encoding_type = RAW;

                        const ColorTable& table = *fakeWindow->portBits.colors;
                        scoped_array<FramebufferPixel> pixels(new FramebufferPixel[width * height]);
                        for (int i = 0; i < width * height; ++i) {
                            uint8_t color = fakeWindow->portBits.baseAddr[i];
                            pixels.get()[i].red = table.color(color).red >> 8;
                            pixels.get()[i].green = table.color(color).green >> 8;
                            pixels.get()[i].blue = table.color(color).blue >> 8;
                        }

                        out.write(server_message_type);
                        out.write(response);
                        out.write(rect);
                        out.write(pixels.get(), width * height);
                }
                break;

            case KEY_EVENT:
                {
                    KeyEventMessage msg;
                    in.read(&msg);
                    printf("key %d\n", msg.key);

                    if (_key_map.find(msg.key) == _key_map.end()) {
                        printf("unknown key\n");
                        exit(1);
                    } else {
                        evt->what = autoKey;
                        evt->message = _key_map[msg.key] << 8;
                        printf("enqueued %d\n", evt->message);
                        _event_queue.push(evt.release());
                    }

                    more = true;
                }
                break;

            case POINTER_EVENT:
                {
                    PointerEventMessage msg;
                    in.read(&msg);

                    if (_button != implicit_cast<bool>(msg.button_mask & 0x1)) {
                        if (_button) {
                            evt->what = mouseUp;
                        } else {
                            evt->what = mouseDown;
                        }
                        evt->where.h = msg.x_position;
                        evt->where.v = msg.y_position;
                        printf("enqueueing %d\n", evt->what);
                        _event_queue.push(evt.release());
                    }

                    _button = msg.button_mask & 0x1;
                    _mouse.h = msg.x_position;
                    _mouse.v = msg.y_position;
                    more = true;
                }
                break;

            case CLIENT_CUT_TEXT:
                {
                    ClientCutTextMessage msg;
                    in.read(&msg);
                    for (size_t i = 0; i < msg.length; ++i) {
                        char c;
                        in.read(&c, 1);
                    }
                }
                break;

            default:
                {
                    fprintf(stderr, "Received %d\n", implicit_cast<int>(client_message_type));
                    exit(1);
                }
            }
        }
        out.flush();
    } while (_event_queue.empty() && (more || usecs() < stop_time));

    return false;
}

VncVideoDriver::VncVideoDriver(int port)
        : _start_time(usecs()),
          _listen(listen_on(port)),
          _socket(accept_on(_listen.fd())),
          _button(false) {
    _mouse.h = 0;
    _mouse.v = 0;

    SocketBinaryReader in(_socket.fd());
    SocketBinaryWriter out(_socket.fd());
    int width = fakeWindow->portRect.right;
    int height = fakeWindow->portRect.bottom;

    {
        // Negotiate version of RFB protocol.  Only 3.8 is offered or accepted.
        ProtocolVersion version;
        strncpy(version.version, "RFB 003.008\n", sizeof(ProtocolVersion));
        out.write(version);
        out.flush();
        in.read(&version);
        if (strncmp(version.version, "RFB 003.008\n", sizeof(ProtocolVersion)) != 0) {
            fprintf(stderr, "Unacceptable client version %11s\n", version.version);
            exit(1);
        }
    }

    {
        // Negotiate security.  No security is provided.
        SecurityMessage security;
        security.number_of_security_types = 1;
        uint8_t security_types[1] = { '\1' };  // None.
        out.write(security);
        out.write(security_types, security.number_of_security_types);
        out.flush();

        uint8_t selected_security;
        in.read(&selected_security);
        if (selected_security != '\1') {
            fprintf(stderr, "Unacceptable security %d\n", implicit_cast<int>(selected_security));
            exit(1);
        }

        SecurityResultMessage result;
        result.status = 0;  // OK.
        out.write(result);
        out.flush();
    }

    {
        // Initialize connection.
        ClientInitMessage client_init;
        in.read(&client_init);

        const char* const name = "Antares";

        ServerInitMessage server_init;
        server_init.width = width;
        server_init.height = height;
        server_init.format.bits_per_pixel = 32;
        server_init.format.depth = 24;
        server_init.format.big_endian = 1;
        server_init.format.true_color = 1;
        server_init.format.red_max = 255;
        server_init.format.green_max = 255;
        server_init.format.blue_max = 255;
        server_init.format.red_shift = 8;
        server_init.format.green_shift = 16;
        server_init.format.blue_shift = 24;
        server_init.name_length = strlen(name);

        out.write(server_init);
        out.write(name, strlen(name));
        out.flush();
    }
    vnc_poll(0);

    _key_map['a'] = 0x00;
    _key_map['b'] = 0x0B;
    _key_map['c'] = 0x08;
    _key_map['d'] = 0x02;
    _key_map['e'] = 0x0E;
    _key_map['f'] = 0x03;
    _key_map['g'] = 0x05;
    _key_map['h'] = 0x04;
    _key_map['i'] = 0x22;
    _key_map['j'] = 0x26;
    _key_map['k'] = 0x28;
    _key_map['l'] = 0x25;
    _key_map['m'] = 0x2E;
    _key_map['n'] = 0x2D;
    _key_map['o'] = 0x1F;
    _key_map['p'] = 0x23;
    _key_map['q'] = 0x0C;
    _key_map['r'] = 0x0F;
    _key_map['s'] = 0x01;
    _key_map['t'] = 0x11;
    _key_map['u'] = 0x20;
    _key_map['v'] = 0x09;
    _key_map['w'] = 0x0D;
    _key_map['x'] = 0x07;
    _key_map['y'] = 0x10;
    _key_map['z'] = 0x06;
    _key_map[' '] = 0x31;

    _key_map[65507] = 0x36;  // CAPS
    _key_map[65289] = 0x30;  // TAB
    _key_map[65307] = 0x35;  // ESC
    _key_map[65293] = 0x24;  // RET

    _key_map[65361] = 0x3B;  // LEFT
    _key_map[65362] = 0x3E;  // UP
    _key_map[65363] = 0x3C;  // RGHT
    _key_map[65364] = 0x3D;  // DOWN
}

void VncVideoDriver::send_event(EventRecord) { }

bool VncVideoDriver::wait_next_event(EventRecord* evt, double sleep) {
    vnc_poll(sleep * 1000000ll);
    if (_event_queue.empty()) {
        return false;
    } else {
        *evt = *_event_queue.front();
        delete _event_queue.front();
        _event_queue.pop();
        return true;
    }
}

bool VncVideoDriver::button() {
    vnc_poll(0);
    return _button;
}

Point VncVideoDriver::get_mouse() {
    vnc_poll(0);
    return _mouse;
}

void VncVideoDriver::get_keys(KeyMap keys) {
    vnc_poll(0);
    bzero(keys, sizeof(KeyMap));
}

void VncVideoDriver::set_game_state(GameState) { }

int VncVideoDriver::get_demo_scenario() {
    int levels[] = { 0, 5, 23 };
    return levels[rand() % 3];
}

void VncVideoDriver::main_loop_iteration_complete(uint32_t) { }

int VncVideoDriver::ticks() {
    return (usecs() - _start_time) * 60 / 1000000;
}

void VncVideoDriver::loop() {
    while (!_listeners.empty()) {
        EventRecord evt;
        if (wait_next_event(&evt, _listeners.next_delay())) {
            _listeners.send(evt);
        } else {
            _listeners.fire_next_timer();
        }
    }
}

void VncVideoDriver::push_listener(EventListener* listener) {
    _listeners.push(listener);
}

void VncVideoDriver::pop_listener(EventListener* listener) {
    _listeners.pop(listener);
}

}  // namespace antares
