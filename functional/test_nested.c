/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Copyright 2019 Joyent, Inc.
 */

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
	/* cast a wide net */
	ev.events = EPOLLIN|EPOLLRDNORM|EPOLLRDBAND|EPOLLMSG;
	ev.events |= EPOLLOUT|EPOLLWRNORM|EPOLLWRBAND;
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

	test_done();
}
