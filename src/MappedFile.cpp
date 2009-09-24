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

#include "MappedFile.hpp"

#include "fcntl.h"
#include "stdio.h"
#include "sys/mman.h"
#include "sys/stat.h"
#include "PosixException.hpp"

AutoClosedFd::~AutoClosedFd() {
    Close();
}

void AutoClosedFd::Open(const std::string& path, int oflag, mode_t mode) {
    Close();
    _fd = open(path.c_str(), oflag, mode);
    if (!IsValid()) {
        perror(path.c_str());
        throw PosixException();
    }
}

void AutoClosedFd::Close() {
    if (IsValid()) {
        close(_fd);
    }
}

MappedFile::MappedFile(const std::string& path)
        : _path(path),
          _size(0),
          _data(NULL) {
    _file.Open(path, O_RDONLY);

    struct stat st;
    if (fstat(_file.fd(), &st) < 0) {
        perror("fstat");
        throw PosixException();
    }
    _size = st.st_size;

    _data = reinterpret_cast<char*>(mmap(NULL, _size, PROT_READ, MAP_PRIVATE, _file.fd(), 0));
    if (_data == MAP_FAILED) {
        perror("mmap");
        throw PosixException();
    }
}

MappedFile::~MappedFile() {
    munmap(const_cast<char*>(_data), _size);
}
