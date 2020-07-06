# Copyright (C) 2020 The Antares Authors
# This file is part of Antares, a tactical space combat game.
# Antares is free software, distributed under the LGPL+. See COPYING.

COLOR_TEST := $(OUT)/color-test$(EXE_SUFFIX)
COLOR_TEST_CXX_SRCS := \
	$(ANTARES_ROOT)/src/drawing/color.test.cpp
COLOR_TEST_CXX_OBJS := $(COLOR_TEST_CXX_SRCS:%=$(OUT)/%.o)
COLOR_TEST_OBJS := $(COLOR_TEST_CXX_OBJS)

COLOR_TEST_LDFLAGS := \
	$(LIBGLFW3_LDFLAGS) \
	$(LIBANTARES_LDFLAGS)

$(COLOR_TEST): $(COLOR_TEST_OBJS) $(LIBANTARES_A) $(LIBPNG_A) $(LIBSNDFILE_A) $(LIBMODPLUG_A) $(LIBZIPXX_A) $(LIBSFZ_A) $(LIBPROCYON_CPP_A) $(LIBPROCYON_A) $(LIBGMOCK_MAIN_A)
	$(CXX) $+ -o $@ $(LDFLAGS) $(COLOR_TEST_LDFLAGS)

COLOR_TEST_PRIVATE_CPPFLAGS := \
	$(LIBANTARES_CPPFLAGS) \
	$(LIBGMOCK_MAIN_CPPFLAGS)

$(COLOR_TEST_CXX_OBJS): $(OUT)/%.cpp.o: %.cpp
	@$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(COLOR_TEST_PRIVATE_CPPFLAGS) -c $< -o $@

-include $(COLOR_TEST_OBJS:.o=.d)
