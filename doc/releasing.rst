Releasing Antares
=================

Starting a release
------------------

1.  Run ``./scripts/release start ${VERSION?}`` (this checks out the
    release branch).
2.  Run ``make dist`` to build ``antares-X.X.X~rc1.zip``.
3.  Unzip the distfile and run ``./configure && make appdist`` in it to
    build ``antares-mac-X.X.X~rc1.zip``.
4.  Upload the archives to `downloads.arescentral.org`_.
5.  Add a new news article to `arescentral.org`_.

..  _BUILD.gn: /BUILD.gn

Bumping a release
-----------------

1.  Checkout the ``release/X.X.X`` branch.
2.  Increase the ``rcX`` number in ``antares_version`` in `BUILD.gn`_.
3.  Run ``make dist`` to build ``antares-X.X.X~rc1.zip``.
4.  Unzip the distfile and run ``./configure && make appdist`` in it to
    build ``antares-mac-X.X.X~rc1.zip``.
5.  Upload the archives to `downloads.arescentral.org`_.
6.  Add a new news article to `arescentral.org`_.

Finishing a release
-------------------

1.  Run ``./scripts/release finish ${VERSION?}`` (this checks out the
    stable branch).
2.  Run ``make dist`` to build ``antares-X.X.X.zip``.
3.  Unzip the distfile and run ``./configure && make appdist`` in it to
    build ``antares-mac-X.X.X~rc1.zip``.
4.  Build the `Ubuntu packages`_ from ``antares-X.X.X.tgz``.
5.  Upload the archives to `downloads.arescentral.org`_.
6.  Announce the release.

    a.  Push ``stable`` to Github, including ``--tags``.
    b.  Push the new debs to `apt.arescentral.org`_.
    c.  Update `arescentral.org`_ to reference the latest version.
    d.  Add a new news article to `arescentral.org`_.
    e.  Send an email to the `announcement mailing list`_.
    f.  Create a new thread in the `Ares Briefing Room`_.
    g.  Update the topic on the `#ares irc channel`_.

7.  Create a pull request and merge ``release/X.X.X`` to ``master``.

..  _downloads.arescentral.org: http://downloads.arescentral.org/
..  _arescentral.org: https://github.com/arescentral/arescentral.org
..  _apt.arescentral.org: https://github.com/arescentral/apt.arescentral.org
..  _announcement mailing list: https://groups.google.com/a/arescentral.org/group/antares-announce
..  _ares briefing room: http://www.ambrosiasw.com/forums/index.php?showforum=15
..  _#ares irc channel: irc://irc.afternet.org/#ares

Ubuntu packages
---------------

To build packages for Ubuntu, you need:

*   A machine running Ubuntu (any supported release should do).
*   A checked-out antares-deb_ repository.
*   A .tgz distfile with the latest release.

..  _antares-deb: https://github.com/arescentral/antares-deb

Build packages for whatever `Ubuntu releases`__ are (still) supported,
unless they’re too old to build Antares (this excludes `trusty`). As of
October 2018, that means building packages for `xenial`, `bionic`, and
`cosmic`.

__ https://en.wikipedia.org/wiki/Ubuntu_version_history

As of October 2018, the following branches are used::

    upstream (pristine copy of distfile)
      master (addition of debian/ directory, minus debian/changelog)
        xenial (xenial-specific debian/changelog and modifications)
        bionic (bionic-specific debian/changelog and modifications)
        cosmic (cosmic-specific debian/changelog and modifications)

Changelogs are specific to each Ubuntu release. This is because version
strings are specific to each release. Generally, they look like
``0.9.0-1~18.04``: in this example, ``0.9.0`` is the Antares version,
``1`` is the Debian revision, and ``18.04`` is the Ubuntu release
(bionic). Putting the Ubuntu release version here ensures that Antares
will upgrade after a system upgrade.

Do one-time setup:

*   Install packages and configurations:

    ..  code:: shell

        $ sudo apt-get install debhelper git-buildpackage pbuilder
        $ cp doc/pbuilderrc.sample ~/.pbuilderrc

*   For each supported Ubuntu release, set up pbuilder (using bionic as
    an example):

    ..  code:: shell

        $ sudo OS=ubuntu DIST=bionic ARCH=amd64 pbuilder create

Cut debs for an Antares release:

*   Import the distfile into ``master``:

    ..  code:: shell

        $ git checkout release
        $ cp ${PATH_TO?}/antares-${VERSION?}.tgz ../antares_${VERSION?}.orig.tar.gz
        $ gbp import-orig ../antares_${VERSION?}.orig.tar.gz

*   For each supported Ubuntu release, update and build a deb for that
    release (using bionic [18.04] as an example):

    ..  code:: shell

        $ export DIST=bionic
        $ export DISTVER=18.04
        $ export OS=ubuntu
        $ export ARCH=amd64
        $ git checkout ${DIST?}
        $ git merge release
        $ dch -v ${VERSION?}-1~${DISTVER?}
        $ dch -r
        $ git commit debian -m "Update ${VERSION?} for ${DIST?}"
        $ pdebuild
