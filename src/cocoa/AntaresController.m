// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2012 The Antares Authors
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

#include "cocoa/AntaresController.h"

#include <stdlib.h>
#include <ApplicationServices/ApplicationServices.h>
#include "cocoa/AntaresExtractDataController.h"
#include "cocoa/c/AntaresController.h"
#include "cocoa/c/scenario-list.h"

#define kScreenWidth @"ScreenWidth"
#define kScreenHeight @"ScreenHeight"
#define kScenario @"Scenario"
#define kFullscreen @"Fullscreen"
#define kSkipSettings @"SkipSettings"

#define kIdentifier @"Identifier"
#define kTitle @"Title"
#define kDownloadURL @"DownloadURL"
#define kAuthor @"Author"
#define kAuthorURL @"AuthorURL"
#define kVersion @"Version"

#define kFactoryScenario @"com.biggerplanet.ares"

// A comparator for display mode dictionaries.  Orders the objects as
// the pair (screen height, screen width).  The case where this ordering
// differs from (screen width, screen height) is unlikely enough that
// it's not worth doing anything fancier.
static NSInteger compare_resolutions(id x, id y, void* unused) {
    (void)unused;
    int x_width = CGDisplayModeGetWidth((CGDisplayModeRef)x);
    int x_height = CGDisplayModeGetHeight((CGDisplayModeRef)x);
    int y_width = CGDisplayModeGetWidth((CGDisplayModeRef)y);
    int y_height = CGDisplayModeGetHeight((CGDisplayModeRef)y);

    if (x_height < y_height) {
        return NSOrderedAscending;
    } else if (x_height > y_height) {
        return NSOrderedDescending;
    } else if (x_width < y_width) {
        return NSOrderedAscending;
    } else if (x_width > y_width) {
        return NSOrderedDescending;
    } else {
        return NSOrderedSame;
    }
}

static NSInteger compare_scenarios(id x, id y, void* unused) {
    (void)unused;

    return [[x objectForKey:kTitle] caseInsensitiveCompare:[y objectForKey:kTitle]];
}

static NSString* str(const char* utf8_bytes) {
    return [NSString stringWithUTF8String:utf8_bytes];
}

static NSURL* url(const char* utf8_bytes) {
    return [NSURL URLWithString:str(utf8_bytes)];
}

@interface AntaresController (Private)
- (NSURL*)authorURL;
- (void)setAuthorURL:(NSURL*)authorURL;
- (NSURL*)downloadURL;
- (void)setDownloadURL:(NSURL*)downloadURL;
@end

@implementation AntaresController
- (void)application:(NSApplication*)app openFile:(NSString*)filename {
    [_window orderOut:self];
    AntaresExtractDataController* extract = [[[AntaresExtractDataController alloc]
        initWithTarget:self selector:@selector(installDone:) path:filename] autorelease];
    if (!extract) {
        NSLog(@"Failed to create AntaresExtractDataController");
        exit(1);
    }
}

- (void)applicationDidFinishLaunching:(NSNotification*)aNotification {
    bool skip =
        [[NSUserDefaults standardUserDefaults] boolForKey:kSkipSettings]
        && !([NSEvent modifierFlags]
                & NSDeviceIndependentModifierFlagsMask
                & NSAlternateKeyMask);
    if (skip) {
        [self settingsDone:self];
    } else {
        [_window center];
        [_window makeKeyAndOrderFront:self];
    }
}

- (void)setResolutionFrom:(NSMenuItem*)sender {
    // Assumes that the sender's representedObject is a display mode
    // dictionary, as set below.  Updates user defaults with the width
    // and height of the selected resolution.
    int width = CGDisplayModeGetWidth((CGDisplayModeRef)[sender representedObject]);
    int height = CGDisplayModeGetHeight((CGDisplayModeRef)[sender representedObject]);;
    [[NSUserDefaults standardUserDefaults] setInteger:width forKey:kScreenWidth];
    [[NSUserDefaults standardUserDefaults] setInteger:height forKey:kScreenHeight];
}

