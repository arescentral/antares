#!/usr/bin/env python
#
# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import cfg
import os
import sys
import urllib2

def download(url):
    try:
        return urllib2.urlopen(url).read()
    except urllib2.HTTPError as e:
        print("%s: %s" % (url, e))
        sys.exit(1)

assert not(sys.argv[1:])  # no args to script
out_path = os.path.join(os.path.dirname(__file__), "gn")
host_os = cfg.host_os()
repo = "https://chromium.googlesource.com/chromium/buildtools/+/master"
digest = download("%s/%s/gn.sha1?format=TEXT" % (repo, host_os)).decode("base64")
data = download("https://storage.googleapis.com/chromium-gn/%s" % digest)
with open(out_path, "w") as f:
    f.write(data)
os.chmod(out_path, 0755)
