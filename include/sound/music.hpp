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

#ifndef ANTARES_SOUND_MUSIC_HPP_
#define ANTARES_SOUND_MUSIC_HPP_

#include <memory>
#include <pn/string>

using std::unique_ptr;

namespace antares {

class Sound;
class SoundChannel;

class Music {
  public:
    static const pn::string_view title_song;
    static const pn::string_view prologue_song;
    static const pn::string_view victory_song;
    static const pn::string_view briefing_song;

    enum Type {
        IN_GAME,
        IDLE,
    };

    void init();
    void play(Type type, pn::string_view song);
    void stop();
    void toggle();
    void sync();

  private:
    void StopSong();

    bool                     _playing = false;
    Type                     _song_type;
    pn::string               _song;
    unique_ptr<Sound>        _song_sound;
    unique_ptr<SoundChannel> _channel;
};

}  // namespace antares

#endif  // ANTARES_SOUND_MUSIC_HPP_
