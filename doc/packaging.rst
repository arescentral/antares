Packaging Antares
=================

Considering packaging Antares? Thanks! Here are some things to consider:

Licensing
---------

A complete Antares installation has components under three different
licenses:

*   The binaries are licensed under the GPL2 or LGPL3. These are all
    built from source and installed to ``$PREFIX/bin/``. Source code is
    tracked in the antares_ repo.

*   Most of the application data is CC-BY-NC-SA-licensed; note the "NC"
    (Non-Commercial), which makes this non-open-source. This is included
    in the Antares distfile, but tracked separately in the antares-data_
    repo. Also, the icon is under this license but is in the main repo
    (maybe I should fix this). This goes to
    ``$PREFIX/share/antares/app/``.

*   The main scenario is not under any license. Because Ares is
    shareware, the unmodified application can be redistributed. The
    ``antares-install-data`` binary will automatically download and
    extract the data to ``~/.local/share/antares/``. If you want to
    handle downloading yourself, you can stick Ares-1.2.0.zip_ in
    ``~/.local/share/antares/downloads/``.

Because of the differences between these three components, you might
want to keep them as separate packages, e.g. ``antares`` (GPL,
binaries), depending on ``antares-data`` (CC-BY-NC-SA) and
``antares-scenario`` (proprietary).

Since it's installed to a user's home directory, it may not actually be
possible to package ``antares-scenario``. Installing it to a shared
directory would be a possible change but would require some extra
thought around upgrades.

..  _antares: https://github.com/arescentral/antares
..  _antares-data: https://github.com/arescentral/antares-data
..  _Ares-1.2.0.zip: http://downloads.arescentral.org/Ares/Ares-1.2.0.zip

Dependencies
------------

The ``./configure`` script should make it clear what the build-time
dependencies are. There's no list of runtime dependencies, but it should
be clear from the build-time list what's needed. If something seems to
be missing, you can file an issue to have it added.
