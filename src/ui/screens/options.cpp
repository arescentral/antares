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

#include "ui/screens/options.hpp"

#include <algorithm>
#include <sfz/sfz.hpp>

#include "config/keys.hpp"
#include "config/ledger.hpp"
#include "config/preferences.hpp"
#include "data/interface.hpp"
#include "data/string-list.hpp"
#include "drawing/color.hpp"
#include "drawing/styled-text.hpp"
#include "drawing/text.hpp"
#include "game/main.hpp"
#include "game/scenario-maker.hpp"
#include "game/time.hpp"
#include "sound/driver.hpp"
#include "sound/music.hpp"
#include "ui/card.hpp"
#include "ui/interface-handling.hpp"
#include "video/driver.hpp"

using sfz::Exception;
using sfz::String;
using sfz::format;
using std::make_pair;
using std::pair;
using std::unique_ptr;
using std::vector;

namespace antares {

OptionsScreen::OptionsScreen()
    : _state(SOUND_CONTROL),
      _preferences(Preferences::preferences()) { }

void OptionsScreen::become_front() {
    switch (_state) {
      case SOUND_CONTROL:
        stack()->push(new SoundControlScreen(&_state, _preferences));
        break;

      case KEY_CONTROL:
        stack()->push(new KeyControlScreen(&_state, _preferences));
        break;

      case ACCEPT:
        PrefsDriver::driver()->save(*_preferences);
        stack()->pop(this);
        break;

      case CANCEL:
        PrefsDriver::driver()->load(_preferences);
        SoundDriver::driver()->set_global_volume(_preferences->volume());
        stack()->pop(this);
        break;
    }
}

SoundControlScreen::SoundControlScreen(OptionsScreen::State* state, Preferences* preferences)
        : InterfaceScreen("options/sound", {0, 0, 640, 480}, true),
          _state(state),
          _preferences(preferences) { }

SoundControlScreen::~SoundControlScreen() { }

void SoundControlScreen::adjust_interface() {
    dynamic_cast<CheckboxButton&>(mutable_item(IDLE_MUSIC)).on = _preferences->play_idle_music();
    dynamic_cast<CheckboxButton&>(mutable_item(GAME_MUSIC)).on = _preferences->play_music_in_game();
    dynamic_cast<CheckboxButton&>(mutable_item(SPEECH_ON)).on = _preferences->speech_on();

    if (false) {  // TODO(sfiera): if speech available.
        dynamic_cast<Button&>(mutable_item(SPEECH_ON)).status = kActive;
    } else {
        dynamic_cast<Button&>(mutable_item(SPEECH_ON)).status = kDimmed;
    }

    if (_preferences->volume() > 0) {
        dynamic_cast<Button&>(mutable_item(VOLUME_DOWN)).status = kActive;
    } else {
        dynamic_cast<Button&>(mutable_item(VOLUME_DOWN)).status = kDimmed;
    }

    if (_preferences->volume() < kMaxVolumePreference) {
        dynamic_cast<Button&>(mutable_item(VOLUME_UP)).status = kActive;
    } else {
        dynamic_cast<Button&>(mutable_item(VOLUME_UP)).status = kDimmed;
    }
}

void SoundControlScreen::handle_button(Button& button) {
    switch (button.id) {
      case GAME_MUSIC:
        _preferences->set_play_music_in_game(!dynamic_cast<CheckboxButton&>(button).on);
        adjust_interface();
        break;

      case IDLE_MUSIC:
        _preferences->set_play_idle_music(!dynamic_cast<CheckboxButton&>(button).on);
        if (_preferences->play_idle_music()) {
            LoadSong(kTitleSongID);
            PlaySong();
        } else {
            StopAndUnloadSong();
        }
        adjust_interface();
        break;

      case SPEECH_ON:
        _preferences->set_speech_on(!dynamic_cast<CheckboxButton&>(button).on);
        adjust_interface();
        break;

      case VOLUME_DOWN:
        _preferences->set_volume(_preferences->volume() - 1);
        SoundDriver::driver()->set_global_volume(_preferences->volume());
        adjust_interface();
        break;

      case VOLUME_UP:
        _preferences->set_volume(_preferences->volume() + 1);
        SoundDriver::driver()->set_global_volume(_preferences->volume());
        adjust_interface();
        break;

      case DONE:
      case CANCEL:
      case KEY_CONTROL:
        *_state = button_state(button.id);
        stack()->pop(this);
        break;

      default:
        throw Exception(format("Got unknown button {0}.", button.id));
    }
}

void SoundControlScreen::overlay() const {
    const int volume = _preferences->volume();
    Rect bounds = item(VOLUME_BOX).bounds();
    Point off = offset();
    bounds.offset(off.h, off.v);

    const int notch_width = bounds.width() / kMaxVolumePreference;
    const int notch_height = bounds.height() - 4;
    Rect notch_bounds(0, 0, notch_width * kMaxVolumePreference, notch_height);
    notch_bounds.center_in(bounds);

    Rect notch(notch_bounds.left, notch_bounds.top,
            notch_bounds.left + notch_width, notch_bounds.bottom);
    notch.inset(3, 6);

    Rects rects;
    for (int i = 0; i < volume; ++i) {
        const RgbColor color = GetRGBTranslateColorShade(PALE_PURPLE, 2 * (i + 1));
        rects.fill(notch, color);
        notch.offset(notch_width, 0);
    }
}

OptionsScreen::State SoundControlScreen::button_state(int button) {
    switch (button) {
      case DONE:
        return OptionsScreen::ACCEPT;
      case CANCEL:
        return OptionsScreen::CANCEL;
      case KEY_CONTROL:
        return OptionsScreen::KEY_CONTROL;
      default:
        throw Exception(format("unknown sound control button {0}", button));
    }
}

static const usecs kFlashTime = ticks(12);

// Indices of the keys controlled by each tab.  The "Ship" tab specifies keys 0..7, the "Command"
// tab specifies keys 8..18, and so on.
static const size_t kKeyIndices[] = { 0, 8, 19, 28, 34, 44 };

static size_t get_tab_num(size_t key) {
    for (size_t i = 1; i < 5; ++i) {
        if (key < kKeyIndices[i]) {
            return i - 1;
        }
    }
    return 4;
}

KeyControlScreen::KeyControlScreen(OptionsScreen::State* state, Preferences* preferences)
        : InterfaceScreen("options/keys", {0, 0, 640, 480}, true),
          _state(state),
          _preferences(preferences),
          _key_start(size()),
          _selected_key(-1),
          _flashed_on(false),
          _tabs(2009),
          _keys(2005) {
    set_tab(SHIP);
}

KeyControlScreen::~KeyControlScreen() { }

void KeyControlScreen::key_down(const KeyDownEvent& event) {
    if (_selected_key >= 0) {
        switch (event.key()) {
          case Keys::ESCAPE:
          case Keys::RETURN:
          case Keys::CAPS_LOCK:
            PlayVolumeSound(kWarningTone, kMediumVolume, kMediumPersistence, kLowPrioritySound);
            _selected_key = -1;
            break;

          default:
            _preferences->set_key(_selected_key, event.key() + 1);
            if (++_selected_key == kKeyIndices[_tab + 1]) {
              _selected_key = -1;
            }
            adjust_interface();
            break;
        }
        update_conflicts();
        adjust_interface();
    }
}

void KeyControlScreen::key_up(const KeyUpEvent& event) {
    static_cast<void>(event);
}

bool KeyControlScreen::next_timer(wall_time& time) {
    if (_next_flash > wall_time()) {
        time = _next_flash;
        return true;
    }
    return false;
}

void KeyControlScreen::fire_timer() {
    wall_time now = antares::now();
    while (_next_flash < now) {
        _next_flash += kFlashTime;
        _flashed_on = !_flashed_on;
    }
    adjust_interface();
}

void KeyControlScreen::adjust_interface() {
    for (size_t i = SHIP_TAB; i <= HOT_KEY_TAB; ++i) {
        dynamic_cast<TabBoxButton&>(mutable_item(i)).hue = AQUA;
    }

    for (size_t i = _key_start; i < size(); ++i) {
        size_t key = kKeyIndices[_tab] + i - _key_start;
        int key_num = _preferences->key(key);
        dynamic_cast<Button&>(mutable_item(i)).key = key_num;
        if (key == _selected_key) {
            dynamic_cast<Button&>(mutable_item(i)).status = kIH_Hilite;
        } else {
            dynamic_cast<Button&>(mutable_item(i)).status = kActive;
        }
        dynamic_cast<Button&>(mutable_item(i)).hue = AQUA;
    }

    if (_flashed_on) {
        for (vector<pair<size_t, size_t>>::const_iterator it = _conflicts.begin();
                it != _conflicts.end(); ++it) {
            flash_on(it->first);
            flash_on(it->second);
        }
    }

    if (_conflicts.empty()) {
        dynamic_cast<Button&>(mutable_item(DONE)).status = kActive;
        dynamic_cast<Button&>(mutable_item(SOUND_CONTROL)).status = kActive;
    } else {
        dynamic_cast<Button&>(mutable_item(DONE)).status = kDimmed;
        dynamic_cast<Button&>(mutable_item(SOUND_CONTROL)).status = kDimmed;
    }
}

void KeyControlScreen::handle_button(Button& button) {
    switch (button.id) {
      case DONE:
      case CANCEL:
      case SOUND_CONTROL:
        *_state = button_state(button.id);
        stack()->pop(this);
        break;

      case SHIP_TAB:
      case COMMAND_TAB:
      case SHORTCUT_TAB:
      case UTILITY_TAB:
      case HOT_KEY_TAB:
        set_tab(button_tab(button.id));
        adjust_interface();
        break;

      default:
        {
            size_t key = kKeyIndices[_tab] + button.id - _key_start;
            if ((kKeyIndices[_tab] <= key) && (key < kKeyIndices[_tab + 1])) {
                // TODO(sfiera): ensure that the button stays highlighted, instead of flashing.
                _selected_key = key;
                adjust_interface();
            } else {
                throw Exception(format("Got unknown button {0}.", button.id));
            }
        }
        break;
    }
}

void KeyControlScreen::overlay() const {
    if (!_conflicts.empty()) {
        const size_t key_one = _conflicts[0].first;
        const size_t key_two = _conflicts[0].second;

        // TODO(sfiera): permit localization.
        String text(format("{0}: {1} conflicts with {2}: {3}",
                    _tabs.at(get_tab_num(key_one)), _keys.at(key_one),
                    _tabs.at(get_tab_num(key_two)), _keys.at(key_two)));

        const TextRect& box = dynamic_cast<const TextRect&>(item(CONFLICT_TEXT));
        vector<inlinePictType> pict;
        Rect bounds = box.bounds();
        Point off = offset();
        bounds.offset(off.h, off.v);
        draw_text_in_rect(bounds, text, box.style, box.hue, pict);
    }
}

OptionsScreen::State KeyControlScreen::button_state(int button) {
    switch (button) {
      case DONE:
        return OptionsScreen::ACCEPT;
      case CANCEL:
        return OptionsScreen::CANCEL;
      case SOUND_CONTROL:
        return OptionsScreen::SOUND_CONTROL;
      default:
        throw Exception(format("unknown key control button {0}", button));
    }
}

KeyControlScreen::Tab KeyControlScreen::button_tab(int button) {
    switch (button) {
      case SHIP_TAB:
        return SHIP;
      case COMMAND_TAB:
        return COMMAND;
      case SHORTCUT_TAB:
        return SHORTCUT;
      case UTILITY_TAB:
        return UTILITY;
      case HOT_KEY_TAB:
        return HOT_KEY;
      default:
        throw Exception(format("unknown key control tab {0}", button));
    }
}

void KeyControlScreen::set_tab(Tab tab) {
    static const int buttons[] = {
        SHIP_TAB,
        COMMAND_TAB,
        SHORTCUT_TAB,
        UTILITY_TAB,
        HOT_KEY_TAB,
    };

    truncate(_key_start);
    for (int i = SHIP_TAB; i <= HOT_KEY_TAB; ++i) {
        TabBoxButton& item = dynamic_cast<TabBoxButton&>(mutable_item(i));
        if (buttons[tab] == i) {
            item.on = true;
            extend(item.tab_content);
        } else {
            item.on = false;
        }
    }
    _tab = tab;
    _selected_key = -1;
}

void KeyControlScreen::update_conflicts() {
    vector<pair<size_t, size_t>> new_conflicts;
    for (size_t i = 0; i < kKeyExtendedControlNum; ++i) {
        for (size_t j = i + 1; j < kKeyExtendedControlNum; ++j) {
            if (_preferences->key(i) == _preferences->key(j)) {
                new_conflicts.push_back(make_pair(i, j));
            }
        }
    }

    _conflicts.swap(new_conflicts);

    if (_conflicts.empty()) {
        _next_flash = wall_time();
        _flashed_on = false;
    } else if (_next_flash == wall_time()) {
        _next_flash = now() + kFlashTime;
        _flashed_on = true;
    }
}

void KeyControlScreen::flash_on(size_t key) {
    if (kKeyIndices[_tab] <= key && key < kKeyIndices[_tab + 1]) {
        Button& item = dynamic_cast<Button&>(mutable_item(key - kKeyIndices[_tab] + _key_start));
        item.hue = GOLD;
    } else {
        Button& item = dynamic_cast<Button&>(mutable_item(SHIP_TAB + get_tab_num(key)));
        item.hue = GOLD;
    }
}

}  // namespace antares