- (void)updateResolutionList {
    // When selecting a resolution, we'll pick the largest resolution we
    // see that has (width <= user_width) and (height <= user_height).
    // If the user has not yet selected a screen size, we default
    // user_width and user_height to INT_MAX.
    //
    // This scheme has three important properties:
    //
    // * If the user has selected a resolution, and it is available,
    //   then we will pick exactly that.
    // * If the user has selected a resolution, but no longer has a
    //   screen that big, then we will pick the largest resolution that
    //   is still available.
    // * If the user has never selected a resolution, then we will pick
    //   the largest one available.
    int user_width = INT_MAX;
    int user_height = INT_MAX;
    if ([[NSUserDefaults standardUserDefaults] objectForKey:kScreenWidth] != nil) {
        user_width = [[NSUserDefaults standardUserDefaults] integerForKey:kScreenWidth];
    }
    if ([[NSUserDefaults standardUserDefaults] objectForKey:kScreenHeight] != nil) {
        user_height = [[NSUserDefaults standardUserDefaults] integerForKey:kScreenHeight];
    }
    int best_resolution = -1;

    [_resolution_list removeAllItems];
    NSMutableArray* display_modes = [NSMutableArray
        arrayWithArray:
            [(NSArray*)CGDisplayCopyAllDisplayModes(kCGDirectMainDisplay, nil) autorelease]];
    [display_modes sortUsingFunction:compare_resolutions context:nil];
    int i;
    for (i = 0; i < [display_modes count]; ++i) {
        NSDictionary* display_mode = [display_modes objectAtIndex:i];
        int width = CGDisplayModeGetWidth((CGDisplayModeRef)display_mode);
        int height = CGDisplayModeGetHeight((CGDisplayModeRef)display_mode);

        // Formatted string is as produced in the "Displays" pane in
        // System Preferences.
        NSString* title = [NSString stringWithFormat:@"%d × %d", width, height];

        // We get multiple display modes with the same width and height,
        // because display modes also encode color depth and maybe other
        // things we're not interested in.  If 
        if ([_resolution_list indexOfItemWithTitle:title] >= 0) {
            continue;
        }

        // Use the display mode as the represented object of the menu
        // item.  It has some extraneous information we're not
        // interested in, but we've already got it and it has the width
        // and height of the screen.
        NSMenuItem* menu_item = [[[NSMenuItem alloc] initWithTitle:title action:nil
            keyEquivalent:@""] autorelease];
        [menu_item setRepresentedObject:display_mode];
        [menu_item setTarget:self];
        [menu_item setAction:@selector(setResolutionFrom:)];
        [[_resolution_list menu] addItem:menu_item];

        if ((height <= user_height) && (width <= user_width)) {
            best_resolution = [_resolution_list numberOfItems] - 1;
        }
    }

    // Select the best resolution from the list.  This doesn't trigger
    // its action, so fire that off manually.  We care about doing so
    // because, in the event that the user had an invalid preference or
    // no preference, we want to use the best match as chosen above.
    [_resolution_list selectItemAtIndex:best_resolution];
    [self setResolutionFrom:[_resolution_list itemAtIndex:best_resolution]];

    // TODO(sfiera): watch for resolution changes and update the list.
}

- (void)setScenarioFrom:(NSMenuItem*)sender {
    NSString* identifier = [[sender representedObject] objectForKey:kIdentifier];
    NSString* title = [[sender representedObject] objectForKey:kTitle];
    NSString* author = [[sender representedObject] objectForKey:kAuthor];
    NSString* version = [[sender representedObject] objectForKey:kVersion];

    [_scenario_button setTitle:title];
    [self setDownloadURL:[[sender representedObject] objectForKey:kDownloadURL]];
    [_author_button setTitle:author];
    [self setAuthorURL:[[sender representedObject] objectForKey:kAuthorURL]];
    [_version_label setStringValue:version];

    [[NSUserDefaults standardUserDefaults] setObject:identifier forKey:kScenario];
}

