Antares
=======

Download: `Antares 0.9.0 for Mac OS X`_

..  _Antares 0.9.0 for Mac OS X: http://downloads.arescentral.org/Antares/Antares-0.9.0.zip

..  image:: https://travis-ci.org/arescentral/antares.svg?branch=master
    :target: https://travis-ci.org/arescentral/antares

Antares is based on Ares_, a game developed by `Nathan Lamont`_, and
released for the classic Mac OS in 1996. After a re-release by `Ambrosia
Software`_ and a major expansion which added support for plug-ins, the
game fell into obscurity, as it was not ported to Mac OS X. However, in
2008, the source code was released. Antares is a port of that code to
modern systems.

There are several issues that need to be fixed before the 1.0 release;
the issue tracker contains a `list of them`_. After they are fixed,
Antares will have feature parity with Ares in all respects except for
multiplayer. The timeline for multiplayer is less certain.

..  _Ares: https://en.wikipedia.org/wiki/Ares_(computer_game)
..  _Nathan Lamont: http://biggerplanet.com/
..  _Ambrosia Software: https://www.ambrosiasw.com/
..  _list of them: https://github.com/arescentral/antares/issues?q=is%3Aissue+is%3Aopen+-milestone%3ALater+

Building Antares
----------------

`The long version is here`_. The short version is::

    $ ./configure
    $ make

You may need to install some extra software first; the configure script
will give you instructions if so.

..  _the long version is here: https://arescentral.org/antares/contributing/building/

Running Antares
---------------

The short version is::

    $ make run

A launcher will appear, letting you choose a plugin to play. By default,
the original Ares scenario will be listed. There aren’t any video
settings; just resize or maximize the window and Antares will adapt to
the new size.

On Linux, you can also install the game::

    $ sudo make install

This will install the game to /usr/local/games/antares, by default.

On Mac, you can put the game anywhere you’d like::

    $ mv out/cur/Antares.app /Applications

Links
-----

*   `Antares website`_

    *   `Contributing to Antares`_

*   `Announcements mailing list`_
*   `Development mailing list`_

*   `Ares Forums`_

..  _Antares website: https://arescentral.org/antares/
..  _Contributing to Antares: https://arescentral.org/antares/contributing/
..  _Announcements mailing list: https://groups.google.com/a/arescentral.org/group/antares-announce
..  _Development mailing list: https://groups.google.com/a/arescentral.org/group/antares-dev
..  _Ares Forums: https://www.ambrosiasw.com/forums/index.php?showforum=88

..  -*- tab-width: 4; fill-column: 72 -*-
