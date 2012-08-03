// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2012 The Antares Authors
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

#include <AudioToolbox/AudioToolbox.h>
#include <sfz/sfz.hpp>

#include "data/resource.hpp"

using sfz::Bytes;
using sfz::BytesSlice;
using sfz::Exception;
using sfz::PrintItem;
using sfz::StringSlice;
using sfz::format;
using sfz::scoped_ptr;

namespace antares {

namespace {

const char* al_error_to_string(int error) {
    switch (error) {
      case AL_NO_ERROR:             return "AL_NO_ERROR";
      case AL_INVALID_NAME:         return "AL_INVALID_NAME";
      case AL_INVALID_ENUM:         return "AL_INVALID_ENUM";
      case AL_INVALID_VALUE:        return "AL_INVALID_VALUE";
      case AL_INVALID_OPERATION:    return "AL_INVALID_OPERATION";
      case AL_OUT_OF_MEMORY:        return "AL_OUT_OF_MEMORY";
      default:                      return "unknown OpenAL error";
    }
}

void check_al_error(const StringSlice& method) {
    int error = alGetError();
    if (error != AL_NO_ERROR) {
        throw Exception(format("{0}: {1}", method, al_error_to_string(error)));
    }
}

void check_os_err(OSStatus err, const StringSlice& method) {
    if (err != noErr) {
        throw Exception(format("{0}: {1}", method, err));
    }
}

class AudioFile {
  public:
    AudioFile(const BytesSlice& data);

    ~AudioFile();

    void convert(Bytes* data, ALenum* format, ALsizei* frequency) const;

    AudioFileID id() const { return _id; }

  private:
    static OSStatus read_proc(void* this_, SInt64 in_pos, UInt32 req_count, void* buffer,
            UInt32* actual_count);
    static SInt64 get_size_proc(void* this_);
    OSStatus read(SInt64 in_pos, UInt32 req_count, void* buffer, UInt32* actual_count) const;
    SInt64 get_size() const;

    AudioFileID _id;
    BytesSlice _data;

    DISALLOW_COPY_AND_ASSIGN(AudioFile);
};

class ExtAudioFile {
  public:
    ExtAudioFile(const AudioFile& audio_file) {
        OSStatus err = ExtAudioFileWrapAudioFileID(audio_file.id(), false, &_id);
        if (err != noErr) {
            throw Exception("ExtAudioFileWrapAudioFileID() failed.");
        }
    }

    ~ExtAudioFile() {
        ExtAudioFileDispose(_id);
    }

    void convert(Bytes* data, ALenum* format, ALsizei* frequency);

    ExtAudioFileRef id() const { return _id; }

  private:
    ExtAudioFileRef _id;

