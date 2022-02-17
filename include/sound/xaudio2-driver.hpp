// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2022 The Antares Authors
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

#ifndef ANTARES_SOUND_XAUDIO2_DRIVER_HPP_
#define ANTARES_SOUND_XAUDIO2_DRIVER_HPP_

#include "sound/driver.hpp"

#include <stdint.h>
#include <memory>

struct IXAudio2;
struct IXAudio2MasteringVoice;

namespace antares {

class XAudio2SoundDriver : public SoundDriver {
  public:
    XAudio2SoundDriver();
    XAudio2SoundDriver(const XAudio2SoundDriver&) = delete;
    XAudio2SoundDriver& operator=(const XAudio2SoundDriver&) = delete;
    ~XAudio2SoundDriver();

    virtual std::unique_ptr<SoundChannel> open_channel();
    virtual std::unique_ptr<Sound>        open_sound(pn::string_view path);
    virtual std::unique_ptr<Sound>        open_music(pn::string_view path);
    virtual void                          set_global_volume(uint8_t volume);

  private:
    class XAudio2Channel;
    class XAudio2Sound;
    class XAudio2SoundInstance;
    class XAudio2VoiceInstance;
    class XAudio2SourceVoiceInstance;
    class XAudio2GarbageCollector;
    class XAudio2GarbageCollectable;
    class XAudio2VoiceCallbacks;

    void garbage_collect(XAudio2GarbageCollectable* garbage_collectable);

    IXAudio2* get_xa2() const { return _xa2; }
    IXAudio2MasteringVoice* get_mv() const { return _mv; }
    uint32_t                get_sample_rate() const { return _sample_rate; }
    XAudio2VoiceCallbacks*  get_voice_callbacks() const { return _voice_callbacks.get(); }

    IXAudio2*                                   _xa2;
    IXAudio2MasteringVoice*                     _mv;
    XAudio2Channel*                             _active_channel;
    std::unique_ptr<XAudio2GarbageCollector>    _gc;
    uint32_t                                    _sample_rate;
    std::unique_ptr<XAudio2VoiceCallbacks>      _voice_callbacks;
    bool                                        _is_com_initialized;
};

}  // namespace antares

#endif  // ANTARES_SOUND_XAUDIO2_DRIVER_HPP_
