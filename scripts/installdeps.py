#!/usr/bin/env python3

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
    ("libpng16", "libpng-dev"),
    ("neon", "libneon27-dev"),
    ("openal", "libopenal-dev"),
    ("sndfile", "libsndfile1-dev"),
    ("x11", "libx11-dev"),
    ("xcursor", "libxcursor-dev"),
    ("xinerama", "libxinerama-dev"),
    ("xrandr", "libxrandr-dev"),
    ("xxf86vm", "libxxf86vm-dev"),
    ("zlib", "zlib1g-dev"),
])


def main():
    import os
    import platform
    try:
        from shlex import quote
    except ImportError:
        from pipes import quote

    if len(sys.argv) == 1:
        distro = linux_distribution()[0].lower()
        flags = []
        if not distro:
            sys.stderr.write("This script is Linux-only, sorry.\n")
            sys.exit(1)
    elif len(sys.argv) >= 2:
        distro = sys.argv[1]
        flags = sys.argv[2:]

    if distro not in COMMAND:
        sys.stderr.write("I don't know {}, sorry.\n".format(distro))
        sys.exit(1)
    command = COMMAND[distro] + flags + list(PACKAGE[distro].values())
    print(" ".join(quote(arg) for arg in command))
    os.execvp(command[0], command)


def linux_distribution():
    """Replacement for deprecated platform.linux_distribution()

    Only tested on Ubuntu so far.
    """
    try:
        with open("/etc/lsb-release") as f:
            lines = f.readlines()
    except (OSError, IOError):
        return ("", "", "")
    distrib = dict(line.split("=", 1) for line in lines)
    return (
        distrib.get("DISTRIB_ID", "").strip(),
        distrib.get("DISTRIB_RELEASE", "").strip(),
        distrib.get("DISTRIB_CODENAME", "").strip(),
    )


if __name__ == "__main__":
    main()
