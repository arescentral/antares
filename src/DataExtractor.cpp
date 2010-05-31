// Ares, a tactical space combat game.
// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#include "DataExtractor.hpp"

#include <fcntl.h>
#include "rezin/rezin.hpp"
#include "rgos/rgos.hpp"
#include "sfz/sfz.hpp"
#include "zipxx/zipxx.hpp"
#include "HttpDriver.hpp"

using rezin::AppleDouble;
using rezin::ResourceFork;
using rezin::ResourceType;
using rezin::read_snd;
using rezin::write_aiff;
using rgos::Json;
using sfz::Bytes;
using sfz::BytesPiece;
using sfz::Exception;
using sfz::MappedFile;
using sfz::PrintTarget;
using sfz::ScopedFd;
using sfz::Sha1;
using sfz::String;
using sfz::StringPiece;
using sfz::WriteTarget;
using sfz::array_range;
using sfz::format;
using sfz::makedirs;
using sfz::posix_strerror;
using sfz::quote;
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

void verbatim(const BytesPiece& data, WriteTarget out) {
    write(out, data);
}

void convert_snd(const BytesPiece& data, WriteTarget out) {
    Json snd = read_snd(data);
    Bytes bytes;
    write_aiff(snd, &bytes);
    write(out, data);
}


const struct ResourceFile {
    const char* path;
    struct ExtractedResource {
        const char* resource;
        const char* output_directory;
        const char* output_extension;
        void (*convert)(const BytesPiece& data, WriteTarget out);
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
            { "NLRP",  "replays",         "NLRP",  verbatim },
            { "STR#",  "strings",         "STR#",  verbatim },
            { "TEXT",  "text",            "txt",   verbatim },
            { "rot ",  "rotation-table",  "rot ",  verbatim },
        },
    },
};

const char kOriginalDownloadBase[] = "http://antares.sfiera.net/downloads";
const char kSupplementalDownloadBase[] = "http://github.com/downloads/sfiera/antares-data";

}  // namespace

DataExtractor::DataExtractor(const StringPiece& downloads_dir, const StringPiece& output_dir)
    : _downloads_dir(downloads_dir),
      _output_dir(output_dir) { }

bool DataExtractor::current() const {
    if (path::isdir(_output_dir)) {
        return (tree_digest(_output_dir) ==
                (Sha1::Digest){{0xc81b97ec, 0xebcf7db2, 0x29176dbb, 0x29073d03, 0xd7bc12e7}});
    }
    return false;
}

void DataExtractor::extract(PrintTarget status) const {
    download(status, kOriginalDownloadBase, "Ares-1.2.0.zip",
            (Sha1::Digest){{0x246c393c, 0xa598af68, 0xa58cfdd1, 0x8e1601c1, 0xf4f30931}});
    download(status, kSupplementalDownloadBase, "Antares-Music-0.3.0.zip",
            (Sha1::Digest){{0x9a1ceb4e, 0x2e0d4e7d, 0x61ed9934, 0x1274355e, 0xd8238bc4}});
    download(status, kSupplementalDownloadBase, "Antares-Pictures-0.3.0.zip",
            (Sha1::Digest){{0x2c7961df, 0xb68c1b2b, 0xafbf83b9, 0xf27a4f62, 0x13ca8189}});
    download(status, kSupplementalDownloadBase, "Antares-Text-0.3.0.zip",
            (Sha1::Digest){{0x2b5f3d50, 0xcc243db1, 0x35173461, 0x819f5e1b, 0xabde1519}});

    rmtree(_output_dir);
    extract_original(status, "Ares-1.2.0.zip");
    extract_supplemental(status, "Antares-Music-0.3.0.zip");
    extract_supplemental(status, "Antares-Pictures-0.3.0.zip");
    extract_supplemental(status, "Antares-Text-0.3.0.zip");
}

void DataExtractor::download(PrintTarget status, const StringPiece& base, const StringPiece& file,
        const Sha1::Digest& expected_digest) const {
    String full_path(format("{0}/{1}", _downloads_dir, file));

    // Don't download `file` if it has already been downloaded.  If there is a regular file at
    // `full_path` and it has the expected digest, then return without doing anything.  Otherwise,
    // delete whatever's there (if anything).
    if (path::exists(full_path)) {
        if (path::isfile(full_path)) {
            MappedFile file(full_path);
            Sha1 sha;
            sha.append(file.data());
            if (sha.digest() == expected_digest) {
                return;
            }
        }
        rmtree(full_path);
    }

    String url(format("{0}/{1}", base, file));
    print(status, format("downloading {0}\n", url));

    // Download the file from `url`.  Check its digest when it has been downloaded; if it is not
    // the right file, then throw an exception without writing it to disk.  Otherwise, write it to
    // disk.
    Bytes download;
    HttpDriver::driver()->get(url, &download);
    Sha1 sha;
    sha.append(download);
    if (sha.digest() != expected_digest) {
        throw Exception(format("Downloaded {0}, size={1} but it didn't have the right digest.",
                quote(url), download.size()));
    }

    // If we got the file, write it out at `full_path`.
    makedirs(path::dirname(full_path), 0755);
    ScopedFd fd(open(full_path, O_WRONLY | O_CREAT | O_EXCL, 0644));
    write(&fd, download.data(), download.size());
}

void DataExtractor::extract_original(PrintTarget status, const StringPiece& file) const {
    String full_path(format("{0}/{1}", _downloads_dir, file));
    ZipArchive archive(full_path, 0);
    foreach (it, array_range(kResourceFiles)) {
        String path(utf8::decode(it->path));
        print(status, format("extracting data from {0},{1}\n", file, path));
        ZipFileReader file(&archive, path);
        AppleDouble apple_double(file.data());
        ResourceFork rsrc(apple_double.at(AppleDouble::RESOURCE_FORK));

        foreach (it, array_range(it->resources)) {
            if (!it->resource) {
                continue;
            }

            const ResourceType& type = rsrc.at(it->resource);
            foreach (jt, type) {
                Bytes data;
                it->convert(jt->data(), &data);
                String output(format("{0}/{1}/{2}.{3}",
                            _output_dir, it->output_directory, jt->id(), it->output_extension));
                makedirs(path::dirname(output), 0755);
                ScopedFd fd(open(output, O_WRONLY | O_CREAT | O_TRUNC, 0644));
                write(&fd, data.data(), data.size());
            }
        }
    }
}

void DataExtractor::extract_supplemental(PrintTarget status, const StringPiece& file) const {
    print(status, format("extracting data from {0}\n", file));
    String full_path(format("{0}/{1}", _downloads_dir, file));
    ZipArchive archive(full_path, 0);

    foreach (i, range(archive.size())) {
        ZipFileReader file(&archive, i);

        // Ignore directories.
        if (file.path().rfind('/') == (file.path().size() - 1)) {
            continue;
        }

        // Ignore files in the root.
        size_t slash = file.path().find('/');
        if (slash == String::npos) {
            continue;
        }

        StringPiece input = file.path().substr(slash + 1);
        if ((input == "README") || (input == "COPYING")) {
            continue;
        }

        String output(format("{0}/{1}", _output_dir, file.path().substr(slash + 1)));
        makedirs(path::dirname(output), 0755);
        ScopedFd fd(open(output, O_WRONLY | O_CREAT | O_TRUNC, 0644));
        write(&fd, file.data());
    }
}

}  // namespace antares
