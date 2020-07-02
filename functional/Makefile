#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

#
# Copyright 2019 Joyent, Inc.
# Copyright 2020 Oxide Computer Company
#

CFLAGS	= -fno-omit-frame-pointer
TESTS	= $(PLAIN_TESTS) $(THREAD_TESTS)
PLAIN_TESTS	=	test_create test_depth1 test_depth2 test_dir test_file
PLAIN_TESTS	+=	test_loop test_nested test_nested_et test_replace
PLAIN_TESTS	+=	test_oneshot test_timeout test_errevent test_hupevent
THREAD_TESTS	=	test_et test_exclusive
COMMON	= common.o

UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
    CFLAGS += -D_OS_LINUX
endif
ifeq ($(UNAME), SunOS)
    CFLAGS += -D_OS_SUNOS
    PLAIN_TESTS	+=	test_illumos
endif

all: $(TESTS)

$(TESTS): % : %.o $(COMMON)

$(THREAD_TESTS:%=%.o) $(COMMON): CFLAGS += -pthread
$(THREAD_TESTS): LDFLAGS += -pthread

run:
	for n in $(TESTS); do ./$$n; done

clean:
	rm -f $(TESTS) *.o

.PHONY: clean run
