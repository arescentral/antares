# Copyright (C) 2020 The Antares Authors
# This file is part of Antares, a tactical space combat game.
# Antares is free software, distributed under the LGPL+. See COPYING.

LIBANTARES_RELEASE_A := $(OUT)/libantares-release.a
LIBANTARES_RELEASE_CXX_SRCS :=

ifeq ($(TARGET_OS),mac)
LIBANTARES_RELEASE_CXX_SRCS += \
	$(ANTARES_ROOT)/src/config/mac-dirs.cpp
endif
ifeq ($(TARGET_OS),linux)
LIBANTARES_RELEASE_CXX_SRCS += \
	$(ANTARES_ROOT)/src/config/linux-dirs.cpp
endif
ifeq ($(TARGET_OS),win)
LIBANTARES_RELEASE_CXX_SRCS += \
	$(ANTARES_ROOT)/src/config/win-dirs.cpp
endif

LIBANTARES_RELEASE_CXX_OBJS := $(LIBANTARES_RELEASE_CXX_SRCS:%=$(OUT)/%.o)
LIBANTARES_RELEASE_OBJS := $(LIBANTARES_RELEASE_CXX_OBJS)

$(LIBANTARES_RELEASE_A): $(LIBANTARES_RELEASE_OBJS)
	$(AR) rcs $@ $+

LIBANTARES_RELEASE_PRIVATE_CPPFLAGS := \
	$(LIBANTARES_CPPFLAGS)

$(LIBANTARES_RELEASE_CXX_OBJS): $(OUT)/%.cpp.o: %.cpp
	@$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LIBANTARES_RELEASE_PRIVATE_CPPFLAGS) -c $< -o $@

-include $(LIBANTARES_RELEASE_OBJS:.o=.d)
