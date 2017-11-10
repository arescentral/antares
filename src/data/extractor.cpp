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
#include "data/pn.hpp"
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
using sfz::Bytes;
using sfz::BytesSlice;
using sfz::Exception;
using sfz::MappedFile;
using sfz::ScopedFd;
using sfz::Sha1;
using sfz::StringMap;
using sfz::StringSlice;
using sfz::WriteTarget;
using sfz::dec;
using sfz::format;
using sfz::makedirs;
using sfz::quote;
using sfz::range;
using sfz::read;
using sfz::tree_digest;
using sfz::write;
using std::set;
using std::unique_ptr;
using std::vector;
using zipxx::ZipArchive;
using zipxx::ZipFileReader;

namespace path = sfz::path;
namespace utf8 = sfz::utf8;

namespace antares {

namespace {

bool verbatim(pn::string_view, bool, int16_t, BytesSlice data, WriteTarget out) {
    write(out, data);
    return true;
}

enum {
    THE_STARS_HAVE_EARS    = 600,
    WHILE_THE_IRON_IS_HOT  = 605,
    SPACE_RACE_THE_MUSICAL = 623,
};

int32_t nlrp_chapter(int16_t id) {
    switch (id) {
        case THE_STARS_HAVE_EARS: return 4;
        case WHILE_THE_IRON_IS_HOT: return 6;
        case SPACE_RACE_THE_MUSICAL: return 26;
    };
    throw Exception(format("invalid replay ID {0}", id));
}

bool convert_nlrp(pn::string_view, bool, int16_t id, BytesSlice data, WriteTarget out) {
    ReplayData replay;
    replay.scenario.identifier = kFactoryScenarioIdentifier;
    replay.scenario.version    = "1.1.1";
    replay.chapter_id          = nlrp_chapter(id);
    read(data, replay.global_seed);

    uint32_t keys = 0;
    uint32_t at   = 0;
    while (!data.empty()) {
        const uint32_t ticks     = read<uint32_t>(data) + 1;
        const uint32_t new_keys  = read<uint32_t>(data);
        const uint32_t keys_down = new_keys & ~keys;
        const uint32_t keys_up   = keys & ~new_keys;

        for (int i = 0; i < 19; ++i) {
            if (keys_down & (1 << i)) {
                replay.key_down(at, i);
            }
            if (keys_up & (1 << i)) {
                replay.key_up(at, i);
            }
        }
        at += ticks;

        keys = new_keys;
    }
    replay.duration = at;

    write(out, replay);
    return true;
}

bool convert_pict(pn::string_view, bool, int16_t, BytesSlice data, WriteTarget out) {
    rezin::Picture pict(data);
    if ((pict.version() == 2) && (pict.is_raster())) {
        write(out, png(pict));
        return true;
    }
    return false;
}

static pn::array pn_str(const StringList& str_list) {
    pn::array a;
    for (const std::shared_ptr<const sfz::String>& s : str_list.strings) {
        a.push_back(sfz2pn(*s));
    }
    return a;
}

bool convert_str(pn::string_view, bool, int16_t, BytesSlice data, WriteTarget out) {
    Options    options;
    StringList list(data, options);
    pn::string string = pn::dump(pn::value{pn_str(list)});
    write(out, string.data(), string.size());
    return true;
}

bool convert_text(pn::string_view, bool, int16_t, BytesSlice data, WriteTarget out) {
    Options    options;
    pn::string string = sfz2pn(sfz::String(options.decode(data)));
    write(out, string.data(), string.size());
    return true;
}

bool convert_snd(pn::string_view, bool, int16_t, BytesSlice data, WriteTarget out) {
    Sound snd(data);
    write(out, aiff(snd));
    return true;
}

void convert_frame(pn::string_view dir, int16_t id, BytesSlice data, PixMap::View pix) {
    auto width  = pix.size().width;
    auto height = pix.size().height;

    pix.fill(RgbColor::clear());
    for (auto y : range(height)) {
        for (auto x : range(width)) {
            uint8_t byte = read<uint8_t>(data);
            if (byte) {
                pix.set(x, y, RgbColor::at(byte));
            }
        }
    }
}

void convert_overlay(pn::string_view dir, int16_t id, BytesSlice data, PixMap::View pix) {
    auto width  = pix.size().width;
    auto height = pix.size().height;

    int white_count = 0;
    int pixel_count = 0;
    for (auto b : data) {
        if (b) {
            ++pixel_count;
            if (b < 0x10) {
                ++white_count;
            }
        }
    }

    // If more than 1/3 of the opaque pixels in the frame are in the
    // 'white' band of the color table, then colorize all opaque
    // (non-0x00) pixels.  Otherwise, only colors pixels which are
    // opaque and outside of the white band (0x01..0x0F).
    const uint8_t color_mask = (white_count > (pixel_count / 3)) ? 0xFF : 0xF0;
    for (auto y : range(height)) {
        for (auto x : range(width)) {
            uint8_t byte = read<uint8_t>(data);
            if (byte & color_mask) {
                byte          = (byte & 0x0f) | 0x10;
                uint8_t value = RgbColor::at(byte).red;
                pix.set(x, y, rgb(value, 0, 0));
            } else {
                pix.set(x, y, RgbColor::clear());
            }
        }
    }
}

static pn::map pn_rect(const Rect& r) {
    pn::map x;
    x["left"]   = r.left;
    x["top"]    = r.top;
    x["right"]  = r.right;
    x["bottom"] = r.bottom;
    return x;
}

static pn::map pn_point(const Point& p) {
    pn::map x;
    x["x"] = p.h;
    x["y"] = p.v;
    return x;
}

void alphatize(ArrayPixMap& image) {
    // Find the brightest pixel in the image.
    using std::max;
    uint8_t brightest = 0;
    for (int y : range(image.size().height)) {
        for (int x : range(image.size().width)) {
            auto p    = image.get(x, y);
            brightest = max(max(brightest, p.red), max(p.green, p.blue));
        }
    }

    if (!brightest) {
        return;
    }

    for (int y : range(image.size().height)) {
        for (int x : range(image.size().width)) {
            auto p = image.get(x, y);
            if (p.alpha) {
                // Brightest pixel has alpha of 255.
                p.alpha = max(max(p.red, p.green), p.blue);
                p.alpha = (p.alpha * 255) / brightest;
                if (p.alpha) {
                    // Un-premultiply alpha.
                    p.red   = (p.red * 255) / p.alpha;
                    p.green = (p.green * 255) / p.alpha;
                    p.blue  = (p.blue * 255) / p.alpha;
                }
            }
            image.set(x, y, p);
        }
    }
}

bool convert_smiv(
        pn::string_view dir, bool factory, int16_t id, BytesSlice data, WriteTarget out) {
    BytesSlice header = data;
    header.shift(4);
    uint32_t         size = read<uint32_t>(header);
    vector<uint32_t> offsets;
    vector<Rect>     bounds;

    pn::array frames;
    for (auto i : range(size)) {
        static_cast<void>(i);
        uint32_t   offset     = read<uint32_t>(header);
        BytesSlice frame_data = data.slice(offset);
        auto       width      = read<uint16_t>(frame_data);
        auto       height     = read<uint16_t>(frame_data);
        auto       x_offset   = -read<int16_t>(frame_data);
        auto       y_offset   = -read<int16_t>(frame_data);
        offsets.push_back(offset);
        bounds.emplace_back(Point(x_offset, y_offset), Size(width, height));

        frames.push_back(pn_rect(bounds.back()));
    }

    Rect max_bounds = bounds[0];
    for (const auto& rect : bounds) {
        max_bounds.left   = std::min(max_bounds.left, rect.left);
        max_bounds.top    = std::min(max_bounds.top, rect.top);
        max_bounds.right  = std::max(max_bounds.right, rect.right);
        max_bounds.bottom = std::max(max_bounds.bottom, rect.bottom);
    }

    // Do our best to find a square layout of all the frames.  In the
    // event of a prime number of frames, we'll end up with one row and
    // many columns.
    int rows = sqrt(size);
    int cols = size;
    while (rows > 1) {
        if ((size % rows) == 0) {
            cols = size / rows;
            break;
        } else {
            --rows;
        }
    }

    ArrayPixMap image(max_bounds.width() * cols, max_bounds.height() * rows);
    ArrayPixMap overlay(max_bounds.width() * cols, max_bounds.height() * rows);
    image.fill(RgbColor::clear());
    overlay.fill(RgbColor::clear());
    for (auto i : range(size)) {
        int col = i % cols;
        int row = i / cols;

        pn::map    frame;
        BytesSlice frame_data = data.slice(offsets[i]);
        frame_data.shift(8);
        Rect r = bounds[i];
        r.offset(max_bounds.width() * col, max_bounds.height() * row);
        r.offset(-max_bounds.left, -max_bounds.top);

        convert_frame(dir, id, frame_data.slice(0, r.area()), image.view(r));
        convert_overlay(dir, id, frame_data.slice(0, r.area()), overlay.view(r));
    }

    set<int> alpha_sprites = {
            504, 516, 517, 519, 522, 526, 541, 547, 548, 554, 557, 558, 580, 581,
            584, 589, 598, 605, 606, 611, 613, 614, 620, 645, 653, 655, 656,
    };
    if (factory && (alpha_sprites.find(id) != alpha_sprites.end())) {
        alphatize(image);
    }

    pn::string sprite_dir = pn::format("{0}/{1}", dir, id);
    makedirs(pn2sfz(sprite_dir), 0755);

    {
        pn::string output = pn::format("{0}/{1}/image.png", dir, id);
        ScopedFd   fd(open(pn2sfz(output), O_WRONLY | O_CREAT | O_TRUNC, 0644));
        write(fd, Bytes(image));
    }

    {
        pn::string output = pn::format("{0}/{1}/overlay.png", dir, id);
        ScopedFd   fd(open(pn2sfz(output), O_WRONLY | O_CREAT | O_TRUNC, 0644));
        write(fd, Bytes(overlay));
    }

    pn::string string(
            pn::dump(pn::map{{"image", pn::format("sprites/{0}/image.png", id)},
                             {"overlay", pn::format("sprites/{0}/overlay.png", id)},
                             {"rows", rows},
                             {"cols", cols},
                             {"center", pn_point(Point(-max_bounds.left, -max_bounds.top))},
                             {"frames", std::move(frames)}}));
    write(out, string.data(), string.size());
    return true;
}

struct ResourceFile {
    const char* path;
    struct ExtractedResource {
        const char* resource;
        const char* output_directory;
        const char* output_extension;
        bool (*convert)(
                pn::string_view dir, bool factory, int16_t id, BytesSlice data, WriteTarget out);
    } resources[16];
};

static const ResourceFile kResourceFiles[] = {
        {
                "__MACOSX/Ares 1.2.0 ƒ/Ares Data ƒ/._Ares Scenarios",
                {
                        {"PICT", "pictures", "png", convert_pict},
                        {"STR#", "strings", "pn", convert_str},
                        {"TEXT", "text", "txt", convert_text},
                        {"bsob", "objects", "bsob", verbatim},
                        {"nlAG", "scenario-info", "nlAG", verbatim},
                        {"obac", "object-actions", "obac", verbatim},
                        {"race", "races", "race", verbatim},
                        {"snbf", "scenario-briefing-points", "snbf", verbatim},
                        {"sncd", "scenario-conditions", "sncd", verbatim},
                        {"snit", "scenario-initial-objects", "snit", verbatim},
                        {"snro", "scenarios", "snro", verbatim},
                },
        },
        {
                "__MACOSX/Ares 1.2.0 ƒ/Ares Data ƒ/._Ares Sounds",
                {
                        {"snd ", "sounds", "aiff", convert_snd},
                },
        },
        {
                "__MACOSX/Ares 1.2.0 ƒ/Ares Data ƒ/._Ares Sprites",
                {
                        {"SMIV", "sprites", "pn", convert_smiv},
                },
        },
        {
                "__MACOSX/Ares 1.2.0 ƒ/._Ares",
                {
                        {"NLRP", "replays", "NLRP", convert_nlrp},
                },
        },
};

static const ResourceFile::ExtractedResource kPluginFiles[] = {
        {"PICT", "pictures", "png", convert_pict},
        {"NLRP", "replays", "NLRP", convert_nlrp},
        {"SMIV", "sprites", "pn", convert_smiv},
        {"STR#", "strings", "pn", convert_str},
        {"TEXT", "text", "txt", convert_text},
        {"bsob", "objects", "bsob", verbatim},
        {"nlAG", "scenario-info", "nlAG", verbatim},
        {"obac", "object-actions", "obac", verbatim},
        {"race", "races", "race", verbatim},
        {"snbf", "scenario-briefing-points", "snbf", verbatim},
        {"sncd", "scenario-conditions", "sncd", verbatim},
        {"snd ", "sounds", "aiff", convert_snd},
        {"snit", "scenario-initial-objects", "snit", verbatim},
        {"snro", "scenarios", "snro", verbatim},
};

static const char kDownloadBase[] = "http://downloads.arescentral.org";
static const char kVersion[]      = "16\n";

static const char kPluginVersionFile[]    = "data/version";
static const char kPluginVersion[]        = "1\n";
static const char kPluginIdentifierFile[] = "data/identifier";

void check_version(ZipArchive& archive, pn::string_view expected) {
    ZipFileReader version_file(archive, kPluginVersionFile);
    pn::string    actual = sfz2pn(utf8::decode(version_file.data()));
    if (actual != expected) {
        throw Exception(format("unsupported plugin version {0}", quote(pn2sfz(actual))));
    }
}

void read_identifier(ZipArchive& archive, pn::string& out) {
    ZipFileReader identifier_file(archive, kPluginIdentifierFile);
    sfz::String   actual(utf8::decode(identifier_file.data()));
    if (actual.at(actual.size() - 1) != '\n') {
        throw Exception(format("missing newline in plugin identifier {0}", quote(actual)));
    }
    out = sfz2pn(actual.slice(0, actual.size() - 1));
}

void check_identifier(ZipArchive& archive, pn::string_view expected) {
    pn::string actual;
    read_identifier(archive, actual);
    if (expected != actual) {
        throw Exception(format("mismatch in plugin identifier {0}", quote(pn2sfz(actual))));
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
        ZipArchive archive(pn2sfz(path), 0);
        check_version(archive, kPluginVersion);
        read_identifier(archive, found_scenario);
    }

    // Copy it to $DOWNLOADS/$IDENTIFIER.antaresplugin.  This is where
    // extract_scenario() will expect it later.
    pn::string out_path = pn::format("{0}/{1}.antaresplugin", _downloads_dir, found_scenario);
    if (path != out_path) {
        makedirs(path::dirname(pn2sfz(out_path)), 0755);
        MappedFile file(pn2sfz(path));
        ScopedFd   fd(open(pn2sfz(out_path), O_WRONLY | O_CREAT | O_TRUNC, 0644));
        write(fd, file.data());
    }
    pn::string scenario_dir = pn::format("{0}/{1}", _output_dir, found_scenario);
    if (path::exists(pn2sfz(scenario_dir))) {
        rmtree(pn2sfz(scenario_dir));
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
                (Sha1::Digest){{0x246c393c, 0xa598af68, 0xa58cfdd1, 0x8e1601c1, 0xf4f30931}});

        pn::string scenario_dir = pn::format("{0}/{1}", _output_dir, kFactoryScenarioIdentifier);
        rmtree(pn2sfz(scenario_dir));
        extract_original(observer, "Ares-1.2.0.zip");
        write_version(kFactoryScenarioIdentifier);
    }
}

void DataExtractor::extract_plugin_scenario(Observer* observer) const {
    if ((_scenario != kFactoryScenarioIdentifier) && !scenario_current(_scenario)) {
        pn::string scenario_dir = pn::format("{0}/{1}", _output_dir, _scenario);
        rmtree(pn2sfz(scenario_dir));
        extract_plugin(observer);
        write_version(_scenario);
    }
}

bool DataExtractor::scenario_current(pn::string_view scenario) const {
    pn::string path = pn::format("{0}/{1}/version", _output_dir, scenario);
    BytesSlice version(kVersion);
    try {
        MappedFile file(pn2sfz(path));
        return file.data() == version;
    } catch (Exception& e) {
        return false;
    }
}

void DataExtractor::download(
        Observer* observer, pn::string_view base, pn::string_view name, pn::string_view version,
        const Sha1::Digest& expected_digest) const {
    pn::string full_path = pn::format("{0}/{1}-{2}.zip", _downloads_dir, name, version);

    // Don't download `file` if it has already been downloaded.  If there is a regular file at
    // `full_path` and it has the expected digest, then return without doing anything.  Otherwise,
    // delete whatever's there (if anything).
    if (path::exists(pn2sfz(full_path))) {
        if (path::isfile(pn2sfz(full_path))) {
            MappedFile file(pn2sfz(full_path));
            Sha1       sha;
            write(sha, file.data());
            if (sha.digest() == expected_digest) {
                return;
            }
        }
        rmtree(pn2sfz(full_path));
    }

    pn::string url = pn::format("{0}/{1}/{1}-{2}.zip", base, name, version);

    pn::string status = pn::format("Downloading {0}-{1}.zip...", name, version);
    observer->status(status);

    // Download the file from `url`.  Check its digest when it has been downloaded; if it is not
    // the right file, then throw an exception without writing it to disk.  Otherwise, write it to
    // disk.
    Bytes download;
    http::get(url, download);
    Sha1 sha;
    write(sha, download);
    if (sha.digest() != expected_digest) {
        throw Exception(
                format("Downloaded {0}, size={1} but it didn't have the right digest.",
                       pn2sfz(pn::dump(pn::string_view{url}, pn::dump_short)), download.size()));
    }

    // If we got the file, write it out at `full_path`.
    makedirs(path::dirname(pn2sfz(full_path)), 0755);
    ScopedFd fd(open(pn2sfz(full_path), O_WRONLY | O_CREAT | O_EXCL, 0644));
    write(fd, download.data(), download.size());
}

void DataExtractor::write_version(pn::string_view scenario_identifier) const {
    pn::string path = pn::format("{0}/{1}/version", _output_dir, scenario_identifier);
    makedirs(path::dirname(pn2sfz(path)), 0755);
    ScopedFd   fd(open(pn2sfz(path), O_WRONLY | O_CREAT | O_TRUNC, 0644));
    BytesSlice version(kVersion);
    write(fd, version);
}

void DataExtractor::extract_original(Observer* observer, pn::string_view file) const {
    pn::string status = pn::format("Extracting {0}...", file);
    observer->status(status);
    pn::string full_path = pn::format("{0}/{1}", _downloads_dir, file);
    ZipArchive archive(pn2sfz(full_path), 0);

    rezin::Options options;
    options.line_ending = rezin::Options::CR;

    for (const ResourceFile& resource_file : kResourceFiles) {
        pn::string    path = sfz2pn(utf8::decode(resource_file.path));
        ZipFileReader file(archive, pn2sfz(path));
        AppleDouble   apple_double(file.data());
        ResourceFork  rsrc(apple_double.at(AppleDouble::RESOURCE_FORK), options);

        for (const ResourceFile::ExtractedResource& conversion : resource_file.resources) {
            if (!conversion.resource) {
                continue;
            }

            const ResourceType& type = rsrc.at(conversion.resource);
            for (const ResourceEntry& entry : type) {
                Bytes      data;
                pn::string output = pn::format(
                        "{0}/{1}/{2}/{3}.{4}", _output_dir, kFactoryScenarioIdentifier,
                        conversion.output_directory, entry.id(), conversion.output_extension);
                if (conversion.convert(
                            sfz2pn(path::dirname(pn2sfz(output))), true, entry.id(), entry.data(),
                            data)) {
                    makedirs(path::dirname(pn2sfz(output)), 0755);
                    ScopedFd fd(open(pn2sfz(output), O_WRONLY | O_CREAT | O_TRUNC, 0644));
                    write(fd, data.data(), data.size());
                }
            }
        }
    }
}

void DataExtractor::extract_plugin(Observer* observer) const {
    pn::string file   = pn::format("{0}.antaresplugin", _scenario);
    pn::string status = pn::format("Extracting {0}...", file);
    observer->status(status);
    pn::string full_path = pn::format("{0}/{1}", _downloads_dir, file);
    ZipArchive archive(pn2sfz(full_path), 0);

    rezin::Options options;
    options.line_ending = rezin::Options::CR;

    check_version(archive, kPluginVersion);
    check_identifier(archive, _scenario);

    for (size_t i : range(archive.size())) {
        ZipFileReader file(archive, i);
        StringSlice   path = file.path();

        // Skip directories and special files.
        if ((path.rfind("/") == (path.size() - 1)) || (path == kPluginIdentifierFile) ||
            (path == kPluginVersionFile)) {
            continue;
        }

        // Parse path into "data/$TYPE/$ID etc.".
        StringSlice data;
        StringSlice resource_type_slice;
        StringSlice id_slice;
        int16_t     id;
        if (!partition(data, "/", path) || (data != "data") ||
            !partition(resource_type_slice, "/", path) || !partition(id_slice, " ", path) ||
            !string_to_int(id_slice, id) || (path.find('/') != StringSlice::npos)) {
            throw Exception(format("bad plugin file {0}", quote(file.path())));
        }

        sfz::String resource_type(resource_type_slice);
        if (resource_type.size() < 4) {
            resource_type.resize(4, ' ');
        }

        for (const ResourceFile::ExtractedResource& conversion : kPluginFiles) {
            if (conversion.resource == resource_type) {
                Bytes      data;
                pn::string output = pn::format(
                        "{0}/{1}/{2}/{3}.{4}", _output_dir, _scenario, conversion.output_directory,
                        id, conversion.output_extension);
                if (conversion.convert(
                            sfz2pn(path::dirname(pn2sfz(output))), false, id, file.data(), data)) {
                    makedirs(path::dirname(pn2sfz(output)), 0755);
                    ScopedFd fd(open(pn2sfz(output), O_WRONLY | O_CREAT | O_TRUNC, 0644));
                    write(fd, data.data(), data.size());
                }
                goto next;
            }
        }

        throw Exception(format("unknown resource type {0}", quote(resource_type)));

    next:;  // labeled continue.
    }
}

}  // namespace antares
