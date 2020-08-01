#!/usr/bin/env python3

import argparse
import os
import sys
import textwrap


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--prefix", required=True)
    parser.add_argument("--out", required=True)
    args = parser.parse_args()

    script = textwrap.dedent("""
        #!/bin/bash

        set -o errexit
        set -o nounset

        PREFIX=${{DESTDIR-}}{args.prefix}
        BINDIR=$PREFIX/games
        APPDIR=$PREFIX/share/applications
        MIMEDIR=$PREFIX/share/mime/packages
        ICONDIR=$PREFIX/share/icons
        DATADIR=$PREFIX/share/games/antares

        BINARIES="antares antares-install-data"

        @() {{
            echo "$@"
            "$@"
        }}

        case "$#-$1" in
            1-bin)
                @ install -m 755 -d $BINDIR
                for BIN in $BINARIES; do
                    @ install -m 755 out/cur/$BIN $BINDIR/antares
                done
                ;;

            1-data)
                for ICON in resources/antares.iconset/icon_*.png; do
                    ICONSIZE=${{ICON#resources/antares.iconset/icon_}}
                    ICONSIZE=${{ICON%.png}}
                    @ install -m 755 -d $ICONDIR/hicolor/$ICONSIZE/apps
                    @ install -m 644 $ICON $ICONDIR/hicolor/$ICONSIZE/apps/antares.png
                done

                @ install -m 755 -d $APPDIR
                @ install -m 644 resources/antares.desktop $APPDIR

                @ install -m 755 -d $MIMEDIR
                @ install -m 644 resources/antares.xml $MIMEDIR

                @ install -m 755 -d $DATADIR/app
                for DATA in data/*; do
                    if [[ -d "$DATA" ]]; then
                        @ cp -r data/fonts $DATADIR/app
                    else
                        @ install -m 644 $DATA $DATADIR/app
                    fi
                done
                ;;

            1-scenario)
                   @ out/cur/antares-install-data -s $DATADIR/downloads -d $DATADIR/scenarios
               ;;
        esac
    """).lstrip().format(args=args)

    try:
        with open(args.out) as f:
            if f.read() == script:
                sys.exit(0)
    except FileNotFoundError:
        pass

    with open(args.out, "w") as f:
        f.write(script)
    os.chmod(args.out, 0o755)


if __name__ == "__main__":
    main()
