# Copyright (C) 2020 The Antares Authors
# This file is part of Antares, a tactical space combat game.
# Antares is free software, distributed under the LGPL+. See COPYING.

LIBANTARES_CPPFLAGS := \
	$(LIBPROCYON_CPPFLAGS) \
	$(LIBSFZ_CPPFLAGS) \
	$(LIBGLFW3_CPPFLAGS) \
	$(LIBPNG_CPPFLAGS) \
	$(LIBSNDFILE_CPPFLAGS) \
	$(LIBMODPLUG_CPPFLAGS) \
	$(LIBZIPXX_CPPFLAGS) \
	-I $(ANTARES_ROOT)/include
LIBANTARES_LDFLAGS := \
	$(LIBPROCYON_LDFLAGS) \
	$(LIBSFZ_LDFLAGS) \
	$(LIBGLFW3_LDFLAGS) \
	$(LIBPNG_LDFLAGS) \
	$(LIBSNDFILE_LDFLAGS) \
	$(LIBMODPLUG_LDFLAGS) \
	$(LIBZIPXX_LDFLAGS)

ifeq ($(TARGET_OS),mac)
LIBANTARES_LDFLAGS += \
	-framework OpenAL \
	-framework OpenGL
endif
ifeq ($(TARGET_OS),linux)
LIBANTARES_LDFLAGS += \
	-l openal \
	-l GL \
	-l X11
endif

LIBANTARES_A := $(OUT)/libantares.a
LIBANTARES_CXX_SRCS := \
	$(ANTARES_ROOT)/src/build/defs.cpp \
	$(ANTARES_ROOT)/src/config/dirs.cpp \
	$(ANTARES_ROOT)/src/config/file-prefs-driver.cpp \
	$(ANTARES_ROOT)/src/config/gamepad.cpp \
	$(ANTARES_ROOT)/src/config/keys.cpp \
	$(ANTARES_ROOT)/src/config/ledger.cpp \
	$(ANTARES_ROOT)/src/config/preferences.cpp \
	$(ANTARES_ROOT)/src/data/action.cpp \
	$(ANTARES_ROOT)/src/data/audio.cpp \
	$(ANTARES_ROOT)/src/data/base-object.cpp \
	$(ANTARES_ROOT)/src/data/briefing.cpp \
	$(ANTARES_ROOT)/src/data/cash.cpp \
	$(ANTARES_ROOT)/src/data/condition.cpp \
	$(ANTARES_ROOT)/src/data/counter.cpp \
	$(ANTARES_ROOT)/src/data/distance.cpp \
	$(ANTARES_ROOT)/src/data/extractor.cpp \
	$(ANTARES_ROOT)/src/data/field.cpp \
	$(ANTARES_ROOT)/src/data/font-data.cpp \
	$(ANTARES_ROOT)/src/data/info.cpp \
	$(ANTARES_ROOT)/src/data/initial.cpp \
	$(ANTARES_ROOT)/src/data/interface.cpp \
	$(ANTARES_ROOT)/src/data/level.cpp \
	$(ANTARES_ROOT)/src/data/object-ref.cpp \
	$(ANTARES_ROOT)/src/data/plugin.cpp \
	$(ANTARES_ROOT)/src/data/races.cpp \
	$(ANTARES_ROOT)/src/data/replay.cpp \
	$(ANTARES_ROOT)/src/data/resource.cpp \
	$(ANTARES_ROOT)/src/data/sprite-data.cpp \
	$(ANTARES_ROOT)/src/drawing/briefing.cpp \
	$(ANTARES_ROOT)/src/drawing/build-pix.cpp \
	$(ANTARES_ROOT)/src/drawing/color.cpp \
	$(ANTARES_ROOT)/src/drawing/interface.cpp \
	$(ANTARES_ROOT)/src/drawing/libpng-pix-map.cpp \
	$(ANTARES_ROOT)/src/drawing/pix-map.cpp \
	$(ANTARES_ROOT)/src/drawing/pix-table.cpp \
	$(ANTARES_ROOT)/src/drawing/shapes.cpp \
	$(ANTARES_ROOT)/src/drawing/sprite-handling.cpp \
	$(ANTARES_ROOT)/src/drawing/styled-text.cpp \
	$(ANTARES_ROOT)/src/drawing/text.cpp \
	$(ANTARES_ROOT)/src/game/action.cpp \
	$(ANTARES_ROOT)/src/game/admiral.cpp \
	$(ANTARES_ROOT)/src/game/cheat.cpp \
	$(ANTARES_ROOT)/src/game/condition.cpp \
	$(ANTARES_ROOT)/src/game/cursor.cpp \
	$(ANTARES_ROOT)/src/game/globals.cpp \
	$(ANTARES_ROOT)/src/game/initial.cpp \
	$(ANTARES_ROOT)/src/game/input-source.cpp \
	$(ANTARES_ROOT)/src/game/instruments.cpp \
	$(ANTARES_ROOT)/src/game/labels.cpp \
	$(ANTARES_ROOT)/src/game/level.cpp \
	$(ANTARES_ROOT)/src/game/main.cpp \
	$(ANTARES_ROOT)/src/game/messages.cpp \
	$(ANTARES_ROOT)/src/game/minicomputer.cpp \
	$(ANTARES_ROOT)/src/game/motion.cpp \
	$(ANTARES_ROOT)/src/game/non-player-ship.cpp \
	$(ANTARES_ROOT)/src/game/player-ship.cpp \
	$(ANTARES_ROOT)/src/game/space-object.cpp \
	$(ANTARES_ROOT)/src/game/starfield.cpp \
	$(ANTARES_ROOT)/src/game/sys.cpp \
	$(ANTARES_ROOT)/src/game/vector.cpp \
	$(ANTARES_ROOT)/src/lang/exception.cpp \
	$(ANTARES_ROOT)/src/math/fixed.cpp \
	$(ANTARES_ROOT)/src/math/geometry.cpp \
	$(ANTARES_ROOT)/src/math/random.cpp \
	$(ANTARES_ROOT)/src/math/rotation.cpp \
	$(ANTARES_ROOT)/src/math/scale.cpp \
	$(ANTARES_ROOT)/src/math/special.cpp \
	$(ANTARES_ROOT)/src/sound/driver.cpp \
	$(ANTARES_ROOT)/src/sound/fx.cpp \
	$(ANTARES_ROOT)/src/sound/music.cpp \
	$(ANTARES_ROOT)/src/sound/openal-driver.cpp \
	$(ANTARES_ROOT)/src/ui/card.cpp \
	$(ANTARES_ROOT)/src/ui/editable-text.cpp \
	$(ANTARES_ROOT)/src/ui/event-scheduler.cpp \
	$(ANTARES_ROOT)/src/ui/event.cpp \
	$(ANTARES_ROOT)/src/ui/flows/master.cpp \
	$(ANTARES_ROOT)/src/ui/flows/replay-game.cpp \
	$(ANTARES_ROOT)/src/ui/flows/solo-game.cpp \
	$(ANTARES_ROOT)/src/ui/interface-handling.cpp \
	$(ANTARES_ROOT)/src/ui/screen.cpp \
	$(ANTARES_ROOT)/src/ui/screens/briefing.cpp \
	$(ANTARES_ROOT)/src/ui/screens/debriefing.cpp \
	$(ANTARES_ROOT)/src/ui/screens/help.cpp \
	$(ANTARES_ROOT)/src/ui/screens/loading.cpp \
	$(ANTARES_ROOT)/src/ui/screens/main.cpp \
	$(ANTARES_ROOT)/src/ui/screens/object-data.cpp \
	$(ANTARES_ROOT)/src/ui/screens/options.cpp \
	$(ANTARES_ROOT)/src/ui/screens/play-again.cpp \
	$(ANTARES_ROOT)/src/ui/screens/scroll-text.cpp \
	$(ANTARES_ROOT)/src/ui/screens/select-level.cpp \
	$(ANTARES_ROOT)/src/ui/widget.cpp \
	$(ANTARES_ROOT)/src/video/driver.cpp \
	$(ANTARES_ROOT)/src/video/opengl-driver.cpp \
	$(ANTARES_ROOT)/src/video/transitions.cpp \
	$(ANTARES_ROOT)/out/src/video/glsl/fragment.cpp \
	$(ANTARES_ROOT)/out/src/video/glsl/vertex.cpp

