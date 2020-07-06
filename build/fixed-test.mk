# Copyright (C) 2020 The Antares Authors
# This file is part of Antares, a tactical space combat game.
# Antares is free software, distributed under the LGPL+. See COPYING.

FIXED_TEST := $(OUT)/fixed-test$(EXE_SUFFIX)
FIXED_TEST_CXX_SRCS := \
	$(ANTARES_ROOT)/src/math/fixed.test.cpp
FIXED_TEST_CXX_OBJS := $(FIXED_TEST_CXX_SRCS:%=$(OUT)/%.o)
FIXED_TEST_OBJS := $(FIXED_TEST_CXX_OBJS)

FIXED_TEST_LDFLAGS := \
	$(LIBANTARES_LDFLAGS)

$(FIXED_TEST): $(COLOR_TEST_OBJS) $(LIBANTARES_A) $(LIBPNG_A) $(LIBSNDFILE_A) $(LIBMODPLUG_A) $(LIBZIPXX_A) $(LIBSFZ_A) $(LIBPROCYON_CPP_A) $(LIBPROCYON_A) $(LIBGMOCK_MAIN_A) $(ZLIB_A)
	$(CXX) $+ -o $@ $(LDFLAGS) $(FIXED_TEST_LDFLAGS)

FIXED_TEST_PRIVATE_CPPFLAGS := \
	$(LIBANTARES_CPPFLAGS) \
	$(LIBGMOCK_MAIN_CPPFLAGS)

$(FIXED_TEST_CXX_OBJS): $(OUT)/%.cpp.o: %.cpp
	@$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(FIXED_TEST_PRIVATE_CPPFLAGS) -c $< -o $@

-include $(FIXED_TEST_OBJS:.o=.d)
