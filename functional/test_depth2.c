#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <errno.h>

#include "common.h"

#define	T_TARGET_DEPTH	5

int
main(int argc, char **argv)
{
	int pipe_fd[2], test_fd[T_TARGET_DEPTH + 1];
	int efd, pefd, res, i;
	struct epoll_event ev;
	char buf;

	test_init(argc, argv);

	if (pipe(pipe_fd) != 0) {
		test_fatal("pipe()");
	}
	if (write(pipe_fd[1], "\0", 1) != 1) {
		test_fatal("write()");
	}

	if ((test_fd[0] = epoll_create1(0)) < 0) {
		test_fatal("epoll_create1()");
	}

	for (i = 1; i <= T_TARGET_DEPTH; i++) {
		if ((efd = epoll_create1(0)) < 0) {
			test_fatal("epoll_create1()");
		}
		ev.events = EPOLLIN;
		ev.data.fd = efd;
		res = epoll_ctl(test_fd[i-1], EPOLL_CTL_ADD, efd, &ev);

		if (res != 0) {
			test_fatal("epoll_ctl(EPOLL_CTL_ADD)");
		}
		test_fd[i] = efd;
		/* Keep track of the parent epoll fd, too */
		pefd = test_fd[i-1];
	}

	ev.events = EPOLLIN;
	ev.data.fd = pipe_fd[0];
	if (epoll_ctl(efd, EPOLL_CTL_ADD, pipe_fd[0], &ev) < 0) {
		test_equal(errno, EINVAL);
	} else {
		test_fatal("too deep");
	}

	/*
	 * Remove the last epoll descriptor from the chain, add the pipe to it,
	 * then attempt to re-add it back.
	 */
	if (epoll_ctl(pefd, EPOLL_CTL_DEL, efd, NULL) < 0) {
			test_fatal("epoll_ctl(EPOLL_CTL_DEL)");
	}
	if (epoll_ctl(efd, EPOLL_CTL_ADD, pipe_fd[0], &ev) < 0) {
			test_fatal("epoll_ctl(EPOLL_CTL_ADD)");
	}

	ev.data.fd = efd;
	if (epoll_ctl(pefd, EPOLL_CTL_ADD, efd, &ev) < 0) {
		test_equal(errno, EINVAL);
	} else {
		test_fatal("too deep");
	}

	test_done();
}
