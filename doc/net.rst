Basic sketch of net protocol
============================

..  Contents::

Terms
-----

_`node`
    A running copy of Antares participating in a net game.  Not all
    nodes need connect to each other, but there must necessarily be a
    path from any node to every other.  Nodes' topology may be
    restricted; for example as servers and clients (though no topology
    is assumed in this document).

_`player`
    A human taking actions in a net game.  Every node_ has zero or one
    local players.

_`peer`
    A connected node_.  In a client-server topology, all nodes_ are the
    server's peers, and the clients' only peer is the server.  All
    connections are bi-directional.

_`tail`
    A historical and authoritative copy of the state of the universe.
    A node_'s copy of tail_ for a given timestamp is identical to the
    copy every other node_ has, had, or will have for that time.

_`head`
    A current and hypothetical copy of the state of the universe.  A
    node_'s copy of head_ is probably different from every other
    node_'s, so it is continuously reconstructed from tail_.

_`input log`
    A timestamped list of actions taken by a player_ (local or remote),
    plus a leading timestamp indicating the last time the log was
    updated.  Every node_ keeps an input log for every player_.

_`desynchronization`
    When a node_ constructs a tail_ which didn't or won't happen on some
    other node_.  This can happen either if nodes_ receive conflicting
    `input logs`_, or if they process those logs_ differently.

_`pre-delay networking`
    The old way that Ares networking worked.  All nodes_ agreed on a lag
    value, and all input was delayed that long in order to ensure that
    all clients had knowledge of the input before the time came to
    process it.

_`predictive networking`
    The new way that Ares networking works.  Each node_ re-simulates
    game ticks as it receives new input.  This allows the local
    player_'s input to be shown in real-time.

..  Aliases
..  _nodes: node_
..  _players: player_
..  _peers: peer_
..  _log: `input log`_
..  _logs: log_
..  _input logs: log_
..  _desync: desynchronization_

Protocol
--------

All nodes_ keep two complete versions of the universe, head_ and
tail_.  head_ represents the node_'s optimistic prediction about what
the game should look like at the current time.  tail_ represents the
most recent state known to be authoritative.  All nodes_ also keep an
`input log`_ for every player_.  For each of the node_'s peers_, it
tracks the leading timestamp of each of the peer_'s logs_.

Every tick:

 1. The local player_'s `input log`_ is updated.
 2. tail_ is updated as far the shortest `input log`_ runs.
 3. tail_ is copied wholesale to head_, discarding head_.
 4. head_ is updated as far the longest `input log`_ runs (which would
    be the local player's log).

Some things should happen only on the final iteration of step 4.  For
example, sounds and the starfield should be updated as if head_ were the
true state of the universe.

The net protocol loops independently of the above.  At each iteration,
to each peer_, for each player_, a node_ sends:

  * The leading timestamp of the local log_ for that player_.
  * Any input after the end of the peer_'s log_ for that player_.

Nothing need be sent reliably (e.g. via TCP).  Any input that the peer_
doesn't know about will be resent until it does.  The networking can
(and probably should) be done in a separate thread, or in one thread per
peer_, in which case access to the logs (and only to the logs) needs to
be under lock.

Input
-----

In `pre-delay networking`_, "input" was (?) synonymous with
"mouse/keyboard input".  That needs to change for `predictive
networking`_.  For example, if there is a transport on top of a cruiser,
and the player_ clicks twice, the cruiser should be selected, even if
actions on other nodes_ cause the transport to be destroyed before the
player_'s clicks occur.  If the cruiser were to be destroyed, nothing
should be selected.

In general, only keyboard input intended for a player_'s flagship should
be written to the log_ as such.  Keyboard input involved in fleet
management, such as builds, orders, and transfers, should be translated
into an abstract form instead.  Selection (including mouse input) should
not be sent at all; it is merely local state that gets baked into the
abstract form of an order or transfer.

Some forms of input are irrelevant to the net protocol (zoom, volume,
dismissing messages) or impossible in net games (fast motion).  However,
they should be part of the local input log, so that we can track local
state appropriately.  It's probably easiest to send them so that we can
assume that the local and remote version of an input log are the same.

..  note::

    Some things that are only needed locally can be tested in conditions
    (is control, is target, message displayed, computer state, zoom
    level, on autopilot, not on autopilot, is player).  However, those
    conditions are broken on multiplayer anyway because they test
    against the local player_ and would therefore cause desync_.

`Input logs`_ are circular buffers.  The leading edge is filled either
through local input or from the network.  The trailing edge is trimmed
of any information that tail_ and all peers_ have been updated with.

Assumptions
~~~~~~~~~~~

In step 4, we haven't received input for every player yet and have to
make assumptions.  The default would be to assume no input: no orders
are given (probably OK); their ship continues to fire or not fire
(probably OK); their ship continues to turn or not (probably bad).

We might consider allowing the AI to control their ship after the
end of their log, but it's hard to say whether that would make things
better or worse without trying it.

We might consider more abstract forms of commands for the ship ("turn to
315°" instead of "turn clockwise").  I think that predicting the
intended heading of the ship is what will cause problems; thrust,
weapons, and warp state can all probably be assumed to be unchanged
without further input.

Stableness
~~~~~~~~~~

Sometimes, an input will reference an object in head_ that has not yet
appeared in tail_.  Currently, the ID of an object is based on the
global random number generator, which means that it's likely that remote
events will result in a different identifier.  We need stable
identifiers to use instead of ID; probably, the tuple `(basetype,
creation_time)` is sufficient.

Pausing
-------

We could go a few ways on this:

 1. There is no pausing.  A player_ can go into the play-again screen
    and choose to forfeit, but it won't pause the game on any other
    node_.

 2. When a player_ hits ESC, their node_ immediately pauses.  Other
    nodes_ pause as soon as their tail_ advances to that time, and
    discard any input generated after that time.  When unpausing, nodes_
    other than the pauser will be rewound slightly.

 3. When a player_ hits ESC, it starts a (3-second?) timer, at the end
    of which all nodes_ pause the game.  No node_ advances past that
    time until unpausing.

Honestly, the first option seems best.

Sounds
------

Do we only consider head_ for sounds, or do we try to track remote
changes?  The latter seems hard, and maybe not even possible with the
OpenAL API.

Malice
------

There's nothing in the protocol that prevents a malicious node_ from
falsifying the log_ of a non-local player_.  However, if one does, then
it will cause desync_.  We may not be able to detect that it was a
malicious desync_, which is probably OK for now.

..  note::

    Malice is distinct from "cheating", which is something that players_
    are allowed to do if they negotiate it groncularly.

Replays
-------

The `input logs`_ in a game have all the information we need to make a
replay, so we should implement them as such.

..  -*- tab-width: 4; fill-column: 72 -*-
