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

#ifndef ANTARES_DATA_EXTRACTOR_HPP_
#define ANTARES_DATA_EXTRACTOR_HPP_

#include <sfz/sfz.hpp>

namespace antares {

class DataExtractor {
  public:
    struct Observer {
        virtual ~Observer();
        virtual void status(const sfz::StringSlice& status) = 0;
    };

    DataExtractor(const sfz::StringSlice& downloads_dir, const sfz::StringSlice& output_dir);

    void set_scenario(sfz::StringSlice scenario);
    void set_plugin_file(sfz::StringSlice path);

    bool current() const;
    void extract(Observer* observer) const;

  private:
    bool scenario_current(sfz::StringSlice scenario) const;

    void extract_factory_scenario(Observer* observer) const;
    void extract_plugin_scenario(Observer* observer) const;

    void download(
            Observer* observer, const sfz::StringSlice& base, const sfz::StringSlice& name,
            const sfz::StringSlice& version, const sfz::Sha1::Digest& digest) const;
    void write_version(sfz::StringSlice scenario_identifier) const;
    void extract_original(Observer* observer, const sfz::StringSlice& zip) const;
    void extract_supplemental(Observer* observer, const sfz::StringSlice& zip) const;
    void extract_plugin(Observer* observer) const;

    const sfz::String _downloads_dir;
    const sfz::String _output_dir;
    sfz::String       _scenario;
};

}  // namespace antares

#endif  // ANTARES_DATA_EXTRACTOR_HPP_
