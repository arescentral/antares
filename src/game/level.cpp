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

#include "game/level.hpp"

#include <set>
#include <sfz/sfz.hpp>

#include "data/condition.hpp"
#include "data/plugin.hpp"
#include "data/races.hpp"
#include "drawing/pix-table.hpp"
#include "game/action.hpp"
#include "game/admiral.hpp"
#include "game/condition.hpp"
#include "game/globals.hpp"
#include "game/initial.hpp"
#include "game/instruments.hpp"
#include "game/labels.hpp"
#include "game/messages.hpp"
#include "game/minicomputer.hpp"
#include "game/motion.hpp"
#include "game/non-player-ship.hpp"
#include "game/player-ship.hpp"
#include "game/starfield.hpp"
#include "game/sys.hpp"
#include "game/vector.hpp"
#include "lang/defines.hpp"
#include "math/macros.hpp"
#include "math/random.hpp"
#include "math/rotation.hpp"
#include "math/units.hpp"

using sfz::range;
using std::set;

namespace antares {

namespace {

#ifdef DATA_COVERAGE
ANTARES_GLOBAL set<int32_t> possible_objects;
ANTARES_GLOBAL set<int32_t> possible_actions;
#endif  // DATA_COVERAGE

enum class Required : bool {
    NO  = false,
    YES = true,
};

void AddBaseObjectActionMedia(const std::vector<Action>& actions, std::bitset<16> all_colors);
void AddActionMedia(const Action& action, std::bitset<16> all_colors);

void AddBaseObjectMedia(
        const NamedHandle<const BaseObject>& base, std::bitset<16> all_colors, Required required) {
    if (base.get()) {
        return;
    }
    if (required == Required::YES) {
        load_object(base);
    } else {
        try {
            load_object(base);
        } catch (...) {
            return;
        }
    }

#ifdef DATA_COVERAGE
    possible_objects.insert(base.number());
#endif  // DATA_COVERAGE

    // Load sprites in all possible colors.
    //
    // For non-thinking objects, just load the gray ones. For thinking
    // objects, load gray, plus all admiral colors.
    //
    // This actually loads more colors than are possible in practice;
    // there was older code that determined which objects were possible
    // per admiral, and which could change ownership, but that wasn’t
    // worth the extra complication.
    std::bitset<16> colors;
    if (base->attributes & kCanThink) {
        colors = all_colors;
    } else {
        colors[0] = true;
    }
    for (int i = 0; i < 16; ++i) {
        if (colors[i] && sprite_resource(*base).has_value()) {
            sys.pix.add(*sprite_resource(*base), Hue(i));
        }
    }

    AddBaseObjectActionMedia(base->destroy.action, all_colors);
    AddBaseObjectActionMedia(base->expire.action, all_colors);
    AddBaseObjectActionMedia(base->create.action, all_colors);
    AddBaseObjectActionMedia(base->collide.action, all_colors);
    AddBaseObjectActionMedia(base->activate.action, all_colors);
    AddBaseObjectActionMedia(base->arrive.action, all_colors);

    for (const sfz::optional<BaseObject::Weapon>* weapon :
         {&base->weapons.pulse, &base->weapons.beam, &base->weapons.special}) {
        if (weapon->has_value()) {
            AddBaseObjectMedia((*weapon)->base, all_colors, Required::YES);
        }
    }
}

void AddBaseObjectActionMedia(const std::vector<Action>& actions, std::bitset<16> all_colors) {
    for (const auto& action : actions) {
        AddActionMedia(action, all_colors);
    }
}

void AddActionMedia(const Action& action, std::bitset<16> all_colors) {
#ifdef DATA_COVERAGE
    possible_actions.insert(action.number());
#endif  // DATA_COVERAGE

    switch (action.type()) {
        case Action::Type::CREATE:
            AddBaseObjectMedia(action.create.base, all_colors, Required::YES);
            break;
        case Action::Type::MORPH:
            AddBaseObjectMedia(action.morph.base, all_colors, Required::YES);
            break;
        case Action::Type::EQUIP:
            AddBaseObjectMedia(action.equip.base, all_colors, Required::YES);
            break;

        case Action::Type::PLAY:
            if (action.play.sound.has_value()) {
                sys.sound.load(*action.play.sound);
            } else {
                for (const auto& s : action.play.any) {
                    sys.sound.load(s.sound);
                }
            }
            break;

        case Action::Type::GROUP:
            for (const auto& a : action.group.of) {
                AddActionMedia(a, all_colors);
            }
            break;

        default: break;
    }
}

static coordPointType rotate_coords(int32_t h, int32_t v, int32_t rotation) {
    mAddAngle(rotation, 90);
    Fixed lcos, lsin;
    GetRotPoint(&lcos, &lsin, rotation);
    coordPointType coord;
    coord.h =
            (kUniversalCenter + (Fixed::from_val(h) * -lcos).val() -
             (Fixed::from_val(v) * -lsin).val());
    coord.v =
            (kUniversalCenter + (Fixed::from_val(h) * -lsin).val() +
             (Fixed::from_val(v) * -lcos).val());
    return coord;
}

void GetInitialCoord(Handle<const Initial> initial, coordPointType* coord, int32_t rotation) {
    *coord = rotate_coords(initial->at.h, initial->at.v, rotation);
}

}  // namespace

template <typename Players>
static void construct_players(const Players& players) {
    int i = 0;
    for (const auto& player : players) {
        auto admiral = Admiral::make(i++, player);
        if (admiral->attributes() & kAIsHuman) {
            g.admiral = admiral;
        }
        admiral->pay(Cash{Fixed::from_long(5000)});
        load_race(admiral->race());
    }
}

LoadState start_construct_level(const Level& level) {
    ResetAllSpaceObjects();
    reset_action_queue();
    Vectors::reset();
    ResetAllSprites();
    Label::reset();
    ResetInstruments();
    Admiral::reset();
    ResetAllDestObjectData();
    ResetMotionGlobals();
    plug.races.clear();
    plug.objects.clear();
    gAbsoluteScale = kTimesTwoScale;
    g.sync         = 0;

    g.level = &level;

    if (g.level->base.angle.has_value()) {
        g.angle = *g.level->base.angle;
    } else {
        g.angle = g.random.next(ROT_POS);
    }

    g.victor       = Admiral::none();
    g.next_level   = nullptr;
    g.victory_text = sfz::nullopt;

    switch (g.level->type()) {
        case Level::Type::NONE: throw std::runtime_error("level with type NONE?");
        case Level::Type::DEMO: construct_players(g.level->demo.players); break;
        case Level::Type::SOLO: construct_players(g.level->solo.players); break;
        case Level::Type::NET: throw std::runtime_error("can’t construct net player");
    }
    // *** END INIT ADMIRALS ***

    g.initials.clear();
    g.initials.resize(Initial::all().size());
    g.initial_ids.clear();
    g.initial_ids.resize(Initial::all().size());
    g.condition_enabled.clear();
    g.condition_enabled.resize(g.level->base.conditions.size());

    ///// FIRST SELECT WHAT MEDIA WE NEED TO USE:

    sys.pix.reset();
    sys.sound.reset();

    LoadState s;
    s.max = Initial::all().size() * 3L + 1 +
            g.level->base.start_time.value_or(secs(0))
                    .count();  // for each run through the initial num

    return s;
}

static void load_blessed_objects(std::bitset<16> all_colors) {
    // Load the four blessed objects.  The player's body is needed
    // in all colors; the other three are needed only as neutral
    // objects by default.
    const NamedHandle<const BaseObject>* blessed[] = {
            &kEnergyBlob, &kWarpInFlare, &kWarpOutFlare, &kPlayerBody,
    };
    for (auto id : blessed) {
        AddBaseObjectMedia(*id, all_colors, Required::YES);
    }
}

static void load_initial(Handle<const Initial> initial, std::bitset<16> all_colors) {
    Handle<Admiral>               owner           = initial->owner.value_or(Admiral::none());
    const BuildableObject&        buildableObject = initial->base;
    NamedHandle<const BaseObject> baseObject;
    if (owner.get()) {
        baseObject = get_buildable_object_handle(buildableObject, owner->race());
    } else {
        baseObject = NamedHandle<const BaseObject>{buildableObject.name.copy()};
    }

    // Load the media for this object
    AddBaseObjectMedia(baseObject, all_colors, Required::YES);

    // make sure we're not overriding the sprite
    if (initial->override_.sprite.has_value()) {
        if (baseObject->attributes & kCanThink) {
            sys.pix.add(*initial->override_.sprite, GetAdmiralColor(owner));
        } else {
            sys.pix.add(*initial->override_.sprite, Hue::GRAY);
        }
    }

    // check any objects this object can build
    for (int j = 0; j < initial->build.size(); j++) {
        // check for each player
        for (auto a : Admiral::all()) {
            if (a->active()) {
                NamedHandle<const BaseObject> baseObject =
                        get_buildable_object_handle(initial->build[j], a->race());
                AddBaseObjectMedia(baseObject, all_colors, Required::NO);
            }
        }
    }
}

static void load_condition(Handle<const Condition> condition, std::bitset<16> all_colors) {
    for (const auto& action : condition->action) {
        AddActionMedia(action, all_colors);
    }
    g.condition_enabled[condition.number()] = !condition->disabled.value_or(false);
}

static void run_game_1s() {
    game_ticks start_time = game_ticks(-g.level->base.start_time.value_or(secs(0)));
    do {
        g.time += kMajorTick;
        MoveSpaceObjects(kMajorTick);
        NonplayerShipThink();
        AdmiralThink();
        execute_action_queue();
        CollideSpaceObjects();
        if (((g.time - start_time) % kConditionTick) == ticks(0)) {
            CheckLevelConditions();
        }
        CullSprites();
        Vectors::cull();
    } while ((g.time.time_since_epoch() % secs(1)) != ticks(0));
}

void construct_level(LoadState* state) {
    int32_t         step = state->step;
    std::bitset<16> all_colors;
    all_colors[0] = true;
    for (auto adm : Admiral::all()) {
        if (adm->active()) {
            all_colors[static_cast<int>(GetAdmiralColor(adm))] = true;
        }
    }

    if (step == 0) {
        load_blessed_objects(all_colors);
        load_initial(Handle<const Initial>(step), all_colors);
    } else if (step < Initial::all().size()) {
        load_initial(Handle<const Initial>(step), all_colors);
    } else if (step == Initial::all().size()) {
        // add media for all condition actions
        step -= Initial::all().size();
        for (auto c : Condition::all()) {
            load_condition(c, all_colors);
        }
        create_initial(Handle<const Initial>(step));
    } else if (step < (2 * Initial::all().size())) {
        step -= Initial::all().size();
        create_initial(Handle<const Initial>(step));
    } else if (step < (3 * Initial::all().size())) {
        // double back and set up any defined initial destinations
        step -= (2 * Initial::all().size());
        set_initial_destination(Handle<const Initial>(step), false);
    } else if (step == (3 * Initial::all().size())) {
        RecalcAllAdmiralBuildData();  // set up all the admiral's destination objects
        Messages::clear();
        g.time = game_ticks(-g.level->base.start_time.value_or(secs(0)));
    } else {
        run_game_1s();
    }
    ++state->step;
    if (state->step == state->max) {
        state->done = true;
    }
    return;
}

void DeclareWinner(Handle<Admiral> whichPlayer, const Level* nextLevel, pn::string_view text) {
    if (!whichPlayer.get()) {
        // if there's no winner, we want to exit immediately
        g.next_level   = nextLevel;
        g.victory_text = sfz::make_optional(text.copy());
        g.game_over    = true;
        g.game_over_at = g.time;
    } else {
        if (!g.victor.get()) {
            g.victor       = whichPlayer;
            g.victory_text = sfz::make_optional(text.copy());
            g.next_level   = nextLevel;
            if (!g.game_over) {
                g.game_over    = true;
                g.game_over_at = g.time + secs(3);
            }
        }
    }
}

// GetLevelFullScaleAndCorner:
//  This is really just for the mission briefing.  It calculates the best scale
//  at which to show the entire scenario.

void GetLevelFullScaleAndCorner(
        int32_t rotation, coordPointType* corner, int32_t* scale, Rect* bounds) {
    int32_t biggest, mustFit;
    Point   coord, otherCoord, tempCoord;

    mustFit = bounds->bottom - bounds->top;
    if ((bounds->right - bounds->left) < mustFit)
        mustFit = bounds->right - bounds->left;

    biggest = 0;
    for (const auto& initial : Initial::all()) {
        if (!initial->hide.value_or(false)) {
            GetInitialCoord(initial, reinterpret_cast<coordPointType*>(&coord), g.angle);

            for (const auto& other : Initial::all()) {
                GetInitialCoord(other, reinterpret_cast<coordPointType*>(&otherCoord), g.angle);

                if (ABS(otherCoord.h - coord.h) > biggest) {
                    biggest = ABS(otherCoord.h - coord.h);
                }
                if (ABS(otherCoord.v - coord.v) > biggest) {
                    biggest = ABS(otherCoord.v - coord.v);
                }
            }
        }
    }

    biggest += biggest >> 2L;

    *scale = SCALE_SCALE * mustFit;
    *scale /= biggest;

    otherCoord.h = kUniversalCenter;
    otherCoord.v = kUniversalCenter;
    coord.h      = kUniversalCenter;
    coord.v      = kUniversalCenter;
    for (const auto& initial : Initial::all()) {
        if (!initial->hide.value_or(false)) {
            GetInitialCoord(initial, reinterpret_cast<coordPointType*>(&tempCoord), g.angle);

            if (tempCoord.h < coord.h) {
                coord.h = tempCoord.h;
            }
            if (tempCoord.v < coord.v) {
                coord.v = tempCoord.v;
            }

            if (tempCoord.h > otherCoord.h) {
                otherCoord.h = tempCoord.h;
            }
            if (tempCoord.v > otherCoord.v) {
                otherCoord.v = tempCoord.v;
            }
        }
    }

    biggest = bounds->right - bounds->left;
    biggest *= SCALE_SCALE;
    biggest /= *scale;
    biggest /= 2;
    corner->h = (coord.h + (otherCoord.h - coord.h) / 2) - biggest;
    biggest   = (bounds->bottom - bounds->top);
    biggest *= SCALE_SCALE;
    biggest /= *scale;
    biggest /= 2;
    corner->v = (coord.v + (otherCoord.v - coord.v) / 2) - biggest;
}

coordPointType Translate_Coord_To_Level_Rotation(int32_t h, int32_t v) {
    return rotate_coords(h, v, g.angle);
}

}  // namespace antares
