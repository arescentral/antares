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

#include "mac/AntaresExtractDataController.h"

#include "mac/c/DataExtractor.h"

static NSString* kAntaresDidInstallScenarioFromPath = @"AntaresDidInstallScenarioFromPath";

static void set_label(const char* status, void* userdata) {
    AntaresExtractDataController* controller = userdata;
    NSString*                     label      = [[NSString alloc] initWithUTF8String:status];
    [controller performSelectorOnMainThread:@selector(setAndReleaseLabel:)
                                 withObject:label
                              waitUntilDone:NO];
}

@implementation AntaresExtractDataController

@synthesize _window;
@synthesize _progress_bar;
@synthesize _status_field;

- (id)initWithTarget:(id)target selector:(SEL)selector path:(NSString*)path {
    if (!(self = [super init])) {
        return NULL;
    }
    _target   = [target retain];
    _selector = selector;
    _path     = [path retain];
    _scenario = nil;
    if (![[NSBundle mainBundle] loadNibNamed:@"ExtractData" owner:self topLevelObjects:nil]) {
        [self release];
        return nil;
    }
    return self;
}

- (id)initWithTarget:(id)target selector:(SEL)selector scenario:(NSString*)scenario {
    if (!(self = [super init])) {
        return NULL;
    }
    _target   = [target retain];
    _selector = selector;
    _path     = nil;
    _scenario = [scenario retain];
    if (![[NSBundle mainBundle] loadNibNamed:@"ExtractData" owner:self topLevelObjects:nil]) {
        [self release];
        return nil;
    }
    return self;
}

- (void)dealloc {
    [_target release];
    [_path release];
    [_scenario release];
    [super dealloc];
}

- (void)awakeFromNib {
    [_status_field setStringValue:@"Verifying scenario data..."];
    [_progress_bar startAnimation:self];
    [_window center];
    [_window makeKeyAndOrderFront:self];
    [NSThread detachNewThreadSelector:@selector(doWork) toTarget:self withObject:nil];
}

- (void)setAndReleaseLabel:(NSString*)label {
    [_status_field setStringValue:label];
    [label release];
}

- (void)done:(NSString*)error_message {
    [_window close];
    _success = (error_message == nil);
    if (!_success) {
        NSAlert* alert = [[NSAlert alloc] init];
        [alert setMessageText:@"Oops"];
        [alert setInformativeText:(NSString*)error_message];
        [alert addButtonWithTitle:@"Quit"];
        [alert runModal];
        [error_message release];
    } else {
        [[NSDistributedNotificationCenter defaultCenter]
                postNotificationName:kAntaresDidInstallScenarioFromPath
                              object:[_path stringByStandardizingPath]
                            userInfo:nil
                  deliverImmediately:YES];
    }
    [_target performSelector:_selector withObject:self];
}

- (void)doWork {
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

    NSString* antares   = [[NSSearchPathForDirectoriesInDomains(
            NSApplicationSupportDirectory, NSUserDomainMask, YES) objectAtIndex:0]
            stringByAppendingPathComponent:@"Antares"];
    NSString* downloads = [antares stringByAppendingPathComponent:@"Downloads"];
    NSString* scenarios = [antares stringByAppendingPathComponent:@"Scenarios"];

    CFStringRef error_message;
    if (_path) {
        if (!antares_data_extract_path(
                [downloads UTF8String], [scenarios UTF8String], [_path UTF8String], set_label,
                self, &error_message)) {
            [self performSelectorOnMainThread:@selector(done:) withObject:(NSString*)error_message waitUntilDone:NO];
            return;
        }
    } else {
        if (!antares_data_extract_identifier(
                [downloads UTF8String], [scenarios UTF8String], [_scenario UTF8String], set_label,
                self, &error_message)) {
            [self performSelectorOnMainThread:@selector(done:) withObject:(NSString*)error_message waitUntilDone:NO];
            return;
        }
    }
    [self performSelectorOnMainThread:@selector(done:) withObject:nil waitUntilDone:NO];
    [pool release];
}

- (bool)success {
    return _success;
}

@end
