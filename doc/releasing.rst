Releasing Antares
=================

Starting a release
------------------

1. Run ``./scripts/release start ${VERSION?}`` on master.
2. ``git push --tags origin release/${VERSION?}`` to trigger a Travis
   build.
3. Add a new news article to `arescentral.org`_.

Bumping a release
-----------------

1. Run ``./scripts/release start ${VERSION?} rc${N?}`` on release/X.
2. ``git push --tags origin release/${VERSION?}`` the release to
   trigger a Travis build.
3. Add a new news article to `arescentral.org`_.

Finishing a release
-------------------

1. Run ``./scripts/release finish ${VERSION?}`` on release/X.
2. ``git push --tags stable`` to trigger a Travis build.
3. Build the `Ubuntu packages`_ from ``antares-X.X.X.tgz``.
4. Announce the release.

   a.  Push the new debs to `apt.arescentral.org`_.
   b.  Update `arescentral.org`_ to reference the latest version.
   c.  Add a new news article to `arescentral.org`_.
   d.  Send an email to the `announcement mailing list`_.
   e.  Update the topic on the `#ares irc channel`_.

5. Create a pull request and merge ``release/X.X.X`` to ``master``.

.. _downloads.arescentral.org: https://downloads.arescentral.org/
.. _arescentral.org: https://github.com/arescentral/arescentral.org
.. _apt.arescentral.org: https://github.com/arescentral/apt.arescentral.org
.. _announcement mailing list: https://groups.google.com/a/arescentral.org/group/antares-announce
.. _ares briefing room: http://www.ambrosiasw.com/forums/index.php?showforum=15
.. _#ares irc channel: irc://irc.afternet.org/#ares

Ubuntu packages
---------------

To build packages for Ubuntu, you need:

*  A machine running Ubuntu (any supported Ubuntu release should do).
*  A checked-out antares-deb_ repository.
*  A .tgz distfile with the latest Antares release.

.. _antares-deb: https://github.com/arescentral/antares-deb

Build packages for whatever `Ubuntu releases`__ are (still) supported,
unless they’re too old to build Antares. As of June 2020, that means
building packages for `xenial`, `bionic`, and `focal`.

__ https://en.wikipedia.org/wiki/Ubuntu_version_history

As of June 2020, the following branches are used::

   upstream (pristine copy of distfile)
     master (addition of debian/ directory, minus debian/changelog)
       xenial (xenial-specific debian/changelog and modifications)
       bionic (bionic-specific debian/changelog and modifications)
       focal (focal-specific debian/changelog and modifications)

Changelogs are specific to each Ubuntu release. This is because version
strings are specific to each release. Generally, they look like
``0.9.1-1~20.04``: in this example, ``0.9.1`` is the Antares version,
``1`` is the Debian revision, and ``20.04`` is the Ubuntu release
(focal). Putting the Ubuntu release version here ensures that Antares
will upgrade after a system upgrade.

Do one-time setup:

*  Install packages and configurations:

   .. code:: shell

      $ sudo apt-get install debhelper git-buildpackage pbuilder
      $ cp doc/pbuilderrc.sample ~root/.pbuilderrc

*  For each supported Ubuntu release, set up pbuilder (using focal as
   an example):

   .. code:: shell

      $ sudo OS=ubuntu DIST=focal ARCH=amd64 pbuilder create

Cut debs for an Antares release:

*  Import the distfile into ``master``:

   .. code:: shell

      $ git checkout upstream
      $ cp ${PATH_TO?}/antares-${VERSION?}.tgz ../antares_${VERSION?}.orig.tar.gz
      $ gbp import-orig ../antares_${VERSION?}.orig.tar.gz

*  For each supported Ubuntu release, update and build a deb for that
   release (using focal [20.04] as an example):

   .. code:: shell

      $ export DIST=focal
      $ export DISTVER=20.04
      $ export OS=ubuntu
      $ export ARCH=amd64
      $ git checkout ${DIST?}
      $ git merge master
      $ dch -v ${VERSION?}-1~${DISTVER?}
      $ dch -r
      $ git commit debian -m "Update ${VERSION?} for ${DIST?}"
      $ pdebuild --auto-debsign

..  -*- tab-width: 3; fill-column: 72 -*-
