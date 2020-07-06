# Copyright (C) 2020 The Antares Authors
# This file is part of Antares, a tactical space combat game.
# Antares is free software, distributed under the LGPL+. See COPYING.

ifneq ($(TARGET_OS),win)

ANTARES := $(OUT)/antares$(EXE_SUFFIX)
ANTARES_CXX_SRCS := \
	$(ANTARES_ROOT)/src/glfw/video-driver.cpp \
	$(ANTARES_ROOT)/src/glfw/main.cpp
ANTARES_CXX_OBJS := $(ANTARES_CXX_SRCS:%=$(OUT)/%.o)
ANTARES_OBJS := $(ANTARES_CXX_OBJS)

ANTARES_CPPFLAGS := \
	$(LIBGLFW3_LDFLAGS) \
	$(LIBANTARES_LDFLAGS)
ANTARES_LDFLAGS := \
	$(LIBGLFW3_LDFLAGS) \
	$(LIBANTARES_LDFLAGS)

$(ANTARES): $(ANTARES_OBJS) $(LIBANTARES_RELEASE_OBJS) $(LIBANTARES_A) $(LIBGLFW3_A) $(LIBPNG_A) $(LIBSNDFILE_A) $(LIBMODPLUG_A) $(LIBZIPXX_A) $(LIBSFZ_A) $(LIBPROCYON_CPP_A) $(LIBPROCYON_A) $(ZLIB_A)
	$(CXX) $+ -o $@ $(LDFLAGS) $(ANTARES_LDFLAGS)

$(ANTARES_CXX_OBJS): $(OUT)/%.cpp.o: %.cpp
	@$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(ANTARES_CPPFLAGS) -c $< -o $@

-include $(ANTARES_OBJS:.o=.d)

endif
