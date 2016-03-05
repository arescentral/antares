// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2016 The Antares Authors
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

#include "data/plugin.hpp"

#include "data/resource.hpp"
#include "lang/defines.hpp"
#include "data/space-object.hpp"

using sfz::BytesSlice;
using sfz::Exception;

namespace antares {

static const int16_t kLevelNameID                = 4600;
static const int16_t kSpaceObjectNameResID       = 5000;
static const int16_t kSpaceObjectShortNameResID  = 5001;

static const int16_t kLevelResID                 = 500;
static const int16_t kLevelInitialResID          = 500;
static const int16_t kLevelConditionResID        = 500;
static const int16_t kLevelBriefResID            = 500;
static const int16_t kBaseObjectResID            = 500;
static const int16_t kObjectActionResID          = 500;
static const int16_t kRaceResID                  = 500;

ANTARES_GLOBAL ScenarioGlobals plug;

void PluginInit() {
    {
        Resource rsrc("scenario-info", "nlAG", 128);
        BytesSlice in(rsrc.data());
        read(in, plug.meta);
        if (!in.empty()) {
            throw Exception("didn't consume all of scenario file info data");
        }
    }

    {
        StringList chapter_names(kLevelNameID);
        plug.chapters.clear();
        Resource rsrc("scenarios", "snro", kLevelResID);
        BytesSlice in(rsrc.data());
        while (!in.empty()) {
            Level level;
            read(in, level);
            level.name.assign(chapter_names.at(level.levelNameStrNum - 1));
            plug.chapters.push_back(level);
        }
    }

    {
        plug.initials.clear();
        Resource rsrc("scenario-initial-objects", "snit", kLevelInitialResID);
        BytesSlice in(rsrc.data());
        while (!in.empty()) {
            Level::InitialObject initial;
            read(in, initial);
            plug.initials.push_back(initial);
        }
    }

    {
        plug.conditions.clear();
        Resource rsrc("scenario-conditions", "sncd", kLevelConditionResID);
        BytesSlice in(rsrc.data());
        while (!in.empty()) {
            Level::Condition condition;
            read(in, condition);
            plug.conditions.push_back(condition);
        }
    }

    {
        plug.briefings.clear();
        Resource rsrc("scenario-briefing-points", "snbf", kLevelBriefResID);
        BytesSlice in(rsrc.data());
        while (!in.empty()) {
            Level::BriefPoint brief_point;
            read(in, brief_point);
            plug.briefings.push_back(brief_point);
        }
    }

    {
        StringList object_names(kSpaceObjectNameResID);
        StringList object_short_names(kSpaceObjectShortNameResID);
        Resource rsrc("objects", "bsob", kBaseObjectResID);
        BytesSlice in(rsrc.data());
        size_t count = rsrc.data().size() / BaseObject::byte_size;
        plug.objects.resize(count);
        for (size_t i = 0; i < count; ++i) {
            read(in, plug.objects[i]);
            plug.objects[i].name.assign(object_names.at(i));
            plug.objects[i].short_name.assign(object_short_names.at(i));
        }
        if (!in.empty()) {
            throw Exception("didn't consume all of base object data");
        }
    }

    {
        Resource rsrc("object-actions", "obac", kObjectActionResID);
        BytesSlice in(rsrc.data());
        size_t count = rsrc.data().size() / Action::byte_size;
        plug.actions.resize(count);
        for (size_t i = 0; i < count; ++i) {
            read(in, plug.actions[i]);
        }
        if (!in.empty()) {
            throw Exception("didn't consume all of object action data");
        }
    }

    {
        Resource rsrc("races", "race", kRaceResID);
        BytesSlice in(rsrc.data());
        size_t count = rsrc.data().size() / Race::byte_size;
        plug.races.resize(count);
        for (size_t i = 0; i < count; ++i) {
            read(in, plug.races[i]);
        }
        if (!in.empty()) {
            throw Exception("didn't consume all of race data");
        }
    }
}

}  // namespace antares
