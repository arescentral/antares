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

#ifndef ANTARES_STRING_LIST_HPP_
#define ANTARES_STRING_LIST_HPP_

#include <string>
#include <vector>

class StringList {
  public:
    void load(int id);
    ssize_t index_of(std::string& result) const;
    size_t size() const;
    const std::string& at(size_t index) const;

  private:
    std::vector<std::string> _strings;
};

void string_to_pstring(const std::string& src, unsigned char* dest);

#endif // ANTARES_STRING_LIST_HPP_
