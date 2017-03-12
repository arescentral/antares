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

#ifndef ANTARES_DATA_REPLAY_HPP_
#define ANTARES_DATA_REPLAY_HPP_

#include <stdint.h>
#include <pn/file>
#include <pn/string>
#include <vector>

#include "ui/event.hpp"

namespace antares {

struct ReplayData {
    struct Scenario {
        pn::string identifier;
        pn::string version;
        void       write_to(pn::file_view out) const;
    };

    struct Action {
        uint64_t             at;
        std::vector<uint8_t> keys_down;
        std::vector<uint8_t> keys_up;
        void                 write_to(pn::file_view out) const;
    };

    Scenario            scenario;
    int32_t             chapter_id;
    int32_t             global_seed;
    uint64_t            duration;
    std::vector<Action> actions;

    ReplayData();
    ReplayData(pn::data_view in);

    void write_to(pn::file_view out) const;
    void key_down(uint64_t at, uint32_t key);
    void key_up(uint64_t at, uint32_t key);
};
bool read_from(pn::file_view in, ReplayData* replay);
bool read_from(pn::file_view in, ReplayData::Scenario* scenario);
bool read_from(pn::file_view in, ReplayData::Action* action);

class ReplayBuilder : public EventReceiver {
  public:
    ReplayBuilder();

    void init(
            pn::string_view scenario_identifier, pn::string_view scenario_version,
            int32_t chapter_id, int32_t global_seed);
    void         start();
    virtual void key_down(const KeyDownEvent& key);
    virtual void key_up(const KeyUpEvent& key);
    void         next();
    void         finish();

  private:
    pn::file             _file;
    ReplayData::Scenario _scenario;
    int32_t              _chapter_id;
    int32_t              _global_seed;
    uint64_t             _at;
};

}  // namespace antares

#endif  // ANTARES_DATA_REPLAY_HPP_
