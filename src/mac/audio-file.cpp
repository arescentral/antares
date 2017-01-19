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

#include "mac/audio-file.hpp"

#include <AudioToolbox/AudioToolbox.h>
#include <sfz/sfz.hpp>

#include "data/resource.hpp"

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

static void check_os_err(OSStatus err, const StringSlice& method) {
    if (err != noErr) {
        throw Exception(format("{0}: {1}", method, err));
    }
}

namespace {

class ExtAudioFile {
  public:
    ExtAudioFile(const AudioFile& audio_file) {
        OSStatus err = ExtAudioFileWrapAudioFileID(audio_file.id(), false, &_id);
        if (err != noErr) {
            throw Exception("ExtAudioFileWrapAudioFileID() failed.");
        }
    }

    ~ExtAudioFile() { ExtAudioFileDispose(_id); }

    void convert(Bytes& data, ALenum& format, ALsizei& frequency);

    ExtAudioFileRef id() const { return _id; }

  private:
    ExtAudioFileRef _id;

    DISALLOW_COPY_AND_ASSIGN(ExtAudioFile);
};

}  // namespace

AudioFile::AudioFile(const BytesSlice& data) : _data(data) {
    OSStatus err = AudioFileOpenWithCallbacks(
            this, read_proc, NULL, get_size_proc, NULL, kAudioFileAIFFType, &_id);
    check_os_err(err, "AudioFileOpenWithCallbacks");
}

AudioFile::~AudioFile() {
    AudioFileClose(_id);
}

void AudioFile::convert(Bytes& data, ALenum& format, ALsizei& frequency) const {
    ExtAudioFile ext(*this);
    ext.convert(data, format, frequency);
}

void ExtAudioFile::convert(Bytes& data, ALenum& format, ALsizei& frequency) {
    OSStatus err;

    // Read in the original file format.
    AudioStreamBasicDescription in_format;
    UInt32                      in_format_size = sizeof(AudioStreamBasicDescription);
    err                                        = ExtAudioFileGetProperty(
            _id, kExtAudioFileProperty_FileDataFormat, &in_format_size, &in_format);
    check_os_err(err, "ExtAudioFileGetProperty");

    frequency = in_format.mSampleRate;
    if (in_format.mChannelsPerFrame == 1) {
        format = AL_FORMAT_MONO16;
    } else if (in_format.mChannelsPerFrame == 2) {
        format = AL_FORMAT_STEREO16;
    } else {
        throw Exception("audio file has more than two channels");
    }

    // Convert to 16-bit native-endian linear PCM.  Preserve the frequency and channel count
    // of the original format.
    AudioStreamBasicDescription out_format = in_format;
    out_format.mFormatID                   = kAudioFormatLinearPCM;
    out_format.mBytesPerPacket             = 2 * out_format.mChannelsPerFrame;
    out_format.mFramesPerPacket            = 1;
    out_format.mBytesPerFrame              = 2 * out_format.mChannelsPerFrame;
    out_format.mBitsPerChannel             = 16;
    out_format.mFormatFlags = kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked |
                              kAudioFormatFlagIsSignedInteger;
    err = ExtAudioFileSetProperty(
            _id, kExtAudioFileProperty_ClientDataFormat, sizeof(AudioStreamBasicDescription),
            &out_format);
    check_os_err(err, "ExtAudioFileSetProperty");

    // Get the number of frames.
    SInt64 frame_count;
    UInt32 frame_count_size = sizeof(int64_t);
    err                     = ExtAudioFileGetProperty(
            _id, kExtAudioFileProperty_FileLengthFrames, &frame_count_size, &frame_count);
    check_os_err(err, "ExtAudioFileGetProperty");

    // Read the converted frames into memory.
    UInt32 frame_count_32 = frame_count;
    data.resize(frame_count * out_format.mBytesPerFrame);
    AudioBufferList data_buffer;
    data_buffer.mNumberBuffers              = 1;
    data_buffer.mBuffers[0].mDataByteSize   = data.size();
    data_buffer.mBuffers[0].mNumberChannels = out_format.mChannelsPerFrame;
    data_buffer.mBuffers[0].mData           = data.data();
    err                                     = ExtAudioFileRead(_id, &frame_count_32, &data_buffer);
    check_os_err(err, "ExtAudioFileRead");
}

OSStatus AudioFile::read_proc(
        void* this_, SInt64 in_pos, UInt32 req_count, void* buffer, UInt32* actual_count) {
    return reinterpret_cast<AudioFile*>(this_)->read(in_pos, req_count, buffer, actual_count);
}

SInt64 AudioFile::get_size_proc(void* this_) {
    return reinterpret_cast<AudioFile*>(this_)->get_size();
}

OSStatus AudioFile::read(
        SInt64 in_pos, UInt32 req_count, void* buffer, UInt32* actual_count) const {
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

}  // namespace antares
