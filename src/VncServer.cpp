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
#include <sys/socket.h>
#include <exception>

#include "Casts.hpp"
#include "FakeDrawing.hpp"
#include "Resource.hpp"

extern CTabHandle fakeCTabHandle;
extern FakeWindow fakeWindow;

class VncServerException : public std::exception { };

int listen_on(int port) {
    int one = 1;
    int sock;
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        throw VncServerException();
    }
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&one), sizeof(one));

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        throw VncServerException();
    }
    if (listen(sock, 5) < 0) {
        throw VncServerException();
    }

    return sock;
}

int accept_on(int sock) {
    int one = 1;
    sockaddr addr;
    socklen_t addrlen;
    int fd = accept(sock, &addr, &addrlen);
    if (fd < 0) {
        throw VncServerException();
    }

    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&one), sizeof(one));
    return fd;
}

template <typename T>
void recv_from(int fd, T* t) {
    char* data = reinterpret_cast<char*>(t);
    int len = sizeof(T);
    while (len > 0) {
        int n = recv(fd, data, len, 0);
        if (n <= 0) {
            throw VncServerException();
        }
        data += n;
        len -= n;
    }
}

template <typename T>
void send_to(int fd, const T& t) {
    const char* data = reinterpret_cast<const char*>(&t);
    int len = sizeof(T);
    while (len > 0) {
        int n = send(fd, data, len, 0);
        if (n <= 0) {
            throw VncServerException();
        }
        data += n;
        len -= n;
    }
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
};

// 6.1. Handshaking Messages.

struct ProtocolVersion {
    char version[12];
};

struct SecurityMessage {
    uint8_t number_of_security_types;
};

struct SecurityResultMessage {
    uint32_t status;
};

// 6.3. Initialization Messages.

struct ClientInitMessage {
    uint8_t shared_flag;
};

struct ServerInitMessage {
    uint16_t width;
    uint16_t height;
    PixelFormat format;
    uint32_t name_length;
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
    uint8_t message_type;
    uint8_t unused[3];
    PixelFormat format;
};

struct SetEncodingsMessage {
    uint8_t message_type;
    uint8_t unused;
    uint16_t number_of_encodings;
};

struct FramebufferUpdateRequestMessage {
    uint8_t message_type;
    uint8_t incremental;
    uint16_t x;
    uint16_t y;
    uint16_t w;
    uint16_t h;
};

struct KeyEventMessage {
    uint8_t message_type;
    uint8_t down_flag;
    uint8_t unused[2];
    uint32_t key;
};

struct PointerEventMessage {
    uint8_t message_type;
    uint8_t button_mask;
    uint16_t x_position;
    uint16_t y_position;
};

struct ClientCutTextMessage {
    uint8_t message_type;
    uint8_t unused[3];
    uint32_t length;
};

// 6.5. Server-to-Client Messages.

enum ServerToClientMessageType {
    FRAMEBUFFER_UPDATE = 0,
    SET_COLOR_MAP_ENTRIES = 1,
    BELL = 2,
    SERVER_CUT_TEXT = 3,
};

struct FramebufferUpdateMessage {
    uint8_t message_type;
    uint8_t padding;
    uint16_t number_of_rectangles;
};

struct FramebufferUpdateRectangle {
    uint16_t x_position;
    uint16_t y_position;
    uint16_t width;
    uint16_t height;
    int32_t encoding_type;
};

struct SetColorMapEntriesMessage {
    uint8_t message_type;
    uint8_t padding;
    uint16_t first_color;
    uint16_t number_of_colors;
};

struct SetColorMapEntriesColor {
    uint16_t red;
    uint16_t green;
    uint16_t blue;
};

struct ServerCutTextMessage {
    uint8_t message_type;
    uint8_t unused[3];
    uint32_t length;
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
};

FramebufferPixel results[640 * 480];

