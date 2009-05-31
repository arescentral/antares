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

#include "FakeHandles.hpp"

#include <assert.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <exception>

#include "FakeSounds.hpp"
#include "Fakes.hpp"

static void* const kMmapFailed = (void*)-1;

class NoSuchResourceException : public std::exception { };

class AutoClosedFile {
  public:
    AutoClosedFile() : _fd(-1) { }
    AutoClosedFile(int fd) : _fd(fd) { }
    ~AutoClosedFile() { Close(); }

    bool IsValid() const { return _fd >= 0; }

    int fd() const { return _fd; }

    bool Open(const char* filename, int oflag, mode_t mode = 0600) {
        Close();
        _fd = open(filename, oflag, mode);
        return IsValid();
    }

  private:
    void Close() {
        if (IsValid()) {
            close(_fd);
        }
    }

    int _fd;
};

class ResourceData {
  public:
    ResourceData(FourCharCode code, int id)
            : _size(0),
              _data(NULL) {
        char filename[64];
        char code_chars[5] = {
            code >> 24,
            code >> 16,
            code >> 8,
            code,
            '\0',
        };
        sprintf(filename, "data/original/rsrc/%s/r.%d", code_chars, id);

        if (!_file.Open(filename, O_RDONLY)) {
            perror(filename);
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

    ~ResourceData() {
        if (_data != NULL && _data != kMmapFailed) {
            munmap(_data, _size);
        }
    }

    size_t size() const { return _size; }
    char* data() const { return _data; }

  private:
    AutoClosedFile _file;
    size_t _size;
    char* _data;
};

Handle GetResource(FourCharCode code, int id) {
    switch (code) {
      case 'PICT':
        return reinterpret_cast<Handle>(GetPicture(id));

      case 'snd ':
        return GetSound(id);

      default:
        try {
            ResourceData rsrc(code, id);
            return (new HandleData<void>(rsrc.size(), rsrc.data()))->ToHandle();
        } catch (NoSuchResourceException& e) {
            return NULL;
        }
    }
}

Handle NewHandle(size_t size) {
    return (new HandleData<void>(size))->ToHandle();
}

int GetHandleSize(Handle handle) {
    return HandleBase::FromHandle(handle)->Size();
}

void GetIndString(unsigned char* result, int id, int index) {
    if (index <= 0) {
        *result = '\0';
        return;
    }
    ResourceData rsrc('STR#', id);
    uint16_t count = *reinterpret_cast<uint16_t*>(rsrc.data());
    assert(index <= count);
    char* pstr = rsrc.data() + 2;
    uint8_t size = *pstr;
    while (index > 1) {
        pstr += size + 1;
        size = *pstr;
        --index;
    }
    memcpy(result, pstr, size + 1);
}

void BlockMove(void* src, void* dst, size_t size) {
    memcpy(dst, src, size);
}

OSErr PtrToHand(void* ptr, Handle* handle, int len) {
    *handle = (new HandleData<void>(len, ptr))->ToHandle();
    return noErr;
}

OSErr HandToHand(Handle* handle) {
    *handle = HandleBase::FromHandle(*handle)->Clone()->ToHandle();
    return noErr;
}

void FakeHandlesInit() {
}
