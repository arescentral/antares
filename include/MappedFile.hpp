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

#ifndef ANTARES_MAPPED_FILE_HPP_
#define ANTARES_MAPPED_FILE_HPP_

#include <stdint.h>
#include <string>

namespace antares {

class AutoClosedFd {
  public:
    AutoClosedFd() : _fd(-1) { }
    AutoClosedFd(int fd) : _fd(fd) { }
    ~AutoClosedFd();

    bool IsValid() const { return _fd >= 0; }

    int fd() const { return _fd; }

    void Open(const std::string& path, int oflag, mode_t mode = 0600);

  private:
    void Close();

    int _fd;
};

class MappedFile {
  public:
    explicit MappedFile(const std::string& path);
    ~MappedFile();

    const std::string& path() const { return _path; }
    size_t size() const { return _size; }
    const char* data() const { return _data; }

  private:
    AutoClosedFd _file;
    const std::string _path;
    size_t _size;
    const char* _data;
};

}  // namespace antares

#endif // ANTARES_MAPPED_FILE_HPP_
