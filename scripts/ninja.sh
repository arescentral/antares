#!/bin/bash
# Copyright (C) 2017 The Antares Authors
# This file is part of Antares, a tactical space combat game.
# Antares is free software, distributed under the LGPL+. See COPYING.

# Pick ninja or ninja-build as appropriate.

set -o nounset
set -o errexit

if which ninja-build 1>/dev/null 2>&1; then
    exec ninja-build "$@"
else
    exec ninja "$@"
fi
