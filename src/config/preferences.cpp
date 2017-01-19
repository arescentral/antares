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

#include "config/preferences.hpp"

#include <algorithm>
#include <sfz/sfz.hpp>

#include "config/dirs.hpp"
#include "config/keys.hpp"
#include "data/resource.hpp"
#include "game/globals.hpp"
#include "game/sys.hpp"
#include "lang/defines.hpp"

using sfz::BytesSlice;
using sfz::Exception;
using sfz::ReadSource;
using sfz::StringSlice;
using sfz::range;
using sfz::read;
using std::max;
using std::min;
using std::unique_ptr;

namespace antares {

template <typename T>
static T clamp(T value, T lower, T upper) {
    return min(upper, max(lower, value));
}

Preferences::Preferences() {
    keys[kUpKeyNum]    = 1 + Keys::N8;
    keys[kDownKeyNum]  = 1 + Keys::N5;
    keys[kLeftKeyNum]  = 1 + Keys::N4;
    keys[kRightKeyNum] = 1 + Keys::N6;
    keys[kOneKeyNum]   = 1 + Keys::L_OPTION;
    keys[kTwoKeyNum]   = 1 + Keys::L_COMMAND;
    keys[kEnterKeyNum] = 1 + Keys::SPACE;
    keys[kWarpKeyNum]  = 1 + Keys::TAB;

    keys[kSelectFriendKeyNum] = 1 + Keys::N_CLEAR;
    keys[kSelectFoeKeyNum]    = 1 + Keys::N_EQUALS;
    keys[kSelectBaseKeyNum]   = 1 + Keys::N_DIVIDE;
    keys[kDestinationKeyNum]  = 1 + Keys::L_SHIFT;
    keys[kOrderKeyNum]        = 1 + Keys::L_CONTROL;

    keys[kZoomInKeyNum]  = 1 + Keys::N_PLUS;
    keys[kZoomOutKeyNum] = 1 + Keys::N_MINUS;

    keys[kCompUpKeyNum]     = 1 + Keys::UP_ARROW;
    keys[kCompDownKeyNum]   = 1 + Keys::DOWN_ARROW;
    keys[kCompAcceptKeyNum] = 1 + Keys::RIGHT_ARROW;
    keys[kCompCancelKeyNum] = 1 + Keys::LEFT_ARROW;

    keys[kTransferKeyNum]     = 1 + Keys::F8;
    keys[kScale121KeyNum]     = 1 + Keys::F9;
    keys[kScale122KeyNum]     = 1 + Keys::F10;
    keys[kScale124KeyNum]     = 1 + Keys::F11;
    keys[kScale1216KeyNum]    = 1 + Keys::F12;
    keys[kScaleHostileKeyNum] = 1 + Keys::HELP;
    keys[kScaleObjectKeyNum]  = 1 + Keys::HOME;
    keys[kScaleAllKeyNum]     = 1 + Keys::PAGE_UP;

    keys[kMessageNextKeyNum] = 1 + Keys::BACKSPACE;
    keys[kHelpKeyNum]        = 1 + Keys::F1;
    keys[kVolumeDownKeyNum]  = 1 + Keys::F2;
    keys[kVolumeUpKeyNum]    = 1 + Keys::F3;
    keys[kActionMusicKeyNum] = 1 + Keys::F4;
    keys[kNetSettingsKeyNum] = 1 + Keys::F5;
    keys[kFastMotionKeyNum]  = 1 + Keys::F6;

    keys[kFirstHotKeyNum + 0] = 1 + Keys::K1;
    keys[kFirstHotKeyNum + 1] = 1 + Keys::K2;
    keys[kFirstHotKeyNum + 2] = 1 + Keys::K3;
    keys[kFirstHotKeyNum + 3] = 1 + Keys::K4;
    keys[kFirstHotKeyNum + 4] = 1 + Keys::K5;
    keys[kFirstHotKeyNum + 5] = 1 + Keys::K6;
    keys[kFirstHotKeyNum + 6] = 1 + Keys::K7;
    keys[kFirstHotKeyNum + 7] = 1 + Keys::K8;
    keys[kFirstHotKeyNum + 8] = 1 + Keys::K9;
    keys[kFirstHotKeyNum + 9] = 1 + Keys::K0;

    play_idle_music    = true;
    play_music_in_game = false;
    speech_on          = false;

    volume = 7;

    scenario_identifier.assign(kFactoryScenarioIdentifier);
}

Preferences Preferences::copy() const {
    Preferences copy;
    memcpy(copy.keys, keys, sizeof(keys));
    copy.play_idle_music    = play_idle_music;
    copy.play_music_in_game = play_music_in_game;
    copy.speech_on          = speech_on;
    copy.volume             = volume;
    copy.scenario_identifier.assign(scenario_identifier);
    return copy;
}

PrefsDriver::PrefsDriver() {
    if (sys.prefs) {
        throw Exception("PrefsDriver is a singleton");
    }
    sys.prefs = this;
}

PrefsDriver::~PrefsDriver() {
    sys.prefs = NULL;
}

void PrefsDriver::set_key(size_t index, uint32_t key) {
    Preferences p(get());
    p.keys[index] = key;
    set(p);
}

void PrefsDriver::set_play_idle_music(bool on) {
    Preferences p(get());
    p.play_idle_music = on;
    set(p);
}

void PrefsDriver::set_play_music_in_game(bool on) {
    Preferences p(get());
    p.play_music_in_game = on;
    set(p);
}

void PrefsDriver::set_speech_on(bool on) {
    Preferences p(get());
    p.speech_on = on;
    set(p);
}

void PrefsDriver::set_volume(int volume) {
    Preferences p(get());
    p.volume = volume;
    set(p);
}

void PrefsDriver::set_scenario_identifier(sfz::StringSlice id) {
    Preferences p(get());
    p.scenario_identifier.assign(id);
    set(p);
}

NullPrefsDriver::NullPrefsDriver() {}

NullPrefsDriver::NullPrefsDriver(Preferences defaults) : _saved(defaults) {}

const Preferences& NullPrefsDriver::get() const {
    return _saved;
}

void NullPrefsDriver::set(const Preferences& prefs) {
    _saved = prefs.copy();
}

}  // namespace antares
