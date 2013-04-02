# -*- mode: python -*-

APPNAME = "Antares"
VERSION = "0.6.1"

WARNINGS = ["-Wall", "-Werror", "-Wno-sign-compare", "-Wno-deprecated-declarations"]

def common(ctx):
    ctx.default_sdk = "10.7"
    ctx.default_compiler = "clang"
    ctx.cxx_std = "c++11"
    ctx.load("compiler_c compiler_cxx")
    ctx.load("core externals", tooldir="ext/waf-sfiera")
    ctx.load("antares_test", tooldir="tools")
    ctx.external("libmodplug-waf libpng-waf libsfz libzipxx rezin")

def dist(dst):
    dst.algo = "zip"
    dst.excl = (
        " test **/.* **/*.zip **/*.pyc **/build ext/*/ext"
        " ext/libsfz/waf ext/libzipxx/waf ext/rezin/waf"
    )

def options(opt):
    common(opt)
    opt.add_option(
            "--smoke", action="store_true", default=False,
            help="run tests quickly")

def configure(cnf):
    common(cnf)

    cnf.env.append_value("FRAMEWORK_antares/system/audio-toolbox", "AudioToolbox")
    cnf.env.append_value("FRAMEWORK_antares/system/cocoa", "Cocoa")
    cnf.env.append_value("FRAMEWORK_antares/system/carbon", "Carbon")
    cnf.env.append_value("FRAMEWORK_antares/system/core-foundation", "CoreFoundation")
    cnf.env.append_value("FRAMEWORK_antares/system/openal", "OpenAL")
    cnf.env.append_value("FRAMEWORK_antares/system/opengl", "OpenGL")

