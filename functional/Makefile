
CFLAGS	= -fno-omit-frame-pointer
TESTS	= $(PLAIN_TESTS) $(THREAD_TESTS)
PLAIN_TESTS	=	test_create test_depth1 test_depth2 test_dir test_file
PLAIN_TESTS	+=	test_loop test_nested test_nested_et test_replace
PLAIN_TESTS	+=	test_oneshot test_timeout test_errevent test_hupevent
THREAD_TESTS	=	test_et
COMMON	= common.o

UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
    CFLAGS += -D_OS_LINUX
endif
ifeq ($(UNAME), SunOS)
    CFLAGS += -D_OS_SUNOS
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
