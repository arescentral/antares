#!/bin/bash

set -o errexit
set -o nounset

if [[ "$TRAVIS_OS_NAME" != "osx" ]] || [[ -z "${CERTIFICATE_OSX_PASS:-}" ]]; then
    exit
fi

security create-keychain -p "$CERTIFICATE_OSX_PASS" build.keychain
security default-keychain -s build.keychain
security unlock-keychain -p "$CERTIFICATE_OSX_PASS" build.keychain
security set-keychain-settings -t 3600 -l build.keychain

security import build/AppleWWDRCA.cer \
    -k build.keychain -T /usr/bin/codesign
security import build/developer.p12 -P "$CERTIFICATE_OSX_PASS" \
    -k build.keychain -T /usr/bin/codesign
security set-key-partition-list -S apple-tool:,apple: -s -k "$CERTIFICATE_OSX_PASS" \
    build.keychain >/dev/null
security find-identity -v | grep 'Developer ID Application'  # Fail if missing
