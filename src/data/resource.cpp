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

#include "data/resource.hpp"

#include <stdio.h>
#include <pn/file>
#include <sfz/sfz.hpp>

#include "config/dirs.hpp"
#include "config/preferences.hpp"
#include "game/sys.hpp"

using std::unique_ptr;

namespace path = sfz::path;

namespace antares {

static unique_ptr<sfz::mapped_file> load_first(
        pn::string_view resource_path, const std::vector<pn::string_view>& dirs) {
    for (const auto& dir : dirs) {
        pn::string path = pn::format("{0}/{1}", dir, resource_path);
        if (path::isfile(path)) {
            return unique_ptr<sfz::mapped_file>(new sfz::mapped_file(path));
        }
    }
    throw std::runtime_error(
            pn::format("couldn't find resource {0}", pn::dump(resource_path, pn::dump_short))
                    .c_str());
}

static unique_ptr<sfz::mapped_file> load(pn::string_view resource_path) {
    pn::string scenario = scenario_dir(sys.prefs->scenario_identifier());
    pn::string factory  = scenario_dir(kFactoryScenarioIdentifier);
    pn::string app      = application_path().copy();
    return load_first(resource_path, {scenario, factory, app});
}

Resource::Resource(pn::string_view type, pn::string_view extension, int id)
        : Resource(pn::format("{0}/{1}.{2}", type, id, extension)) {}

Resource::Resource(pn::string_view resource_path) : _file(load(resource_path)) {}

Resource::~Resource() {}

pn::data_view Resource::data() const { return _file->data(); }

pn::string_view Resource::string() const {
    return pn::string_view{reinterpret_cast<const char*>(_file->data().data()),
                           static_cast<int>(_file->data().size())};
}

}  // namespace antares
