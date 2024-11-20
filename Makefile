# Copyright (C) 2017 The Antares Authors
# This file is part of Antares, a tactical space combat game.
# Antares is free software, distributed under the LGPL+. See COPYING.

CUR := out/$(shell readlink out/cur || cat out/cur)
MAC_BIN := $(CUR)/Antares.app/Contents/MacOS/Antares
BUILD := $(CUR)/ninja -C $(CUR)

.PHONY: build
build:
	@$(BUILD)

.PHONY: test
test: build
	scripts/test.py

.PHONY: test-wine
test-wine: build
	scripts/test.py --wine

.PHONY: smoke-test
smoke-test: build
	scripts/test.py --smoke

.PHONY: clean
clean:
	@$(BUILD) -t clean

.PHONY: dist
dist:
	scripts/dist.py src zip
	scripts/dist.py src gz
	scripts/dist.py src bz2

.PHONY: distclean
distclean:
	rm -Rf out/
	rm -f scripts/*.pyc build/lib/scripts/*.pyc

.PHONY: run
run: build
	@[ -f $(MAC_BIN) ] && $(MAC_BIN) || true

.PHONY: sign
sign: build
	codesign --force \
		--options runtime \
		--timestamp \
		--sign "Developer ID Application" \
		--entitlements resources/entitlements.plist \
		$(CUR)/Antares.app

.PHONY: notarize
notarize: sign
	scripts/notarize

.PHONY: install-deps
install-deps:
	@scripts/installdeps.py

.PHONY: install
install: install-bin install-data install-scenario

.PHONY: install-bin
install-bin: build
	@$(CUR)/install bin

.PHONY: install-data
install-data: build
	@$(CUR)/install data

.PHONY: install-scenario
install-scenario: build
	@$(CUR)/install scenario

.PHONY: pull-request
pull-request:
	hub pull-request -b arescentral:master

.PHONY: friends
friends:
	@echo "Sure! You can email me at sfiera@twotaled.com."

.PHONY: love
love:
	@echo "Sorry, I'm not that kind of Makefile."

.PHONY: time
time:
	@echo "I've always got time for you."
