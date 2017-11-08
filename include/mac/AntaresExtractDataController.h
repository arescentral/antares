// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2017 The Antares Authors
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
// License along with Antares.  If not, see http://www.gnu.org/licenses/

#ifndef ANTARES_MAC_ANTARES_EXTRACT_DATA_CONTROLLER_H_
#define ANTARES_MAC_ANTARES_EXTRACT_DATA_CONTROLLER_H_

#include <Cocoa/Cocoa.h>

@interface AntaresExtractDataController : NSObject {
    id        _target;
    SEL       _selector;
    NSString* _path;
    NSString* _scenario;

    NSWindow*            _window;
    NSProgressIndicator* _progress_bar;
    NSTextField*         _status_field;
}

@property(strong) IBOutlet NSWindow* _window;
@property(strong) IBOutlet NSProgressIndicator* _progress_bar;
@property(strong) IBOutlet NSTextField* _status_field;

- (id)initWithTarget:(id)target selector:(SEL)selector path:(NSString*)path;
- (id)initWithTarget:(id)target selector:(SEL)selector scenario:(NSString*)scenario;

@end

#endif  // ANTARES_MAC_ANTARES_EXTRACT_DATA_CONTROLLER_H_
