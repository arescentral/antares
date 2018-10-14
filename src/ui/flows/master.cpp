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

#include "ui/flows/master.hpp"

#include "config/preferences.hpp"
#include "data/plugin.hpp"
#include "data/resource.hpp"
#include "game/admiral.hpp"
#include "game/cheat.hpp"
#include "game/cursor.hpp"
#include "game/globals.hpp"
#include "game/instruments.hpp"
#include "game/labels.hpp"
#include "game/level.hpp"
#include "game/messages.hpp"
#include "game/motion.hpp"
#include "game/space-object.hpp"
#include "game/sys.hpp"
#include "game/vector.hpp"
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
    TitleScreenFade(bool* fast) : PictFade(&plug.splash, fast), _fast(fast) {}

  protected:
    virtual usecs fade_time() const { return ticks(*_fast ? 20 : 100); }

    virtual usecs display_time() const { return secs(*_fast ? 1 : 5); }

    virtual bool skip() const { return false; }

  private:
    bool* _fast;
};

}  // namespace

Master::Master(int32_t seed) : _state(START), _seed(seed), _skipped(false) {}

void Master::become_front() {
    switch (_state) {
        case START:
            init();
            _state = PUBLISHER_PICT;
            if (_publisher_screen) {
                stack()->push(new PictFade(&_publisher_screen, &_skipped));
                break;
            }
            [[clang::fallthrough]];

        case PUBLISHER_PICT:
            _state = EGO_PICT;
            if (!_skipped) {
                stack()->push(new PictFade(&_ego_screen, &_skipped));
                break;
            }
            [[clang::fallthrough]];

        case EGO_PICT:
            _state = TITLE_SCREEN_PICT;
            stack()->push(new TitleScreenFade(&_skipped));
            break;

        case TITLE_SCREEN_PICT:
            _state = INTRO_SCROLL;
            // TODO(sfiera): prevent the intro screen from displaying on subsequent launches.
            if (plug.info.intro.has_value()) {
                stack()->push(new ScrollTextScreen(*plug.info.intro, 450, kSlowScrollInterval));
                break;
            }
            [[clang::fallthrough]];

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

void Master::draw() const {}

void Master::init() {
    RgbColor initialFadeColor;

    init_globals();

    sys.audio->set_global_volume(sys.prefs->volume());

    initialFadeColor.red = initialFadeColor.green = initialFadeColor.blue = 0;

    g.random.seed = _seed;

    sys_init();
    Label::init();
    Messages::init();
    InstrumentInit();
    SpriteHandlingInit();
    PluginInit();
    SpaceObjectHandlingInit();  // MUST be after ScenarioMakerInit()
    InitMotion();
    Admiral::init();
    Vectors::init();

    sys.music.play(Music::IDLE, Music::title_song);

    _publisher_screen = nullptr;
    _ego_screen       = Resource::texture("credit");
}

}  // namespace antares
