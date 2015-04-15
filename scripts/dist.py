#!/usr/bin/env python

import os
import sys
import zipfile


def main():
    assert not sys.argv[1:]

    with open("./antares.gyp") as f:
        data = eval(f.read(), {"__builtins__": None}, {})

    version = data["target_defaults"]["variables"]["ANTARES_VERSION"]
    ziproot = "Antares-%s" % version
    path = "./Antares-Source-%s.zip" % version

    " test **/.* **/*.zip **/*.pyc **/build ext/*/ext"
    " ext/gmock-waf/waf ext/libpng-waf/waf ext/libsfz/waf ext/libzipxx/waf ext/rezin/waf"

    with zipfile.ZipFile(path, "w", compression=zipfile.ZIP_DEFLATED) as z:
        for root, dirs, files in os.walk("."):
            root = root[2:]
            files[:] = [f for f in files if should_write(root, f)]
            dirs[:] = [d for d in dirs if should_recurse(root, d)]

            for f in files:
                real_path = os.path.join(root, f)
                zip_path = os.path.join(ziproot, root, f)
                z.write(real_path, zip_path)


def should_write(root, base):
    _, ext = os.path.splitext(base)
    if base.startswith("."):
        return False
    elif ext in [".pyc", ".zip"]:
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
