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
    String cfkey(key);
    PropertyList plist(CFPreferencesCopyAppValue(cfkey.c_obj(), kCFPreferencesCurrentApplication));
    return plist.c_obj() && move(value, plist);
}

template <typename T>
void set_preference(const StringSlice& key, const T& value) {
    String cfkey(key);
    CFPreferencesSetAppValue(cfkey.c_obj(), value.c_obj(), kCFPreferencesCurrentApplication);
}

}  // namespace
}  // namespace cf

CoreFoundationPrefsDriver::CoreFoundationPrefsDriver() { }

void CoreFoundationPrefsDriver::load(Preferences* preferences) {
    clear(*preferences);

    cf::Array key_settings;
    if (cf::get_preference(kKeySettingsPreference, key_settings)) {
        SFZ_FOREACH(int i, range(min<int>(KEY_COUNT, CFArrayGetCount(key_settings.c_obj()))), {
            cf::Type item(CFRetain(CFArrayGetValueAtIndex(key_settings.c_obj(), i)));
            cf::Number number;
            int key;
            if (move(number, item) && CFNumberGetValue(number.c_obj(), kCFNumberIntType, &key)) {
                preferences->set_key(i, key);
            }
        });
    }

    cf::Boolean bool_value;
    if (cf::get_preference(kIdleMusicPreference, bool_value)) {
        preferences->set_play_idle_music(bool_value.c_obj() == kCFBooleanTrue);
    }
    if (cf::get_preference(kGameMusicPreference, bool_value)) {
        preferences->set_play_music_in_game(bool_value.c_obj() == kCFBooleanTrue);
    }
    if (cf::get_preference(kSpeechOnPreference, bool_value)) {
        preferences->set_speech_on(bool_value.c_obj() == kCFBooleanTrue);
    }

    cf::Number number_value;
    double double_value;
    if (cf::get_preference(kVolumePreference, number_value)
            && CFNumberGetValue(number_value.c_obj(), kCFNumberDoubleType, &double_value)) {
        preferences->set_volume(clamp<int>(8 * double_value, 0, 8));
    }

    Size screen_size = preferences->screen_size();
    int32_t int_value;
    if (cf::get_preference(kScreenWidthPreference, number_value)
            && CFNumberGetValue(number_value.c_obj(), kCFNumberSInt32Type, &int_value)) {
        screen_size.width = int_value;
    }
    if (cf::get_preference(kScreenHeightPreference, number_value)
            && CFNumberGetValue(number_value.c_obj(), kCFNumberSInt32Type, &int_value)) {
        screen_size.height = int_value;
    }
    preferences->set_screen_size(screen_size);

    cf::String string_value;
    if (cf::get_preference(kScenarioPreference, string_value)) {
        String id(string_value);
        preferences->set_scenario_identifier(id);
    }
}

void CoreFoundationPrefsDriver::save(const Preferences& preferences) {
    cf::MutableArray key_settings(CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks));
    SFZ_FOREACH(int i, range<int>(KEY_COUNT), {
        int key = preferences.key(i);
        cf::Number cfkey(CFNumberCreate(NULL, kCFNumberIntType, &key));
        CFArrayAppendValue(key_settings.c_obj(), cfkey.c_obj());
    });
    cf::set_preference(kKeySettingsPreference, key_settings);

    cf::Boolean play_idle_music(preferences.play_idle_music());
    cf::set_preference(kIdleMusicPreference, play_idle_music);

    cf::Boolean play_music_in_game(preferences.play_music_in_game());
    cf::set_preference(kGameMusicPreference, play_music_in_game);

    cf::Boolean speech_on(preferences.speech_on());
    cf::set_preference(kSpeechOnPreference, speech_on);

    double volume_double = 0.125 * preferences.volume();
    cf::Number volume(CFNumberCreate(NULL, kCFNumberDoubleType, &volume_double));
    cf::set_preference(kVolumePreference, volume);

    const Size screen_size = preferences.screen_size();
    cf::Number screen_width(CFNumberCreate(NULL, kCFNumberSInt32Type, &screen_size.width));
    cf::Number screen_height(CFNumberCreate(NULL, kCFNumberSInt32Type, &screen_size.height));
    cf::set_preference(kScreenWidthPreference, screen_width);
    cf::set_preference(kScreenHeightPreference, screen_height);

    cf::String scenario(preferences.scenario_identifier());
    cf::set_preference(kScenarioPreference, scenario);

    CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
}

}  // namespace antares
