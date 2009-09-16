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

#ifndef ANTARES_RESOURCE_HPP_
#define ANTARES_RESOURCE_HPP_

#include <stdint.h>
#include <string>

class NoSuchResourceException : public std::exception { };

class AutoClosedFile {
  public:
    AutoClosedFile() : _fd(-1) { }
    AutoClosedFile(int fd) : _fd(fd) { }
    ~AutoClosedFile();

    bool IsValid() const { return _fd >= 0; }

    int fd() const { return _fd; }

    bool Open(const std::string& filename, int oflag, mode_t mode = 0600);

  private:
    void Close();

    int _fd;
};

class Resource {
  public:
    Resource(uint32_t code, int id);
    ~Resource();

    size_t size() const { return _size; }
    char* data() const { return _data; }

  private:
    AutoClosedFile _file;
    size_t _size;
    char* _data;
};

#endif // ANTARES_RESOURCE_HPP_
