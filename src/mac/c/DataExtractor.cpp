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

#include "data/extractor.hpp"
#include "lang/exception.hpp"
#include "mac/core-foundation.hpp"

using std::unique_ptr;

namespace antares {
namespace {

class UserDataObserver : public DataExtractor::Observer {
  public:
    UserDataObserver(void (*callback)(const char*, void*), void* userdata)
            : _callback(callback), _userdata(userdata) {}

    virtual void status(pn::string_view status) { _callback(status.copy().c_str(), _userdata); }

  private:
    void (*_callback)(const char*, void*);
    void* _userdata;
};

}  // namespace

extern "C" bool antares_data_extract_path(
        const char* download_dir, const char* scenario_dir, const char* plugin_file,
        void (*callback)(const char*, void*), void* userdata, CFStringRef* error_message) {
    try {
        DataExtractor extractor(download_dir, scenario_dir);
        extractor.set_plugin_file(plugin_file);
        if (!extractor.current()) {
            UserDataObserver observer(callback, userdata);
            extractor.extract(&observer);
        }
    } catch (std::exception& e) {
        *error_message = cf::wrap(pn::string_view{full_exception_string(e)}).release();
        return false;
    }
    return true;
}

extern "C" bool antares_data_extract_identifier(
        const char* download_dir, const char* scenario_dir, const char* identifier,
        void (*callback)(const char*, void*), void* userdata, CFStringRef* error_message) {
    try {
        DataExtractor extractor(download_dir, scenario_dir);
        extractor.set_scenario(identifier);
        if (!extractor.current()) {
            UserDataObserver observer(callback, userdata);
            extractor.extract(&observer);
        }
    } catch (std::exception& e) {
        *error_message = cf::wrap(pn::string_view{full_exception_string(e)}).release();
        return false;
    }
    return true;
}

}  // namespace antares
