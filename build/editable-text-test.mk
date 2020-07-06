# Copyright (C) 2020 The Antares Authors
# This file is part of Antares, a tactical space combat game.
# Antares is free software, distributed under the LGPL+. See COPYING.

EDITABLE_TEXT_TEST := $(OUT)/editable-text-test$(EXE_SUFFIX)
EDITABLE_TEXT_TEST_CXX_SRCS := \
	$(ANTARES_ROOT)/src/ui/editable-text.test.cpp
EDITABLE_TEXT_TEST_CXX_OBJS := $(EDITABLE_TEXT_TEST_CXX_SRCS:%=$(OUT)/%.o)
EDITABLE_TEXT_TEST_OBJS := $(EDITABLE_TEXT_TEST_CXX_OBJS)

EDITABLE_TEXT_TEST_LDFLAGS := \
	$(LIBGLFW3_LDFLAGS) \
	$(LIBANTARES_LDFLAGS)

$(EDITABLE_TEXT_TEST): $(COLOR_TEST_OBJS) $(LIBANTARES_A) $(LIBPNG_A) $(LIBSNDFILE_A) $(LIBMODPLUG_A) $(LIBZIPXX_A) $(LIBSFZ_A) $(LIBPROCYON_CPP_A) $(LIBPROCYON_A) $(LIBGMOCK_MAIN_A)
	$(CXX) $+ -o $@ $(LDFLAGS) $(EDITABLE_TEXT_TEST_LDFLAGS)

EDITABLE_TEXT_TEST_PRIVATE_CPPFLAGS := \
	$(LIBANTARES_CPPFLAGS) \
	$(LIBGMOCK_MAIN_CPPFLAGS)

$(EDITABLE_TEXT_TEST_CXX_OBJS): $(OUT)/%.cpp.o: %.cpp
	@$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(EDITABLE_TEXT_TEST_PRIVATE_CPPFLAGS) -c $< -o $@

-include $(EDITABLE_TEXT_TEST_OBJS:.o=.d)