    DISALLOW_COPY_AND_ASSIGN(ExtAudioFile);
};

AudioFile::AudioFile(const BytesSlice& data)
        : _data(data) {
    OSStatus err = AudioFileOpenWithCallbacks(
            this, read_proc, NULL, get_size_proc, NULL, kAudioFileAIFFType, &_id);
    check_os_err(err, "AudioFileOpenWithCallbacks");
}

AudioFile::~AudioFile() {
    AudioFileClose(_id);
}

void AudioFile::convert(Bytes* data, ALenum* format, ALsizei* frequency) const {
    ExtAudioFile ext(*this);
    ext.convert(data, format, frequency);
}

void ExtAudioFile::convert(Bytes* data, ALenum* format, ALsizei* frequency) {
    OSStatus err;

    // Read in the original file format.
    AudioStreamBasicDescription in_format;
    UInt32 in_format_size = sizeof(AudioStreamBasicDescription);
    err = ExtAudioFileGetProperty(_id, kExtAudioFileProperty_FileDataFormat, &in_format_size,
            &in_format);
    check_os_err(err, "ExtAudioFileGetProperty");

    *frequency = in_format.mSampleRate;
    if (in_format.mChannelsPerFrame == 1) {
        *format = AL_FORMAT_MONO16;
    } else if (in_format.mChannelsPerFrame == 2) {
        *format = AL_FORMAT_STEREO16;
    } else {
        throw Exception("audio file has more than two channels");
    }

    // Convert to 16-bit native-endian linear PCM.  Preserve the frequency and channel count
    // of the original format.
    AudioStreamBasicDescription out_format = in_format;
    out_format.mFormatID            = kAudioFormatLinearPCM;
    out_format.mBytesPerPacket      = 2 * out_format.mChannelsPerFrame;
    out_format.mFramesPerPacket     = 1;
    out_format.mBytesPerFrame       = 2 * out_format.mChannelsPerFrame;
    out_format.mBitsPerChannel      = 16;
    out_format.mFormatFlags         = kAudioFormatFlagsNativeEndian
                                    | kAudioFormatFlagIsPacked
                                    | kAudioFormatFlagIsSignedInteger;
    err = ExtAudioFileSetProperty(_id, kExtAudioFileProperty_ClientDataFormat,
            sizeof(AudioStreamBasicDescription), &out_format);
    check_os_err(err, "ExtAudioFileSetProperty");

    // Get the number of frames.
    SInt64 frame_count;
    UInt32 frame_count_size = sizeof(int64_t);
    err = ExtAudioFileGetProperty(_id, kExtAudioFileProperty_FileLengthFrames, &frame_count_size,
            &frame_count);
    check_os_err(err, "ExtAudioFileGetProperty");

    // Read the converted frames into memory.
    UInt32 frame_count_32 = frame_count;
    data->resize(frame_count * out_format.mBytesPerFrame);
    AudioBufferList data_buffer;
    data_buffer.mNumberBuffers = 1;
    data_buffer.mBuffers[0].mDataByteSize = data->size();
    data_buffer.mBuffers[0].mNumberChannels = out_format.mChannelsPerFrame;
    data_buffer.mBuffers[0].mData = data->data();
    err = ExtAudioFileRead(_id, &frame_count_32, &data_buffer);
    check_os_err(err, "ExtAudioFileRead");
}

OSStatus AudioFile::read_proc(void* this_, SInt64 in_pos, UInt32 req_count, void* buffer,
        UInt32* actual_count) {
    return reinterpret_cast<AudioFile*>(this_)->read(in_pos, req_count, buffer, actual_count);
}

SInt64 AudioFile::get_size_proc(void* this_) {
    return reinterpret_cast<AudioFile*>(this_)->get_size();
}

OSStatus AudioFile::read(SInt64 in_pos, UInt32 req_count, void* buffer,
        UInt32* actual_count) const {
    if (in_pos > _data.size()) {
        return kAudioFileInvalidPacketOffsetError;
    }
    *actual_count = std::min<UInt32>(req_count, _data.size() - in_pos);
    memcpy(buffer, _data.slice(in_pos, *actual_count).data(), *actual_count);
    return noErr;
}

SInt64 AudioFile::get_size() const {
    return _data.size();
}

}  // namespace

class OpenAlSoundDriver::OpenAlSound : public Sound {
  public:
    OpenAlSound(const OpenAlSoundDriver& driver):
            _driver(driver),
            _buffer(generate_buffer()) { }

    ~OpenAlSound() {
        alDeleteBuffers(1, &_buffer);
        alGetError();  // discard.
    }

    virtual void play();
    virtual void loop();

    void buffer(const AudioFile& audio_file) {
        Bytes data;
        ALenum format;
        ALsizei frequency;
        audio_file.convert(&data, &format, &frequency);
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
    ALuint _buffer;

    DISALLOW_COPY_AND_ASSIGN(OpenAlSound);
};

class OpenAlSoundDriver::OpenAlChannel : public SoundChannel {
  public:
    OpenAlChannel(OpenAlSoundDriver& driver):
            _driver(driver) {
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

    virtual void activate() {
        _driver._active_channel = this;
    }

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
    ALuint _source;

    DISALLOW_COPY_AND_ASSIGN(OpenAlChannel);
};

void OpenAlSoundDriver::OpenAlSound::play() {
    _driver._active_channel->play(*this);
}

void OpenAlSoundDriver::OpenAlSound::loop() {
    _driver._active_channel->loop(*this);
}

OpenAlSoundDriver::OpenAlSoundDriver():
        _active_channel(NULL) {
    // TODO(sfiera): error-checking.
    _device = alcOpenDevice(NULL);
    _context = alcCreateContext(_device, NULL);
    alcMakeContextCurrent(_context);
}

OpenAlSoundDriver::~OpenAlSoundDriver() {
    alcDestroyContext(_context);
    alcCloseDevice(_device);
}

void OpenAlSoundDriver::open_channel(scoped_ptr<SoundChannel>& channel) {
    channel.reset(new OpenAlChannel(*this));
}

void OpenAlSoundDriver::open_sound(PrintItem path, scoped_ptr<Sound>& sound) {
    scoped_ptr<OpenAlSound> result(new OpenAlSound(*this));
    Resource rsrc(path);
    AudioFile audio_file(rsrc.data());
    result->buffer(audio_file);
    sound.reset(result.release());
}

void OpenAlSoundDriver::set_global_volume(uint8_t volume) {
    alListenerf(AL_GAIN, volume / 8.0);
}

}  // namespace antares
