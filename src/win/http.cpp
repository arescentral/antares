// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2018 The Antares Authors
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

#include <windows.h>
#include <wininet.h>

#include <exception>
#include <memory>
#include <pn/output>
#include <pn/string>

#include "build/defs.hpp"

namespace antares {
namespace http {

namespace {

struct url_components {
    // http://user:pass@host.com:port/fullpath?and=query_string
    pn::string_view scheme;
    pn::string_view username;
    pn::string_view password;
    pn::string_view hostname;
    int             port;
    pn::string_view fullpath;
};

url_components parse_url(pn::string_view url) {
    URL_COMPONENTSA u  = {};
    u.dwStructSize     = sizeof(u);
    u.dwSchemeLength   = 1;
    u.dwHostNameLength = 1;
    u.dwUserNameLength = 1;
    u.dwPasswordLength = 1;
    u.dwUrlPathLength  = 1;
    if (!InternetCrackUrlA(url.data(), url.size(), 0, &u)) {
        throw std::runtime_error(pn::format("{}: url parse error", url).c_str());
    }

    url_components out;
    out.scheme   = pn::string_view(u.lpszScheme, u.dwSchemeLength);
    out.username = pn::string_view(u.lpszUserName, u.dwUserNameLength);
    out.password = pn::string_view(u.lpszPassword, u.dwPasswordLength);
    out.hostname = pn::string_view(u.lpszHostName, u.dwHostNameLength);
    out.port     = u.nPort;
    out.fullpath = pn::string_view(u.lpszUrlPath, (url.data() + url.size()) - u.lpszUrlPath);
    return out;
}

}  // namespace

void get(pn::string_view url, pn::output_view out) {
    auto u = parse_url(url);

    std::unique_ptr<void, decltype(&InternetCloseHandle)> session{
            InternetOpenA(
                    pn::format("arescentral.org/antares (win {})", kAntaresVersion).c_str(),
                    INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0),
            InternetCloseHandle};
    if (!session) {
        throw std::runtime_error(pn::format("{}: error opening internet session", url).c_str());
    }

    std::unique_ptr<void, decltype(&InternetCloseHandle)> connect{
            InternetConnectA(
                    session.get(), u.hostname.copy().c_str(), u.port,
                    u.username.empty() ? nullptr : u.username.copy().c_str(),
                    u.password.empty() ? nullptr : u.password.copy().c_str(),
                    INTERNET_SERVICE_HTTP, 0, 0),
            InternetCloseHandle};
    if (!connect) {
        throw std::runtime_error(pn::format("{}: error connecting to internet", url).c_str());
    }

    std::unique_ptr<void, decltype(&InternetCloseHandle)> request{
            HttpOpenRequestA(
                    connect.get(), "GET", u.fullpath.copy().c_str(), nullptr, nullptr, nullptr, 0,
                    0),
            InternetCloseHandle};
    if (!request) {
        throw std::runtime_error(pn::format("{}: error opening request", url).c_str());
    } else if (!HttpSendRequestA(request.get(), nullptr, 0, nullptr, 0)) {
        throw std::runtime_error(pn::format("{}: error sending request", url).c_str());
    }

    uint8_t buf[1024];
    while (true) {
        DWORD bytes_read;
        if (!InternetReadFile(request.get(), buf, 1024, &bytes_read)) {
            throw std::runtime_error(pn::format("{}: error reading data", url).c_str());
        } else if (bytes_read == 0) {
            break;
        }
        out.write(pn::data_view(buf, bytes_read));
    }
}

}  // namespace http
}  // namespace antares
