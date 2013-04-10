// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2008-2012 The Antares Authors
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

#include "cocoa/prefs-driver.hpp"

#include <algorithm>
#include <CoreFoundation/CoreFoundation.h>

#include "cocoa/core-foundation.hpp"
#include "config/keys.hpp"
#include "config/preferences.hpp"

using sfz::String;
using sfz::StringSlice;
using sfz::range;
using std::min;
using std::swap;

namespace antares {

namespace {

const char kKeySettingsPreference[]     = "KeySettings";
const char kIdleMusicPreference[]       = "PlayIdleMusic";
const char kGameMusicPreference[]       = "PlayGameMusic";
const char kSpeechOnPreference[]        = "SpeechOn";
const char kVolumePreference[]          = "Volume";
const char kFullscreenPreference[]      = "Fullscreen";
const char kScreenWidthPreference[]     = "ScreenWidth";
const char kScreenHeightPreference[]    = "ScreenHeight";
const char kScenarioPreference[]        = "Scenario";

template <typename T>
T clamp(T value, T min, T max) {
    if (value < min) {
        return min;
    } else if (value > max) {
        return max;
    } else {
        return value;
    }
}

template <typename T>
void clear(T& t) {
    t = T();
}

}  // namespace

namespace cf {
namespace {

template <typename T>
bool get_preference(const StringSlice& key, T& value) {
    PropertyList plist(CFPreferencesCopyAppValue(
                cf::wrap(key).c_obj(), kCFPreferencesCurrentApplication));
    if (plist.c_obj()) {
        value = cast<T>(std::move(plist));
        return true;
    }
    return false;
}

template <typename T>
void set_preference(const StringSlice& key, const T& value) {
    CFPreferencesSetAppValue(
            cf::wrap(key).c_obj(), value.c_obj(), kCFPreferencesCurrentApplication);
}

}  // namespace
}  // namespace cf

CoreFoundationPrefsDriver::CoreFoundationPrefsDriver() { }

void CoreFoundationPrefsDriver::load(Preferences* preferences) {
    clear(*preferences);

    cf::Array key_settings;
    if (cf::get_preference(kKeySettingsPreference, key_settings)) {
        for (int i: range(min<int>(KEY_COUNT, key_settings.size()))) {
            cf::Number number = cf::cast<cf::Number>(cf::Type(CFRetain(key_settings.get(i))));
            int key;
            if (cf::unwrap(number, key)) {
                preferences->set_key(i, key);
            }
        }
    }

    {
        cf::Boolean cfbool;
        bool val;
        if (cf::get_preference(kIdleMusicPreference, cfbool) && cf::unwrap(cfbool, val)) {
            preferences->set_play_idle_music(val);
        }
        if (cf::get_preference(kGameMusicPreference, cfbool) && cf::unwrap(cfbool, val)) {
            preferences->set_play_music_in_game(val);
        }
        if (cf::get_preference(kSpeechOnPreference, cfbool) && cf::unwrap(cfbool, val)) {
            preferences->set_speech_on(val);
        }
        if (cf::get_preference(kFullscreenPreference, cfbool) && cf::unwrap(cfbool, val)) {
            preferences->set_fullscreen(val);
        }
    }

    {
        cf::Number cfnum;
        double val;
        if (cf::get_preference(kVolumePreference, cfnum) && cf::unwrap(cfnum, val)) {
            preferences->set_volume(clamp<int>(8 * val, 0, 8));
        }
    }

    {
        cf::Number cfnum;
        Size screen_size = preferences->screen_size();
        int32_t val;
        if (cf::get_preference(kScreenWidthPreference, cfnum) && cf::unwrap(cfnum, val)) {
            screen_size.width = val;
        }
        if (cf::get_preference(kScreenHeightPreference, cfnum) && cf::unwrap(cfnum, val)) {
            screen_size.height = val;
        }
        preferences->set_screen_size(screen_size);
    }

    cf::String cfstr;
    String id;
    if (cf::get_preference(kScenarioPreference, cfstr) && cf::unwrap(cfstr, id)) {
        preferences->set_scenario_identifier(id);
    }
}

void CoreFoundationPrefsDriver::save(const Preferences& preferences) {
    const Size screen_size = preferences.screen_size();

    cf::MutableArray key_settings(CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks));
    for (int i: range<int>(KEY_COUNT)) {
        int key = preferences.key(i);
        key_settings.append(cf::wrap(key).c_obj());
    }
    cf::set_preference(kKeySettingsPreference, key_settings);
    cf::set_preference(kIdleMusicPreference, cf::wrap(preferences.play_idle_music()));
    cf::set_preference(kGameMusicPreference, cf::wrap(preferences.play_music_in_game()));
    cf::set_preference(kSpeechOnPreference, cf::wrap(preferences.speech_on()));
    cf::set_preference(kFullscreenPreference, cf::wrap(preferences.fullscreen()));
    cf::set_preference(kVolumePreference, cf::wrap(0.125 * preferences.volume()));
    cf::set_preference(kScreenWidthPreference, cf::wrap(screen_size.width));
    cf::set_preference(kScreenHeightPreference, cf::wrap(screen_size.height));
    cf::set_preference(kScenarioPreference, cf::wrap(preferences.scenario_identifier()));
    CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
}

}  // namespace antares
