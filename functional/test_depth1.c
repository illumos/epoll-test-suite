#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>

#include "common.h"


#define	T_MAX_DEPTH	8
#define	T_TARGET_DEPTH	5

int main(int argc, char **argv)
{
	int pipefd[2], testfd[T_MAX_DEPTH], pfd, i;
	struct epoll_event ev;

	test_init(argc, argv);

	if (pipe(pipefd) != 0) {
		test_fatal("pipe()");
	}
	if (write(pipefd[1], "\0", 1) != 1) {
		test_fatal("write()");
	}

	pfd = pipefd[0];

	for (i = 0; i < T_MAX_DEPTH; i++) {
		int efd, res;

		ev.events = EPOLLIN;
		ev.data.fd = pfd;

		if ((efd = epoll_create1(0)) < 0) {
			test_fatal("epoll_create1()");
		}
		res = epoll_ctl(efd, EPOLL_CTL_ADD, pfd, &ev);

		if (res == 0) {
			if (i >= T_TARGET_DEPTH) {
				// too deep
				test_fail("too deep");
			} else {
				test_pass();
			}

			testfd[i] = efd;
			pfd = efd;
		} else {
			if (i == T_TARGET_DEPTH) {
				test_pass();
				break;
			} else {
				test_fatal("epoll_ctl(EPOLL_CTL_ADD)");
			}
		}

	}
	if (epoll_wait(pfd, &ev, 1, 1) != 1) {
		test_fail("event missing");
	} else {
		test_pass();
	}

	test_done();
}
