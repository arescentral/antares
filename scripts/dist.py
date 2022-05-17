#!/usr/bin/env python3
# Copyright (C) 2017 The Antares Authors
# This file is part of Antares, a tactical space combat game.
# Antares is free software, distributed under the LGPL+. See COPYING.

import os
import subprocess
import sys
import tarfile
import zipfile


def main():
    try:
        archive_content, archive_format = sys.argv[1:]
    except ValueError:
        sys.stdout.write(f"usage: {sys.argv[0]} src|mac|win zip|gz|bz2\n")
        sys.exit(64)

    try:
        tag = subprocess.check_output("git describe --tags --exact-match HEAD".split(),
                                      stderr=subprocess.DEVNULL)
        tag = tag.decode("utf-8").strip()
    except subprocess.CalledProcessError:
        tag = None
    if subprocess.check_output("git status --porcelain".split()):
        tag = None

    try:
        os.makedirs("dist")
    except FileExistsError:
        pass

    if tag is None:
        version = "git"
    else:
        version = tag.lstrip("v")
    if archive_content != "src":
        version = "%s-%s" % (archive_content, version)
    archive_root = "antares-%s" % version

    if archive_format == "zip":
        path = "dist/%s.%s" % (archive_root, archive_format)
        with zipfile.ZipFile(path, "w", compression=zipfile.ZIP_DEFLATED) as z:

            def add(real_path, archive_path):
                print(archive_path)
                z.write(real_path, archive_path)

            add_files(archive_content, archive_root, add)
    elif archive_format in ["gz", "bz2"]:
        path = "dist/%s.t%s" % (archive_root, archive_format)
        with tarfile.open(path, "w:%s" % archive_format) as t:

            def add(real_path, archive_path):
                print(archive_path)
                t.add(real_path, arcname=archive_path)

            add_files(archive_content, archive_root, add)
    else:
        raise RuntimeError(archive_format)


def add_files(archive_content, archive_root, add):
    if archive_content == "src":
        for real_path, archive_path in walk(archive_root, "."):
            add(real_path, archive_path)
    elif archive_content == "mac":
        for real_path, archive_path in walk("Antares.app", "out/mac/opt/Antares.app"):
            add(real_path, archive_path)
    elif archive_content == "win":
        for real_path, archive_path in walk("Antares/Antares Data ƒ", "data"):
            add(real_path, archive_path)
        add("out/win/opt/antares.exe", "Antares/Antares.exe")
    else:
        raise RuntimeError(archive_content)


def walk(archive_root, walk_root):
    for root, dirs, files in os.walk(walk_root):
        root = root[1 + len(walk_root):]
        files[:] = [f for f in files if should_write(f)]
        dirs[:] = [d for d in dirs if should_recurse(root, d)]

        for f in files:
            real_path = os.path.join(walk_root, root, f)
            archive_path = os.path.join(archive_root, root, f)
            yield real_path, archive_path


def should_write(base):
    _, ext = os.path.splitext(base)
    if base == ".gn":
        return True
    elif base.startswith("."):
        return False
    elif ext in [".pyc", ".zip", ".tgz", ".tbz2", "cer", "p12"]:
        return False
    return True


def should_recurse(root, base):
    path = os.path.join(root, base)
    if base.startswith("."):
        return False
    elif path in ["out", "test"]:
        return False
    elif path in ["data/scenarios", "data/downloads"]:
        return False
    elif root.startswith("ext/") and (base == "ext"):
        return False
    return True


if __name__ == "__main__":
    main()
