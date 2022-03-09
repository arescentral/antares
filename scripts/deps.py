#!/usr/bin/env python3

import collections
import os
import platform
import shlex
import subprocess
import sys

sys.path.append(os.path.join(os.path.dirname(__file__), "..", "build", "lib", "scripts"))
try:
    import cfg
except ImportError:
    pass

Distro = collections.namedtuple("Distro", "name packages sources install update add_key".split())

DEBIAN = Distro(
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

MAC = Distro(
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

WIN = Distro(
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


def main():
    import argparse

    if platform.system() == "Darwin":
        distro, codename = MAC.name, None
    elif platform.system() == "Linux":
        _, distro, codename = cfg.dist_proto()
    elif platform.system() == "Windows":
        distro, codename = WIN.name, None
    else:
        sys.stderr.write("This script is Mac-, Linux-, and Windows-only, sorry.\n")
        sys.exit(1)

    parser = argparse.ArgumentParser(description="Install build deps for Antares")
    parser.add_argument("action", choices="check install".split())
    parser.add_argument("--distro", choices=sorted(DISTROS.keys()), default=distro)
    parser.add_argument("--codename", type=str, default=codename)
    parser.add_argument("--dry-run", action="store_const", const=True, default=False)
    args, flags = parser.parse_known_args()

    distro = DISTROS[args.distro]
    if args.action == "check":
        if not check(distro=distro, codename=args.codename):
            sys.exit(1)
    elif args.action == "install":
        install(distro=distro, codename=args.codename, dry_run=args.dry_run, flags=flags)


def check(*, distro, codename, prefix=""):
    checkers = {
        "clang": cfg.check_clang,
        "clang++": cfg.check_clangxx,
        "gn": cfg.check_gn,
        "ninja": cfg.check_ninja,
        "pkg-config": cfg.check_pkg_config,
    }

    pkg_config = None
    missing_pkgs = []
    config = {}
    for name in distro.packages:
        if name in checkers:
            dep = checkers[name]()
            if dep is None:
                missing_pkgs.append(name)
            else:
                config[name] = dep
            continue

        pkg_config = config.pop("pkg-config", pkg_config)
        if pkg_config is None:
            continue
        if not cfg.check_pkg(pkg_config, name):
            missing_pkgs.append(name)

    if not missing_pkgs:
        return config
    commands = []

    for name, url, component, key in distro.sources:
        path = "/etc/apt/sources.list.d/%s.list" % name
        if os.path.exists(path):
            continue
        commands.extend([
            _command(prefix, distro.add_key + [key]),
            "%s | %s" % (
                _command("", ["echo", "deb", url, codename, component]),
                _command(prefix, ["tee", path]),
            ),
        ])

    if distro.update:
        commands.append(_command(prefix, distro.update))
    missing_pkgs = sorted(set(distro.packages[pkg] for pkg in missing_pkgs))
    commands.append(_command(prefix, distro.install + missing_pkgs))

    print()
    print("missing dependencies: %s" % " ".join(missing_pkgs))
    if len(missing_pkgs) == 1:
        print("On %s, you can install it with:" % codename)
    else:
        print("On %s, you can install them with:" % codename)
    print()
    for command in commands:
        print("    $ %s" % command)
    print()
    print("Then, try ./configure again")
    return None


def _command(prefix, args):
    return " ".join(shlex.quote(x) for x in shlex.split(prefix) + args)


def install(*, distro, codename, dry_run=False, flags=[]):
    update = False
    for name, url, component, key in distro.sources:
        path = "/etc/apt/sources.list.d/%s.list" % name
        if os.path.exists(path):
            continue
        run(dry_run, distro.add_key + [key])
        line = "deb %s %s %s" % (url, codename, component)
        write(dry_run, line, path)
        update = True

    if update:
        run(dry_run, distro.update)
    run(dry_run, distro.install + list(distro.packages.values()) + flags)


def run(dry_run, command):
    print(" ".join(shlex.quote(arg) for arg in command))
    if not dry_run:
        subprocess.check_call(command)


def write(dry_run, content, path):
    print("+ tee %s" % shlex.quote(path))
    print(content)
    if not dry_run:
        with open(path, "w") as f:
            f.write(content)


if __name__ == "__main__":
    main()
