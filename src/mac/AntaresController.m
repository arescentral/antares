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

- (NSURL*)authorURL;
- (void)setAuthorURL:(NSURL*)authorURL;
- (NSURL*)downloadURL;
- (void)setDownloadURL:(NSURL*)downloadURL;
@end

@implementation AntaresController
- (void)application:(NSApplication*)app openFile:(NSString*)filename {
    [_window orderOut:self];
}

- (void)applicationDidFinishLaunching:(NSNotification*)aNotification {
    bool skip =
            [[NSUserDefaults standardUserDefaults] boolForKey:kSkipSettings] &&
            !([NSEvent modifierFlags] & NSDeviceIndependentModifierFlagsMask & NSAlternateKeyMask);
    if (skip) {
        [self settingsDone:self];
    } else {
        [_window center];
        [_window makeKeyAndOrderFront:self];
    }
}

- (void)setScenarioFrom:(NSMenuItem*)sender {
    NSString* identifier   = [[sender representedObject] objectForKey:kIdentifier];
    NSString* title        = [[sender representedObject] objectForKey:kTitle];
    NSURL*    download_url = [[sender representedObject] objectForKey:kDownloadURL];
    NSString* author       = [[sender representedObject] objectForKey:kAuthor];
    NSURL*    author_url   = [[sender representedObject] objectForKey:kAuthorURL];
    NSString* version      = [[sender representedObject] objectForKey:kVersion];

    [_scenario_button setHidden:YES];
    [_scenario_label setHidden:YES];
    if (download_url) {
        [_scenario_button setTitle:title];
        [self setDownloadURL:download_url];
        [_scenario_button setHidden:NO];
    } else {
        [_scenario_label setStringValue:title];
        [_scenario_label setHidden:NO];
    }

    [_author_button setHidden:YES];
    [_author_label setHidden:YES];
    if (author_url) {
        [_author_button setTitle:author];
        [self setAuthorURL:author_url];
        [_author_button setHidden:NO];
    } else {
        [_author_label setStringValue:author];
        [_author_label setHidden:NO];
    }

    [_version_label setStringValue:version];

    [[NSUserDefaults standardUserDefaults] setObject:identifier forKey:kScenario];
}

- (void)awakeFromNib {
    bool skip_settings = [[NSUserDefaults standardUserDefaults] boolForKey:kSkipSettings];
    [_skip_checkbox setIntValue:skip_settings];
    _application_should_terminate = NSTerminateNow;
}

- (IBAction)openScenarioURL:(id)sender {
    [[NSWorkspace sharedWorkspace] openURL:_download_url];
}

- (IBAction)openAuthorURL:(id)sender {
    [[NSWorkspace sharedWorkspace] openURL:_author_url];
}

- (IBAction)setSkipSettingsFrom:(id)sender {
    bool skip_settings = [sender intValue];
    [[NSUserDefaults standardUserDefaults] setBool:skip_settings forKey:kSkipSettings];
}

- (IBAction)settingsDone:(id)sender {
    [_window close];

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

- (void)fail:(NSString*)message {
    NSAlert* alert = [[NSAlert alloc] init];
    [alert setMessageText:@"Oops"];
    [alert setInformativeText:message];
    [alert addButtonWithTitle:@"Quit"];
    [alert runModal];
    exit(1);
}

- (void)installDone:(id)sender {
    [_window center];
    [_window makeKeyAndOrderFront:self];
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

- (void)dealloc {
    [_download_url release];
    [_author_url release];
    [super dealloc];
}

- (NSURL*)authorURL {
    @synchronized(self) {
        return [[_author_url retain] autorelease];
    }
}

- (void)setAuthorURL:(NSURL*)authorURL {
    @synchronized(self) {
        [authorURL retain];
        [_author_url release];
        _author_url = authorURL;
    }
}

- (NSURL*)downloadURL {
    @synchronized(self) {
        return [[_download_url retain] autorelease];
    }
}

- (void)setDownloadURL:(NSURL*)downloadURL {
    @synchronized(self) {
        [downloadURL retain];
        [_download_url release];
        _download_url = downloadURL;
    }
}
@end
