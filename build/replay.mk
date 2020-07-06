# Copyright (C) 2020 The Antares Authors
# This file is part of Antares, a tactical space combat game.
# Antares is free software, distributed under the LGPL+. See COPYING.

REPLAY := $(OUT)/replay$(EXE_SUFFIX)
REPLAY_CXX_SRCS := \
	$(ANTARES_ROOT)/src/bin/replay.cpp
REPLAY_CXX_OBJS := $(REPLAY_CXX_SRCS:%=$(OUT)/%.o)
REPLAY_OBJS := $(REPLAY_CXX_OBJS)

REPLAY_LDFLAGS := \
	$(LIBGLFW3_LDFLAGS) \
	$(LIBANTARES_LDFLAGS)

$(REPLAY): $(REPLAY_OBJS) $(LIBANTARES_TEST_OBJS) $(LIBANTARES_A) $(LIBGLFW3_A) $(LIBPNG_A) $(LIBSNDFILE_A) $(LIBMODPLUG_A) $(LIBZIPXX_A) $(LIBSFZ_A) $(LIBPROCYON_CPP_A) $(LIBPROCYON_A)
	$(CXX) $+ -o $@ $(LDFLAGS) $(REPLAY_LDFLAGS)

$(REPLAY_CXX_OBJS): $(OUT)/%.cpp.o: %.cpp
	@$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LIBANTARES_CPPFLAGS) -c $< -o $@

-include $(REPLAY_OBJS:.o=.d)
