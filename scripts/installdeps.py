#!/usr/bin/env python

import collections
import sys

PACKAGE = {}
COMMAND = {}

UBUNTU = "ubuntu"
DEBIAN = "debian"
COMMAND[UBUNTU] = COMMAND[DEBIAN] = "apt-get install".split()
PACKAGE[UBUNTU] = PACKAGE[DEBIAN] = collections.OrderedDict([
    ("clang", "clang"),
    ("pkg-config", "pkg-config"),
    ("gl", "libgl1-mesa-dev"),
    ("glfw3", "libglfw3-dev"),
    ("glu", "libglu1-mesa-dev"),
    ("libc++", "libc++-dev"),
    ("libc++abi", "libc++abi-dev"),
    ("libmodplug", "libmodplug-dev"),
    ("libzip", "libzip-dev"),
    ("libpng16", "libpng16-dev"),
    ("neon", "libneon27-dev"),
    ("openal", "libopenal-dev"),
    ("sndfile", "libsndfile1-dev"),
    ("x11", "libx11-dev"),
    ("xcursor", "libxcursor-dev"),
    ("xinerama", "libxinerama-dev"),
    ("xrandr", "libxrandr-dev"),
    ("xxf86vm", "libxxf86vm-dev"),
    ("zlib", "zlib1g-dev"),
    ("python-gi", "python-gi"),
    ("python-gtk3", "gir1.2-gtk-3.0"),
])

if __name__ == "__main__":
    import os
    import platform
    try:
        from shlex import quote
    except ImportError:
        from pipes import quote

    if len(sys.argv) == 1:
        distro = platform.linux_distribution()[0].lower()
        if not distro:
            sys.stderr.write("This script is Linux-only, sorry.\n")
            sys.exit(1)
    elif len(sys.argv) == 2:
        distro = sys.argv[1]
    else:
        sys.stderr.write("usage: {} [DISTRO]\n".format(sys.argv[0]))
        sys.exit(64)

    if distro not in COMMAND:
        sys.stderr.write("I don't know {}, sorry.\n".format(distro))
        sys.exit(1)
    command = COMMAND[distro] + list(PACKAGE[distro].values())
    print(" ".join(quote(arg) for arg in command))
    os.execvp(command[0], command)
