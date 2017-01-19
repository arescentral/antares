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
#include <sfz/sfz.hpp>
#include <vector>

#include "ui/event.hpp"

namespace antares {

struct ReplayData {
    struct Scenario {
        sfz::String identifier;
        sfz::String version;
    };

    struct Action {
        uint64_t             at;
        std::vector<uint8_t> keys_down;
        std::vector<uint8_t> keys_up;
    };

    Scenario            scenario;
    int32_t             chapter_id;
    int32_t             global_seed;
    uint64_t            duration;
    std::vector<Action> actions;

    ReplayData();
    ReplayData(sfz::BytesSlice in);

    void key_down(uint64_t at, uint32_t key);
    void key_up(uint64_t at, uint32_t key);
};
void read_from(sfz::ReadSource in, ReplayData& replay);
void read_from(sfz::ReadSource in, ReplayData::Scenario& scenario);
void read_from(sfz::ReadSource in, ReplayData::Action& action);
void write_to(sfz::WriteTarget out, const ReplayData& replay);
void write_to(sfz::WriteTarget out, const ReplayData::Scenario& scenario);
void write_to(sfz::WriteTarget out, const ReplayData::Action& action);

class ReplayBuilder : public EventReceiver {
  public:
    ReplayBuilder();

    void init(
            sfz::StringSlice scenario_identifier, sfz::StringSlice scenario_version,
            int32_t chapter_id, int32_t global_seed);
    void         start();
    virtual void key_down(const KeyDownEvent& key);
    virtual void key_up(const KeyUpEvent& key);
    void next();
    void finish();

  private:
    std::unique_ptr<sfz::ScopedFd> _file;
    ReplayData::Scenario           _scenario;
    int32_t                        _chapter_id;
    int32_t                        _global_seed;
    uint64_t                       _at;
};

}  // namespace antares

#endif  // ANTARES_DATA_REPLAY_HPP_
