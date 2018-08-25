// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2015-2017 The Antares Authors
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

#include "data/action.hpp"

#include <sfz/sfz.hpp>

#include "data/base-object.hpp"
#include "data/field.hpp"
#include "data/initial.hpp"
#include "data/level.hpp"
#include "data/resource.hpp"

namespace antares {

// clang-format off
#define COMMON_ACTION_FIELDS                                                                      \
            {"type", &ActionBase::type},                                                          \
            {"reflexive", &ActionBase::reflexive},                                                \
            {"if", &ActionBase::filter},                                                          \
            {"override", &ActionBase::override_}
// clang-format on

Action::Type Action::type() const { return base.type; }

Action::Action(AgeAction a) : age(std::move(a)) {}
Action::Action(AssumeAction a) : assume(std::move(a)) {}
Action::Action(CapSpeedAction a) : cap_speed(std::move(a)) {}
Action::Action(CaptureAction a) : capture(std::move(a)) {}
Action::Action(CheckAction a) : check(std::move(a)) {}
Action::Action(CloakAction a) : cloak(std::move(a)) {}
Action::Action(ConditionAction a) : condition(std::move(a)) {}
Action::Action(CreateAction a) : create(std::move(a)) {}
Action::Action(DelayAction a) : delay(std::move(a)) {}
Action::Action(DestroyAction a) : destroy(std::move(a)) {}
Action::Action(DisableAction a) : disable(std::move(a)) {}
Action::Action(EnergizeAction a) : energize(std::move(a)) {}
Action::Action(EquipAction a) : equip(std::move(a)) {}
Action::Action(FireAction a) : fire(std::move(a)) {}
Action::Action(FlashAction a) : flash(std::move(a)) {}
Action::Action(GroupAction a) : group(std::move(a)) {}
Action::Action(HealAction a) : heal(std::move(a)) {}
Action::Action(HoldAction a) : hold(std::move(a)) {}
Action::Action(KeyAction a) : key(std::move(a)) {}
Action::Action(LandAction a) : land(std::move(a)) {}
Action::Action(MessageAction a) : message(std::move(a)) {}
Action::Action(MorphAction a) : morph(std::move(a)) {}
Action::Action(MoveAction a) : move(std::move(a)) {}
Action::Action(OccupyAction a) : occupy(std::move(a)) {}
Action::Action(PayAction a) : pay(std::move(a)) {}
Action::Action(PlayAction a) : play(std::move(a)) {}
Action::Action(PushAction a) : push(std::move(a)) {}
Action::Action(RemoveAction a) : remove(std::move(a)) {}
Action::Action(RevealAction a) : reveal(std::move(a)) {}
Action::Action(ScoreAction a) : score(std::move(a)) {}
Action::Action(SelectAction a) : select(std::move(a)) {}
Action::Action(SlowAction a) : slow(std::move(a)) {}
Action::Action(SparkAction a) : spark(std::move(a)) {}
Action::Action(SpeedAction a) : speed(std::move(a)) {}
Action::Action(SpinAction a) : spin(std::move(a)) {}
Action::Action(StopAction a) : stop(std::move(a)) {}
Action::Action(TargetAction a) : target(std::move(a)) {}
Action::Action(ThrustAction a) : thrust(std::move(a)) {}
Action::Action(WarpAction a) : warp(std::move(a)) {}
Action::Action(WinAction a) : win(std::move(a)) {}
Action::Action(ZoomAction a) : zoom(std::move(a)) {}

Action::Action(Action&& a) {
    switch (a.type()) {
        case Action::Type::AGE: new (this) Action(std::move(a.age)); break;
        case Action::Type::ASSUME: new (this) Action(std::move(a.assume)); break;
        case Action::Type::CAP_SPEED: new (this) Action(std::move(a.cap_speed)); break;
        case Action::Type::CAPTURE: new (this) Action(std::move(a.capture)); break;
        case Action::Type::CHECK: new (this) Action(std::move(a.check)); break;
        case Action::Type::CLOAK: new (this) Action(std::move(a.cloak)); break;
        case Action::Type::CONDITION: new (this) Action(std::move(a.condition)); break;
        case Action::Type::CREATE: new (this) Action(std::move(a.create)); break;
        case Action::Type::DELAY: new (this) Action(std::move(a.delay)); break;
        case Action::Type::DESTROY: new (this) Action(std::move(a.destroy)); break;
        case Action::Type::DISABLE: new (this) Action(std::move(a.disable)); break;
        case Action::Type::ENERGIZE: new (this) Action(std::move(a.energize)); break;
        case Action::Type::EQUIP: new (this) Action(std::move(a.equip)); break;
        case Action::Type::FIRE: new (this) Action(std::move(a.fire)); break;
        case Action::Type::FLASH: new (this) Action(std::move(a.flash)); break;
        case Action::Type::GROUP: new (this) Action(std::move(a.group)); break;
        case Action::Type::HEAL: new (this) Action(std::move(a.heal)); break;
        case Action::Type::HOLD: new (this) Action(std::move(a.hold)); break;
        case Action::Type::KEY: new (this) Action(std::move(a.key)); break;
        case Action::Type::LAND: new (this) Action(std::move(a.land)); break;
        case Action::Type::MESSAGE: new (this) Action(std::move(a.message)); break;
        case Action::Type::MORPH: new (this) Action(std::move(a.morph)); break;
        case Action::Type::MOVE: new (this) Action(std::move(a.move)); break;
        case Action::Type::OCCUPY: new (this) Action(std::move(a.occupy)); break;
        case Action::Type::PAY: new (this) Action(std::move(a.pay)); break;
        case Action::Type::PLAY: new (this) Action(std::move(a.play)); break;
        case Action::Type::PUSH: new (this) Action(std::move(a.push)); break;
        case Action::Type::REMOVE: new (this) Action(std::move(a.remove)); break;
        case Action::Type::REVEAL: new (this) Action(std::move(a.reveal)); break;
        case Action::Type::SCORE: new (this) Action(std::move(a.score)); break;
        case Action::Type::SELECT: new (this) Action(std::move(a.select)); break;
        case Action::Type::SLOW: new (this) Action(std::move(a.slow)); break;
        case Action::Type::SPARK: new (this) Action(std::move(a.spark)); break;
        case Action::Type::SPEED: new (this) Action(std::move(a.speed)); break;
        case Action::Type::SPIN: new (this) Action(std::move(a.spin)); break;
        case Action::Type::STOP: new (this) Action(std::move(a.stop)); break;
        case Action::Type::TARGET: new (this) Action(std::move(a.target)); break;
        case Action::Type::THRUST: new (this) Action(std::move(a.thrust)); break;
        case Action::Type::WARP: new (this) Action(std::move(a.warp)); break;
        case Action::Type::WIN: new (this) Action(std::move(a.win)); break;
        case Action::Type::ZOOM: new (this) Action(std::move(a.zoom)); break;
    }
}

Action& Action::operator=(Action&& a) {
    this->~Action();
    new (this) Action(std::move(a));
    return *this;
}

Action::~Action() {
    switch (type()) {
        case Action::Type::AGE: age.~AgeAction(); break;
        case Action::Type::ASSUME: assume.~AssumeAction(); break;
        case Action::Type::CAP_SPEED: cap_speed.~CapSpeedAction(); break;
        case Action::Type::CAPTURE: capture.~CaptureAction(); break;
        case Action::Type::CHECK: check.~CheckAction(); break;
        case Action::Type::CLOAK: cloak.~CloakAction(); break;
        case Action::Type::CONDITION: condition.~ConditionAction(); break;
        case Action::Type::CREATE: create.~CreateAction(); break;
        case Action::Type::DELAY: delay.~DelayAction(); break;
        case Action::Type::DESTROY: destroy.~DestroyAction(); break;
        case Action::Type::DISABLE: disable.~DisableAction(); break;
        case Action::Type::ENERGIZE: energize.~EnergizeAction(); break;
        case Action::Type::EQUIP: equip.~EquipAction(); break;
        case Action::Type::FIRE: fire.~FireAction(); break;
        case Action::Type::FLASH: flash.~FlashAction(); break;
        case Action::Type::GROUP: group.~GroupAction(); break;
        case Action::Type::HEAL: heal.~HealAction(); break;
        case Action::Type::HOLD: hold.~HoldAction(); break;
        case Action::Type::KEY: key.~KeyAction(); break;
        case Action::Type::LAND: land.~LandAction(); break;
        case Action::Type::MESSAGE: message.~MessageAction(); break;
        case Action::Type::MORPH: morph.~MorphAction(); break;
        case Action::Type::MOVE: move.~MoveAction(); break;
        case Action::Type::OCCUPY: occupy.~OccupyAction(); break;
        case Action::Type::PAY: pay.~PayAction(); break;
        case Action::Type::PLAY: play.~PlayAction(); break;
        case Action::Type::PUSH: push.~PushAction(); break;
        case Action::Type::REMOVE: remove.~RemoveAction(); break;
        case Action::Type::REVEAL: reveal.~RevealAction(); break;
        case Action::Type::SCORE: score.~ScoreAction(); break;
        case Action::Type::SELECT: select.~SelectAction(); break;
        case Action::Type::SLOW: slow.~SlowAction(); break;
        case Action::Type::SPARK: spark.~SparkAction(); break;
        case Action::Type::SPEED: speed.~SpeedAction(); break;
        case Action::Type::SPIN: spin.~SpinAction(); break;
        case Action::Type::STOP: stop.~StopAction(); break;
        case Action::Type::TARGET: target.~TargetAction(); break;
        case Action::Type::THRUST: thrust.~ThrustAction(); break;
        case Action::Type::WARP: warp.~WarpAction(); break;
        case Action::Type::WIN: win.~WinAction(); break;
        case Action::Type::ZOOM: zoom.~ZoomAction(); break;
    }
}

static uint32_t optional_flags(path_value x, const std::map<pn::string_view, int>& flags) {
    if (x.value().is_null()) {
        return 0;
    } else if (x.value().is_map()) {
        uint32_t result = 0;
        for (auto kv : flags) {
            if (read_field<sfz::optional<bool>>(x.get(kv.first)).value_or(false)) {
                result |= 1 << kv.second;
            }
        }
        for (auto kv : x.value().as_map()) {
            if (flags.find(kv.key()) == flags.end()) {
                path_value v = x.get(kv.key());
                throw std::runtime_error(pn::format("{0}unknown flag", v.prefix()).c_str());
            }
        }
        return result;
    } else {
        throw std::runtime_error(pn::format("{0}must be null or map", x.prefix()).c_str());
    }
}

FIELD_READER(ActionBase::Filter::Attributes) {
    ActionBase::Filter::Attributes attributes;
    attributes.bits = optional_flags(
            x, {{"can_be_engaged", 1},
                {"does_bounce", 6},
                {"can_be_destination", 10},
                {"can_engage", 11},
                {"can_evade", 12},
                {"can_accept_build", 14},
                {"can_accept_destination", 15},
                {"autotarget", 16},
                {"animation_cycle", 17},
                {"can_collide", 18},
                {"can_be_hit", 19},
                {"is_destination", 20},
                {"hide_effect", 21},
                {"release_energy_on_death", 22},
                {"hated", 23},
                {"occupies_space", 24},
                {"static_destination", 25},
                {"can_be_evaded", 26},
                {"neutral_death", 27},
                {"is_guided", 28},
                {"appear_on_radar", 29}});
    return attributes;
}

FIELD_READER(ActionBase::Filter) {
    return optional_struct<ActionBase::Filter>(
                   x, {{"attributes", &ActionBase::Filter::attributes},
                       {"tags", &ActionBase::Filter::tags},
                       {"owner", &ActionBase::Filter::owner}})
            .value_or(ActionBase::Filter{});
}

FIELD_READER(ActionBase::Override) {
    return optional_struct<ActionBase::Override>(
                   x, {{"subject", &ActionBase::Override::subject},
                       {"direct", &ActionBase::Override::direct}})
            .value_or(ActionBase::Override{});
}

FIELD_READER(Action::Type) {
    return required_enum<Action::Type>(
            x, {{"age", Action::Type::AGE},
                {"assume", Action::Type::ASSUME},
                {"cap-speed", Action::Type::CAP_SPEED},
                {"capture", Action::Type::CAPTURE},
                {"check", Action::Type::CHECK},
                {"cloak", Action::Type::CLOAK},
                {"condition", Action::Type::CONDITION},
                {"create", Action::Type::CREATE},
                {"delay", Action::Type::DELAY},
                {"destroy", Action::Type::DESTROY},
                {"disable", Action::Type::DISABLE},
                {"energize", Action::Type::ENERGIZE},
                {"equip", Action::Type::EQUIP},
                {"fire", Action::Type::FIRE},
                {"flash", Action::Type::FLASH},
                {"group", Action::Type::GROUP},
                {"heal", Action::Type::HEAL},
                {"hold", Action::Type::HOLD},
                {"key", Action::Type::KEY},
                {"land", Action::Type::LAND},
                {"message", Action::Type::MESSAGE},
                {"morph", Action::Type::MORPH},
                {"move", Action::Type::MOVE},
                {"occupy", Action::Type::OCCUPY},
                {"target", Action::Type::TARGET},
                {"pay", Action::Type::PAY},
                {"play", Action::Type::PLAY},
                {"push", Action::Type::PUSH},
                {"remove", Action::Type::REMOVE},
                {"reveal", Action::Type::REVEAL},
                {"score", Action::Type::SCORE},
                {"select", Action::Type::SELECT},
                {"slow", Action::Type::SLOW},
                {"spark", Action::Type::SPARK},
                {"speed", Action::Type::SPEED},
                {"spin", Action::Type::SPIN},
                {"stop", Action::Type::STOP},
                {"thrust", Action::Type::THRUST},
                {"warp", Action::Type::WARP},
                {"win", Action::Type::WIN},
                {"zoom", Action::Type::ZOOM}});
}

FIELD_READER(Within) {
    return optional_enum<Within>(x, {{"circle", Within::CIRCLE}, {"square", Within::SQUARE}})
            .value_or(Within::CIRCLE);
}

static Action age_action(path_value x) {
    return required_struct<AgeAction>(
            x, {COMMON_ACTION_FIELDS,
                {"relative", &AgeAction::relative},
                {"value", &AgeAction::value}});
}

static Action assume_action(path_value x) {
    return required_struct<AssumeAction>(
            x, {COMMON_ACTION_FIELDS, {"which", &AssumeAction::which}});
}

static Action cap_speed_action(path_value x) {
    return required_struct<CapSpeedAction>(
            x, {COMMON_ACTION_FIELDS, {"value", &CapSpeedAction::value}});
}

static Action capture_action(path_value x) {
    return required_struct<CaptureAction>(
            x, {COMMON_ACTION_FIELDS, {"player", &CaptureAction::player}});
}

static Action check_action(path_value x) {
    return required_struct<CheckAction>(x, {COMMON_ACTION_FIELDS});
}

static Action cloak_action(path_value x) {
    return required_struct<CloakAction>(x, {COMMON_ACTION_FIELDS});
}

static Action condition_action(path_value x) {
    return required_struct<ConditionAction>(
            x, {COMMON_ACTION_FIELDS,
                {"enable", &ConditionAction::enable},
                {"disable", &ConditionAction::disable}});
}

static Action create_action(path_value x) {
    return required_struct<CreateAction>(
            x, {COMMON_ACTION_FIELDS,
                {"base", &CreateAction::base},
                {"count", &CreateAction::count},
                {"relative_velocity", &CreateAction::relative_velocity},
                {"relative_direction", &CreateAction::relative_direction},
                {"distance", &CreateAction::distance},
                {"within", &CreateAction::within},
                {"inherit", &CreateAction::inherit},
                {"legacy_random", &CreateAction::legacy_random}});
}

static Action delay_action(path_value x) {
    return required_struct<DelayAction>(
            x, {COMMON_ACTION_FIELDS, {"duration", &DelayAction::duration}});
}

static Action destroy_action(path_value x) {
    return required_struct<DestroyAction>(x, {COMMON_ACTION_FIELDS});
}

static Action disable_action(path_value x) {
    return required_struct<DisableAction>(
            x, {COMMON_ACTION_FIELDS, {"duration", &DisableAction::duration}});
}

static Action energize_action(path_value x) {
    return required_struct<EnergizeAction>(
            x, {COMMON_ACTION_FIELDS, {"value", &EnergizeAction::value}});
}

FIELD_READER(Weapon) {
    return required_enum<Weapon>(
            x, {{"pulse", Weapon::PULSE}, {"beam", Weapon::BEAM}, {"special", Weapon::SPECIAL}});
}

static Action equip_action(path_value x) {
    return required_struct<EquipAction>(
            x,
            {COMMON_ACTION_FIELDS, {"which", &EquipAction::which}, {"base", &EquipAction::base}});
}

static Action fire_action(path_value x) {
    return required_struct<FireAction>(x, {COMMON_ACTION_FIELDS, {"which", &FireAction::which}});
}

static Action flash_action(path_value x) {
    return required_struct<FlashAction>(
            x, {COMMON_ACTION_FIELDS,
                {"duration", &FlashAction::duration},
                {"color", &FlashAction::color}});
}

static Action group_action(path_value x) {
    return required_struct<GroupAction>(x, {COMMON_ACTION_FIELDS, {"of", &GroupAction::of}});
}

static Action heal_action(path_value x) {
    return required_struct<HealAction>(x, {COMMON_ACTION_FIELDS, {"value", &HealAction::value}});
}

static Action hold_action(path_value x) {
    return required_struct<HoldAction>(x, {COMMON_ACTION_FIELDS});
}

FIELD_READER(KeyAction::Key) {
    return required_enum<KeyAction::Key>(
            x, {{"up", KeyAction::Key::UP},
                {"down", KeyAction::Key::DOWN},
                {"left", KeyAction::Key::LEFT},
                {"right", KeyAction::Key::RIGHT},
                {"pulse", KeyAction::Key::PULSE},
                {"beam", KeyAction::Key::BEAM},
                {"special", KeyAction::Key::SPECIAL},
                {"warp", KeyAction::Key::WARP},
                {"select_friend", KeyAction::Key::SELECT_FRIEND},
                {"select_foe", KeyAction::Key::SELECT_FOE},
                {"select_base", KeyAction::Key::SELECT_BASE},
                {"target", KeyAction::Key::TARGET},
                {"order", KeyAction::Key::ORDER},
                {"zoom_in", KeyAction::Key::ZOOM_IN},
                {"zoom_out", KeyAction::Key::ZOOM_OUT},
                {"comp_up", KeyAction::Key::COMP_UP},
                {"comp_down", KeyAction::Key::COMP_DOWN},
                {"comp_accept", KeyAction::Key::COMP_ACCEPT},
                {"comp_back", KeyAction::Key::COMP_BACK},

                {"comp_message", KeyAction::Key::COMP_MESSAGE},
                {"comp_special", KeyAction::Key::COMP_SPECIAL},
                {"comp_build", KeyAction::Key::COMP_BUILD},
                {"zoom_shortcut", KeyAction::Key::ZOOM_SHORTCUT},
                {"send_message", KeyAction::Key::SEND_MESSAGE},
                {"mouse", KeyAction::Key::MOUSE}});
}

static Action key_action(path_value x) {
    return required_struct<KeyAction>(
            x, {COMMON_ACTION_FIELDS,
                {"enable", &KeyAction::enable},
                {"disable", &KeyAction::disable}});
}

static Action land_action(path_value x) {
    return required_struct<LandAction>(x, {COMMON_ACTION_FIELDS, {"speed", &LandAction::speed}});
}

static Action message_action(path_value x) {
    return required_struct<MessageAction>(
            x,
            {COMMON_ACTION_FIELDS, {"id", &MessageAction::id}, {"pages", &MessageAction::pages}});
}

static Action morph_action(path_value x) {
    return required_struct<MorphAction>(
            x, {COMMON_ACTION_FIELDS,
                {"base", &MorphAction::base},
                {"keep_ammo", &MorphAction::keep_ammo}});
}

FIELD_READER(sfz::optional<coordPointType>) {
    auto p = read_field<sfz::optional<Point>>(x);
    if (!p.has_value()) {
        return sfz::nullopt;
    }
    return sfz::make_optional(coordPointType{(uint32_t)p->h, (uint32_t)p->v});
}

FIELD_READER(sfz::optional<MoveAction::Origin>) {
    return optional_enum<MoveAction::Origin>(
            x, {{"level", MoveAction::Origin::LEVEL},
                {"subject", MoveAction::Origin::SUBJECT},
                {"direct", MoveAction::Origin::DIRECT}});
}

static Action move_action(path_value x) {
    return required_struct<MoveAction>(
            x, {COMMON_ACTION_FIELDS,
                {"origin", &MoveAction::origin},
                {"to", &MoveAction::to},
                {"distance", &MoveAction::distance},
                {"within", &MoveAction::within}});
}

static Action occupy_action(path_value x) {
    return required_struct<OccupyAction>(
            x, {COMMON_ACTION_FIELDS, {"value", &OccupyAction::value}});
}

static Action pay_action(path_value x) {
    return required_struct<PayAction>(
            x,
            {COMMON_ACTION_FIELDS, {"value", &PayAction::value}, {"player", &PayAction::player}});
}

FIELD_READER(PlayAction::Sound) {
    return required_struct<PlayAction::Sound>(x, {{"sound", &PlayAction::Sound::sound}});
}

FIELD_READER(PlayAction::Priority) {
    int level = int_field_within(x, {0, 6});
    return PlayAction::Priority{level};
}

static Action play_action(path_value x) {
    return required_struct<PlayAction>(
            x, {COMMON_ACTION_FIELDS,
                {"priority", &PlayAction::priority},
                {"persistence", &PlayAction::persistence},
                {"absolute", &PlayAction::absolute},
                {"volume", &PlayAction::volume},
                {"sound", &PlayAction::sound},
                {"any", &PlayAction::any}});
}

static Action push_action(path_value x) {
    return required_struct<PushAction>(x, {COMMON_ACTION_FIELDS, {"value", &PushAction::value}});
}

static Action remove_action(path_value x) {
    return required_struct<RemoveAction>(x, {COMMON_ACTION_FIELDS});
}

static Action reveal_action(path_value x) {
    return required_struct<RevealAction>(
            x, {COMMON_ACTION_FIELDS, {"initial", &RevealAction::initial}});
}

static Action score_action(path_value x) {
    return required_struct<ScoreAction>(
            x, {COMMON_ACTION_FIELDS,
                {"counter", &ScoreAction::counter},
                {"value", &ScoreAction::value}});
}

static Action select_action(path_value x) {
    return required_struct<SelectAction>(
            x, {COMMON_ACTION_FIELDS,
                {"screen", &SelectAction::screen},
                {"line", &SelectAction::line}});
}

static Action slow_action(path_value x) {
    return required_struct<SlowAction>(x, {COMMON_ACTION_FIELDS, {"value", &SlowAction::value}});
}

static Action spark_action(path_value x) {
    return required_struct<SparkAction>(
            x, {COMMON_ACTION_FIELDS,
                {"count", &SparkAction::count},
                {"hue", &SparkAction::hue},
                {"age", &SparkAction::age},
                {"velocity", &SparkAction::velocity}});
}

static Action speed_action(path_value x) {
    return required_struct<SpeedAction>(
            x, {COMMON_ACTION_FIELDS,
                {"value", &SpeedAction::value},
                {"relative", &SpeedAction::relative}});
}

static Action spin_action(path_value x) {
    return required_struct<SpinAction>(x, {COMMON_ACTION_FIELDS, {"value", &SpinAction::value}});
}

static Action stop_action(path_value x) {
    return required_struct<StopAction>(x, {COMMON_ACTION_FIELDS});
}

static Action target_action(path_value x) {
    return required_struct<TargetAction>(x, {COMMON_ACTION_FIELDS});
}

static Action thrust_action(path_value x) {
    return required_struct<ThrustAction>(
            x, {COMMON_ACTION_FIELDS, {"value", &ThrustAction::value}, {"relative", nullptr}});
}

static Action warp_action(path_value x) {
    return required_struct<WarpAction>(x, {COMMON_ACTION_FIELDS});
}

static Action win_action(path_value x) {
    return required_struct<WinAction>(
            x, {COMMON_ACTION_FIELDS,
                {"player", &WinAction::player},
                {"next", &WinAction::next},
                {"text", &WinAction::text}});
}

static Action zoom_action(path_value x) {
    return required_struct<ZoomAction>(x, {COMMON_ACTION_FIELDS, {"value", &ZoomAction::value}});
}

DEFINE_FIELD_READER(Action) {
    switch (required_object_type(x, read_field<Action::Type>)) {
        case Action::Type::AGE: return age_action(x);
        case Action::Type::ASSUME: return assume_action(x);
        case Action::Type::CAP_SPEED: return cap_speed_action(x);
        case Action::Type::CAPTURE: return capture_action(x);
        case Action::Type::CHECK: return check_action(x);
        case Action::Type::CLOAK: return cloak_action(x);
        case Action::Type::CONDITION: return condition_action(x);
        case Action::Type::CREATE: return create_action(x);
        case Action::Type::DELAY: return delay_action(x);
        case Action::Type::DESTROY: return destroy_action(x);
        case Action::Type::DISABLE: return disable_action(x);
        case Action::Type::ENERGIZE: return energize_action(x);
        case Action::Type::EQUIP: return equip_action(x);
        case Action::Type::FIRE: return fire_action(x);
        case Action::Type::FLASH: return flash_action(x);
        case Action::Type::GROUP: return group_action(x);
        case Action::Type::HEAL: return heal_action(x);
        case Action::Type::HOLD: return hold_action(x);
        case Action::Type::KEY: return key_action(x);
        case Action::Type::LAND: return land_action(x);
        case Action::Type::MESSAGE: return message_action(x);
        case Action::Type::MORPH: return morph_action(x);
        case Action::Type::MOVE: return move_action(x);
        case Action::Type::OCCUPY: return occupy_action(x);
        case Action::Type::PAY: return pay_action(x);
        case Action::Type::PLAY: return play_action(x);
        case Action::Type::PUSH: return push_action(x);
        case Action::Type::REMOVE: return remove_action(x);
        case Action::Type::REVEAL: return reveal_action(x);
        case Action::Type::SCORE: return score_action(x);
        case Action::Type::SELECT: return select_action(x);
        case Action::Type::SLOW: return slow_action(x);
        case Action::Type::SPARK: return spark_action(x);
        case Action::Type::SPEED: return speed_action(x);
        case Action::Type::SPIN: return spin_action(x);
        case Action::Type::STOP: return stop_action(x);
        case Action::Type::TARGET: return target_action(x);
        case Action::Type::THRUST: return thrust_action(x);
        case Action::Type::WARP: return warp_action(x);
        case Action::Type::WIN: return win_action(x);
        case Action::Type::ZOOM: return zoom_action(x);
    }
}

}  // namespace antares
