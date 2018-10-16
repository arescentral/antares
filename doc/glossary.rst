Glossary
========

..	_action:
..	_actions:

action
	A description of something to do. Actions have a verb and possibly
	subject_ and `direct objects`_. Typical actions include “create a
	projectile”, “play a sound”, and “capture planet”.

	Often, “action” is used to refer to an `action sequence`_.

..	_action sequence:
..	_action (sequence):
..	_action sequences:
..	|action (sequence)| replace:: action

action sequence
	A sequence of actions_. Objects have up to six associated action
	sequences: create_, collide_, destroy_, expire_, activate_, and
	arrive_. Conditions_ also have an associated action sequence.

	Sometimes action sequences are referred to as simply “actions”.

..	_activate:
..	_activate action:

activate
	1.	Of a `space object`_, to execute a periodic action. On
		activation, an object will execute its activate |action
		(sequence)|_ with itself as subject_ and `direct object`_.

	2.	Of a weapon_, to fire. On activation, a weapon will execute its
		activate |action (sequence)|_ with the firing object as the
		`subject object`_ (not itself, as devices_ are never instantiated
		as `space objects`_) and the nearest enemy as `direct object`_.

..	_admiral:
..	_admirals:

admiral
	A player in a currently-running level_. Admirals can be humans or
	CPUs. Human admirals have a flagship_ and can select ships as their
	control_ and target_ objects.

..	_advantage:

advantage
	A fixed-point_ number describing the relative power of a `race`_'s
	ships.

..	_animated:
..	_animation:
..	_animations:

animation
	A sprite_ object_ that changes its appearance with time. Animations
	cycle through frames at a fixed rate. They can animate forwards,
	backwards, or not at all.

	Animated objects are capable of turning, but doing so has no visible
	effect.

	Typical animated objects include pulses_, effects_, hazards_,
	scenery_, planets_ and stations_.

..	_arrive:

arrive
	Of an object, to come within a configurable range of its target_. On
	arrival, an object will execute its arrive |action (sequence)|_ with
	itself as the `subject object`_ and its target as the `direct
	object`_.

..	_base object:
..	_base objects:

base object
	A template for an object, contrasted with a `space object`_.

..	_beam:
..	_beams:

beam weapon
	The second weapon_ on a ship. References the loadout on Ishiman
	cruisers and heavy cruisers, where the second weapon is a PK Cannon
	or Rapid PK Cannon.

..	_blessed object:
..	_blessed objects:

blessed object
	One of four special `base object`_ definitions required by Antares:

	*	``sfx/energy``: the energy blob, created for every 500 energy
		a destroyed_ object with ``destroy.release_energy`` has.
	*	``sfx/crew``: the castaway crew member, created when the
		flagship_ of a human admiral_ dies_.
	*	``sfx/warp/in``: the warp-in flare, created when a ship enters
		hyperspace.
	*	``sfx/warp/out``: the warp-out flare, created when a ship leaves
		hyperspace.

	The `factory scenario`_ definitions of these are usually sufficient,
	and plugins_ do not need to provide their own.

..	_bolt:
..	_bolts:

bolt
	A moving vector_ object_. Bolts exist at single points in space,
	but are drawn as lines from their previous position to their current
	position. A faster bolt will therefore appear as a longer line.

	Bolts have a solid color (Ishiman photokinetic beam, Human laser),
	or may be invisible (Gaitori concussive pellet).

..	_briefing:
..	_briefings:

briefing
	Text shown before a `solo level`_ describing what the player should
	do to complete the level successfully.

..	_build object:

build object
	For an admiral_, the object_ that can be controlled from the build
	menu of the minicomputer_. If the admiral_ selects a `control
	object`_ capable of building ships, that object will become their
	build object.

..	_buildable object:

buildable object
	A name, specifying a `base object`_ or group of base objects. When
	looking up the object by name, Antares first checks for the name
	within the directory corresponding to a player’s race and uses that
	if it exists.

	For example, if a player is playing the Martians (``mar``) and
	builds a Flying Saucer (``saucer``), the buildable object refers to
	``mar/saucer`` if that exists, or ``saucer`` if it doesn’t.

..	_campaign:
..	_campaigns:

campaign
	A sequence of `solo levels`_ contained within a plugin_. Campaigns
	may be linear, where each chapter_ must be completely successfully
	before moving on to the next, or branching, where different victory
	conditions lead to different subsequent levels.

..	_chapter:
..	_chapters:

chapter
	A level_. Typically, a level will be referred to as a chapter when
	it exists in a sequence (e.g. “Chapter 1”).

..	_collide:
..	_collision:

