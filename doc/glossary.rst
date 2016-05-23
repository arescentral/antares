Glossary
========

..  _action:

action
    1.  A description of something to do. Actions have a verb and
        possibly a subject_, object_, and focus_. Typical actions
        include “create a projectile”, “play a sound”, and “alter
        owner”.

    2.  A sequence of the above. Objects have up to six actions:
        create_, collide_, destroy_, expire_, activate_, and arrive_.
        Conditions_ also have an associated action.

..  _activate:

activate
    1.  Of a `space object`_, to execute a periodic action. On
        activation, an object will execute its activate action_ with
        itself as subject_ and no object_.

    2.  Of a `weapon`_, to fire. On activation, a weapon will execute
        its activate action_ with the firing object as subject_ (not
        itself, as devices_ are never instantiated as `space objects`_)
        and the nearest enemy as object_.

..  _admiral:

admiral
    A player in a currently-running level_. Admirals can be humans or
    CPUs. Human admirals have a flagship_ and can select ships as their
    control_ and target_ objects_.

..  _advantage:

advantage
    A fixed-point_ number describing the relative power of a `race`_'s ships.

..  _animated:

animated
    Of an object_, having the ``kIsSelfAnimated (0x00000080)`` flag set.
    Animated objects have a `sprite`_, and cycle through frames at a
    fixed rate. They can animate forwards, backwards, or not at all.

    Animated objects are capable of turning, but have no field for
    controlling their turn rate. They assume a turn rate of 2.000.

    Typical animated objects include pulses_, effects_, hazards_,
    scenery_, planets_ and stations_.

..  _arrive:

arrive
    Of an object, to come within a configurable range of its target_. On
    arrival, an object will execute its arrive action_ with itself as
    subject_ and its target as object_.

..  _base object:

base object
    A template for an object, contrasted with a `space object`_.

..  _beam:

beam
    1.  A vector_ object_ with a fixed origin. Beams may be
        object-to-object (Salrilian T-Space Bolt) or object-to-point
        (Audemedon Trazer).

    2.  A historical name for the second weapon_ on a ship. References
        the loadout on Ishiman cruisers and heavy cruisers, where the
        second weapon is a PK Cannon or Rapid PK Cannon.

..  _blessed:

blessed
    Of objects_, one of four special objects required by Antares:

    *   The energy blob, created for every 500 energy contained by a
        destroyed_ object with ``kReleaseEnergyOnDeath (0x00400000)``
        set.
    *   The castaway crew member, created when the flagship_ of a human
        admiral_ dies_.
    *   The warp-in flare, created when a ship enters hyperspace.
    *   The warp-out flare, created when a ship leaves hyperspace.

..  _bolt:

bolt
    A moving vector_ object_. Bolts exist at single points in space,
    but are drawn as lines from their previous position to their current
    position. A faster bolt will therefore appear as a longer line.

    Bolts have a solid color (Ishiman photokinetic beam, Human laser),
    or may be invisible (Gaitori concussive pellet).

..  _briefing:

briefing
    Text shown before a `solo level`_ describing what the player should
    do to complete the level successfully.

..  _build:

build
    For an admiral_, the object_ that can be controlled from the build
    menu of the minicomputer_. If the admiral_ selects a control_ object
    capable of building ships, that object will become their build
    object.

..  _campaign:

campaign
    A sequence of `solo levels`_ contained within a plugin_. Campaigns
    may be linear, where each chapter_ must be completely successfully
    before moving on to the next, or branching, where different victory
    conditions lead to different subsequent levels.

..  _chapter:

chapter
    A level_. Typically, a level will be referred to as a chapter when
    it exists in a sequence (e.g. “Chapter 1”).

..  _class:

class
    A group of ships_ from different races_ that have the same name and
    numeric class ID. Each class also uses a consistent symbol when
    zoomed out: diamonds for capital ships, triangles for other combat
    ships, and plusses for non-combat ships. Well-established classes
    include:

    *   10: EVAT (triangle)
    *   100: Fighter (triangle)
    *   200: Cruiser (triangle)
    *   250: Heavy Cruiser (triangle)
    *   300: Gunship (triangle)
    *   450: Heavy Destroyer (triangle)
    *   500: Carrier (diamond)
    *   600: Defense Drone (triangle)
    *   800: Transport (plus)
    *   850: Engineer Pod (plus)
    *   860: Assault Transport (plus)

    Bases capable of building ships specify a list of classes that they
    can build; an admiral_ can build the ships associated with their
    race and those classes. Also, in `net levels`_, initials_ will be
    mapped to their race using classes.

    Sometimes a level_ may use a modified version of a ship; these are
    typically assigned class numbers close to their template to indicate
    the connection. For example, the Modified Ishiman Heavy Cruiser has
    class 260.

