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

#include "FakeSounds.hpp"
#include "Fakes.hpp"
#include "Resource.hpp"

Handle GetResource(FourCharCode code, int id) {
    switch (code) {
      case 'NLRP':
      case 'TEXT':
      case 'intr':
      case 'nlFD':
      case 'nlFM':
        try {
            Resource rsrc(code, id);
            return (new HandleData<void>(rsrc.size(), rsrc.data()))->ToHandle();
        } catch (NoSuchResourceException& e) {
            return NULL;
        }
        break;

      case 'vers':
        return NULL;

      default:
        const char code_string[5] = {
            code >> 24, code >> 16, code >> 8, code, '\0',
        };
        fprintf(stderr, "GetResource() no longer handles code '%s'\n", code_string);
        exit(1);
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
    Resource rsrc('STR#', id);
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
