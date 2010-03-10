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

#include "OptionsScreen.hpp"

#include <set>
#include "sfz/Exception.hpp"
#include "AresMain.hpp"
#include "AresPreferences.hpp"
#include "CardStack.hpp"
#include "ColorTranslation.hpp"
#include "DirectText.hpp"
#include "Error.hpp"
#include "KeyMapTranslation.hpp"
#include "Ledger.hpp"
#include "OffscreenGWorld.hpp"
#include "Options.hpp"
#include "PlayerInterface.hpp"
#include "PlayerInterfaceItems.hpp"
#include "RetroText.hpp"
#include "ScenarioMaker.hpp"
#include "StringList.hpp"
#include "Time.hpp"
#include "VideoDriver.hpp"

using sfz::Exception;
using sfz::String;
using sfz::format;
using std::make_pair;
using std::pair;
using std::set;
using std::vector;

namespace antares {

extern PixMap* gRealWorld;

OptionsScreen::OptionsScreen()
    : _state(SOUND_CONTROL),
      _preferences(new Preferences(*globals()->gPreferencesData)) { }

void OptionsScreen::become_front() {
    switch (_state) {
      case SOUND_CONTROL:
        stack()->push(new SoundControlScreen(&_state, _preferences.get()));
        break;

      case KEY_CONTROL:
        stack()->push(new KeyControlScreen(&_state, _preferences.get()));
        break;

      case ACCEPT:
        for (int i = 0; i < kKeyExtendedControlNum; ++i) {
            GetKeyMapFromKeyNum(_preferences->keyMap[i], globals()->gKeyControl[i]);
        }
        globals()->gPreferencesData.reset(_preferences.release());
        stack()->pop(this);
        break;

      case CANCEL:
        stack()->pop(this);
        break;
    }
}

namespace {

const int kSoundControlScreenResID = 5007;

}  // namespace

SoundControlScreen::SoundControlScreen(OptionsScreen::State* state, Preferences* preferences)
        : InterfaceScreen(kSoundControlScreenResID, gRealWorld->bounds(), true),
          _state(state),
          _preferences(preferences) { }

SoundControlScreen::~SoundControlScreen() { }

void SoundControlScreen::become_front() {
    InterfaceScreen::become_front();
    VideoDriver::driver()->set_game_state(OPTIONS_INTERFACE);
}

void SoundControlScreen::adjust_interface() {
    mutable_item(IDLE_MUSIC)->item.checkboxButton.on = _preferences->options & kOptionMusicIdle;
    mutable_item(GAME_MUSIC)->item.checkboxButton.on = _preferences->options & kOptionMusicPlay;
    mutable_item(SPEECH_ON)->item.checkboxButton.on = _preferences->options & kOptionSpeechOn;

    if (globals()->gOptions & kOptionSpeechAvailable) {
        mutable_item(SPEECH_ON)->set_status(kActive);
    } else {
        mutable_item(SPEECH_ON)->set_status(kDimmed);
    }

    if (_preferences->volume > 0) {
        mutable_item(VOLUME_DOWN)->set_status(kActive);
    } else {
        mutable_item(VOLUME_DOWN)->set_status(kDimmed);
    }

    if (_preferences->volume < kMaxVolumePreference) {
        mutable_item(VOLUME_UP)->set_status(kActive);
    } else {
        mutable_item(VOLUME_UP)->set_status(kDimmed);
    }
}

void SoundControlScreen::handle_button(int button) {
    switch (button) {
      case GAME_MUSIC:
        _preferences->options ^= kOptionMusicPlay;
        adjust_interface();
        draw();
        break;

      case IDLE_MUSIC:
        _preferences->options ^= kOptionMusicIdle;
        // TODO(sfiera): switch music on or off.
        adjust_interface();
        draw();
        break;

      case SPEECH_ON:
        _preferences->options ^= kOptionSpeechOn;
        adjust_interface();
        draw();
        break;

      case VOLUME_DOWN:
        if (_preferences->volume > 0) {
            --_preferences->volume;
        }
        adjust_interface();
        draw();
        break;

      case VOLUME_UP:
        if (_preferences->volume < kMaxVolumePreference) {
            ++_preferences->volume;
        }
        adjust_interface();
        draw();
        break;

      case DONE:
      case CANCEL:
      case KEY_CONTROL:
        *_state = button_state(button);
        stack()->pop(this);
        break;

      default:
        fail("Got unknown button %d.", button);
    }
}

void SoundControlScreen::draw() const {
    InterfaceScreen::draw();

    const int volume = _preferences->volume;
    Rect bounds = item(VOLUME_BOX).bounds;

    const int notch_width = bounds.width() / kMaxVolumePreference;
    const int notch_height = bounds.height() - 4;
    Rect notch_bounds(0, 0, notch_width * kMaxVolumePreference, notch_height);
    notch_bounds.center_in(bounds);

    Rect notch(notch_bounds.left, notch_bounds.top,
            notch_bounds.left + notch_width, notch_bounds.bottom);
    notch.inset(3, 6);

    for (int i = 0; i < kMaxVolumePreference; ++i) {
        RgbColor color;
        if (i < volume) {
            GetRGBTranslateColorShade(&color, PALE_PURPLE, 2 * (i + 1));
        } else {
            color = RgbColor::kBlack;
        }
        pix()->view(notch).fill(color);
        notch.offset(notch_width, 0);
    }
    gRealWorld->copy(*pix());
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
        throw Exception("unknown sound control button {0}", button);
    }
}

