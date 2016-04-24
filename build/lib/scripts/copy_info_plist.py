#!/usr/bin/env python
#
# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys

_, src, dst = sys.argv[:3]
subs = sys.argv[3:]
with open(src) as f:
    data = f.read()

for sub in subs:
    key, val = sub.split("=", 1)
    key = "${%s}" % key
    data = data.replace(key, val)

with open(dst, "w") as f:
    f.write(data)
