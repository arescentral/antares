# Copyright (C) 2020 The Antares Authors
# This file is part of Antares, a tactical space combat game.
# Antares is free software, distributed under the LGPL+. See COPYING.

LIBANTARES_TEST_A := $(OUT)/libantares-test.a
LIBANTARES_TEST_CXX_SRCS := \
	$(ANTARES_ROOT)/src/config/test-dirs.cpp \
	$(ANTARES_ROOT)/src/video/text-driver.cpp

ifeq ($(TARGET_OS),mac)
LIBANTARES_TEST_CXX_SRCS += \
	$(ANTARES_ROOT)/src/mac/offscreen.cpp \
	$(ANTARES_ROOT)/src/video/offscreen-driver.cpp
endif
ifeq ($(TARGET_OS),linux)
LIBANTARES_TEST_CXX_SRCS += \
	$(ANTARES_ROOT)/src/linux/offscreen.cpp \
	$(ANTARES_ROOT)/src/video/offscreen-driver.cpp
endif
ifeq ($(TARGET_OS),win)
endif

LIBANTARES_TEST_CXX_OBJS := $(LIBANTARES_TEST_CXX_SRCS:%=$(OUT)/%.o)
LIBANTARES_TEST_OBJS := $(LIBANTARES_TEST_CXX_OBJS)

$(LIBANTARES_TEST_A): $(LIBANTARES_TEST_OBJS)
	$(AR) rcs $@ $+

LIBANTARES_TEST_PRIVATE_CPPFLAGS := \
	$(LIBANTARES_CPPFLAGS) \
	-D ANTARES_DATA=./data

$(LIBANTARES_TEST_CXX_OBJS): $(OUT)/%.cpp.o: %.cpp
	@$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LIBANTARES_TEST_PRIVATE_CPPFLAGS) -c $< -o $@

-include $(LIBANTARES_TEST_OBJS:.o=.d)