def build(bld):
    common(bld)

    bld(
        rule="${SRC} %s %s ${TGT}" % (VERSION, bld.options.sdk),
        source="scripts/generate-info-plist.py",
        target="antares/Info.plist",
    )

    bld.program(
        target="antares/Antares",
        features="universal",
        mac_app=True,
        mac_plist="antares/Info.plist",
        mac_resources=[
            "resources/Antares.icns",
            "resources/ExtractData.nib",
            "resources/MainMenu.nib",
            "data/fonts",
            "data/interfaces",
            "data/pictures",
            "data/rotation-table",
            "data/strings",
            "data/text",
        ],
        source=[
            "src/cocoa/AntaresController.m",
            "src/cocoa/AntaresExtractDataController.m",
            "src/cocoa/main.m",
            "src/cocoa/video-driver.cpp",
            "src/cocoa/prefs-driver.cpp",
            "src/cocoa/resource.cpp",
            "src/cocoa/c/AntaresController.cpp",
            "src/cocoa/c/CocoaVideoDriver.m",
            "src/cocoa/c/DataExtractor.cpp",
            "src/cocoa/c/scenario-list.cpp",
        ],
        cflags=WARNINGS,
        cxxflags=WARNINGS,
        use=[
            "antares/libantares",
            "antares/system/cocoa",
            "antares/system/carbon",
        ],
    )

    bld.program(
        target="antares/offscreen",
        features="universal",
        source="src/bin/offscreen.cpp",
        cxxflags=WARNINGS,
        use="antares/libantares-test",
    )

    bld.program(
        target="antares/replay",
        features="universal",
        source="src/bin/replay.cpp",
        cxxflags=WARNINGS,
        use="antares/libantares-test",
    )

    bld.program(
        target="antares/build-pix",
        features="universal",
        source="src/bin/build-pix.cpp",
        cxxflags=WARNINGS,
        use="antares/libantares-test",
    )

    bld.program(
        target="antares/shapes",
        features="universal",
        source="src/bin/shapes.cpp",
        cxxflags=WARNINGS,
        use="antares/libantares-test",
    )

    bld.program(
        target="antares/tint",
        features="universal",
        source="src/bin/tint.cpp",
        cxxflags=WARNINGS,
        use="antares/libantares-test",
    )

    bld.program(
        target="antares/ls-scenarios",
        features="universal",
        source="src/bin/ls-scenarios.cpp",
        cxxflags=WARNINGS,
        use="antares/libantares-test",
    )

    bld.program(
        target="antares/object-data",
        features="universal",
        source="src/bin/object-data.cpp",
        cxxflags=WARNINGS,
        use="antares/libantares-test",
    )

    bld.program(
        target="antares/extract-data",
        features="universal",
        source="src/bin/extract-data.cpp",
        cxxflags=WARNINGS,
        use="antares/libantares-test",
    )

    bld.stlib(
        target="antares/libantares",
        features="cxx universal",
        source=[],
        use=[
            "antares/libantares-config",
            "antares/libantares-data",
            "antares/libantares-drawing",
            "antares/libantares-game",
            "antares/libantares-math",
            "antares/libantares-sound",
            "antares/libantares-ui",
            "antares/libantares-video",
        ],
    )

    bld.platform(
        target="antares/libantares",
        source=[
            "src/cocoa/core-foundation.cpp",
            "src/cocoa/http.cpp",
        ],
        platform="darwin",
        use="antares/system/core-foundation",
    )

    bld.stlib(
        target="antares/libantares-test",
        features="universal",
        source=[
            "src/video/offscreen-driver.cpp",
            "src/video/text-driver.cpp",
            "src/test/resource.cpp",
        ],
        defines="ANTARES_DATA=%s" % bld.path.find_dir("data").abspath(),
        cxxflags=WARNINGS,
        includes="./include",
        export_includes="./include",
        use=[
            "antares/libantares",
            "antares/system/opengl",
            "antares/opengl",
        ],
    )

    bld.stlib(
        target="antares/libantares-config",
        features="universal",
        source=[
            "src/config/keys.cpp",
            "src/config/ledger.cpp",
            "src/config/preferences.cpp",
        ],
        cxxflags=WARNINGS,
        includes="./include",
        export_includes="./include",
        use="libsfz/libsfz",
    )

    bld.stlib(
        target="antares/libantares-data",
        features="universal",
        source=[
            "src/data/extractor.cpp",
            "src/data/interface.cpp",
            "src/data/picture.cpp",
            "src/data/races.cpp",
            "src/data/replay.cpp",
            "src/data/replay-list.cpp",
            "src/data/resource.cpp",
            "src/data/scenario.cpp",
            "src/data/scenario-list.cpp",
            "src/data/space-object.cpp",
            "src/data/string-list.cpp",
        ],
        cxxflags=WARNINGS,
        includes="./include",
        export_includes="./include",
        use=[
            "libpng/libpng",
            "libsfz/libsfz",
            "rezin/librezin",
            "zipxx/libzipxx",
        ],
    )

    bld.stlib(
        target="antares/libantares-drawing",
        features="universal",
        source=[
            "src/drawing/briefing.cpp",
            "src/drawing/build-pix.cpp",
            "src/drawing/color.cpp",
            "src/drawing/interface.cpp",
            "src/drawing/libpng-pix-map.cpp",
            "src/drawing/pix-map.cpp",
            "src/drawing/pix-table.cpp",
            "src/drawing/shapes.cpp",
            "src/drawing/sprite-handling.cpp",
            "src/drawing/styled-text.cpp",
            "src/drawing/text.cpp",
        ],
        cxxflags=WARNINGS,
        includes="./include",
        export_includes="./include",
        use=[
            "libpng/libpng",
            "libsfz/libsfz",
        ],
    )

    bld.stlib(
        target="antares/libantares-game",
        features="universal",
        source=[
            "src/game/admiral.cpp",
            "src/game/beam.cpp",
            "src/game/cheat.cpp",
            "src/game/cursor.cpp",
            "src/game/globals.cpp",
            "src/game/input-source.cpp",
            "src/game/instruments.cpp",
            "src/game/labels.cpp",
            "src/game/main.cpp",
            "src/game/messages.cpp",
            "src/game/minicomputer.cpp",
            "src/game/motion.cpp",
            "src/game/non-player-ship.cpp",
            "src/game/player-ship.cpp",
            "src/game/scenario-maker.cpp",
            "src/game/space-object.cpp",
            "src/game/starfield.cpp",
            "src/game/time.cpp",
        ],
        cxxflags=WARNINGS,
        includes="./include",
        export_includes="./include",
        use="libsfz/libsfz",
    )

    bld.stlib(
        target="antares/libantares-math",
        features="universal",
        source=[
            "src/math/fixed.cpp",
            "src/math/geometry.cpp",
            "src/math/random.cpp",
            "src/math/rotation.cpp",
            "src/math/special.cpp",
        ],
        cxxflags=WARNINGS,
        includes="./include",
        export_includes="./include",
        use="libsfz/libsfz",
    )

    bld.stlib(
        target="antares/libantares-sound",
        features="universal",
        source=[
            "src/sound/driver.cpp",
            "src/sound/fx.cpp",
            "src/sound/music.cpp",
        ],
        cxxflags=WARNINGS,
        includes="./include",
        export_includes="./include",
        use=[
            "libmodplug/libmodplug",
            "libsfz/libsfz",
        ],
    )

    bld.platform(
        target="antares/libantares-sound",
        platform="darwin",
        source="src/sound/openal-driver.cpp",
        use=[
            "antares/system/audio-toolbox",
            "antares/system/openal",
        ],
    )

    bld.stlib(
        target="antares/libantares-ui",
        features="universal",
        source=[
            "src/ui/card.cpp",
            "src/ui/event.cpp",
            "src/ui/event-scheduler.cpp",
            "src/ui/event-tracker.cpp",
            "src/ui/flows/master.cpp",
            "src/ui/flows/replay-game.cpp",
            "src/ui/flows/solo-game.cpp",
            "src/ui/interface-handling.cpp",
            "src/ui/screen.cpp",
            "src/ui/screens/briefing.cpp",
            "src/ui/screens/debriefing.cpp",
            "src/ui/screens/help.cpp",
            "src/ui/screens/loading.cpp",
            "src/ui/screens/main.cpp",
            "src/ui/screens/object-data.cpp",
            "src/ui/screens/options.cpp",
            "src/ui/screens/play-again.cpp",
            "src/ui/screens/scroll-text.cpp",
            "src/ui/screens/select-level.cpp",
        ],
        cxxflags=WARNINGS,
        includes="./include",
        export_includes="./include",
        use="libsfz/libsfz",
    )

    bld.stlib(
        target="antares/libantares-video",
        features="universal",
        source=[
            "src/video/driver.cpp",
            "src/video/transitions.cpp",
        ],
        cxxflags=WARNINGS,
        includes="./include",
        export_includes="./include",
        use=[
            "libpng/libpng",
            "libsfz/libsfz",
        ],
    )

    bld.platform(
        target="antares/libantares-video",
        platform="darwin",
        source=[
            "src/cocoa/core-opengl.cpp",
            "src/cocoa/fullscreen.cpp",
            "src/video/opengl-driver.cpp",
            "src/cocoa/windowed.cpp",
        ],
        use="antares/system/opengl",
    )

    def unit_test(name):
        bld.test(
            target="antares/%s" % name,
            features="universal",
            source="src/%s.test.cpp" % name,
            cxxflags=WARNINGS,
            defines="GTEST_USE_OWN_TR1_TUPLE=1",
            use=[
                "antares/libantares",
                "gmock/gmock-main",
            ],
        )

    def data_test(name):
        bld.antares_test(
            target="antares/%s" % name,
            rule="antares/%s" % name,
            expected="test/%s" % name,
        )

    def regression_test(name):
        if bld.options.smoke:
            bld.antares_test(
                target="antares/%s" % name.split()[0],
                rule="antares/offscreen %s --text" % name,
                expected="test/smoke/%s" % name.split()[0],
            )
        else:
            bld.antares_test(
                target="antares/%s" % name.split()[0],
                rule="antares/offscreen %s" % name,
                expected="test/%s" % name.split()[0],
            )

    def replay_test(name):
        if bld.options.smoke:
            bld.antares_test(
                target="antares/replay/%s" % name,
                rule="antares/replay --text --smoke",
                srcs="test/%s.NLRP" % name,
                expected="test/smoke/%s" % name,
            )
        else:
            bld.antares_test(
                target="antares/replay/%s" % name,
                rule="antares/replay --text",
                srcs="test/%s.NLRP" % name,
                expected="test/%s" % name,
            )

    unit_test("math/fixed")

    data_test("build-pix")
    data_test("object-data")
    data_test("shapes")
    data_test("tint")

    regression_test("main-screen")
    regression_test("mission-briefing --text")
    regression_test("options")
    regression_test("pause --text")

    replay_test("and-it-feels-so-good")
    replay_test("blood-toil-tears-sweat")
    replay_test("hand-over-fist")
    replay_test("make-way")
    replay_test("out-of-the-frying-pan")
    replay_test("space-race")
    replay_test("the-left-hand")
    replay_test("the-mothership-connection")
    replay_test("the-stars-have-ears")
    replay_test("while-the-iron-is-hot")
    replay_test("yo-ho-ho")
    replay_test("you-should-have-seen-the-one-that-got-away")
