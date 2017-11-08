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

#include "sound/openal-driver.hpp"

#include <libmodplug/modplug.h>
#include <sfz/sfz.hpp>

#include "data/resource.hpp"
#include "sound/sndfile.hpp"

using sfz::Bytes;
using sfz::BytesSlice;
using sfz::Exception;
using sfz::PrintItem;
using sfz::String;
using sfz::StringSlice;
using sfz::format;
using sfz::quote;
using std::unique_ptr;

namespace antares {

namespace {

const char* al_error_to_string(int error) {
    switch (error) {
        case AL_NO_ERROR: return "AL_NO_ERROR";
        case AL_INVALID_NAME: return "AL_INVALID_NAME";
        case AL_INVALID_ENUM: return "AL_INVALID_ENUM";
        case AL_INVALID_VALUE: return "AL_INVALID_VALUE";
        case AL_INVALID_OPERATION: return "AL_INVALID_OPERATION";
        case AL_OUT_OF_MEMORY: return "AL_OUT_OF_MEMORY";
        default: return "unknown OpenAL error";
    }
}

void check_al_error(const StringSlice& method) {
    int error = alGetError();
    if (error != AL_NO_ERROR) {
        throw Exception(format("{0}: {1}", method, al_error_to_string(error)));
    }
}

class ModPlugFile {
  public:
    ModPlugFile(BytesSlice data) {
        ModPlug_Settings settings;
        ModPlug_GetSettings(&settings);
        settings.mFlags          = MODPLUG_ENABLE_OVERSAMPLING;
        settings.mChannels       = 2;
        settings.mBits           = 16;
        settings.mFrequency      = 44100;
        settings.mResamplingMode = MODPLUG_RESAMPLE_LINEAR;
        ModPlug_SetSettings(&settings);
        file = ModPlug_Load(data.data(), data.size());
    }

    ModPlugFile(const ModPlugFile&) = delete;

    ~ModPlugFile() {
        if (file) {
            ModPlug_Unload(file);
        }
    }

    void convert(Bytes& data, ALenum& format, ALsizei& frequency) const {
        format    = AL_FORMAT_STEREO16;
        frequency = 44100;
        uint8_t buffer[1024];
        ssize_t read;
        do {
            read = ModPlug_Read(file, buffer, 1024);
            data.push(BytesSlice(buffer, read));
        } while (read > 0);
    }

  private:
    ::ModPlugFile* file;
};

}  // namespace

class OpenAlSoundDriver::OpenAlSound : public Sound {
  public:
    OpenAlSound(const OpenAlSoundDriver& driver) : _driver(driver), _buffer(generate_buffer()) {}

    ~OpenAlSound() {
        alDeleteBuffers(1, &_buffer);
        alGetError();  // discard.
    }

    virtual void play();
    virtual void loop();

    template <typename T>
    void buffer(const T& file) {
        Bytes   data;
        ALenum  format;
        ALsizei frequency;
        file.convert(data, format, frequency);
        alBufferData(_buffer, format, data.data(), data.size(), frequency);
        check_al_error("alBufferData");
    }

    ALuint buffer() const { return _buffer; }

  private:
    static ALuint generate_buffer() {
        ALuint buffer;
        alGenBuffers(1, &buffer);
        check_al_error("alGenBuffers");
        return buffer;
    }

    const OpenAlSoundDriver& _driver;
    ALuint                   _buffer;

    DISALLOW_COPY_AND_ASSIGN(OpenAlSound);
};

class OpenAlSoundDriver::OpenAlChannel : public SoundChannel {
  public:
    OpenAlChannel(OpenAlSoundDriver& driver) : _driver(driver) {
        alGenSources(1, &_source);
        check_al_error("alGenSources");
        alSourcef(_source, AL_PITCH, 1.0f);
        alSourcef(_source, AL_GAIN, 1.0f);
        check_al_error("alSourcef");
    }

    ~OpenAlChannel() {
        alDeleteSources(1, &_source);
        alGetError();  // discard.
    }

    virtual void activate() { _driver._active_channel = this; }

    void play(const OpenAlSound& sound) {
        alSourcei(_source, AL_LOOPING, AL_FALSE);
        check_al_error("alSourcei");
        alSourcei(_source, AL_BUFFER, sound.buffer());
        check_al_error("alSourcei");
        alSourcePlay(_source);
        check_al_error("alSourcePlay");
    }

    void loop(const OpenAlSound& sound) {
        alSourcei(_source, AL_LOOPING, AL_TRUE);
        check_al_error("alSourcei");
        alSourcei(_source, AL_BUFFER, sound.buffer());
        check_al_error("alSourcei");
        alSourcePlay(_source);
        check_al_error("alSourcePlay");
    }

    virtual void amp(uint8_t volume) {
        alSourcef(_source, AL_GAIN, volume / 256.0f);
        check_al_error("alSourcef");
    }

    virtual void quiet() {
        alSourceStop(_source);
        check_al_error("alSourceStop");
    }

  private:
    OpenAlSoundDriver& _driver;
    ALuint             _source;

    DISALLOW_COPY_AND_ASSIGN(OpenAlChannel);
};

void OpenAlSoundDriver::OpenAlSound::play() {
    _driver._active_channel->play(*this);
}

void OpenAlSoundDriver::OpenAlSound::loop() {
    _driver._active_channel->loop(*this);
}

OpenAlSoundDriver::OpenAlSoundDriver() : _active_channel(NULL) {
    // TODO(sfiera): error-checking.
    _device  = alcOpenDevice(NULL);
    _context = alcCreateContext(_device, NULL);
    alcMakeContextCurrent(_context);
}

OpenAlSoundDriver::~OpenAlSoundDriver() {
    alcDestroyContext(_context);
    alcCloseDevice(_device);
}

unique_ptr<SoundChannel> OpenAlSoundDriver::open_channel() {
    return unique_ptr<SoundChannel>(new OpenAlChannel(*this));
}

template <typename T>
void OpenAlSoundDriver::read_sound(BytesSlice data, OpenAlSound& sound) {
    T file(data);
    sound.buffer(file);
}

unique_ptr<Sound> OpenAlSoundDriver::open_sound(PrintItem path) {
    static const struct {
        const char ext[6];
        void (*fn)(BytesSlice, OpenAlSound&);
    } fmts[] = {
            {".aiff", read_sound<Sndfile>},
            {".s3m", read_sound<ModPlugFile>},
            {".xm", read_sound<ModPlugFile>},
    };

    String                  path_string(path);
    unique_ptr<OpenAlSound> sound(new OpenAlSound(*this));
    for (const auto& fmt : fmts) {
        try {
            Resource rsrc(format("{0}{1}", path_string, fmt.ext));
            fmt.fn(rsrc.data(), *sound);
            return std::move(sound);
        } catch (Exception& e) {
            continue;
        }
    }
    throw Exception(format("couldn't load sound {0}", quote(path_string)));
}

void OpenAlSoundDriver::set_global_volume(uint8_t volume) {
    alListenerf(AL_GAIN, volume / 8.0);
}

}  // namespace antares
