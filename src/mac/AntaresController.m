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

#include "mac/AntaresController.h"

#include <ApplicationServices/ApplicationServices.h>
#include <stdlib.h>
#include "mac/AntaresExtractDataController.h"
#include "mac/c/AntaresController.h"

#define kScreenWidth @"ScreenWidth"
#define kScreenHeight @"ScreenHeight"
#define kScenario @"Scenario"
#define kSkipSettings @"SkipSettings"

#define kIdentifier @"Identifier"
#define kTitle @"Title"
#define kDownloadURL @"DownloadURL"
#define kAuthor @"Author"
#define kAuthorURL @"AuthorURL"
#define kVersion @"Version"

#define kFactoryScenarioIdentifier @"4cab7415715aeeacf1486a352267ae82c0efb220"

@interface AntaresController (Private)
- (void)fail:(NSString*)message;
@end

@implementation AntaresController
- (void)application:(NSApplication*)app openFile:(NSString*)filename {
}

- (void)applicationDidFinishLaunching:(NSNotification*)aNotification {
    CFStringRef error_message;
    drivers = antares_controller_create_drivers(&error_message);
    if (!drivers) {
        [self fail:(NSString*)error_message];
    }

    AntaresExtractDataController* extract = [[[AntaresExtractDataController alloc]
            initWithTarget:self
                  selector:@selector(extractDone:)] autorelease];
    if (!extract) {
        [self fail:@"Failed to create AntaresExtractDataController"];
    }
}

- (void)awakeFromNib {
    _application_should_terminate = NSTerminateNow;
}

- (void)fail:(NSString*)message {
    NSAlert* alert = [[NSAlert alloc] init];
    [alert setMessageText:@"Oops"];
    [alert setInformativeText:message];
    [alert addButtonWithTitle:@"Quit"];
    [alert runModal];
    exit(1);
}

- (void)extractDone:(id)sender {
    if (![sender success]) {
        return;
    }
    _application_should_terminate = NSTerminateCancel;

    NSFileManager* files = [NSFileManager defaultManager];
    NSURL*         url   = [files URLForDirectory:NSApplicationSupportDirectory
                               inDomain:NSUserDomainMask
                      appropriateForURL:nil
                                 create:NO
                                  error:nil];

    NSString* scenario_identifier = [[NSUserDefaults standardUserDefaults] stringForKey:kScenario];
    NSString* scenario_path       = [[url.path stringByAppendingPathComponent:@"Antares/Scenarios"]
            stringByAppendingPathComponent:scenario_identifier];
    const char* path              = [scenario_path UTF8String];
    path                          = nil;

    CFStringRef error_message;
    if (!antares_controller_loop(drivers, path, &error_message)) {
        [self fail:(NSString*)error_message];
    }

    antares_controller_destroy_drivers(drivers);
    _application_should_terminate = NSTerminateNow;
    [NSApp terminate:self];
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender {
    return _application_should_terminate;
}
@end
