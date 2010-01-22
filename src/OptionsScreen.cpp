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

#include "AresMain.hpp"
#include "AresPreferences.hpp"
#include "CardStack.hpp"
#include "ColorTranslation.hpp"
#include "DirectText.hpp"
#include "Error.hpp"
#include "KeySetupScreen.hpp"
#include "Ledger.hpp"
#include "OffscreenGWorld.hpp"
#include "Options.hpp"
#include "PlayerInterface.hpp"
#include "RetroText.hpp"
#include "ScenarioMaker.hpp"
#include "VideoDriver.hpp"

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
        if (Key_Setup_Screen_Do()) {
            _state = ACCEPT;
        } else {
            _state = SOUND_CONTROL;
        }
        become_front();
        break;

      case ACCEPT:
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
        *_state = OptionsScreen::ACCEPT;
        stack()->pop(this);
        break;

      case CANCEL:
        *_state = OptionsScreen::CANCEL;
        stack()->pop(this);
        break;

      case KEY_CONTROL:
        *_state = OptionsScreen::KEY_CONTROL;
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

}  // namespace antares