namespace {

// Resource IDs of the various interfaces used in the key control interface.
const int kKeyControlScreenResID    = 5030;
const int kKeyControlTabIds[] = { 5031, 5032, 5033, 5034, 5035 };

const double kFlashTime = 0.2;

// Indices of the keys controlled by each tab.  The "Ship" tab specifies keys 0..7, the "Command"
// tab specifies keys 8..18, and so on.
const size_t kKeyIndices[] = { 0, 8, 19, 28, 34, 44 };

size_t get_tab_num(size_t key) {
    for (size_t i = 1; i < 5; ++i) {
        if (key < kKeyIndices[i]) {
            return i - 1;
        }
    }
    return 4;
}

}  // namespace

KeyControlScreen::KeyControlScreen(OptionsScreen::State* state, Preferences* preferences)
        : InterfaceScreen(kKeyControlScreenResID, gRealWorld->bounds(), true),
          _state(state),
          _preferences(preferences),
          _key_start(size()),
          _selected_key(-1),
          _next_flash(0.0),
          _flashed_on(false) {
    set_tab(SHIP);
}

KeyControlScreen::~KeyControlScreen() { }

void KeyControlScreen::become_front() {
    InterfaceScreen::become_front();
    VideoDriver::driver()->set_game_state(KEY_CONTROL_INTERFACE);
}

void KeyControlScreen::key_down(const KeyDownEvent& event) {
    if (_selected_key >= 0) {
        uint32_t key = event.key();
        if ((key == 53) || (key == 36) || (key == 57)) {  // ESC, RTRN, CAPS.
            // beep angrily.
            _selected_key = -1;
        } else {
            _preferences->keyMap[_selected_key] = key + 1;
            _selected_key = -1;  // TODO(sfiera): select next key.
        }
        update_conflicts();
        adjust_interface();
        draw();
    }
}

void KeyControlScreen::key_up(const KeyUpEvent& event) {
    static_cast<void>(event);
}

double KeyControlScreen::next_timer() {
    return _next_flash;
}

void KeyControlScreen::fire_timer() {
    double now = now_secs();
    while (_next_flash < now) {
        _next_flash += kFlashTime;
        _flashed_on = !_flashed_on;
    }
    adjust_interface();
    draw();
}

