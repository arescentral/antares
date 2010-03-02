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
#include "sfz/Bytes.hpp"
#include "sfz/Exception.hpp"
#include "sfz/String.hpp"
#include "AresMain.hpp"
#include "CardStack.hpp"
#include "CocoaVideoDriver.hpp"
#include "FakeDrawing.hpp"
#include "FakeSounds.hpp"
#include "ImageDriver.hpp"
#include "Ledger.hpp"
#include "LibpngImageDriver.hpp"
#include "VideoDriver.hpp"

using sfz::Bytes;
using sfz::Exception;
using sfz::String;
using sfz::StringPiece;
using sfz::ascii_encoding;
using sfz::utf8_encoding;
using antares::AresInit;
using antares::FakeDrawingInit;
using antares::CardStack;
using antares::CocoaVideoDriver;
using antares::DirectoryLedger;
using antares::ImageDriver;
using antares::Ledger;
using antares::LibpngImageDriver;
using antares::NullLedger;
using antares::NullSoundDriver;
using antares::SoundDriver;
using antares::VideoDriver;

@implementation AntaresController

- (void)applicationWillFinishLaunching:(NSNotification*)aNotification {
    chdir([[[NSBundle mainBundle] resourcePath] UTF8String]);
    FakeDrawingInit(640, 480);
    ImageDriver::set_driver(new LibpngImageDriver);
    VideoDriver::set_driver(new CocoaVideoDriver);
    SoundDriver::set_driver(new NullSoundDriver);

    if (getenv("HOME") == NULL) {
        Ledger::set_ledger(new NullLedger);
    } else {
        String directory(getenv("HOME"), utf8_encoding());
        directory.append("/Library/Application Support/Antares", ascii_encoding());
        Ledger::set_ledger(new DirectoryLedger(directory));
    }

    _stack = new CardStack(AresInit());
    try {
        VideoDriver::driver()->loop(_stack);
    } catch (Exception& e) {
        String string;
        e.print_to(&string);
        Bytes bytes(string, utf8_encoding());
        bytes.resize(bytes.size() + 1);
        NSLog(@"sfz::Exception: %s", reinterpret_cast<const char*>(bytes.data()));
    }
    [NSApp terminate:self];
}

@end
