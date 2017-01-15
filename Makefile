-include out/cur/args.gn
NINJA=scripts/ninja.sh -C out/cur
MAC_BIN=out/cur/Antares.app/Contents/MacOS/Antares

BINDIR=$(prefix)/bin
DATADIR=$(prefix)/share/antares/app

all:
	@$(NINJA)

test: all
	scripts/test.py

smoke-test: all
	scripts/test.py --smoke

clean:
	@$(NINJA) -t clean

dist:
	scripts/dist.py zip
	scripts/dist.py gz
	scripts/dist.py bz2

distclean:
	rm -Rf out/

run: all
	@[ -f $(MAC_BIN) ] && $(MAC_BIN) || true
	@[ ! -f $(MAC_BIN) ] && scripts/antares-launcher || true

sign:
	codesign --force \
		--sign "Developer ID Application" \
		--entitlements resources/entitlements.plist \
		out/cur/Antares.app

install: all
ifeq ($(target_os), "linux")
	install -m 755 -d $(DESTROOT)$(BINDIR)
	install -m 755 scripts/antares-launcher $(DESTROOT)$(BINDIR)/antares
	install -m 755 out/cur/antares-glfw $(DESTROOT)$(BINDIR)/antares-glfw
	install -m 755 out/cur/antares-install-data $(DESTROOT)$(BINDIR)/antares-install-data
	install -m 755 out/cur/antares-ls-scenarios $(DESTROOT)$(BINDIR)/antares-ls-scenarios
	install -m 755 -d $(DESTROOT)$(DATADIR)
	install -m 644 resources/Antares.png $(DESTROOT)$(DATADIR)
	install -m 644 data/COPYING $(DESTROOT)$(DATADIR)
	install -m 644 data/AUTHORS $(DESTROOT)$(DATADIR)
	install -m 644 data/README.md $(DESTROOT)$(DATADIR)
	cp -r data/fonts $(DESTROOT)$(DATADIR)
	cp -r data/interfaces $(DESTROOT)$(DATADIR)
	cp -r data/music $(DESTROOT)$(DATADIR)
	cp -r data/pictures $(DESTROOT)$(DATADIR)
	cp -r data/rotation-table $(DESTROOT)$(DATADIR)
	cp -r data/strings $(DESTROOT)$(DATADIR)
	cp -r data/text $(DESTROOT)$(DATADIR)
else
	@echo "nothing to install on '$(target_os)'."
endif

friends:
	@echo "Sure! You can email me at sfiera@sfzmail.com."

love:
	@echo "Sorry, I'm not that kind of Makefile."

time:
	@echo "I've always got time for you."

.PHONY: all test smoke-test clean dist distclean run sign friends love time
