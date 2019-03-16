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
#include <sfz/sfz.hpp>
#include <zipxx/zipxx.hpp>

#include "config/dirs.hpp"
#include "data/field.hpp"
#include "data/info.hpp"
#include "data/replay.hpp"
#include "drawing/pix-map.hpp"
#include "math/geometry.hpp"
#include "net/http.hpp"

using sfz::makedirs;
using sfz::mapped_file;
using sfz::range;
using sfz::rmtree;
using sfz::sha1;
using zipxx::ZipArchive;
using zipxx::ZipFileReader;

namespace path = sfz::path;

namespace antares {

namespace {

const char kAresSounds[] = "__MACOSX/Ares 1.2.0 ƒ/Ares Data ƒ/._Ares Sounds";

struct SoundInfo {
    char name[32];
    struct {
        int     id;
        double  sample_rate;
        int64_t offset;
        int64_t size;
    } input;
    sha1::digest digest;
};

const SoundInfo kNonFreeSounds[] = {
        {"dev/fusion",
         {502, 22050, 959406, 19261},
         {0x29de6afe, 0xaf09ba9b, 0xa321e2df, 0x058efe4a, 0x7a104604}},
        {"dev/pk/can",
         {504, 22050, 384, 6750},
         {0xeb62526b, 0x413ccc64, 0xa3c40b1c, 0xd9eaab44, 0x7dacffd4}},
        {"dev/anti",
         {505, 22050, 1321083, 18693},
         {0x870a0c2a, 0xcd29683b, 0xba5684c4, 0x04828cb8, 0x026eb7b7}},
        {"gui/beep/order",
         {506, 22050, 1312482, 8555},
         {0x7160680b, 0xda68a321, 0xd4ab8045, 0x77a239ed, 0xab11484a}},
        {"gui/beep/select",
         {507, 22050, 1339822, 2322},
         {0x664d3624, 0x98828500, 0x0f9e91ff, 0x8ef13c90, 0x9355253e}},
        {"gui/beep/build",
         {508, 22050, 1173417, 3892},
         {0x758b84ce, 0xd7f57c82, 0xedbdb831, 0xdc7fb4a2, 0xb10473fc}},
        {"gui/beep/button",
         {509, 22050, 1194454, 2223},
         {0x7d33dc2c, 0x72168237, 0xec66bd18, 0x04597cb2, 0xf854ee18}},
        {"tpl/transport/land",
         {513, 11025, 1351516, 50332},
         {0x5bae5076, 0xbcacff7e, 0x48f4876a, 0x4b6800ab, 0xc6079a72}},
        {"sfx/energy",
         {514, 22050, 1287398, 6665},
         {0xfb2d4ce9, 0x03098593, 0x96f7af06, 0x8b7639ed, 0xb1f362da}},
        {"sfx/explosion/large",
         {515, 11025, 1146575, 26796},
         {0x70b1fb3e, 0x8251a6f2, 0xdfebcf0a, 0xdaa5a316, 0x45b2d329}},
        {"zzz/sfx/explosion/huge",
         {516, 11025, 1196723, 66678},
         {0xaf1e0119, 0x73b6433a, 0xc154d3a0, 0x59dab9ea, 0xfb74fe10}},
        {"dev/rplasma",
         {530, 22050, 68036, 16128},
         {0x90c38cfc, 0x1965c1c7, 0x936729a9, 0x1fe76215, 0xdd7a0f3c}},
        {"dev/repulser",
         {531, 22050, 84210, 10553},
         {0x8463f15d, 0x3be17c70, 0xce209796, 0xb69c95ef, 0x4b6f4f23}},
        {"sfx/explosion/gas",
         {532, 22050, 94809, 25268},
         {0xb6dad1e2, 0x249c1bbe, 0x467c0be3, 0x3b26ecd9, 0x11e7216e}},
        {"gui/beep/message",
         {535, 22050, 142420, 1252},
         {0x72a8e5c7, 0xb5db2587, 0x19ca8927, 0x940f57d9, 0x38955afd}},
        {"sfx/explosion/flak",
         {536, 22050, 143718, 15616},
         {0x2bd9ded2, 0xd03f682c, 0x8a8e37ae, 0xdbd12d66, 0xef2229d8}},
        {"dev/onas",
         {540, 22050, 726536, 16535},
         {0xe7dbe00e, 0x2dcab111, 0x133bf193, 0xd63c5744, 0xbb85b0c1}},
        {"zrb/destroy",
         {548, 11025, 1428461, 16768},
         {0x1479096a, 0xb0f32be0, 0x3998af62, 0x5b5d9a53, 0x525a5858}},
        {"dev/conc",
         {550, 22050, 159380, 6470},
         {0xb439f0d6, 0xe32c0a86, 0xab49b700, 0xd592083c, 0xf0667114}},
        {"dev/conc/ricochet/1",
         {552, 22050, 1524780, 9792},
         {0x4d37321a, 0x3503b96d, 0x1540adf6, 0xb33c1c36, 0xc15af67d}},
        {"dev/conc/ricochet/2",
         {553, 22050, 1534618, 10176},
         {0x812ea3f1, 0xe73ae797, 0xe5141ce1, 0xede5dd36, 0xeb62f862}},
        {"zzz/lectrocute",
         {562, 11025, 1621564, 18979},
         {0x0246bf0f, 0x12c38a6c, 0xf1ad2662, 0x8dffa3c5, 0x41c7bc6a}},
        {"sfx/pop",
         {571, 22050, 2985648, 2730},
         {0x88d47d4d, 0x759b3084, 0x0259bb8d, 0xc3169b82, 0x354b5f30}},
        {"zzz/transport/land/2",
         {2000, 22050, 986014, 100663},
         {0xabbc1783, 0x08a92270, 0xfbaeebcc, 0xe3c73ae5, 0xf6f44979}},
        {"zzz/transport/land/3",
         {2001, 11025, 1086723, 50332},
         {0x7a9db74e, 0xc4f1602c, 0x8a9a509d, 0xe9af8711, 0xdbe28c07}},
        {"tpl/evat/destroy",
         {12558, 11025, 1445275, 12160},
         {0xa79d0450, 0xc19b91f0, 0x5ca71911, 0x16b01e65, 0xc2594f6b}},
        {"ish/etc/tug/attach",
         {17406, 22050, 186727, 14880},
         {0xd72524cd, 0x1dbc1c26, 0x37655ed9, 0x8e76aead, 0xde4dce91}},
        {"dev/pk/sal",
         {28007, 22050, 263487, 26624},
         {0x7c430c15, 0xd3899d1b, 0x0b7ce3a4, 0x8cc0fc7d, 0x8d73e59c}},
        {"tpl/evat/board",
         {31989, 22050, 201653, 31360},
         {0xa7aa6c8f, 0xe3147850, 0x374beda8, 0x45630aa2, 0xd0d8f9d7}},
};

// Write `d` as an IEEE 754 80-bit floating point (extended precision) number.
//
// Not verified for special values (positive and negative zero and infinity, NaN) correctly,
// because those aren't valid sample rates, and that's the reason this function was written.
//
// @param [out] out     The pn::file_view to write the value to.
// @param [in] d        A double-precision floating point value.
pn::file_view write_float80(pn::file_view out, double d) {
    // Reinterpret `d` as uint64_t, since we will be manupulating bits.
    uint64_t sample_rate_bits;
    memcpy(&sample_rate_bits, &d, sizeof(uint64_t));

    // Deconstruct the double:
    //   * the sign, 1 bit.
    //   * the exponent, 11 bits with a bias of 1023.
    //   * the fraction, 52 bits, with an implicit '1' bit preceding.
    uint64_t sign     = sample_rate_bits >> 63;
    uint64_t exponent = ((sample_rate_bits >> 52) & ((1 << 11) - 1)) - 1023;
    uint64_t fraction = (1ull << 52) | (sample_rate_bits & ((1ull << 52) - 1));

    // Reconstruct the 80-bit float:
    //   * the sign, 1 bit.
    //   * the exponent, 15 bits with a bias of 16383.
    //   * the fraction, 64 bits, with no implicit '1' bit.
    return out.write<uint16_t, uint64_t>(
            (sign << 15) | ((exponent + 16383) & 0x7FFF), fraction << 11);
}

pn::data convert_snd(const SoundInfo& info, pn::data_view data) {
    pn::data samples = data.slice(info.input.offset, info.input.size).copy();
    for (int i = 0; i < samples.size(); ++i) {
        samples[i] ^= 0x80;
    }

    static const pn::string_view form = "FORM";
    static const pn::string_view aiff = "AIFF";
    static const pn::string_view comm = "COMM";
    static const pn::string_view ssnd = "SSND";

    uint16_t channels    = 1;
    uint32_t data_size   = samples.size();
    uint32_t ssnd_size   = data_size + 8;
    uint32_t form_size   = data_size + 46;
    uint32_t comm_size   = 18;
    uint64_t zeros       = 0;
    uint16_t sample_size = 8;
    pn::data sample_rate;
    write_float80(sample_rate.open("w"), info.input.sample_rate).check();

    pn::data out;
    pn::file f = out.open("w").check();
    f.write(form, form_size, aiff, comm, comm_size, channels, data_size, sample_size, sample_rate,
            ssnd, ssnd_size, zeros, samples)
            .check();
    return out;
}

static const char kDownloadBase[] = "http://downloads.arescentral.org";
static int64_t    kVersion        = 21;

static const char kPluginInfoFile[] = "info.pn";

Info info_for_zip_archive(ZipArchive& archive) {
    ZipFileReader file{archive, kPluginInfoFile};
    try {
        pn::value  x;
        pn_error_t e;
        if (!pn::parse(file.data().open(), &x, &e)) {
            throw std::runtime_error(
                    pn::format("{0}:{1}: {2}", e.lineno, e.column, pn_strerror(e.code)).c_str());
        }
        return info(path_value{x});
    } catch (...) {
        std::throw_with_nested(std::runtime_error(archive.path().copy().c_str()));
    }
}

void check_version(const Info& archive, int64_t expected) {
    int64_t actual = archive.format;
    if (actual != expected) {
        throw std::runtime_error(
                pn::format("unsupported plugin version {0}", pn::dump(actual, pn::dump_short))
                        .c_str());
    }
}

void check_identifier(const Info& archive, pn::string_view expected) {
    if (archive.identifier.hash != expected) {
        throw std::runtime_error(pn::format(
                                         "mismatch in plugin identifier {0}",
                                         pn::dump(archive.identifier.hash, pn::dump_short))
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
        Info       info = info_for_zip_archive(archive);
        check_version(info, kVersion);
        found_scenario = info.identifier.hash.copy();
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
    }
}

void DataExtractor::extract_plugin_scenario(Observer* observer) const {
    if ((_scenario != kFactoryScenarioIdentifier) && !scenario_current(_scenario)) {
        pn::string scenario_dir = pn::format("{0}/{1}", _output_dir, _scenario);
        rmtree(scenario_dir);
        extract_plugin(observer);
    }
}

bool DataExtractor::scenario_current(pn::string_view scenario) const {
    return sfz::path::isdir(pn::format("{0}/{1}", _output_dir, scenario));
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

void DataExtractor::extract_original(Observer* observer, pn::string_view file) const {
    pn::string status = pn::format("Extracting {0}...", file);
    observer->status(status);
    pn::string full_path = pn::format("{0}/{1}", _downloads_dir, file);
    ZipArchive archive(full_path, 0);

    ZipFileReader zip(archive, kAresSounds);
    for (const SoundInfo& info : kNonFreeSounds) {
        pn::data data = convert_snd(info, zip.data());

        sha1 sha;
        sha.write(data);
        if (sha.compute() != info.digest) {
            throw std::runtime_error(pn::format("sound {0}: digest mismatch", info.name).c_str());
        }

        pn::string output = pn::format(
                "{0}/{1}/sounds/{2}.aiff", _output_dir, kFactoryScenarioIdentifier, info.name);
        makedirs(path::dirname(output), 0755);
        pn::file file = pn::open(output, "w");
        file.write(data);
    }
}

void DataExtractor::extract_plugin(Observer* observer) const {
    pn::string file   = pn::format("{0}.antaresplugin", _scenario);
    pn::string status = pn::format("Extracting {0}...", file);
    observer->status(status);
    pn::string full_path = pn::format("{0}/{1}", _downloads_dir, file);
    ZipArchive archive(full_path, 0);
    Info       info = info_for_zip_archive(archive);

    check_version(info, kVersion);
    check_identifier(info, _scenario);

    for (size_t i : range(archive.size())) {
        ZipFileReader   file(archive, i);
        pn::string_view in_path = file.path();

        // Skip directories and identifier file.
        if (in_path.rfind("/") == (in_path.size() - 1)) {
            continue;
        }

        pn::string output_path = pn::format("{0}/{1}/{2}", _output_dir, _scenario, in_path);
        makedirs(path::dirname(output_path), 0755);
        pn::open(output_path, "w").write(file.data()).check();
    }
}

}  // namespace antares
