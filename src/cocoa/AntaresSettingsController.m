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

#include <ApplicationServices/ApplicationServices.h>

#define kScreenWidth @"ScreenWidth"
#define kScreenHeight @"ScreenHeight"

// A comparator for display mode dictionaries.  Orders the objects as
// the pair (screen height, screen width).  The case where this ordering
// differs from (screen width, screen height) is unlikely enough that
// it's not worth doing anything fancier.
static int compare_resolutions(id x, id y, void* unused) {
    (void)unused;

    int x_width = [[x objectForKey:(NSString*)kCGDisplayWidth] intValue];
    int x_height = [[x objectForKey:(NSString*)kCGDisplayHeight] intValue];
    int y_width = [[y objectForKey:(NSString*)kCGDisplayWidth] intValue];
    int y_height = [[y objectForKey:(NSString*)kCGDisplayHeight] intValue];

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

- (void)setResolutionFrom:(NSMenuItem*)sender {
    // Assumes that the sender's representedObject is a display mode
    // dictionary, as set below.  Updates user defaults with the width
    // and height of the selected resolution.
    int width = [[[sender representedObject] objectForKey:(NSString*)kCGDisplayWidth] intValue];
    int height = [[[sender representedObject] objectForKey:(NSString*)kCGDisplayHeight] intValue];
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
        arrayWithArray:(NSArray*)CGDisplayAvailableModes(kCGDirectMainDisplay)];
    [display_modes sortUsingFunction:compare_resolutions context:nil];
    int i;
    for (i = 0; i < [display_modes count]; ++i) {
        NSDictionary* display_mode = [display_modes objectAtIndex:i];
        int width = [[display_mode objectForKey:(NSString*)kCGDisplayWidth] intValue];
        int height = [[display_mode objectForKey:(NSString*)kCGDisplayHeight] intValue];

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

- (void)awakeFromNib {
    [self updateResolutionList];
    [_window center];
    [_window makeKeyAndOrderFront:self];
}

- (IBAction)openScenarioURL:(id)sender {
    // TODO(sfiera): read from nlAG.
    [[NSWorkspace sharedWorkspace] openURL: [NSURL URLWithString:@"http://www.arescentral.com"]];
}

- (IBAction)openAuthorURL:(id)sender {
    // TODO(sfiera): read from nlAG.
    [[NSWorkspace sharedWorkspace] openURL: [NSURL URLWithString:@"http://www.biggerplanet.com"]];
}

- (IBAction)startGame:(id)sender {
    [_window close];
    [_target performSelector:_selector withObject:self];
    [self release];
}

@end