..  _collide:

collide
    Of two capable `space objects`_, to intersect. In order to collide,
    one object must have the “can collide” flag set and the other “can
    be hit”, and the objects must have different owners. The object with
    “can collide” will execute its collide action_ with itself as
    subject_ and the other object as object_.

    Rotating_ and animated_ objects_ use the bounding box of their
    sprite_ for collision detection. Vector_ objects use their lines.
    Two vector objects are not capable of colliding.

    Objects that occupy space will bounce off each other according to
    their configured mass.

..  _condition:

condition
    An action_ and a criterion for when to execute it. Conditions can
    specify initials_ to as their subject_ and object_; these are used
    for the action and for some condition criteria.

..  _control:

control
    For an admiral_, the object_ that will have its target_ set if an
    order is issued. An admiral can also transfer control to their
    control object, making it their flagship_.

..  _create:

create
    Of a `space object`_, to come into existence. On creation, an object
    will execute its create action_ with itself as subject_ and no
    object_.

    The create action of projectiles_ is usually used to play the sound
    associated with firing the weapon_. This ensures that the same sound
    is played for different weapons that use the same projectile (such
    as the PK-Cannon and Rapid PK-Cannon).

..  _destroy:

destroy
    Of a `space object`_, to be reduced to zero health. On destruction,
    an object will execute its destroy action_ with itself as subject_
    and no object_.

    By default, objects die_ when they are destroyed. Also, objects with
    the “neutral death” flag instead lose their owner.

..  _die:

die
    Of a `space object`_, to cease to exist. Objects usually die as a
    result of destruction_ or expiration_.

..  _device:

device
    An object_ without any flag defining an appearance. Devices are
    never instantiated as `space objects`_. They are linked from other
    objects’ weapon_ loadout and determine most of the properties of the
    weapon. Most devices are either guns_ or turrets_, but some devices
    such as stealth fields are neither.

..  _effect:

effect
    A non-interactive animated_ object_ drawn in the top layer_. Typical
    effects include explosions, contrails, and sparkles.

..  _expire:

expire
    1.  Of a `space object`_, to be reduced to zero age. On expiration,
        an object will execute its expire action_ with itself as
        subject_ and no object_.

    2.  Of a `space object`_, to finish a “land at” action. On landing,
        an object will execute its expire action_ with itself as
        subject_ and the landing target as object_.

..  _factory scenario:

factory scenario
    The Ares scenario_. Other plugins_ implicitly use resources from it.

..  _fixed-point:

fixed-point
    A number with a fractional component, represented as an integer and
    a constant scaling factor. Usually, in Antares, the scaling factor
    is 1/256.

..  _flagship:

flagship
    A ship_ being piloted directly by a human admiral_.

..  _focus:

focus
    When executing a non-reflexive action_, the object_. When executing
    a reflexive action, the subject_. The results of the action are
    typically applied to the focus. For example, an “alter owner” action
    changes the owner of the focus.

..  _gun:

gun
    An attack device_ without the “autotarget” flag set.

..  _hazard:

hazard
    An animated_ object_ which can collide_ and be hit. They are drawn in
    the middle layer_ with ships_. Typical hazards are asteroids and
    nastiroids.

..  _initial:

initial
    A `space object`_ that is created at the start of a level_.

..  _layer:

layer
    One of three planes drawn in sequence. The bottom layer contains
    planets_, stations_, and scenery_; the middle layer contains ships_,
    and the top layer contains projectiles_ and effects_.

..  _level:

level
    A chapter_. Typically, a level will be referred to as a chapter when
    it exists in a sequence (e.g. “Chapter 1”).

..  _minicomputer:

minicomputer
    The menu on the left side of the screen, used for building ships_
    and issuing special commands.

..  _missile:

missile
    A guided rotating_ projectile_. They are not controllable or
    selectable, and typically have no weapons_, so they attempt to fly
    towards and ram their target.

..  _net level:

net level
    A multiplayer level_, containing at least two human admirals_.

..  _object:

object
    1.  A `base object`_ or `space object`_.

    2.  When executing an action_, the second noun, contrasted with the
        subject_. The object is the focus_ of a non-reflexive action.

..  _planet:

planet
    An animated_ object_ capturable by landing a transport on it.
    Planets typically generate resources and can often build ships_.

..  _plugin:

plugin
    A bundle containing resources:

    *   `actions`_
    *   `base objects`_
    *   `briefings`_
    *   `conditions`_
    *   `initials`_
    *   `levels`_
    *   music
    *   pictures
    *   sounds
    *   `sprites`_
    *   strings
    *   text

    Plugins also contain various pieces of meta-information: a bundle
    identifier, name, author, URLs, and pointers to the four blessed_
    objects_.

