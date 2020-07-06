# Copyright (C) 2020 The Antares Authors
# This file is part of Antares, a tactical space combat game.
# Antares is free software, distributed under the LGPL+. See COPYING.

TINT := $(OUT)/tint$(EXE_SUFFIX)
TINT_CXX_SRCS := \
	$(ANTARES_ROOT)/src/bin/tint.cpp
TINT_CXX_OBJS := $(TINT_CXX_SRCS:%=$(OUT)/%.o)
TINT_OBJS := $(TINT_CXX_OBJS)

TINT_LDFLAGS := \
	$(LIBGLFW3_LDFLAGS) \
	$(LIBANTARES_LDFLAGS)

$(TINT): $(TINT_OBJS) $(LIBANTARES_TEST_OBJS) $(LIBANTARES_A) $(LIBGLFW3_A) $(LIBPNG_A) $(LIBSNDFILE_A) $(LIBMODPLUG_A) $(LIBZIPXX_A) $(LIBSFZ_A) $(LIBPROCYON_CPP_A) $(LIBPROCYON_A)
	$(CXX) $+ -o $@ $(LDFLAGS) $(TINT_LDFLAGS)

$(TINT_CXX_OBJS): $(OUT)/%.cpp.o: %.cpp
	@$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LIBANTARES_CPPFLAGS) -c $< -o $@

-include $(TINT_OBJS:.o=.d)
