#!/bin/bash

set -o errexit

if [[ -z "$ANTARES" ]]; then
    echo >&2 'must set $ANTARES'
    exit 1
fi
cd $ANTARES

# Allow paths to utilities to be provided; use /usr/bin as default.
GIT=${GIT-/usr/bin/git}
ZIP=${ZIP-/usr/bin/zip}
GSUTIL=${GSUTIL-/usr/bin/gsutil}

# Before we start messing with the repo, make sure it's the right one.
grep >/dev/null '^APPNAME = "Antares"$' wscript

# Ensure that we are building from a clean and up-to-date checkout of
# the "develop" branch.  Remove untracked files.
$GIT checkout develop
$GIT reset --hard develop
$GIT pull
$GIT submodule init
$GIT submodule update
$GIT clean -f

# Check that there is a line 'VERSION = "..."' in the wscript file.
# Append the string "-nightly" to the version.
grep >/dev/null '^VERSION = ".*"$' wscript
sed -i '' 's/^VERSION = "\(.*\)"$/VERSION = "\1-nightly"/' wscript

# Build Antares.  Note that we don't make any attempt to clean the
# working directory before we build.
./configure -m opt
./waf

# Zip the product of the build.
cd build/antares
rm -f Antares-nightly.zip
$ZIP -r Antares-nightly.zip Antares.app

# Copy to downloads.arescentral.org.
$GSUTIL cp Antares-nightly.zip gs://downloads.arescentral.org/Antares/Antares-nightly.zip
echo "done"
