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

#include "Ledger.hpp"

#include <fcntl.h>
#include <unistd.h>
#include "sfz/sfz.hpp"

using sfz::Bytes;
using sfz::Exception;
using sfz::MappedFile;
using sfz::ScopedFd;
using sfz::String;
using sfz::StringPiece;
using sfz::makedirs;
using sfz::print;
using sfz::scoped_ptr;
using sfz::string_to_int32_t;
using sfz::write;

namespace path = sfz::path;
namespace utf8 = sfz::utf8;

namespace antares {

Ledger::~Ledger() { }

namespace {

Ledger* ledger;

}  // namespace

Ledger* Ledger::ledger() {
    return ::antares::ledger;
}

void Ledger::set_ledger(Ledger* ledger) {
    ::antares::ledger = ledger;
}

NullLedger::NullLedger() {
    _chapters.insert(1);
}

void NullLedger::unlock_chapter(int chapter) {
    _chapters.insert(chapter);
}

void NullLedger::unlocked_chapters(std::vector<int>* chapters) {
    *chapters = std::vector<int>(_chapters.begin(), _chapters.end());
}

DirectoryLedger::DirectoryLedger(const String& directory)
        : _directory(directory) {
    load();
}

void DirectoryLedger::unlock_chapter(int chapter) {
    _chapters.insert(chapter);
    save();
}

void DirectoryLedger::unlocked_chapters(std::vector<int>* chapters) {
    *chapters = std::vector<int>(_chapters.begin(), _chapters.end());
}

void DirectoryLedger::load() {
    String path(_directory);
    path.append("/com.biggerplanet.ares.json");
    _chapters.clear();
    scoped_ptr<MappedFile> file;
    try {
        file.reset(new MappedFile(path));
    } catch (Exception& e) {
        _chapters.insert(1);
        return;
    }

    String data(utf8::decode(file->data()));

    // This is not a real JSON parser, but it plays on on the Interstellar News Network.  It simply
    // finds all integers in the file, which is fine for now.  The only numerical data we currently
    // write to the ledger is the chapter numbers, and all chapter numbers are integers.
    foreach (it, StringPiece(data)) {
        String number;
        while ('0' <= *it && *it <= '9') {
            number.append(1, *it);
            ++it;
        }
        int32_t chapter;
        if (number.size() > 0 && string_to_int32_t(number, &chapter)) {
            _chapters.insert(chapter);
        }
    }
}

void DirectoryLedger::save() {
    String path(_directory);
    path.append("/com.biggerplanet.ares.json");

    String contents;
    contents.append("{\n");
    contents.append("  \"unlocked-levels\": [");
    for (std::set<int>::const_iterator it = _chapters.begin(); it != _chapters.end(); ++it) {
        if (it != _chapters.begin()) {
            print(&contents, ", ");
        }
        print(&contents, *it);
    }
    contents.append("]\n");
    contents.append("}\n");

    makedirs(path::dirname(path), 0755);
    ScopedFd fd(open(path, O_WRONLY | O_CREAT, 0644));
    Bytes bytes(utf8::encode(contents));
    write(&fd, bytes);
}

}  // namespace antares
