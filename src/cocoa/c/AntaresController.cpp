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

#include "cocoa/c/AntaresController.h"

#include <stdlib.h>
#include <sfz/sfz.hpp>

#include "cocoa/core-foundation.hpp"
#include "cocoa/prefs-driver.hpp"
#include "cocoa/video-driver.hpp"
#include "config/ledger.hpp"
#include "config/preferences.hpp"
#include "game/globals.hpp"
#include "game/main.hpp"
#include "sound/openal-driver.hpp"
#include "ui/card.hpp"
#include "video/driver.hpp"

using sfz::CString;
using sfz::Exception;
using sfz::String;
using antares::AresInit;
using antares::CardStack;
using antares::CocoaVideoDriver;
using antares::CoreFoundationPrefsDriver;
using antares::DirectoryLedger;
using antares::Ledger;
using antares::NullLedger;
using antares::OpenAlSoundDriver;
using antares::Preferences;
using antares::PrefsDriver;
using antares::SoundDriver;
using antares::VideoDriver;
using antares::world;

namespace utf8 = sfz::utf8;

namespace antares {

extern "C" bool antares_controller_set_drivers(CFStringRef* error_message) {
    if (getenv("HOME") == NULL) {
        cf::String msg("Couldn't get $HOME");
        *error_message = msg.release();
        return false;
    }
    const String home(utf8::decode(getenv("HOME")));

    Preferences::set_preferences(new Preferences);
    PrefsDriver::set_driver(new CoreFoundationPrefsDriver);
    PrefsDriver::driver()->load(Preferences::preferences());
    VideoDriver::set_driver(new CocoaVideoDriver(Preferences::preferences()->screen_size()));
    SoundDriver::set_driver(new OpenAlSoundDriver);
    const String ledger_directory(format("{0}/Library/Application Support/Antares", home));
    Ledger::set_ledger(new DirectoryLedger(ledger_directory));

    return true;
}

extern "C" bool antares_controller_loop(CFStringRef* error_message) {
    try {
        VideoDriver::driver()->loop(AresInit());
    } catch (Exception& e) {
        cf::String msg(e.message());
        *error_message = msg.release();
        return false;
    }
    return true;
}

}  // namespace antares
