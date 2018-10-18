#!/bin/bash

set -o errexit
set -o nounset

if [[ "$TRAVIS_OS" != "mac" ]]; then
    exit
fi

security create-keychain -p "$CERTIFICATE_OSX_PASS" build.keychain
security default-keychain -s build.keychain
security unlock-keychain -p "$CERTIFICATE_OSX_PASS"  build.keychain
security import build/developer.p12 -k build.keychain -P "$CERTIFICATE_OSX_PASS" -T /usr/bin/codesign
security find-identity -v