collide
	Of two capable `space objects`_, to intersect. The objects must have
	different owners. One object (or both) must have
	``collide.as.subject: true``, and the other must have
	``collide.as.object: true``.

	The object with ``collide.as.subject: true`` will execute its
	collide |action (sequence)|_ with itself as subject_ and the other
	object as object_. If both have both flags, then the collision
	then happens in the other direction.

	Sprite_ objects (rotations_ and animations_) use the bounding box of
	their sprite for collision detection. Vector_ objects (bolts_ and
	rays_) objects use their lines. Two vector objects are not capable
	of colliding.

	Solid objects (``collide.solid: true``) will bounce off each other
	according to their configured mass. If either or both objects are
	not solid, they will pass through each other.

..	_condition:
..	_conditions:

condition
	An |action (sequence)|_ and a criterion for when to execute it.
	Conditions can specify initials_ to use as their subject_ and
	`direct objects`_.

..	_control:
..	_control object:

control object
	For an admiral_, the object_ that will have its target_ set if an
	order is issued. An admiral can also transfer control to their
	control object, making it their flagship_.

..	_create:
..	_created:

create
	Of a `space object`_, to come into existence. On creation, an object
	executes its create |action (sequence)|_ with itself as the subject_
	and `direct object`_.

	The create action of projectiles_ is usually used to play the sound
	associated with firing the weapon_. This ensures that the same sound
	is played for different weapons that use the same projectile (such
	as the PK-Cannon and Rapid PK-Cannon).

..	_destroy:
..	_destroyed:
..	_destruction:

destroy
	Of a `space object`_, to be reduced to zero health. On destruction,
	an object will execute its destroy |action (sequence)|_ with itself
	as the subject_ and `direct object`_.

	By default, objects die_ when they are destroyed. Also, objects with
	the “destroy.neutralize” flag instead lose their owner.

..	_die:
..	_dies:

die
	Of a `space object`_, to cease to exist. Objects usually die as a
	result of destruction_ or expiration_.

..	_direct:
..	_direct object:
..	_direct objects:

direct object
	When executing an action_, the second noun, contrasted with the
	`subject object`_. In a non-reflexive action, the direct object is
	typically the object acted upon.

..	_device:
..	_devices:

device
	An object_ without any presence or appearance. Devices are never
	instantiated as `space objects`_. They are linked from other
	objects’ weapon_ loadout and determine most of the properties of the
	weapon. Most devices are either guns_ or turrets_, but some devices
	such as stealth fields are neither.

..	_effect:
..	_effects:

effect
	A non-interactive animation_ drawn in the top layer_. Typical
	effects include explosions, contrails, and sparkles.

..	_expire:
..	_expired:
..	_expiration:

expire
	1.	Of a `space object`_, to die at the end of its lifetime. On
		expiration, an object will execute its expire |action
		(sequence)|_ with itself as the subject_ and `direct object`_.
		Most expiring objects have an age (``expire.after.age``)
		determining how long they live before expiring. Animations may
		also expire at the end of their last frame
		(``expire.after.animation``).

	2.	Of a `space object`_, to finish a “land at” action. On landing,
		an object will execute its expire |action (sequence)|_ with
		itself as the `subject object`_ and the landing target as the
		`direct object`_.

..	_factory scenario:

factory scenario
	The Ares scenario_. Other plugins_ implicitly use resources from it.

..	_fixed-point:

fixed-point
	A number with a fractional component, represented as an integer and
	a constant scaling factor. Usually, in Antares, the scaling factor
	is 1/256.

..	_flagship:

flagship
	A ship_ being piloted directly by a human admiral_.

..	_gun:
..	_guns:

gun
	An attack device_ with “direction: "fore"”. The projectiles it
	creates can only be used to attack targets in front of the wielder.

..	_hazard:
..	_hazards:

hazard
	An animation_ which can collide_ and be hit. They are drawn in
	the middle layer_ with ships_. Typical hazards are asteroids,
	nastiroids, and debris.

..	_initial:
..	_initials:

initial
	A `space object`_ that is created at the start of a level_.

..	_layer:
..	_layers:

layer
	One of three planes drawn in sequence. The bottom layer contains
	planets_, stations_, and scenery_; the middle layer contains ships_
	and hazards_; and the top layer contains projectiles_ and effects_.
	Vector_ objects are drawn on top of all of these.

..	_level:
..	_levels:

level
	A chapter_. Typically, a level will be referred to as a chapter when
	it exists in a sequence (e.g. “Chapter 1”).

..	_minicomputer:

minicomputer
	The menu on the left side of the screen, used for building ships_
	and issuing special commands.

..	_missile:
..	_missiles:

missile
	A guided rotating_ projectile_. They are not controllable or
	selectable, and typically have no weapons_, so they attempt to fly
	towards and ram their target.

..	_net level:
..	_net levels:

net level
	A multiplayer level_, containing at least two human admirals_.

..	_object:
..	_objects:

object
	A `base object`_ or `space object`_.

..	_planet:
..	_planets:

planet
	An animation_ capturable by landing a transport on it. Planets
	typically generate resources and can often build ships_.

..	_plugin:
..	_plugins:

