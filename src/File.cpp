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

#include "File.hpp"

#include <fcntl.h>
#include "sfz/Exception.hpp"

using sfz::Bytes;
using sfz::Exception;
using sfz::String;
using sfz::StringPiece;
using sfz::ascii_encoding;
using sfz::utf8_encoding;

namespace antares {

namespace {

const static String kSlash("/", ascii_encoding());
const static String kDot(".", ascii_encoding());

}  // namespace

StringPiece BaseName(const StringPiece& path) {
    if (path == kSlash) {
        return path;
    }
    size_t pos = path.rfind('/', path.size() - 1);
    if (pos == StringPiece::kNone) {
        return path;
    } else if (pos == path.size() - 1) {
        return BaseName(path.substr(0, path.size() - 1));
    } else {
        return path.substr(pos + 1);
    }
}

StringPiece DirName(const StringPiece& path) {
    size_t pos = path.rfind('/', path.size() - 1);
    if (pos == 0) {
        return kSlash;
    } else if (pos == StringPiece::kNone) {
        return kDot;
    } else if (pos == path.size() - 1) {
        return DirName(path.substr(0, path.size() - 1));
    } else {
        return path.substr(0, pos);
    }
}

bool IsDir(const StringPiece& path) {
    Bytes path_bytes(path, utf8_encoding());
    path_bytes.resize(path_bytes.size() + 1);
    struct stat st;
    return stat(reinterpret_cast<const char*>(path_bytes.data()), &st) == 0
        && (st.st_mode & S_IFDIR);
}

void Mkdir(const StringPiece& path, mode_t mode) {
    Bytes path_bytes(path, utf8_encoding());
    path_bytes.resize(path_bytes.size() + 1);
    if (mkdir(reinterpret_cast<const char*>(path_bytes.data()), mode) != 0) {
        throw Exception("TODO(sfiera): posix error message");
    }
}

void MakeDirs(const StringPiece& path, mode_t mode) {
    if (!IsDir(path)) {
        MakeDirs(DirName(path), mode);
        Mkdir(path, mode);
    }
}

int open_path(const StringPiece& path, int oflag, mode_t mode) {
    Bytes path_bytes(path, utf8_encoding());
    path_bytes.resize(path_bytes.size() + 1);
    return open(reinterpret_cast<const char*>(path_bytes.data()), oflag, mode);
}

}  // namespace antares