void* vnc_server(void*) {
    AutoClosedFile sock(listen_on(5901));
    AutoClosedFile stream(accept_on(sock.fd()));

    {
        // Negotiate version of RFB protocol.  Only 3.8 is offered or accepted.
        ProtocolVersion version;
        strncpy(version.version, "RFB 003.008\n", sizeof(ProtocolVersion));
        send_to(stream.fd(), version);
        recv_from(stream.fd(), &version);
        if (strncmp(version.version, "RFB 003.008\n", sizeof(ProtocolVersion)) != 0) {
            throw VncServerException();
        }
    }

    {
        // Negotiate security.  No security is provided.
        SecurityMessage security;
        security.number_of_security_types = 1;
        uint8_t security_types[1] = { '\1' };  // None.
        send_to(stream.fd(), security);
        send_to(stream.fd(), security_types);

        uint8_t selected_security;
        recv_from(stream.fd(), &selected_security);
        if (selected_security != '\1') {
            throw VncServerException();
        }

        SecurityResultMessage result;
        result.status = 0;  // OK.
        send_to(stream.fd(), result);
    }

    {
        // Initialize connection.
        ClientInitMessage client_init;
        recv_from(stream.fd(), &client_init);  // Ignored.

        const char* const name = "Antares";

        ServerInitMessage server_init;
        server_init.width = 640;
        server_init.height = 480;
        server_init.format.bits_per_pixel = 32;
        server_init.format.depth = 24;
        server_init.format.big_endian = 1;
        server_init.format.true_color = 1;
        server_init.format.red_max = 255;
        server_init.format.green_max = 255;
        server_init.format.blue_max = 255;
        server_init.format.red_shift = 16;
        server_init.format.green_shift = 8;
        server_init.format.blue_shift = 0;
        server_init.name_length = strlen(name);

        send_to(stream.fd(), server_init);
        if (send(stream.fd(), name, strlen(name), 0) != implicit_cast<ssize_t>(strlen(name))) {
            throw VncServerException();
        }
    }

    while (true) {
        uint8_t client_message_type;
        recv(stream.fd(), &client_message_type, 1, MSG_PEEK);
        switch (client_message_type) {
        case SET_PIXEL_FORMAT:
            {
                SetPixelFormatMessage msg;
                recv_from(stream.fd(), &msg);
            }
            break;
        case SET_ENCODINGS:
            {
                SetEncodingsMessage msg;
                recv_from(stream.fd(), &msg);
                for (int i = 0; i < msg.number_of_encodings; ++i) {
                    int32_t encoding_type;
                    recv_from(stream.fd(), &encoding_type);
                }
            }
            break;
        case FRAMEBUFFER_UPDATE_REQUEST:
            {
                FramebufferUpdateRequestMessage request;
                recv_from(stream.fd(), &request);

                FramebufferUpdateMessage response;
                response.message_type = FRAMEBUFFER_UPDATE;
                response.number_of_rectangles = 1;

                FramebufferUpdateRectangle rect;
                rect.x_position = 0;
                rect.y_position = 0;
                rect.width = 640;
                rect.height = 480;
                rect.encoding_type = RAW;

                ColorSpec* colors = (*fakeCTabHandle)->ctTable;
                for (int i = 0; i < 640 * 480; ++i) {
                    uint8_t color = fakeWindow.portBits.baseAddr[i];
                    results[i].red = colors[color].rgb.red;
                    results[i].green = colors[color].rgb.green;
                    results[i].blue = colors[color].rgb.blue;
                }

                send_to(stream.fd(), response);
                send_to(stream.fd(), rect);
                send_to(stream.fd(), results);
            }
            break;
        case KEY_EVENT:
            {
                KeyEventMessage msg;
                recv_from(stream.fd(), &msg);
            }
            break;
        case POINTER_EVENT:
            {
                PointerEventMessage msg;
                recv_from(stream.fd(), &msg);
            }
            break;
        case CLIENT_CUT_TEXT:
            {
                ClientCutTextMessage msg;
                recv_from(stream.fd(), &msg);
                printf("%d bytes\n", msg.length);
                for (size_t i = 0; i < msg.length; ++i) {
                    char c;
                    recv_from(stream.fd(), &c);
                }
            }
            break;
        default:
            {
                printf("Received %d\n", implicit_cast<int>(client_message_type));
                exit(1);
            }
        }
    }
    return NULL;
}

void VncServerInit() {
    pthread_t thread;
    if (pthread_create(&thread, NULL, vnc_server, NULL) != 0) {
        throw VncServerException();
    }
}
