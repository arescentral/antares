#!/usr/bin/env python
#
# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import cfg
import os
import sys
import urllib2
import platform


def main():
    assert not(sys.argv[1:])  # no args to script

    host = cfg.host_os(), platform.machine()
    try:
        host = {
            ("mac", "x86_64"): "mac",
            ("linux", "x86_64"): "linux64",
        }[host]
    except KeyError:
        print("no gn available for %s on %s" % host)

    repo = "https://chromium.googlesource.com/chromium/buildtools/+/master"
    digest = download("%s/%s/gn.sha1?format=TEXT" %
                      (repo, host)).decode("base64")
    data = download("https://storage.googleapis.com/chromium-gn/%s" % digest)

    out = os.path.join(os.path.dirname(__file__), "gn")
    with open(out, "w") as f:
        f.write(data)
    os.chmod(out, 0o755)


def download(url):
    try:
        return urllib2.urlopen(url).read()
    except urllib2.HTTPError as e:
        print("%s: %s" % (url, e))
        sys.exit(1)

if __name__ == "__main__":
    main()
