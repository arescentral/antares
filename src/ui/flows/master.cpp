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

#include "ui/flows/master.hpp"

#include "drawing/text.hpp"
#include "game/admiral.hpp"
#include "game/cheat.hpp"
#include "game/cursor.hpp"
#include "game/globals.hpp"
#include "game/instruments.hpp"
#include "game/labels.hpp"
#include "game/messages.hpp"
#include "game/motion.hpp"
#include "game/scenario-maker.hpp"
#include "game/space-object.hpp"
#include "math/rotation.hpp"
#include "sound/driver.hpp"
#include "sound/music.hpp"
#include "ui/interface-handling.hpp"
#include "ui/screens/main.hpp"
#include "ui/screens/scroll-text.hpp"
#include "video/transitions.hpp"

namespace antares {

namespace {

class TitleScreenFade : public PictFade {
  public:
    TitleScreenFade(bool* fast)
            : PictFade(502, fast),
              _fast(fast) { }

  protected:
    virtual int64_t fade_time() const {
        return (*_fast ? 1e6 : 5e6) / 3;
    }

    virtual int64_t display_time() const {
        return *_fast ? 1e6 : 5e6;
    }

    virtual bool skip() const {
        return false;
    }

  private:
    bool* _fast;
};

}  // namespace

Master::Master(int32_t seed):
        _state(START),
        _seed(seed),
        _skipped(false) { }

void Master::become_front() {
    switch (_state) {
      case START:
        init();
        _state = PUBLISHER_PICT;
        // We don't have permission to display the Ambrosia logo.
        // stack()->push(new PictFade(2000, &_skipped));
        // break;

      case PUBLISHER_PICT:
        _state = EGO_PICT;
        if (!_skipped) {
            stack()->push(new PictFade(2001, &_skipped));
            break;
        }
        // fall through.

      case EGO_PICT:
        _state = TITLE_SCREEN_PICT;
        stack()->push(new TitleScreenFade(&_skipped));
        break;

      case TITLE_SCREEN_PICT:
        _state = INTRO_SCROLL;
        // TODO(sfiera): prevent the intro screen from displaying on subsequent launches.
        stack()->push(new ScrollTextScreen(5600, 450, 15.0));
        break;

      case INTRO_SCROLL:
        _state = MAIN_SCREEN;
        stack()->push(new MainScreen);
        break;

      case MAIN_SCREEN:
        // When the main screen returns, exit loop.
        stack()->pop(this);
        break;
    }
}

void Master::draw() const { }

void Master::init() {
    RgbColor                initialFadeColor;

    init_globals();

    SoundDriver::driver()->set_global_volume(Preferences::preferences()->volume());

    world = Rect(Point(0, 0), Preferences::preferences()->screen_size());
    play_screen = Rect(
        world.left + kLeftPanelWidth, world.top,
        world.right - kRightPanelWidth, world.bottom);
    viewport = play_screen;

    initialFadeColor.red = initialFadeColor.green = initialFadeColor.blue = 0;

    RotationInit();
    g.random.seed = _seed;

    InitDirectText();
    Label::init();
    Messages::init();
    InstrumentInit();
    SpriteHandlingInit();
    AresCheatInit();
    ScenarioMakerInit();
    SpaceObjectHandlingInit();  // MUST be after ScenarioMakerInit()
    InitSoundFX();
    MusicInit();
    InitMotion();
    Admiral::init();
    Beams::init();

    if (Preferences::preferences()->play_idle_music()) {
        LoadSong( kTitleSongID);
        SetSongVolume( kMaxMusicVolume);
        PlaySong();
    }
}

}  // namespace antares
