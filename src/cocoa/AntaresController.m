// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2011 Ares Central
//
// This file is part of Antares, a tactical space combat game.
//
// Antares is free software: you can redistribute it and/or modify it
// under the terms of the Lesser GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Antares is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program.  If not, see
// <http://www.gnu.org/licenses/>.

#include "cocoa/AntaresController.h"

#include <stdlib.h>

#include "cocoa/AntaresExtractDataController.h"
#include "cocoa/c/AntaresController.h"

@implementation AntaresController

- (void)applicationWillFinishLaunching:(NSNotification*)aNotification {
    CFStringRef error_message;
    if (!antares_controller_set_drivers(&error_message)) {
        NSLog(@"%@", error_message);
        CFRelease(error_message);
        exit(1);
    }

    AntaresExtractDataController* extract = [[[AntaresExtractDataController alloc]
        initWithTarget:self selector:@selector(run:)] autorelease];
    if (!extract) {
        NSLog(@"Failed to create AntaresExtractDataController");
        exit(1);
    }
}

- (void)run:(id)sender {
    CFStringRef error_message;
    if (!antares_controller_loop(&error_message)) {
        NSLog(@"%@", error_message);
        CFRelease(error_message);
        exit(1);
    }
    [NSApp terminate:self];
}

@end