- (void)updateScenarioList {
    NSString* user_scenario = [[NSUserDefaults standardUserDefaults] stringForKey:kScenario];
    int factory_scenario = -1;
    int best_scenario = -1;

    NSMutableArray* scenarios = [NSMutableArray array];
    AntaresScenarioList* list = antares_scenario_list_create();
    size_t i;
    for (i = 0; i < antares_scenario_list_size(list); ++i) {
        AntaresScenarioListEntry* entry = antares_scenario_list_at(list, i);
        NSString* title = str(antares_scenario_list_entry_title(entry));
        NSString* identifier = str(antares_scenario_list_entry_identifier(entry));
        NSDictionary* scenario_info = [NSDictionary dictionaryWithObjectsAndKeys:
            identifier, kIdentifier,
            title, kTitle,
            url(antares_scenario_list_entry_download_url(entry)), kDownloadURL,
            str(antares_scenario_list_entry_author(entry)), kAuthor,
            url(antares_scenario_list_entry_author_url(entry)), kAuthorURL,
            str(antares_scenario_list_entry_version(entry)), kVersion,
            NULL
        ];
        [scenarios addObject:scenario_info];
    }
    antares_scenario_list_destroy(list);
    [scenarios sortUsingFunction:compare_scenarios context:nil];

    [_scenario_list removeAllItems];
    for (i = 0; i < [scenarios count]; ++i) {
        NSDictionary* scenario = [scenarios objectAtIndex:i];
        NSString* title = [scenario objectForKey:kTitle];
        NSString* identifier = [scenario objectForKey:kIdentifier];

        NSMenuItem* menu_item = [[[NSMenuItem alloc] initWithTitle:title action:nil
            keyEquivalent:@""] autorelease];
        [menu_item setRepresentedObject:scenario];
        [menu_item setTarget:self];
        [menu_item setAction:@selector(setScenarioFrom:)];
        [[_scenario_list menu] addItem:menu_item];

        if ([identifier isEqualToString:user_scenario]) {
            best_scenario = i;
        }
        if ([identifier isEqualToString:kFactoryScenario]) {
            factory_scenario = i;
        }
    }

    if (best_scenario < 0) {
        best_scenario = factory_scenario;
    }

    [_scenario_list selectItemAtIndex:best_scenario];
    [self setScenarioFrom:[_scenario_list itemAtIndex:best_scenario]];
}

- (void)awakeFromNib {
    [self updateResolutionList];
    [self updateScenarioList];
    bool windowed = 
        [[NSUserDefaults standardUserDefaults] objectForKey:kFullscreen]
        && ![[NSUserDefaults standardUserDefaults] boolForKey:kFullscreen];
    [_window_checkbox setIntValue:windowed];
    bool skip_settings = 
        [[NSUserDefaults standardUserDefaults] boolForKey:kSkipSettings];
    [_skip_checkbox setIntValue:skip_settings];
    [self setWindowedFrom:_window_checkbox];
}

- (IBAction)openScenarioURL:(id)sender {
    [[NSWorkspace sharedWorkspace] openURL:_download_url];
}

- (IBAction)openAuthorURL:(id)sender {
    [[NSWorkspace sharedWorkspace] openURL:_author_url];
}

- (IBAction)setWindowedFrom:(id)sender {
    bool fullscreen = ![sender intValue];
    [[NSUserDefaults standardUserDefaults] setBool:fullscreen forKey:kFullscreen];
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
        NSLog(@"%@", error_message);
        CFRelease(error_message);
        exit(1);
    }

    NSString* scenario = [[NSUserDefaults standardUserDefaults] stringForKey:kScenario];
    AntaresExtractDataController* extract = [[[AntaresExtractDataController alloc]
        initWithTarget:self selector:@selector(extractDone:) scenario:scenario] autorelease];
    if (!extract) {
        NSLog(@"Failed to create AntaresExtractDataController");
        exit(1);
    }
}

- (void)installDone:(id)sender {
    [self updateScenarioList];
    [_window center];
    [_window makeKeyAndOrderFront:self];
}

- (void)extractDone:(id)sender {
    CFStringRef error_message;
    if (!antares_controller_loop(drivers, &error_message)) {
        NSLog(@"%@", error_message);
        CFRelease(error_message);
        exit(1);
    }
    antares_controller_destroy_drivers(drivers);
    [NSApp terminate:self];
}

- (void)dealloc {
    [_download_url release];
    [_author_url release];
    [super dealloc];
}

- (NSURL*)authorURL {
    @synchronized (self) {
        return [[_author_url retain] autorelease];
    }
}

- (void)setAuthorURL:(NSURL*)authorURL {
    @synchronized (self) {
        [authorURL retain];
        [_author_url release];
        _author_url = authorURL;
    }
}

- (NSURL*)downloadURL {
    @synchronized (self) {
        return [[_download_url retain] autorelease];
    }
}

- (void)setDownloadURL:(NSURL*)downloadURL {
    @synchronized (self) {
        [downloadURL retain];
        [_download_url release];
        _download_url = downloadURL;
    }
}
@end
