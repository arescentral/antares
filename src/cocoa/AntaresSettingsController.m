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

#include "cocoa/AntaresSettingsController.h"

@implementation AntaresSettingsController

- (id)initWithTarget:(id)target selector:(SEL)selector {
    if (!(self = [super init])) {
        return NULL;
    }
    _target = target;
    _selector = selector;
    if (![NSBundle loadNibNamed:@"Settings" owner:self]) {
        [self release];
        return nil;
    }
    return [self retain];
}

- (void)awakeFromNib {
    [_author_button setTarget:self];
    [_window center];
    [_window makeKeyAndOrderFront:self];
}

- (IBAction)openScenarioURL:(id)sender {
    [[NSWorkspace sharedWorkspace] openURL: [NSURL URLWithString:@"http://www.arescentral.com"]];
}

- (IBAction)openAuthorURL:(id)sender {
    [[NSWorkspace sharedWorkspace] openURL: [NSURL URLWithString:@"http://www.biggerplanet.com"]];
}

- (IBAction)startGame:(id)sender {
    [_window close];
    [_target performSelector:_selector withObject:self];
    [self release];
}

@end
