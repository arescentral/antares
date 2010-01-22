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
#include "File.hpp"
#include "MappedFile.hpp"
#include "PosixException.hpp"

namespace antares {

Ledger::~Ledger() { }

namespace {

Ledger* ledger;

void append_int(int i, std::string* out) {
    char buffer[20];
    sprintf(buffer, "%d", i);
    out->append(buffer);
}

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

DirectoryLedger::DirectoryLedger(const std::string& directory)
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
    const std::string path = _directory + "/com.biggerplanet.ares.json";
    _chapters.clear();
    scoped_ptr<MappedFile> file;
    try {
        file.reset(new MappedFile(path));
    } catch (PosixException& e) {
        _chapters.insert(1);
        return;
    }

    // Copy file data into a string, so that it's NUL-terminated, then close the file.
    const std::string data(file->data(), file->size());
    file.reset();

    // This is not a real JSON parser, but it plays on on the Interstellar News Network.  It simply
    // finds all integers in the file, which is fine for now.  The only numerical data we currently
    // write to the ledger is the chapter numbers, and all chapter numbers are integers.
    const char* pos = data.c_str();
    while (*pos) {
        char* end;
        int chapter = strtol(pos, &end, 10);
        if (pos == end) {
            ++pos;
        } else {
            _chapters.insert(chapter);
            pos = end;
        }
    }
}

void DirectoryLedger::save() {
    const std::string path = _directory + "/com.biggerplanet.ares.json";
    std::string contents = "{\n  \"unlocked-levels\" = [";
    for (std::set<int>::const_iterator it = _chapters.begin(); it != _chapters.end(); ++it) {
        if (it != _chapters.begin()) {
            contents.append(", ");
        }
        append_int(*it, &contents);
    }
    contents.append("]\n}\n");

    MakeDirs(DirName(path), 0755);
    int fd = open(path.c_str(), O_WRONLY | O_CREAT, 0644);
    write(fd, contents.c_str(), contents.size());
    close(fd);
}

}  // namespace antares