plugin
	A bundle containing resources:

	*	`actions`_
	*	`base objects`_
	*	`briefings`_
	*	`conditions`_
	*	`initials`_
	*	`levels`_
	*	music
	*	pictures
	*	sounds
	*	`sprites`_
	*	strings
	*	text

	Plugins also contain various pieces of meta-information: a bundle
	identifier, name, author, URLs, and pointers to the four `blessed
	objects`_.

..	_projectile:
..	_projectiles:

projectile
	An object_ fired from a weapon_ and drawn in the top layer_.
	Projectiles can collide_ but not necessary be hit.

..	_pulse:
..	_pulses:

pulse
	An animated_ projectile_, such as a Fullerene Pulse.

..	_pulse weapon:
..	_pulse (weapon):

pulse weapon
	The first weapon_ on a ship. References the loadout on Ishiman
	cruisers and heavy cruisers, where the second weapon is an F-Pulse
	Cannon or Rapid F-Pulse Cannon.

..	_race:
..	_races:

race
	A name, ship_ set, and advantage_ number. Races may be complete
	(Ishiman, Cantharan) or have only a very small number of ships
	(Bazidanese, Elejeetian). In `solo levels`_, a race is preassigned
	to all admirals_; in `net levels`_, a player may choose their race
	before the game.

..	_ray:
..	_rays:

ray
	A vector_ object_ with a fixed origin. Rays may be object-to-object
	(Salrilian T-Space Bolt) or object-to-point (Audemedon Trazer).

..	_rocket:
..	_rockets:

rocket
	An unguided rotating_ projectile_. They are not thinking or
	selectable. Under this classification, a Fusion Pulse is a rocket,
	not a pulse_.

..	_rotating:
..	_rotation:
..	_rotations:

rotation
	A sprite_ object_ that changes its appearance with direction.
	Typical rotations include ships_, missiles_, and rockets_.

..	_scenario:
..	_scenarios:

scenario
	1.	A plugin_ or the `factory scenario`_.

	2.	A historical term for a level_. This use dates from before Ares
		supported anything beyond the factory scenario.

..	_scenery:

scenery
	A non-interactive animation_ drawn in the bottom layer_. Suns and
	jump gate nodes are scenery.

..	_ship:
..	_ships:

ship
	A thinking, selectable, controllable rotation_. By convention, ships
	in the `factory scenario`_ are further divided into categories and
	classes:

	*	 Warship (triangular icons)

		*	Fighter
		*	Cruiser
		*	Heavy Cruiser
		*	Schooner
		*	Gunship
		*	Heavy Destroyer
		*	Defense Drone

	*	Capship (diamond icons)

		*	Carrier

	*	Utilship (plus icons)

		*	Transport
		*	Engineer Pod
		*	Assault Transport

	Many non-standard ships exist: escorts, battleships, gateships,
	modified standard ships, and more.

..	_solo level:
..	_solo levels:

solo level
	A single-player level_, containing one human admiral_.

..	_space object:
..	_space objects:

space object
	An instantiated `base object`_ that exists in a currently-running
	level_. Space objects are created for rotations_, animations_,
	bolts_, and rays_, but not devices_.

..	_spark:
..	_sparks:

spark
	A moving pixel created by the “create sparks” action_. Sparks are
	not `space objects`_; they are handled separately.

..	_special:
..	_special weapon:

special weapon
	The third weapon_ on a ship_. Ships with stealth fields or launch
	bays always have them set as the special weapon.

	Special weapons have some special rules. Firing the special weapon
	of a cloaked ship will not cause it to decloak.

..	_sprite:
..	_sprites:

sprite
	A set of images (frames) drawn on the screen, contrasted with a
	vector_. Sprite dimensions define the boundaries of an object for
	the purposes of collision_.

..	_station:
..	_stations:

station
	An animation_ capturable by occupying it with EVATs. Stations
	typically generate resources, but usually can’t build ships_.

..	_subject:
..	_subject object:

subject object
	When executing an action_, the first noun, contrasted with the
	`direct object`_. In a non-reflexive action, the subject object is
	typically the object taking action.

..	_target:
..	_target object:

target object
	1.	For an admiral_, the object that will be set as the target of
		the `control object`_ object if an order is issued.

	2.	For a `space object`_, the object that the object will attempt
		to go to if it is not in combat.

..	_turret:
..	_turrets:

turret
	An attack device_ with “direction: "omni"”. Projectiles_ created by
	the turret must also have “autotarget: true”.

..	_vector:

vector
	A line (or lines) drawn on the screen, contrasted with a sprite_.
	Vector objects_ are either bolts_ or rays_. They never occupy space.

..	_weapon:
..	_weapons:

weapon
	A device_ linked from the weapon list of another object_. Ships may
	have up to three weapons, which are sometimes referred to as the
	|pulse (weapon)|_, beam_ and special_ weapons.

..	|pulse (weapon)| replace:: pulse

.. -*- tab-width: 3; indent-tabs-mode: nil; fill-column: 72 -*-