..  _projectile:

projectile
    An object_ fired from a weapon_ and drawn in the top layer_.
    Projectiles can collide_ but not necessary be hit.

..  _pulse:

pulse
    1.  An animated_ projectile_, such as a Fullerene Pulse.

    2.  A historical name for the first weapon_ on a ship. References
        the loadout on Ishiman cruisers and heavy cruisers, where the
        second weapon is an F-Pulse Cannon or Rapid F-Pulse Cannon.

..  _race:

race
    A name, ship_ set, and advantage_ number. Races may be complete
    (Ishiman, Cantharan) or have only a very small number of ships
    (Bazidanese, Elejeetian). In `solo levels`_, a race is preassigned
    to all admirals_; in `net levels`_, a player may choose their race
    before the game.

..  _rocket:

rocket
    An unguided rotating_ projectile_. They are not thinking or
    selectable. Under this classification, a Fusion Pulse is a rocket,
    not a pulse.

..  _rotating:

rotating
    Of an object_, having the ``kShapeFromDirection (0x00000100)`` flag
    set. Rotating objects have a `sprite`_, and determine which frame to
    use based on their orientation.

    Typical rotating objects include ships_, missiles_, and rockets_.

..  _scenario:

scenario
    1.  A plugin_.

    2.  A historical term for a level_. This use dates from before Ares
        supported anything beyond the factory scenario. This definition
        is reflected in the resource type codes “snro”, “snbf”, “sncd”,
        and “snit”.

..  _scenery:

scenery
    A non-interactive animated_ object_ drawn in the bottom layer_. Suns
    and jump gate nodes are scenery.

..  _ship:

ship
    A thinking, selectable, controllable rotating_ object_. Ships are
    divided up by race_ and class_.

..  _solo level:

solo level
    A single-player level_, containing one human admiral_ and at least
    one cpu admiral.

..  _space object:

space object
    An instantiated `base object`_ that exists in a currently-running
    level_. Space objects are created for rotating_, animated_, and
    vector_ objects, but not devices_.

..  _spark:

spark
    A moving pixel created by the “create sparks” action_. Sparks are
    not `space objects`_; they are handled separately.

..  _special:

special
    The third weapon_ on a ship_. Ships with stealth fields or launch
    bays always have them set as the special weapon.

    Special weapons have some special rules. Firing the special weapon
    of a cloaked ship will not cause it to decloak.

..  _sprite:

sprite
    A set of frames. Sprite dimensions define the boundaries of an
    object for the purposes of collision_.

..  _station:

station
    An animated_ object_ capturable by occupying it with EVATs. Stations
    typically generate resources, but usually can’t build ships_.

..  _subject:

subject
    When executing an action_, the first noun, contrasted with the
    object_. The subject is typically used as a source of information
    for the action. For example, an “alter owner” action usually uses
    the owner of the subject as the new owner.

..  _target:

target
    1.  For an admiral_, the object that will be set as the target of
        the control_ object if an order is issued.

    2.  For a `space object`_, the object that the object will attempt
        to go to if it is not in combat.

..  _turret:

turret
    An attack device_ with the “autotarget” flag set. Projectiles_
    created by the turret must also have the flag set.

..  _vector:

vector
    Of an object_, having the ``kIsBeam (0x00000020)`` flag set. Vector
    objects are either bolts_ or beams_. They never occupy space.

..  _weapon:

weapon
    A device_ linked from the weapon list of another object_. Ships may
    have up to three weapons, which are sometimes referred to as the
    pulse_, beam_ and special_ weapons.

..  Aliases
..  _actions: action_
..  _admirals: admiral_
..  _base objects: `base object`_
..  _beams: beam_
..  _bolts: bolt_
..  _briefings: briefing_
..  _collision: collide_
..  _conditions: condition_
..  _destroyed: destroy_
..  _destruction: destroy_
..  _devices: device_
..  _dies: die_
..  _effects: effect_
..  _expiration: expire_
..  _guns: gun_
..  _hazards: hazard_
..  _initials: initial_
..  _levels: level_
..  _missiles: missile_
..  _objects: object_
..  _planets: planet_
..  _plugins: plugin_
..  _projectiles: projectile_
..  _pulses: pulse
..  _net levels: `net level`_
..  _races: race_
..  _rockets: rocket_
..  _ships: ship_
..  _solo levels: solo level_
..  _space objects: `space object`_
..  _sprites: sprite_
..  _stations: station_
..  _turrets: turret_
..  _weapons: weapon_

..  -*- mode: rst; fill-column: 72 -*-
