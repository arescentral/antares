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

#ifndef ANTARES_LEDGER_HPP_
#define ANTARES_LEDGER_HPP_

#include <set>
#include <vector>
#include "sfz/Macros.hpp"
#include "sfz/String.hpp"

namespace antares {

class Ledger {
  public:
    virtual ~Ledger();

    static Ledger* ledger();
    static void set_ledger(Ledger* ledger);

    virtual void unlock_chapter(int chapter) = 0;
    virtual void unlocked_chapters(std::vector<int>* chapters) = 0;
};

class NullLedger : public Ledger {
  public:
    NullLedger();
    virtual void unlock_chapter(int chapter);
    virtual void unlocked_chapters(std::vector<int>* chapters);

  private:
    std::set<int> _chapters;

    DISALLOW_COPY_AND_ASSIGN(NullLedger);
};

class DirectoryLedger : public Ledger {
  public:
    DirectoryLedger(const sfz::String& directory);
    virtual void unlock_chapter(int chapter);
    virtual void unlocked_chapters(std::vector<int>* chapters);

  private:
    void load();
    void save();

    const sfz::String _directory;
    std::set<int> _chapters;

    DISALLOW_COPY_AND_ASSIGN(DirectoryLedger);
};

}  // namespace antares

#endif  // ANTARES_LEDGER_HPP_
