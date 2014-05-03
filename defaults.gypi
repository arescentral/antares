{ "make_global_settings":
  [ ["CXX","/usr/bin/clang++"]
  , ["LINK","/usr/bin/clang++"]
  ]
, "target_defaults":
  { "variables":
    { "MACOSX_VERSION": "10.8"
    }
  , "default_configuration": "opt"
  , "configurations":
    { "dbg":
      { "cflags": ["-g"]
      , "cxxflags": ["-g"]
      }
    , "dev": { }
    , "opt":
      { "cflags": ["-Os"]
      , "cxxflags": ["-Os"]
      , "defines": ["NDEBUG"]
      }
    }
  , "cxxflags":
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
    }
  }
}
# -*- mode: python; tab-width: 2 -*-
