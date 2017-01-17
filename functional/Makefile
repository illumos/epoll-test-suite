
CFLAGS	= -fno-omit-frame-pointer
TESTS	= $(PLAIN_TESTS) $(THREAD_TESTS)
PLAIN_TESTS	= test_depth1 test_depth2 test_loop test_nested1
THREAD_TESTS	= test_et1
COMMON	= common.o

all: $(TESTS)

$(TESTS): % : %.o $(COMMON)

$(THREAD_TESTS:%=%.o) $(COMMON): CFLAGS += -pthread
$(THREAD_TESTS): LDFLAGS += -pthread

clean:
	rm -f $(TESTS) *.o
.PHONEY: clean
