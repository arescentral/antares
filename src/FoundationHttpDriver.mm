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

#include "FoundationHttpDriver.hpp"

#include <Cocoa/Cocoa.h>
#include <sfz/sfz.hpp>
#include "CocoaAdditions.hpp"

using antares::Scoped;
using sfz::Bytes;
using sfz::BytesPiece;
using sfz::Exception;
using sfz::StringPiece;
using sfz::WriteTarget;
using sfz::format;

namespace utf8 = sfz::utf8;

@interface NSData (SfzAdditions)
- (BytesPiece) bytesPiece;
@end

@implementation NSData (SfzAdditions)
- (BytesPiece) bytesPiece {
    return BytesPiece(reinterpret_cast<const uint8_t*>([self bytes]), [self length]);
}
@end

namespace antares {

void FoundationHttpDriver::get(const StringPiece& url, WriteTarget out) {
    Scoped<NSAutoreleasePool> pool([[NSAutoreleasePool alloc] init]);
    NSURL* nsurl = [NSURL URLWithString:[new_nsstring_from(url) autorelease]];
    NSData* data = [NSData dataWithContentsOfURL:nsurl];
    if (data) {
        out.append([data bytesPiece]);
    } else {
        throw Exception(format("Couldn't load requested url {0}", url));
    }
}

}  // namespace antares
