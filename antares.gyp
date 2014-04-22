{ "target_defaults":
  { "variables":
    { "ANTARES_VERSION": "0.7.2rc1"
    }
  , "cxxflags":
    [ "-Wall"
    , "-Werror"
    , "-Wno-sign-compare"
    , "-Wno-deprecated-declarations"
    ]
  , "xcode_settings":
    { "ANTARES_VERSION": "<(ANTARES_VERSION)"
    , "SYSTEM_VERSION": "<(MACOSX_VERSION)"
    }
  , "include_dirs": ["include"]
  }

, "targets":
  [ { "target_name": "libantares"
    , "type": "static_library"
    , "dependencies":
      [ "libantares-cocoa"
      , "libantares-config"
      , "libantares-data"
      , "libantares-drawing"
      , "libantares-game"
      , "libantares-math"
      , "libantares-sound"
      , "libantares-ui"
      , "libantares-video"
      ]
    , "export_dependent_settings":
      [ "libantares-cocoa"
      , "libantares-config"
      , "libantares-data"
      , "libantares-drawing"
      , "libantares-game"
      , "libantares-math"
      , "libantares-sound"
      , "libantares-ui"
      , "libantares-video"
      ]
    , "conditions":
      [ [ "OS != 'mac'"
        , { "dependencies!": ["libantares-cocoa"]
          , "export_dependent_settings!": ["libantares-cocoa"]
          }
        ]
      ]
    }

  , { "target_name": "libantares-config"
    , "type": "static_library"
    , "sources":
      [ "src/config/gamepad.cpp"
      , "src/config/keys.cpp"
      , "src/config/ledger.cpp"
      , "src/config/mac-dirs.cpp"
      , "src/config/preferences.cpp"
      ]
    , "dependencies": ["<(DEPTH)/ext/libsfz/libsfz.gyp:libsfz"]
    , "export_dependent_settings": ["<(DEPTH)/ext/libsfz/libsfz.gyp:libsfz"]
    , "conditions":
      [ [ "OS != 'mac'"
        , { "sources!": ["src/config/mac-dirs.cpp"]
          }
        ]
      ]
    }

  , { "target_name": "libantares-data"
    , "type": "static_library"
    , "sources":
      [ "src/data/extractor.cpp"
      , "src/data/interface.cpp"
      , "src/data/picture.cpp"
      , "src/data/races.cpp"
      , "src/data/replay-list.cpp"
      , "src/data/replay.cpp"
      , "src/data/resource.cpp"
      , "src/data/scenario-list.cpp"
      , "src/data/scenario.cpp"
      , "src/data/space-object.cpp"
      , "src/data/string-list.cpp"
      ]
    , "dependencies":
      [ "<(DEPTH)/ext/libpng-gyp/libpng.gyp:libpng"
      , "<(DEPTH)/ext/libsfz/libsfz.gyp:libsfz"
      , "<(DEPTH)/ext/libzipxx/libzipxx.gyp:libzipxx"
      , "<(DEPTH)/ext/rezin/rezin.gyp:librezin"
      ]
    , "export_dependent_settings":
      [ "<(DEPTH)/ext/libpng-gyp/libpng.gyp:libpng"
      , "<(DEPTH)/ext/libsfz/libsfz.gyp:libsfz"
      , "<(DEPTH)/ext/libzipxx/libzipxx.gyp:libzipxx"
      , "<(DEPTH)/ext/rezin/rezin.gyp:librezin"
      ]
    }

  , { "target_name": "libantares-drawing"
    , "type": "static_library"
    , "sources":
      [ "src/drawing/briefing.cpp"
      , "src/drawing/build-pix.cpp"
      , "src/drawing/color.cpp"
      , "src/drawing/interface.cpp"
      , "src/drawing/libpng-pix-map.cpp"
      , "src/drawing/pix-map.cpp"
      , "src/drawing/pix-table.cpp"
      , "src/drawing/shapes.cpp"
      , "src/drawing/sprite-handling.cpp"
      , "src/drawing/styled-text.cpp"
      , "src/drawing/text.cpp"
      ]
    , "dependencies":
      [ "<(DEPTH)/ext/libpng-gyp/libpng.gyp:libpng"
      , "<(DEPTH)/ext/libsfz/libsfz.gyp:libsfz"
      ]
    , "export_dependent_settings":
      [ "<(DEPTH)/ext/libpng-gyp/libpng.gyp:libpng"
      , "<(DEPTH)/ext/libsfz/libsfz.gyp:libsfz"
      ]
    }

  , { "target_name": "libantares-game"
    , "type": "static_library"
    , "sources":
      [ "src/game/admiral.cpp"
      , "src/game/beam.cpp"
      , "src/game/cheat.cpp"
      , "src/game/cursor.cpp"
      , "src/game/globals.cpp"
      , "src/game/input-source.cpp"
      , "src/game/instruments.cpp"
      , "src/game/labels.cpp"
      , "src/game/main.cpp"
      , "src/game/messages.cpp"
      , "src/game/minicomputer.cpp"
      , "src/game/motion.cpp"
      , "src/game/non-player-ship.cpp"
      , "src/game/player-ship.cpp"
      , "src/game/scenario-maker.cpp"
      , "src/game/space-object.cpp"
      , "src/game/starfield.cpp"
      , "src/game/time.cpp"
      ]
    , "dependencies": ["<(DEPTH)/ext/libsfz/libsfz.gyp:libsfz"]
    , "export_dependent_settings": ["<(DEPTH)/ext/libsfz/libsfz.gyp:libsfz"]
    }

  , { "target_name": "libantares-math"
    , "type": "static_library"
    , "sources":
      [ "src/math/fixed.cpp"
      , "src/math/geometry.cpp"
      , "src/math/random.cpp"
      , "src/math/rotation.cpp"
      , "src/math/special.cpp"
      ]
    , "dependencies": ["<(DEPTH)/ext/libsfz/libsfz.gyp:libsfz"]
    , "export_dependent_settings": ["<(DEPTH)/ext/libsfz/libsfz.gyp:libsfz"]
    }

  , { "target_name": "libantares-sound"
    , "type": "static_library"
    , "sources":
      [ "src/sound/driver.cpp"
      , "src/sound/fx.cpp"
      , "src/sound/music.cpp"
      , "src/sound/openal-driver.cpp"
      ]
    , "dependencies":
      [ "<(DEPTH)/ext/libmodplug-gyp/libmodplug.gyp:libmodplug"
      , "<(DEPTH)/ext/libsfz/libsfz.gyp:libsfz"
      ]
    , "export_dependent_settings":
      [ "<(DEPTH)/ext/libmodplug-gyp/libmodplug.gyp:libmodplug"
      , "<(DEPTH)/ext/libsfz/libsfz.gyp:libsfz"
      ]
    , "link_settings":
      { "libraries":
        [ "$(SDKROOT)/System/Library/Frameworks/AudioToolbox.framework"
        , "$(SDKROOT)/System/Library/Frameworks/OpenAL.framework"
        ]
      }
    , "conditions":
      [ [ "OS != 'mac'"
        , { "sources!": ["src/sound/openal-driver.cpp"]
          , "link_settings":
            { "libraries!":
              [ "$(SDKROOT)/System/Library/Frameworks/AudioToolbox.framework"
              , "$(SDKROOT)/System/Library/Frameworks/OpenAL.framework"
              ]
            }
          }
        ]
      ]
    }

  , { "target_name": "libantares-ui"
    , "type": "static_library"
    , "sources":
      [ "src/ui/card.cpp"
      , "src/ui/event-scheduler.cpp"
      , "src/ui/event-tracker.cpp"
      , "src/ui/event.cpp"
      , "src/ui/flows/master.cpp"
      , "src/ui/flows/replay-game.cpp"
      , "src/ui/flows/solo-game.cpp"
      , "src/ui/interface-handling.cpp"
      , "src/ui/screen.cpp"
      , "src/ui/screens/briefing.cpp"
      , "src/ui/screens/debriefing.cpp"
      , "src/ui/screens/help.cpp"
      , "src/ui/screens/loading.cpp"
      , "src/ui/screens/main.cpp"
      , "src/ui/screens/object-data.cpp"
      , "src/ui/screens/options.cpp"
      , "src/ui/screens/play-again.cpp"
      , "src/ui/screens/scroll-text.cpp"
      , "src/ui/screens/select-level.cpp"
      ]
    , "dependencies": ["<(DEPTH)/ext/libsfz/libsfz.gyp:libsfz"]
    , "export_dependent_settings": ["<(DEPTH)/ext/libsfz/libsfz.gyp:libsfz"]
    }

  , { "target_name": "libantares-video"
    , "type": "static_library"
    , "sources":
      [ "src/video/driver.cpp"
      , "src/video/opengl-driver.cpp"
      , "src/video/transitions.cpp"
      ]
    , "dependencies":
      [ "<(DEPTH)/ext/libpng-gyp/libpng.gyp:libpng"
      , "<(DEPTH)/ext/libsfz/libsfz.gyp:libsfz"
      ]
    , "export_dependent_settings":
      [ "<(DEPTH)/ext/libpng-gyp/libpng.gyp:libpng"
      , "<(DEPTH)/ext/libsfz/libsfz.gyp:libsfz"
      ]
    , "link_settings":
      { "libraries": ["$(SDKROOT)/System/Library/Frameworks/OpenGL.framework"]
      }
    , "conditions":
      [ [ "OS != 'mac'"
        , { "sources!": ["src/video/opengl-driver.cpp"]
          , "link_settings":
            { "libraries!": ["$(SDKROOT)/System/Library/Frameworks/OpenGL.framework"]
            }
          }
        ]
      ]
    }

  , { "target_name": "offscreen"
    , "type": "executable"
    , "sources": ["src/bin/offscreen.cpp"]
    , "dependencies": ["libantares-test"]
    }

  , { "target_name": "build-pix"
    , "type": "executable"
    , "sources": ["src/bin/build-pix.cpp"]
    , "dependencies": ["libantares-test"]
    }

  , { "target_name": "extract-data"
    , "type": "executable"
    , "sources": ["src/bin/extract-data.cpp"]
    , "dependencies": ["libantares-test"]
    }

  , { "target_name": "hash-data"
    , "type": "executable"
    , "sources": ["src/bin/hash-data.cpp"]
    , "dependencies": ["libantares-test"]
    }

  , { "target_name": "ls-scenarios"
    , "type": "executable"
    , "sources": ["src/bin/ls-scenarios.cpp"]
    , "dependencies": ["libantares-test"]
    }

  , { "target_name": "object-data"
    , "type": "executable"
    , "sources": ["src/bin/object-data.cpp"]
    , "dependencies": ["libantares-test"]
    }

  , { "target_name": "replay"
    , "type": "executable"
    , "sources": ["src/bin/replay.cpp"]
    , "dependencies": ["libantares-test"]
    }

  , { "target_name": "shapes"
    , "type": "executable"
    , "sources": ["src/bin/shapes.cpp"]
    , "dependencies": ["libantares-test"]
    }

  , { "target_name": "tint"
    , "type": "executable"
    , "sources": ["src/bin/tint.cpp"]
    , "dependencies": ["libantares-test"]
    }

  , { "target_name": "libantares-test"
    , "type": "static_library"
    , "sources":
      [ "src/test/resource.cpp"
      , "src/video/offscreen-driver.cpp"
      , "src/video/text-driver.cpp"
      ]
    , "defines": ["ANTARES_DATA=<(DEPTH)/data"]
    , "dependencies": ["libantares"]
    , "export_dependent_settings": ["libantares"]
    }

  , { "target_name": "fixed-test"
    , "type": "executable"
    , "sources": ["src/math/fixed.test.cpp"]
    , "dependencies":
      [ "libantares-test"
      , "<(DEPTH)/ext/gmock-gyp/gmock.gyp:gmock_main"
      ]
    }
  ]

, "conditions":
  [ [ "OS == 'mac'"
    , { "targets":
        [ { "target_name": "libantares-cocoa"
          , "type": "static_library"
          , "sources":
            [ "src/cocoa/core-foundation.cpp"
            , "src/cocoa/core-opengl.cpp"
            , "src/cocoa/fullscreen.cpp"
            , "src/cocoa/http.cpp"
            , "src/cocoa/windowed.cpp"
            ]
          , "dependencies":
            [ "<(DEPTH)/ext/libpng-gyp/libpng.gyp:libpng"
            , "<(DEPTH)/ext/libsfz/libsfz.gyp:libsfz"
            ]
          , "export_dependent_settings":
            [ "<(DEPTH)/ext/libpng-gyp/libpng.gyp:libpng"
            , "<(DEPTH)/ext/libsfz/libsfz.gyp:libsfz"
            ]
          , "link_settings":
            { "libraries":
              [ "$(SDKROOT)/System/Library/Frameworks/CoreFoundation.framework"
              , "$(SDKROOT)/System/Library/Frameworks/IOKit.framework"
              , "$(SDKROOT)/System/Library/Frameworks/OpenGL.framework"
              ]
            }
          }

        , { "target_name": "antares"
          , "product_name": "Antares"
          , "type": "executable"
          , "mac_bundle": 1
          , "xcode_settings":
            { "INFOPLIST_FILE": "resources/Antares-Info.plist"
            , "CODE_SIGN_IDENTITY": "Developer ID Application"
            , "CODE_SIGN_ENTITLEMENTS": "resources/entitlements.plist"
            }
          , "mac_bundle_resources":
            [ "data/fonts"
            , "data/interfaces"
            , "data/music"
            , "data/pictures"
            , "data/rotation-table"
            , "data/strings"
            , "data/text"
            , "resources/Antares.icns"
            , "resources/ExtractData.nib"
            , "resources/MainMenu.nib"
            , "resources/container-migration.plist"
            ]
          , "sources":
            [ "src/cocoa/AntaresController.m"
            , "src/cocoa/AntaresExtractDataController.m"
            , "src/cocoa/main.m"
            , "src/cocoa/prefs-driver.cpp"
            , "src/cocoa/resource.cpp"
            , "src/cocoa/video-driver.cpp"
            , "src/cocoa/c/AntaresController.cpp"
            , "src/cocoa/c/CocoaVideoDriver.m"
            , "src/cocoa/c/DataExtractor.cpp"
            , "src/cocoa/c/scenario-list.cpp"
            ]
          , "dependencies": ["libantares"]
          , "link_settings":
            { "libraries":
              [ "$(SDKROOT)/System/Library/Frameworks/Carbon.framework"
              , "$(SDKROOT)/System/Library/Frameworks/Cocoa.framework"
              ]
            }
          }
        ]
      }
    ]
  ]
}
# -*- mode: python; tab-width: 2 -*-
