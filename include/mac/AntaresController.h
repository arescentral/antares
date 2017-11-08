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

#ifndef ANTARES_MAC_ANTARES_CONTROLLER_H_
#define ANTARES_MAC_ANTARES_CONTROLLER_H_

#include <Cocoa/Cocoa.h>

@interface AntaresController : NSObject {
    struct AntaresDrivers* drivers;

    NSURL* _download_url;
    NSURL* _author_url;

    IBOutlet NSWindow* _window;
    IBOutlet NSButton* _no_show_again_checkbox;

    IBOutlet NSPopUpButton* _scenario_list;
    IBOutlet NSButton* _scenario_button;
    IBOutlet NSButton* _author_button;
    IBOutlet NSTextField* _version_label;

    IBOutlet NSButton* _skip_checkbox;
}

- (IBAction)openScenarioURL:(id)sender;
- (IBAction)openAuthorURL:(id)sender;
- (IBAction)setSkipSettingsFrom:(id)sender;

- (IBAction)settingsDone:(id)sender;

@end

#endif  // ANTARES_MAC_ANTARES_CONTROLLER_H_
