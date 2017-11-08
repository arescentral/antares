Antares
=======

Download: `Antares 0.8.2 for Mac OS X`_

Antares is based on Ares_, a game developed by `Nathan Lamont`_, and
released for the classic Mac OS in 1996. After a re-release by `Ambrosia
Software`_ and a major expansion which added support for plug-ins, the
game fell into obscurity, as it was not ported to Mac OS X. However, in
2008, the source code was released. Antares is a port of that code to
modern systems.

There are several issues that need to be fixed before the 1.0 release;
the issue tracker contains a `list of them`_. After they are fixed,
Antares will have feature parity with Ares in all respects, except for
plugins and multiplayer. These are considered features to be added after
1.0.

..  _Antares 0.8.2 for Mac OS X: http://downloads.arescentral.org/Antares/Antares-0.8.2.zip
..  _Ares: http://en.wikipedia.org/wiki/Ares_(computer_game)
..  _Nathan Lamont: http://biggerplanet.com/
..  _Ambrosia Software: http://www.ambrosiasw.com/
..  _list of them: https://github.com/arescentral/antares/issues?q=is%3Aissue+is%3Aopen+-milestone%3ALater+

Building Antares
----------------

`The long version is here`_. The short version is::

    $ ./configure
    $ make

You may need to install some extra software first; the configure script
give you instructions if so.

..  _the long version is here: http://arescentral.org/antares/contributing/building/

Running Antares
---------------

The short version is::

    $ make run

On Mac, the launcher will let you choose video settings, and optionally
a plugin to run.

On Linux, you'll need to run `out/cur/antares-install-data` first.
Afterwards, you may want to run `out/cur/antares` directly.  You can
choose video settings with command line options; see `antares --help`.
There's no plugin support yet.

Links
-----

*   `Antares website`_

    *   `Contributing to Antares`_

*   `Announcements mailing list`_
*   `Development mailing list`_

*   `Ares Forums`_
*   Xsera_

..  _Antares website: http://arescentral.org/antares/
..  _Contributing to Antares: http://arescentral.org/antares/contributing/
..  _Announcements mailing list: https://groups.google.com/a/arescentral.org/group/antares-announce
..  _Development mailing list: https://groups.google.com/a/arescentral.org/group/antares-dev
..  _Ares Forums: http://www.ambrosiasw.com/forums/index.php?showforum=88
..  _Xsera: http://www.xsera.org/

..  -*- tab-width: 4; fill-column: 72 -*-
