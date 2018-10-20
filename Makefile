# Copyright (C) 2017 The Antares Authors
# This file is part of Antares, a tactical space combat game.
# Antares is free software, distributed under the LGPL+. See COPYING.

-include out/cur/args.gn
NINJA=build/lib/bin/ninja -C out/cur
MAC_BIN=out/cur/Antares.app/Contents/MacOS/Antares

BINDIR=$(prefix)/games
APPDIR=$(prefix)/share/applications
ICONDIR=$(prefix)/share/icons
DATADIR=$(prefix)/share/games/antares

.PHONY: build
build:
	@$(NINJA)

.PHONY: test
test: build
	scripts/test.py

.PHONY: smoke-test
smoke-test: build
	scripts/test.py --smoke

.PHONY: clean
clean:
	@$(NINJA) -t clean

.PHONY: dist
dist:
	scripts/dist.py zip
	scripts/dist.py gz
	scripts/dist.py bz2

.PHONY: macdist
macdist: sign
	scripts/dist.py mac

.PHONY: distclean
distclean:
	rm -Rf out/
	rm -f build/lib/scripts/*.pyc build/lib/scripts/gn build/lib/scripts/ninja

.PHONY: run
run: build
	@[ -f $(MAC_BIN) ] && $(MAC_BIN) || true
	@[ ! -f $(MAC_BIN) ] && scripts/antares_launcher.py || true

.PHONY: sign
sign: build
	codesign --force \
		--sign "Developer ID Application" \
		--entitlements resources/entitlements.plist \
		out/cur/Antares.app

.PHONY: install-deps
install-deps:
	@scripts/installdeps.py

ifeq ($(target_os), "linux")
.PHONY: install
install: install-bin install-data install-scenario

.PHONY: install-bin
install-bin: build
	install -m 755 -d $(DESTDIR)$(BINDIR)
	install -m 755 scripts/antares_launcher.py $(DESTDIR)$(BINDIR)/antares
	install -m 755 out/cur/antares-glfw $(DESTDIR)$(BINDIR)/antares-glfw
	install -m 755 out/cur/antares-install-data $(DESTDIR)$(BINDIR)/antares-install-data
	install -m 755 out/cur/antares-ls-scenarios $(DESTDIR)$(BINDIR)/antares-ls-scenarios

.PHONY: install-data
install-data: build
	install -m 755 -d $(DESTDIR)$(ICONDIR)/hicolor/16x16/apps
	install -m 644 resources/antares.iconset/icon_16x16.png $(DESTDIR)$(ICONDIR)/hicolor/16x16/apps/antares.png
	install -m 755 -d $(DESTDIR)$(ICONDIR)/hicolor/32x32/apps
	install -m 644 resources/antares.iconset/icon_32x32.png $(DESTDIR)$(ICONDIR)/hicolor/32x32/apps/antares.png
	install -m 755 -d $(DESTDIR)$(ICONDIR)/hicolor/128x128/apps
	install -m 644 resources/antares.iconset/icon_128x128.png $(DESTDIR)$(ICONDIR)/hicolor/128x128/apps/antares.png
	install -m 755 -d $(DESTDIR)$(ICONDIR)/hicolor/512x512/apps
	install -m 644 resources/antares.iconset/icon_512x512.png $(DESTDIR)$(ICONDIR)/hicolor/512x512/apps/antares.png
	
	install -m 755 -d $(DESTDIR)$(APPDIR)
	install -m 644 resources/antares.desktop $(DESTDIR)$(APPDIR)
	
	install -m 755 -d $(DESTDIR)$(DATADIR)/app
	install -m 644 data/COPYING $(DESTDIR)$(DATADIR)/app
	install -m 644 data/AUTHORS $(DESTDIR)$(DATADIR)/app
	install -m 644 data/README.md $(DESTDIR)$(DATADIR)/app
	install -m 644 data/info.pn $(DESTDIR)$(DATADIR)/app
	install -m 644 data/rotation-table $(DESTDIR)$(DATADIR)/app
	cp -r data/fonts $(DESTDIR)$(DATADIR)/app
	cp -r data/interfaces $(DESTDIR)$(DATADIR)/app
	cp -r data/levels $(DESTDIR)$(DATADIR)/app
	cp -r data/music $(DESTDIR)$(DATADIR)/app
	cp -r data/objects $(DESTDIR)$(DATADIR)/app
	cp -r data/pictures $(DESTDIR)$(DATADIR)/app
	cp -r data/races $(DESTDIR)$(DATADIR)/app
	cp -r data/replays $(DESTDIR)$(DATADIR)/app
	cp -r data/sounds $(DESTDIR)$(DATADIR)/app
	cp -r data/sprites $(DESTDIR)$(DATADIR)/app
	cp -r data/strings $(DESTDIR)$(DATADIR)/app

.PHONY: install-scenario
install-scenario: build
	out/cur/antares-install-data -s $(DESTDIR)$(DATADIR)/downloads -d $(DESTDIR)$(DATADIR)/scenarios
endif

.PHONY: pull-request
pull-request:
	hub pull-request -b arescentral:master

.PHONY: test-install
test-install: build
	# Check that deps for launcher were installed:
	python -c "from scripts import antares_launcher"

	# Check that antares-ls-scenarios finds scenario only after installation:
	sudo rm -Rf $(prefix)/share/games/antares
	! out/cur/antares-ls-scenarios
	sudo make install
	$(prefix)/games/antares-ls-scenarios

.PHONY: friends
friends:
	@echo "Sure! You can email me at sfiera@sfzmail.com."

.PHONY: love
love:
	@echo "Sorry, I'm not that kind of Makefile."

.PHONY: time
time:
	@echo "I've always got time for you."
