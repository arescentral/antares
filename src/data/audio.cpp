// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2015-2017 The Antares Authors
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

#include "data/audio.hpp"

#include <libmodplug/modplug.h>
#include <sndfile.h>
#include <string.h>
#include <memory>
#include <pn/file>

namespace antares {

namespace sndfile {

namespace {

struct VirtualFile {
    pn::data_view data;
    size_t        pointer;

    sf_count_t get_filelen() { return data.size(); }

    sf_count_t seek(sf_count_t offset, int whence) {
        switch (whence) {
            case SEEK_CUR:
                if (offset < 0) {
                    if (-offset > pointer) {
                        return pointer = 0;
                    }
                } else {
                    if (offset > (data.size() - pointer)) {
                        return pointer = data.size();
                    }
                }
                return pointer += offset;
            case SEEK_SET:
                if (offset < 0) {
                    return pointer = 0;
                } else if (offset > data.size()) {
                    return pointer = data.size();
                }
                return pointer = offset;
            case SEEK_END:
                if (offset >= 0) {
                    return pointer = data.size();
                } else if (-offset > data.size()) {
                    return pointer = 0;
                }
                return pointer = data.size() + offset;
        }
        return pointer;
    }

    sf_count_t read(void* ptr, sf_count_t count) {
        if ((data.size() - pointer) < count) {
            count = data.size() - pointer;
        }
        memcpy(ptr, data.data() + pointer, count);
        pointer += count;
        return count;
    }

    sf_count_t write(const void* ptr, sf_count_t count) { return -1; }

    sf_count_t tell() { return pointer; }
};

}  // namespace

static sf_count_t sf_vio_get_filelen(void* user_data) {
    return reinterpret_cast<VirtualFile*>(user_data)->get_filelen();
}

static sf_count_t sf_vio_seek(sf_count_t offset, int whence, void* user_data) {
    return reinterpret_cast<VirtualFile*>(user_data)->seek(offset, whence);
}

static sf_count_t sf_vio_read(void* ptr, sf_count_t count, void* user_data) {
    return reinterpret_cast<VirtualFile*>(user_data)->read(ptr, count);
}

static sf_count_t sf_vio_write(const void* ptr, sf_count_t count, void* user_data) {
    return reinterpret_cast<VirtualFile*>(user_data)->write(ptr, count);
}

static sf_count_t sf_vio_tell(void* user_data) {
    return reinterpret_cast<VirtualFile*>(user_data)->tell();
}

SoundData convert(pn::data_view in) {
    SF_VIRTUAL_IO io = {
            .get_filelen = sf_vio_get_filelen,
            .seek        = sf_vio_seek,
            .read        = sf_vio_read,
            .write       = sf_vio_write,
            .tell        = sf_vio_tell,
    };
    VirtualFile                                   userdata = {.data = in, .pointer = 0};
    SF_INFO                                       info     = {};
    std::unique_ptr<SNDFILE, decltype(&sf_close)> file(
            sf_open_virtual(&io, SFM_READ, &info, &userdata), sf_close);

    if (!file.get()) {
        throw std::runtime_error(sf_strerror(NULL));
    }

    if (info.channels > 2) {
        throw std::runtime_error(pn::format("audio file has {0} channels", info.channels).c_str());
    }

    SoundData s;
    s.frequency = info.samplerate;
    s.channels  = info.channels;
    int16_t shorts[1024];
    while (auto count = sf_read_short(file.get(), shorts, 1024)) {
        s.data += pn::data_view{reinterpret_cast<uint8_t*>(shorts),
                                static_cast<int>(sizeof(int16_t) * count)};
    }
    return s;
}

}  // namespace sndfile

namespace modplug {

SoundData convert(pn::data_view in) {
    ModPlug_Settings settings;
    ModPlug_GetSettings(&settings);
    settings.mFlags            = MODPLUG_ENABLE_OVERSAMPLING;
    settings.mChannels         = 2;
    settings.mBits             = 16;
    settings.mFrequency        = 44100;
    settings.mStereoSeparation = 128;
    settings.mResamplingMode   = MODPLUG_RESAMPLE_LINEAR;
    ModPlug_SetSettings(&settings);
    std::unique_ptr<::ModPlugFile, decltype(&ModPlug_Unload)> file(
            ModPlug_Load(in.data(), in.size()), ModPlug_Unload);

    SoundData s;
    s.channels  = 2;
    s.frequency = 44100;
    uint8_t buffer[1024];
    ssize_t read;
    do {
        read = ModPlug_Read(file.get(), buffer, 1024);
        s.data += pn::data_view(buffer, read);
    } while (read > 0);
    return s;
}

}  // namespace modplug

}  // namespace antares
