NINJA=ninja -C out/cur
MAC_BIN=out/cur/Antares.app/Contents/MacOS/Antares
LINUX_BIN=out/cur/antares

all:
	@$(NINJA)

test: all
	scripts/test.py

smoke-test: all
	scripts/test.py --smoke

clean:
	@$(NINJA) -t clean

dist:
	scripts/dist.py

distclean:
	rm -Rf out/

run: all
	@[ -f $(MAC_BIN) ] && $(MAC_BIN) || true
	@[ ! -f $(MAC_BIN) ] && $(LINUX_BIN) || true

sign:
	codesign --force \
		--sign "Developer ID Application" \
		--entitlements resources/entitlements.plist \
		out/cur/Antares.app

friends:
	@echo "Sure! You can email me at sfiera@sfzmail.com."

love:
	@echo "Sorry, I'm not that kind of Makefile."

time:
	@echo "I've always got time for you."

.PHONY: all test smoke-test clean dist distclean run sign friends love time
