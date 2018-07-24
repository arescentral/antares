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

#ifndef ANTARES_DATA_LEVEL_HPP_
#define ANTARES_DATA_LEVEL_HPP_

#include <pn/string>
#include <vector>

#include "data/action.hpp"
#include "data/base-object.hpp"
#include "data/briefing.hpp"
#include "data/enums.hpp"
#include "data/handle.hpp"
#include "math/fixed.hpp"
#include "math/geometry.hpp"
#include "math/units.hpp"
#include "video/driver.hpp"

namespace antares {

struct Briefing;
struct Condition;
struct Initial;
struct Race;

struct Level {
    enum class Type { SOLO, NET, DEMO };

    Type type = Type::DEMO;

    struct Player {
        enum class Type { HUMAN, CPU };

        Type                    playerType = Type::CPU;
        NamedHandle<const Race> playerRace;
        pn::string              name;
        sfz::optional<Fixed>    earningPower;
        int16_t                 netRaceFlags = 0;
        Hue                     hue          = Hue::GRAY;
    };

    struct StatusLine {
        sfz::optional<pn::string> text;
        sfz::optional<pn::string> prefix;

        sfz::optional<int64_t>    condition;
        sfz::optional<pn::string> true_;
        sfz::optional<pn::string> false_;

        struct Counter {
            int64_t             player = 0;
            int64_t             which  = 0;
            sfz::optional<bool> fixed;
        };
        sfz::optional<Fixed>   minuend;
        sfz::optional<Counter> counter;

        sfz::optional<pn::string> suffix;
        sfz::optional<bool>       underline;

        StatusLine()                  = default;
        StatusLine(StatusLine&&)      = default;
        StatusLine(const StatusLine&) = delete;
    };

    sfz::optional<int64_t>                  chapter;
    pn::string                              name;
    std::vector<Player>                     players;
    std::vector<StatusLine>                 status;
    sfz::optional<pn::string>               song;
    sfz::optional<Rect>                     starmap;
    secs                                    startTime = secs(0);
    sfz::optional<NamedHandle<const Level>> skip;
    int32_t                                 angle = 0;
    struct Par {
        game_ticks time;
        int64_t    kills;
        int64_t    losses;
    } par;

    std::vector<Initial>   initials;
    std::vector<Condition> conditions;
    std::vector<Briefing>  briefings;

    sfz::optional<pn::string> prologue;           // SOLO
    sfz::optional<pn::string> epilogue;           // SOLO
    sfz::optional<pn::string> own_no_ships_text;  // SOLO, NET
    sfz::optional<pn::string> foe_no_ships_text;  // NET
    sfz::optional<pn::string> description;        // NET

    static const Level* get(int n);
    static const Level* get(pn::string_view n);
};
Level level(pn::value_cref x);

}  // namespace antares

#endif  // ANTARES_DATA_LEVEL_HPP_
