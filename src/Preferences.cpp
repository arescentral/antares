// Ares, a tactical space combat game.
// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#include "Preferences.hpp"

#include "sfz/BinaryReader.hpp"
#include "sfz/Exception.hpp"
#include "AresGlobalType.hpp"
#include "Error.hpp"
#include "KeyMapTranslation.hpp"
#include "Options.hpp"
#include "Resource.hpp"

using sfz::BinaryReader;
using sfz::BytesBinaryReader;
using sfz::Exception;
using sfz::scoped_ptr;

namespace antares {

scoped_ptr<Preferences> Preferences::_preferences;

Preferences* Preferences::preferences() {
    if (_preferences.get() == NULL) {
        throw Exception("Called Preferences::preferences() before Preferences::set_preferences()");
    }
    return _preferences.get();
}

void Preferences::set_preferences(Preferences* preferences) {
    if (preferences == NULL) {
        throw Exception("Called Preferences::set_preferences(NULL)");
    }
    _preferences.reset(preferences);
}

Preferences::Preferences() {
    reset();
}

Preferences::~Preferences() { }

void Preferences::reset() {
    Resource rsrc('ArPr', 1000);
    BytesBinaryReader bin(rsrc.data());

    bin.read(&_version);
    bin.read(_key_map, kKeyControlDataNum);
    bin.read(&_serial_number);
    bin.read(&_options);
    bin.discard(4);
    bin.read(&_volume);
    bin.read(&_minutes_played);
    bin.read(&_kills);
    bin.read(&_losses);
    bin.read(&_race);
    bin.read(&_enemy_color);
    bin.discard(4);
    bin.read(_player_name, 32);
    bin.read(_game_name, 32);
    bin.read(&_resend_delay);
    bin.read(&_registered_setting);
    bin.read(&_registered_flags);
    bin.read(&_protocol_flags);
    bin.read(&_net_level);
    bin.read(&_net_latency);
}

void Preferences::copy(const Preferences& preferences) {
    for (size_t i = 0; i < kKeyControlDataNum; ++i) {
        set_key(i, preferences.key(i));
    }
    set_play_idle_music(preferences.play_idle_music());
    set_play_music_in_game(preferences.play_music_in_game());
    set_speech_on(preferences.speech_on());
    set_volume(preferences.volume());
}

uint32_t Preferences::key(size_t index) const {
    return _key_map[index];
}

bool Preferences::play_idle_music() const {
    return _options & kOptionMusicIdle;
}

bool Preferences::play_music_in_game() const {
    return _options & kOptionMusicPlay;
}

bool Preferences::speech_on() const {
    return _options & kOptionSpeechOn;
}

int Preferences::volume() const {
    return _volume;
}

void Preferences::set_key(size_t index, uint32_t key) {
    _key_map[index] = key;
}

void Preferences::set_play_idle_music(bool on) {
    if (play_idle_music() != on) {
        _options ^= kOptionMusicIdle;
    }
}

void Preferences::set_play_music_in_game(bool on) {
    if (play_music_in_game() != on) {
        _options ^= kOptionMusicPlay;
    }
}

void Preferences::set_speech_on(bool on) {
    if (speech_on() != on) {
        _options ^= kOptionSpeechOn;
    }
}

void Preferences::set_volume(int volume) {
    _volume = volume;
}

void serialNumberType::read(BinaryReader* bin) {
    bin->read(name, 76);
    bin->read(number, kDigitNumber);
}

}  // namespace antares
