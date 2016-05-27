#!/usr/bin/env python

import os
import sys
import tarfile
import zipfile


def main():
    progname, archive_format = sys.argv

    with open("./antares.gyp") as f:
        data = eval(f.read(), {"__builtins__": None}, {})

    version = data["target_defaults"]["variables"]["ANTARES_VERSION"]
    archive_root = "Antares-%s" % version

    if archive_format == "zip":
        path = "./Antares-Source-%s.%s" % (version, archive_format)
        with zipfile.ZipFile(path, "w", compression=zipfile.ZIP_DEFLATED) as z:
            for real_path, archive_path in walk(archive_root):
                    z.write(real_path, archive_path)
    elif archive_format in ["gz", "bz2"]:
        path = "./Antares-Source-%s.t%s" % (version, archive_format)
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
    if base.startswith("."):
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
    elif root.startswith("ext/") and (base == "ext"):
        return False
    return True


if __name__ == "__main__":
    main()
