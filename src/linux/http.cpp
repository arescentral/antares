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

#include "net/http.hpp"

#include <neon/ne_basic.h>
#include <neon/ne_session.h>
#include <neon/ne_uri.h>
#include <unistd.h>
#include <sfz/sfz.hpp>

using sfz::BytesSlice;
using sfz::CString;
using sfz::Exception;
using sfz::StringSlice;
using sfz::WriteTarget;
using sfz::format;
using std::unique_ptr;

namespace antares {
namespace http {

struct ne_userdata {
    WriteTarget out;
    size_t      total;
};

static int accept(void* userdata, ne_request* req, const ne_status* st) {
    ne_userdata* u   = reinterpret_cast<ne_userdata*>(userdata);
    auto         len = ne_get_response_header(req, "Content-Length");
    (void)u;
    (void)len;
    return true;
}

static int reader(void* userdata, const char* buf, size_t len) {
    ne_userdata* u = reinterpret_cast<ne_userdata*>(userdata);
    u->total += len;
    write(u->out, buf, len);
    return 0;
}

void get(const StringSlice& url, WriteTarget out) {
    static int inited = ne_sock_init();
    if (inited != 0) {
        throw Exception("ne_sock_init()");
    }

    CString cstr(url);
    ne_uri  uri = {};
    if (ne_uri_parse(cstr.data(), &uri)) {
        throw Exception("ne_uri_parse()");
    }
    if (uri.port == 0) {
        uri.port = ne_uri_defaultport(uri.scheme);
    }
    unique_ptr<ne_uri, decltype(&ne_uri_free)>            uri_free(&uri, ne_uri_free);
    unique_ptr<ne_session, decltype(&ne_session_destroy)> sess(
            ne_session_create(uri.scheme, uri.host, uri.port), ne_session_destroy);

    unique_ptr<ne_request, decltype(&ne_request_destroy)> req(
            ne_request_create(sess.get(), "GET", uri.path), ne_request_destroy);

    ne_userdata userdata = {out};
    ne_add_response_body_reader(req.get(), accept, reader, &userdata);

    auto err = ne_request_dispatch(req.get());
    if (err != NE_OK) {
        throw Exception("ne_request_dispatch()");
    }
    auto* st = ne_get_status(req.get());
    if (st->code != 200) {
        throw Exception(st->code);
    }
}

}  // namespace http
}  // namespace antares
