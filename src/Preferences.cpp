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

#include <algorithm>
#include <sfz/Exception.hpp>
#include "AresGlobalType.hpp"
#include "Error.hpp"
#include "KeyCodes.hpp"
#include "KeyMapTranslation.hpp"
#include "Options.hpp"
#include "Resource.hpp"

using sfz::BytesPiece;
using sfz::ReadSource;
using sfz::Exception;
using sfz::read;
using sfz::scoped_ptr;
using std::min;
using std::max;

namespace antares {

namespace {

template <typename T>
T clamp(T value, T lower, T upper) {
    return min(upper, max(lower, value));
}

}  // namespace

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
    set_key(kUpKeyNum,              1 + Keys::N8);
    set_key(kDownKeyNum,            1 + Keys::N5);
    set_key(kLeftKeyNum,            1 + Keys::N4);
    set_key(kRightKeyNum,           1 + Keys::N6);
    set_key(kOneKeyNum,             1 + Keys::OPTION);
    set_key(kTwoKeyNum,             1 + Keys::COMMAND);
    set_key(kEnterKeyNum,           1 + Keys::SPACE);
    set_key(kWarpKeyNum,            1 + Keys::TAB);

    set_key(kSelectFriendKeyNum,    1 + Keys::N_CLEAR);
    set_key(kSelectFoeKeyNum,       1 + Keys::N_EQUALS);
    set_key(kSelectBaseKeyNum,      1 + Keys::N_DIVIDE);
    set_key(kDestinationKeyNum,     1 + Keys::SHIFT);
    set_key(kOrderKeyNum,           1 + Keys::CONTROL);

    set_key(kZoomInKeyNum,          1 + Keys::N_PLUS);
    set_key(kZoomOutKeyNum,         1 + Keys::N_MINUS);

    set_key(kCompUpKeyNum,          1 + Keys::UP_ARROW);
    set_key(kCompDownKeyNum,        1 + Keys::DOWN_ARROW);
    set_key(kCompAcceptKeyNum,      1 + Keys::RIGHT_ARROW);
    set_key(kCompCancelKeyNum,      1 + Keys::LEFT_ARROW);

    set_key(kTransferKeyNum,        1 + Keys::F8);
    set_key(kScale121KeyNum,        1 + Keys::F9);
    set_key(kScale122KeyNum,        1 + Keys::F10);
    set_key(kScale124KeyNum,        1 + Keys::F11);
    set_key(kScale1216KeyNum,       1 + Keys::F12);
    set_key(kScaleHostileKeyNum,    1 + Keys::HELP);
    set_key(kScaleObjectKeyNum,     1 + Keys::PAGE_UP);
    set_key(kScaleAllKeyNum,        1 + Keys::HOME);

    set_key(kMessageNextKeyNum,     1 + Keys::BACKSPACE);
    set_key(kHelpKeyNum,            1 + Keys::F1);
    set_key(kVolumeDownKeyNum,      1 + Keys::F2);
    set_key(kVolumeUpKeyNum,        1 + Keys::F3);
    set_key(kActionMusicKeyNum,     1 + Keys::F4);
    set_key(kNetSettingsKeyNum,     1 + Keys::F5);
    set_key(kFastMotionKeyNum,      1 + Keys::F6);

    set_key(kFirstHotKeyNum + 0,    1 + Keys::K1);
    set_key(kFirstHotKeyNum + 1,    1 + Keys::K2);
    set_key(kFirstHotKeyNum + 2,    1 + Keys::K3);
    set_key(kFirstHotKeyNum + 3,    1 + Keys::K4);
    set_key(kFirstHotKeyNum + 4,    1 + Keys::K5);
    set_key(kFirstHotKeyNum + 5,    1 + Keys::K6);
    set_key(kFirstHotKeyNum + 6,    1 + Keys::K7);
    set_key(kFirstHotKeyNum + 7,    1 + Keys::K8);
    set_key(kFirstHotKeyNum + 8,    1 + Keys::K9);
    set_key(kFirstHotKeyNum + 9,    1 + Keys::K0);

    set_play_idle_music(true);
    set_play_music_in_game(false);
    set_speech_on(false);

    set_volume(7);
}

void Preferences::copy(const Preferences& preferences) {
    for (size_t i = 0; i < KEY_COUNT; ++i) {
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
    _volume = clamp(volume, 0, 8);
}

}  // namespace antares
