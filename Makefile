NINJA=ninja -C out/cur

all:
	@$(NINJA)

test: all
	out/cur/fixed-test
	
	scripts/data-test build-pix
	scripts/data-test object-data
	scripts/data-test shapes
	scripts/data-test tint
	
	scripts/offscreen-test main-screen
	scripts/offscreen-test mission-briefing --text
	scripts/offscreen-test options
	scripts/offscreen-test pause --text
	
	scripts/replay-test and-it-feels-so-good  
	scripts/replay-test blood-toil-tears-sweat  
	scripts/replay-test hand-over-fist
	scripts/replay-test make-way  
	scripts/replay-test out-of-the-frying-pan  
	scripts/replay-test space-race
	scripts/replay-test the-left-hand  
	scripts/replay-test the-mothership-connection  
	scripts/replay-test the-stars-have-ears
	scripts/replay-test while-the-iron-is-hot  
	scripts/replay-test yo-ho-ho  
	scripts/replay-test you-should-have-seen-the-one-that-got-away  

clean:
	@$(NINJA) -t clean

dist:
	scripts/dist.py

distclean:
	rm -Rf out/

run: all
	out/cur/Antares.app/Contents/MacOS/Antares

sign:
	codesign --force \
		--sign "Developer ID Application" \
		--entitlements resources/entitlements.plist \
		out/cur/Antares.app

.PHONY: all clean dist distclean run sign test
