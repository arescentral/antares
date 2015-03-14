Releasing Antares
=================

Starting a release
------------------

1.  Run ``./scripts/release start $VERSION``.
2.  Run ``make dist``.
3.  Unzip the distfile and run ``make`` and ``make sign`` in it.

Finishing a release
-------------------

1.  Run ``./scripts/release finish $VERSION``.
2.  Run ``make dist``.
3.  Unzip the distfile and run ``make`` and ``make sign`` in it.
4.  Publish the release.

Places to update
----------------

1.  Upload the new binaries to `downloads.arescentral.org`_.
2.  Mark appropriate `FixPending bugs`_ as Fixed.
3.  Update `the website`_ to reference the latest version.
4.  Add a new news article to `the website`_.
5.  Send an email to the `announcement mailing list`_.
6.  Create a new thread in the `Ares Briefing Room`_.
7.  Update the topic on the `#ares irc channel`_.

..  _downloads.arescentral.org: http://downloads.arescentral.org/
..  _the website: https://github.com/arescentral/arescentral.org
..  _announcement mailing list: https://groups.google.com/a/arescentral.org/group/antares-announce
..  _ares briefing room: http://www.ambrosiasw.com/forums/index.php?showforum=15
..  _#ares irc channel: irc://irc.afternet.org/#ares
..  _fixpending bugs: https://github.com/arescentral/antares/issues?q=is%3Aissue+is%3Aopen+label%3A%220+FixPending%22
