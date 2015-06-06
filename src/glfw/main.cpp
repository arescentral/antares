// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2015 The Antares Authors
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

#include <GLFW/glfw3.h>

#include <sfz/sfz.hpp>
#include <time.h>

#include "config/file-prefs-driver.hpp"
#include "config/preferences.hpp"
#include "config/ledger.hpp"
#include "glfw/video-driver.hpp"
#include "sound/openal-driver.hpp"
#include "ui/flows/master.hpp"

using sfz::String;

namespace antares {

String application_path() {
    return String("./data");
}

void main(int argc, const char* argv[]) {
    FilePrefsDriver prefs;
    DirectoryLedger ledger;
    OpenAlSoundDriver sound;
    GLFWVideoDriver video;
    video.loop(new Master(time(NULL)));
}

}  // namespace antares

int main(int argc, const char* argv[]) {
    antares::main(argc, argv);
    return 0;
}
