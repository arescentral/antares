// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2011 Ares Central
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
// License along with this program.  If not, see
// <http://www.gnu.org/licenses/>.

#include "data/extractor.hpp"

#include <fcntl.h>
#include <stdint.h>
#include <rezin/rezin.hpp>
#include <sfz/sfz.hpp>
#include <zipxx/zipxx.hpp>

#include "data/replay.hpp"
#include "net/http.hpp"

using rezin::AppleDouble;
using rezin::ResourceEntry;
using rezin::ResourceFork;
using rezin::ResourceType;
using rezin::Sound;
using rezin::aiff;
using sfz::Bytes;
using sfz::BytesSlice;
using sfz::Exception;
using sfz::Json;
using sfz::MappedFile;
using sfz::ScopedFd;
using sfz::Sha1;
using sfz::String;
using sfz::StringSlice;
using sfz::WriteTarget;
using sfz::format;
using sfz::makedirs;
using sfz::quote;
using sfz::read;
using sfz::range;
using sfz::scoped_ptr;
using sfz::tree_digest;
using sfz::write;
using zipxx::ZipArchive;
using zipxx::ZipFileReader;

namespace path = sfz::path;
namespace utf8 = sfz::utf8;

namespace antares {

namespace {

void verbatim(int16_t id, BytesSlice data, WriteTarget out) {
    static_cast<void>(id);
    write(out, data);
}

enum {
    THE_STARS_HAVE_EARS = 600,
    WHILE_THE_IRON_IS_HOT = 605,
    SPACE_RACE_THE_MUSICAL = 623,
};

int32_t nlrp_chapter(int16_t id) {
    switch (id) {
      case THE_STARS_HAVE_EARS:
        return 4;
      case WHILE_THE_IRON_IS_HOT:
        return 6;
      case SPACE_RACE_THE_MUSICAL:
        return 26;
    };
    throw Exception(format("invalid replay ID {0}", id));
}

void convert_nlrp(int16_t id, BytesSlice data, WriteTarget out) {
    ReplayData replay;
    replay.chapter_id = nlrp_chapter(id);
    read(data, replay.global_seed);

    int correction = 0;
    uint32_t keys = 0;
    while (!data.empty()) {
        const uint32_t ticks = read<uint32_t>(data) + correction;
        const uint32_t new_keys = read<uint32_t>(data);
        const uint32_t keys_down = new_keys & ~keys;
        const uint32_t keys_up = keys & ~new_keys;

        for (int i = 0; i < 19; ++i) {
            if (keys_down & (1 << i)) {
                replay.key_down(i);
            }
            if (keys_up & (1 << i)) {
                replay.key_up(i);
            }
        }
        replay.wait(ticks);

        correction = 1;
        keys = new_keys;
    }

    write(out, replay);
}

void convert_snd(int16_t id, BytesSlice data, WriteTarget out) {
    static_cast<void>(id);
    Sound snd(data);
    write(out, aiff(snd));
}

const struct ResourceFile {
    const char* path;
    struct ExtractedResource {
        const char* resource;
        const char* output_directory;
        const char* output_extension;
        void (*convert)(int16_t id, BytesSlice data, WriteTarget out);
    } resources[16];
} kResourceFiles[] = {
    {
        "__MACOSX/Ares 1.2.0 ƒ/Ares Data ƒ/._Ares Interfaces",
        {
            { "STR#",  "strings",            "STR#",  verbatim },
            { "TEXT",  "text",               "txt",   verbatim },
            { "intr",  "interfaces",         "intr",  verbatim },
            { "nlFD",  "font-descriptions",  "nlFD",  verbatim },
            { "nlFM",  "font-bitmaps",       "nlFM",  verbatim },
        },
    },
    {
        "__MACOSX/Ares 1.2.0 ƒ/Ares Data ƒ/._Ares Scenarios",
        {
            { "STR#",  "strings",                   "STR#",  verbatim },
            { "TEXT",  "text",                      "txt",   verbatim },
            { "bsob",  "objects",                   "bsob",  verbatim },
            { "nlAG",  "scenario-info",             "nlAG",  verbatim },
            { "obac",  "object-actions",            "obac",  verbatim },
            { "race",  "races",                     "race",  verbatim },
            { "snbf",  "scenario-briefing-points",  "snbf",  verbatim },
            { "sncd",  "scenario-conditions",       "sncd",  verbatim },
            { "snit",  "scenario-initial-objects",  "snit",  verbatim },
            { "snro",  "scenarios",                 "snro",  verbatim },
        },
    },
    {
        "__MACOSX/Ares 1.2.0 ƒ/Ares Data ƒ/._Ares Sounds",
        {
            { "snd ",  "sounds",  "aiff",  convert_snd },
        },
    },
    {
        "__MACOSX/Ares 1.2.0 ƒ/Ares Data ƒ/._Ares Sprites",
        {
            { "SMIV",  "sprites",  "SMIV",  verbatim },
        },
    },
    {
        "__MACOSX/Ares 1.2.0 ƒ/._Ares",
        {
            { "NLRP",  "replays",         "NLRP",  convert_nlrp },
            { "STR#",  "strings",         "STR#",  verbatim },
            { "TEXT",  "text",            "txt",   verbatim },
            { "rot ",  "rotation-table",  "rot ",  verbatim },
        },
    },
};

const char kDownloadBase[] = "http://downloads.arescentral.org";

}  // namespace

DataExtractor::Observer::~Observer() { }

DataExtractor::DataExtractor(const StringSlice& downloads_dir, const StringSlice& output_dir)
    : _downloads_dir(downloads_dir),
      _output_dir(output_dir) { }

bool DataExtractor::current() const {
    if (path::isdir(_output_dir)) {
        return (tree_digest(_output_dir) ==
                (Sha1::Digest){{0x527ff39c, 0xbfcf0942, 0xd8abbc6d, 0xa129b5a9, 0x148e5475}});
    }
    return false;
}

void DataExtractor::extract(Observer* observer) const {
    download(observer, kDownloadBase, "Ares", "1.2.0",
            (Sha1::Digest){{0x246c393c, 0xa598af68, 0xa58cfdd1, 0x8e1601c1, 0xf4f30931}});
    download(observer, kDownloadBase, "Antares-Music", "0.3.0",
            (Sha1::Digest){{0x9a1ceb4e, 0x2e0d4e7d, 0x61ed9934, 0x1274355e, 0xd8238bc4}});
    download(observer, kDownloadBase, "Antares-Pictures", "0.3.0",
            (Sha1::Digest){{0x2c7961df, 0xb68c1b2b, 0xafbf83b9, 0xf27a4f62, 0x13ca8189}});
    download(observer, kDownloadBase, "Antares-Text", "0.3.0",
            (Sha1::Digest){{0x2b5f3d50, 0xcc243db1, 0x35173461, 0x819f5e1b, 0xabde1519}});

    rmtree(_output_dir);
    extract_original(observer, "Ares-1.2.0.zip");
    extract_supplemental(observer, "Antares-Music-0.3.0.zip");
    extract_supplemental(observer, "Antares-Pictures-0.3.0.zip");
    extract_supplemental(observer, "Antares-Text-0.3.0.zip");
}

void DataExtractor::download(Observer* observer, const StringSlice& base, const StringSlice& name,
        const StringSlice& version, const Sha1::Digest& expected_digest) const {
    String full_path(format("{0}/{1}-{2}.zip", _downloads_dir, name, version));

    // Don't download `file` if it has already been downloaded.  If there is a regular file at
    // `full_path` and it has the expected digest, then return without doing anything.  Otherwise,
    // delete whatever's there (if anything).
    if (path::exists(full_path)) {
        if (path::isfile(full_path)) {
            MappedFile file(full_path);
            Sha1 sha;
            write(sha, file.data());
            if (sha.digest() == expected_digest) {
                return;
            }
        }
        rmtree(full_path);
    }

    String url(format("{0}/{1}/{1}-{2}.zip", base, name, version));

    String status(format("Downloading {0}-{1}.zip...", name, version));
    observer->status(status);

    // Download the file from `url`.  Check its digest when it has been downloaded; if it is not
    // the right file, then throw an exception without writing it to disk.  Otherwise, write it to
    // disk.
    Bytes download;
    http::get(url, download);
    Sha1 sha;
    write(sha, download);
    if (sha.digest() != expected_digest) {
        throw Exception(format("Downloaded {0}, size={1} but it didn't have the right digest.",
                quote(url), download.size()));
    }

    // If we got the file, write it out at `full_path`.
    makedirs(path::dirname(full_path), 0755);
    ScopedFd fd(open(full_path, O_WRONLY | O_CREAT | O_EXCL, 0644));
    write(fd, download.data(), download.size());
}

void DataExtractor::extract_original(Observer* observer, const StringSlice& file) const {
    String status(format("Extracting {0}...", file));
    observer->status(status);
    String full_path(format("{0}/{1}", _downloads_dir, file));
    ZipArchive archive(full_path, 0);

    rezin::Options options;
    options.line_ending = rezin::Options::CR;

    SFZ_FOREACH(const ResourceFile& resource_file, kResourceFiles, {
        String path(utf8::decode(resource_file.path));
        ZipFileReader file(archive, path);
        AppleDouble apple_double(file.data());
        ResourceFork rsrc(apple_double.at(AppleDouble::RESOURCE_FORK), options);

        SFZ_FOREACH(const ResourceFile::ExtractedResource& conversion, resource_file.resources, {
            if (!conversion.resource) {
                continue;
            }

            const ResourceType& type = rsrc.at(conversion.resource);
            SFZ_FOREACH(const ResourceEntry& entry, type, {
                Bytes data;
                conversion.convert(entry.id(), entry.data(), data);
                String output(format("{0}/{1}/{2}.{3}",
                            _output_dir, conversion.output_directory, entry.id(),
                            conversion.output_extension));
                makedirs(path::dirname(output), 0755);
                ScopedFd fd(open(output, O_WRONLY | O_CREAT | O_TRUNC, 0644));
                write(fd, data.data(), data.size());
            });
        });
    });
}

void DataExtractor::extract_supplemental(Observer* observer, const StringSlice& file) const {
    String status(format("Extracting {0}...", file));
    observer->status(status);
    String full_path(format("{0}/{1}", _downloads_dir, file));
    ZipArchive archive(full_path, 0);

    SFZ_FOREACH(size_t i, range(archive.size()), {
        ZipFileReader file(archive, i);

        // Ignore directories.
        if (file.path().rfind('/') == (file.path().size() - 1)) {
            continue;
        }

        // Ignore files in the root.
        size_t slash = file.path().find('/');
        if (slash == String::npos) {
            continue;
        }

        StringSlice input = file.path().slice(slash + 1);
        if ((input == "README") || (input == "COPYING")) {
            continue;
        }

        String output(format("{0}/{1}", _output_dir, file.path().slice(slash + 1)));
        makedirs(path::dirname(output), 0755);
        ScopedFd fd(open(output, O_WRONLY | O_CREAT | O_TRUNC, 0644));
        write(fd, file.data());
    });
}

}  // namespace antares
