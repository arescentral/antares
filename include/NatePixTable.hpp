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

#ifndef ANTARES_NATE_PIX_TABLE_HPP_
#define ANTARES_NATE_PIX_TABLE_HPP_

// NatePixTable.h

//#include "NatePix.hpp"
#include <vector>
#include "SpriteHandling.hpp"

namespace antares {

class natePixEntryType;
class natePixType {
  public:
    natePixType();
    natePixType(const natePixType& other);
    natePixType& operator=(const natePixType& other);
    ~natePixType();

    natePixEntryType* at(size_t index) const;
    size_t size() const;

    size_t load_data(const char* data, size_t len);
    void copy_from(const natePixType& other);
    void clear();

  private:
    std::vector<natePixEntryType*> _entries;
};

TypedHandle<natePixType> CreateNatePixTable();
unsigned long GetNatePixTableSize(TypedHandle<natePixType>);
long GetNatePixTablePixNum(TypedHandle<natePixType>);
int GetNatePixTableNatePixWidth(TypedHandle<natePixType>, long);
int GetNatePixTableNatePixHeight(TypedHandle<natePixType>, long);
int GetNatePixTableNatePixHRef(TypedHandle<natePixType>, long);
int GetNatePixTableNatePixVRef(TypedHandle<natePixType>, long);
unsigned char *GetNatePixTableNatePixData(TypedHandle<natePixType>, long);
unsigned char GetNatePixTableNatePixDataPixel(TypedHandle<natePixType>, long, int, int);
Handle GetNatePixTableNatePixDataCopy(TypedHandle<natePixType>, long);
// void GetNatePixTableNatePixPtr( natePix *, Handle, int);
// void GetNatePixTableNatePixDuplicate( natePix *, Handle, int);
void RemapNatePixTableColor(TypedHandle<natePixType>);
void ColorizeNatePixTableColor(TypedHandle<natePixType>, unsigned char);

}  // namespace antares

#endif // ANTARES_NATE_PIX_TABLE_HPP_
