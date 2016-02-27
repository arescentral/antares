{ "target_defaults":
  { "variables":
    { "ANTARES_VERSION": "0.8.0"
    }
  , "cxxflags":
    [ "-Wall"
    , "-Werror"
    , "-Wno-sign-compare"
    , "-Wno-deprecated-declarations"
    ]
  , "xcode_settings":
    { "ANTARES_VERSION": "0.8.0"
    , "SYSTEM_VERSION": "<(MACOSX_TARGET)"
    }
  , "include_dirs": ["include", "<(INTERMEDIATE_DIR)/include"]
  }

, "targets":
  [ { "target_name": "libantares"
    , "type": "static_library"
    , "sources":
      [ "include/lang/casts.hpp"
      , "include/lang/defines.hpp"
      , "include/net/http.hpp"
      ]
    , "dependencies":
      [ "libantares-config"
      , "libantares-data"
      , "libantares-drawing"
      , "libantares-game"
      , "libantares-linux"
      , "libantares-mac"
      , "libantares-math"
      , "libantares-sound"
      , "libantares-ui"
      , "libantares-video"
      ]
    , "export_dependent_settings":
      [ "libantares-config"
      , "libantares-data"
      , "libantares-drawing"
      , "libantares-game"
      , "libantares-linux"
      , "libantares-mac"
      , "libantares-math"
      , "libantares-sound"
      , "libantares-ui"
      , "libantares-video"
      ]
    , "conditions":
      [ [ "OS != 'mac'"
        , { "dependencies!": ["libantares-mac"]
          , "export_dependent_settings!": ["libantares-mac"]
          }
        ]
      , [ "OS != 'linux'"
        , { "dependencies!": ["libantares-linux"]
          , "export_dependent_settings!": ["libantares-linux"]
          }
        ]
      ]
    }

  , { "target_name": "libantares-config"
    , "type": "static_library"
    , "sources":
      [ "include/config/dirs.hpp"
      , "include/config/file-prefs-driver.hpp"
      , "include/config/gamepad.hpp"
      , "include/config/keys.hpp"
      , "include/config/ledger.hpp"
      , "include/config/preferences.hpp"
      , "src/config/file-prefs-driver.cpp"
      , "src/config/gamepad.cpp"
      , "src/config/keys.cpp"
      , "src/config/ledger.cpp"
      , "src/config/linux-dirs.cpp"
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
      , [ "OS != 'linux'"
        , { "sources!": ["src/config/linux-dirs.cpp"]
          }
        ]
      ]
    }

  , { "target_name": "libantares-data"
    , "type": "static_library"
    , "sources":
      [ "include/data/action.hpp"
      , "include/data/extractor.hpp"
      , "include/data/handle.hpp"
      , "include/data/interface.hpp"
      , "include/data/picture.hpp"
      , "include/data/races.hpp"
      , "include/data/replay-list.hpp"
      , "include/data/replay.hpp"
      , "include/data/resource.hpp"
      , "include/data/scenario-list.hpp"
      , "include/data/scenario.hpp"
      , "include/data/space-object.hpp"
      , "include/data/string-list.hpp"
      , "src/data/action.cpp"
      , "src/data/extractor.cpp"
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
      [ "include/drawing/briefing.hpp"
      , "include/drawing/build-pix.hpp"
      , "include/drawing/color.hpp"
      , "include/drawing/interface.hpp"
      , "include/drawing/pix-map.hpp"
      , "include/drawing/pix-table.hpp"
      , "include/drawing/shapes.hpp"
      , "include/drawing/sprite-handling.hpp"
      , "include/drawing/styled-text.hpp"
      , "include/drawing/text.hpp"
      , "src/drawing/briefing.cpp"
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
      [ "include/game/action.hpp"
      , "include/game/admiral.hpp"
      , "include/game/beam.hpp"
      , "include/game/cheat.hpp"
      , "include/game/cursor.hpp"
      , "include/game/globals.hpp"
      , "include/game/input-source.hpp"
      , "include/game/instruments.hpp"
      , "include/game/labels.hpp"
      , "include/game/main.hpp"
      , "include/game/messages.hpp"
      , "include/game/minicomputer.hpp"
      , "include/game/motion.hpp"
      , "include/game/non-player-ship.hpp"
      , "include/game/player-ship.hpp"
      , "include/game/scenario-maker.hpp"
      , "include/game/space-object.hpp"
      , "include/game/starfield.hpp"
      , "include/game/time.hpp"
      , "src/game/action.cpp"
      , "src/game/admiral.cpp"
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
      [ "include/math/fixed.hpp"
      , "include/math/geometry.hpp"
      , "include/math/macros.hpp"
      , "include/math/random.hpp"
      , "include/math/rotation.hpp"
      , "include/math/special.hpp"
      , "include/math/units.hpp"
      , "src/math/fixed.cpp"
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
      [ "include/sound/driver.hpp"
      , "include/sound/fx.hpp"
      , "include/sound/music.hpp"
      , "include/sound/openal-driver.hpp"
      , "include/sound/sndfile.hpp"
      , "src/sound/driver.cpp"
      , "src/sound/fx.cpp"
      , "src/sound/music.cpp"
      , "src/sound/openal-driver.cpp"
      , "src/sound/sndfile.cpp"
      ]
    , "dependencies":
      [ "<(DEPTH)/ext/libmodplug-gyp/libmodplug.gyp:libmodplug"
      , "<(DEPTH)/ext/libsfz/libsfz.gyp:libsfz"
      , "<(DEPTH)/ext/libsndfile-gyp/libsndfile.gyp:libsndfile"
      ]
    , "export_dependent_settings":
      [ "<(DEPTH)/ext/libmodplug-gyp/libmodplug.gyp:libmodplug"
      , "<(DEPTH)/ext/libsfz/libsfz.gyp:libsfz"
      , "<(DEPTH)/ext/libsndfile-gyp/libsndfile.gyp:libsndfile"
      ]
    , "link_settings":
      { "libraries":
        [ "$(SDKROOT)/System/Library/Frameworks/OpenAL.framework"
        , "-lopenal"
        ]
      }
    , "conditions":
      [ [ "OS != 'mac'"
        , { "link_settings":
            { "libraries!":
              [ "$(SDKROOT)/System/Library/Frameworks/OpenAL.framework"
              ]
            }
          }
        ]
      , [ "OS != 'linux'"
        , { "link_settings":
            { "libraries!":
              [ "-lopenal"
              ]
            }
          }
        ]
      ]
    }

  , { "target_name": "libantares-ui"
    , "type": "static_library"
    , "sources":
      [ "include/ui/card.hpp"
      , "include/ui/event-scheduler.hpp"
      , "include/ui/event.hpp"
      , "include/ui/flows/master.hpp"
      , "include/ui/flows/replay-game.hpp"
      , "include/ui/flows/solo-game.hpp"
      , "include/ui/interface-handling.hpp"
      , "include/ui/screen.hpp"
      , "include/ui/screens/briefing.hpp"
      , "include/ui/screens/debriefing.hpp"
      , "include/ui/screens/help.hpp"
      , "include/ui/screens/loading.hpp"
      , "include/ui/screens/main.hpp"
      , "include/ui/screens/object-data.hpp"
      , "include/ui/screens/options.hpp"
      , "include/ui/screens/play-again.hpp"
      , "include/ui/screens/scroll-text.hpp"
      , "include/ui/screens/select-level.hpp"
      , "src/ui/card.cpp"
      , "src/ui/event-scheduler.cpp"
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
      [ "include/video/driver.hpp"
      , "include/video/opengl-driver.hpp"
      , "include/video/transitions.hpp"
      , "src/video/driver.cpp"
      , "src/video/opengl-driver.cpp"
      , "src/video/transitions.cpp"
      , "<(INTERMEDIATE_DIR)/src/video/glsl/fragment.cpp"
      , "<(INTERMEDIATE_DIR)/src/video/glsl/vertex.cpp"
      ]
    , "actions":
      [ { "action_name": "fragment"
        , "message": "Embedding GLSL fragment shader"
        , "inputs":
          [ "scripts/embed.py"
          , "src/video/glsl/fragment.frag"
          ]
        , "outputs":
          [ "<(INTERMEDIATE_DIR)/include/video/glsl/fragment.hpp"
          , "<(INTERMEDIATE_DIR)/src/video/glsl/fragment.cpp"
          ]
        , "action":
          [ "python"
          , "scripts/embed.py"
          , "src/video/glsl/fragment.frag"
          , "<@(_outputs)"
          , "antares::glsl::fragment"
          ]
        }
      , { "action_name": "vertex"
        , "message": "Embedding GLSL vertex shader"
        , "inputs":
          [ "scripts/embed.py"
          , "src/video/glsl/vertex.vert"
          ]
        , "outputs":
          [ "<(INTERMEDIATE_DIR)/include/video/glsl/vertex.hpp"
          , "<(INTERMEDIATE_DIR)/src/video/glsl/vertex.cpp"
          ]
        , "action":
          [ "python"
          , "scripts/embed.py"
          , "src/video/glsl/vertex.vert"
          , "<@(_outputs)"
          , "antares::glsl::vertex"
          ]
        }
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
        [ "$(SDKROOT)/System/Library/Frameworks/OpenGL.framework"
        , "-lGL"
        , "-lX11"
        ]
      }
    , "conditions":
      [ [ "OS != 'mac'"
        , { "link_settings":
            { "libraries!": ["$(SDKROOT)/System/Library/Frameworks/OpenGL.framework"]
            }
          }
        ]
      , [ "OS != 'linux'"
        , { "link_settings":
            { "libraries!": ["-lGL", "-lX11"]
            }
          }
        ]
      ]
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
      [ "include/linux/offscreen.hpp"
      , "include/mac/offscreen.hpp"
      , "include/video/offscreen-driver.hpp"
      , "include/video/text-driver.hpp"
      , "src/linux/offscreen.cpp"
      , "src/mac/offscreen.cpp"
      , "src/test/resource.cpp"
      , "src/video/offscreen-driver.cpp"
      , "src/video/text-driver.cpp"
      ]
    , "defines": ["ANTARES_DATA=<(DEPTH)/data"]
    , "dependencies": ["libantares"]
    , "export_dependent_settings": ["libantares"]
    , "conditions":
      [ [ "OS != 'mac'" , { "sources!": ["src/mac/offscreen.cpp"] } ]
      , [ "OS != 'linux'" , { "sources!": ["src/linux/offscreen.cpp"] } ]
      ]
    }

  , { "target_name": "fixed-test"
    , "type": "executable"
    , "sources": ["src/math/fixed.test.cpp"]
    , "dependencies":
      [ "libantares-test"
      , "<(DEPTH)/ext/gmock-gyp/gmock.gyp:gmock_main"
      ]
    }

  , { "target_name": "offscreen"
    , "type": "executable"
    , "sources": ["src/bin/offscreen.cpp"]
    , "dependencies": ["libantares-test"]
    }

  , { "target_name": "replay"
    , "type": "executable"
    , "sources": ["src/bin/replay.cpp"]
    , "dependencies": ["libantares-test"]
    }

  , { "target_name": "build-pix"
    , "type": "executable"
    , "sources": ["src/bin/build-pix.cpp"]
    , "dependencies": ["libantares-test"]
    }

  , { "target_name": "<(ANTARES_GLFW)"
    , "type": "executable"
    , "sources":
      [ "include/glfw/video-driver.hpp"
      , "src/glfw/main.cpp"
      , "src/glfw/video-driver.cpp"
      ]
    , "dependencies":
      [ "libantares"
      , "<(DEPTH)/ext/glfw/glfw.gyp:libglfw"
      ]
    }

  , { "target_name": "antares-install-data"
    , "type": "executable"
    , "sources": ["src/bin/antares-install-data.cpp"]
    , "dependencies": ["libantares"]
    }
  ]

, "conditions":
  [ [ "OS == 'mac'"
    , { "variables": { "ANTARES_GLFW": "antares-glfw" }
      , "targets":
        [ { "target_name": "libantares-mac"
          , "type": "static_library"
          , "sources":
            [ "include/mac/audio-file.hpp"
            , "include/mac/core-foundation.hpp"
            , "include/mac/core-opengl.hpp"
            , "src/mac/audio-file.cpp"
            , "src/mac/core-foundation.cpp"
            , "src/mac/core-opengl.cpp"
            , "src/mac/http.cpp"
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
              [ "$(SDKROOT)/System/Library/Frameworks/AudioToolbox.framework"
              , "$(SDKROOT)/System/Library/Frameworks/CoreFoundation.framework"
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
            [ "include/mac/AntaresController.h"
            , "include/mac/AntaresExtractDataController.h"
            , "include/mac/prefs-driver.hpp"
            , "include/mac/video-driver.hpp"
            , "include/mac/c/AntaresController.hpp"
            , "include/mac/c/CocoaVideoDriver.hpp"
            , "include/mac/c/DataExtractor.h"
            , "include/mac/c/scenario-list.h"
            , "src/mac/AntaresController.m"
            , "src/mac/AntaresExtractDataController.m"
            , "src/mac/main.m"
            , "src/mac/prefs-driver.cpp"
            , "src/mac/resource.cpp"
            , "src/mac/video-driver.cpp"
            , "src/mac/c/AntaresController.cpp"
            , "src/mac/c/CocoaVideoDriver.m"
            , "src/mac/c/DataExtractor.cpp"
            , "src/mac/c/scenario-list.cpp"
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

  , [ "OS == 'linux'"
    , { "variables": { "ANTARES_GLFW": "antares" }
      , "targets":
        [ { "target_name": "libantares-linux"
          , "type": "static_library"
          , "sources":
            [ "src/linux/http.cpp"
            ]
          , "dependencies":
            [ "<(DEPTH)/ext/libsfz/libsfz.gyp:libsfz"
            ]
          , "export_dependent_settings":
            [ "<(DEPTH)/ext/libsfz/libsfz.gyp:libsfz"
            ]
          , "link_settings":
            { "libraries":
              [ "-lneon"
              ]
            }
          }
        ]
      }
    ]
  ]
}
# -*- mode: python; tab-width: 2 -*-
