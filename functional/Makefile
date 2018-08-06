
CFLAGS	= -fno-omit-frame-pointer
TESTS	= $(PLAIN_TESTS) $(THREAD_TESTS)
PLAIN_TESTS	=	test_create test_depth1 test_depth2 test_dir test_file
PLAIN_TESTS	+=	test_loop test_nested test_nested_et test_replace test_timeout
PLAIN_TESTS	+=	test_oneshot
THREAD_TESTS	=	test_et
COMMON	= common.o

UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
    CFLAGS += -D_OS_LINUX
endif

all: $(TESTS)

$(TESTS): % : %.o $(COMMON)

$(THREAD_TESTS:%=%.o) $(COMMON): CFLAGS += -pthread
$(THREAD_TESTS): LDFLAGS += -pthread

clean:
	rm -f $(TESTS) *.o
.PHONEY: clean
