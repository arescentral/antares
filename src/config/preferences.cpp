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

using sfz::range;
using std::max;
using std::min;
using std::unique_ptr;

namespace antares {

template <typename T>
static T clamp(T value, T lower, T upper) {
    return min(upper, max(lower, value));
}

Preferences::Preferences() {
    keys[kUpKeyNum]    = Key::N8;
    keys[kDownKeyNum]  = Key::N5;
    keys[kLeftKeyNum]  = Key::N4;
    keys[kRightKeyNum] = Key::N6;
    keys[kOneKeyNum]   = Key::L_OPTION;
    keys[kTwoKeyNum]   = Key::L_COMMAND;
    keys[kEnterKeyNum] = Key::SPACE;
    keys[kWarpKeyNum]  = Key::TAB;

    keys[kSelectFriendKeyNum] = Key::N_CLEAR;
    keys[kSelectFoeKeyNum]    = Key::N_EQUALS;
    keys[kSelectBaseKeyNum]   = Key::N_DIVIDE;
    keys[kDestinationKeyNum]  = Key::L_SHIFT;
    keys[kOrderKeyNum]        = Key::L_CONTROL;

    keys[kZoomInKeyNum]  = Key::N_PLUS;
    keys[kZoomOutKeyNum] = Key::N_MINUS;

    keys[kCompUpKeyNum]     = Key::UP_ARROW;
    keys[kCompDownKeyNum]   = Key::DOWN_ARROW;
    keys[kCompAcceptKeyNum] = Key::RIGHT_ARROW;
    keys[kCompCancelKeyNum] = Key::LEFT_ARROW;

    keys[kTransferKeyNum]     = Key::F8;
    keys[kScale121KeyNum]     = Key::F9;
    keys[kScale122KeyNum]     = Key::F10;
    keys[kScale124KeyNum]     = Key::F11;
    keys[kScale1216KeyNum]    = Key::F12;
    keys[kScaleHostileKeyNum] = Key::HELP;
    keys[kScaleObjectKeyNum]  = Key::HOME;
    keys[kScaleAllKeyNum]     = Key::PAGE_UP;

    keys[kMessageNextKeyNum] = Key::BACKSPACE;
    keys[kHelpKeyNum]        = Key::F1;
    keys[kVolumeDownKeyNum]  = Key::F2;
    keys[kVolumeUpKeyNum]    = Key::F3;
    keys[kActionMusicKeyNum] = Key::F4;
    keys[kNetSettingsKeyNum] = Key::F5;
    keys[kFastMotionKeyNum]  = Key::F6;

    keys[kFirstHotKeyNum + 0] = Key::K1;
    keys[kFirstHotKeyNum + 1] = Key::K2;
    keys[kFirstHotKeyNum + 2] = Key::K3;
    keys[kFirstHotKeyNum + 3] = Key::K4;
    keys[kFirstHotKeyNum + 4] = Key::K5;
    keys[kFirstHotKeyNum + 5] = Key::K6;
    keys[kFirstHotKeyNum + 6] = Key::K7;
    keys[kFirstHotKeyNum + 7] = Key::K8;
    keys[kFirstHotKeyNum + 8] = Key::K9;
    keys[kFirstHotKeyNum + 9] = Key::K0;

    play_idle_music    = true;
    play_music_in_game = false;
    speech_on          = false;

    volume = 7;

    scenario_identifier = kFactoryScenarioIdentifier;
}

Preferences Preferences::copy() const {
    Preferences copy;
    memcpy(copy.keys, keys, sizeof(keys));
    copy.play_idle_music     = play_idle_music;
    copy.play_music_in_game  = play_music_in_game;
    copy.speech_on           = speech_on;
    copy.volume              = volume;
    copy.scenario_identifier = scenario_identifier.copy();
    return copy;
}

PrefsDriver::PrefsDriver() {
    if (sys.prefs) {
        throw std::runtime_error("PrefsDriver is a singleton");
    }
    sys.prefs = this;
}

PrefsDriver::~PrefsDriver() { sys.prefs = NULL; }

void PrefsDriver::set_key(size_t index, Key key) {
    Preferences p(get().copy());
    p.keys[index] = key;
    set(p);
}

void PrefsDriver::set_play_idle_music(bool on) {
    Preferences p(get().copy());
    p.play_idle_music = on;
    set(p);
}

void PrefsDriver::set_play_music_in_game(bool on) {
    Preferences p(get().copy());
    p.play_music_in_game = on;
    set(p);
}

void PrefsDriver::set_speech_on(bool on) {
    Preferences p(get().copy());
    p.speech_on = on;
    set(p);
}

void PrefsDriver::set_volume(int volume) {
    Preferences p(get().copy());
    p.volume = volume;
    set(p);
}

void PrefsDriver::set_scenario_identifier(pn::string_view id) {
    Preferences p(get().copy());
    p.scenario_identifier = id.copy();
    set(p);
}

NullPrefsDriver::NullPrefsDriver() {}

NullPrefsDriver::NullPrefsDriver(Preferences defaults) : _saved(defaults.copy()) {}

const Preferences& NullPrefsDriver::get() const { return _saved; }

void NullPrefsDriver::set(const Preferences& prefs) { _saved = prefs.copy(); }

}  // namespace antares
