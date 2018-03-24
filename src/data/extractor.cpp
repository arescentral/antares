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

#include "data/extractor.hpp"

#include <fcntl.h>
#include <math.h>
#include <stdint.h>
#include <pn/array>
#include <pn/file>
#include <pn/string>
#include <rezin/rezin.hpp>
#include <set>
#include <sfz/sfz.hpp>
#include <zipxx/zipxx.hpp>

#include "config/dirs.hpp"
#include "data/replay.hpp"
#include "drawing/pix-map.hpp"
#include "math/geometry.hpp"
#include "net/http.hpp"

using rezin::AppleDouble;
using rezin::Options;
using rezin::ResourceEntry;
using rezin::ResourceFork;
using rezin::ResourceType;
using rezin::Sound;
using rezin::StringList;
using rezin::aiff;
using sfz::StringMap;
using sfz::dec;
using sfz::makedirs;
using sfz::mapped_file;
using sfz::range;
using sfz::rmtree;
using sfz::sha1;
using std::set;
using std::unique_ptr;
using std::vector;
using zipxx::ZipArchive;
using zipxx::ZipFileReader;

namespace path = sfz::path;

namespace antares {

namespace {

const char kAresSounds[]      = "__MACOSX/Ares 1.2.0 ƒ/Ares Data ƒ/._Ares Sounds";
const int  kNonFreeSoundIDs[] = {
        502, 504, 505, 506, 507, 508, 509, 513, 514,  515,  516,   530,   531,   532,   535,
        536, 540, 548, 550, 552, 553, 562, 571, 2000, 2001, 12558, 17406, 28007, 31989,
};

bool convert_snd(pn::string_view, bool, int16_t, pn::data_view data, pn::file_view out) {
    Sound snd(data);
    out.write(aiff(snd));
    return true;
}

static const char kDownloadBase[] = "http://downloads.arescentral.org";
static const char kVersion[]      = "19\n";

static const char kPluginVersionFile[]    = "version";
static const char kPluginIdentifierFile[] = "identifier";

void check_version(ZipArchive& archive, pn::string_view expected) {
    ZipFileReader version_file(archive, kPluginVersionFile);
    pn::string    actual{reinterpret_cast<const char*>(version_file.data().data()),
                      static_cast<int>(version_file.data().size())};
    if (actual != expected) {
        throw std::runtime_error(
                pn::format("unsupported plugin version {0}", pn::dump(actual, pn::dump_short))
                        .c_str());
    }
}

void read_identifier(ZipArchive& archive, pn::string& out) {
    ZipFileReader identifier_file(archive, kPluginIdentifierFile);
    pn::string    actual{reinterpret_cast<const char*>(identifier_file.data().data()),
                      static_cast<int>(identifier_file.data().size())};
    if (actual.data()[actual.size() - 1] != '\n') {
        throw std::runtime_error(pn::format(
                                         "missing newline in plugin identifier {0}",
                                         pn::dump(actual, pn::dump_short))
                                         .c_str());
    }
    out = actual.substr(0, actual.size() - 1).copy();
}

void check_identifier(ZipArchive& archive, pn::string_view expected) {
    pn::string actual;
    read_identifier(archive, actual);
    if (expected != actual) {
        throw std::runtime_error(
                pn::format("mismatch in plugin identifier {0}", pn::dump(actual, pn::dump_short))
                        .c_str());
    }
}

}  // namespace

DataExtractor::Observer::~Observer() {}

DataExtractor::DataExtractor(pn::string_view downloads_dir, pn::string_view output_dir)
        : _downloads_dir(downloads_dir.copy()),
          _output_dir(output_dir.copy()),
          _scenario(kFactoryScenarioIdentifier) {}

void DataExtractor::set_scenario(pn::string_view scenario) { _scenario = scenario.copy(); }

void DataExtractor::set_plugin_file(pn::string_view path) {
    pn::string found_scenario;
    {
        // Make sure that the provided file is actually an archive that
        // will be usable as a plugin, then get its identifier.
        ZipArchive archive(path, 0);
        check_version(archive, kVersion);
        read_identifier(archive, found_scenario);
    }

    // Copy it to $DOWNLOADS/$IDENTIFIER.antaresplugin.  This is where
    // extract_scenario() will expect it later.
    pn::string out_path = pn::format("{0}/{1}.antaresplugin", _downloads_dir, found_scenario);
    if (path != out_path) {
        makedirs(path::dirname(out_path), 0755);
        mapped_file file(path);
        pn::file    f = pn::open(out_path, "w");
        f.write(file.data());
    }
    pn::string scenario_dir = pn::format("{0}/{1}", _output_dir, found_scenario);
    if (path::exists(scenario_dir)) {
        rmtree(scenario_dir);
    }

    std::swap(_scenario, found_scenario);
}

bool DataExtractor::current() const {
    return scenario_current(_scenario) && scenario_current(kFactoryScenarioIdentifier);
}

void DataExtractor::extract(Observer* observer) const {
    extract_factory_scenario(observer);
    extract_plugin_scenario(observer);
}

void DataExtractor::extract_factory_scenario(Observer* observer) const {
    if (!scenario_current(kFactoryScenarioIdentifier)) {
        download(
                observer, kDownloadBase, "Ares", "1.2.0",
                {0x246c393c, 0xa598af68, 0xa58cfdd1, 0x8e1601c1, 0xf4f30931});

        pn::string scenario_dir = pn::format("{0}/{1}", _output_dir, kFactoryScenarioIdentifier);
        rmtree(scenario_dir);
        extract_original(observer, "Ares-1.2.0.zip");
        write_version(kFactoryScenarioIdentifier);
    }
}

void DataExtractor::extract_plugin_scenario(Observer* observer) const {
    if ((_scenario != kFactoryScenarioIdentifier) && !scenario_current(_scenario)) {
        pn::string scenario_dir = pn::format("{0}/{1}", _output_dir, _scenario);
        rmtree(scenario_dir);
        extract_plugin(observer);
        write_version(_scenario);
    }
}

bool DataExtractor::scenario_current(pn::string_view scenario) const {
    pn::string    path    = pn::format("{0}/{1}/version", _output_dir, scenario);
    pn::data_view version = pn::string_view{kVersion}.as_data();
    try {
        mapped_file file(path);
        return file.data() == version;
    } catch (std::exception& e) {
        return false;
    }
}

void DataExtractor::download(
        Observer* observer, pn::string_view base, pn::string_view name, pn::string_view version,
        const sha1::digest& expected_digest) const {
    pn::string full_path = pn::format("{0}/{1}-{2}.zip", _downloads_dir, name, version);

    // Don't download `file` if it has already been downloaded.  If there is a regular file at
    // `full_path` and it has the expected digest, then return without doing anything.  Otherwise,
    // delete whatever's there (if anything).
    if (path::exists(full_path)) {
        if (path::isfile(full_path)) {
            mapped_file file(full_path);
            sha1        sha;
            sha.write(file.data());
            if (sha.compute() == expected_digest) {
                return;
            }
        }
        rmtree(full_path);
    }

    pn::string url = pn::format("{0}/{1}/{1}-{2}.zip", base, name, version);

    pn::string status = pn::format("Downloading {0}-{1}.zip...", name, version);
    observer->status(status);

    // Download the file from `url`.  Check its digest when it has been downloaded; if it is not
    // the right file, then throw an exception without writing it to disk.  Otherwise, write it to
    // disk.
    pn::data download;
    http::get(url, download.open("w"));
    sha1 sha;
    sha.write(download);
    if (sha.compute() != expected_digest) {
        throw std::runtime_error(
                pn::format(
                        "Downloaded {0}, size={1} but it didn't have the right digest.",
                        pn::dump(url, pn::dump_short), download.size())
                        .c_str());
    }

    // If we got the file, write it out at `full_path`.
    makedirs(path::dirname(full_path), 0755);
    pn::file file = pn::open(full_path, "w");
    file.write(download);
}

void DataExtractor::write_version(pn::string_view scenario_identifier) const {
    pn::string path = pn::format("{0}/{1}/version", _output_dir, scenario_identifier);
    makedirs(path::dirname(path), 0755);
    pn::file file = pn::open(path, "w");
    file.write(kVersion);
}

void DataExtractor::extract_original(Observer* observer, pn::string_view file) const {
    pn::string status = pn::format("Extracting {0}...", file);
    observer->status(status);
    pn::string full_path = pn::format("{0}/{1}", _downloads_dir, file);
    ZipArchive archive(full_path, 0);

    rezin::Options options;
    options.line_ending = rezin::Options::CR;

    ZipFileReader       zip(archive, kAresSounds);
    AppleDouble         apple_double(zip.data());
    ResourceFork        rsrc(apple_double.at(AppleDouble::RESOURCE_FORK), options);
    const ResourceType& snd = rsrc.at("snd ");

    for (int id : kNonFreeSoundIDs) {
        pn::data   data;
        pn::string output = pn::format(
                "{0}/{1}/sounds/{2}.{3}", _output_dir, kFactoryScenarioIdentifier, id, "aiff");
        if (convert_snd(path::dirname(output), true, id, snd.at(id).data(), data.open("w"))) {
            makedirs(path::dirname(output), 0755);
            pn::file file = pn::open(output, "w");
            file.write(data);
        }
    }
}

void DataExtractor::extract_plugin(Observer* observer) const {
    pn::string file   = pn::format("{0}.antaresplugin", _scenario);
    pn::string status = pn::format("Extracting {0}...", file);
    observer->status(status);
    pn::string full_path = pn::format("{0}/{1}", _downloads_dir, file);
    ZipArchive archive(full_path, 0);

    rezin::Options options;
    options.line_ending = rezin::Options::CR;

    check_version(archive, kVersion);
    check_identifier(archive, _scenario);

    for (size_t i : range(archive.size())) {
        ZipFileReader   file(archive, i);
        pn::string_view in_path = file.path();

        // Skip directories and identifier file.
        if ((in_path.rfind("/") == (in_path.size() - 1)) || (in_path == kPluginIdentifierFile)) {
            continue;
        }

        pn::string output_path = pn::format("{0}/{1}/{2}", _output_dir, _scenario, in_path);
        makedirs(path::dirname(output_path), 0755);
        pn::open(output_path, "w").write(file.data()).check();
    }
}

}  // namespace antares