ifeq ($(TARGET_OS),mac)
LIBANTARES_CXX_SRCS += \
	$(ANTARES_ROOT)/src/mac/core-foundation.cpp \
	$(ANTARES_ROOT)/src/mac/core-opengl.cpp \
	$(ANTARES_ROOT)/src/mac/http.cpp
endif
ifeq ($(TARGET_OS),linux)
endif
ifeq ($(TARGET_OS),win)
endif

LIBANTARES_CXX_OBJS := $(LIBANTARES_CXX_SRCS:%=$(OUT)/%.o)
LIBANTARES_OBJS := $(LIBANTARES_CXX_OBJS)

$(LIBANTARES_A): $(LIBANTARES_OBJS)
	$(AR) rcs $@ $+

out/src/video/glsl/fragment.cpp: src/video/glsl/fragment.frag
	$(MKDIR_P) $(dir $@)
	build/lib/scripts/embed.py $< /dev/null $@ antares::glsl::fragment

out/src/video/glsl/vertex.cpp: src/video/glsl/vertex.vert
	$(MKDIR_P) $(dir $@)
	build/lib/scripts/embed.py $< /dev/null $@ antares::glsl::vertex

$(LIBANTARES_CXX_OBJS): $(OUT)/%.cpp.o: %.cpp
	@$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LIBANTARES_CPPFLAGS) -c $< -o $@

-include $(LIBANTARES_OBJS:.o=.d)
