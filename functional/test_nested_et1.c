#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/epoll.h>

#include "common.h"

int
main(int argc, char **argv)
{
	int efd, nefd, pipefd[2];
	struct epoll_event ev;

	test_init(argc, argv);

	if (pipe(pipefd) != 0) {
		test_fatal("pipe()");
	}

	if ((efd = epoll_create1(0)) < 0) {
		test_fatal("epoll_create1()");
	}
	ev.events = EPOLLIN;
	ev.data.fd = pipefd[0];
	if (epoll_ctl(efd, EPOLL_CTL_ADD, pipefd[0], &ev) < 0) {
		test_fatal("epoll_ctl(EPOLL_CTL_ADD)");
	}

	if ((nefd = epoll_create1(0)) < 0) {
		test_fatal("epoll_create1()");
	}
	ev.events = EPOLLIN|EPOLLRDNORM|EPOLLET;
	ev.data.fd = efd;
	if (epoll_ctl(nefd, EPOLL_CTL_ADD, efd, &ev) < 0) {
		test_fatal("epoll_ctl(EPOLL_CTL_ADD)");
	}

	/* No events should be pending yet */
	test_equal(epoll_wait(efd, &ev, 1, 0), 0);
	test_equal(epoll_wait(nefd, &ev, 1, 0), 0);

	if (write(pipefd[1], "\0", 1) != 1) {
		test_fatal("write()");
	}

	test_equal(epoll_wait(efd, &ev, 1, 0), 1);
	test_equal(ev.events, EPOLLIN);
	test_equal(ev.data.fd, pipefd[0]);

	test_equal(epoll_wait(nefd, &ev, 1, 0), 1);
	test_equal(ev.events, EPOLLIN|EPOLLRDNORM);
	test_equal(ev.data.fd, efd);

	/* Despite the request of edge-triggering, nested epoll shouldn't */
	test_equal(epoll_wait(nefd, &ev, 1, 0), 1);
	test_equal(ev.events, EPOLLIN|EPOLLRDNORM);
	test_equal(ev.data.fd, efd);

	test_done();
}
