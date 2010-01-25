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

#ifndef ANTARES_FILE_HPP_
#define ANTARES_FILE_HPP_

#include <sys/stat.h>
#include "sfz/String.hpp"

namespace antares {

bool IsDir(const sfz::StringPiece& path);
void Mkdir(const sfz::StringPiece& path, mode_t mode);
void MakeDirs(const sfz::StringPiece& path, mode_t mode);
sfz::StringPiece BaseName(const sfz::StringPiece& path);
sfz::StringPiece DirName(const sfz::StringPiece& path);

int open_path(const sfz::StringPiece& path, int oflag, mode_t mode = 0644);

}  // namespace antares

#endif  // ANTARES_FILE_HPP_
