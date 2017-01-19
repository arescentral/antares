#!/bin/zsh
# Copyright (C) 2017 The Antares Authors
# This file is part of Antares, a tactical space combat game.
# Antares is free software, distributed under the LGPL+. See COPYING.

set -o nounset
set -o errexit

if [ $# != 3 ]; then
    echo 1>&2 "usage: $0 IDENTIFIER [ -a APPLESINGLE | -f FORK ]"
    exit 1
fi
IDENTIFIER=$1; shift

# We want to use `read` with tab-separated values.
IFS="	"

# Create the data directory (fail if it already exists).
mkdir data
echo $IDENTIFIER > data/identifier
echo 1 > data/version

# The set of resources Antares supports in plugins (it would also
# support 'rot ' but that is a very bad idea).
RESOURCES=(
    'NLRP' 'PICT' 'SMIV' 'STR#' 'TEXT' 'bsob' 'intr' 'nlAG' 'nlFD'
    'nlFM' 'obac' 'race' 'snbf' 'sncd' 'snd ' 'snit' 'snro'
)

# Copy the resources into directories.  Any slashes in resource names
# will need to be modified to avoid making the filename look like a
# folder.
rezin "$@" ls >/dev/null
REZIN=(rezin "$@")
for TYPE in $RESOURCES; do
    if $REZIN ls | grep $TYPE; then
        mkdir -p "data/$TYPE"
        $REZIN ls $TYPE | while read ID NAME; do              
            NAME=$(echo "$NAME" | tr / _)
            $REZIN cat "$TYPE" -- $ID > "data/$TYPE/$ID $NAME.$TYPE"
        done
    fi
done

# PICT resources support lots of opcodes, but Antares only supports a
# simplified subset of PICT.  ImageMagick only outputs PICTs written in
# that subset, so round-trip them through ImageMagick.
#
# ImageMagick needs a 512-byte header to work with the PICTs, but we
# need it to not be present, so add and strip it while working.
if [ -d data/PICT ]; then
    for pict in data/PICT/*.PICT; do           
        echo "simplifying $pict"
        dd if=/dev/zero of=$pict.new.PICT count=1 2>/dev/null
        cat $pict >> $pict.new.PICT
        convert $pict.new.PICT $pict.png
        convert $pict.png $pict.new.PICT
        tail -c +513 $pict.new.PICT > $pict
        rm $pict.new.PICT $pict.png
    done
fi

# Zip the data folder. Make sure we are creating a new archive.
echo "zipping"
rm -f $IDENTIFIER.antaresplugin
zip -r $IDENTIFIER.antaresplugin data
