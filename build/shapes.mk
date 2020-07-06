# Copyright (C) 2020 The Antares Authors
# This file is part of Antares, a tactical space combat game.
# Antares is free software, distributed under the LGPL+. See COPYING.

SHAPES := $(OUT)/shapes$(EXE_SUFFIX)
SHAPES_CXX_SRCS := \
	$(ANTARES_ROOT)/src/bin/shapes.cpp
SHAPES_CXX_OBJS := $(SHAPES_CXX_SRCS:%=$(OUT)/%.o)
SHAPES_OBJS := $(SHAPES_CXX_OBJS)

SHAPES_LDFLAGS := \
	$(LIBGLFW3_LDFLAGS) \
	$(LIBANTARES_LDFLAGS)

$(SHAPES): $(SHAPES_OBJS) $(LIBANTARES_TEST_OBJS) $(LIBANTARES_A) $(LIBGLFW3_A) $(LIBPNG_A) $(LIBSNDFILE_A) $(LIBMODPLUG_A) $(LIBZIPXX_A) $(LIBSFZ_A) $(LIBPROCYON_CPP_A) $(LIBPROCYON_A)
	$(CXX) $+ -o $@ $(LDFLAGS) $(SHAPES_LDFLAGS)

$(SHAPES_CXX_OBJS): $(OUT)/%.cpp.o: %.cpp
	@$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LIBANTARES_CPPFLAGS) -c $< -o $@

-include $(SHAPES_OBJS:.o=.d)
