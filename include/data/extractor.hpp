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

#include <pn/string>
#include <sfz/sfz.hpp>

namespace antares {

class DataExtractor {
  public:
    struct Observer {
        virtual ~Observer();
        virtual void status(pn::string_view status) = 0;
    };

    DataExtractor(pn::string_view downloads_dir, pn::string_view output_dir);

    void set_scenario(pn::string_view scenario);
    void set_plugin_file(pn::string_view path);

    bool current() const;
    void extract(Observer* observer) const;

  private:
    bool scenario_current(pn::string_view scenario) const;

    void extract_factory_scenario(Observer* observer) const;
    void extract_plugin_scenario(Observer* observer) const;

    void download(
            Observer* observer, pn::string_view base, pn::string_view name,
            pn::string_view version, const sfz::sha1::digest& digest) const;
    void extract_original(Observer* observer, pn::string_view zip) const;
    void extract_supplemental(Observer* observer, pn::string_view zip) const;
    void extract_plugin(Observer* observer) const;

    const pn::string _downloads_dir;
    const pn::string _output_dir;
    pn::string       _scenario;
};

}  // namespace antares

#endif  // ANTARES_DATA_EXTRACTOR_HPP_
