#!/bin/bash

# Pick ninja or ninja-build as appropriate.

set -o nounset
set -o errexit

if which ninja-build 1>/dev/null 2>&1; then
    exec ninja-build "$@"
else
    exec ninja "$@"
fi
