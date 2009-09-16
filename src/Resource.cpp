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

#include "Resource.hpp"

#include "fcntl.h"
#include "glob.h"
#include "stdio.h"
#include "sys/mman.h"
#include "sys/stat.h"

void* const kMmapFailed = reinterpret_cast<void*>(-1);

AutoClosedFile::~AutoClosedFile() {
    Close();
}

bool AutoClosedFile::Open(const std::string& filename, int oflag, mode_t mode) {
    Close();
    _fd = open(filename.c_str(), oflag, mode);
    return IsValid();
}

void AutoClosedFile::Close() {
    if (IsValid()) {
        close(_fd);
    }
}

Resource::Resource(uint32_t code, int id)
        : _size(0),
          _data(NULL) {
    char fileglob[64];
    char code_chars[5] = {
        code >> 24,
        code >> 16,
        code >> 8,
        code,
        '\0',
    };
    glob_t g;
    g.gl_offs = 0;

    sprintf(fileglob, "data/original/rsrc/%s/%d.%s", code_chars, id, code_chars);
    glob(fileglob, GLOB_DOOFFS, NULL, &g);

    sprintf(fileglob, "data/original/rsrc/%s/%d *.%s", code_chars, id, code_chars);
    glob(fileglob, GLOB_DOOFFS | GLOB_APPEND, NULL, &g);

    if (g.gl_pathc != 1) {
        perror(fileglob);
        throw NoSuchResourceException();
    }

    std::string filename = g.gl_pathv[0];
    globfree(&g);

    if (!_file.Open(filename, O_RDONLY)) {
        perror(filename.c_str());
        throw NoSuchResourceException();
    }

    struct stat st;
    if (fstat(_file.fd(), &st) < 0) {
        perror("fstat");
        throw NoSuchResourceException();
    }
    _size = st.st_size;

    _data = reinterpret_cast<char*>(mmap(NULL, _size, PROT_READ, MAP_PRIVATE, _file.fd(), 0));
    if (_data == kMmapFailed) {
        perror("mmap");
        throw NoSuchResourceException();
    }
}

Resource::~Resource() {
    if (_data != NULL && _data != kMmapFailed) {
        munmap(_data, _size);
    }
}
