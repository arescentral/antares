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

#ifndef ANTARES_SOUND_OPENAL_DRIVER_HPP_
#define ANTARES_SOUND_OPENAL_DRIVER_HPP_

#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <sfz/sfz.hpp>

#include "sound/driver.hpp"

namespace antares {

class OpenAlSoundDriver : public SoundDriver {
  public:
    OpenAlSoundDriver();
    ~OpenAlSoundDriver();

    virtual void open_channel(sfz::scoped_ptr<SoundChannel>& channel);
    virtual void open_sound(sfz::PrintItem path, sfz::scoped_ptr<Sound>& sound);
    virtual void set_global_volume(uint8_t volume);

  private:
    class OpenAlChannel;
    class OpenAlSound;

    ALCcontext* _context;
    ALCdevice* _device;
    OpenAlChannel* _active_channel;

    DISALLOW_COPY_AND_ASSIGN(OpenAlSoundDriver);
};

}  // namespace antares

#endif  // ANTARES_SOUND_OPENAL_DRIVER_HPP_
