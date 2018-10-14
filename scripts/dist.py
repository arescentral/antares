#!/usr/bin/env python
# Copyright (C) 2017 The Antares Authors
# This file is part of Antares, a tactical space combat game.
# Antares is free software, distributed under the LGPL+. See COPYING.

from __future__ import division, print_function, unicode_literals

import os
import sys
import tarfile
import zipfile


def main():
    progname, archive_format = sys.argv

    with open("./BUILD.gn") as f:
        version = None
        for line in f.readlines():
            line = line.strip()
            if line.startswith("antares_version = "):
                version = line.split("=", 1)[1].strip().strip('"')
                break
    if not version:
        print("couldn't determine antares version")
        sys.exit(1)

    archive_root = "antares-%s" % version

    if archive_format == "zip":
        path = "./antares-%s.%s" % (version, archive_format)
        with zipfile.ZipFile(path, "w", compression=zipfile.ZIP_DEFLATED) as z:
            for real_path, archive_path in walk(archive_root):
                z.write(real_path, archive_path)
    elif archive_format in ["gz", "bz2"]:
        path = "./antares-%s.t%s" % (version, archive_format)
        with tarfile.open(path, "w:%s" % archive_format) as t:
            for real_path, archive_path in walk(archive_root):
                t.add(real_path, arcname=archive_path)
    else:
        raise RuntimeError(archive_format)


def walk(archive_root):
    for root, dirs, files in os.walk("."):
        root = root[2:]
        files[:] = [f for f in files if should_write(f)]
        dirs[:] = [d for d in dirs if should_recurse(root, d)]

        for f in files:
            real_path = os.path.join(root, f)
            archive_path = os.path.join(archive_root, root, f)
            yield real_path, archive_path


def should_write(base):
    _, ext = os.path.splitext(base)
    if base == ".gn":
        return True
    elif base.startswith("."):
        return False
    elif ext in [".pyc", ".zip", ".tgz", ".tbz2"]:
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
