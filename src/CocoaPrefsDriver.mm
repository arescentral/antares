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

#include "CocoaPrefsDriver.hpp"

#include <Cocoa/Cocoa.h>
#include "KeyCodes.hpp"
#include "Preferences.hpp"

#define KEY_SETTINGS_DEFAULT    @"KeySettings"
#define IDLE_MUSIC_DEFAULT      @"PlayIdleMusic"
#define GAME_MUSIC_DEFAULT      @"PlayGameMusic"
#define SPEECH_ON_DEFAULT       @"SpeechOn"
#define VOLUME_DEFAULT          @"Volume"

namespace antares {

namespace {

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

}  // namespace

CocoaPrefsDriver::CocoaPrefsDriver() {
    NSUserDefaults* const defaults = [NSUserDefaults standardUserDefaults];
    Preferences const preferences;

    NSMutableArray* const key_settings = [NSMutableArray array];
    for (int i = 0; i < KEY_COUNT; ++i) {
        [key_settings addObject:[NSNumber numberWithInt:preferences.key(i)]];
    }
    [defaults registerDefaults:[NSDictionary dictionaryWithObjectsAndKeys:
        key_settings,                                               KEY_SETTINGS_DEFAULT,
        [NSNumber numberWithBool:preferences.play_idle_music()],    IDLE_MUSIC_DEFAULT,
        [NSNumber numberWithBool:preferences.play_music_in_game()], GAME_MUSIC_DEFAULT,
        [NSNumber numberWithBool:preferences.speech_on()],          SPEECH_ON_DEFAULT,
        [NSNumber numberWithFloat:(0.125 * preferences.volume())],  VOLUME_DEFAULT,
        nil]];
}

void CocoaPrefsDriver::load(Preferences* preferences) {
    NSUserDefaults* const defaults = [NSUserDefaults standardUserDefaults];

    NSArray* const keys = [defaults arrayForKey:KEY_SETTINGS_DEFAULT];
    for (size_t i = 0; i < KEY_COUNT; ++i) {
        preferences->set_key(i, 0);
        if (i < [keys count]) {
            id object = [keys objectAtIndex:i];
            if ([object respondsToSelector:@selector(intValue)]) {
                preferences->set_key(i, [object intValue]);
            }
        }
    }

    preferences->set_play_idle_music([defaults boolForKey:IDLE_MUSIC_DEFAULT]);
    preferences->set_play_music_in_game([defaults boolForKey:GAME_MUSIC_DEFAULT]);
    preferences->set_speech_on([defaults boolForKey:SPEECH_ON_DEFAULT]);
    preferences->set_volume(8 * clamp<float>([defaults floatForKey:VOLUME_DEFAULT], 0.0, 1.0));
}

void CocoaPrefsDriver::save(const Preferences& preferences) {
    NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];

    NSMutableArray* key_settings = [NSMutableArray array];
    for (int i = 0; i < KEY_COUNT; ++i) {
        [key_settings addObject:[NSNumber numberWithInt:preferences.key(i)]];
    }
    [defaults setObject:key_settings forKey:KEY_SETTINGS_DEFAULT];
    [defaults setBool:preferences.play_idle_music() forKey:IDLE_MUSIC_DEFAULT];
    [defaults setBool:preferences.play_music_in_game() forKey:GAME_MUSIC_DEFAULT];
    [defaults setBool:preferences.speech_on() forKey:SPEECH_ON_DEFAULT];
    [defaults setFloat:(0.125 * preferences.volume()) forKey:VOLUME_DEFAULT];
}

}  // namespace antares
