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

#include "AntaresController.h"

#include <stdlib.h>
#include "sfz/sfz.hpp"
#include "AresMain.hpp"
#include "AntaresExtractDataController.h"
#include "CardStack.hpp"
#include "CocoaPrefsDriver.hpp"
#include "CocoaVideoDriver.hpp"
#include "FakeDrawing.hpp"
#include "FakeSounds.hpp"
#include "FoundationHttpDriver.hpp"
#include "ImageDriver.hpp"
#include "Ledger.hpp"
#include "LibpngImageDriver.hpp"
#include "PrefsDriver.hpp"
#include "VideoDriver.hpp"

using sfz::CString;
using sfz::Exception;
using sfz::String;
using antares::AresInit;
using antares::FakeDrawingInit;
using antares::CardStack;
using antares::CocoaPrefsDriver;
using antares::CocoaVideoDriver;
using antares::DirectoryLedger;
using antares::FoundationHttpDriver;
using antares::HttpDriver;
using antares::ImageDriver;
using antares::Ledger;
using antares::LibpngImageDriver;
using antares::NullLedger;
using antares::NullSoundDriver;
using antares::PrefsDriver;
using antares::SoundDriver;
using antares::VideoDriver;

namespace utf8 = sfz::utf8;

@implementation AntaresController

- (void)applicationWillFinishLaunching:(NSNotification*)aNotification {
    if (getenv("HOME") == NULL) {
        NSLog(@"Couldn't get $HOME");
        exit(1);
    }
    const String home(utf8::decode(getenv("HOME")));

    FakeDrawingInit(640, 480);
    ImageDriver::set_driver(new LibpngImageDriver);
    VideoDriver::set_driver(new CocoaVideoDriver);
    SoundDriver::set_driver(new NullSoundDriver);
    PrefsDriver::set_driver(new CocoaPrefsDriver);
    HttpDriver::set_driver(new FoundationHttpDriver);
    const String ledger_directory(format("{0}/Library/Application Support/Antares", home));
    Ledger::set_ledger(new DirectoryLedger(ledger_directory));

    AntaresExtractDataController* extract = [[[AntaresExtractDataController alloc]
        initWithTarget:self selector:@selector(run:)] autorelease];
    if (!extract) {
        NSLog(@"Failed to create AntaresExtractDataController");
        exit(1);
    }
}

- (void)run:(id)sender {
    try {
        antares::CardStack* stack = new CardStack(AresInit());
        VideoDriver::driver()->loop(stack);
    } catch (Exception& e) {
        CString c_str(e.message());
        NSLog(@"sfz::Exception: %s", c_str.data());
    }
    [NSApp terminate:self];
}

@end
