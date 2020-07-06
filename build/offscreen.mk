# Copyright (C) 2020 The Antares Authors
# This file is part of Antares, a tactical space combat game.
# Antares is free software, distributed under the LGPL+. See COPYING.

ifneq ($(TARGET_OS),win)

OFFSCREEN := $(OUT)/offscreen$(EXE_SUFFIX)
OFFSCREEN_CXX_SRCS := \
	$(ANTARES_ROOT)/src/bin/offscreen.cpp
OFFSCREEN_CXX_OBJS := $(OFFSCREEN_CXX_SRCS:%=$(OUT)/%.o)
OFFSCREEN_OBJS := $(OFFSCREEN_CXX_OBJS)

OFFSCREEN_LDFLAGS := \
	$(LIBANTARES_LDFLAGS)

$(OFFSCREEN): $(OFFSCREEN_OBJS) $(LIBANTARES_TEST_OBJS) $(LIBANTARES_A) $(LIBPNG_A) $(LIBSNDFILE_A) $(LIBMODPLUG_A) $(LIBZIPXX_A) $(LIBSFZ_A) $(LIBPROCYON_CPP_A) $(LIBPROCYON_A) $(ZLIB_A)
	$(CXX) $+ -o $@ $(LDFLAGS) $(OFFSCREEN_LDFLAGS)

$(OFFSCREEN_CXX_OBJS): $(OUT)/%.cpp.o: %.cpp
	@$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LIBANTARES_CPPFLAGS) -c $< -o $@

-include $(OFFSCREEN_OBJS:.o=.d)

endif
