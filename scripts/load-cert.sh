#!/bin/bash

set -o errexit
set -o nounset

if [[ -z "${DEVELOPER_P12_PASS_BASE64:-}" ]]; then
    exit 1
fi

CERT=build/developer.p12
KEYCHAIN=build.keychain

echo "$DEVELOPER_P12_BASE64" | base64 -D > $CERT
DEVELOPER_P12_PASS="$(echo $DEVELOPER_P12_PASS_BASE64 | base64 -D)"

security create-keychain -p "$DEVELOPER_P12_PASS" $KEYCHAIN
security default-keychain -s $KEYCHAIN
security unlock-keychain -p "$DEVELOPER_P12_PASS" $KEYCHAIN
security set-keychain-settings -t 21600 -l $KEYCHAIN

security import build/AppleWWDRCA.cer \
    -k $KEYCHAIN -T /usr/bin/codesign
security import $CERT -P "$DEVELOPER_P12_PASS" \
    -k $KEYCHAIN -T /usr/bin/codesign
security set-key-partition-list -S apple-tool:,apple: -s -k "$DEVELOPER_P12_PASS" \
    $KEYCHAIN >/dev/null
security find-identity -v | grep 'Developer ID Application'  # Fail if missing
