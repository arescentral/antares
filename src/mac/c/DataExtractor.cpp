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

#include "mac/c/DataExtractor.h"

#include <sfz/sfz.hpp>

#include "data/extractor.hpp"

using sfz::CString;
using sfz::String;
using sfz::StringSlice;
using std::unique_ptr;

namespace utf8 = sfz::utf8;

struct AntaresDataExtractor {
    antares::DataExtractor cxx_obj;

    AntaresDataExtractor(const StringSlice& downloads_dir, const StringSlice& output_dir)
            : cxx_obj(downloads_dir, output_dir) {}
};

namespace antares {

extern "C" AntaresDataExtractor* antares_data_extractor_create(
        const char* downloads_dir, const char* output_dir) {
    const String downloads_dir_string(utf8::decode(downloads_dir));
    const String output_dir_string(utf8::decode(output_dir));
    return new AntaresDataExtractor(downloads_dir_string, output_dir_string);
}

extern "C" void antares_data_extractor_destroy(AntaresDataExtractor* extractor) {
    delete extractor;
}

extern "C" void antares_data_extractor_set_scenario(
        AntaresDataExtractor* extractor, const char* scenario) {
    const String scenario_string(utf8::decode(scenario));
    extractor->cxx_obj.set_scenario(scenario_string);
}

extern "C" void antares_data_extractor_set_plugin_file(
        AntaresDataExtractor* extractor, const char* path) {
    const String path_string(utf8::decode(path));
    extractor->cxx_obj.set_plugin_file(path_string);
}

extern "C" int antares_data_extractor_current(AntaresDataExtractor* extractor) {
    return extractor->cxx_obj.current();
}

namespace {

class UserDataObserver : public DataExtractor::Observer {
  public:
    UserDataObserver(void (*callback)(const char*, void*), void* userdata)
            : _callback(callback), _userdata(userdata) {}

    virtual void status(const StringSlice& status) {
        CString c_str(status);
        _callback(c_str.data(), _userdata);
    }

  private:
    void (*_callback)(const char*, void*);
    void* _userdata;
};

}  // namespace

extern "C" void antares_data_extractor_extract(
        AntaresDataExtractor* extractor, void (*callback)(const char*, void*), void* userdata) {
    UserDataObserver observer(callback, userdata);
    extractor->cxx_obj.extract(&observer);
}

}  // namespace antares