void KeyControlScreen::adjust_interface() {
    for (size_t i = SHIP_TAB; i <= HOT_KEY_TAB; ++i) {
        mutable_item(i)->color = AQUA;
    }

    for (size_t i = _key_start; i < size(); ++i) {
        size_t key = kKeyIndices[_tab] + i - _key_start;
        int key_num = _preferences->keyMap[key];
        mutable_item(i)->item.plainButton.key = key_num;
        if (key == _selected_key) {
            mutable_item(i)->set_status(kIH_Hilite);
        } else {
            mutable_item(i)->set_status(kActive);
        }
        mutable_item(i)->color = AQUA;
    }

    if (_flashed_on) {
        for (vector<pair<size_t, size_t> >::const_iterator it = _conflicts.begin();
                it != _conflicts.end(); ++it) {
            flash_on(it->first);
            flash_on(it->second);
        }
    }

    if (_conflicts.empty()) {
        mutable_item(DONE)->set_status(kActive);
        mutable_item(SOUND_CONTROL)->set_status(kActive);
    } else {
        mutable_item(DONE)->set_status(kDimmed);
        mutable_item(SOUND_CONTROL)->set_status(kDimmed);
    }
}

void KeyControlScreen::handle_button(int button) {
    switch (button) {
      case DONE:
      case CANCEL:
      case SOUND_CONTROL:
        *_state = button_state(button);
        stack()->pop(this);
        break;

      case SHIP_TAB:
      case COMMAND_TAB:
      case SHORTCUT_TAB:
      case UTILITY_TAB:
      case HOT_KEY_TAB:
        for (int i = SHIP_TAB; i <= HOT_KEY_TAB; ++i) {
            mutable_item(i)->item.radioButton.on = (button == i);
        }
        set_tab(button_tab(button));
        adjust_interface();
        draw();
        break;

      default:
        {
            size_t key = kKeyIndices[_tab] + button - _key_start;
            if ((kKeyIndices[_tab] <= key) && (key < kKeyIndices[_tab + 1])) {
                // TODO(sfiera): ensure that the button stays highlighted, instead of flashing.
                _selected_key = key;
                adjust_interface();
                draw();
            } else {
                throw Exception("Got unknown button {0}.", button);
            }
        }
        break;
    }
}

void KeyControlScreen::draw() const {
    InterfaceScreen::draw();

    if (!_conflicts.empty()) {
        const size_t key_one = _conflicts[0].first;
        const size_t key_two = _conflicts[0].second;
        String text;
        StringList tabs(2009);
        StringList keys(2005);

        // TODO(sfiera): permit localization.
        format(&text, "{0}: {1} conflicts with {2}: {3}",
                tabs.at(get_tab_num(key_one)), keys.at(key_one),
                tabs.at(get_tab_num(key_two)), keys.at(key_two));
        const interfaceItemType& box = item(CONFLICT_TEXT);
        DrawInterfaceTextInRect(box.bounds, text, box.style, box.color, pix(), NULL);
    }

    gRealWorld->copy(*pix());
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
        throw Exception("unknown key control button {0}", button);
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
        throw Exception("unknown key control tab {0}", button);
    }
}

void KeyControlScreen::set_tab(Tab tab) {
    _tab = tab;
    _selected_key = -1;
    truncate(_key_start);
    extend(kKeyControlTabIds[tab], TAB_BOX);
}

void KeyControlScreen::update_conflicts() {
    vector<pair<size_t, size_t> > new_conflicts;
    for (size_t i = 0; i < kKeyExtendedControlNum; ++i) {
        for (size_t j = i + 1; j < kKeyExtendedControlNum; ++j) {
            if (_preferences->keyMap[i] == _preferences->keyMap[j]) {
                new_conflicts.push_back(make_pair(i, j));
            }
        }
    }

    _conflicts.swap(new_conflicts);

    if (_conflicts.empty()) {
        _next_flash = 0.0;
        _flashed_on = false;
    } else if (_next_flash == 0.0) {
        _next_flash = now_secs() + kFlashTime;
        _flashed_on = true;
    }
}

void KeyControlScreen::flash_on(size_t key) {
    if (kKeyIndices[_tab] <= key && key < kKeyIndices[_tab + 1]) {
        mutable_item(key - kKeyIndices[_tab] + _key_start)->color = GOLD;
    } else {
        mutable_item(SHIP_TAB + get_tab_num(key))->color = GOLD;
    }
}

}  // namespace antares
