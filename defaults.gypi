{ "make_global_settings":
  [ ["CXX","/usr/bin/clang++"]
  , ["LINK","/usr/bin/clang++"]
  ]
, "target_defaults":
  { "variables":
    { "MACOSX_VERSION": "10.11"
    , "MACOSX_TARGET": "10.7"
    }
  , "default_configuration": "opt"
  , "configurations":
    { "dbg":
      { "cflags": ["-g", "-O0"]
      , "xcode_settings":
        { "GCC_OPTIMIZATION_LEVEL": "0"
        , "OTHER_CFLAGS": ["-g"]
        }
      }
    , "dev":
      { "cflags": ["-O0"]
      , "xcode_settings":
        { "GCC_OPTIMIZATION_LEVEL": "0"
        }
      }
    , "opt":
      { "cflags": ["-Os"]
      , "defines": ["NDEBUG"]
      , "xcode_settings":
        { "GCC_OPTIMIZATION_LEVEL": "s"
        }
      }
    }
  , "cflags_cc":
    [ "-std=c++11"
    , "-stdlib=libc++"
    ]
  , "ldflags":
    [ "-std=c++11"
    , "-stdlib=libc++"
    ]
  , "xcode_settings":
    { "ARCHS": ["i386", "x86_64"]
    , "CC": "clang"
    , "CXX": "clang++"
    , "CLANG_CXX_LANGUAGE_STANDARD": "c++11"
    , "CLANG_CXX_LIBRARY": "libc++"
    , "SDKROOT": "macosx<(MACOSX_VERSION)"
    , "MACOSX_DEPLOYMENT_TARGET": "<(MACOSX_TARGET)"
    }
  , "conditions":
    [ [ "COVERAGE != ''"
      , { "defines": ["DATA_COVERAGE=<(COVERAGE)"]
        }
      ]
    ]
  }
}
# -*- mode: python; tab-width: 2 -*-
