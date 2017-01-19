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

#ifndef ANTARES_MAC_AUDIO_FILE_HPP_
#define ANTARES_MAC_AUDIO_FILE_HPP_

#include <AudioToolbox/AudioToolbox.h>
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <sfz/sfz.hpp>

namespace antares {

class AudioFile {
  public:
    AudioFile(const sfz::BytesSlice& data);

    ~AudioFile();

    void convert(sfz::Bytes& data, ALenum& format, ALsizei& frequency) const;

    AudioFileID id() const { return _id; }

  private:
    static OSStatus read_proc(
            void* this_, SInt64 in_pos, UInt32 req_count, void* buffer, UInt32* actual_count);
    static SInt64 get_size_proc(void* this_);
    OSStatus read(SInt64 in_pos, UInt32 req_count, void* buffer, UInt32* actual_count) const;
    SInt64 get_size() const;

    AudioFileID     _id;
    sfz::BytesSlice _data;

    DISALLOW_COPY_AND_ASSIGN(AudioFile);
};

}  // namespace antares

#endif  // ANTARES_MAC_AUDIO_FILE_HPP_
