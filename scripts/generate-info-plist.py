#!/usr/bin/env python

import sys

PLIST_TEMPLATE = """\
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleDevelopmentRegion</key>
    <string>English</string>
    <key>CFBundleExecutable</key>
    <string>Antares</string>
    <key>CFBundleIconFile</key>
    <string>Antares.icns</string>
    <key>CFBundleIdentifier</key>
    <string>org.arescentral.antares</string>
    <key>CFBundleInfoDictionaryVersion</key>
    <string>6.0</string>
    <key>CFBundleName</key>
    <string>Antares</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleSignature</key>
    <string>????</string>
    <key>CFBundleShortVersionString</key>
    <string>%(antares-version)s</string>
    <key>LSMinimumSystemVersion</key>
    <string>%(system-version)s</string>
    <key>CFBundleVersion</key>
    <string>1</string>
    <key>NSMainNibFile</key>
    <string>MainMenu</string>
    <key>NSPrincipalClass</key>
    <string>NSApplication</string>
</dict>
</plist>
"""

_, antares_version, system_version, out = sys.argv
with open(out, "w") as f:
    f.write(
        PLIST_TEMPLATE % {
            "antares-version": antares_version,
            "system-version": system_version,
        })
