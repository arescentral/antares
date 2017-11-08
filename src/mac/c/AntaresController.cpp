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

#include "mac/c/AntaresController.h"

#include <stdlib.h>
#include <sfz/sfz.hpp>

#include "config/ledger.hpp"
#include "config/preferences.hpp"
#include "game/globals.hpp"
#include "mac/core-foundation.hpp"
#include "mac/prefs-driver.hpp"
#include "mac/video-driver.hpp"
#include "sound/openal-driver.hpp"
#include "ui/card.hpp"
#include "ui/flows/master.hpp"
#include "video/driver.hpp"

using sfz::CString;
using sfz::Exception;
using sfz::String;
using sfz::StringSlice;
using antares::CardStack;
using antares::CocoaVideoDriver;
using antares::CoreFoundationPrefsDriver;
using antares::DirectoryLedger;
using antares::Ledger;
using antares::Master;
using antares::NullLedger;
using antares::OpenAlSoundDriver;
using antares::Preferences;
using antares::PrefsDriver;
using antares::SoundDriver;
using antares::VideoDriver;

namespace utf8 = sfz::utf8;

struct AntaresDrivers {
    CoreFoundationPrefsDriver prefs;
    CocoaVideoDriver          video;
    OpenAlSoundDriver         sound;
    DirectoryLedger           ledger;
};

namespace antares {

extern "C" AntaresDrivers* antares_controller_create_drivers(CFStringRef* error_message) {
    return new AntaresDrivers();
}

extern "C" void antares_controller_destroy_drivers(AntaresDrivers* drivers) {
    delete drivers;
}

extern "C" bool antares_controller_loop(AntaresDrivers* drivers, CFStringRef* error_message) {
    try {
        drivers->video.loop(new Master(time(NULL)));
    } catch (Exception& e) {
        *error_message = cf::wrap(e.message()).release();
        return false;
    }
    return true;
}

}  // namespace antares
