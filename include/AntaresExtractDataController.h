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

#ifndef ANTARES_EXTRACT_DATA_CONTROLLER_HPP_
#define ANTARES_EXTRACT_DATA_CONTROLLER_HPP_

#include <Cocoa/Cocoa.h>

@interface AntaresExtractDataController : NSObject {
    id _target;
    SEL _selector;
    IBOutlet NSWindow* _window;
    IBOutlet NSProgressIndicator* _progress_bar;
    IBOutlet NSTextField* _status_field;
}

- (id)initWithTarget:(id)target selector:(SEL)selector;

@end

#endif  // ANTARES_EXTRACT_DATA_CONTROLLER_HPP_
