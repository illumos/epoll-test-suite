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
	ev.events = 0;
	ev.data.fd = pipefd[0];
	if (epoll_ctl(efd, EPOLL_CTL_ADD, pipefd[0], &ev) < 0) {
		test_fatal("epoll_ctl(EPOLL_CTL_ADD)");
	}

	/* No events should be pending */
	test_equal(epoll_wait(efd, &ev, 1, 0), 0);

	if (close(pipefd[1]) != 0) {
		test_fatal("close()");
	}

	/*
	 * According to the Linux man page, the EPOLLERR event is reported event
	 * when it has not been set in the events for the subscription.
	 */
	test_equal(epoll_wait(efd, &ev, 1, 0), 1);
	test_equal(ev.events, EPOLLHUP);

	if (epoll_ctl(efd, EPOLL_CTL_DEL, pipefd[0], NULL) < 0) {
		test_fatal("epoll_ctl(EPOLL_CTL_DEL)");
	}

	ev.events = EPOLLONESHOT;
	ev.data.fd = pipefd[0];
	if (epoll_ctl(efd, EPOLL_CTL_ADD, pipefd[0], &ev) < 0) {
		test_fatal("epoll_ctl(EPOLL_CTL_ADD)");
	}

	/* Make sure it behaves as expected in oneshot mode too */
	test_equal(epoll_wait(efd, &ev, 1, 0), 1);
	test_equal(ev.events, EPOLLHUP);
	test_equal(epoll_wait(efd, &ev, 1, 0), 0);

	test_done();
}
