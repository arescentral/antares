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

#include "sfz/Exception.hpp"

using sfz::Exception;

namespace antares {

std::string BaseName(const std::string& path) {
    if (path == "/") {
        return "/";
    }
    std::string::size_type pos = path.rfind('/', path.size() - 1);
    if (pos == std::string::npos) {
        return path;
    } else if (pos == path.size() - 1) {
        return BaseName(path.substr(0, path.size() - 1));
    } else {
        return path.substr(pos + 1);
    }
}

std::string DirName(const std::string& path) {
    std::string::size_type pos = path.rfind('/', path.size() - 1);
    if (pos == 0) {
        return "/";
    } else if (pos == std::string::npos) {
        return ".";
    } else if (pos == path.size() - 1) {
        return DirName(path.substr(0, path.size() - 1));
    } else {
        return path.substr(0, pos);
    }
}

bool IsDir(const std::string& path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0
        && (st.st_mode & S_IFDIR);
}

void Mkdir(const std::string& path, mode_t mode) {
    if (mkdir(path.c_str(), mode) != 0) {
        throw Exception("TODO(sfiera): posix error message");
    }
}

void MakeDirs(const std::string& path, mode_t mode) {
    if (!IsDir(path)) {
        MakeDirs(DirName(path), mode);
        Mkdir(path, mode);
    }
}

}  // namespace antares
