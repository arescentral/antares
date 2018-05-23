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

#include <pn/file>

#include "data/audio.hpp"
#include "data/resource.hpp"

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

void check_al_error(pn::string_view method) {
    int error = alGetError();
    if (error != AL_NO_ERROR) {
        throw std::runtime_error(
                pn::format("{0}: {1}", method, al_error_to_string(error)).c_str());
    }
}

}  // namespace

class OpenAlSoundDriver::OpenAlSound : public Sound {
  public:
    OpenAlSound(const OpenAlSoundDriver& driver) : _driver(driver), _buffer(generate_buffer()) {}

    ~OpenAlSound() {
        alDeleteBuffers(1, &_buffer);
        alGetError();  // discard.
    }

    virtual void play(uint8_t volume);
    virtual void loop(uint8_t volume);

    void buffer(const SoundData& s) {
        ALenum format = (s.channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
        alBufferData(_buffer, format, s.data.data(), s.data.size(), s.frequency);
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

    void activate() override { _driver._active_channel = this; }

    void play(const OpenAlSound& sound, uint8_t volume) {
        quiet();

        alSourcef(_source, AL_GAIN, volume / 255.0f);
        check_al_error("alSourcef");
        alSourcei(_source, AL_LOOPING, AL_FALSE);
        check_al_error("alSourcei");
        alSourcei(_source, AL_BUFFER, sound.buffer());
        check_al_error("alSourcei");
        alSourcePlay(_source);
        check_al_error("alSourcePlay");
    }

    void loop(const OpenAlSound& sound, uint8_t volume) {
        quiet();

        alSourcef(_source, AL_GAIN, volume / 255.0f);
        check_al_error("alSourcef");
        alSourcei(_source, AL_LOOPING, AL_TRUE);
        check_al_error("alSourcei");
        alSourcei(_source, AL_BUFFER, sound.buffer());
        check_al_error("alSourcei");
        alSourcePlay(_source);
        check_al_error("alSourcePlay");
    }

    void quiet() override {
        alSourceStop(_source);
        check_al_error("alSourceStop");
    }

  private:
    OpenAlSoundDriver& _driver;
    ALuint             _source;
};

void OpenAlSoundDriver::OpenAlSound::play(uint8_t volume) {
    _driver._active_channel->play(*this, volume);
}

void OpenAlSoundDriver::OpenAlSound::loop(uint8_t volume) {
    _driver._active_channel->loop(*this, volume);
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

unique_ptr<Sound> OpenAlSoundDriver::open_sound(pn::string_view path) {
    unique_ptr<OpenAlSound> sound(new OpenAlSound(*this));
    SoundData               s = Resource::sound(path);
    sound->buffer(s);
    return std::move(sound);
}

unique_ptr<Sound> OpenAlSoundDriver::open_music(pn::string_view path) {
    unique_ptr<OpenAlSound> music(new OpenAlSound(*this));
    SoundData               s = Resource::music(path);
    music->buffer(s);
    return std::move(music);
}

void OpenAlSoundDriver::set_global_volume(uint8_t volume) { alListenerf(AL_GAIN, volume / 8.0); }

}  // namespace antares
