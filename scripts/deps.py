#!/usr/bin/env python3

import collections
import os
import platform
import shlex
import subprocess
import sys

sys.path.append(os.path.join(os.path.dirname(__file__), "..", "build", "lib", "scripts"))
import cfg

DEBIAN = cfg.Distro(
    name="debian",
    packages={
        # Binaries
        "clang": "clang",
        "clang++": "clang",
        "gn": "gn",
        "ninja": "ninja-build",
        "pkg-config": "pkg-config",

        # Libraries
        "gl": "libgl1-mesa-dev",
        "glfw3": "libglfw3-dev",
        "glu": "libglu1-mesa-dev",
        "libmodplug": "libmodplug-dev",
        "libpng": "libpng-dev",
        "libzip": "libzip-dev",
        "neon": "libneon27-dev",
        "openal": "libopenal-dev",
        "sndfile": "libsndfile1-dev",
        "x11": "libx11-dev",
        "xcursor": "libxcursor-dev",
        "xinerama": "libxinerama-dev",
        "xrandr": "libxrandr-dev",
        "xxf86vm": "libxxf86vm-dev",
        "zlib": "zlib1g-dev",
    },
    sources=[
        ("arescentral", "http://apt.arescentral.org", "contrib",
         "5A4F5210FF46CEE4B799098BAC879AADD5B51AE9"),
    ],
    install="apt-get install".split(),
    update="apt-get update".split(),
    add_key="apt-key adv --keyserver keyserver.ubuntu.com --recv".split(),
)

MAC = cfg.Distro(
    name="mac",
    packages={
        "ninja": "ninja",
        "gn": "sfiera/gn/gn",
    },
    sources=[],
    install="brew install".split(),
    update=None,
    add_key=None,
)

WIN = cfg.Distro(
    name="win",
    packages={
        "ninja":
        "https://github.com/ninja-build/ninja/releases/download/v1.10.2/ninja-win.zip:ninja.exe",
        "gn": "https://chrome-infra-packages.appspot.com/dl/gn/gn/windows-amd64/+/latest:gn.exe",
    },
    sources=[],
    install="python scripts/download".split(),
    update=None,
    add_key=None,
)

DISTROS = {d.name: d for d in [DEBIAN, MAC, WIN]}

if __name__ == "__main__":
    cfg.install_or_check(DISTROS)
