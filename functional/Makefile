
CFLAGS	= -fno-omit-frame-pointer
TESTS	= test_depth1 test_depth2 test_loop
COMMON	= common.o

all: $(TESTS)

$(TESTS): % : %.o $(COMMON)

clean:
	rm -f $(TESTS) *.o
.PHONEY: clean
