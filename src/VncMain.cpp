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

#include "Fakes.hpp"

#include <sys/time.h>
#include <getopt.h>
#include <queue>

#include "sfz/Exception.hpp"
#include "sfz/Format.hpp"
#include "sfz/String.hpp"
#include "AresMain.hpp"
#include "CardStack.hpp"
#include "Error.hpp"
#include "FakeDrawing.hpp"
#include "FakeSounds.hpp"
#include "File.hpp"
#include "ImageDriver.hpp"
#include "Ledger.hpp"
#include "LibpngImageDriver.hpp"
#include "Preferences.hpp"
#include "Threading.hpp"
#include "VideoDriver.hpp"
#include "VncServer.hpp"

using sfz::Exception;
using sfz::String;
using sfz::StringPiece;
using sfz::ascii_encoding;
using sfz::utf8_encoding;

namespace antares {

void usage(const StringPiece& program_name) {
    print(
            2,
            "usage: {0} [<options>]\n"
            "options:\n"
            "    -w|--width=<int>   width of screen (default: 640)\n"
            "    -h|--height=<int>  height of screen (default: 480)\n",
            program_name);
    exit(1);
}

int string_to_int(const char* string) {
    int value = 0;
    char* end = NULL;
    if (string && *string) {
        value = strtol(string, &end, 10);
    }
    if (!string || end != string + strlen(string)) {
        fail("Couldn't parse '%s' as an integer", string);
    }
    return value;
}

void VncMain(int argc, char* const* argv) {
    String program_name(argv[0], utf8_encoding());
    try {
        int width = 640;
        int height = 480;
        option longopts[] = {
            { "width",          required_argument,  NULL,   'w' },
            { "height",         required_argument,  NULL,   'h' },
            { NULL,             0,                  NULL,   0 }
        };

        char ch;
        while ((ch = getopt_long(argc, argv, "w:h:", longopts, NULL)) != -1) {
            switch (ch) {
              case 'w':
                width = string_to_int(optarg);
                break;
              case 'h':
                height = string_to_int(optarg);
                break;
              default:
                {
                    StringPiece opt(argv[optind], utf8_encoding());
                    print(2, "{0}: unknown argument {1}\n", program_name, opt);
                    usage(program_name);
                }
                break;
            }
        }

        argc -= optind;
        argv += optind;
        if (argc != 0) {
            print(2, "{0}: too many arguments\n", program_name);
            usage(program_name);
        }

        FakeDrawingInit(width, height);
        ImageDriver::set_driver(new LibpngImageDriver);
        VideoDriver::set_driver(new VncVideoDriver(5901));
        SoundDriver::set_driver(new NullSoundDriver);

        if (getenv("HOME") == NULL) {
            Ledger::set_ledger(new NullLedger);
        } else {
            String directory(getenv("HOME"), utf8_encoding());
            directory.append("/.antares", ascii_encoding());
            Ledger::set_ledger(new DirectoryLedger(directory));
        }

        CardStack stack(AresInit());
        VideoDriver::driver()->loop(&stack);
    } catch (Exception& e) {
        print(2, "{0}: {1}\n", program_name, e);
    }
}

}  // namespace antares

int main(int argc, char* const* argv) {
    antares::VncMain(argc, argv);
    return 0;
}
