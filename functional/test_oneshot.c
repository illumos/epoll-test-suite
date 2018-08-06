#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/epoll.h>

#include "common.h"


int
main(int argc, char **argv)
{
	int efd, pipefd[2], res, err;
	struct epoll_event ev;
	char b = '\0';

	test_init(argc, argv);

	if (pipe(pipefd) != 0) {
		test_fatal("pipe()");
	}

	if ((efd = epoll_create1(0)) < 0) {
		test_fatal("epoll_create1()");
	}
	ev.events = EPOLLIN|EPOLLONESHOT;
	ev.data.fd = pipefd[0];
	if (epoll_ctl(efd, EPOLL_CTL_ADD, pipefd[0], &ev) < 0) {
		test_fatal("epoll_ctl(EPOLL_CTL_ADD)");
	}

	/* No events should be pending */
	test_equal(epoll_wait(efd, &ev, 1, 0), 0);

	if (write(pipefd[1], &b, 1) != 1) {
		test_fatal("write()");
	}

	test_equal(epoll_wait(efd, &ev, 1, 0), 1);

	if (read(pipefd[0], &b, 1) != 1) {
		test_fatal("read()");
	}

	ev.events = EPOLLIN|EPOLLONESHOT;
	ev.data.fd = pipefd[0];
	if (epoll_ctl(efd, EPOLL_CTL_MOD, pipefd[0], &ev) < 0) {
		test_fatal("epoll_ctl(EPOLL_CTL_MOD)");
	}

	/* check re-armed fd for event handling */
	test_equal(epoll_wait(efd, &ev, 1, 0), 0);
	if (write(pipefd[1], &b, 1) != 1) {
		test_fatal("write()");
	}
	test_equal(epoll_wait(efd, &ev, 1, 0), 1);
	if (read(pipefd[0], &b, 1) != 1) {
		test_fatal("read()");
	}

	if (epoll_ctl(efd, EPOLL_CTL_DEL, pipefd[0], &ev) < 0) {
		test_fatal("epoll_ctl(EPOLL_CTL_ADD)");
	}

	test_done();
}
