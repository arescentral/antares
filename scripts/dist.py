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
    progname, archive_format = sys.argv

    try:
        tag = subprocess.check_output("git describe --tags --exact-match HEAD".split())
        version = tag.decode().strip().lstrip("v")
    except subprocess.CalledProcessError:
        tag = None
        version = "git"

    if subprocess.check_output("git status --porcelain".split()):
        tag = None
        version = "git"

    if os.environ.get("TRAVIS") == "true":
        if os.environ["TRAVIS_TAG"]:
            assert tag == os.environ["TRAVIS_TAG"]
        elif os.environ["TRAVIS_BRANCH"] != "master":
            print("not building distfiles; not on master")
            return
        elif os.environ["TRAVIS_PULL_REQUEST"] != "false":
            print("not building distfiles; pull request")
            return

    try:
        os.makedirs("dist")
    except FileExistsError:
        pass

    archive_root = "antares-%s" % version
    if archive_format == "zip":
        path = "dist/%s.%s" % (archive_root, archive_format)
        with zipfile.ZipFile(path, "w", compression=zipfile.ZIP_DEFLATED) as z:
            for real_path, archive_path in walk(archive_root, "."):
                z.write(real_path, archive_path)
    elif archive_format in ["gz", "bz2"]:
        path = "dist/%s.t%s" % (archive_root, archive_format)
        with tarfile.open(path, "w:%s" % archive_format) as t:
            for real_path, archive_path in walk(archive_root, "."):
                t.add(real_path, arcname=archive_path)
    elif archive_format == "mac":
        path = "dist/antares-mac-%s.zip" % version
        with zipfile.ZipFile(path, "w", compression=zipfile.ZIP_DEFLATED) as z:
            for real_path, archive_path in walk("Antares.app", "out/mac/opt/Antares.app"):
                z.write(real_path, archive_path)
    else:
        raise RuntimeError(archive_format)


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
