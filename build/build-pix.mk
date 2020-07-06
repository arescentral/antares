# Copyright (C) 2020 The Antares Authors
# This file is part of Antares, a tactical space combat game.
# Antares is free software, distributed under the LGPL+. See COPYING.

ifneq ($(TARGET_OS),win)

BUILD_PIX := $(OUT)/build-pix$(EXE_SUFFIX)
BUILD_PIX_CXX_SRCS := \
	$(ANTARES_ROOT)/src/bin/build-pix.cpp
BUILD_PIX_CXX_OBJS := $(BUILD_PIX_CXX_SRCS:%=$(OUT)/%.o)
BUILD_PIX_OBJS := $(BUILD_PIX_CXX_OBJS)

BUILD_PIX_LDFLAGS := \
	$(LIBANTARES_LDFLAGS)

$(BUILD_PIX): $(BUILD_PIX_OBJS) $(LIBANTARES_TEST_OBJS) $(LIBANTARES_A) $(LIBPNG_A) $(LIBSNDFILE_A) $(LIBMODPLUG_A) $(LIBZIPXX_A) $(LIBSFZ_A) $(LIBPROCYON_CPP_A) $(LIBPROCYON_A) $(ZLIB_A)
	$(CXX) $+ -o $@ $(LDFLAGS) $(BUILD_PIX_LDFLAGS)

$(BUILD_PIX_CXX_OBJS): $(OUT)/%.cpp.o: %.cpp
	@$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LIBANTARES_CPPFLAGS) -c $< -o $@

-include $(BUILD_PIX_OBJS:.o=.d)

endif
