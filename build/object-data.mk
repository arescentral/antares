# Copyright (C) 2020 The Antares Authors
# This file is part of Antares, a tactical space combat game.
# Antares is free software, distributed under the LGPL+. See COPYING.

OBJECT_DATA := $(OUT)/object-data$(EXE_SUFFIX)
OBJECT_DATA_CXX_SRCS := \
	$(ANTARES_ROOT)/src/bin/object-data.cpp
OBJECT_DATA_CXX_OBJS := $(OBJECT_DATA_CXX_SRCS:%=$(OUT)/%.o)
OBJECT_DATA_OBJS := $(OBJECT_DATA_CXX_OBJS)

OBJECT_DATA_LDFLAGS := \
	$(LIBGLFW3_LDFLAGS) \
	$(LIBANTARES_LDFLAGS)

$(OBJECT_DATA): $(OBJECT_DATA_OBJS) $(LIBANTARES_TEST_OBJS) $(LIBANTARES_A) $(LIBGLFW3_A) $(LIBPNG_A) $(LIBSNDFILE_A) $(LIBMODPLUG_A) $(LIBZIPXX_A) $(LIBSFZ_A) $(LIBPROCYON_CPP_A) $(LIBPROCYON_A)
	$(CXX) $+ -o $@ $(LDFLAGS) $(OBJECT_DATA_LDFLAGS)

$(OBJECT_DATA_CXX_OBJS): $(OUT)/%.cpp.o: %.cpp
	@$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LIBANTARES_CPPFLAGS) -c $< -o $@

-include $(OBJECT_DATA_OBJS:.o=.d)
