# Copyright (C) 2020 The Antares Authors
# This file is part of Antares, a tactical space combat game.
# Antares is free software, distributed under the LGPL+. See COPYING.

all:

OUT ?= out/cur
ANTARES_ROOT ?= .
EXT ?= ext
PROCYON ?= $(EXT)/procyon
GMOCK ?= $(EXT)/gmock
LIBSFZ_ROOT ?= $(EXT)/libsfz
LIBZIPXX_ROOT ?= $(EXT)/libzipxx
LIBMODPLUG_ROOT ?= $(EXT)/libmodplug
LIBSNDFILE_ROOT ?= $(EXT)/libsndfile
LIBPNG_ROOT ?= $(EXT)/libpng
GLFW_ROOT ?= $(EXT)/glfw
MKDIR_P ?= mkdir -p

VERSION=
MACOSX_VERSION_MIN=10.7

-include $(OUT)/config.mk

CPPFLAGS += -MMD -MP -D ANTARES_VERSION=\"$(VERSION)\"
CFLAGS += -Wall
CXXFLAGS += -std=c++11 -stdlib=libc++ -Wall
LDFLAGS += -std=c++11 -stdlib=libc++

ifeq ($(TARGET_OS),mac)
-mmacosx-version-min=10.7
endif

include ext/gmock/build/targets.mk
include ext/procyon/build/targets.mk
include ext/libsfz/build/targets.mk
include ext/libzipxx/build/targets.mk
include ext/libmodplug/build/targets.mk
include ext/libsndfile/build/targets.mk
include ext/libpng/build/targets.mk
include ext/glfw/build/targets.mk
include build/targets.mk

all: $(ANTARES) \
	$(REPLAY) $(OFFSCREEN) \
	$(TINT) $(OBJECT_DATA) $(BUILD_PIX) $(SHAPES) \
	$(COLOR_TEST) $(EDITABLE_TEXT_TEST) $(FIXED_TEST)

.PHONY: run
run: all
	@[ -f $(MAC_BIN) ] && $(MAC_BIN) || true

.PHONY: test
test: all
	scripts/test.py

.PHONY: test-wine
test-wine: all
	scripts/test.py --wine

.PHONY: smoke-test
smoke-test: all
	scripts/test.py --smoke

.PHONY: dist
dist:
	scripts/dist.py zip
	scripts/dist.py gz
	scripts/dist.py bz2

.PHONY: macdist
macdist: sign notarize
	scripts/dist.py mac

.PHONY: sign
sign: build
	codesign --force \
		--options runtime \
		--timestamp \
		--sign "Developer ID Application" \
		--entitlements resources/entitlements.plist \
		out/cur/Antares.app

.PHONY: notarize
notarize: sign
	scripts/notarize

.PHONY: install-deps
install-deps:
	@scripts/installdeps.py

BINDIR=$(PREFIX)/games
APPDIR=$(PREFIX)/share/applications
ICONDIR=$(PREFIX)/share/icons
DATADIR=$(PREFIX)/share/games/antares

ifeq ($(TARGET_OS),linux)
.PHONY: install
install: install-bin install-data install-scenario

.PHONY: install-bin
install-bin: build
	install -m 755 -d $(DESTDIR)$(BINDIR)
	install -m 755 out/cur/antares-glfw $(DESTDIR)$(BINDIR)/antares-glfw
	install -m 755 out/cur/antares-install-data $(DESTDIR)$(BINDIR)/antares-install-data

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
	cp -r data/text $(DESTDIR)$(DATADIR)/app

.PHONY: install-scenario
install-scenario: build
	out/cur/antares-install-data -s $(DESTDIR)$(DATADIR)/downloads -d $(DESTDIR)$(DATADIR)/scenarios
endif

.PHONY: clean
clean:
	$(RM) -r $(OUT)/

.PHONY: distclean
distclean:
	$(RM) -r out

.PHONY: friends
friends:
	@echo "Sure! You can email me at sfiera@twotaled.com."

.PHONY: love
love:
	@echo "Sorry, I'm not that kind of Makefile."

.PHONY: time
time:
	@echo "I've always got time for you."
