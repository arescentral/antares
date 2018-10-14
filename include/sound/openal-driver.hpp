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

#ifndef ANTARES_SOUND_OPENAL_DRIVER_HPP_
#define ANTARES_SOUND_OPENAL_DRIVER_HPP_

#include "sound/driver.hpp"

#ifdef __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif

namespace antares {

class OpenAlSoundDriver : public SoundDriver {
  public:
    OpenAlSoundDriver();
    OpenAlSoundDriver(const OpenAlSoundDriver&) = delete;
    OpenAlSoundDriver& operator=(const OpenAlSoundDriver&) = delete;
    ~OpenAlSoundDriver();

    virtual std::unique_ptr<SoundChannel> open_channel();
    virtual std::unique_ptr<Sound>        open_sound(pn::string_view path);
    virtual std::unique_ptr<Sound>        open_music(pn::string_view path);
    virtual void                          set_global_volume(uint8_t volume);

  private:
    class OpenAlChannel;
    class OpenAlSound;

    ALCcontext*    _context;
    ALCdevice*     _device;
    OpenAlChannel* _active_channel;
};

}  // namespace antares

#endif  // ANTARES_SOUND_OPENAL_DRIVER_HPP_
